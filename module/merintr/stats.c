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

HWND hStats;                        // Window containing stats area
static list_type stats = NULL;      // List of stats currently displayed

static AREA stats_area;

static int current_group;           // Group number currently being displayed
static int group_type;              // Type of group currently being displayed

/* local function prototypes */
static void StatsCreateGroup(void);
static void StatsDestroyGroup(void);
static void StatsCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static BOOL CALLBACK StatsWindowProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
static void StatRedraw(Statistic *s);
/************************************************************************/
/*
 * StatsCreate:  Create the stats area.
 */
void StatsCreate(HWND hParent)
{
  CreateDialog(hInst, MAKEINTRESOURCE(IDD_STATS), hParent, StatsWindowProc);
  
  current_group = STATS_INVENTORY;   // Group to start displaying
  group_type = GROUP_NONE;
  StatCacheCreate();
  StatButtonsCreate();
  RequestStatGroups();   
}
/************************************************************************/
/* 
 * StatsWindowProc:  Subclass stats window to have transparent background.
 */
BOOL CALLBACK StatsWindowProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
   const DRAWITEMSTRUCT *lpdis;
  
   switch (message)
   {
   case WM_INITDIALOG:
      hStats = hwnd;
      return FALSE;

   case WM_ERASEBKGND:
		if( StatsGetCurrentGroup() == STATS_SPELLS || StatsGetCurrentGroup() == STATS_SKILLS )
		{
			StatsClearArea();
			InvalidateRect( hStats, NULL, FALSE );
		}
      return TRUE;

      HANDLE_MSG(hwnd, WM_COMMAND, StatsCommand);
      HANDLE_MSG(hwnd, WM_VSCROLL, StatsNumVScroll);

   case WM_MEASUREITEM:
      StatsListMeasureItem(hwnd, (MEASUREITEMSTRUCT *) lParam);
      return TRUE;
   case WM_DRAWITEM: 
     lpdis = (const DRAWITEMSTRUCT *)(lParam);

     switch (lpdis->CtlID)
     {
     case IDC_STATBUTTON:
       StatButtonDrawItem(hwnd, lpdis);
       return False;

     default:
       return StatsListDrawItem(hwnd, lpdis);
     }
     return TRUE;
      
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
void StatsSetFocus(Bool forward)
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
   }
}

/************************************************************************/
/*
 * StatsDrawNumItem:  Redraw stats area.
 */
Bool StatsDrawNumItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
{
   StatsDraw();
   return True;
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
   if( current_group != STATS_INVENTORY )
		RequestStats(current_group);
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
   switch(group_type)
   {
   case STATS_NUMERIC:
		if( StatsGetCurrentGroup() != STATS_INVENTORY && StatsGetCurrentGroup() != STATS_SPELLS && StatsGetCurrentGroup() != STATS_SKILLS )
			StatsNumDraw(stats);
      break;

   case STATS_LIST:
//	   InvalidateRect( hStats, NULL, FALSE );
      break;
   }
}

/************************************************************************/
/*
 * StatChange:  Called when server tells us that a statistic has changed value.
 */
void StatChange(BYTE group, Statistic *s)
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
void StatsReceiveGroup(BYTE group, list_type l)
{
	if (group == STATS_MAIN)
		StatsMainReceive(l);
	else 
	{
		//	ajw added 5/22/97
		//	This fixes the bug that occurred when the server sends new spells/skills when Inventory is the
		//	current group. (Receiving "main stats" never forces a shift in viewed group.)
		if (group != StatsGetCurrentGroup())
		{
			//	ajw Changes to make Inventory act somewhat like one of the stats groups.
			if( StatsGetCurrentGroup() == STATS_INVENTORY )
			{
				//	Inventory must be going away.
				ShowInventory( False );
			}
		}
		DisplayStatGroup(group, l);
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
int StatsGetCurrentGroup(void)
{
   return current_group;
}

/************************************************************************/
/*
 * DisplayStatGroup:  Display the given group of stats.
 */
void DisplayStatGroup(BYTE group, list_type l)
{
   current_group = group;

   StatsDestroyGroup();
   stats = l;

   if (stats == NULL)
      group_type = GROUP_NONE;
   else group_type = ((Statistic *) (stats->data))->type;

   StatsClearArea();

   StatsCreateGroup();
   InvalidateRect(hStats, NULL, TRUE);
   StatsMove();
}

/************************************************************************/
/*
 * DisplayInventoryAsStatGroup:  ajw - Like DisplayStatGroup, but called when Inventory becomes the shown "group".
 */
void DisplayInventoryAsStatGroup( BYTE group )
{
	current_group = group;
}

/************************************************************************/
/*
 * StatsShowGroup:  ajw - Show or hide controls for a group of stats.
 */
void StatsShowGroup( Bool bShow )
{
	int group_type_temp;              // Type of group currently being displayed
	if (stats == NULL)
		group_type_temp = GROUP_NONE;
	else group_type_temp = ((Statistic *) (stats->data))->type;
	switch( group_type_temp )
	{
	case STATS_NUMERIC:
		ShowStatsNum( bShow, stats );
		break;

	case STATS_LIST:
		ShowStatsList( bShow );
		break;
	}
}
