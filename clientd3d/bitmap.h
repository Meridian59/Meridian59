// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * bitmap.h
 *
 */

#ifndef _BITMAP_H
#define _BITMAP_H


// Object and grid bitmaps start with the same fields so that they can be handled by a single
// cache handling function.

typedef struct {
   ID            idnum;
   Bitmaps       bmaps;
} *grid_bitmap_type, grid_bitmap_struct;

typedef struct {
   ID      idnum;
   Bitmaps bmaps;
} *object_bitmap_type, object_bitmap_struct;

void FreeBitmaps(void);

/* Functions to retrieve bitmaps based on ID numbers */
object_bitmap_type GetObjectBitmap(ID obj_id);
grid_bitmap_type GetGridBitmap(WORD type);
object_bitmap_type GetBackgroundBitmap(ID rsc);

#endif /* #ifndef _BITMAP_H */
