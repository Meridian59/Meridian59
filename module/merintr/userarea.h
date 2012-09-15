// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * userarea.h:  Header file for userarea.c
 */

#ifndef _USERAREA_H
#define _USERAREA_H

#define IDC_USERAREA 5001

/* Size to draw user's face in client area */
#define USERAREA_WIDTH  64
#define USERAREA_HEIGHT 64

void UserAreaCreate(void);
void UserAreaDestroy(void);
void UserAreaRedraw(void);
void UserAreaResize(int xsize, int ysize, AREA *view);

#endif /* #ifndef _USERAREA_H */
