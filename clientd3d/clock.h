// Meridian 59, Copyright 1994-2026 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

#ifndef _CLOCK_H
#define _CLOCK_H

// Returns delta time in seconds.
float GetDeltaTime();
// Returns delta time in milliseconds for legacy systems that still use milliseconds.
int GetDeltaTime_Ms();

void InitializeTime();
void UpdateTime();

#endif	/* #ifndef _CLOCK_H */
