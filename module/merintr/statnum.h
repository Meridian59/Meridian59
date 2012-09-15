// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statnum.h:  Header file for statnum.c
 */

#ifndef _STATNUM_H
#define _STATNUM_H

void StatsNumCreate(list_type stats);
void StatsNumDestroy(list_type stats);
void StatsNumResize(list_type stats);
void StatsNumChangeStat(Statistic *s);
void StatsNumVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
void StatsNumDraw(list_type stats);

void ShowStatsNum( Bool bShow, list_type stats );	//	ajw

#endif /* #ifndef _STATNUM_H */
