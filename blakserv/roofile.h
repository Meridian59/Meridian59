// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * roofile.h
 *
 */

#ifndef _ROOFILE_H
#define _ROOFILE_H

// switch to 1 to enable debug output for BSP LOS
#define DEBUGLOS 0

#define ROO_VERSION     14
#define ROO_SIGNATURE   0xB14F4F52
#define OBJECTHEIGHTROO 768
#define ROOFINENESS     1024.0f

#define FINENESSKODTOROO(x) ((x) * 16.0f)
#define FINENESSROOTOKOD(x) ((x) * 0.0625f)

#define FLOATTOKODINT(x) \
   (((x) > (float)MAX_KOD_INT) ? MAX_KOD_INT : (((x) < (float)-MIN_KOD_INT) ? -MIN_KOD_INT : (int)x))

float BSPGetHeight(room_type* Room, V2* P, bool Floor, bool WithDepth);
bool BSPLineOfSight(room_type* Room, V3* S, V3* E);
void BSPChangeTexture(room_type* Room, unsigned int ServerID, unsigned short NewTexture, unsigned int Flags);
void BSPMoveSector(room_type* Room, unsigned int ServerID, bool Floor, float Height, float Speed);

Bool BSPRooFileLoadServer(char *fname, room_type *room);
void BSPRoomFreeServer(room_type *room);

#endif
