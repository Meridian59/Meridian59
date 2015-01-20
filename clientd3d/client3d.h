// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * client3d.h:  Header file for client3d.c
 */

#ifndef _CLIENT3D_H
#define _CLIENT3D_H

#define NO_COORD_CHECK -1

int  GetPointFloor(int x, int y);
int  GetPointCeiling(int x, int y);
int GetFloorBase(int x, int y);
Bool GetPointHeights(int x, int y, int *floor, int *ceiling);
PDIB GetPointCeilingTexture(int x, int y);
PDIB GetPointFloorTexture(int x, int y);
int GetPointDepth(int x, int y);

M59EXPORT list_type    GetObjects3D(int x, int y, int distance, int pos_flags, int neg_flags,
                                    BYTE pos_drawingtype, BYTE neg_drawingtype);
M59EXPORT room_contents_node *GetObjectByPosition(int x, int y, int distance, int pos_flags, int neg_flags);
int          GetVisibleObjects(ObjectRange **objs);
ObjectRange *FindVisibleObjectById(ID obj_id);

#endif /* #ifndef _CLIENT3D_H */
