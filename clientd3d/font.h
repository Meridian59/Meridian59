// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * font.h:  Header file for font.c
 */

#ifndef _FONT_H
#define _FONT_H

/* Font identifiers */
enum { 
   FONT_LIST = 0,           /* For object list boxes */
   FONT_TITLES,             /* For titles of objects, large text */
   FONT_EDIT,               /* For main window's text edit box */
   FONT_MAIL,               /* For mail message text */
   FONT_STATS,              /* For game statistics */
   FONT_INPUT,              /* For input edit boxes */
   FONT_SYSMSG,             /* For system message */
   FONT_LABELS,             /* For labels in graphics area */
   FONT_ADMIN,              /* For admin mode */
   FONT_STATNUM,            /* For numbers in statistic graph bars */
   FONT_MAP_TITLE,
   FONT_MAP_LABEL,
   FONT_MAP_TEXT,
   MAXFONTS,
};

void FontsCreate(Bool use_defaults);
void FontsDestroy(void);
M59EXPORT HFONT GetFont(WORD font);
void FontsSave(void);
void FontsRestoreDefaults(void);

void UserSelectFont(WORD font);
M59EXPORT int GetFontHeight(HFONT hFont);
LOGFONT *GetLogfont(int fontNum);

// Return a new font that's the given font scaled by the given factor.
HFONT FontsGetScaledFont(HFONT hFont, float scale);

#endif /* #ifndef _FONT_H */
