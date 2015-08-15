// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statlist.c:  Handle list box of statistics.
 * 
 * Only statistics groups with tag STAT_LIST use the list box.
 *
 * A statistic structure is stored in the item data for each entry in the list box.
 *
 * Because the background of the list box is owner drawn to match the background of the main
 * window, we can't let Windows do default list box scrolling for us.  This means that
 * we must also handle the scrollbar ourselves.
 */

#include "client.h"
#include "merintr.h"
#include "skills.h"

static HWND hList;                  // Listbox containing stats
static WNDPROC lpfnDefStatListProc; // Default window procedure for stats list box

extern HWND hStats;

static int num_visible;             // # of stats visible in list box

static int StatListFindItem(int num);
static long CALLBACK StatsListProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
static void StatsListDrawStat(const DRAWITEMSTRUCT *lpdis, Bool selected, Bool bShowSpellIcon );
static int  StatsListGetItemHeight(void);
static void StatsListLButton(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static void StatsListLButtonDblClk(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static void StatsListRButton(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static void StatsListVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
static Bool StatListKey(HWND hwnd, UINT key, Bool fDown, int cRepeat, UINT flags);
/************************************************************************/
/*
 * StatsListCreate:  Create stats list box, and fill it with the given stats.
 */
void StatsListCreate(list_type stats)
{
   list_type l;

   if (StatsGetCurrentGroup() == STATS_QUESTS)
   {
      hList = CreateWindow("listbox", NULL,
         WS_CHILD |
         LBS_HASSTRINGS | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
         0, 0, 0, 0, hStats, (HMENU)IDC_STATLIST, hInst, NULL);
   }
   else
   {
      hList = CreateWindow("listbox", NULL,
         WS_CHILD |
         LBS_HASSTRINGS | LBS_SORT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
         0, 0, 0, 0, hStats, (HMENU)IDC_STATLIST, hInst, NULL);
   }

   lpfnDefStatListProc = SubclassWindow(hList, StatsListProc);

   SetWindowFont(hList, GetFont(FONT_STATS), FALSE);

   for (l = stats; l != NULL; l = l->next)
   {
      int index;
      Statistic *s = (Statistic *) (l->data);
      
      index = ListBox_AddString(hList, LookupNameRsc(s->name_res));
      ListBox_SetItemData(hList, index, s);
   }
}
/************************************************************************/
/*
 * StatsListDestroy:  Destroy stats list box, if it exists.
 */
void StatsListDestroy(list_type stats)
{
   DestroyWindow(hList);
   hList = NULL;
}
/************************************************************************/
/*
 * StatsListResize:  Called when main window resized.
 */
void StatsListResize(list_type stats)
{
   AREA stats_area;
   int y;

   StatsGetArea(&stats_area);

   y = StatsGetButtonBorder();
   MoveWindow(hList, 0, y, stats_area.cx, stats_area.cy - y, TRUE);

   // Hide scrollbar if not needed
   num_visible = (stats_area.cy - y) / max(ListBox_GetItemHeight(hList, 0), 1);
   if (num_visible >= ListBox_GetCount(hList))
     SetScrollRange(hList, SB_VERT, 0, 0, TRUE);
   else
     {
       SetScrollRange(hList, SB_VERT, 0, ListBox_GetCount(hList) - num_visible, TRUE);
       SetScrollPos(hList, SB_VERT, ListBox_GetTopIndex(hList), TRUE);
     }

	if( StatsGetCurrentGroup() != STATS_INVENTORY )			//	ajw
		ShowWindow(hList, SW_NORMAL);
}
/************************************************************************/
/*
 * StatsListChangeStat:  Redisplay current statistic, whose value has changed.
 *   Requires that s is a list type stat in the current group.
 */
void StatsListChangeStat(Statistic *s)
{
   int index, top;

   if (s->num < 0 || s->num > ListBox_GetCount(hList))
   {
      debug(("StatListChangeList got illegal stat #%d\n", (int) s->num));
      return;
   }

   top = ListBox_GetTopIndex(hList);

   index = StatListFindItem(s->num);   
   if (index == -1)
   {
      debug(("StatListChangeList got illegal stat #%d\n", (int) s->num));
      return;
   }

   WindowBeginUpdate(hList);
   ListBox_DeleteString(hList, index);

   index = ListBox_AddString(hList, LookupNameRsc(s->name_res));
   ListBox_SetItemData(hList, index, s);

   ListBox_SetTopIndex(hList, top);
   WindowEndUpdate(hList);
}
/************************************************************************/
/*
 * StatListFindItem:  Return list box index of stat with given number,
 *   or -1 if none.
 */
int StatListFindItem(int num)
{
   int index, num_stats;
   Statistic *s;

   num_stats = ListBox_GetCount(hList);

   for (index = 0; index < num_stats; index++)
   {
      s = (Statistic *) ListBox_GetItemData(hList, index);
      if (s->num == num)
	 return index;
   }

   return -1;
}
/************************************************************************/
/*
 * StatsListProc:  Subclassed window procedure for list box.
 */
long CALLBACK StatsListProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
   case WM_KEYDOWN:
      if (HANDLE_WM_KEYDOWN_BLAK(hwnd, wParam, lParam, StatListKey) == True)
      	 return 0;
      break;

      HANDLE_MSG(hwnd, WM_VSCROLL, StatsListVScroll);

   case WM_ERASEBKGND:
//		StatsClearArea();
	   return 1;

   case WM_LBUTTONDOWN:
      HANDLE_WM_LBUTTONDOWN(hwnd, wParam, lParam, StatsListLButton);
      return 1;        /* Don't select item under cursor */

   case WM_LBUTTONDBLCLK:		//	ajw
      HANDLE_WM_LBUTTONDBLCLK( hwnd, wParam, lParam, StatsListLButtonDblClk );
      return 1;

   case WM_RBUTTONDOWN:
      HANDLE_WM_RBUTTONDOWN(hwnd, wParam, lParam, StatsListRButton);
      return 0;        /* Don't select item under cursor */

   case WM_SETFOCUS:
   case WM_KILLFOCUS:
      StatsDrawBorder();
      break;

//	case WM_CTLCOLORSCROLLBAR:						// ajw
//		return (long)GetStockObject( BLACK_BRUSH );		//	xxx
   }
   return CallWindowProc(lpfnDefStatListProc, hwnd, message, wParam, lParam);
}

/*****************************************************************************/
/* 
 * StatsListMeasureItem:  Message handler for stats list box.
 */
void StatsListMeasureItem(HWND hwnd, MEASUREITEMSTRUCT *lpmis)
{
   lpmis->itemHeight = StatsListGetItemHeight();
}
/*****************************************************************************/
/* 
 * StatsListDrawItem:  Message handler for stats list boxes.  Return TRUE iff
 *   message is handled.
 */
BOOL StatsListDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
{
   AREA stats_area;

   if (hList == NULL)
      return TRUE;

   switch (lpdis->itemAction)
   {
   case ODA_SELECT:
   case ODA_DRAWENTIRE:
      /* If box is empty, do nothing */
      if (lpdis->itemID == -1)
	 return TRUE;

      /* Draw window background at correct offset */
      StatsGetArea(&stats_area);
      DrawWindowBackgroundColor(pinventory_bkgnd(), lpdis->hDC, (RECT *) (&lpdis->rcItem),			//	was NULL
				stats_area.x + lpdis->rcItem.left,
				stats_area.y + lpdis->rcItem.top + StatsGetButtonBorder(), -1);

      /* Draw info on stat */
	  StatsListDrawStat(lpdis, (Bool) (lpdis->itemState & ODS_SELECTED), (Bool)( StatsGetCurrentGroup() == STATS_SPELLS ) );
      break;

   case ODA_FOCUS:
//      DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
      break;
   }
   
   return TRUE;
}
/*****************************************************************************/
/* 
 * StatsListDrawStat:  Draw info about stat in stat list box.
 *   selected is True iff the item is currently selected.
 */
void StatsListDrawStat(const DRAWITEMSTRUCT *lpdis, Bool selected, Bool bShowSpellIcon )
{
   HFONT hOldFont;
   Statistic *s;
   char str[MAXRSCSTRING + 10];
   RECT r;
   int lastItem = ListBox_GetCount(lpdis->hwndItem) - 1;
   RECT rcWnd;

   GetClientRect(lpdis->hwndItem,&rcWnd);
   if (lpdis->rcItem.bottom > rcWnd.bottom)
      return;

   hOldFont = (HFONT) SelectObject(lpdis->hDC, GetFont(FONT_STATS));

   s = (Statistic *) ListBox_GetItemData(hList, lpdis->itemID);
   if (s == NULL)
      return;

   if (StatsGetCurrentGroup() == STATS_QUESTS)
      sprintf(str, "%s", LookupNameRsc(s->name_res));
   else
      sprintf(str, "%s %d%%", LookupNameRsc(s->name_res), s->list.value);

   SetBkMode(lpdis->hDC, TRANSPARENT);

   memcpy(&r, &lpdis->rcItem, sizeof(RECT));
   if (StatsGetCurrentGroup() == STATS_QUESTS && s->list.value == 0)
   {
      r.left += 2;
   }
   else
   {
      r.left += ENCHANT_SIZE + 2;
   }
   // Draw text with drop shadow
   SetTextColor(lpdis->hDC, GetColor(COLOR_STATSBGD));
   //DrawText(lpdis->hDC, str, strlen(str), &r,  DT_CENTER);
   DrawText( lpdis->hDC, str, strlen(str), &r, DT_LEFT );
   OffsetRect(&r, 1, 1);
   if (StatsGetCurrentGroup() == STATS_QUESTS && s->list.value == 0)
   {
      SetTextColor(lpdis->hDC, GetColor(COLOR_QUEST_HEADER));
   }
   else
   {
      SetTextColor(lpdis->hDC, selected ? GetColor(COLOR_HIGHLITE) : GetColor(COLOR_STATSFGD));
   }
   //DrawText(lpdis->hDC, str, strlen(str), &r,  DT_CENTER);
   DrawText( lpdis->hDC, str, strlen(str), &r, DT_LEFT );

   SelectObject(lpdis->hDC, hOldFont);

   switch (StatsGetCurrentGroup())
   {
   AREA areaIcon;
   case STATS_SPELLS:
   case STATS_SKILLS:
   case STATS_QUESTS:
      areaIcon.x = lpdis->rcItem.left;
      areaIcon.y = lpdis->rcItem.top;
      areaIcon.cx = ENCHANT_SIZE;		//xxx
      areaIcon.cy = ENCHANT_SIZE;
      DrawObjectIcon(lpdis->hDC, s->list.icon, 0, True, &areaIcon, NULL, 0, 0, True);
      break;
   }
   if (lastItem == (int)lpdis->itemID)
   {
      RECT rcWnd;
      GetClientRect(lpdis->hwndItem,&rcWnd);
      rcWnd.top = lpdis->rcItem.bottom;
      if (rcWnd.top < rcWnd.bottom)
	 InvalidateRect(lpdis->hwndItem,&rcWnd,TRUE);
   }
}
/*****************************************************************************/
/* 
 * StatsListGetItemHeight:  Return height of an item in the stats list box.
 */
int StatsListGetItemHeight(void)
{
   return GetFontHeight(GetFont(FONT_STATS)) + STATUS_SPACING;
}

/*****************************************************************************/
/*
 * StatsListLButton:  User clicked left button on stats list.
 */
void StatsListLButton(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   int index;

   SetFocus(hwnd);
   index = y / ListBox_GetItemHeight(hwnd, 0) + ListBox_GetTopIndex(hwnd);

   if (index >= ListBox_GetCount(hwnd))
      return;

   ListBox_SetCurSel(hwnd, index);	
}

/*****************************************************************************/
/*
 * StatsListLButtonDblClk:  User double-clicked left button on stats list.			ajw
 */
void StatsListLButtonDblClk(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   char pszLabel[256];
   char pszCommand[256];
   int iLabelLen;
   int index = y / ListBox_GetItemHeight(hwnd, 0) + ListBox_GetTopIndex(hwnd);
   
   if (index >= ListBox_GetCount(hwnd))
      return;

   if ((GetPlayerInfo()->viewID != 0) && (GetPlayerInfo()->viewID != GetPlayerInfo()->id))
   {
      if (!(GetPlayerInfo()->viewFlags & REMOTE_VIEW_CAST))
         return;
   }
   ListBox_SetCurSel(hwnd, index);
   iLabelLen = ListBox_GetTextLen( hwnd, index );

   if( ListBox_GetText( hwnd, index, pszLabel ) != LB_ERR )
   {
      if( StatsGetCurrentGroup() == STATS_SPELLS )
      {
         strcpy(pszCommand, GetString(hInst, IDS_DOCAST));
      }
      else if (StatsGetCurrentGroup() == STATS_SKILLS)
      {
         strcpy( pszCommand, "perform " );
         return; // not implimented yet
      }
      else
      {
         return;  // not going to do anything for non skills/stats
      }
      strcat( pszCommand, pszLabel );
      PerformAction( A_GOTOMAIN, NULL );	//	For targeting icon to appear, focus must be on main view window.
      PerformAction( A_TEXTCOMMAND, pszCommand );
   }
}

/*****************************************************************************/
/*
 * StatsListRButton:  User clicked right button on stats list.
 */
void StatsListRButton(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   int index;
   Statistic *s;

   SetFocus(hwnd);
   index = y / ListBox_GetItemHeight(hwnd, 0) + ListBox_GetTopIndex(hwnd);

   if (index >= ListBox_GetCount(hwnd))
      return;

   ListBox_SetCurSel(hwnd, index);	

   s = (Statistic *) ListBox_GetItemData(hwnd, index);
   if (s == NULL)
      return;

   if (StatsGetCurrentGroup() == STATS_QUESTS && s->list.value == 0)
      return;

   SetDescParams(cinfo->hMain, DESC_NONE);
   RequestLook(s->list.id);
}

/************************************************************************/
/*
 * StatsListCommand:  Handle WM_COMMAND notifications for stats list.
 */
void StatsListCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
   if (codeNotify != LBN_SELCHANGE)
      return;

   InvalidateRect(hList, NULL, TRUE);
}
/************************************************************************/
/*
 * StatsListVScroll:  User did something with stats list scroll bar.
 */
void StatsListVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
   int old_top;  // Current top row index
   int new_top;  // New top row index
   int num_items;

   old_top = ListBox_GetTopIndex(hwnd);
   num_items = ListBox_GetCount(hwnd);

   switch (code)
   {
   case SB_LINEUP:
      new_top = old_top - 1;
      break;

   case SB_LINEDOWN:
      new_top = old_top + 1;
      break;

   case SB_PAGEUP:
      new_top = old_top - num_visible;
      break;

   case SB_PAGEDOWN:
      new_top = old_top + num_visible;
      break;

   case SB_THUMBPOSITION:
      new_top = pos;
      break;

   case SB_THUMBTRACK:
      new_top = pos;
      break;

   case SB_BOTTOM:
      new_top = num_items - num_visible;
      break;

   case SB_TOP:
      new_top = 0;
      break;
      
   default:
      // Pointless "SB_ENDSCROLL" added recently
      return;
   }
   new_top = max(new_top, 0);
   new_top = min(new_top, num_items - num_visible);

   if (new_top != old_top)
   {
      SetScrollPos(hwnd, SB_VERT, new_top, TRUE); 

      WindowBeginUpdate(hwnd);
      ListBox_SetTopIndex(hwnd, new_top);
      WindowEndUpdate(hwnd);
   }
}
/************************************************************************/
/*
 * StatListKey:  User pressed a key on stat list box.
 *   Return True iff key should NOT be passed on to Windows for default processing.
 */
