// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statmain.c:  Deal with drawing permanently displayed stats on the main window.
 *
 * All stats in group 1 are assumed to be for permanent display; others are displayed in
 * the normal way in stats.c.
 *
 * For these stats, the "name resource" sent from the server is actually an icon resource id;
 * we display these stats with icons next to the graph bars.
 */

#include "client.h"
#include "merintr.h"

#define STAT_VIGOR 3         // Position in main stat group of vigor stat

#define STAT_EMERGENCY_COLOR   RGB(255, 0, 0)   // Draw critical stats in this color when low

#define STAT_TOOLTIP_BASE 100  // Base for tooltip IDs

static list_type main_stats; // List of main stats (also kept in stat cache)

int stat_x;           // x position of left of main stats
int stat_bar_x;       // x position of left stats bars
int stat_width;       // Width of graph bars 

static void StatsMainMove(void);
static void StatsMainSetColor(Statistic *s);
/************************************************************************/
/*
 * StatsMainReceive:  We received main group of stats from server.
 *   stats is a list of pointers to Statistic structures.
 */
void StatsMainReceive(list_type stats)
{
   int height, y, count;
   list_type l;
   TOOLINFO ti;

   StatsMainDestroy();        // Destroy existent controls

   main_stats = stats;

   pinfo.vigor = MIN_VIGOR;   // Initialize for changing color of bar graph
   ti.cbSize = sizeof(TOOLINFO);
   ti.uFlags = 0;
   ti.hwnd   = cinfo->hMain;
   ti.hinst  = hInst;
   ti.lpszText = 0;
   count = 0;

   // Create graph controls for integer stats
   height = STAT_ICON_HEIGHT + STATS_MAIN_SPACING;	 
   //y = ENCHANT_SIZE + 2 * ENCHANT_BORDER - 1 + EDGETREAT_HEIGHT;
   y = ENCHANT_BORDER + EDGETREAT_HEIGHT + ((USERAREA_HEIGHT - (STAT_ICON_HEIGHT * 3)) / 2);
   for (l = stats; l != NULL; l = l->next)
   {
      Statistic *s = (Statistic *) (l->data);
      
      s->y = y;
      s->cy = height;
      y += height;
      
      if (s->numeric.tag != STAT_INT)
	 continue;
      
      s->hControl = CreateWindow(GraphCtlGetClassName(), NULL,
				 WS_CHILD | WS_VISIBLE | GCS_LIMITBAR | GCS_NUMBER,
				 0, 0, 0, 0, cinfo->hMain,
				 NULL, hInst, NULL);

      StatsMainSetColor(s);
      StatsMainChange(s);

      // Add tooltip for icon
      ti.uId = STAT_TOOLTIP_BASE + count;

      SendMessage(cinfo->hToolTips, TTM_ADDTOOL, 0, (LPARAM) &ti);
      count++;
   }

   StatsMainMove();
}
/************************************************************************/
/*
 * StatsMainDestroy:  Destroy controls for main stats group.
 */
void StatsMainDestroy(void)
{
   list_type l;
   TOOLINFO ti;
   int count = 0;
   
   ti.cbSize = sizeof(TOOLINFO);
   ti.hwnd   = cinfo->hMain;

   for (l = main_stats; l != NULL; l = l->next)
   {
      Statistic *s = (Statistic *) (l->data);
      
      if (s->numeric.tag != STAT_INT)
	 continue;
      
      DestroyWindow(s->hControl);

      // Destroy tooltip
      ti.uId    = STAT_TOOLTIP_BASE + count;
      SendMessage(cinfo->hToolTips, TTM_DELTOOL, 0, (LPARAM) &ti);
      count++;
   }
   main_stats = NULL;
}
/************************************************************************/
/*
 * StatsMainChange:  Given stat in main group changed to given value.
 */
void StatsMainChange(Statistic *s)
{
   int old_vigor;

   SendMessage(s->hControl, GRPH_RANGESET, s->numeric.min, s->numeric.max);
   SendMessage(s->hControl, GRPH_POSSET, 0, s->numeric.value);
   SendMessage(s->hControl, GRPH_LIMITSET, 0, s->numeric.current_max);	 

   if (s->num == STAT_VIGOR)
   {
      old_vigor = pinfo.vigor;
      pinfo.vigor = s->numeric.value;

      // Set color of graph if vigor is very low
      if (pinfo.vigor < MIN_VIGOR && old_vigor >= MIN_VIGOR)
	 SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_BAR, STAT_EMERGENCY_COLOR);

      if (pinfo.vigor >= MIN_VIGOR && old_vigor < MIN_VIGOR)
	 SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_BAR, GetColor(COLOR_BAR1));	 
   }
}

