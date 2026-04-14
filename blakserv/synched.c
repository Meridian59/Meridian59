// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * synched.c
 *

 This module supports one mode of a session, STATE_SYNCED.  This is
 the state that a session is in if the user is running the Blakston
 client and has synchronized with a couple strings sent back and forth
 (see trysync.c or resync.c).  The user is here when they have to
 login, and also when they are at the client main menu.
 */

#include "blakserv.h"
#include <sstream>

/* local function prototypes */
void SynchedProtocolParse(session_node *s,client_msg *msg);
void SynchedAcceptLogin(session_node *s,char *name,char *password);
void LogUserData(session_node *s);
void SynchedSendMenuChoice(session_node *s);
void SynchedDoMenu(session_node *s);
void SynchedGotoGame(session_node *s,int last_download_time);
void SynchedSendDLFileInfo(char *str,int time,int type,int size);
void SynchedAddDelFile(char *str);

void SynchedInit(session_node *s)
{
   /* if you ever get here, you ARE running our client */
   s->blak_client = true;
   
   s->syn = (synched_data *)s->session_state_data;

   s->syn->failed_tries = 0;
   s->syn->download_count = 0;
   
   SetSessionTimer(s,60*ConfigInt(INACTIVE_SYNCHED));

   if (s->account == NULL)      /* only need password if first time in this mode */
   {
      AddByteToPacket(AP_GETLOGIN);
      SendPacket(s->session_id);
   }
   else
   {
      /* if login not verified, we're sitting here waiting for portal to say ok */
      if (s->login_verified)
			SynchedDoMenu(s);
   }
}

void SynchedExit(session_node *s)
{
}

void SynchedProcessSessionTimer(session_node *s)
{
   AddByteToPacket(AP_TIMEOUT);
   SendPacket(s->session_id);
   HangupSession(s);
}

void SynchedProcessSessionBuffer(session_node *s)
{
   client_msg msg;
   
   SetSessionTimer(s,60*ConfigInt(INACTIVE_SYNCHED));

   /* need to copy only as many bytes as we can hold */
   while (s->receive_list != NULL)
   {
      if (PeekSessionBytes(s,HEADERBYTES,&msg) == false)
	 return;

      if (msg.len != msg.len_verify || msg.seqno != 0)
      {
	 AddByteToPacket(AP_RESYNC);
	 SendPacket(s->session_id);
	 SetSessionState(s,STATE_RESYNC);
	 return;
      }

      if (msg.len > LEN_MAX_CLIENT_MSG)
      {
	 eprintf("SynchedProcessSessionBuffer got message too long %i\n",msg.len);
	 AddByteToPacket(AP_RESYNC);
	 SendPacket(s->session_id);
	 SetSessionState(s,STATE_RESYNC);

	 return;
      }
      
      /* now read the header for real, plus the actual data */
      if (ReadSessionBytes(s,msg.len+HEADERBYTES,&msg) == false)
	 return;

#if 0
      if (GetCRC16(msg.data,msg.len) != msg.crc16)
      {
	 /* dprintf("SPSB found bad CRC16\n"); */
	 AddByteToPacket(AP_RESYNC);
	 SendPacket(s->session_id);
	 SetSessionState(s,STATE_RESYNC);
	 return;
      }
#endif

      SynchedProtocolParse(s,&msg);

      /* if hung up, don't touch */
      if (s->hangup == true)
	 return;

      if (s->state != STATE_SYNCHED)
	 return;
   }
}

