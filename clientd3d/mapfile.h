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

Bool MapFileLoadRoom(room_type *room);
Bool MapFileSaveRoom(room_type *room);

#endif /* #ifndef _MAPFILE_H */
