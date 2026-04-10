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

#define STAT_HEALTH 1        // Position in main stat group of health stat
#define STAT_MANA   2        // Position in main stat group of mana stat
#define STAT_VIGOR  3        // Position in main stat group of vigor stat

#define STAT_EMERGENCY_COLOR   RGB(255, 0, 0)   // Draw critical stats in this color when low

static const COLORREF STAT_HEALTH_COLOR       = RGB(170, 50, 50);    // Health bar and icon: red
static const COLORREF STAT_HEALTH_LIMIT_COLOR = RGB(100, 30, 30);    // Health limit bar: dark red
static const COLORREF STAT_MANA_COLOR         = RGB(55, 100, 170);   // Mana bar: blue
static const COLORREF STAT_MANA_LIMIT_COLOR   = RGB(32, 60, 100);    // Mana limit bar: dark blue
static const COLORREF STAT_MANA_ICON_COLOR    = RGB(125, 135, 175);  // Mana icon: silver-blue
static const COLORREF STAT_VIGOR_COLOR        = RGB(170, 148, 40);   // Vigor bar and icon: gold
static const COLORREF STAT_VIGOR_LIMIT_COLOR  = RGB(100, 88, 25);    // Vigor limit bar: dark gold

#define STAT_TOOLTIP_BASE 100  // Base for tooltip IDs

static list_type main_stats; // List of main stats (also kept in stat cache)

int stat_x;           // x position of left of main stats
int stat_bar_x;       // x position of left stats bars
int stat_width;       // Width of graph bars 

static void StatsMainMove(void);
static void StatsMainSetColor(Statistic *s);
/************************************************************************/
/*
 * GetStatIconSize:  Returns the stat icon dimension for the active theme.
 *   Non-default themes use a larger size so the Unicode symbols are legible.
 */
static int GetStatIconSize(void)
{
   return IsNonDefaultTheme() ? 20 : STAT_ICON_SIZE;
}
/************************************************************************/
/*
 * GetStatsBarHeight:  Returns the stat bar height for the active theme.
 *   Non-default themes use a taller bar to match the larger icon row.
 */
int GetStatsBarHeight(void)
{
   return IsNonDefaultTheme() ? STATS_BAR_HEIGHT_LARGE : STATS_BAR_HEIGHT;
}
/************************************************************************/
/*
 * StatsMainGetInitialY:  Returns the Y offset for the first stat row,
 *   accounting for the active theme's layout.
 */
static int StatsMainGetInitialY(int icon_size)
{
   if (IsNonDefaultTheme())
      return TOP_BORDER + EDGETREAT_HEIGHT + 1;
   return ENCHANT_BORDER + EDGETREAT_HEIGHT + ((USERAREA_HEIGHT - (icon_size * 3)) / 2);
}
/************************************************************************/
/*
 * StatsMainGetBarY:  Returns the Y position for a graph bar control
 *   within its stat row, accounting for the active theme's layout.
 */