void SynchedProtocolParse(session_node *s,client_msg *msg)
{
   char name[MAX_LOGIN_NAME+1],password[MAX_LOGIN_PASSWORD+1];
   short len;
   int index;
   int last_download_time;
   int pkkcatch;

   /*
   {
      int i;
      dprintf("sess %i len %i cmd %u: ",s->session_id,msg->len,
	      msg->data[0]); 
      for (i=0;i<msg->len;i++)
	 dprintf("%02X ",msg->data[i]);
      dprintf("\n");
   }
   */

   if (msg->len == 0)
      return;

   if (s->syn->download_count != 0 && msg->data[0] != AP_PING)
   {
      s->syn->download_count = 0;
      InterfaceUpdateSession(s);
   }

   
   index = 1;

   if (msg->data[0] != AP_LOGIN && s->account == NULL)
   {
      /* shouldn't be possible, but for security we must check this */
      return;
   }

   switch (msg->data[0])
   {
   case AP_LOGIN : 
      if (msg->len < 75) /* fixed size of AP_LOGIN, before strings */
	 break;
      s->version_major = *(char *)(msg->data+index);
      index++;
      s->version_minor = *(char *)(msg->data+index);
      index++;
      s->os_type = *(int *)(msg->data+index);
      index += 4;
      s->os_version_major = *(int *)(msg->data+index);
      index += 4;
      s->os_version_minor = *(int *)(msg->data+index);
      index += 4;
      s->machine_ram = *(int *)(msg->data+index);
      index += 4;

      s->screen_x = *(short *)(msg->data+index);
      index += 2;
      s->screen_y = *(short *)(msg->data+index);
      index += 2;

      s->screen_color_depth = *(int *)(msg->data+index);
      index += 4;
      s->os_build_number = *(int *)(msg->data+index);
      index += 4;
      s->renderer_mode = *(int *)(msg->data+index);
      index += 4;
      s->vram_mb = *(int *)(msg->data+index);
      index += 4;
      s->session_bucket = *(int *)(msg->data+index);
      index += 4;
      s->gpu_vendor_id = *(int *)(msg->data+index);
      index += 4;
      s->last_avg_fps = *(int *)(msg->data+index);
      index += 4;
      s->last_low_fps = *(int *)(msg->data+index);
      index += 4;
      s->last_max_fps = *(int *)(msg->data+index);
      index += 4;
      s->partner = (short)*(int *)(msg->data+index);
      index += 4;
      s->is_wine = *(int *)(msg->data+index);
      index += 4;
      s->crc16 = *(int *)(msg->data+index);
      index += 4;

      // The following line was commented out because I added support for the 3 4-byte integers
      // index += 12; /* 12 bytes future expansion space */

      len = *(short *)(msg->data + index);
      if (index + 2 + len > msg->len) /* 2 = length word len */
         break;
      if (len >= sizeof(name))
         break;
      memcpy(name, msg->data + index + 2, len);
      name[len] = 0; /* null terminate string */
      index += 2 + len;
      
      len = *(short *)(msg->data + index);
      if (index + 2 + len > msg->len)
         break;
      if (len >= sizeof(password))
         break;
      memcpy(password, msg->data + index + 2, len);
      password[len] = 0; /* null terminate string */
      index += 2 + len;
      
      if (index + 2 <= msg->len) {
         len = *(short *)(msg->data + index);
         if (index + 2 + len <= msg->len) {
            s->gpu_desc.assign((char*)(msg->data + index + 2), len);
            index += 2 + len;
         }
      }
      
      SynchedAcceptLogin(s,name,password);
      
      break;
   case AP_GETCLIENT :
      eprintf("SynchedProtocolParse AP_GETCLIENT no longer supported\n");
      break;
   case AP_REQ_GAME :
		lprintf("SynchedProtocolParse account %u version %u %u\n",s->account->account_id,s->version_major,s->version_minor);
      if (s->version_major * 100 + s->version_minor < ConfigInt(LOGIN_MIN_VERSION))
      {
         AddByteToPacket(AP_MESSAGE);
         AddStringToPacket(strlen(ConfigStr(LOGIN_OLD_VERSION_STR)),
                           ConfigStr(LOGIN_OLD_VERSION_STR));
         AddByteToPacket(LA_LOGOFF);
         SendPacket(s->session_id);
         break;
      }
      /* if game locked, only admins allowed on */
      if (s->account->type != ACCOUNT_ADMIN && s->account->type != ACCOUNT_DM)
      {
         if (IsGameLocked())
         {
            AddByteToPacket(AP_MESSAGE);
            AddStringToPacket(strlen(GetGameLockedReason()),GetGameLockedReason());
            AddByteToPacket(LA_LOGOFF);
            SendPacket(s->session_id);
            break;
         }
      }
/*
  if (s->account->credits <= 0)
  {
  AddByteToPacket(AP_NOCREDITS);
  SendPacket(s->session_id);
  break;
  }
*/
      last_download_time = *(int *)(msg->data + index);
      index += 4; 
      
      pkkcatch = *(int *)(msg->data + index);
      index += 4; 
      
      len = *(short *)(msg->data+index);
      if (index + 2 + len > msg->len)
         break;
      if (len >= sizeof(name))
         break;
      memcpy(name,msg->data+index+2,len);
      name[len] = 0; /* null terminate string */
      index += 2 + len;
      
      SynchedGotoGame(s,last_download_time);
      break;
   case AP_REQ_ADMIN :
      if (s->account->type == ACCOUNT_ADMIN)
      {
	 AddByteToPacket(AP_ADMIN);
	 SendPacket(s->session_id);
	 SetSessionState(s,STATE_ADMIN);
      }
      break;

   case AP_REGISTER :
      len = *(short *)(msg->data+index);
      if (index + 2 + len > msg->len) /* 2 = length word len */
	 break;
      lprintf("SynchedProtocolParse got a registration form\n");
      AppendTextFile(s,REGFORM_FILE,len,msg->data+index+2);
      break;

   case AP_RESYNC :
      SetSessionState(s,STATE_RESYNC);
      break;

   case AP_ADMINNOTE :
      len = *(short *)(msg->data+index);
      if (index + 2 + len > msg->len) /* 2 = length word len */
	 break;
      lprintf("SynchedProtocolParse got a note for admins\n");
      AppendTextFile(s,NOTE_FILE,len,msg->data+index+2);
      break;

   case AP_REQ_MENU :
      SynchedDoMenu(s);
      break;

   case AP_PING :
      /* they're there, so hangup timer reset, and we are done */
      break;
      
   default : 
      eprintf("SynchedProtocolParse got invalid packet %u\n",msg->data[0]);
      break;
   }
}

