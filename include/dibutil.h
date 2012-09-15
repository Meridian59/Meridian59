// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * dibutil.h:  Header file for dibutil.c
 */

#ifndef _DIBUTIL_H
#define _DIBUTIL_H

#define MAX_BITMAPNAME 32

typedef struct {
   int    width;
   int    height;
   int    xoffset;  // X offset to use when displaying this bitmap over another one
   int    yoffset;  // Y offset to use when displaying this bitmap over another one
   POINT *hotspots; // Hotspot positions; tell where overlay bitmaps should be placed
   unsigned int	uniqueID;	// unique identifier to identify this bitmap, even if pointer changes
   unsigned int	uniqueID2;	// unique identifier to identify this bitmap, even if pointer changes
   char		frame;			// indicates which bitmap in group this particular dib is
   BYTE   shrink;   // Divide by this to get actual size of object 
   BYTE   num_hotspots;   // # of hotspots in bitmap
   char  *numbers;  // Hotspot numbers:  > 0 is an overlay, < 0 is an underlay
} DIBHEADER, *PDIB;

// The Bitmaps structure stores all the bitmaps for a single object.  The bitmaps are 
// divided into groups; one group is active for a given object at any one time.  This
// is like the state of the object, i.e. one group of bitmaps for walking, one for 
// exploding, etc.  The bitmaps within a group are meant to show the object at different angles.
//
// The bitmaps are stored contiguously, and the groups are composed of indices into the
// array of bitmaps.  The groups are stored as a 2D array; there is one row for each group,
// and as many columns as the maximum number of bitmaps in any group.  Column 0 tells
// how many bitmaps are in that row's group.
//
// In order to allow objects of various resolutions, the "shrink" element tells how to
// determine the object's actual size from its bitmap size.  When shrink=1, the bitmap
// is drawn 1:1.  When shrink is higher, the object has more bitmap pixels per screen
// pixels, i.e. it has higher resolution.
//

typedef struct {
   int     num_bitmaps;          /* Number of bitmaps in pdibs */
   int     num_groups;           /* Number of bitmap groups */
   int     max_indices;          /* Max # of indices in an index group */
   PDIB   *pdibs;                /* Placeholder; points to array of pdibs */   
   int    *indices;              /* Array of indices */
   int     size;                 /* Estimate of memory taken up by all bitmaps */
} Bitmaps;

/***************************************************************************
   External function declarations
 **************************************************************************/

Bool DibOpenFile(char *szFile, Bitmaps *b);
void DibFree(PDIB pdib);
void BitmapsFree(Bitmaps *b);

/****************************************************************************
   Bitmaps structure macros
 ****************************************************************************/
// Number of bitmap groups in Bitmaps structure "bmap"
#define BitmapsNumGroups(bmap) ((bmap).num_groups)

// Maximum number of indices in a single bitmap group
#define BitmapsArraySize(bmap) ((bmap).max_indices)

// Offset of first index in group "group"
#define BitmapsGroupFirst(bmap, group) (BitmapsArraySize(bmap) * (group))

// Number of indices in a bitmap group "group"
#define BitmapsInGroup(bmap, group) ((bmap).indices[BitmapsGroupFirst(bmap, group)])

// Index of "offset"-th PDIB in group "group"
#define BitmapsGetIndex(bmap, group, offset) \
   ((bmap).indices[ BitmapsGroupFirst(bmap, group) + (offset) + 1])

// "offset"-th PDIB in group "group"
#define BitmapsGetPdib(bmap, group, offset) ((bmap).pdibs[BitmapsGetIndex(bmap, group, offset)])

// "index"-th PDIB in bitmap structure
#define BitmapsGetPdibByIndex(bmap, index) ((bmap).pdibs[index])

// Index that means that there's no bitmap at this place in structure (used for overlays)
#define BITMAP_MISSING  (-1)

// Estimate of total size of structure + bitmaps
#define BitmapsTotalSize(bmap) ((bmap).size)

/****************************************************************************
   DIB macros
 ****************************************************************************/

#define WIDTHBYTES(i)            ((unsigned)((i+31)&(~31))/8)  /* ULONG aligned ! */
#define DIBWIDTH(x)              (((x) + 3) & (~3))            // 4 byte aligned

#define DibWidth(lpbi)            ((lpbi)->width)
#define DibHeight(lpbi)           ((lpbi)->height)
#define DibXOffset(lpbi)          ((lpbi)->xoffset)
#define DibYOffset(lpbi)          ((lpbi)->yoffset)
#define DibNumHotspots(lpbi)      ((lpbi)->num_hotspots)
#define DibHotspotNumber(lpbi, n) ((lpbi)->numbers[n])
#define DibHotspotIndex(lpbi, n)  ((lpbi)->hotspots[n])
#define DibShrinkFactor(lpbi)     ((lpbi)->shrink)


#define DibSizeImage(lpbi)       (DibWidth(lpbi) * DibHeight(lpbi))

#define DibSize(lpbi)            (DibSizeImage(lpbi) + sizeof(DIBHEADER))

// Get bits of bitmap from PDIB
#define DibPtr(lpbi)             ((LPBYTE)(lpbi) + sizeof(DIBHEADER))

/***************************************************************************/


#endif /* #ifndef _DIBUTIL_H */
