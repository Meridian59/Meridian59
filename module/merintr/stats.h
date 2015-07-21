// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * stats.h:  Header for stats.c
 */

#ifndef _STATS_H
#define _STATS_H

/* Size parameters for stats area */
#define STATUS_LEFT_BORDER  10
#define STATUS_MIN_WIDTH    (120 + STATUS_LEFT_BORDER)
#define STATUS_MAX_WIDTH    (250 + STATUS_LEFT_BORDER)
#define STATUS_SPACING      3
#define STATUS_BUTTON_SPACING      1 // was 5

#define STATS_BAR_HEIGHT    12

/* Numbers for groups of statistics */
#define GROUP_NONE -1
#define STATS_MAIN 1

//	ajw added constants to refer to other stats "groups". Equate with value returned from StatsGetCurrentGroup().
#define	STATS_SPELLS    3
#define	STATS_SKILLS    4
#define	STATS_QUESTS    5
#define	STATS_INVENTORY 6


/* game statistics */
typedef struct {
   BYTE  num;       /* ordinal # of statistic within group */
   ID    name_res;  /* Name of statistic */
   BYTE  type;      // Statistic type (numeric or list)
   union {
      // List item stat
      struct {
	 ID    id;                     // Object to examine when this stat selected
	 int   value;                  // Value associated with stat
	 ID    icon;
      } list;

      // Numeric value stat
      struct {
	 BYTE  tag;                    /* Type of value: resource or integer */
	 ID    value;                  /* resource id or integer value */
	 int   min, max, current_max;  // Limits for numeric stats 
      } numeric;
   };

   // fields used internally by client
   HWND  hControl;  /* Control window for this stat */
   int   y, cy;     /* Vertical position and size of stat display */
} Statistic;

#define STATS_GRAPH_BORDER 8   // Space between bar graphs

void StatsCreate(HWND hParent);
void StatsDestroy(void);
void StatsResize(int xsize, int ysize, AREA *view);
void StatsResetFont(void);
void StatsChangeColor(void);
Bool StatsDrawNumItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);
void StatsSetFocus(Bool forward);
void StatsDrawBorder(void);
void StatsGetArea(AREA *a);
int  StatsGetCurrentGroup(void);
void StatsDraw(void);
void StatsMove(void);
void StatsClearArea(void);

void StatsGroupsInfo(BYTE num_groups, ID *names);
void StatChange(BYTE group, Statistic *s);
void StatsReceiveGroup(BYTE group, list_type l);

void DisplayStatGroup(BYTE group, list_type l);
void DisplayInventoryAsStatGroup( BYTE group );
void StatsShowGroup( Bool bShow );

#endif /* #ifndef _STATS_H */
