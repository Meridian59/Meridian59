// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * timer.c
 *

 This module maintains a linked list of timers for the Blakod.  It
 also contains the main loop of the program.

 */

#include "blakserv.h"

static Bool in_main_loop = False;
static int numActiveTimers = 0;

timer_node *timers;
int next_timer_num;

timer_node *deleted_timers;

int pause_time;

/* local function prototypes */
void AddTimerNode(timer_node *t);
void StoreDeletedTimer(timer_node *t);
void ResetLastMessageTimes(session_node *s);

int  GetNumActiveTimers(void)
{
   return numActiveTimers;
}

void InitTimer(void)
{
   timers = NULL;
   next_timer_num = 0;
   numActiveTimers = 0;
   deleted_timers = NULL;
   
   pause_time = 0;
}

void ResetTimer(void)
{
   ClearTimer();
}

void ClearTimer(void)
{
   timer_node *t,*temp;

   t = timers;
   while (t != NULL)
   {
      temp = t;
      t = t->next;
      FreeMemory(MALLOC_ID_TIMER,temp,sizeof(timer_node));
   }
   timers = NULL;
   next_timer_num = 0;
   numActiveTimers = 0;

   t = deleted_timers;
   while (t != NULL)
   {
      temp = t;
      t = t->next;
      FreeMemory(MALLOC_ID_TIMER,temp,sizeof(timer_node));
   }
   deleted_timers = NULL;
}

void PauseTimers(void)
{
   if (pause_time != 0)
   {
      eprintf("PauseTimers called when they were already paused at %s\n",TimeStr(pause_time));
      return;
   }
   pause_time = GetTime();
}

void UnpauseTimers(void)
{
   int add_time;
   timer_node *t;
   
   if (pause_time == 0)
   {
      eprintf("UnpauseTimers called when they were not paused\n");
      return;
   }
   add_time = 1000*(GetTime() - pause_time);

   t = timers;
   while (t != NULL)
   {
      t->time += add_time;
      t = t->next;
   }

   pause_time = 0;
   
   /* after timers unpaused, we should reset last message times of people in the game
      so they aren't logged because of what seems to be lag */

   ForEachSession(ResetLastMessageTimes);

}

void ResetLastMessageTimes(session_node *s)
{
   if (s->state != STATE_GAME)
      return;

   s->game->game_last_message_time = GetTime();
}

void AddTimerNode(timer_node *t)
{
   timer_node *temp,*prev;

   /* insert in sorted increasing order by time */

   if (timers == NULL || timers->time > t->time)
   {
      t->next = timers; /* order is important! do this FIRST */
      timers = t;

      /* we're making a new first-timer, so the time main loop should wait might
	 have changed, so have it break out of loop and recalibrate */
      PostThreadMessage(main_thread_id,WM_BLAK_MAIN_RECALIBRATE,0,0);
      return;
   }

   temp = timers;
   do
   {
      prev = temp;
      temp = temp->next;
   } while (temp != NULL && temp->time <= t->time);
   
   t->next = temp;
   prev->next = t;

}

int CreateTimer(int object_id,int message_id,int milliseconds)
{
   timer_node *t;

   if (deleted_timers == NULL)
      t = (timer_node *)AllocateMemory(MALLOC_ID_TIMER,sizeof(timer_node));
   else
   {
      /* dprintf("recovering former timer id %i\n",deleted_timers->timer_id); */
      t = deleted_timers;
      deleted_timers = deleted_timers->next;
   }
      
   t->timer_id = next_timer_num++;
   t->object_id = object_id;
   t->message_id = message_id;
   t->time = GetMilliCount() + milliseconds;

   AddTimerNode(t);
   numActiveTimers++;

   return t->timer_id;
}

Bool LoadTimer(int timer_id,int object_id,char *message_name,int milliseconds)
{
   object_node *o;
   timer_node *t;
   message_node *m;

   o = GetObjectByID(object_id);
   if (o == NULL)
   {
      eprintf("LoadTimer can't find object %i\n",object_id);
      return False;
   }

   m = GetMessageByName(o->class_id,message_name,NULL);
   if (m == NULL) 
   {
      eprintf("LoadTimer can't find message name %s\n",message_name);
      return False;
   }

   t = (timer_node *)AllocateMemory(MALLOC_ID_TIMER,sizeof(timer_node));
   t->timer_id = timer_id;
   t->object_id = object_id;
   t->message_id = m->message_id;
   t->time = GetMilliCount() + milliseconds;

   AddTimerNode(t);
   numActiveTimers++;

   /* the timers weren't saved in numerical order, but they were
    * compacted to first x non-negative integers
    */
   if (timer_id >= next_timer_num)
      next_timer_num = timer_id + 1;

   return True;
}

void StoreDeletedTimer(timer_node *t)
{
   t->next = deleted_timers;
   deleted_timers = t;
   numActiveTimers--;
   /* dprintf("storing timer id %i\n",deleted_timers->timer_id); */
}

