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

void MoveObject2(ID object_id, int x, int y, BYTE speed, BOOL turnToFace);
Bool ObjectsMove(int dt);
Bool MoveSingle(Motion *m, int dt);

#endif /* #ifndef _MOVEOBJ_H */
