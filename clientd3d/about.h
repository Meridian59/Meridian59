// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * about.h:  Header file for about.c
 */

#ifndef _ABOUT_H
#define _ABOUT_H

#define ABOUT_RSC  19999    // Hard-coded rsc for About box graphic
#define ABOUT_RSC1 19998    // Hard-coded rsc for About box graphic #1
#define ABOUT_RSC2 19997    // Hard-coded rsc for About box graphic #2
#define ABOUT_RSC3 19996    // Hard-coded rsc for About box graphic #3

BOOL CALLBACK AboutDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
void AbortAboutDialog(void);

#endif /* #ifndef _ABOUT_H */
