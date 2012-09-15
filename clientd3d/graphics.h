// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * graphics.h
 *
 */

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#define MIN_WINDOW_WIDTH  390
#define MIN_WINDOW_HEIGHT 380

#define MIN_TOP_TOOLBAR 6      // Minimum height of top area with toolbar present
#define MIN_TOP_NOTOOLBAR 26   // Minimum height of top area without toolbar present

M59EXPORT void CopyCurrentView(AREA *v);
M59EXPORT void CopyCurrentAreaMiniMap( AREA* v );

void ResizeAll(void);
M59EXPORT Bool MouseToRoom(int *x, int *y);
Bool TranslateToRoom(int client_x, int client_y, int *x, int *y);

//	ajw
Bool MouseToMiniMap( int* x, int* y );

M59EXPORT void RedrawAll(void);
Bool NeedRedraw(void);
void RedrawForce(void);

void GraphicsAreaCreate(HWND hwnd);
void GraphicsAreaDestroy();
void GraphicsAreaRedraw(HDC hdc);
void GraphicsSizeRoom(room_type room);
void GraphicsAreaResize(int xsize, int ysize);
void GraphicsToggleMap(void);
void DrawGridBorder(void);
Bool MapVisible(void);
Bool GraphicsReleaseCapture(void);

void UserMouseMove(void);
void UserStartDrag(void);
void UserEndDrag(void);
Bool GraphicsMouseCaptured(void);
int GetFPS(void);
int GetMSDrawFrame(void);

#endif

