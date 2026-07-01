// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * moveobj.h:  Header file for moveobj.c
 */

#ifndef _MOVEOBJ_H
#define _MOVEOBJ_H

#define GRAVITY_ACCELERATION (- 5 * FINENESS)   // In FINENESS units / second / second

// Smooth-motion increment is the fraction of the journey covered per millisecond:
// (speed / period) / distance.  Object motion is given in # of grid squares per 10
// seconds; projectiles are given per 1 second, so the same speed value moves a
// projectile ten times as far per millisecond.
static const float PROJECTILE_INTERVAL_MS = 1000.0f;
static const float OBJECT_INTERVAL_MS     = 10.0f * PROJECTILE_INTERVAL_MS;

void MoveObject2(ID object_id, int x, int y, BYTE speed, BOOL turnToFace);
bool ObjectsMove(int dt);
bool MoveSingle(Motion *m, int dt);

#endif /* #ifndef _MOVEOBJ_H */
