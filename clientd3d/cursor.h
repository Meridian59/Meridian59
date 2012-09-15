// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * cursor.h:  Header file for cursor.c
 */

#ifndef _CURSOR_H
#define _CURSOR_H

BOOL MainSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg);
M59EXPORT void SetMainCursor(HCURSOR cursor);
void UserMoveCursor(int command, int dy, int dx);
void UserMoveEsc(void);

M59EXPORT BOOL GameWindowSetCursor(void);

#endif /* #ifndef _CURSOR_H */

