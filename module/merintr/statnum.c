// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statnum.c: Handle numeric statistic groups.
 *
 * Only statistics groups with tag STAT_NUMERIC use this file.
 *
 */

#include "client.h"
#include "merintr.h"

static HWND hStatsScroll;           // Numeric stats scroll bar

static int num_visible;             // # of numeric stats visible
static int stats_bar_width;         // Width of stats bar
static int stats_scrollbar_width;   // Width of scrollbar for numeric stats
static int top_stat;                // Number of stat currently displayed at top of stats area

extern HWND hStats;

static void DisplayNumericStat(Statistic *s);
/************************************************************************/
/*
 * StatsNumCreate:  Create numeric stat controls for given stats.
 */
void StatsNumCreate(list_type stats)
{
   list_type l;
   int y, height;
   
   // Create graph controls for integer stats
   height = GetFontHeight(GetFont(FONT_STATS)) + STATUS_SPACING;	 
   y = StatsGetButtonBorder();
   for (l = stats; l != NULL; l = l->next)
   {
      Statistic *s = (Statistic *) (l->data);
      
      s->y = y;
      s->cy = height;
      y += height;
      
      if (s->numeric.tag != STAT_INT)
	 continue;
      
      s->hControl = CreateWindow(GraphCtlGetClassName(), NULL,
				 WS_CHILD | GCS_LIMITBAR | GCS_NUMBER,
				 0, 0, 0, 0, hStats,
				 NULL, hInst, NULL);
      
      StatsNumChangeStat(s);
   }
   
   stats_scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
   hStatsScroll = CreateWindow("scrollbar", NULL, WS_CHILD | SBS_VERT,
			       0, 0, 100, 100,  /* Make sure scrollbar drawn ok */
			       hStats, NULL, hInst, NULL);
   top_stat = 0;
   
   // Set colors of graphs
   StatsChangeColor();
}
/************************************************************************/
/*
 * StatsNumDestroy:  Destroy numeric stat controls.
 */
void StatsNumDestroy(list_type stats)
{
   list_type l;

   for (l = stats; l != NULL; l = l->next)
   {
      Statistic *s = (Statistic *) (l->data);
      
      if (s->numeric.tag != STAT_INT)
	 continue;
      
      DestroyWindow(s->hControl);
   }   
   DestroyWindow(hStatsScroll);
}
/************************************************************************/
/*
 * StatsNumResize:  Called when main window resized.
 */
void StatsNumResize(list_type stats)
{
   int x, y, height, count, num_stats;
   list_type l;
   AREA stats_area;
   Bool has_scrollbar = False;

   StatsGetArea(&stats_area);

   // First, see how many stats will fit
   y = StatsGetButtonBorder();
   num_visible = 0;
   num_stats = 0;
   for (l = stats; l != NULL; l = l->next)
   {
      Statistic *s = (Statistic *) (l->data);

      if (s->numeric.tag != STAT_INT)
	 continue;

      num_stats++;

      if (y + s->cy <= stats_area.cy)
      {
	 y += s->cy;
	 num_visible++;
      }
   }

   if (num_visible < num_stats)
      has_scrollbar = True;
   
   top_stat = min(top_stat, num_stats - num_visible);

   // Move graph bars
   x = stats_area.cx / 2;
   stats_bar_width = has_scrollbar ? 
     stats_area.cx / 2 - stats_scrollbar_width - 1 : stats_area.cx / 2;
   stats_bar_width -= RIGHT_BORDER;
   num_visible = 0;
   count = 0;
   
   y = StatsGetButtonBorder();
   
   for (l = stats; l != NULL; l = l->next)
   {
      Statistic *s = (Statistic *) (l->data);
      
      if (s->numeric.tag != STAT_INT)
	 continue;
      
      // Take into account position of scrollbar
      if (count++ < top_stat)
      {
	 ShowWindow(s->hControl, SW_HIDE);
	 continue;
      }
      
      // Center bar vertically
      s->y = y;
      y += s->cy;
      
      MoveWindow(s->hControl, x, s->y + (s->cy - STATS_BAR_HEIGHT) / 2, 
		 stats_bar_width, STATS_BAR_HEIGHT, TRUE);
      
      // Only show graph bar if it's completely visible
	  //	And Inventory is not selected.	ajw
      if (s->y + s->cy <= stats_area.cy && StatsGetCurrentGroup() != STATS_INVENTORY )
      {
	 ShowWindow(s->hControl, SW_NORMAL);
	 num_visible++;
      }
      else ShowWindow(s->hControl, SW_HIDE);
      height = s->cy;
   }
   
   // Show scrollbar if necessary
   if (has_scrollbar)
   {
      y = StatsGetButtonBorder();
      MoveWindow(hStatsScroll, stats_area.cx - stats_scrollbar_width, 
		 y, stats_scrollbar_width, num_visible * height, FALSE);
      ShowWindow(hStatsScroll, SW_HIDE);
      SetScrollRange(hStatsScroll, SB_CTL, 0, num_stats - num_visible, TRUE);
      SetScrollPos(hStatsScroll, SB_CTL, top_stat, TRUE); 
	  if( StatsGetCurrentGroup() != STATS_INVENTORY )	//	ajw
			ShowWindow(hStatsScroll, SW_SHOWNORMAL);
   }
   else ShowWindow(hStatsScroll, SW_HIDE);
   
   StatsDraw();
}
/************************************************************************/
/*
 * StatsNumChangeStat:  Redisplay current statistic, whose value has changed.
 *   Requires that s is a numeric type stat in the current group.
 */
