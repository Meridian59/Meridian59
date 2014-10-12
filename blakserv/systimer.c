// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * systimer.c
 *

 This module takes care of system timers, which just means that every
 once in a while the system needs to do things.  Somewhere in
 session.c (PollSessions()) calls us to see if we need to do anything.
 The times are all seconds.

 The way these guys work is that if the current time mod a timer's 
 period equal the timer's time, then it is activated.  For example,
 the garbage collector could be activated on the hour with time = 0
 period = 60*60 (one hour).

 Garbage collecting, saving, sending a "time has passed" message to
 Blakod, and updating our window interface are currently what we do.

 */

#include "blakserv.h"

systimer_node *systimers;

/* local function prototypes */
void CreateInitialSysTimers();
void ProcessOneSysTimer(systimer_node *st);

void InitSysTimer()
{
   systimers = NULL;
   CreateInitialSysTimers();
}

void ResetSysTimer()
{
   systimer_node *st,*temp;

   st = systimers;
   while (st != NULL)
   {
      temp = st->next;
      FreeMemory(MALLOC_ID_SYSTIMER,st,sizeof(systimer_node));
      st = temp;
   }
}

void CreateSysTimer(int type,int time,int period)
{
   systimer_node *st;
   int now,next_time;

   if (period == 0)
   {
      eprintf("CreateSysTimer got timer type %d time %d period ZERO! Probably doomed.\n",
	     type, time);
      period = 9999999;
      return;
   }

   st = (systimer_node *)AllocateMemory(MALLOC_ID_SYSTIMER,sizeof(systimer_node));
   st->systimer_type = type;
   st->time = time;
   st->period = period;
   st->enabled = True;
   st->next = NULL;

   now = GetTime();
   next_time = now - (now % period) + time;
   if (now > next_time)
      next_time += period;

   st->next_time_activate = next_time;
   
   
   st->next = systimers;
   systimers = st;
}

void CreateInitialSysTimers()
{
   CreateSysTimer(SYST_RESET_POOL,60*ConfigInt(AUTO_RESET_POOL_TIME),
		  60*ConfigInt(AUTO_RESET_POOL_PERIOD));
   CreateSysTimer(SYST_RESET_TRANSMITTED,ConfigInt(AUTO_TRANSMITTED_TIME),
		  ConfigInt(AUTO_TRANSMITTED_PERIOD));/* reset transmitted bytes every minute */
   CreateSysTimer(SYST_INTERFACE_UPDATE,0,ConfigInt(AUTO_INTERFACE_UPDATE));
   CreateSysTimer(SYST_BLAKOD_HOUR,60*ConfigInt(AUTO_KOD_TIME),
		  60*ConfigInt(AUTO_KOD_PERIOD));
   CreateSysTimer(SYST_SAVE,60*ConfigInt(AUTO_SAVE_TIME),
		  60*ConfigInt(AUTO_SAVE_PERIOD));
   /*
	  no garbage collection now
   CreateSysTimer(SYST_GARBAGE,60*ConfigInt(AUTO_GARBAGE_TIME),
		  60*ConfigInt(AUTO_GARBAGE_PERIOD));
	*/
}

void ProcessSysTimer(int time)
{
   systimer_node *st;

   st = systimers;

   while (st != NULL)
   {
      if (time >= st->next_time_activate)
      {
	 st->next_time_activate += st->period;
	 if (st->enabled)
	    ProcessOneSysTimer(st);
      }
      st = st->next;
   }
}

void ProcessOneSysTimer(systimer_node *st)
{
   switch (st->systimer_type)
   {
   case SYST_BLAKOD_HOUR :
      SendTopLevelBlakodMessage(GetSystemObjectID(),NEW_HOUR_MSG,0,NULL);
      break;

   case SYST_GARBAGE :
      PauseTimers();
      lprintf("ProcessOneSysTimer garbage collecting\n");
      SendBlakodBeginSystemEvent(SYSEVENT_GARBAGE);
      GarbageCollect();
      AllocateParseClientListNodes(); 
      SendBlakodEndSystemEvent(SYSEVENT_GARBAGE);
      UnpauseTimers();
      break;

   case SYST_SAVE :
      PauseTimers();
      lprintf("ProcessOneSysTimer saving\n");
      SendBlakodBeginSystemEvent(SYSEVENT_SAVE);
      GarbageCollect();
      SaveAll();
      SendBlakodEndSystemEvent(SYSEVENT_SAVE);
      AllocateParseClientListNodes(); 
      UnpauseTimers();
      break;

   case SYST_INTERFACE_UPDATE :
      InterfaceUpdate();
      break;

   case SYST_RESET_TRANSMITTED :
      if (ConfigBool(DEBUG_TRANSMITTED_BYTES))
	 dprintf("In last %i seconds, server has transmitted %i bytes.\n",
		 ConfigInt(AUTO_TRANSMITTED_PERIOD),GetTransmittedBytes());

      
      ResetTransmittedBytes();
      break;

   case SYST_RESET_POOL :
      ResetBufferPool();
      break;  
   }
}

void ForEachSysTimer(void (*callback_func)(systimer_node *st))
{
   systimer_node *st;

   st = systimers;

   while (st != NULL)
   {
      callback_func(st);
      st = st->next;
   }
}

Bool DisableSysTimer(int systimer_type)
{
   systimer_node *st;

   st = systimers;

   while (st != NULL)
   {
      if (st->systimer_type == systimer_type)
      {
	 st->enabled = False;
	 return True;
      }
      st = st->next;
   }
   return False;
}

Bool EnableSysTimer(int systimer_type)
{
   systimer_node *st;

   st = systimers;

   while (st != NULL)
   {
      if (st->systimer_type == systimer_type)
      {
	 st->enabled = True;
	 return True;
      }
      st = st->next;
   }
   return False;
}



