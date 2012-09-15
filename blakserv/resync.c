// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * resync.c
 *

 This module supports one mode of a session, STATE_RESYNC.  This is
 the state that a session is in if it was in either STATE_SYNCHED or
 STATE_GAME and an error is detected (because the header of a packet
 is fried).  The client is supposed to send us beacon_str, which sends
 us to STATE_TRYSYNC where we send back a string and then we'll be
 synched again.

 */

#include "blakserv.h"

/* string the client sends over and over to tell us it's trying to resync */
unsigned char beacon_str[] = { 1, 255, 66, 76, 65, 75, 10, 13, 2 };

/* local function prototypes */
void ResyncInputChar(session_node *s,char ch);

void ResyncInit(session_node *s)
{
   s->resyn = (resync_data *)s->session_state_data;

   s->resyn->beacon_index = 0;

   SetSessionTimer(s,RESYNC_NOSYNC_SECS);

}

void ResyncExit(session_node *s)
{
}

void ResyncProcessSessionTimer(session_node *s)
{
   HangupSession(s);
}

void ResyncProcessSessionBuffer(session_node *s)
{
   char ch;

   while (s->receive_list != NULL)
   {
      if (ReadSessionBytes(s,1,&ch) == False)
	 return;
      
      ResyncInputChar(s,ch);

      /* any character could change our state.  if so, leave */
      if (s->state != STATE_RESYNC)
	 return;
   }
}


void ResyncInputChar(session_node *s,char ch)
{
   if (ch == beacon_str[s->resyn->beacon_index])
      s->resyn->beacon_index++;
   else
      s->resyn->beacon_index = 0;

   if (s->resyn->beacon_index == LENGTH_BEACON)
   {
      SetSessionState(s,STATE_TRYSYNC);
      return;
   }

}
