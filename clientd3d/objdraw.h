// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * objdraw.h:  Header file for objdraw.c
 */

#ifndef _OBJDRAW_H
#define _OBJDRAW_H

// Structure passed to inner loops
typedef struct {
   int          flags;             // Boolean object flags.
   BYTE         drawingtype;      // Object flags for drawing effects (invisibility, lighting type etc.)
   int          minimapflags;      // Minimap dot color flags
   unsigned int namecolor;         // Player name color flags
   object_type  objecttype;        // Enum of object type (i.e. outlaw, murderer, NPC)
   moveon_type  moveontype;        // MoveOn type of the object
   BYTE         translation;       // Palette translation to use
   BYTE         secondtranslation; // Another palette translation to use
   BYTE         *start_ptr;        // Points to start of row to draw in
   BYTE         *end_ptr;          // Points to last pixel to draw in
   BYTE         *obj_bits;         // Points to current row of bitmap to draw
   int          row;               // Row of frame buffer we're drawing into
   int          x;                 // The x coordinate on the offscreen buffer, in fixed point
   int          xinc;              // The x increment in the object bitmap per screen pixel, in fixed point
   BYTE         *palette;          // Palette to indirect through; provides light level
   int          xsize;             // x size of offscreen buffer in pixels
   int          ysize;             // y size of offscreen buffer in pixels
} ObjectRowData;

typedef void (*DrawingLoop)(ObjectRowData *d);

void DrawTargetHalo( ObjectRowData* d, BYTE* obj_bits, long y, long yinc, long bitmap_width, Bool bTopRow, Bool bBottomRow );
void DrawTargetEffect( ObjectRowData* d );		//	ajw

#endif /* #ifndef _OBJDRAW_H */
