// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * mermain.h:  Header file for mermain.c
 */

#ifndef _MERMAIN_H
#define _MERMAIN_H

// Hold player/game data that's known only in module
typedef struct {
   bool   resting;        // True when player is resting (idle)
   int    vigor;          // Current value of player's vigor stat
   bool   aggressive;     // True when people allowed to kill others (safety off)
} PInfo;

void InterfaceInit(void);
void InterfaceExit(void);

void InterfaceResizeModule(int xsize, int ysize, AREA *view);
void InterfaceRedrawModule(HDC hdc);
bool InterfaceDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);

void InterfaceFontChanged(WORD font_id, LOGFONT *font);
void InterfaceColorChanged(WORD color_id, COLORREF color);
void InterfaceResetData(void);
bool InterfaceTab(int control, bool forward);
bool InterfaceAction(int action, void *action_data);
void InterfaceNewRoom(void);
void InterfaceConfigChanged(void);
void InterfaceUserChanged(void);

char *GetPlayerName(char *str, char **next);
ID FindPlayerByName(char *name);
ID FindPlayerByNameExact(char *name);

extern PInfo pinfo;

#define REST_INDEX 3                 // Index of "rest" command in toolbar list
extern AddButton default_buttons[];  // Toolbar button info

#endif /* #ifndef _MERMAIN_H */
