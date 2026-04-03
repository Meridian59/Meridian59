// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * stats.c:  Handle display of game statistics area in the main window.
 * 
 * Stat groups are numbered starting with 1.
 *
 * The stats area is a modeless dialog box, with the main window as its parent.
 */

#include "client.h"
#include "merintr.h"

HWND hStats;                        // Window containing stats area.
static list_type stats = NULL;      // List of stats currently being processed.

static AREA stats_area;

static StatGroup current_group;           // Group number currently being processed.
static StatGroupType group_type;              // Type of group currently being processed.
static StatGroup pending_group;           // Explicit group request waiting on the server.

/* local function prototypes */
static void StatsCreateGroup(void);
static void StatsDestroyGroup(void);
static void StatsCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static void StatsVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
static void StatsMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys);
static int StatsGetWheelSteps(int zDelta);
static INT_PTR CALLBACK StatsWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static void StatRedraw(Statistic *s);
/************************************************************************/
/*
 * StatsCreate:  Create the stats area.
 */
void StatsCreate(HWND hParent)
{
  CreateDialog(hInst, MAKEINTRESOURCE(IDD_STATS), hParent, StatsWindowProc);
  
  current_group = STATS_INVENTORY;   // Group to start displaying
   pending_group = GROUP_NONE;
  group_type = StatGroupType::INVALID_TYPE;
  StatCacheCreate();
  StatButtonsCreate();
  RequestStatGroups();   
}
/************************************************************************/
/* 
 * StatsWindowProc:  Subclass stats window to have transparent background.
 */
INT_PTR CALLBACK StatsWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   const DRAWITEMSTRUCT *lpdis;
  
   switch (message)
   {
   case WM_INITDIALOG:
      hStats = hwnd;
      return FALSE;

   case WM_ERASEBKGND:
      if (StatsGetCurrentGroup() == STATS_CHARACTER)
      {
         StatsClearArea();
         StatsDraw();
      }
      return TRUE;

   case WM_MOUSEWHEEL:
      HANDLE_WM_MOUSEWHEEL(hwnd, wParam, lParam, StatsMouseWheel);
      return TRUE;

      HANDLE_MSG(hwnd, WM_COMMAND, StatsCommand);
      HANDLE_MSG(hwnd, WM_VSCROLL, StatsVScroll);

   case WM_MEASUREITEM:
      StatsListMeasureItem(hwnd, (MEASUREITEMSTRUCT *) lParam);
      return TRUE;
   case WM_DRAWITEM: 
     lpdis = (const DRAWITEMSTRUCT *)(lParam);

     switch (lpdis->CtlID)
     {
     case IDC_STATBUTTON:
       StatButtonDrawItem(hwnd, lpdis);
       return false;

     default:
       return StatsListDrawItem(hwnd, lpdis);
     }
     return TRUE;
      
   case WM_CTLCOLORSCROLLBAR:
     return (INT_PTR) GetSysColorBrush(COLOR_SCROLLBAR);

   case WM_SETFOCUS:
   case WM_KILLFOCUS:
      StatsSetButtonFocus(current_group);
      break;

   case WM_ACTIVATE:
     if (wParam == 0)
       *cinfo->hCurrentDlg = NULL;
     else *cinfo->hCurrentDlg = hwnd;
     return TRUE;
   }
   return FALSE;
}
/****************************************************************************/
void StatsCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
   switch (id)
   {
   case IDC_STATBUTTON:
      StatButtonCommand(hwnd, id, hwndCtl, codeNotify);
      break;

   case IDC_STATLIST:
      StatsListCommand(hwnd, id, hwndCtl, codeNotify);
      break;
   }
}
/************************************************************************/
/*
 * StatsVScroll:  Dispatch vertical scrollbar messages to the active
 *   stats group implementation.
 */
static void StatsVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
   switch (group_type)
   {
   case STATS_NUMERIC:
      StatsNumVScroll(hwnd, hwndCtl, code, pos);
      break;

   case STATS_LIST:
      StatsListVScroll(GetDlgItem(hwnd, IDC_STATLIST), hwndCtl, code, pos);
      break;
   }
}
/************************************************************************/
/*
 * StatsGetWheelSteps:  Convert accumulated wheel delta into whole-item
 *   scroll steps.
 */
static int StatsGetWheelSteps(int zDelta)
{
   static int wheel_delta;

   wheel_delta += zDelta;

   int steps = wheel_delta / WHEEL_DELTA;
   wheel_delta -= steps * WHEEL_DELTA;
   return steps;
}
/************************************************************************/
/*
 * StatsMouseWheel:  Scroll the active stats group in response to mouse
 *   wheel input.  Parameter order must be (zDelta, fwKeys) to match
 *   the HANDLE_WM_MOUSEWHEEL macro expansion from windowsx.h.
 */
