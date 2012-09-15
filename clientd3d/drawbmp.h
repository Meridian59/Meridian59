// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * drawbmp.h: Header for drawbmp.c
 */

#ifndef _DRAWBMP_H
#define _DRAWBMP_H

void DrawBitmapInitialize(void);
void DrawBitmapClose(void);

M59EXPORT void DrawStretchedObjectDefault(HDC hdc, object_node *obj, AREA *area, HBRUSH brush);
M59EXPORT void DrawStretchedOverlays(HDC hdc, object_node *obj, AREA *area, HBRUSH brush);
M59EXPORT void DrawStretchedObjectGroup(HDC hdc, object_node *obj, int group, AREA *area, HBRUSH brush);
M59EXPORT void DrawStretchedOverlayRange(HDC hdc, object_node *obj, AREA *area, HBRUSH brush, 
					int low, int high);

void CreateWindowBackground(void);
M59EXPORT void DrawWindowBackgroundColor(RawBitmap *bg, HDC hdc, RECT *rect, int xin, int yin, int index);
void DrawWindowBackgroundMem(RawBitmap *bg, BYTE *ptr, RECT *r, int block_width, int xin, int yin);
M59EXPORT void OffscreenWindowBackground(RawBitmap *bg, int xin, int yin, int width, int height);
M59EXPORT void OffscreenWindowColor(int width, int height, int index);
void DrawUserBitmap(void);

#define DrawWindowBackground(hdc, rect, xin, yin) \
DrawWindowBackgroundColor(NULL, hdc, rect, xin, yin, -1)

// Options to OffscreenBitBlt
#define OBB_FLIP         0x00000001        // Flip bitmap vertically (for Windows BMPs)
#define OBB_TRANSPARENT  0x00000002        // Mask transparency
#define OBB_COPY         0x00000004        // Copy result from offscreen bitmap to screen

M59EXPORT void OffscreenBitBlt(HDC hdc, int dest_x, int dest_y, int width, int height,
		     BYTE *bits, int source_x, int source_y, int source_width, int options);
M59EXPORT void OffscreenStretchBlt(HDC hdc, int dest_x, int dest_y, int dest_width, int dest_height,
			 BYTE *bits, int source_x, int source_y, int source_width, int source_height,
			 int options);
M59EXPORT void DrawObject(HDC hdc, object_node *obj, int group, Bool draw_obj, AREA *area, HBRUSH brush,
			int x, int y, int angle, Bool copy);
M59EXPORT void DrawObjectIcon(HDC hdc, ID icon, int group, Bool draw_obj, AREA *area, HBRUSH brush,
			int x, int y, Bool copy);
M59EXPORT void OffscreenCopy(HDC hdc, int dest_x, int dest_y, int width, int height, 
		   int source_x, int source_y);

void BitCopy( BYTE* dest_bits, int dest_bits_width, int dest_x, int dest_y, int width, int height,
			     BYTE *bits, int source_x, int source_y, int source_width, int options);

#endif /* #ifndef _DRAWBMP_H */
