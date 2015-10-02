// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * move.h:  Header file for move.c
 */

#ifndef _MOVE_H
#define _MOVE_H

// Initial velocities when climbing and falling, in FINENESS units per second
#define CLIMB_VELOCITY_0   (FINENESS * 9 / 2)	// climb velocity set to be 3x faster than fall, to keep people from seeing through the floor
#define FALL_VELOCITY_0    (-FINENESS * 2 / 3)

void ResetPlayerPosition(void);

void UserMovePlayer(int action);
Bool ObjectsMove(int dt);
Bool MoveSingle(Motion *m, int dt);

void PlayerChangeHeight(int dz);
void PlayerChangeHeightMouse(int dz);
void PlayerResetHeight(void);
int  PlayerGetHeight(void);
int  PlayerGetHeightOffset(void);

void ServerMovedPlayer(void);
void MoveUpdateServer(void);
void MoveSetValidity(Bool valid);
void MoveUpdatePosition(void);

void UserTurnPlayer(int action);
void UserTurnPlayerMouse(int delta);
void UserFlipPlayer(void);

#endif /* #ifndef _MOVE_H */
