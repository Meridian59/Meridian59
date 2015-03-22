// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * game.c
 *

 This module supports one mode of a session, STATE_GAME.  This is for
 when the session is selecting their character or in the game.  Since
 the protocol in this mode has a message header and text, if things
 get garbled resynchronization must occur.  This is done with the game
 state's own state, game_state.  Also, credits are drained in this
 mode.

 */

#include "blakserv.h"

/* 
 * Credit draining in game mode (after character is selected) is
 * ConfigInt(CREDIT_DRAIN_AMOUNT) 100ths of a credit every
 * ConfigInt(CREDIT_DRAIN_TIME) seconds.  This is equivalent to
 * 36*Amount/Time credits per hour.
 * 
 */

/* local function prototypes */

unsigned int GameRandomStreamsStep(session_node *s);
void GameSendResync(session_node *s);
void GameSyncInit(session_node *s);
void GameSyncProcessSessionBuffer(session_node *s);
void GameSyncInputChar(session_node *s,char ch);
void GameWarnLowCredits(session_node *s);
void GameProtocolParse(session_node *s,client_msg *msg);
void GameStartUser(session_node *s,user_node *u);
void GameTryGetUser(session_node *s);
void GameSendEachUserChoice(user_node *u);
void GameSendSystemEnter(session_node *s);
void GameDMCommand(session_node *s,int type,char *str);

static char* _redbookstring = NULL;
static int _redbookid = 0;

/////////////////////////////////////////////////////////////////////////////

void GameInit(session_node *s)
{
   s->game = (game_data *)s->session_state_data;

   s->game->object_id = INVALID_OBJECT;

   SetSessionTimer(s,60*ConfigInt(INACTIVE_SELECTCHAR));

   s->game->game_state = GAME_NORMAL;

   s->game->game_last_message_time = GetTime();

   GameSendSystemEnter(s);
}

void GameClientExit(session_node *s)
{
   AddByteToPacket(BP_QUIT);
   SendPacket(s->session_id);
}

void GameExit(session_node *s)
{
   s->exiting_state = True;	/* in case the write fails, don't let it call us again */

   if (s->game->object_id == INVALID_OBJECT)
      GameClientExit(s);
   else
      ClientHangupToBlakod(s);

   s->exiting_state = False;
}

void GameProcessSessionTimer(session_node *s)
{
   if (s->game->object_id == INVALID_OBJECT)
   {
      /* timed out before credits drain means took too long picking character */
      HangupSession(s);
      return;
   }

   /* they're in the game.  First thing to check is if we haven't gotten any
      message in a while, even ping.  If so, they are so lagged they should
      be hung up. */

   /* dprintf("%u %u %u\n",GetTime(),s->game->game_last_message_time,ConfigInt(INACTIVE_GAME)); */

   if (GetTime() - s->game->game_last_message_time > ConfigInt(INACTIVE_GAME))
   {
      lprintf("GameProcessSessionTimer logging out ACCOUNT %i (%s) which hasn't been heard from.\n",
	 s->account->account_id, s->account->name);
      HangupSession(s);
      return;
   }

   SetSessionTimer(s,ConfigInt(CREDIT_DRAIN_TIME));

   s->account->credits -= ConfigInt(CREDIT_DRAIN_AMOUNT);

   if (s->game->game_state != GAME_NORMAL)
      return;

#if 0   
   /* warn them when they are low */
   if ((s->account->credits <= 100*ConfigInt(CREDIT_WARN1)) &&
       (s->account->credits + ConfigInt(CREDIT_DRAIN_AMOUNT) > 100*ConfigInt(CREDIT_WARN1)) ||
       (s->account->credits == 100*ConfigInt(CREDIT_WARN2)) &&
       (s->account->credits + ConfigInt(CREDIT_DRAIN_AMOUNT) > 100*ConfigInt(CREDIT_WARN2)))
      GameWarnLowCredits(s);

   if (s->account->credits == 0)
   {
      /* send them an out-of-credits message */
      GameClientExit(s);
      SetSessionState(s,STATE_SYNCHED);
   }
#endif
}