int num_with_same_ip;
struct in_addr check_addr;
void CheckIPAddress(session_node *s)
{
   if (s->conn.addr.s_addr == check_addr.s_addr)
      ++num_with_same_ip;
}

void SynchedAcceptLogin(session_node *s,char *name,char *password)
{
   session_node *other;
   account_node *a;
   INT64 now = GetTime();

   a = AccountLoginByName(name); /* maps the GUEST_ACCOUNT_NAME into a real account */

   /* bad username, bad password, or suspended? */
   if (a == NULL || a->password != password)
   {
      s->syn->failed_tries++;
      if (s->syn->failed_tries == ConfigInt(LOGIN_MAX_ATTEMPTS))
      {
         AddByteToPacket(AP_TOOMANYLOGINS);
         SendPacket(s->session_id);
         HangupSession(s);
         return;
      }
      
      AddByteToPacket(AP_LOGINFAILED);
      SendPacket(s->session_id);
      return;
   }

   // Too many connections from the same IP address?
   if (ConfigInt(LOGIN_MAX_PER_IP) != 0)
   {
      num_with_same_ip = 0;
      check_addr = s->conn.addr;

      ForEachSession(CheckIPAddress);

      // Include ourselves in the count
      if (num_with_same_ip > ConfigInt(LOGIN_MAX_PER_IP))
      {
         AddByteToPacket(AP_MESSAGE);
         AddStringToPacket(strlen(ConfigStr(LOGIN_TOO_MANY_PER_IP_STR)),
                           ConfigStr(LOGIN_TOO_MANY_PER_IP_STR));
         AddByteToPacket(LA_LOGOFF);
         SendPacket(s->session_id);
         return;
      }
   }
   
   /* suspension still in effect? */
   if (a->suspend_time > now)
   {
      AddByteToPacket(AP_MESSAGE);
      AddStringToPacket(strlen(ConfigStr(LOGIN_SUSPEND_STR)),
                        ConfigStr(LOGIN_SUSPEND_STR));
      AddByteToPacket(LA_LOGOFF);
      SendPacket(s->session_id);
      return;
   }
   
   /* suspension lifted naturally? */
   if (a->suspend_time)
      SuspendAccountAbsolute(a, 0);
   
   /* check if anyone already logged in on same account */
   other = GetSessionByAccount(a);
   if (other != NULL)
   {
      if (ConfigBool(INACTIVE_OVERRIDE))
      {
         // Since they've passed all security checks by this time,
         // we will just hang up the other session that is on this account.
         //
         lprintf("ACCOUNT %i (%s) in use; new connection overrides old one.\n",
                 a->account_id, a->name.c_str());
         HangupSession(other);
      }
      else
      {
         // Tell the user they're out of luck until we time the connection out.
         //
         AddByteToPacket(AP_ACCOUNTUSED);
         SendPacket(s->session_id);
      }
   }

   /* check if we're too busy, if not an admin. */
   if (a->type != ACCOUNT_ADMIN && a->type != ACCOUNT_DM && !s->active)
   {
      AddByteToPacket(AP_MESSAGE);
      AddStringToPacket(strlen(ConfigStr(SESSION_BUSY)),ConfigStr(SESSION_BUSY));
      AddByteToPacket(LA_LOGOFF);
      SendPacket(s->session_id);
      HangupSession(s);
      return;
   }

   s->account = a;
   s->account->last_login_time = now;

   InterfaceUpdateSession(s);

   s->account->last_login_time = now;

   /* start billing the account */
	VerifyLogin(s);
}

void SendSynchedMessage(session_node *s,char *str,char logoff)
{
   AddByteToPacket(AP_MESSAGE);
   AddStringToPacket(strlen(str),str);
   AddByteToPacket(logoff);
   SendPacket(s->session_id);
}

