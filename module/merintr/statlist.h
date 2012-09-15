// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statlist.h:  Header file for statlist.c
 */

#ifndef _STATLIST_H
#define _STATLIST_H

void StatsListCreate(list_type stats);
void StatsListDestroy(list_type stats);
void StatsListResize(list_type stats);
void StatsListChangeStat(Statistic *s);

void StatsListMeasureItem(HWND hwnd, MEASUREITEMSTRUCT *lpmis);
BOOL StatsListDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);
void StatsListCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

void ShowStatsList( Bool bShow );	//	ajw

#endif /* #ifndef _STATLIST_H */