static void StatsMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys)
{
   int steps = StatsGetWheelSteps(zDelta);
   UINT code = steps > 0 ? SB_LINEUP : SB_LINEDOWN;

   if (StatsGetCurrentGroup() == STATS_INVENTORY || group_type == StatGroupType::INVALID_TYPE)
      return;

   if (steps == 0)
      return;

   for (int index = 0; index < abs(steps); ++index)
      StatsVScroll(hwnd, NULL, code, 0);
}
/************************************************************************/
/*
 * StatsDestroy:  Destroy the stats area.
 */
void StatsDestroy(void)
{
   StatsDestroyGroup();
   StatsDestroyButtons();
   stats = NULL;     // Actual list freed by cache
   DestroyWindow(hStats);

   StatButtonsDestroy();
   StatCacheDestroy();
}
/************************************************************************/
/*
 * StatsResize:  Resize the stats area when the main window is resized
 *   to (xsize, ysize).  view is the current grid area view.
 */
void StatsResize(int xsize, int ysize, AREA *view)
{
   int yMiniMap, iHeightAvailableForMapAndStats, iHeightMiniMap;

   stats_area.x = view->x + view->cx + LEFT_BORDER + 3 * HIGHLIGHT_THICKNESS;
   stats_area.cx = xsize - stats_area.x - 3 * HIGHLIGHT_THICKNESS - EDGETREAT_WIDTH;

   yMiniMap = 2 * TOP_BORDER + USERAREA_HEIGHT + EDGETREAT_HEIGHT + MAPTREAT_HEIGHT;
   iHeightAvailableForMapAndStats = ysize - yMiniMap - 2 * HIGHLIGHT_THICKNESS - EDGETREAT_HEIGHT;
   iHeightMiniMap = (int)( iHeightAvailableForMapAndStats * PROPORTION_MINIMAP ) - HIGHLIGHT_THICKNESS - MAPTREAT_HEIGHT;
   
   stats_area.y = yMiniMap + iHeightMiniMap + 3 * HIGHLIGHT_THICKNESS + MAPTREAT_HEIGHT + MAP_STATS_GAP_HEIGHT;
   stats_area.cy = ysize - EDGETREAT_HEIGHT - HIGHLIGHT_THICKNESS - stats_area.y - STATS_BOTTOM_GAP_HEIGHT;
   
   MoveWindow(hStats, stats_area.x, stats_area.y, stats_area.cx, stats_area.cy, FALSE);
   ShowWindow(hStats, SW_SHOWNORMAL);
   
   StatsMoveButtons();
   StatsMove();
}
/************************************************************************/
void StatsSetFocus(bool forward)
{
   SetFocus(hStats);
}
/************************************************************************/
void StatsDrawBorder(void)
{
	HWND hFocus = GetFocus();

	AREA areaXXXTemp;
	areaXXXTemp.x = stats_area.x - HIGHLIGHT_THICKNESS;
	areaXXXTemp.y = stats_area.y - HIGHLIGHT_THICKNESS;
	areaXXXTemp.cx = stats_area.cx + 2 * HIGHLIGHT_THICKNESS;
	areaXXXTemp.cy = stats_area.cy + 2 * HIGHLIGHT_THICKNESS;

   // Check child windows & self
	if ( hFocus == hStats || IsChild(hStats, hFocus) || hFocus == GetHwndInv() )
		DrawBorder(&areaXXXTemp, HIGHLIGHT_INDEX, NULL);
	else if (IsDarkMode())
		DrawBorderRGB(&areaXXXTemp, RGB(45, 45, 48), NULL);
	else DrawBorder(&areaXXXTemp, -1, NULL);
}
/************************************************************************/
/*
 * StatsResetFont:  Set stats font when the font changes.
 */
void StatsResetFont(void)
{
   StatsDestroyGroup();
   StatsCreateGroup();
   StatsMove();
   InvalidateRect(hStats, NULL, TRUE);
}
/************************************************************************/
/*
 * StatsChangeColor:  Called when user changes a color.
 */
void StatsChangeColor(void)
{
   list_type l;
   // Change colors in bar graphs
   
   for (l = stats; l != NULL; l = l->next)
   {
      Statistic *s = (Statistic *) (l->data);

      if (s->numeric.tag != STAT_INT)
	 continue;
      
      SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_BAR, GetColor(COLOR_BAR1));
      SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_LIMITBAR, GetColor(COLOR_BAR2));
      SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_BKGND, GetColor(COLOR_BAR3));

      if (IsNonClassicTheme())
         SendMessage(s->hControl, GRPH_COLORSET, GRAPHCOLOR_FRAME, RGB(0, 0, 0));
   }

   StatsNumRetheme();
   StatsListRetheme();
}

