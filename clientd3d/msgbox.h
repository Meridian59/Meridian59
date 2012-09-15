// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * msgbox.h:  Header file for msgbox.c
 */

#ifndef _MSGBOX_H
#define _MSGBOX_H

void _cdecl Info(HINSTANCE hModule, HWND hParent, int fmt_id, ...);
int ClientMessageBox(HWND hwndParent, char *text, char *title, UINT style);

M59EXPORT void __cdecl ClientError(HINSTANCE hModule, HWND hParent, int fmt_id, ...);
M59EXPORT Bool _cdecl AreYouSure(HINSTANCE hInst, HWND hParent, int defbutton, int fmt_id, ...);

enum { YES_BUTTON = 1, NO_BUTTON = 2 };  

#endif /* #ifndef _MSGBOX_H */