void GameProcessSessionBuffer(session_node *s)
{
   client_msg msg;
   unsigned short security;

   s->game->game_last_message_time = GetTime();

   if (s->game->game_state != GAME_NORMAL)
   {
      GameSyncProcessSessionBuffer(s);
      return;
   }

   /* need to copy only as many bytes as we can hold */
   while (s->receive_list != NULL)
   {
      if (PeekSessionBytes(s,HEADERBYTES,&msg) == False)
	 return;

      if (msg.len != msg.len_verify)
      {
	 /* dprintf("GPSB found len != len_verify %i %i\n",msg_len,msg_len_verify); */
	 GameSendResync(s);
	 GameSyncInit(s);
	 GameSyncProcessSessionBuffer(s);
	 return;
      }

      if (msg.len > LEN_MAX_CLIENT_MSG)
      {
         eprintf("GameProcessSessionBuffer got message too long %i\n",msg.len);
	 GameSendResync(s);
	 GameSyncInit(s);
	 GameSyncProcessSessionBuffer(s);
	 return;
      }
      
      /* now read the header for real, plus the actual data */
      if (ReadSessionBytes(s,msg.len+HEADERBYTES,&msg) == False)
	 return;

      /* dprintf("got crc %08x\n",msg.crc16); */

      security = GameRandomStreamsStep(s);

      /* dprintf("%08x is the next stream thing\n",security); */

      security ^= msg.len;
      security ^= ((unsigned int)msg.data[0] << 4);
      security ^= GetCRC16(msg.data,msg.len);

      /* dprintf("%08x is the next stream thing after xor\n",security); */

      if (msg.crc16 != security && !s->seeds_hacked)
      {
			s->seeds_hacked = True;
			if (ConfigBool(SECURITY_LOG_SPOOFS))
			{
				lprintf("GameProcessSessionBuffer found invalid security account %i\n",
						  s->account->account_id);
			}
			if (ConfigBool(SECURITY_HANGUP_SPOOFS))
			{
				HangupSession(s);
			}

			// can't use the packet, so throw it away and go into resync mode
			GameSendResync(s);
			GameSyncInit(s);
			GameSyncProcessSessionBuffer(s);
			return;
      }

      if (msg.seqno != GetEpoch()) /* old sequence ok, just ignore */
      {
	 /* dprintf("Game got bad epoch from session %i\n",s->session_id); */
	 continue;
      }
      
      /* give up receive mutex, so the interface/socket thread can
	 read data for us, even if doing something long (GC/save/reload sys) */
      
      if (!ReleaseMutex(s->muxReceive))
	 eprintf("GPSB released mutex it didn't own in session %i\n",s->session_id);
	 
      GameProtocolParse(s,&msg);
      
      if (WaitForSingleObject(s->muxReceive,10000) != WAIT_OBJECT_0)
      {
	 eprintf("GPSB bailed waiting for mutex on session %i\n",s->session_id);
	 return;
      }
      
      /* if hung up, don't touch */
      if (s->hangup == True)
	 return;

      if (s->state != STATE_GAME)
	 return;
      
      if (s->game->game_state != GAME_NORMAL)
	 return;
      
   }
}

unsigned int GameRandomStreamsStep(session_node *s)
{
   int i, stream;

   for (i=0; i < SEED_COUNT; i++)
   {
      /* Step this stream */
      s->seeds[i] = (s->seeds[i] * 9301 + 49297) % 233280;
   }
   
   stream = s->seeds[SEED_COUNT - 1] % (SEED_COUNT - 1);
   return s->seeds[stream];
}

void GameSendResync(session_node *s)
{
   int i;

   for (i=0;i<10;i++)
   {
      /* dprintf("got error reading session %i\n",s->session_id); */
      AddByteToPacket(BP_RESYNC);
      SendPacket(s->session_id);
   }
}

void GameSyncInit(session_node *s)
{
/*    dprintf("GameSyncInit\n"); */
   s->game->game_state = GAME_BEACON;
   s->game->game_sync_index = 0;

   InterfaceUpdateSession(s);
}