/************************************************************************/
/*
 * StatsDrawNumItem:  Redraw stats area.
 */
bool StatsDrawNumItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
{
   StatsDraw();
   return true;
}

/************************************************************************/
void StatsClearArea(void)
{
   HDC hdc = GetDC(cinfo->hMain);
   RECT r;

   AreaToRect(&stats_area, &r);
   r.top += StatsGetButtonBorder() - STATUS_BUTTON_SPACING;
   if (r.top < stats_area.y)
      r.top = stats_area.y;
   DrawWindowBackgroundColor( pinventory_bkgnd(), hdc, &r, r.left, r.top, -1 );
   ReleaseDC(cinfo->hMain, hdc);
}
/************************************************************************/
/*
 * GetHwndStats:  Return the stats dialog window handle.
 */
HWND GetHwndStats(void)
{
   return hStats;
}
/************************************************************************/
/*
 * ActivateInventory: Activate the inventory.
 */
void ActivateInventory()
{
   pending_group = GROUP_NONE;
	InvalidateRect(GetHwndInv(), NULL, FALSE);
	DisplayInventoryAsStatGroup(StatGroup::STATS_INVENTORY);

	InventoryRedraw();
	InventorySetFocus(true);

	// The inventory is special and takes focus unlike the other stat groups.
	// We now return focus to the main window.
	SetFocus(cinfo->hMain);
}
/************************************************************************/
/*
 * ActiveStatGroup: Activate a specific stat group.
 */
void ActivateStatGroup(StatGroup stat_group)
{
	list_type stat_list;
	if (StatCacheGetEntry(stat_group, &stat_list) == true)
	{
		DisplayStatGroup(stat_group, stat_list);
	}
	else
	{
		RequestStats(stat_group);
	}
}
/****************************************************************************/
/*
 * RestoreActiveStatGroup:  Restore the player's active stat group
 *   from the INI config after login.  When the saved group is not
 *   inventory, hides the inventory and requests the saved group
 *   from the server.
 */
void RestoreActiveStatGroup()
{
	StatGroup active_stat_group = GetStatGroup();
	bool inventory_group = (active_stat_group == StatGroup::STATS_INVENTORY);
	StatsShowGroup(!inventory_group);
	ShowInventory(inventory_group);

	if (inventory_group)
	{
		ActivateInventory();
	}
	else
	{
		/* EventInventory already set current_group to STATS_INVENTORY.
		 * Clear it so StatsSetActiveGroup detects a group change when
		 * the user clicks the inventory tab before the stats response
		 * arrives from the server. */
		DisplayInventoryAsStatGroup(StatGroup::GROUP_NONE);
		ActivateStatGroup(active_stat_group);
	}
}
/************************************************************************/
/*
 * StatsGroupsInfo:  We've just received info on stat groups from the server.
 *   num_groups gives the number of groups; names is an array of their name resources.
 */
void StatsGroupsInfo(BYTE num_groups, ID *names)
{
   // We actually don't use stat names, since stat buttons are bitmaps
   SafeFree(names);

   StatsDestroyGroup();
   StatsMainDestroy();
   stats = NULL;
	//	ajw - Hard-coded number of "groups" as 5, as there are now 4 buttons.
	//	To keep things the way they were apparently intended, I'd have to change the message sent from the server.
	//	This seems pointless - I'm not sure why the number of stats list groups was being sent at all...
   //StatsSetButtons(num_groups);	ajw
   StatsSetButtons( 5 );
   StatCacheSetSize(num_groups);
   RequestStats(STATS_MAIN);              // Always get main stats
   RestoreActiveStatGroup();
}

/************************************************************************/
/*
 * StatsCreateGroup:  Create controls for a group of stats.
 *   Requires that group_type and stats have already been set.
 */
void StatsCreateGroup(void)
{
   switch(group_type)
   {
   case STATS_NUMERIC:
      StatsNumCreate(stats);
      break;

   case STATS_LIST:
      StatsListCreate(stats);
      break;
   }   
}
/************************************************************************/
/*
 * StatsDestroyGroup:  Destroy controls for a group of stats.
 */
void StatsDestroyGroup(void)
{
   switch(group_type)
   {
   case STATS_NUMERIC:
      StatsNumDestroy(stats);
      break;

   case STATS_LIST:
      StatsListDestroy(stats);
      break;
   }
}
/************************************************************************/
/*
 * StatsMove:  Move stat controls when main window is resized.
 */
