// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * lagbox.h:  Header for lagbox.c
 */

#ifndef _LAGBOX_H
#define _LAGBOX_H

/***************************************************************************/

#define LAGBOXICON_RSC 19995 // Hard-coded rsc for graphic of lagbox
#define LAGBOXNAME_RSC 19994 // Hard-coded rsc for name of lagbox

BOOL Lagbox_Create();
void Lagbox_Destroy();
DWORD Lagbox_GetLatency(void);
void Lagbox_Reposition();
void Lagbox_Update(DWORD dwLatency);
BOOL Lagbox_OnDrawItem(HWND hWnd, const DRAWITEMSTRUCT* lpdis);
void Lagbox_Command(HWND hWnd, int id, HWND hwndCtrl, UINT uNotify);
void Lagbox_Animate(int dt);
M59EXPORT void Lagbox_GetRect(LPRECT lpRect);

/***************************************************************************/

#endif /* #ifndef _LAGBOX_H */