Bool StatListKey(HWND hwnd, UINT key, Bool fDown, int cRepeat, UINT flags)
{
   Bool held_down = (flags & 0x4000) ? True : False;  /* Is key being held down? */
   int old_top, new_top, old_pos, new_pos;

   UserDidSomething();

   old_pos = ListBox_GetCurSel(hwnd);
   new_top = old_top = ListBox_GetTopIndex(hwnd);

   // Handle keys manually to prevent Windows from copying list box background, which would
   // cause background to flash when it's redrawn in the correct location.
   switch (key)
   {
   case VK_UP:
     WindowBeginUpdate(hwnd);
     new_pos = max(0, old_pos - 1);
     if (new_pos < old_top)
       new_top = new_pos;
     ListBox_SetTopIndex(hwnd, new_top);
     ListBox_SetCurSel(hwnd, new_pos);
     SetScrollPos(hwnd, SB_VERT, new_top, TRUE); 
     WindowEndUpdate(hwnd);
     return True;

   case VK_DOWN:
     WindowBeginUpdate(hwnd);
     new_pos = min(old_pos + 1, ListBox_GetCount(hwnd) - 1);
     if (new_pos > old_top + num_visible)
       new_top = new_pos - num_visible;
     ListBox_SetTopIndex(hwnd, new_top);
     ListBox_SetCurSel(hwnd, new_pos);
     SetScrollPos(hwnd, SB_VERT, new_top, TRUE); 
     WindowEndUpdate(hwnd);
     return True;

   case VK_PRIOR:
     WindowBeginUpdate(hwnd);
     new_pos = max(0, old_pos - num_visible);
     new_top = new_pos;
     ListBox_SetTopIndex(hwnd, new_top);
     ListBox_SetCurSel(hwnd, new_pos);
     SetScrollPos(hwnd, SB_VERT, new_top, TRUE); 
     WindowEndUpdate(hwnd);
     return True;

   case VK_NEXT:
     WindowBeginUpdate(hwnd);
     new_pos = min(old_pos + num_visible, ListBox_GetCount(hwnd) - 1);
     new_top = new_pos - num_visible;
     ListBox_SetTopIndex(hwnd, new_top);
     ListBox_SetCurSel(hwnd, new_pos);
     SetScrollPos(hwnd, SB_VERT, new_top, TRUE); 
     WindowEndUpdate(hwnd);
     return True;

	case VK_RETURN:	//	ajw
		{
			int iLabelLen = ListBox_GetTextLen( hwnd, old_pos );
			char* pszLabel = (char*)SafeMalloc( ( iLabelLen + 1 ) * sizeof( char ) );
			if( ListBox_GetText( hwnd, old_pos, pszLabel ) != LB_ERR )
			{
				//	ajwxxx Hard-coded group number constants for spells list. There may be a var that could be used instead.
				char* pszCommand;
				if( StatsGetCurrentGroup() == STATS_SPELLS )
				{
					pszCommand = (char*)SafeMalloc( ( iLabelLen + 6 ) * sizeof( char ) );
					strcpy( pszCommand, GetString(hInst,IDS_DOCAST));
				}
				else
				{
					pszCommand = (char*)SafeMalloc( ( iLabelLen + 9 ) * sizeof( char ) );
					strcpy( pszCommand, "perform " );
				}
				strcat( pszCommand, pszLabel );
				PerformAction( A_GOTOMAIN, NULL );	//	For targeting icon to appear, focus must be on main view window.
				PerformAction( A_TEXTCOMMAND, pszCommand );
				SafeFree( pszCommand );
			}
			SafeFree( pszLabel );
		}
		return True;
   }

   return StatInputKey(hwnd, key, fDown, cRepeat, flags);
}

/************************************************************************/
/*
 * ShowStatsList: Added by ajw.
 */
void ShowStatsList( Bool bShow )
{
	ShowWindow( hList, bShow ? SW_SHOW : SW_HIDE );
//	StatsClearArea();
}
