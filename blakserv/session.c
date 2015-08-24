// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* session.c
*

  This module handles an array of sessions, which are connections with
  theh outside world.  It checks for new input on the connections,
  reads bytes, and handles changing states.  It also has functions to
  write to the session, either straight up (like admin mode or login
  mode) or with our protocol (for synched and game modes).
  
*/

#include "blakserv.h"
#include <assert.h>

unsigned char epoch; /* number to send to game clients as 7th byte in message */

session_node *sessions;
int num_sessions;

int transmitted_bytes; /* keep a tab on bandwidth use */

CRITICAL_SECTION csSessions; /* need to add/remove or search through list of sessions */

/* local function prototypes */
session_node *AllocateSession(void);

void CloseSession(int session_id);
void CloseConnection(connection_node conn);

void ProcessSessionTimer(session_node *s);
void ProcessSessionBuffer(session_node *s);

void SendGameClient(session_node *s,char *data,unsigned short len_data,int seqno);

void SendGameClientBufferList(session_node *s,buffer_node *blist,char seqno);

void SendBufferList(session_node *s,buffer_node *blist);
void SessionAddBufferList(session_node *s,buffer_node *blist);


/* InitSession
*
* This allocates an array of session nodes for max people that can be connected,
* even if they're overflow and we're gonna hang them up.  This array is never 
* freed.
*/
void InitSession()
{
	epoch = 1;
	transmitted_bytes = 0;
	
	sessions = (session_node *)
		AllocateMemory(MALLOC_ID_SESSION_MODES,ConfigInt(SESSION_MAX_CONNECT)*sizeof(session_node));
	
	num_sessions = 0;
	
	if (sizeof(admin_data) > SESSION_STATE_BYTES)
		FatalError("sizeof(admin_data) must be <= SESSION_STATE_BYTES");
	
	if (sizeof(game_data) > SESSION_STATE_BYTES)
		FatalError("sizeof(game_data) must be <= SESSION_STATE_BYTES");
	
	if (sizeof(trysync_data) > SESSION_STATE_BYTES)
		FatalError("sizeof(trysync_data) must be <= SESSION_STATE_BYTES");
	
	if (sizeof(synched_data) > SESSION_STATE_BYTES)
		FatalError("sizeof(synched_data) must be <= SESSION_STATE_BYTES");
	
	if (sizeof(resync_data) > SESSION_STATE_BYTES)
		FatalError("sizeof(resync_data) must be <= SESSION_STATE_BYTES");
	
	
	InitializeCriticalSection(&csSessions);
}

int GetEpoch()
{
	return epoch;
}

void NewEpoch()
{
	epoch++;
	if (epoch == 0)
		epoch = 1;
}

int GetTransmittedBytes()
{
	return transmitted_bytes;
}

void ResetTransmittedBytes()
{
	transmitted_bytes = 0;
}

void EnterSessionLock(void)
{
	EnterCriticalSection(&csSessions);
}

void LeaveSessionLock(void)
{
	LeaveCriticalSection(&csSessions);
}


session_node *AllocateSession(void)
{
	int i;
	
	for (i=0;i<num_sessions;i++)
	{
		if (sessions[i].connected == False)
			break;
	}
	
	if (i == num_sessions)
	{
	/* if no emptied low number sessions and using every session, can't
		use them */
		if (num_sessions == ConfigInt(SESSION_MAX_CONNECT))
			return NULL;
		
		num_sessions++;
	}
	
	/* we're gonna hang 'em up once synched if too many people on*/
	sessions[i].active = i < ConfigInt(SESSION_MAX_ACTIVE);
	
	sessions[i].session_id = i;
	sessions[i].blak_client = False;
	sessions[i].login_verified = False;
	sessions[i].hangup = False;
	sessions[i].account = NULL;
	sessions[i].exiting_state = False;
	sessions[i].receive_list = NULL;
	sessions[i].receive_index = 0;
	sessions[i].send_list = NULL;
	sessions[i].version_major = 0;
	sessions[i].version_minor = 0;
	sessions[i].seeds_hacked = False;
	sessions[i].secure_token = 0;
	sessions[i].sliding_token = NULL;
	
	return &sessions[i];
}