void VerifyLogin(session_node *s)
{
   char *str;
   LogUserData(s);

   s->login_verified = true;

   AddByteToPacket(AP_LOGINOK);
   AddByteToPacket((char)(s->account->type));
   SendPacket(s->session_id);

   /* they're logged in now.  Check their version number, and if old tell 'em
      to update it */

   if (s->version_major * 100 + s->version_minor < ConfigInt(LOGIN_INVALID_VERSION))
   {
      SendSynchedMessage(s,ConfigStr(LOGIN_INVALID_VERSION_STR),LA_LOGOFF);
      HangupSession(s);
      return;
   }


   if (s->version_major * 100 + s->version_minor < ConfigInt(LOGIN_MIN_VERSION))
   {
      AddByteToPacket(AP_GETCLIENT);

      str = LockConfigStr(UPDATE_CLIENT_MACHINE);
      AddStringToPacket(strlen(str),str);
      UnlockConfigStr();
      
      str = LockConfigStr(UPDATE_CLIENT_FILE);
      AddStringToPacket(strlen(str),str);
      UnlockConfigStr();

      SendPacket(s->session_id);
   }
   else
   {
      SynchedDoMenu(s);
   }
 
}

void LogUserData(session_node *s)
{
   std::string buf;

   buf += "Login account " + std::to_string(s->account->account_id) + " from " + s->conn.name + ": ";

   // Windows version (os_version_major/minor from RtlGetVersion)
   buf += "Win " + std::to_string(s->os_version_major) + "." + std::to_string(s->os_version_minor);
   if (s->os_build_number > 0)
      buf += " build " + std::to_string(s->os_build_number);
   buf += ", ";

   // RAM
   {
      int ramMB = s->machine_ram;
      if (ramMB >= 1024)
         buf += std::to_string(ramMB / 1024) + " GB RAM, ";
      else
         buf += std::to_string(ramMB) + " MB RAM, ";
   }

   // Screen
   buf += std::to_string(s->screen_x) + "x" + std::to_string(s->screen_y)
        + "@" + std::to_string(s->screen_color_depth) + "bpp, ";

   // GPU vendor + VRAM + renderer + session length
   {
      int renderer_mode     = s->renderer_mode;
      unsigned int vramMB   = (unsigned int)s->vram_mb;
      unsigned int sessBuck = (unsigned int)s->session_bucket;
      unsigned int vendorId = (unsigned int)s->gpu_vendor_id;

      const char *rendStr  = (renderer_mode == 2) ? "D3D+GpuEff"
                           : (renderer_mode == 1) ? "D3D" : "Software";
      
      std::string gpuInfo;
      if (!s->gpu_desc.empty()) {
         gpuInfo = s->gpu_desc;
      } else {
         const char *gpuVendor = (vendorId == 0x10DE) ? "NVIDIA"
                               : (vendorId == 0x1002) ? "AMD"
                               : (vendorId == 0x8086) ? "Intel"
                               : (vendorId == 0)      ? "N/A" : "GPU";
         std::stringstream ssVendor;
         if (vendorId != 0 && vendorId != 0x10DE && vendorId != 0x1002 && vendorId != 0x8086)
            ssVendor << " 0x" << std::hex << vendorId;
         gpuInfo = std::string(gpuVendor) + ssVendor.str();
      }
      buf += std::string(rendStr) + ", " + gpuInfo;
      if (vramMB >= 1024)
         buf += " " + std::to_string((vramMB + 512) / 1024) + "GB, ";
      else if (vramMB > 0)
         buf += " " + std::to_string(vramMB) + "MB, ";
      else
         buf += ", ";

      // Session bucket (seconds-based)
      static const char *sessBucketStr[] = {
         NULL, "<1min", "~2min", "~10min", "~20min", "~45min",
         "~1.5hr", "~3hr", "~5hr", "~7hr", "~10hr", "12+hr"
      };
      if (sessBuck > 0 && sessBuck < 12)
         buf += "session " + std::string(sessBucketStr[sessBuck]) + ", ";
      else if (sessBuck >= 12)
         buf += "session 12+hr, ";
   }

   // Last session FPS
   {
      unsigned int avgFps = (unsigned int)s->last_avg_fps;
      unsigned int lowFps = (unsigned int)s->last_low_fps;
      unsigned int maxFps = (unsigned int)s->last_max_fps;
      bool         wine   = s->is_wine != 0;

      if (avgFps > 0)
      {
         buf += "fps avg=" + std::to_string(avgFps)
              + " 1%low=" + std::to_string(lowFps);
         if (maxFps > 0)
            buf += " max=" + std::to_string(maxFps);
      }
      else
         buf += "fps=N/A";

      if (wine)
         buf += " [Wine]";
   }

   if (s->partner)
      buf += ", Partner " + std::to_string(s->partner);

   buf += "\n";
   lprintf("%s", buf.c_str());
}

