// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * color.h:  Header file for color.c
 */

#ifndef _COLOR_H
#define _COLOR_H

/* Color identifiers */
enum { 
   COLOR_BGD = 0,         /* Main window background */
   COLOR_FGD,             /* Main window foreground */
   COLOR_LISTSELBGD,      /* Selected list item background */
   COLOR_LISTSELFGD,      /* Selected list item foreground */
   COLOR_MAILBGD,         /* Mail background */
   COLOR_MAILFGD,         /* Mail foreground */
   COLOR_HIGHLITE,        /* Main highlight rectangle */
   COLOR_EDITFGD,         /* Main text area foreground */
   COLOR_EDITBGD,         /* Main text area background */
   COLOR_SYSMSGFGD,       /* System messages foreground */
   COLOR_MAINEDITFGD,     /* Main edit box foreground */
   COLOR_MAINEDITBGD,     /* Main edit box background */
   COLOR_LISTFGD,         /* List box foreground */
   COLOR_LISTBGD,         /* List box background */
   COLOR_RMMSGFGD,        /* Room title foreground */
   COLOR_RMMSGBGD,        /* Room title background */
   COLOR_STATSFGD,        /* Statistics foreground */
   COLOR_STATSBGD,        /* Statistics background */
   COLOR_BAR1,            /* Bar graph color #1 (normal bars in graphs) */
   COLOR_BAR2,            /* Bar graph color #2 (limit bars in graphs) */
   COLOR_BAR3,            /* Bar graph color #3 (background in graphs) */
   COLOR_BAR4,            /* Bar graph color #4 (numbers in graphs) */
   COLOR_INVNUMFGD,       /* Inventory number foreground */
   COLOR_INVNUMBGD,       /* Inventory number background */
   COLOR_ITEM_MAGIC_FG,   /* Color for magic weapons in lists*/
   MAXCOLORS,
};

/* Colors of owner-drawn list boxes */
enum { UNSEL_FGD, UNSEL_BGD, SEL_FGD, SEL_BGD };

// Turn normal RGB color into palette-relative RGB color
#define MAKEPALETTERGB(c) ((c) | 0x02000000)

void ColorsCreate(Bool use_defaults);
void ColorsDestroy(void);
M59EXPORT COLORREF GetColor(WORD color);
M59EXPORT HBRUSH GetBrush(WORD color);
COLORREF GetPlayerNameColor(int flags,char*name);
COLORREF GetPlayerWhoNameColor(int flags,char*name);

void UserSelectColor(WORD color);
void UserSelectColors(WORD fg, WORD bg);
void ColorsSave(void);
void ColorsRestoreDefaults(void);

M59EXPORT HBRUSH DialogCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);
HBRUSH MainCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);

WORD GetItemListColor(HWND hwnd, int type, int flags);

#endif /* #ifndef _COLOR_H */
