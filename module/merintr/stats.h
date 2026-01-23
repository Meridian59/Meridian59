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

// Numbers for groups of statistics Equate with value returned from StatsGetCurrentGroup()
// NOTE: These values are matched to server-side values - seen in user.kod for example
enum StatGroup : BYTE {
   GROUP_NONE = (BYTE)-1,
   STATS_MAIN = 1,      // (Health, Mana, Vigor) Not shown on skills panel, shown on character panel
   STATS_CHARACTER = 2, // (Might, Int, Stam, Agil, Myst, Aim, Karma)
   STATS_SPELLS = 3,
   STATS_SKILLS = 4,
   STATS_INVENTORY = 5, // Not a real stat group, client-side only

   STATS_COUNT,  // Number of stat groups defined
};

/* game statistics */
typedef struct
{
   BYTE num;        /* ordinal # of statistic within group */
   ID name_res;     /* Name of statistic */
   StatGroupType type;  // Statistic type (STATS_NUMERIC or STATS_LIST)
   union
   {
      // List item stat
      struct
      {
         ID id;      // Object to examine when this stat selected
         int value;  // Value associated with stat
         ID icon;
      } list;

      // Numeric value stat
      struct
      {
         BYTE tag;                   /* Type of value: resource or integer */
         ID value;                   /* resource id or integer value */
         int min, max, current_max;  // Limits for numeric stats
      } numeric;
   };

   // fields used internally by client
   HWND hControl; /* Control window for this stat */
   int y, cy;     /* Vertical position and size of stat display */
} Statistic;

#define STATS_GRAPH_BORDER 8   // Space between bar graphs

void StatsCreate(HWND hParent);
void StatsDestroy(void);
void StatsResize(int xsize, int ysize, AREA *view);
void StatsResetFont(void);
void StatsChangeColor(void);
bool StatsDrawNumItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);
void StatsSetFocus(bool forward);
void StatsDrawBorder(void);
void StatsGetArea(AREA *a);
StatGroup StatsGetCurrentGroup(void);
void StatsDraw(void);
void StatsMove(void);
void StatsClearArea(void);

void StatsGroupsInfo(BYTE num_groups, ID *names);
void StatChange(StatGroup group, Statistic *s);
void StatsReceiveGroup(StatGroup group, list_type l);

void DisplayStatGroup(StatGroup group, list_type l);
void DisplayInventoryAsStatGroup(StatGroup group);
void StatsShowGroup(bool bShow);

#endif /* #ifndef _STATS_H */
