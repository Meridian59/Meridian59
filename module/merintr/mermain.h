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
   Bool   resting;        // True when player is resting (idle)
   int    vigor;          // Current value of player's vigor stat
   Bool   aggressive;     // True when people allowed to kill others (safety off)
   Bool   tempsafe;       // True when player gets a temporary angel on death
   Bool   grouping;       // True when player will join building groups
   Bool   autoloot;       // True when player autoloots items
   Bool   autocombine;    // True when player autocombines spellitems
   Bool   reagentbag;     // True when player auto-stashes items in reagent bag
   Bool   spellpower;     // True when player has enabled spellpower display
} PInfo;

typedef struct {
   object_node obj;
   BYTE        num_targets;
   BYTE        school;
} spelltemp;     /* A magical spell--must have obj struct to use in owner drawn list box */

void InterfaceInit(void);
void InterfaceExit(void);

void InterfaceResizeModule(int xsize, int ysize, AREA *view);
void InterfaceRedrawModule(HDC hdc);
Bool InterfaceDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);

void InterfaceFontChanged(WORD font_id, LOGFONT *font);
void InterfaceColorChanged(WORD color_id, COLORREF color);
void InterfaceResetData(void);
Bool InterfaceTab(int control, Bool forward);
Bool InterfaceAction(int action, void *action_data);
Bool CheckForAlwaysActiveSpells(spelltemp *sp);
void InterfaceNewRoom(void);
void InterfaceConfigChanged(void);
void InterfaceUserChanged(void);

char *GetPlayerName(char *str, char **next);
ID FindPlayerByName(char *name);
ID FindPlayerByNameExact(char *name);

extern PInfo pinfo;

#if 0
#define MAP_INDEX  4                 // Index of "map" command in toolbar list
#endif
#define REST_INDEX 3                 // Index of "rest" command in toolbar list
extern AddButton default_buttons[];  // Toolbar button info

#endif /* #ifndef _MERMAIN_H */
