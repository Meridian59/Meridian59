// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * util.h:  Header for util.c
 */

#ifndef _UTIL_H
#define _UTIL_H

void CenterWindow(HWND hwnd, HWND hwndParent);
void ClearMessageQueue(void);
char *GetString(HMODULE hModule, int idnum);

#endif _UTIL_H
