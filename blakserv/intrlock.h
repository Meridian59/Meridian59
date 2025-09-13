// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * intrlock.h
 *
 */

#ifndef _INTRLOCK_H
#define _INTRLOCK_H

void InitInterfaceLocks(void);

void EnterServerLock(void);
bool TryEnterServerLock(void);
void LeaveServerLock(void);

void SetQuit(void);
bool GetQuit(void);

void SignalSession(int session_id);

#endif
