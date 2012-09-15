// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * drawint.h:  Header file for drawint.c
 */

#ifndef _DRAWINT_H
#define _DRAWINT_H

void InterfaceDrawInit(void);
void InterfaceDrawExit(void);
void InterfaceDrawResize(int xsize, int ysize, AREA *view);
void InterfaceDrawElements(HDC hdc);
//void InterfaceDrawBarBorder(HDC hdc, AREA *a);
void InterfaceDrawBarBorder( RawBitmap* prawbmpBackground, HDC hdc, AREA *a );

#endif /* #ifndef _DRAWINT_H */