void SynchedDoMenu(session_node *s)
{
   SynchedSendMenuChoice(s);
             
   AddByteToPacket(AP_CREDITS);
   /* round any fraction up */
   AddIntToPacket(20);
   /*
   if (s->account->credits%100 == 0)
      AddIntToPacket(s->account->credits/100);
   else
      AddIntToPacket(s->account->credits/100+1);
      */
   SendPacket(s->session_id);
}

void SynchedSendMenuChoice(session_node *s)
{
   int i;

   /* this message tells the client that it's at the main menu, and initializes
      the pseudo-random # sequence thing for game messages */

   AddByteToPacket(AP_GETCHOICE);
   s->seeds[0] = (int)GetTime()*2;
   s->seeds[1] = (int)GetMilliCount();
   s->seeds[2] = rand();
   s->seeds[3] = rand()*rand();
   s->seeds[4] = rand()+(int)GetMilliCount();

   for (i=0;i<SEED_COUNT;i++)
      AddIntToPacket(s->seeds[i]);
   SendPacket(s->session_id);

}

void SynchedGotoGame(session_node *s,int last_download_time)
{
   int num_new_files;
   char *str;
   
   /* first check to see if they can goto game (if they have >= 1 char) */

   if (CountUserByAccountID(s->account->account_id) <= 0)
   {
      /* Tell user that he has no characters */
      AddByteToPacket(AP_NOCHARACTERS);
      SendPacket(s->session_id);
      return;
   }

   /* now check new files to download/delete */

   /* dprintf("s %i time %s\n",s->session_id,TimeStr(last_download_time)); */

   s->last_download_time = last_download_time;

   /* dprintf("sess %i has %i new files to delete\n",s->session_id,CountNewDelresFile(s)); */

   num_new_files = CountNewDLFile(s);
   if (num_new_files > 0)
   {
      // Tell client there's files to be downloaded, and don't go into game mode.

      AddByteToPacket(AP_DOWNLOAD);

      AddShortToPacket((short)num_new_files);
      
      str = LockConfigStr(UPDATE_PACKAGE_MACHINE);
      AddStringToPacket(strlen(str),str);
      UnlockConfigStr();
      
      str = LockConfigStr(UPDATE_PACKAGE_PATH);
      AddStringToPacket(strlen(str),str);
      UnlockConfigStr();

      str = LockConfigStr(UPDATE_DOWNLOAD_REASON);
      AddStringToPacket(strlen(str),str);
      UnlockConfigStr();

      str = LockConfigStr(UPDATE_DEMO_BUTTON);
      AddStringToPacket(strlen(str),str);
      UnlockConfigStr();

      ForEachNewDLFile(s,SynchedSendDLFileInfo);
      
      SendPacket(s->session_id);

      s->syn->download_count = num_new_files;
      InterfaceUpdateSession(s);

      /* set timeout real long, since they're downloading */
      SetSessionTimer(s,60*ConfigInt(INACTIVE_TRANSFER));

      return;      
   }

   if (ConfigBool(SERVICE_ENABLED))
   {
      // Tell client how they could Put an FTP file for a service report.

      AddByteToPacket(AP_SERVICEREPORT);

      str = LockConfigStr(SERVICE_MACHINE);
      AddStringToPacket(strlen(str),str);
      UnlockConfigStr();
      
      str = LockConfigStr(SERVICE_DIRECTORY);
      AddStringToPacket(strlen(str),str);
      UnlockConfigStr();

      str = LockConfigStr(SERVICE_USERNAME);
      AddStringToPacket(strlen(str),str);
      UnlockConfigStr();

      str = LockConfigStr(SERVICE_PASSWORD);
      AddStringToPacket(strlen(str),str);
      UnlockConfigStr();

      SendPacket(s->session_id);
   }

   // All set to go to game mode.

   AddByteToPacket(AP_GAME);
   SendPacket(s->session_id);
   
   SetSessionState(s,STATE_GAME);
}

void SynchedSendDLFileInfo(char *str,int time,int type,int size)
{
   AddIntToPacket(time);
   AddIntToPacket(type);
   AddIntToPacket(size);
   AddStringToPacket(strlen(str),str);
}

void SynchedAddDelFile(char *str)
{
   AddStringToPacket(strlen(str),str);
}