session_node * GetSessionByID(int session_id)
{
	if (session_id < 0 || session_id >= num_sessions)
		return NULL;
	
	if (sessions[session_id].session_id == session_id)
		return &sessions[session_id];
	else
		return NULL;
}

session_node * GetSessionByAccount(account_node *a)
{
	int i;
	
	for (i=0;i<num_sessions;i++)
		if (sessions[i].connected && sessions[i].account == a)
			return &sessions[i];
		return NULL;
}

/* called from interface thread, so it should make sure it has session lock first */
session_node * GetSessionBySocket(SOCKET sock)
{
	int i;
	
	for (i=0;i<num_sessions;i++)
		if (sessions[i].connected && sessions[i].conn.type == CONN_SOCKET &&
			sessions[i].conn.socket == sock && sessions[i].hangup == False)
		{
			return &sessions[i];
		}
		
		return NULL;
}

void ForEachSession(void (*callback_func)(session_node *s))
{
	int i;
	
	/* the callback function shouldn't delete the session,
	in case it becomes a linked list */
	
	for (i=0;i<num_sessions;i++)
		if (sessions[i].connected)
			callback_func(&sessions[i]);
}
/*
int GetUsedSessions()
{
int i,used;

  used = 0;
  
	for (i=1;i<num_sessions;i++)
	if (sessions[i].connected)
	used++;
	
	  return used;
	  }
*/

const char * GetStateName(session_node *s)
{
	const char *str=NULL;
	static char buf[50];
	
	/* static buffer for upload means that it shouldn't be used by
	other threads without protection, but interface thread does.
	That's bad */
	
	switch (s->state)
	{
		case STATE_ADMIN :
			str = "Admin";
			break;
		case STATE_MAINTENANCE :
			str = "Maintenance";
			break;
		case STATE_GAME : 
			if (s->game->game_state == GAME_BEACON)
				str = "Game - beacon";
			if (s->game->game_state == GAME_FINAL_SYNC)
				str = "Game - sync";
			if (s->game->game_state == GAME_NORMAL)
			{
				if (s->game->object_id == INVALID_OBJECT)
					str = "Game - pick";
				else
					str = "Game";
			}
			break;
		
		case STATE_TRYSYNC :
			str = "TrySync";
			break;
		case STATE_SYNCHED :
			if (s->syn->download_count == 0)
			{
				if (s->login_verified)
					str = "Synchronized";
				else
					str = "Synchronized - validating";
				break;
			}
			
			if(1+sprintf(buf,"Transferring %i",s->syn->download_count)>50)
				eprintf("Overflowed buf in GetStateName session.c");

			str = buf;
			break;
		case STATE_RESYNC : 
			str = "Resync"; 
			break;
		default : 
			str = "?";
	}
	
	return str;
}

void InitSessionState(session_node *s,int state)
{
	s->state = state;
	s->timer = 0;		/* no timer */
	
	switch (s->state)
	{
	case STATE_ADMIN :
		AdminInit(s);
		break;
	case STATE_MAINTENANCE :
		MaintenanceInit(s);
		break;
	case STATE_GAME :
		GameInit(s);
		break;
	case STATE_TRYSYNC :
		TrySyncInit(s);
		break;
	case STATE_SYNCHED :
		SynchedInit(s);
		break;
	case STATE_RESYNC :
		ResyncInit(s);
		break;
	}
	if (s->connected)
		InterfaceUpdateSession(s);
		/*
		if (s->account != NULL)
		dprintf("%s is in state %s",s->account->name,GetStateName(s));
		else
		dprintf("someone is in state %s\n",GetStateName(s));
	*/
}

void ExitSessionState(session_node *s)
{
/* these functions must NOT write to their session, because it may have
	hung up and you will get infinite loops */
	
	switch (s->state)
	{
	case STATE_ADMIN :
		AdminExit(s);
		break;
	case STATE_MAINTENANCE :
		MaintenanceExit(s);
		break;
	case STATE_GAME :
		GameExit(s);
		break;
	case STATE_TRYSYNC :
		TrySyncExit(s);
		break;
	case STATE_SYNCHED :
		SynchedExit(s);
		break;
	case STATE_RESYNC :
		ResyncExit(s);
		break;
	}
}

