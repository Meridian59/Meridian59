// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statbtn.h:  Header file for statbtn.c
 */

#ifndef _STATBTN_H
#define _STATBTN_H

void StatButtonsCreate(void);
void StatButtonsDestroy(void);
void StatsSetButtons(int num_groups);
void StatsDestroyButtons(void);
void StatsMoveButtons(void);
void StatsSetButtonFocus(int group);

Bool StatInputKey(HWND hwnd, UINT key, Bool fDown, int cRepeat, UINT flags);

int  StatsGetButtonBorder(void);
Bool StatButtonDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);
void StatButtonCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

int StatsFindGroupByHwnd(HWND hwnd);

#endif /* #ifndef _STATBTN_H */