void GameSyncProcessSessionBuffer(session_node *s)
{
   char ch;

   while (s->receive_list != NULL)
   {
      if (ReadSessionBytes(s,1,&ch) == False)
	 return;
      
      GameSyncInputChar(s,ch);

      /* any character could change our state back to game normal.  if so, leave */
      if (s->game->game_state == GAME_NORMAL)
      {
	 if (s->game->object_id == INVALID_OBJECT)
	    GameSendSystemEnter(s);
	 GameProcessSessionBuffer(s);
	 break;
      }
   }
}

void GameSyncInputChar(session_node *s,char ch)
{
   int sync_len;
   unsigned char *sync_buf;

   switch (s->game->game_state)
   {
   case GAME_BEACON :
      sync_buf = beacon_str;
      sync_len = LENGTH_BEACON;
      break;
   case GAME_FINAL_SYNC :
      sync_buf = detect_str;
      sync_len = LENGTH_DETECT;
      break;
   default :
      eprintf("GameSyncInputChar can't get here\n");
   }

   if (ch == sync_buf[s->game->game_sync_index])
      s->game->game_sync_index++;
   else
   {
      if (s->game->game_state == GAME_FINAL_SYNC)
      {
	 /* bad char here means line noise */
/* 	 dprintf("bad char on final sync\n"); */
	 GameSendResync(s);
	 GameSyncInit(s);
	 return;
      }
      s->game->game_sync_index = 0;
   }

/*   dprintf("GameSync got char %i, pos %i for sync\n",ch,s->game->game_sync_index); */

   if (s->game->game_sync_index == sync_len)
   {
      switch (s->game->game_state)
      {
      case GAME_BEACON :
         SendBytes(s,(char *) tell_cli_str,LENGTH_TELL_CLI); /* sends exact bytes, no header */
	 s->game->game_state = GAME_FINAL_SYNC;
	 s->game->game_sync_index = 0;
	 InterfaceUpdateSession(s);
	 break;
      case GAME_FINAL_SYNC :
	 s->game->game_state = GAME_NORMAL;
	 InterfaceUpdateSession(s);
	 break;
      }
/*      dprintf("Changed to game state %i\n",s->game->game_state); */
   }
   
}

void GameWarnLowCredits(session_node *s)
{
   val_type str_val;
   parm_node blak_parm[1];
   char text[100];

   sprintf(text,"You have only %i credit%s remaining",s->account->credits/100,
	   (s->account->credits/100 == 1) ? "" : "s");

   SetTempString(text,strlen(text));
   str_val.v.tag = TAG_TEMP_STRING;
   str_val.v.data = 0;		/* the data field doesn't matter for TAG_TEMP_STRING */

   blak_parm[0].type = CONSTANT;
   blak_parm[0].value = str_val.int_val;
   blak_parm[0].name_id = STRING_PARM;   

   SendTopLevelBlakodMessage(s->game->object_id,SYSTEM_STRING_MSG,1,blak_parm);
}

void GameEchoPing(session_node *s)
{
   unsigned int token;

   if (!s || s->state != STATE_GAME ||
       !s->game || s->game->game_state != GAME_NORMAL ||
       s->conn.type == CONN_CONSOLE)
   {
      if (s)
      {
	 s->secure_token = 0;
	 s->sliding_token = NULL;
      }
      return;
   }

   // BP_PING is sign that client is still alive.
   // We return a BP_ECHO_PING in response.
   // We also take this opportunity to choose a new security token,
   // and tack it onto the BP_ECHO_PING.
   //
   // NOTE: This may be called anytime; the client can handle
   // unrequested BP_ECHO_PINGs to synchronize the security token.

   token = (s->seeds[0] & 0xFF);

   AddByteToPacket(BP_ECHO_PING);
   AddByteToPacket((unsigned char)(token ^ 0xED));
   AddIntToPacket(GetSecurityRedbookID());
   SendPacket(s->session_id);

   s->secure_token = token;
   s->sliding_token = GetSecurityRedbook();
}

