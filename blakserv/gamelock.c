// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * gamelock.c
 *

 This module keeps the status of whether the game is locked by an
 administrator or not, and the reason if it is.

 */

#include "blakserv.h"

Bool game_lock;
char reason_str[400];

void InitGameLock()
{
   SetGameUnlock();
}

void SetGameLock(char *str)
{
   game_lock = True;
   strncpy(reason_str,str,400);
}

void SetGameUnlock()
{
   game_lock = False;
}

Bool IsGameLocked()
{
   return game_lock;
}

char * GetGameLockedReason()
{
   return reason_str;
}
