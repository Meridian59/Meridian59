// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statmain.h:  Header for statmain.c
 */

#ifndef _STATMAIN_H
#define _STATMAIN_H

#define MIN_VIGOR 10   // Minimum value of vigor needed to run

#define STAT_ICON_HEIGHT 15  // Height of stat icons
#define STAT_ICON_WIDTH 17   // Width of stat icons
#define STATS_MAIN_SPACING 1 // Space between adjacent stats

void StatsMainReceive(list_type stats);
void StatsMainDestroy(void);
void StatsMainChange(Statistic *s);
void StatsMainResize(int xsize, int ysize, AREA *view);
void StatsMainRedraw(void);
void StatsMainChangeColor(void);


#endif /* #ifndef _STATMAIN_H */
