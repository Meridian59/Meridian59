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

// Initializes the clock state. Should be called once before the clock is first refreshed.
void InitializeTime();

// Refreshes the clock state. Should be called once at the start of each frame.
void UpdateTime();

#endif	/* #ifndef _CLOCK_H */
