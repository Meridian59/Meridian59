// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * map.h:  Header file for map.c
 */

#ifndef _MAP_H
#define _MAP_H

void MapInitialize(void);
void MapClose(void);

void MapEnterRoom(room_type *room);
void MapDraw( HDC hdc, BYTE *bits, AREA *area, room_type *room, int width, Bool bMiniMap );
void MapZoom(int direction);
void MapScreenToRoom( int *x, int *y, Bool bMiniMap );
void MapMiniSizeChanged(AREA *newArea);

void MapEnterRoom(room_type *room);
void MapExitRoom(room_type *room);

void DrawMap( room_type *room, Draw3DParams *params, Bool bMiniMap );
M59EXPORT void MapShowAllWalls(room_type *room, Bool show);
void PrintMap(BOOL useDefault);

#endif /* #ifndef _MAP_H */
