// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statstrt.h:  Header file for statstrt.c
 */

#ifndef _STATSTRT_H
#define _STATSTRT_H

void AddCharsStartup(char *message, int len);
void StartupInit(int final_state);
void AbortStartupTimer(void);

#endif /* #ifndef _STATSTRT_H */
