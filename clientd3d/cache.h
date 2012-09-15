// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * cache.h:  Header file for cache.c
 */

#ifndef _CACHE_H
#define _CACHE_H

object_bitmap_type FindObjectBitmap(ID rsc);
grid_bitmap_type   FindGridBitmap(WORD grid_num);

void CacheInitialize(void);
void CacheAddObject(object_bitmap_type new_bmap);
void CacheAddGrid(grid_bitmap_type new_bmap);
void CacheReport(void);
void CacheClearAll(void);
list_type CacheGetGridList(void);


#endif /* #ifndef _CACHE_H */
