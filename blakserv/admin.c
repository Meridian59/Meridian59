// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * admin.c
 *
 
 This module supports one mode of a session, STATE_ADMIN.  Only
 administrators are allowed in this mode.  The admin commands are
 sent to adminfn.c for processing, since this can be done through a 
 game command as well.

 */

#include "blakserv.h"

/* local function prototypes */
void AdminInputChar(session_node *s,char ch);

void AdminInit(session_node *s)
{
   s->adm = (admin_data *)s->session_state_data;
   
   s->adm->command[0] = 0;

   cprintf(s->session_id,"Administrator mode\n");
}

void AdminExit(session_node *s)
{
}

void AdminProcessSessionBuffer(session_node *s)
{
   char ch;

   while (s->receive_list != NULL)
   {
      if (ReadSessionBytes(s,1,&ch) == False)
	 return;
      
      /* give up receive mutex, so the interface/socket thread can
	 read data for us, even if doing something long (GC/save/reload sys) */

      if (!ReleaseMutex(s->muxReceive))
	 eprintf("APSBPollSession released mutex it didn't own in session %i\n",
		 s->session_id);
      
      AdminInputChar(s,ch);

      if (WaitForSingleObject(s->muxReceive,10000) != WAIT_OBJECT_0)
      {
	 eprintf("APSB bailed waiting for mutex on session %i\n",s->session_id);
	 return;
      }

      /* any character could change our state.  if so, leave */
      if (s->hangup == True || s->state != STATE_ADMIN)
	 return;
   }
}


void AdminInputChar(session_node *s,char ch)
{
   int len,session_id;

   if (ch != CR)
   {
      len = strlen(s->adm->command);
      if (len < MAX_ADMIN_COMMAND - 1)
      {
	 s->adm->command[len] = ch;
	 s->adm->command[len+1] = 0;
      }
      return;
   }

   /* they hit CR, so they've done a command */

   cprintf(s->session_id,"> %s\n",s->adm->command);
   session_id = s->session_id;
   
   if (!strnicmp(s->adm->command,"QUIT",4))
   {
      cprintf(s->session_id,"%c%c",27,'G'); /* this tells client to resynchronize */
      SetSessionState(s,STATE_RESYNC);

      return;
      
   }
   
   TryAdminCommand(s->session_id,s->adm->command);
   
   /* this next stuff is fuzzy--not sure how much is needed 7/27/95 */
   
   /* right here, s could be invalid if we got hung up.
      check with the saved id */
   s = GetSessionByID(session_id);
   if (s == NULL)
      return;
   if (s->state == STATE_ADMIN)
   {
      /* set string to zero because the rest of line parameters (R) go past
       * a zero from a strtok, and could have residue from previous commands.
       * Of course, only do this if we didn't change state.
       */
      
      s->adm->command[0] = 0;
      memset(s->adm->command,0,MAX_ADMIN_COMMAND);
   }
}