/************************************************************************/
/*
 * StatsMainRedraw:  Called when the main window needs to be redrawn.
 */
void StatsMainRedraw(void)
{
   list_type l;
   HDC hdc;
   AREA a, b;
   object_node *obj;   // Fake object node for DrawObject

   hdc = GetDC(cinfo->hMain);

   obj = ObjectGetBlank();

   a.x    = stat_x;
   a.cx   = STAT_ICON_HEIGHT;

   for (l = main_stats; l != NULL; l = l->next)
   {
      Statistic *s = (Statistic *) (l->data);

      a.y    = s->y;
      a.cy   = s->cy;

      obj->icon_res = s->name_res;

      OffscreenWindowBackground(NULL, a.x, a.y, a.cx, a.cy);
      DrawStretchedObjectDefault(hdc, obj, &a, NULL); 
      GdiFlush();

      b.x  = stat_bar_x;
      b.cx = stat_width;
      b.y  = a.y + STATS_MAIN_SPACING;
      b.cy = s->cy - 4 * STATS_MAIN_SPACING;
      InterfaceDrawBarBorder(NULL, hdc, &b);
   }

   ObjectDestroyAndFree(obj);
   ReleaseDC(cinfo->hMain, hdc);
}
/************************************************************************/
/*
 * StatsMainResize:  Called when the main window is resized.
 */
void StatsMainResize(int xsize, int ysize, AREA *view)
{
   stat_x = view->x + view->cx + LEFT_BORDER + USERAREA_WIDTH + RIGHT_BORDER + MAPTREAT_WIDTH;
   stat_bar_x = stat_x + STAT_ICON_WIDTH + RIGHT_BORDER;
   stat_width = xsize - stat_bar_x - RIGHT_BORDER - EDGETREAT_WIDTH - MAPTREAT_WIDTH - 4;
   StatsMainMove();
}
/************************************************************************/
/*
 * StatsMainMove:  Move main stat displays when created or resized.
 */
void StatsMainMove(void)
{
   list_type l;
   int count = 0;
   TOOLINFO ti;

   ti.cbSize = sizeof(TOOLINFO);
   ti.hwnd   = cinfo->hMain;
   ti.uFlags = 0;
   ti.hinst  = hInst;

   // Move graph bars
   for (l = main_stats; l != NULL; l = l->next)
   {
      Statistic *s = (Statistic *) (l->data);
      
      if (s->numeric.tag != STAT_INT)
	 continue;

      MoveWindow(s->hControl, stat_bar_x, s->y + STATS_MAIN_SPACING, 
		 stat_width, STATS_BAR_HEIGHT, TRUE);

      // Move tooltip
      ti.uId    = STAT_TOOLTIP_BASE + count;

      if (cinfo->hToolTips != NULL)
      {
	 ti.rect.left   = stat_x;
	 ti.rect.right  = ti.rect.left + STAT_ICON_WIDTH;
	 ti.rect.top    = s->y;
	 ti.rect.bottom = ti.rect.top + STAT_ICON_HEIGHT;
	 switch (count)
	 {
	 case 0: ti.lpszText = (LPSTR) IDS_HEALTH; break;
	 case 1: ti.lpszText = (LPSTR) IDS_MANA;   break;
	 case 2: ti.lpszText = (LPSTR) IDS_VIGOR;  break;
	 case 3: ti.lpszText = (LPSTR) IDS_TOUGHER; break;
	 default:
	    debug(("StatsMainMove got unknown stat number %d\n", count));
	    ti.lpszText = 0;
	    break;
	 }
	 SendMessage(cinfo->hToolTips, TTM_SETTOOLINFO, 0, (LPARAM) &ti);
      }
      count++;
   }
}
/************************************************************************/
/*
 * StatsMainChangeColor:  Called when user changes a color.
 */
void StatsMainChangeColor(void)
{
   list_type l;

   // Change colors in bar graphs   
   for (l = main_stats; l != NULL; l = l->next)
   {
      Statistic *s = (Statistic *) (l->data);
      StatsMainSetColor(s);
   }
}
/************************************************************************/
/*
 * StatsMainSetColor:  Set colors for a single graph control in a stat.
 */
void StatsMainSetColor(Statistic *s)
{
   if (s->numeric.tag != STAT_INT)
      return;
   
   SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_BAR, GetColor(COLOR_BAR1));
   SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_LIMITBAR, GetColor(COLOR_BAR2));
   SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_BKGND, GetColor(COLOR_BAR3));   
}
