// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * winmenu.h:  Header file for winmenu.c
 */

#ifndef _WINMENU_H
#define _WINMENU_H

void EnableGameMenus(void);
void DisableGameMenus(void);

void MenuDisplaySettings(HWND hwnd);
void MenuConnect(void);
void MenuDisconnect(void);
void MenuCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

#endif /* #ifndef _WINMENU_H */
