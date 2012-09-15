// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* 
 * statterm.h
 *
 */

#ifndef _STATTERM_H
#define _STATTERM_H

#define MAX_HISTORY 30000   /* Max # of chars in admin history list */
#define MAX_ADMIN   250     /* Max length of admin command string */

void TermInit(void);
void TermExit(void);
void TermResize(HWND hwnd, UINT resize_flag, int xsize, int ysize);
void TermSetFocus(HWND hwnd, HWND hwndOldFocus);

void TermChangeFont(void);
void AddCharTerm(char *message, int num_chars);
void DoneCharsTerm(void);

#endif
