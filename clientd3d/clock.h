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

// Sets up baseline clock snapshot.
// Called within WinMain() in client.c right before entering primary game update loop.
void InitializeTime();

// Refreshes clock snapshot for the current frame.
// Called every game loop at the very start of GameIdle() in statgame.c.
void UpdateTime();

#endif	/* #ifndef _CLOCK_H */
