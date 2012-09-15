// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * draw.h: Header for draw.c
 */

#ifndef _DRAW_H
#define _DRAW_H

// Palette indices for borders
#define BORDER_INDEX 79
//#define HIGHLIGHT_INDEX 251
// new border colors for grey background in new client - mistery
#define HIGHLIGHT_INDEX 195

Bool DrawInitialize(void);
void DrawClose(void);
void DrawChangeColor(void);

PDIB GetObjectPdib(ID id, long angle, int group);
PDIB GetGridPdib(WORD grid_id);
PDIB GetBackgroundPdib(DWORD rsc);

void DrawRoom(HDC hdc, int x, int y, room_type *room, Bool map);

//	ajw For added DrawBorder functionality. (Allows corners of area to be excluded from border drawing.)
typedef struct DRAWBORDEREXCLUDETAG
{
	int iTopEdge_Left;
	int iTopEdge_Right;
	int iBottomEdge_Left;
	int iBottomEdge_Right;
	int iLeftEdge_Top;
	int iLeftEdge_Bottom;
	int iRightEdge_Top;
	int iRightEdge_Bottom;
} DRAWBORDEREXCLUDE;

M59EXPORT void DrawBorder( AREA *area, int index, DRAWBORDEREXCLUDE* pExclude );
M59EXPORT void DrawWindowBackgroundBorder( RawBitmap *bg, HDC hdc, AREA *area, int thickness, int xin, int yin, int index, DRAWBORDEREXCLUDE* pExclude );

M59EXPORT HDC CreateMemBitmap(int width, int height, HBITMAP *gOldBitmap, BYTE **gBits);

void DrawTransBytes(BYTE *dest, const BYTE *src, int count);
void DrawTransOffetDest(BYTE *dest, const BYTE *src, int count);

#endif /* #ifndef _DRAW_H */
