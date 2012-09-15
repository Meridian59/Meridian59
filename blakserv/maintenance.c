// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * maintenance.c
 *
 
 This module supports one mode of a session, STATE_MAINTENANCE.
 This state is reachable by connecting to a special port on the 
 server (ConfigInt(SOCKET_PORT_MTNIN)) and not by normal clients.

 */

#include "blakserv.h"

/* local function prototypes */
void MaintenanceInputChar(session_node *s,char ch);

void MaintenanceInit(session_node *s)
{
   s->mtn = (admin_data *)s->session_state_data;
   
   s->mtn->command[0] = 0;

   /* cprintf(s->session_id,"Maintenance mode\n"); */
}

void MaintenanceExit(session_node *s)
{
}

void MaintenanceProcessSessionBuffer(session_node *s)
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
      
      MaintenanceInputChar(s,ch);

      if (WaitForSingleObject(s->muxReceive,10000) != WAIT_OBJECT_0)
      {
	 eprintf("APSB bailed waiting for mutex on session %i\n",s->session_id);
	 return;
      }

      /* any character could change our state.  if so, leave */
      if (s->hangup == True || s->state != STATE_MAINTENANCE)
	 return;
   }
}


void MaintenanceInputChar(session_node *s,char ch)
{
   int len,session_id;

   if (ch != CR)
   {
      len = strlen(s->mtn->command);
      if (len < MAX_ADMIN_COMMAND - 1)
      {
	 s->mtn->command[len] = ch;
	 s->mtn->command[len+1] = 0;
      }
      return;
   }

   /* they hit CR, so they've done a command */

   cprintf(s->session_id,"> %s\n",s->mtn->command);
   session_id = s->session_id;
   
   if (!strnicmp(s->mtn->command,"QUIT",4))
   {
      cprintf(s->session_id,"Bye\n");
      HangupSession(s);
      return;
      
   }
   
   TryAdminCommand(s->session_id,s->mtn->command);
   
   /* right here, s could be invalid if we got hung up.
      check with the saved id */
   s = GetSessionByID(session_id);
   if (s == NULL)
      return;

   if (s->state == STATE_MAINTENANCE)
   {
      /* set string to zero because the rest of line parameters (R) go past
       * a zero from a strtok, and could have residue from previous commands.
       * Of course, only do this if we didn't change state.
       */
      
      s->mtn->command[0] = 0;
      memset(s->mtn->command,0,MAX_ADMIN_COMMAND);
   }
}
