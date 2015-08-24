// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * session.h
 *
 */

#ifndef _SESSION_H
#define _SESSION_H

/* for async.c to add bytes it writes */
extern int transmitted_bytes; 

#define SEED_COUNT 5

enum
{
   CR = 13,
   LF = 10,
   BELL = 7,
   BS = 8
};

#define LENBYTES 2
#define CRCBYTES 2
#define HEADERBYTES 7

enum { CONN_SOCKET, CONN_CONSOLE };

enum { STATE_ADMIN, STATE_GAME, STATE_TRYSYNC, 
       STATE_SYNCHED, STATE_RESYNC, STATE_MAINTENANCE };

#define SESSION_STATE_BYTES 120

enum
{
   BUFFER_SIZE = 10000, /* used in bufpool.c, but also related here! */
   MAX_SESSION_BUFFER_LIST_LEN = 20,
};


/* this structure is created in conn.c, and put into a session */
typedef struct
{
   int type;
   
   SOCKET socket;
   char peer_data[MAXGETHOSTSTRUCT]; /* filled in by async database call to get DNS name */
   HANDLE hLookup;                   /* handle to async lookup call */
   
   char name[100];

   struct in6_addr addr;
} connection_node;

typedef struct
{
   int session_id;
   connection_node conn;
   Bool active;			/* False if we're gonna hang 'em up
				   because too many people online */
   Bool connected;
   int connected_time;
   int state;
   Bool hangup;                 /* if set, PollSessions will hang us up next time 'round */
   int timer;			/* time to call its state timer */

   char session_state_data[SESSION_STATE_BYTES];

   /* these guys are just set to point to session_state_data */
   union
   {
      admin_data *adm;
      admin_data *mtn; /* same data as admin_data */
      game_data *game;
      trysync_data *ts;
      synched_data *syn;
      resync_data *resyn;
   };

   account_node *account;
   Bool login_verified;         /* Portal said they're ok */
   Bool blak_client;		/* if they are running our client */

   /* data about the user's machine, only valid if blak_client is true */
   char version_major;		/* version of client */
   char version_minor;
   int os_type;
   int os_version_major;
   int os_version_minor;
   int machine_ram;
   int machine_cpu;
   short screen_x;
   short screen_y;
   short screen_color_depth;
   short partner;
   int displays_possible;
   int bandwidth;
   int reserved;
   
   Bool exiting_state;		/* true iff in ExitXXX, so errors on writing don't inf loop */
				/* only needs to be set if you write, so it's only in exitgame */

   int last_download_time; /* as reported by the client.  Needed to communicate from
			      synched mode to upload mode */

   unsigned int seeds[SEED_COUNT]; /* for security in game mode */
   Bool seeds_hacked;
   unsigned int secure_token;
   char* sliding_token;

   HANDLE muxReceive;
   /* this protects the list of received data: receive_list, and receive_index */
   buffer_node *receive_list;
   int receive_index; /* index into first buffer of receive_list, of where we are */


   HANDLE muxSend;
   /* this protects the list of buffers to be sent: send_list */
   buffer_node *send_list;

} session_node;

/* state function prototypes that have to come after session_node */
void AdminInit(session_node *s);
void AdminExit(session_node *s);
void AdminProcessSessionTimer(session_node *s);
void AdminProcessSessionBuffer(session_node *s);
void MaintenanceInit(session_node *s);
void MaintenanceExit(session_node *s);
void MaintenanceProcessSessionTimer(session_node *s);
void MaintenanceProcessSessionBuffer(session_node *s);
void GameInit(session_node *s);
void GameExit(session_node *s);
void GameClientExit(session_node *s);
void GameCleanupExit(session_node *s);
void GameProcessSessionTimer(session_node *s);
void GameProcessSessionBuffer(session_node *s);
void TrySyncInit(session_node *s);
void TrySyncExit(session_node *s);
void TrySyncProcessSessionTimer(session_node *s);
void TrySyncProcessSessionBuffer(session_node *s);
void SynchedInit(session_node *s);
void SynchedExit(session_node *s);
void SynchedProcessSessionTimer(session_node *s);
void SynchedProcessSessionBuffer(session_node *s);
void SendSynchedMessage(session_node *s,char *str,char logoff);
void VerifyLogin(session_node *s);
void ResyncInit(session_node *s);
void ResyncExit(session_node *s);
void ResyncProcessSessionTimer(session_node *s);
void ResyncProcessSessionBuffer(session_node *s);


void InitSession(void);
int GetEpoch(void);
void NewEpoch(void);
int GetTransmittedBytes(void);
void ResetTransmittedBytes(void);
void EnterSessionLock(void);
void LeaveSessionLock(void);
void SendBytes(session_node *s,char *buf,int len_buf);
void InitSessionState(session_node *s,int state);
session_node * CreateSession(connection_node conn);
session_node *GetSessionByAccount(account_node *a);
session_node * GetSessionBySocket(SOCKET sock);
void ForEachSession(void (*callback_func)(session_node *s));
int GetUsedSessions(void);
const char * GetStateName(session_node *s);
session_node *GetSessionByID(int session_id);
void SetSessionState(session_node *s,int state);
void SetSessionTimer(session_node *s,int seconds);
void ClearSessionTimer(session_node *s);
int GetSessionReadBytes(session_node *s);
Bool ReadSessionBytes(session_node *s,int num_bytes,void *buf);
Bool PeekSessionBytes(session_node *s,int num_bytes,void *buf);
void SendClientStr(int session_id,char *str);
void SendClient(int session_id,char *data,unsigned short len_data);
void SendClientBufferList(int session_id,buffer_node *blist);
void HangupSessionNow(session_node *s);
void HangupSession(session_node *s);
void CloseAllSessions(void);
void PollSessions(void);
void PollSession(int session_id);
void VerifiedLoginSession(int session_id);


unsigned int __inline GetCRC16(char *buf,int len_buf)
{
   return 0xffff & CRC32(buf,len_buf);
}


#endif
