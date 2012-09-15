// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// draw.h:  Include file for draw.c

#ifndef _DRAW_H
#define _DRAW_H

void DrawInit(void);
void DrawClose(void);
BOOL DrawBitmap(PDIB pdib, int x, int y, float shrink, BOOL transparent);
void DrawHotspot(HDC hdc, int x, int y, BOOL highlight);

void DisplayBitmaps(HDC hdc, int x, int y, int width, int height);
void ClearBitmap(void);

#endif // _DRAW_H