void StatsNumChangeStat(Statistic *s)
{
   // Set range & current value
   SendMessage(s->hControl, GRPH_RANGESET, s->numeric.min, s->numeric.max);
   SendMessage(s->hControl, GRPH_POSSET, 0, s->numeric.value);
   SendMessage(s->hControl, GRPH_LIMITSET, 0, s->numeric.current_max);	 
   
   // Redraw name label
   DisplayNumericStat(s);
}
/************************************************************************/
/*
 * StatsNumVScroll:  Handle messages from stats scroll bar
 */
void StatsNumVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
   int new_top;  // New top row index
   int min_stat, max_stat;

   GetScrollRange(hStatsScroll, SB_CTL, &min_stat, &max_stat);

   switch (code)
   {
   case SB_LINEUP:
      new_top = top_stat - 1;
      break;

   case SB_LINEDOWN:
      new_top = top_stat + 1;
      break;

   case SB_PAGEUP:
      new_top = top_stat - num_visible;
      break;

   case SB_PAGEDOWN:
      new_top = top_stat + num_visible;
      break;

   case SB_THUMBPOSITION:
      new_top = pos;
      break;

   case SB_THUMBTRACK:
      new_top = pos;
      break;

   case SB_BOTTOM:
      new_top = max_stat;
      break;

   case SB_TOP:
      new_top = min_stat;
      break;
      
   default:
      // Pointless "SB_ENDSCROLL" added recently
      return;
   }

   new_top = max(new_top, min_stat);
   new_top = min(new_top, max_stat);

   if (new_top != top_stat)
   {
      top_stat = new_top;
      StatsMove();
   }
}
/************************************************************************/
/*
 * StatsNumDraw:  Redraw all numeric stats.
 */
void StatsNumDraw(list_type stats)
{
   list_type l;

   StatsClearArea();

   for (l = stats; l != NULL; l = l->next)
      DisplayNumericStat((Statistic *) l->data);
}
/************************************************************************/
/*
 * DisplayNumericStat:  Display the given numeric type stat from the current group on the
 *   main window.
 */
void DisplayNumericStat(Statistic *s)
{
   RECT r;
   HDC hdc;
   HFONT hOldFont;
   char *name, *str;
   AREA stats_area;
   AREA a;

	//	ajw - Avoid drawing if Inventory is selected as the "group".
	if( StatsGetCurrentGroup() == STATS_INVENTORY )
		return;
	
   StatsGetArea(&stats_area);

   r.left   = 0;
   r.right  = stats_area.cx / 2;
   r.top    = s->y;
   r.bottom = r.top + s->cy;

   /* If stat is out of stats area, abort */
   if (r.bottom > stats_area.cy || s->num <= top_stat)
      return;

   hdc = GetDC(hStats);
//   DrawWindowBackground(hdc, &r, stats_area.x + r.left, stats_area.y + r.top);
   DrawWindowBackgroundColor( pinventory_bkgnd(), hdc, &r, stats_area.x + r.left, stats_area.y + r.top, -1 );

   hOldFont = (HFONT) SelectObject(hdc, GetFont(FONT_STATS));

   SetBkMode(hdc, TRANSPARENT);

   name = LookupNameRsc(s->name_res);

   // Draw with drop shadow
   SetTextColor(hdc, GetColor(COLOR_STATSBGD));
   DrawText(hdc, name, strlen(name), &r, DT_LEFT);
   OffsetRect(&r, 1, 1);
   SetTextColor(hdc, GetColor(COLOR_STATSFGD));
   DrawText(hdc, name, strlen(name), &r, DT_LEFT);

   switch (s->numeric.tag)
   {
   case STAT_RES:
      r.left  = stats_area.cx / 2;
      r.right = stats_area.cx;
      DrawWindowBackgroundColor( pinventory_bkgnd(), hdc, &r, stats_area.x + r.left, stats_area.y + r.top, -1 );

      str = LookupNameRsc(s->numeric.value);
      DrawText(hdc, str, strlen(str), &r, DT_RIGHT);
      break;

   case STAT_INT:
      // Draw background around stat bar
     a.x = stats_area.cx / 2;
     a.cx = stats_bar_width;
     a.y = s->y + (s->cy - STATS_BAR_HEIGHT) / 2;
     a.cy = STATS_BAR_HEIGHT;
     InterfaceDrawBarBorder( pinventory_bkgnd(), hdc, &a );
     break;
   }

   SelectObject(hdc, hOldFont);
   ReleaseDC(hStats, hdc);

   InvalidateRect( s->hControl, NULL, FALSE );
}

/************************************************************************/
/*
 * ShowStatsNum:  ajw - Shows or hides numeric stats.
 */
void ShowStatsNum( Bool bShow, list_type stats )
{
	list_type l;

	for (l = stats; l != NULL; l = l->next)
	{
		Statistic *s = (Statistic *) (l->data);
  
		if (s->numeric.tag != STAT_INT)
			continue;
  
		ShowWindow(s->hControl, bShow ? SW_SHOW : SW_HIDE );
	}   
	ShowWindow( hStatsScroll, bShow ? SW_SHOW : SW_HIDE );
//	StatsClearArea();
}