void StatsMove(void)
{
   switch(group_type)
   {
   case STATS_NUMERIC:
      StatsNumResize(stats);
      break;

   case STATS_LIST:
      StatsListResize(stats);
      break;
   }
}

/************************************************************************/
/*
 * StatsDraw:  Display all the main stats.
 */
void StatsDraw(void)
{
   switch (group_type)
   {
   case STATS_NUMERIC:
      if (StatsGetCurrentGroup() == StatGroup::STATS_MAIN || StatsGetCurrentGroup() == StatGroup::STATS_CHARACTER)
         StatsNumDraw(stats);
      break;

   case STATS_LIST:
      // InvalidateRect( hStats, NULL, FALSE );
      break;
   }
}

/************************************************************************/
/*
 * StatChange:  Called when server tells us that a statistic has changed value.
 */
void StatChange(StatGroup group, Statistic *s)
{
   Statistic *new_stat;

   new_stat = StatCacheUpdate(group, s);

   if (group == STATS_MAIN && new_stat != NULL)
   {
      StatsMainChange(new_stat);
      return;
   }

   if (new_stat == NULL || group != current_group)
      return;

   StatRedraw(new_stat);
}
/************************************************************************/
/*
 * StatRedraw:  Redraw a statistic whose value has changed.
 */
void StatRedraw(Statistic *s)
{
   switch (s->type)
   {
   case STATS_NUMERIC:
      StatsNumChangeStat(s);
      break;

   case STATS_LIST:
      StatsListChangeStat(s);
      break;
   }
}
/************************************************************************/
/*
 * StatsReceiveGroup:  Called when we receive a group of stats from the server.
 */
void StatsReceiveGroup(StatGroup group, list_type l)
{
	if (group == STATS_MAIN)
		StatsMainReceive(l);
	else 
	{
      if (group == pending_group)
      {
         pending_group = GROUP_NONE;
         DisplayStatGroup(group, l);
      }
      else if (StatsGetCurrentGroup() == STATS_INVENTORY && group != STATS_INVENTORY)
		{
         /*
          * When inventory is the active display, don't switch away to show
          * an incoming stat group (e.g. server-pushed skill/spell updates).
          * Just cache the data; the user will see it when they switch tabs.
          */
			StatCacheSetEntry(group, l);
			return;
		}
      else DisplayStatGroup(group, l);
	}

   StatCacheSetEntry(group, l);
}

/************************************************************************/
void StatsGetArea(AREA *a)
{
   memcpy(a, &stats_area, sizeof(stats_area));
}
/************************************************************************/
/*
 * StatsGetCurrentGroup:  Return the stat group currently being displayed
 */
StatGroup StatsGetCurrentGroup(void)
{
   return current_group;
}

/************************************************************************/
/*
 * StatsGetPendingGroup:  Return the stat group currently waiting on a
 *   server response, or GROUP_NONE if there is no pending request.
 */
StatGroup StatsGetPendingGroup(void)
{
   return pending_group;
}

/************************************************************************/
/*
 * StatsSetPendingGroup:  Set the stat group currently waiting on a server
 *   response.
 */
void StatsSetPendingGroup(StatGroup group)
{
   pending_group = group;
}

/************************************************************************/
/*
 * DisplayStatGroup:  Display the given group of stats.
 */
void DisplayStatGroup(StatGroup group, list_type l)
{
   current_group = group;
   if (pending_group == group)
      pending_group = GROUP_NONE;

   StatsDestroyGroup();
   stats = l;

   if (stats == NULL)
      group_type = StatGroupType::INVALID_TYPE;
   else
      group_type = ((Statistic *) (stats->data))->type;

   StatsClearArea();

   StatsCreateGroup();
   InvalidateRect(hStats, NULL, TRUE);
   StatsMove();
}

/************************************************************************/
/*
 * DisplayInventoryAsStatGroup: Like DisplayStatGroup, but called when Inventory becomes the shown "group".
 */
void DisplayInventoryAsStatGroup( StatGroup group )
{
	current_group = group;
   pending_group = GROUP_NONE;
}

/************************************************************************/
/*
 * StatsShowGroup:  Show or hide controls for a group of stats.
 */
void StatsShowGroup(bool bShow)
{
   StatGroupType group_type_temp = StatGroupType::INVALID_TYPE;  // Type of group currently being displayed
   if (stats != NULL)
      group_type_temp = ((Statistic *) (stats->data))->type;

   switch (group_type_temp)
   {
   case STATS_NUMERIC:
      ShowStatsNum(bShow, stats);
      break;

   case STATS_LIST:
      ShowStatsList(bShow);
      break;
   }
}
