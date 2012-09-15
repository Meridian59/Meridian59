// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * textin.h:  Header file for textin.c
 */

#ifndef _TEXTIN_H
#define _TEXTIN_H

#define TEXTINPUT_HEIGHT 25

#define MAXSAY 512         // Maximum length of input string
extern AREA input_area;

void TextInputCreate(HWND hParent);
void TextInputDestroy(void);
void TextInputResize(int xsize, int ysize, AREA view);
M59EXPORT void TextInputSetFocus(Bool forward);
void TextInputSetText(char *text, Bool focus);
void TextInputDrawBorder(void);
void TextInputResetFont(void);
M59EXPORT int GetTextInputHeight(void);

#endif /* #ifndef _TEXTIN_H */