Bool DeleteTimer(int timer_id)
{
   timer_node *t,*prev;

   if (timers == NULL)
      return False;
   
   if (timers->timer_id == timer_id)
   {
      t = timers->next;

      /* put deleted timer on deleted_timer list */
      StoreDeletedTimer(timers);

      timers = t;
      return True;
   }

   prev = timers;
   t = timers->next;
   while (t != NULL)
   {
      if (t->timer_id == timer_id)
      {
	 prev->next = t->next;

	 /* put deleted timer on deleted_timer list */
	 StoreDeletedTimer(t);

	 return True;
      }
      prev = t;
      t = t->next;
   }

   bprintf("DeleteTimer can't find timer %i\n",timer_id);

#if 0
   // list the active timers.
   t = timers;
   while (t != NULL)
   {
      dprintf("%i ",t->timer_id);
      t = t->next;
   }
   dprintf("\n");
#endif
   return False;
}

/* activate the 1st timer, if it is time */
void TimerActivate()
{
   timer_node *temp;
   int object_id,message_id;
   UINT64 now;
   val_type timer_val;
   parm_node p[1];
   
   if (timers == NULL)
      return;
   
   now = GetMilliCount();
   if (now > timers->time)
   {
	/*
     if (now - timers->time > TIMER_DELAY_WARN)
       dprintf("Timer handled %i.%03is late\n",
	       (now-timers->time)/1000,(now-timers->time)%1000);
	*/

      object_id = timers->object_id;
      message_id = timers->message_id;
      
      timer_val.v.tag = TAG_TIMER;
      timer_val.v.data = timers->timer_id;
      
      p[0].type = CONSTANT;
      p[0].value = timer_val.int_val;
      p[0].name_id = TIMER_PARM;
      
      temp = timers;
      timers = timers->next;
      
      /* put deleted timer on deleted_timer list */
      StoreDeletedTimer(temp);
      
      SendTopLevelBlakodMessage(object_id,message_id,1,p);
   }
}

Bool InMainLoop(void)
{
   return in_main_loop;
}

void ServiceTimers(void)
{
   MSG msg;
   INT64 ms;

   StartupComplete(); /* for the interface to report no errors on startup */
   InterfaceUpdate();
   lprintf("Status: %i accounts\n",GetNextAccountID());

   lprintf("-------------------------------------------------------------------------------------\n");
   dprintf("-------------------------------------------------------------------------------------\n");
   eprintf("-------------------------------------------------------------------------------------\n");

   in_main_loop = True;
   SetWindowText(hwndMain, ConfigStr(CONSOLE_CAPTION));

	AsyncSocketStart();

   for(;;)
   {
      if (timers == NULL)
			ms = 500;
      else
      {
			ms = timers->time - GetMilliCount();
			if (ms <= 0)
				ms = 0;
	 
			if (ms > 500)
				ms = 500;
      }	 
      
      if (MsgWaitForMultipleObjects(0,NULL,0,(DWORD)ms,QS_ALLINPUT) == WAIT_OBJECT_0)
      {
	 while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	 {
	    if (msg.message == WM_QUIT)
	    {
	       lprintf("ServiceTimers shutting down the server\n");   
	       return;
	    }
	    
	    switch (msg.message)
	    {
	    case WM_BLAK_MAIN_READ :
	       EnterServerLock();
	       
	       PollSession(msg.lParam);
	       TimerActivate();
	       
	       LeaveServerLock();
	       break;
	    case WM_BLAK_MAIN_RECALIBRATE :
	       /* new soonest timer, so we should recalculate our time left... 
		  so we just need to restart the loop! */
	       break;
	    case WM_BLAK_MAIN_DELETE_ACCOUNT :
	       EnterServerLock();
	       DeleteAccountAndAssociatedUsersByID(msg.lParam);
	       LeaveServerLock();
	       break;

	    case WM_BLAK_MAIN_VERIFIED_LOGIN :
	       EnterServerLock();
	       VerifiedLoginSession(msg.lParam);
	       LeaveServerLock();
	       break;
       case WM_BLAK_MAIN_LOAD_GAME :
          EnterServerLock();
          LoadFromKod(msg.lParam);
          LeaveServerLock();
          break;

	    default :
	       dprintf("ServiceTimers got unknown message %i\n",msg.message);
	       break;
	    }
	 }
      }
      else
      {
	 /* a Blakod timer is ready to go */
	 
	 EnterServerLock();
	 PollSessions(); /* really just need to check session timers */
	 TimerActivate();
	 LeaveServerLock();
      }
   }
}

timer_node * GetTimerByID(int timer_id)
{
   timer_node *t;

   t = timers;
   while (t != NULL)
   {
      if (t->timer_id == timer_id)
	 return t;
      t = t->next;
   }
   return NULL;
}

void ForEachTimer(void (*callback_func)(timer_node *t))
{
   timer_node *t;

   t = timers;
   while (t != NULL)
   {
      callback_func(t);
      t = t->next;
   }
}

/* functions for garbage collection */

void SetNumTimers(int new_next_timer_num)
{
   next_timer_num = new_next_timer_num;
}
