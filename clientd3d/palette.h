// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * palette.h:  Header file for palette.c
 */

#ifndef _PALETTE_H
#define _PALETTE_H

typedef struct {
   BYTE red, green, blue;
} Color;

void        ClearSystemPalette(void);
HPALETTE    InitializePalette(void);
void        SetDIBPalette(HDC gdc);
COLORREF    GetNearestPaletteColor(COLORREF c);
int         GetClosestPaletteIndex(COLORREF c);
void        PaletteActivate(Bool is_foreground);
void        PaletteDeactivate(void);
void		SetBMIColors( BITMAPINFO* pbmInfo );


#endif /* #ifndef _PALETTE_H */
