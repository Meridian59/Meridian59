// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * gamelock.h
 *
 */

#ifndef _GAMELOCK_H
#define _GAMELOCK_H

void InitGameLock(void);
void SetGameLock(char *str);
void SetGameUnlock(void);
Bool IsGameLocked(void);
char * GetGameLockedReason(void);

#endif