void GameProtocolParse(session_node *s,client_msg *msg)
{
   user_node *u;
   int object_id;
   char *ptr;

   char password[MAX_LOGIN_PASSWORD+1],new_password[MAX_LOGIN_PASSWORD+1];
   int len,index;

   char admin_cmd[500];
   int admin_len;
   int dm_type;

   GameMessageCount((unsigned char)msg->data[0]);

   switch ((unsigned char)msg->data[0])
   {
   case BP_REQ_QUIT :
      GameClientExit(s);
      SetSessionState(s,STATE_SYNCHED);
      break;
      
   case BP_RESYNC :
      // dprintf("client said it had an error\n");
      GameSyncInit(s);
      GameSyncProcessSessionBuffer(s);
      break;

   case BP_PING :
      GameEchoPing(s);
      break;

   case BP_AD_SELECTED :
      /* they clicked on an ad; log it */
      switch (msg->data[1])
      {
      case 1:
	 ptr = LockConfigStr(ADVERTISE_URL1);
	 lprintf("GameProtocolParse found account %i visited ad 1, %s\n",s->account->account_id,
		 ptr);
	 UnlockConfigStr();
	 break;
      case 2:
	 ptr = LockConfigStr(ADVERTISE_URL2);
	 lprintf("GameProtocolParse found account %i visited ad 2, %s\n",s->account->account_id,
		 ptr);
	 UnlockConfigStr();
	 break;
      default :
	 eprintf("GameProtocolParse found account %i visited unknown ad %i\n",
		 s->account->account_id,msg->data[1]);
      }
      break;
         
   case BP_USE_CHARACTER :
      if (s->game->object_id == INVALID_OBJECT)
      {
	 object_id = *((int *)(msg->data+1));
	 u = GetUserByObjectID(object_id);
	 if (u != NULL && u->account_id == s->account->account_id)
	    GameStartUser(s,u);
	 else
	    eprintf("GameProtocolParse can't find user for obj %i in use char!!!\n",
		    object_id);
	 InterfaceUpdateSession(s);
      }
      break;

   case BP_REQ_ADMIN :
      if (s->account->type != ACCOUNT_ADMIN)
      {
	 eprintf("GameProtocolParse got admin command from non admin account %i\n",
		 s->account->account_id);
	 break;
      }
      admin_len = (int) * ((short *)(msg->data + 1));
      if (admin_len > msg->len - 3)
	 return;
      if (admin_len > sizeof(admin_cmd)-2)
	 return;
      memcpy(admin_cmd,&msg->data[3],admin_len);
      admin_cmd[admin_len] = 0;
      SendSessionAdminText(s->session_id,"> %s\n",admin_cmd); /* echo it to 'em */
      TryAdminCommand(s->session_id,admin_cmd);
      break;

   case BP_REQ_DM :
      if (s->account->type != ACCOUNT_ADMIN && s->account->type != ACCOUNT_DM)
      {
	 eprintf("GameProtocolParse got DM command from non admin or DM account %i\n",
		 s->account->account_id);
	 break;
      }
      dm_type = msg->data[1];

      admin_len = (int) * ((short *)(msg->data + 2));
      if (admin_len > msg->len - 4)
	 return;
      if (admin_len > sizeof(admin_cmd)-2)
	 return;
      memcpy(admin_cmd,&msg->data[4],admin_len);
      admin_cmd[admin_len] = 0;
      GameDMCommand(s,dm_type,admin_cmd);
      break;

   case BP_SEND_CHARACTERS :
      GameTryGetUser(s);
      break;

   case BP_CHANGE_PASSWORD :
      index = 1;
      len = *(short *)(msg->data+index);
      if (index + 2 + len > msg->len) /* 2 = length word len */
	 break;
      if (len-1 > sizeof(password))
	 break;
      memcpy(password,msg->data+index+2,len);
      password[len] = 0; /* null terminate string */
      index += 2 + len;
      
      len = *(short *)(msg->data+index);
      if (index + 2 + len > msg->len)
	 break;
      if (len-1 > sizeof(new_password))
	 break;
      memcpy(new_password,msg->data+index+2,len);
      new_password[len] = 0; /* null terminate string */
      index += 2 + len;

      if (strcmp(s->account->password,password))
      {
	 AddByteToPacket(BP_PASSWORD_NOT_OK);
	 SendPacket(s->session_id);
	 break;
      }

      SetAccountPasswordAlreadyEncrypted(s->account,new_password);
      AddByteToPacket(BP_PASSWORD_OK);
      SendPacket(s->session_id);

      break;

   default :
      ClientToBlakodUser(s,msg->len,msg->data);
      
      break;
   }
}

