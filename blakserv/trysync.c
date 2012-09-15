// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * trysync.c
 *

 This module supports one mode of a session, STATE_TRYSYNC.  This is
 the state that a session is in when we get the beacon_str from the
 user which means they're running the Blakston client.  We then send
 back a string, and must then get detect_str at which time we are
 synchronized, and go to STATE_SYNCHED.

 We wait in this state, no matter how many bytes we read, until a certain
 amount of time goes by.  This is because there can be many leftover
 beacon strings in the queue.  The value for TRYSYNC_TIMEOUT_SECS is
 critical.  Needs to be analyzed a bit.

 */

#include "blakserv.h"

/* string we send the client to tell it we're ready to go to sync'ed mode */
unsigned char tell_cli_str[] = { 3, 251, 98, 108, 97, 107, 10, 13, 1 };

/* string the client sends once when it is sync'ed */
unsigned char detect_str[] = { 7, 230, 98, 108, 97, 107, 10, 13, 8 };

/* local function prototypes */
void TrySyncInputChar(session_node *s,char ch);


void TrySyncInit(session_node *s)
{
   s->ts = (trysync_data *)s->session_state_data;
   s->ts->detect_index = 0;

   SetSessionTimer(s,TRYSYNC_TIMEOUT_SECS);

   SendClient(s->session_id,(char *) tell_cli_str,LENGTH_TELL_CLI);
}

void TrySyncExit(session_node *s)
{
}

void TrySyncProcessSessionTimer(session_node *s)
{
   /* we're going back to resync mode, which means we wait for the client
      to send us beacon_str.  It might not know that, so we send some
      junk which MUST generate a protocol error, so it resyncs */
   cprintf(s->session_id,"aoeuaoeuaoeutha");      
   SetSessionState(s,STATE_RESYNC);
}

void TrySyncProcessSessionBuffer(session_node *s)
{
   char ch;

   while (s->receive_list != NULL)
   {
      if (ReadSessionBytes(s,1,&ch) == False)
	 return;
      
      TrySyncInputChar(s,ch);

      /* any character could change our state.  if so, leave */
      if (s->state != STATE_TRYSYNC)
	 return;
   }
}


void TrySyncInputChar(session_node *s,char ch)
{
   if (ch == detect_str[s->ts->detect_index])
      s->ts->detect_index++;
   else
   {
      s->ts->detect_index = 0;
      return;
   }

   if (s->ts->detect_index == LENGTH_DETECT)
   {
      SetSessionState(s,STATE_SYNCHED);
      return;
   }
}



