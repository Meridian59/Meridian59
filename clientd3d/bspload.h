// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * bspload.h:  Header file for bspload.c
 */

#ifndef _BSPLOAD_H
#define _BSPLOAD_H

Bool BSPRooFileLoad(char *fname, room_type *room);
void BSPRoomFree(room_type *room);

void BSPDumpTree(BSPnode *tree, int level);
int BSPGetNumWalls(void);

void SetWallHeights(WallData *wall);

void RoomSetupCycleAnimation(RoomAnimate *ra, BYTE speed);
void RoomSetupScrollAnimation(RoomAnimate *ra, int period, BYTE direction);
void RoomSetupFlickerAnimation(RoomAnimate *ra, BYTE original_light, WORD server_id);

#endif /* #ifndef _BSPLOAD_H */
