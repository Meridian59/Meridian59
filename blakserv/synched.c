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
   s->blak_client = True;
   
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
      if (PeekSessionBytes(s,HEADERBYTES,&msg) == False)
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
      if (ReadSessionBytes(s,msg.len+HEADERBYTES,&msg) == False)
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
      if (s->hangup == True)
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
      if (msg->len < 39) /* fixed size of AP_LOGIN, before strings */
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
      s->machine_cpu = *(int *)(msg->data+index);
      index += 4;

      s->screen_x = *(short *)(msg->data+index);
      index += 2;
      s->screen_y = *(short *)(msg->data+index);
      index += 2;

      s->displays_possible = *(int *)(msg->data+index);
      index += 4;
      s->bandwidth = *(int *)(msg->data+index);
      index += 4;
      s->reserved = *(int *)(msg->data+index);
      index += 4;
      s->screen_color_depth = (short)(s->reserved & 0xFF);
      s->partner = (short)((s->reserved & 0xFF00) >> 8);

      // The following line was commented out because I added support for the 3 4-byte integers
      // index += 12; /* 12 bytes future expansion space */

      len = *(short *)(msg->data+index);
      if (index + 2 + len > msg->len) /* 2 = length word len */
	 break;
      if (len > sizeof(name))
	 break;
      memcpy(name,msg->data+index+2,len);
      name[len] = 0; /* null terminate string */
      index += 2 + len;
      
      len = *(short *)(msg->data+index);
      if (index + 2 + len > msg->len)
	 break;
      if (len > sizeof(name))
	 break;
      memcpy(password,msg->data+index+2,len);
      password[len] = 0; /* null terminate string */
      index += 2 + len;
      
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
      if (len > sizeof(name))
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
      eprintf("SynchedProtocolParse AP_REGISTER no longer supported\n");
      break;

   case AP_RESYNC :
      SetSessionState(s,STATE_RESYNC);
      break;

   case AP_ADMINNOTE :
      eprintf("SynchedProtocolParse AP_ADMINNOTE no longer supported\n");
      break;

   case AP_REQ_MENU :
      SynchedDoMenu(s);
      break;

   case AP_PING :
      /* they're there, so hangup timer reset, and we are done */
      break;
      
   default :
      int client_version = s->version_minor + 100 * s->version_major;
      account_node *a = s->account;
      int account_id;
      if (a)
         account_id = a->account_id;
      else
         account_id = INVALID_ID;

      eprintf("SynchedProtocolParse got invalid packet %u from account %i, cli vers %i\n",
         msg->data[0], account_id, client_version);
      break;
   }
}

int num_with_same_ip;
struct in6_addr check_addr;
void CheckIPAddress(session_node *s)
{
	BOOL equal = 1;
	for (int i = 0; i < sizeof(check_addr.u.Byte); i++)
	{
		if (s->conn.addr.u.Byte[i] != check_addr.u.Byte[i])
		{
			equal = 0;
			break;
		}
	}
	
	if (equal)
		++num_with_same_ip;
}

void SynchedAcceptLogin(session_node *s,char *name,char *password)
{
   session_node *other;
   account_node *a;
   int now = GetTime();

   a = AccountLoginByName(name); /* maps the GUEST_ACCOUNT_NAME into a real account */

   /* bad username, bad password, or suspended? */
   if (a == NULL ||
       (a->type != ACCOUNT_GUEST && strcmp(a->password,password) != 0))
   {
      s->syn->failed_tries++;
      if (s->syn->failed_tries == ConfigInt(LOGIN_MAX_ATTEMPTS))
      {
         AddByteToPacket(AP_TOOMANYLOGINS);
         SendPacket(s->session_id);
         HangupSession(s);
         return;
      }
      if (!stricmp(name,ConfigStr(GUEST_ACCOUNT)))
      {
         AddByteToPacket(AP_GUEST);
         AddByteToPacket(1); /* we're hanging 'em up */
         AddIntToPacket(ConfigInt(GUEST_SERVER_MIN));
         AddIntToPacket(ConfigInt(GUEST_SERVER_MAX));
         
         /*
           char *too_many_str;
           
           too_many_str = ConfigStr(GUEST_TOO_MANY);
           AddByteToPacket(AP_MESSAGE);
           AddStringToPacket(strlen(too_many_str),too_many_str);
           AddByteToPacket(LA_LOGOFF);
         */
         
         SendPacket(s->session_id);
         HangupSession(s);
      }
      else
      {
         AddByteToPacket(AP_LOGINFAILED);
         SendPacket(s->session_id);
      }
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
   
   /* tell guest client what other servers may be available */
   if (!stricmp(name,ConfigStr(GUEST_ACCOUNT)))
   {
      AddByteToPacket(AP_GUEST);
      AddByteToPacket(0); /* we're letting 'em stay, give 'em the new range */
      AddIntToPacket(ConfigInt(GUEST_SERVER_MIN));
      AddIntToPacket(ConfigInt(GUEST_SERVER_MAX));
      SendPacket(s->session_id);
   }
   
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
                 a->account_id, a->name);
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

   s->login_verified = True;

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
   char buf[500];

   sprintf(buf,"LogUserData/4 got %i from %s, ",s->account->account_id,s->conn.name);

   switch (s->os_type)
   {
   case VER_PLATFORM_WIN32_WINDOWS :
		if ((s->os_version_major > 4) ||  ((s->os_version_major == 4) && (s->os_version_minor > 0)))
			strcat(buf,"Windows 98");
		else
			strcat(buf,"Windows 95");
      break;
   case VER_PLATFORM_WIN32_NT :
      strcat(buf,"Windows NT");
      break;
   default :
      sprintf(buf+strlen(buf),"%i",s->os_type);
      break;
   }
   
   sprintf(buf+strlen(buf),", %i, %i, ",s->os_version_major,s->os_version_minor);
   


   switch (s->machine_cpu&0xFFFF)	/* charlie: the cpu level is in the top 16 bits */
   {
   case PROCESSOR_INTEL_386 :
      strcat(buf,"386");
      break;
   case PROCESSOR_INTEL_486 :
      strcat(buf,"486");
      break;
   case PROCESSOR_INTEL_PENTIUM :
      strcat(buf,"Pentium");
      break;
   default :
      sprintf(buf+strlen(buf),"%i",s->machine_cpu&0xFFFF);
      break;
   }
   
   strcat(buf,", ");

   sprintf(buf+strlen(buf),"%i MB",s->machine_ram/(1024*1024));
   strcat(buf,", ");
   
   sprintf(buf+strlen(buf),"%ix%ix%i (0x%08X)",s->screen_x,s->screen_y,s->screen_color_depth,((s->machine_cpu&0xFFFF0000)|s->displays_possible));

   if (s->partner)
      sprintf(buf+strlen(buf),", Partner %d",s->partner);

   strcat(buf,", ");
   sprintf(buf+strlen(buf),"%s",LockConfigStr(ADVERTISE_FILE1));
   UnlockConfigStr();

   strcat(buf,", ");
   sprintf(buf+strlen(buf),"%s",LockConfigStr(ADVERTISE_FILE2));
   UnlockConfigStr();

   strcat(buf,"\n");

   lprintf("%s",buf);
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
   s->seeds[0] = GetTime()*2;
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