void SetSessionState(session_node *s,int state)
{
/* a recent write could have failed, if so don't do anything since
	they were closed */
	
	
    if (s->connected)
    {
		/* dprintf("state %i to %i\n",s->state,state); */
		ExitSessionState(s);
		InitSessionState(s,state);
    }
}

void SetSessionTimer(session_node *s,int seconds)
{
	s->timer = GetTime() + seconds;
}

void ClearSessionTimer(session_node *s)
{ 
	s->timer = 0;
}

/* called from interface thread, but with server lock */
session_node * CreateSession(connection_node conn)
{
	session_node *session;
	
	session = AllocateSession();
	if (session == NULL)
	{
		lprintf("CreateSession closing connection--connect overload\n");
		CloseConnection(conn);
		return NULL;
	}
	
	session->conn = conn; // struct copy
	
	if (conn.type == CONN_SOCKET)
	{
		session->muxReceive = CreateMutex(NULL,FALSE,NULL);
		session->muxSend = CreateMutex(NULL,FALSE,NULL);
	}
	
	session->connected = True;
	session->connected_time = GetTime();
	
	/* dprintf("CreateSession making session %i\n",session->session_id); */
	
	InterfaceLogon(session);
	
	return session;
}

void CloseConnection(connection_node conn)
{
	if (conn.type == CONN_SOCKET)
		closesocket(conn.socket);
}

void CloseSession(int session_id)
{
	session_node *s;
	
	s = GetSessionByID(session_id);
	if (s == NULL)
	{
		eprintf("CloseSession can't find session %i\n",session_id);
		return;
	}
	
	if (!s->connected)
	{
		eprintf("CloseSession can't close unconnected session %i\n",
			s->session_id);
		return;
	}
	
	EnterSessionLock();
	
	s->connected = False;
	
	if (!s->exiting_state)	/* if a write error occurred during an exit, don't */
		ExitSessionState(s);	/* go into infinite loop */
	
	if ((s->state != STATE_MAINTENANCE) && (s->account == NULL))
		lprintf("CloseSession closing session with no account from %s\n",
		s->conn.name);
	else
	{
		AccountLogoff(s->account);
		if (s->account != NULL)
			lprintf("CloseSession/4 logging off %i\n",s->account->account_id);
	}
	
	s->account = NULL;
	
	InterfaceLogoff(s);
	
	if (s->conn.type == CONN_SOCKET)
	{
		if (WaitForSingleObject(s->muxSend,10000) != WAIT_OBJECT_0)
			eprintf("CloseSession couldn't get session %i muxSend\n",s->session_id);
		else
		{
			DeleteBufferList(s->send_list);
			s->send_list = NULL;
			
			/* no need to release mutex... we're closing it */
			/*
			if (!ReleaseMutex(s->muxSend))
			eprintf("File %s line %i release of non-owned mutex\n",__FILE__,__LINE__);
			*/
			
		}
		
		if (WaitForSingleObject(s->muxReceive,10000) != WAIT_OBJECT_0)
			eprintf("CloseSession couldn't get session %i muxReceive\n",s->session_id);
		else
		{
			DeleteBufferList(s->receive_list);
			s->receive_list = NULL;
			
			/* no need to release mutex... we're closing it */
			/*
			if (!ReleaseMutex(s->muxReceive))
			eprintf("File %s line %i release of non-owned mutex\n",__FILE__,__LINE__);
			*/
			
		}
		
		if (!CloseHandle(s->muxSend))
			eprintf("CloseSession error (%s) closing send mutex %i in session %i\n",
			GetLastErrorStr(),s->muxSend,s->session_id);
		
		if (!CloseHandle(s->muxReceive))
			eprintf("CloseSession error (%s) closing receive mutex %i in session %i\n",
			GetLastErrorStr(),s->muxReceive,s->session_id);
		
		CloseConnection(s->conn);
	}
	
	s->session_id = -1;
	s->state = -1;
	
	LeaveSessionLock();
}

