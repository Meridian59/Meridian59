// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * systimer.h
 *
 */

#ifndef _SYSTIMER_H
#define _SYSTIMER_H

enum
{
   SYST_GARBAGE, SYST_SAVE, SYST_BLAKOD_HOUR, SYST_INTERFACE_UPDATE,
   SYST_RESET_TRANSMITTED, SYST_RESET_POOL,
};

typedef struct systimer_struct
{
   int systimer_type;
   int time;
   int period;
   int next_time_activate;
   Bool enabled;
   struct systimer_struct *next;
} systimer_node;

void InitSysTimer();
void ResetSysTimer();
void CreateSysTimer(int type,int time,int period);
void ProcessSysTimer(int time);
void ForEachSysTimer(void (*callback_func)(systimer_node *st));
Bool DisableSysTimer(int systimer_type);
Bool EnableSysTimer(int systimer_type);

#endif
