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
   BYTE   shrink;   // Divide by this to get actual size of object 
   BYTE   num_hotspots;   // # of hotspots in bitmap
   char  *numbers;  // Hotspot numbers:  > 0 is an overlay, < 0 is an underlay
   POINT *hotspots; // Hotspot positions; tell where overlay bitmaps should be placed
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
   char    name[MAX_BITMAPNAME]; /* String name of bitmap */
   int     num_bitmaps;          /* Number of bitmaps in each group */
   int     num_groups;           /* Number of bitmap groups */
   int     max_indices;          /* Max # of indices in an index group */
   PDIB   *pdibs;                /* Placeholder; points to array of pdibs */   
   int    *indices;              /* Array of indices */
} Bitmaps;

/***************************************************************************
   External function declarations
 **************************************************************************/

Bool DibInitCompression();
Bool DibCloseCompression();
Bool DibOpenFile(const char *szFile, Bitmaps *b);
void DibFree(PDIB pdib);
void BitmapsFree(Bitmaps *b);
Bool DibOpenFileSimple(const char *szFile, Bitmaps *b, BYTE *shrink, int *width, int *height);

/****************************************************************************
   Bitmaps structure macros
 ****************************************************************************/
// String name of Bitmaps structure "bmap"
#define BitmapsName(bmap) ((bmap).name)

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

/****************************************************************************
   DIB macros
 ****************************************************************************/

#define WIDTHBYTES(i)            ((unsigned)((i+31)&(~31))/8)  /* ULONG aligned ! */

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