// Hangs up a session straight away, instead of posting to main loop.
void HangupSessionNow(session_node *s)
{
   if (!s->connected)
      return;

   if (s->conn.type != CONN_SOCKET)
      return;

   CloseSession(s->session_id);
   HangupSession(s);
}

void HangupSession(session_node *s)
{
	s->hangup = True;
	
	/* set event so that main loop will check it out */
	SignalSession(s->session_id);
}

void CloseAllSessions()
{
	int i;
	
	for (i=0;i<num_sessions;i++)
		if (sessions[i].connected)
			CloseSession(i);
}

void PollSessions()
{
	session_node *s;
	int i,poll_time;
	
	poll_time = GetTime();
	
	ProcessSysTimer(poll_time);
	
	for (i=0;i<num_sessions;i++)
	{
		s = GetSessionByID(i);
		if (s == NULL)
			continue;
		
		PollSession(s->session_id);
		
		if (s->connected)
		{
			if (s->timer != 0 && poll_time >= s->timer)
				ProcessSessionTimer(s);
		}
		
	}
}

void PollSession(int session_id)
{
	session_node *s;
	
	s = GetSessionByID(session_id);
	if (s == NULL)
		return;
	
	if (!s->connected)
		return;
	
	if (s->conn.type != CONN_SOCKET)
		return;
	
		/* someone hung up session or if the read/write thread for this session is dead, 
	hang it up. */
	if (s->hangup == True)
	{
		CloseSession(s->session_id);
		return;
	}
	
	if (WaitForSingleObject(s->muxReceive,10000) != WAIT_OBJECT_0)
	{
		eprintf("PollSession bailed waiting for mutex on session %i\n",s->session_id);
		HangupSession(s);
		return;
	}
	
	/*
	if (s->num_receiving > 0)
	{
	int i;
	dprintf("got in %s\n",GetStateName(s));
	for (i=0;i<s->num_receiving;i++)
	dprintf("%5u",s->receiving_buf[i]);
	dprintf("\n");
	}
	*/
	
	if (s->receive_list != NULL)
		ProcessSessionBuffer(s);
	
	
	if (!ReleaseMutex(s->muxReceive))
	{
		eprintf("PollSession released mutex it didn't own in session %i\n",s->session_id);
		HangupSession(s);
	}
}

void VerifiedLoginSession(int session_id)
{
	session_node *s;
	
	s = GetSessionByID(session_id);
	if (s == NULL)
		return;
	
	if (!s->connected)
		return;
	
	if (s->conn.type != CONN_SOCKET)
		return;
	
	if (s->state != STATE_SYNCHED || s->login_verified)
		return;
	
	VerifyLogin(s);
}

void ProcessSessionBuffer(session_node *s)
{
	int prev_session_state;
	
	/* need a loop, in case the state changes, so the new
	state can handle the rest of the waiting bytes */
	
	/*
	int i;
	if (s->session_id == 0)
	{
	for (i=0;i<s->num_receiving;i++)
	dprintf("%5u\n",(unsigned)(unsigned char)(s->receiving_buf[i]));
	}
	*/
	do
	{
		prev_session_state = s->state;
		
		switch (s->state)
		{
		case STATE_ADMIN :
			AdminProcessSessionBuffer(s);
			break;
		case STATE_MAINTENANCE :
			MaintenanceProcessSessionBuffer(s);
			break;
		case STATE_GAME :
			GameProcessSessionBuffer(s);
			break;
		case STATE_TRYSYNC :
			TrySyncProcessSessionBuffer(s);
			break;
		case STATE_SYNCHED :
			SynchedProcessSessionBuffer(s);
			break;
		case STATE_RESYNC :
			ResyncProcessSessionBuffer(s);
			break;
		}
	} while (s->connected && s->state != prev_session_state);
}

void ProcessSessionTimer(session_node *s)
{
	ClearSessionTimer(s);
	
	switch (s->state)
	{
	case STATE_ADMIN :
		/*      AdminProcessSessionTimer(s); */
		break;
	case STATE_MAINTENANCE :
		/*      MaintenanceProcessSessionTimer(s); */
		break;
	case STATE_GAME :
		GameProcessSessionTimer(s); 
		break;
	case STATE_TRYSYNC :
		TrySyncProcessSessionTimer(s);
		break;
	case STATE_SYNCHED :
		SynchedProcessSessionTimer(s); 
		break;
	case STATE_RESYNC :
		ResyncProcessSessionTimer(s);
		break;
	}
	
}