void GameStartUser(session_node *s,user_node *u)
{
   parm_node p;
   val_type session_id_const;

   s->game->object_id = u->object_id;

   session_id_const.v.tag = TAG_SESSION;
   session_id_const.v.data = s->session_id;

   p.type = CONSTANT;
   p.value = session_id_const.int_val;
   p.name_id = SESSION_ID_PARM;
   SendTopLevelBlakodMessage(s->game->object_id,USER_ENTER_GAME_MSG,1,&p);

   SetSessionTimer(s,ConfigInt(CREDIT_DRAIN_TIME));
}

void GameTryGetUser(session_node *s)
{
   char *ptr;

   if (0 && s->account->type == ACCOUNT_GUEST) /* let guests choose too */
   {
      GameStartUser(s,GetFirstUserByAccountID(s->account->account_id));
   }
   else
   {
      AddByteToPacket(BP_CHARACTERS);
      AddShortToPacket((short)CountUserByAccountID(s->account->account_id));
      ForEachUserByAccountID(GameSendEachUserChoice,s->account->account_id);
      AddStringToPacket(GetMotdLength(),GetMotd());

      /* advertising stuff */
      AddByteToPacket(2);

      ptr = LockConfigStr(ADVERTISE_FILE1);
      AddStringToPacket(strlen(ptr),ptr);
      UnlockConfigStr();
      ptr = LockConfigStr(ADVERTISE_URL1);
      AddStringToPacket(strlen(ptr),ptr);
      UnlockConfigStr();
      ptr = LockConfigStr(ADVERTISE_FILE2);
      AddStringToPacket(strlen(ptr),ptr);
      UnlockConfigStr();
      ptr = LockConfigStr(ADVERTISE_URL2);
      AddStringToPacket(strlen(ptr),ptr);
      UnlockConfigStr();
      
      SendPacket(s->session_id);   
   }
}

void GameSendEachUserChoice(user_node *u)
{
   val_type name_val,num_val;
   resource_node *r;
   
   AddIntToPacket(u->object_id);

   name_val.int_val = SendTopLevelBlakodMessage(u->object_id,USER_NAME_MSG,0,NULL);
   if (name_val.v.tag != TAG_RESOURCE)
   {
      eprintf("GameSendEachUserChoice object %i has non-resource name %i,%i\n",
	      u->object_id,name_val.v.tag,name_val.v.data);
      AddStringToPacket(0,"");
   }
   else
   {
      r = GetResourceByID(name_val.v.data);
      if (r == NULL)
      {
	 bprintf("GameSendEachUserChoice can't find resource %i as a resource/string\n",
		 name_val.v.data);
	    return;
      }
      AddStringToPacket(strlen(r->resource_val),r->resource_val);
   }

   num_val.int_val = SendTopLevelBlakodMessage(u->object_id,IS_FIRST_TIME_MSG,0,NULL);
   if (num_val.v.data != 0)
      AddByteToPacket(1);   /* char has been in game before */
   else
      AddByteToPacket(0);   /* char has NOT been in game before */
      
}

void GameSendSystemEnter(session_node *s)
{
   parm_node p;
   val_type session_id_const;

   session_id_const.v.tag = TAG_SESSION;
   session_id_const.v.data = s->session_id;
   
   p.type = CONSTANT;
   p.value = session_id_const.int_val;
   p.name_id = SESSION_ID_PARM;
   
   SendTopLevelBlakodMessage(GetSystemObjectID(),SYSTEM_ENTER_GAME_MSG,1,&p);

}

