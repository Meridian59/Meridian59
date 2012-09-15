// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * editbox.h:  Header for editbox.c
 */

#ifndef _EDITBOX_H
#define _EDITBOX_H

void EditBoxCreate(HWND hParent);
void EditBoxDestroy(void);
void EditBoxReset(int xsize, int ysize, AREA view);
void EditBoxResize(int xsize, int ysize, AREA view);
void EditBoxSetFocus(Bool forward);
void EditBoxDrawBorder(void);
void EditBoxGetArea(AREA *a);
HWND EditBoxWindow(void);

void EditBoxResetFont(void);
M59EXPORT void EditBoxStartAdd(void);
M59EXPORT void EditBoxEndAdd(void);
M59EXPORT void EditBoxAddText(char *message, int color, int style);
void EditBoxResetFont(void);
void EditBoxChangeColor(void);
void EditBoxSetNormalFormat(void);

#endif /* #ifndef _EDITBOX_H */