int GetSessionReadBytes(session_node *s)
{
	buffer_node *bn;
	int bytes;
	
	bytes = 0;
	if (s->receive_list != NULL)
	{
		bytes += s->receive_list->len_buf - s->receive_index;
		bn = s->receive_list->next;
		while (bn != NULL)
		{
			bytes += bn->len_buf;
			bn = bn->next;
		}
	}
	return bytes;
}

/* if possible, read num_bytes from session.  If not possible,
return false and write nothing.  DO NOT pass in 0 bytes to read. */
Bool ReadSessionBytes(session_node *s,int num_bytes,void *buf)
{
	buffer_node *bn,*blist;
	int copied,copy_bytes;
	
	if (GetSessionReadBytes(s) < num_bytes)
		return False;
	
	blist = s->receive_list;
	
	if (blist->len_buf - s->receive_index > num_bytes)
	{
		memcpy(buf,blist->buf + s->receive_index,num_bytes);
		s->receive_index += num_bytes;
		return True;
	}
	
	/* reading at least as many bytes as there are in first buffer */
	
	copy_bytes = blist->len_buf - s->receive_index;
	memcpy(buf,blist->buf + s->receive_index,copy_bytes);
	copied = copy_bytes;
	
	bn = blist->next;
	DeleteBuffer(blist);
	s->receive_list = bn;
	s->receive_index = 0;
	
	copy_bytes = 0; /* in case exactly read message, set receive_index right */
	while (s->receive_list != NULL && copied < num_bytes)
	{
	  copy_bytes = std::min(num_bytes-copied,s->receive_list->len_buf);
	  memcpy((char *)buf+copied,s->receive_list->buf,copy_bytes);
	  
	  copied += copy_bytes;
	  if (copy_bytes == s->receive_list->len_buf)
	    {
	      bn = s->receive_list->next;
	      DeleteBuffer(s->receive_list);
	      s->receive_list = bn;
	      copy_bytes = 0; /* if last copy ended with this buffer,
				 set receive_index right */
	    }
	}
	s->receive_index = copy_bytes;
	return True;
}

/* if possible, peek at num_bytes from session.  If not possible,
return false and write nothing.  DO NOT pass in 0 bytes to read. */
Bool PeekSessionBytes(session_node *s,int num_bytes,void *buf)
{
	buffer_node *bn,*blist;
	int copied,copy_bytes;
	
	if (GetSessionReadBytes(s) < num_bytes)
		return False;
	
	blist = s->receive_list;
	
	if (blist->len_buf - s->receive_index > num_bytes)
	{
		memcpy(buf,blist->buf + s->receive_index,num_bytes);
		return True;
	}
	
	/* reading at least as many bytes as there are in first buffer */
	
	copy_bytes = blist->len_buf - s->receive_index;
	memcpy(buf,blist->buf + s->receive_index,copy_bytes);
	
	bn = blist->next;
	
	copied = copy_bytes;
	while (bn != NULL && copied < num_bytes)
	{
	  copy_bytes = std::min(num_bytes-copied,bn->len_buf);
	  memcpy((char *)buf+copied,bn->buf,copy_bytes);
	  
	  copied += copy_bytes;
	  if (copy_bytes == bn->len_buf)
	    bn = bn->next;
	}
	
	return True;
}

void SendClientStr(int session_id,char *str)
{
	SendClient(session_id,str,(unsigned short)strlen(str));
}

void SendClient(int session_id,char *data,unsigned short len_data)
{
	session_node *s;
	
	s = GetSessionByID(session_id);
	if (s == NULL)
	{
		/* the session has already closed, so happens all the time */
		/* eprintf("SendClient can't output to non-session %i\n",session_id); */
		return;
	}
	
	switch (s->state)
	{
	case STATE_GAME :
		SendGameClient(s,data,len_data,epoch);
		break;
	case STATE_SYNCHED :
		SendGameClient(s,data,len_data,0);
		break;
	case STATE_ADMIN :
	case STATE_MAINTENANCE :
	case STATE_TRYSYNC :
		SendBytes(s,data,len_data);
		break;
	}
}