void GameDMCommand(session_node *s,int type,char *str)
{
   char buf[LEN_MAX_CLIENT_MSG + 200];
   int acctype;

   acctype = ACCOUNT_NORMAL;
   if (s && s->account)
      acctype = s->account->type;

//   dprintf("DM command %i by session %i account %i acctype %i; string is \"%s\".\n",
//      type, s->session_id, s->account->account_id, acctype, str);

   switch (type)
   {
   case DM_CMD_GO_ROOM :
      if (0 != atoi(str))
      {
	 //ASSERT(0 != ACCOUNT_ADMIN);
	 //ASSERT(0 != ACCOUNT_DM);
	 if (ConfigInt(RIGHTS_GOROOMBYNUM) == 0)
	 {
	    dprintf("DM Command GOROOM (by number) disabled.");
	    break;
	 }
	 if (ConfigInt(RIGHTS_GOROOMBYNUM) == ACCOUNT_ADMIN && acctype == ACCOUNT_DM)
	 {
	    dprintf("DM Command GOROOM (by number) disabled for DMs; must be Admin.");
	    break;
	 }
      }
      if (ConfigInt(RIGHTS_GOROOM) == 0)
      {
	 dprintf("DM Command GOROOM disabled.");
	 break;
      }
      if (ConfigInt(RIGHTS_GOROOM) == ACCOUNT_ADMIN && acctype == ACCOUNT_DM)
      {
	 dprintf("DM Command GOROOM disabled for DMs; must be Admin.");
	 break;
      }
      sprintf(buf,"send object %i teleportto rid int %s",s->game->object_id,str);
      SendSessionAdminText(s->session_id,"~B> %s\n",buf); /* echo it to 'em */
      TryAdminCommand(s->session_id,buf); 
      break;

   case DM_CMD_GO_PLAYER :
      if (ConfigInt(RIGHTS_GOPLAYER) == 0)
	 break;
      if (ConfigInt(RIGHTS_GOPLAYER) == ACCOUNT_ADMIN && acctype == ACCOUNT_DM)
	 break;
      sprintf(buf,"send object %i admingotoobject what object %s",s->game->object_id,str);
      SendSessionAdminText(s->session_id,"~B> %s\n",buf); /* echo it to 'em */
      TryAdminCommand(s->session_id,buf); 
      break;

   case DM_CMD_GET_PLAYER :
      if (ConfigInt(RIGHTS_GETPLAYER) == 0)
	 break;
      if (ConfigInt(RIGHTS_GETPLAYER) == ACCOUNT_ADMIN && acctype == ACCOUNT_DM)
	 break;
      sprintf(buf,"send object %s admingotoobject what object %i",str,s->game->object_id);
      SendSessionAdminText(s->session_id,"~B> %s\n",buf); /* echo it to 'em */
      TryAdminCommand(s->session_id,buf); 
      break;

   default :
     eprintf("GameDMCommand got invalid DM command type %i\n",type);
     break;
      
   }
}

void UpdateSecurityRedbook()
{
   // Okay, timing is important here:
   // The _redbookstring must be updated to any new value
   // before we call GameEchoPing() on any sessions, but the
   // current strings held by any of the sessions now must
   // remain valid so the GameEchoPing() output doesn't access
   // a freed old string.
   //
   // We remember the old string for comparisons and for freeing last.
   // We get the new resource.
   // We update _redbookstring and _redbookid.
   // We see if users need an update at all.
   // We free the string which sessions were using AFTER the GameEchoPing.
   //
   char* pRscName;
   char* old;
   resource_node* r = NULL;

   old = _redbookstring;
   _redbookstring = NULL;
   _redbookid = 0;

   pRscName = LockConfigStr(SECURITY_REDBOOK_RSC);
   if (pRscName)
      r = GetResourceByName(pRscName);
   UnlockConfigStr();

   if (r)
   {
      if (!old || (r->resource_val && 0 != strcmp(old,r->resource_val)))
	 _redbookstring = strdup(r->resource_val);
      else
	 _redbookstring = old;
      _redbookid = r->resource_id;
   }

   if (old != _redbookstring)
   {
      ForEachSession(GameEchoPing);
      if (old)
	 free(old);
   }
}

const char* GetSecurityRedbook()
{
   if (!_redbookstring)
      return "BLAKSTON: Greenwich Q Zjiria";

   return _redbookstring;
}

int GetSecurityRedbookID()
{
   return _redbookid;
}
