// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// bmparea.h:  Include file for bmparea.c

#ifndef _BMPAREA_H
#define _BMPAREA_H

void DrawAreaInit(void);
long CALLBACK MainButtonProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
void OffsetDrag(HWND hwnd);
void HotspotDrag(HWND hwnd);
void DoDraw(HDC hDC1, HWND hWnd);

#endif // _BMPAREA_H