void SendGameClient(session_node *s,char *data,unsigned short len_data,int seqno)
{
	unsigned short crc16;
	char buf[HEADERBYTES];
	
	/*
	dprintf("Sending %i bytes\n",len_data); 
	dprintf("Sending msg command = %i\n",(unsigned char)data);
	*/
	crc16 = GetCRC16(data,len_data);
	
	memcpy(buf,&len_data,LENBYTES);
	memcpy(buf + LENBYTES,&crc16,CRCBYTES);
	memcpy(buf + LENBYTES + CRCBYTES,&len_data,LENBYTES);
	assert(seqno>255);
	buf[LENBYTES*2 + CRCBYTES] = seqno; /* epoch or 0 for AP stuff */
	
	SendBytes(s,buf,sizeof(buf));
	if (!s->connected)
		return;
	SendBytes(s,(char *)data,len_data);
}

void SendBytes(session_node *s,char *buf,int len_buf)
{
	if (s->conn.type == CONN_CONSOLE)
	{
		InterfaceSendBytes(buf,len_buf);
		return;
	}
	
	if (s->hangup)
		return;   
	
	if (WaitForSingleObject(s->muxSend,10000) != WAIT_OBJECT_0)
	{
		eprintf("SendBytes couldn't get session %i muxSend\n",s->session_id);
		return;
	}
	
	if (s->send_list == NULL)
	{
		/* if nothing in queue, try to send right now */
		
		if (send(s->conn.socket,buf,len_buf,0) == SOCKET_ERROR)
		{
			if (GetLastError() != WSAEWOULDBLOCK)
			{
				/* eprintf("SendBytes got send error %i\n",GetLastError()); */
				if (!ReleaseMutex(s->muxSend))
					eprintf("File %s line %i release of non-owned mutex\n",__FILE__,__LINE__);
				HangupSession(s);
				return;
			}
			
			s->send_list = AddToBufferList(s->send_list,buf,len_buf);
		}
		else
		{
			transmitted_bytes += len_buf;
		}
	}
	else
	{
		s->send_list = AddToBufferList(s->send_list,buf,len_buf);
	}
	
	if (!ReleaseMutex(s->muxSend))
		eprintf("File %s line %i release of non-owned mutex\n",__FILE__,__LINE__);
}

/*------------ above here is junk-o byte buffer sending */
/*------------ below here is nice buffer-list sending */

void SendClientBufferList(int session_id,buffer_node *blist)
{
	session_node *s;
	
	s = GetSessionByID(session_id);
	if (s == NULL)
	{
		DeleteBufferList(blist);
		return;
	}
	
	switch (s->state)
	{
	case STATE_GAME :
		SendGameClientBufferList(s,blist,epoch);
		break;
	case STATE_SYNCHED :
		SendGameClientBufferList(s,blist,0);
		break;
	case STATE_ADMIN :
	case STATE_MAINTENANCE :
	case STATE_TRYSYNC :
		SendBufferList(s,blist);
		break;
	}
}

unsigned short __inline GetCRC16BufferList(buffer_node *blist)
{
	unsigned int crc32;
	
	crc32 = (unsigned int)-1;
	
	while (blist != NULL)
	{
		crc32 = CRC32Incremental(crc32,blist->buf,blist->len_buf);
		blist = blist->next;
	}
	
	crc32 ^= -1;
	
	return (unsigned short)(0xffff & crc32);
}