static int StatsMainGetBarY(Statistic *s, int barH)
{
   if (IsNonDefaultTheme())
      return s->y + (s->cy - barH) / 2;
   return s->y + STATS_MAIN_SPACING;
}
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
   int icon_size = GetStatIconSize();
   height = icon_size + STATS_MAIN_SPACING;	 
   //y = ENCHANT_SIZE + 2 * ENCHANT_BORDER - 1 + EDGETREAT_HEIGHT;
   y = StatsMainGetInitialY(icon_size);
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
   StatsMainRedraw();
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
         StatsMainSetColor(s);
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
   int stat_index = 0;

   /* Unicode symbols for non-default stat icons: cross, ankh, lightning */
   static const wchar_t *stat_symbols[] = { L"\u271A", L"\u2625", L"\u26A1" };

   hdc = GetDC(cinfo->hMain);

   obj = ObjectGetBlank();

   a.x    = stat_x;
   a.cx   = GetStatIconSize();

   for (l = main_stats; l != NULL; l = l->next)
   {
      Statistic *s = (Statistic *) (l->data);

      a.y    = s->y;
      a.cy   = s->cy;

      if (IsNonDefaultTheme() && stat_index < 3)
      {
         /*
          * Non-default themes: draw a Unicode symbol instead of the BGF sprite.
          * Paint the tiled background first, then overlay the colored symbol.
          */
         SidebarWindowBackground(a.x, a.y, a.cx, a.cy);
         OffscreenCopy(hdc, a.x, a.y, a.cx, a.cy, 0, 0);

         /*
          * Per-symbol font size bump: the ankh and lightning glyphs render
          * smaller than the cross at the same point size in Segoe UI Symbol.
          */
         static const int stat_icon_size_adj[] = { 0, 4, 3 };

         /*
          * Per-symbol Y offset: the ankh glyph has internal whitespace
          * above the crossbar, making it appear lower than the cross
          * and lightning. Nudge it up to equalize visual spacing.
          */
         static const int stat_icon_y_adj[] = { 0, -2, 0 };

         HFONT hSymFont = CreateFontW(GetStatIconSize() + 4 + stat_icon_size_adj[stat_index],
            0, 0, 0, FW_BLACK,
            FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
         HFONT hOldFont = (HFONT)SelectObject(hdc, hSymFont);

         SetBkMode(hdc, TRANSPARENT);

         /* Per-stat icon colors matching the bar colors */
         static const COLORREF stat_icon_colors[] = {
            STAT_HEALTH_COLOR,
            STAT_MANA_ICON_COLOR,
            STAT_VIGOR_COLOR
         };

         SIZE sz;
         GetTextExtentPoint32W(hdc, stat_symbols[stat_index], 1, &sz);
         int tx = a.x + (a.cx - sz.cx) / 2;
         int ty = a.y + (a.cy - sz.cy) / 2 + stat_icon_y_adj[stat_index];

         /* Draw a 1px dark outline for contrast against the sidebar */
         SetTextColor(hdc, RGB(0, 0, 0));
         TextOutW(hdc, tx - 1, ty, stat_symbols[stat_index], 1);
         TextOutW(hdc, tx + 1, ty, stat_symbols[stat_index], 1);
         TextOutW(hdc, tx, ty - 1, stat_symbols[stat_index], 1);
         TextOutW(hdc, tx, ty + 1, stat_symbols[stat_index], 1);

         SetTextColor(hdc, stat_icon_colors[stat_index]);
         TextOutW(hdc, tx, ty, stat_symbols[stat_index], 1);

         SelectObject(hdc, hOldFont);
         DeleteObject(hSymFont);
      }
      else
      {
         obj->icon_res = s->name_res;

         // Default theme: use BGF sprite with tiled texture background.
         SidebarWindowBackground(a.x, a.y, a.cx, a.cy);
         DrawStretchedObjectDefault(hdc, obj, &a, NULL); 
         GdiFlush();
      }

      b.x  = stat_bar_x;
      b.cx = stat_width;
      b.y  = a.y + STATS_MAIN_SPACING;
      b.cy = s->cy - 4 * STATS_MAIN_SPACING;
      InterfaceDrawBarBorder(NULL, hdc, &b);
      stat_index++;
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
   stat_bar_x = stat_x + GetStatIconSize() + RIGHT_BORDER;
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

      int barH = GetStatsBarHeight();
      int barY = StatsMainGetBarY(s, barH);
      MoveWindow(s->hControl, stat_bar_x, barY, 
		 stat_width, barH, TRUE);

      // Move tooltip
      ti.uId    = STAT_TOOLTIP_BASE + count;

      if (cinfo->hToolTips != NULL)
      {
	 ti.rect.left   = stat_x;
	 ti.rect.right  = ti.rect.left + GetStatIconSize();
	 ti.rect.top    = s->y;
	 ti.rect.bottom = ti.rect.top + GetStatIconSize();
	 switch (count)
	 {
	 case 0: ti.lpszText = (LPSTR) IDS_HEALTH; break;
	 case 1: ti.lpszText = (LPSTR) IDS_MANA;   break;
	 case 2: ti.lpszText = (LPSTR) IDS_VIGOR;  break;
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
 * StatsMainChangeColor:  Called when user changes a color or theme.
 *   Recalculates layout because non-default themes use larger bars.
 */
void StatsMainChangeColor(void)
{
   list_type l;
   int icon_size = GetStatIconSize();
   int height = icon_size + STATS_MAIN_SPACING;
   int y = StatsMainGetInitialY(icon_size);

   for (l = main_stats; l != NULL; l = l->next)
   {
      Statistic *s = (Statistic *) (l->data);

      s->y = y;
      s->cy = height;
      y += height;

      StatsMainSetColor(s);
   }

   StatsMainMove();
   StatsMainRedraw();
}
/************************************************************************/
/*
 * StatsMainSetColor:  Set bar colors for a single graph control.
 *   Non-default themes use per-stat colors (red health, blue mana, gold vigor).
 *   Default theme uses the uniform green/red palette colors.
 */
static void StatsMainSetColor(Statistic *s)
{
   if (s->numeric.tag != STAT_INT)
      return;

   COLORREF barColor, limitColor;

   if (IsNonDefaultTheme())
   {
      switch (s->num)
      {
      case STAT_HEALTH:
         barColor = STAT_HEALTH_COLOR;
         limitColor = STAT_HEALTH_LIMIT_COLOR;
         break;
      case STAT_MANA:
         barColor = STAT_MANA_COLOR;
         limitColor = STAT_MANA_LIMIT_COLOR;
         break;
      case STAT_VIGOR:
         barColor = STAT_VIGOR_COLOR;
         limitColor = STAT_VIGOR_LIMIT_COLOR;
         break;
      default:
         barColor = GetColor(COLOR_BAR1);
         limitColor = GetColor(COLOR_BAR2);
         break;
      }
   }
   else
   {
      barColor = GetColor(COLOR_BAR1);
      limitColor = GetColor(COLOR_BAR2);
   }

   SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_BAR, barColor);
   SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_LIMITBAR, limitColor);
   SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_BKGND, GetColor(COLOR_BAR3));

   if (IsNonDefaultTheme())
      SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_FRAME, RGB(0, 0, 0));
}
