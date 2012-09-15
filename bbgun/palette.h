// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// palette.h:  Include file for palette.c

#ifndef _PALETTE_H
#define _PALETTE_H

typedef struct {
   BYTE red, green, blue;
} Color;

HPALETTE InitializePalette(void);
void     SetDIBPalette(HDC gdc);
COLORREF GetNearestPaletteColor(COLORREF c);

extern HPALETTE hPal;                       /* our standard drawing palette */

#endif // _PALETTE_H