void SendGameClientBufferList(session_node *s,buffer_node *blist,char seqno)
{
	buffer_node *bn;
	unsigned short crc16;
	unsigned int len;
	
	if (blist == NULL)
		return;
	
	len = 0;
	bn = blist;
	while (bn != NULL)
	{
		len += bn->len_buf;
		bn = bn->next;
	}
	
	/* dprintf("SendClientBufferList %i bytes\n",len); */
	
	crc16 = GetCRC16BufferList(blist);
	
	
	memcpy(blist->prebuf,&len,LENBYTES);
	memcpy(blist->prebuf + LENBYTES,&crc16,CRCBYTES);
	memcpy(blist->prebuf + LENBYTES + CRCBYTES,&len,LENBYTES);
	blist->prebuf[LENBYTES*2 + CRCBYTES] = seqno;
	
	blist->buf = blist->prebuf;
	blist->len_buf += HEADERBYTES;
	/*
	bn = NULL;
	bn = AddToBufferList(bn,&len,LENBYTES);
	bn = AddToBufferList(bn,&crc16,CRCBYTES);
	bn = AddToBufferList(bn,&len,LENBYTES);
	bn = AddToBufferList(bn,&seqno,1);
	
	  bn->next = blist;
	*/ 
	SendBufferList(s,blist);
	
}

void SendBufferList(session_node *s,buffer_node *blist)
{
	buffer_node *bn;
	
	if (s->conn.type == CONN_CONSOLE)
	{
		InterfaceSendBufferList(blist);
		return;
	}
	
	if (!s->connected || s->hangup)
	{
		DeleteBufferList(blist);
		return;
	}
	
	
	if (WaitForSingleObject(s->muxSend,10000) != WAIT_OBJECT_0)
	{
		eprintf("SendBufferList couldn't get session %i muxSend\n",s->session_id);
		return;
	}
	
	if (s->send_list == NULL)
	{
		/* if nothing in queue, try to send right now */
		
		while (blist != NULL)
		{
			if (send(s->conn.socket,blist->buf,blist->len_buf,0) == SOCKET_ERROR)
			{
				if (GetLastError() != WSAEWOULDBLOCK)
				{
					if (!ReleaseMutex(s->muxSend))
						eprintf("File %s line %i release of non-owned mutex\n",__FILE__,__LINE__);
					/* eprintf("SendBufferList got send error %i\n",GetLastError()); */
					DeleteBufferList(blist);
					HangupSession(s);
					return;
				}
				/* dprintf("%i adding to buffer list\n",s->session_id); */
				SessionAddBufferList(s,blist);
				break;
			}
			else
			{
				transmitted_bytes += blist->len_buf;
				
				bn = blist->next;
				DeleteBuffer(blist);
				blist = bn;
			}	 
		}
	}
	else
	{
		SessionAddBufferList(s,blist);
	}
	
	if (!ReleaseMutex(s->muxSend))
		eprintf("File %s line %i release of non-owned mutex\n",__FILE__,__LINE__);
}

void SessionAddBufferList(session_node *s,buffer_node *blist)
{
	buffer_node *bn,*temp;
	int length;
	
	/* prereq: we must already hold the muxSend for s */
	
	if (s->send_list == NULL)
	{
		/* put first node on, then try the compress junk */
		s->send_list = blist;
		blist = blist->next;
		s->send_list->next = NULL;
		bn = s->send_list;
	}
	else
	{
		length = 1;
		bn = s->send_list;
		while (bn->next != NULL)
		{
			length++;
			bn = bn->next;
		}
		
		/* dprintf("non-blank send list len %i\n",length); */      
		
		/* if currently backed up more than MAX_SESSION_BUFFER_LIST_LEN,
		then don't waste any more memory on them */
		if (length > MAX_SESSION_BUFFER_LIST_LEN)
		{
			//dprintf("SessionAddBufferList deleting because buffer list too long %i\n",s->session_id);
			DeleteBufferList(blist);
			return;
		}
	}
	
	/* simple approach: set bn->next to blist.  However, this can use up
	a ton of buffers, when the amount of data to be sent is small.  So
	do a couple discreet checks, and perhaps memcpy's. */
	while (blist != NULL && blist->len_buf < (bn->size_prebuf - bn->len_buf - HEADERBYTES))
	{
		/* dprintf("squeezing %i in %i\n",blist->len_buf,bn->size_buf-bn->len_buf); */
		memcpy(bn->buf+bn->len_buf,blist->buf,blist->len_buf);
		bn->len_buf += blist->len_buf;
		temp = blist->next;
		DeleteBuffer(blist);
		blist = temp;
	}
	
	bn->next = blist;
}

