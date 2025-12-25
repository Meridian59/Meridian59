// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * mapfile.h:  Header file for mapfile.c
 */

#ifndef _MAPFILE_H
#define _MAPFILE_H

void MapFileInitialize(void);
void MapFileClose(void);

bool MapFileLoadRoom(room_type *room);
bool MapFileSaveRoom(room_type *room);
bool MapFileSaveRoomAnnotations(room_type *room);
#ifndef M59_RETAIL
bool MapFileValidateAllRooms();
#endif
bool MapFileFindNextKnownData(int *next_offset, int curr_offset);
bool IsValidRoomAnnotation(MapAnnotation *annotation);

#endif /* #ifndef _MAPFILE_H */
