// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statstrt.c:  Handle resynchronization state, where we are trying to establish a
 *   connection with the server.
 * 
 * This state is no longer used at startup; it's only used for resynchronizing when an error
 * occurs.  (It used to be used at startup for serial port support.)
 * 
 * We send client_string1 repeatedly and wait for server_string for a certain amount of time.
 * If we don't get it, we hang up and quit.
 * If we do, we send client_string2 and go into login mode.
 */

#include "client.h"

#define BEACON_INTERVAL 2000  /* # of milliseconds between sends of 1st string */
#define BEACON_TIMEOUT  60000 /* # of milliseconds until we bail out */

/* Codes defined in login.txt */
#define INITSTR_LENGTH 9  /* Length of following strings */
static unsigned char client_string1[] = { 1, 255, 66, 76, 65, 75, 10, 13, 2};
static unsigned char server_string[]  = { 3, 251, 98, 108, 97, 107, 10, 13, 1};
static unsigned char client_string2[] = { 7, 230, 98, 108, 97, 107, 10, 13, 8};

static int pos;          /* Where we are in server_string */

static int timer_id = 0; /* Timer currently in use, or 0 if none */
static int timeout;      /* # of milliseconds that have passed so far */

static int dest_state;   /* State we should go to when resync succeeds */

void CALLBACK StartupTimerProc(HWND hwnd, UINT msg, UINT timer, DWORD dwTime);

/****************************************************************************/
/* 
 * StartupInit:  Send startup string, and wait for string from server.
 *   final_state is the state we should end up in if the resynchronization succeeds.
 *   This should be STATE_LOGIN if we are just connecting to the server or if we 
 *   try to resync in login mode, and STATE_GAME if we are trying to resync in game mode.
 */
void StartupInit(int final_state)
{
   dest_state = final_state;
   pos = 0;
   timeout = 0;

   /* Set timer to keep writing initial string to server */
   WriteServer((char *) client_string1, INITSTR_LENGTH);
   timer_id = SetTimer(NULL, 0, BEACON_INTERVAL, StartupTimerProc);
   if (timer_id == 0)
   {
      ClientError(hInst, hMain, IDS_NOTIMERS);
      return;
   }
}
/************************************************************************/
void CALLBACK StartupTimerProc(HWND hwnd, UINT msg, UINT timer, DWORD dwTime)
{
   /* See if we've timed out */
   timeout += BEACON_INTERVAL;
   debug(("In timer proc\n"));
   if (timeout >= BEACON_TIMEOUT)
   {
      Logoff();
      ClientError(hInst, hMain, IDS_CONNECTERROR);
      return;
   }

   /* Spew at server again */
   WriteServer((char *) client_string1, INITSTR_LENGTH);
}
/****************************************************************************/
/*
 * AddCharsStartup:  We got some stuff from the server; see what it is.
 */
void AddCharsStartup(char *message, int len)
{
   int i;
   Bool done = False;

   for (i=0; i < len; i++)
   {
      /* If character doesn't match code, start over */
      if (server_string[pos] != (unsigned char) message[i])
      {
	 pos = 0;
	 continue;
      }
      pos++;

      /* If we reach end of server string, win */
      if (pos == INITSTR_LENGTH)
      {
	 done = True;
	 break;
      }
   }

   if (!done)
      return;

   debug(("Got response from server\n"));

   /* We found server's code string.  Send response and go into correct mode. */
   AbortStartupTimer();
   WriteServer((char *) client_string2, INITSTR_LENGTH);
   
   switch (dest_state)
   {
   case STATE_LOGIN:
      MainSetState(STATE_LOGIN);
      break;

   case STATE_GAME:
      /* We're already in game state, so don't do MainSetState--just get new game data */
      ResetUserData();
      break;
   }
}
/****************************************************************************/
/*
 * AbortStartupTimer:  Stop the timer which is trying to synchronize with the
 *   server, if it is running.  This is useful if the user aborts a login attempt;
 *   it's also called when we leave the startup state.
 */
void AbortStartupTimer(void)
{
   if (timer_id != 0)
   {
      KillTimer(NULL, timer_id);
      timer_id = 0;
   }
}
