// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * ping.h:  Header file for ping.c
 */

#ifndef _PING_H
#define _PING_H

void PingTimerStart(void);
void PingTimerAbort(void);
void PingGotReply(void);
void CALLBACK PingTimerProc(HWND hwnd, UINT msg, UINT timer, DWORD dwTime);

#endif /* #ifndef _PING_H */
