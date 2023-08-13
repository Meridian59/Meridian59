// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * inventry.c:  Handle inventory area on main window.
 *
 * A relative position in the inventory is the (row, column) location of the item on the
 * screen.  The absolute position of the item is then (row + top_row, column).
 */

#include "client.h"
#include "merintr.h"

static HWND hwndInvDialog;      /* Inventory modeless dialog */
static HWND hwndInv;            /* Inventory button (main area) */
static HWND hwndInvScroll;      /* Inventory scroll bar */
static WNDPROC lpfnDefProc;     /* Default event procedure for inventory */

/* Position & size of inventory */
static AREA inventory_area;
static int  inventory_scrollbar_width;

static int inventory_bg_index;  // Palette index of color to use for window bg border

typedef struct {
   object_node *obj;            // Pointer to node for object in inventory list
   Bool         is_using;       // True iff item is in use
} InvItem;

static list_type items;         // List of InvItem structures for items in inventory
static int       num_items;     // # of items currently in inventory

static int rows, cols;          // # of rows and columns visible in inventory area
static int top_row;             // Row # currently displayed at top of inventory

static int cursor_row, cursor_col;  // Absolute position of currently selected item
static Bool has_scrollbar;      // True when scroll bar displayed

static Bool capture;            // True when inventory has mouse capture

static BYTE *cursor_bits;       // Bitmap for inventory cursor
BYTE *inuse_bits;        // Bitmap for in-use highlight
BYTE	*selftrgt_bits;

static RawBitmap inventory_bkgnd;              // Background bitmap for inventory area

static HBRUSH	hbrushScrollBack;

/* Keys to override default actions */
keymap inventory_key_table[] = {
{ VK_UP,          KEY_ANY,              A_CURSORUP },
{ VK_DOWN,        KEY_ANY,              A_CURSORDOWN },
{ VK_LEFT,        KEY_ANY,              A_CURSORLEFT },
{ VK_RIGHT,       KEY_ANY,              A_CURSORRIGHT }, 
{ VK_PRIOR,       KEY_ANY,              A_CURSORUPRIGHT }, 
{ VK_HOME,        KEY_ANY,              A_CURSORUPLEFT }, 
{ VK_NEXT,        KEY_ANY,              A_CURSORDOWNRIGHT }, 
{ VK_END,         KEY_ANY,              A_CURSORDOWNLEFT }, 
{ VK_NUMPAD8,     KEY_ANY,              A_CURSORUP },
{ VK_NUMPAD2,     KEY_ANY,              A_CURSORDOWN },
{ VK_NUMPAD4,     KEY_ANY,              A_CURSORLEFT },
{ VK_NUMPAD6,     KEY_ANY,              A_CURSORRIGHT }, 
{ VK_NUMPAD9,     KEY_ANY,              A_CURSORUPRIGHT }, 
{ VK_NUMPAD7,     KEY_ANY,              A_CURSORUPLEFT }, 
{ VK_NUMPAD3,     KEY_ANY,              A_CURSORDOWNRIGHT }, 
{ VK_NUMPAD1,     KEY_ANY,              A_CURSORDOWNLEFT }, 
{ VK_SPACE,       KEY_ANY,              A_TOGGLEUSE },

{ VK_RBUTTON,     KEY_NONE,             A_LOOKINVENTORY },
{ VK_LDBLCLK,     KEY_NONE,             A_TOGGLEUSE },

{ VK_TAB,         KEY_NONE,             A_TABFWD,     (void *) IDC_INVENTORY },
{ VK_TAB,         KEY_SHIFT,            A_TABBACK,    (void *) IDC_INVENTORY },
{ VK_ESCAPE,      KEY_ANY,              A_GOTOMAIN },
{ VK_DELETE,      KEY_NONE,             A_DROP },
{ 'L',            KEY_NONE,             A_LOOKINVENTORY },
{ 'P',            KEY_NONE,             A_PUT },
{ 'R',            KEY_NONE,             A_TOGGLEUSE },
{ 'U',            KEY_NONE,             A_TOGGLEUSE },

{ VK_F1,          KEY_NONE,             A_TEXTCOMMAND, },  // These keys filled in by aliases
{ VK_F2,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F3,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F4,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F5,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F6,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F7,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F8,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F9,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F10,         KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F11,         KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F12,         KEY_NONE,             A_TEXTCOMMAND, },

{ 0, 0, 0},   // Must end table this way
};

/* local function prototypes */
static LRESULT CALLBACK InventoryProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK InventoryDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static Bool InventoryKey(HWND hwnd, UINT key, Bool fDown, int cRepeat, UINT flags);
static void InventoryLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static void InventoryLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
static void InventoryRButton(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static Bool InventoryClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static void ToggleUse(ID obj_id);
static void InventoryDisplayScrollbar(void);
static void InventoryScrollRange(void);
static Bool InventoryCompareIdItem(void *idnum, void *item);
static void InventoryDrawSingleItem(InvItem *item, int row, int col);
static void InventoryRedrawSingleItem(InvItem *item);
static InvItem *InventoryGetCurrentItem(void);
static ID   InventoryGetCurrentId(void);
static Bool InventoryItemVisible(int row, int col);
static void InventoryCursorMove(int action);
static Bool InventoryReleaseCapture(void);
static Bool InventoryDropCurrentItem(room_contents_node *container);
static Bool InventoryMoveCurrentItem(int x, int y);
static void InventoryVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
static void InventoryComputeRowsCols(void);

/************************************************************************/
/*
 * InventoryBoxCreate:  Create the inventory list box.
 */
void InventoryBoxCreate(HWND hParent)
{
   BITMAPINFOHEADER *ptr;
	LOGBRUSH logbrush;

   hwndInvDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_INVENTORY), 
				hParent, InventoryDialogProc);

   hwndInv = CreateWindow("button", NULL, 
			  WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			  0, 0, 0, 0,
			  hwndInvDialog, (HMENU) IDC_INVENTORY, hInst, NULL);
   lpfnDefProc = SubclassWindow(hwndInv, InventoryProc);

   hwndInvScroll = CreateWindow("scrollbar", NULL, 
				WS_CHILD | SBS_VERT,
				0, 0, 100, 100,  /* Make sure scrollbar drawn ok */
				hwndInvDialog, (HMENU) IDC_INVSCROLL, hInst, NULL);

   inventory_scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
   num_items = 0;
   top_row = 0;
   cursor_row = cursor_col = -1;
   items = NULL;
   capture = False;

   // Get bitmaps for cursor and in-use highlight
   ptr = GetBitmapResource(hInst, IDB_INVCURSOR);
   if (ptr == NULL)
      cursor_bits = NULL;
   else cursor_bits = ((BYTE *) ptr) + sizeof(BITMAPINFOHEADER) + NUM_COLORS * sizeof(RGBQUAD);
      
   ptr = GetBitmapResource(hInst, IDB_INVINUSE);
   if (ptr == NULL)
      inuse_bits = NULL;
   else inuse_bits = ((BYTE *) ptr) + sizeof(BITMAPINFOHEADER) + NUM_COLORS * sizeof(RGBQUAD);

   ptr = GetBitmapResource(hInst, IDB_SELFTRGT);
   if (ptr == NULL)
      selftrgt_bits = NULL;
   else selftrgt_bits = ((BYTE *) ptr) + sizeof(BITMAPINFOHEADER) + NUM_COLORS * sizeof(RGBQUAD);

   if (!GetBitmapResourceInfo(hInst, IDB_INVBKGND, &inventory_bkgnd))
     debug(("InventoryBoxCreate couldn't load inventory background bitmap\n"));

	if( !( ptr = GetBitmapResource( hInst, IDB_INVBKGND ) ) )
		debug(("InventoryBoxCreate couldn't load inventory scroll bar texture bitmap\n"));

	logbrush.lbStyle = BS_DIBPATTERNPT;
	logbrush.lbColor = DIB_RGB_COLORS;
	logbrush.lbHatch = (ULONG_PTR) ptr;
	
	hbrushScrollBack = CreateBrushIndirect( &logbrush );
	//	ajw end...

   // Compute background palette index
   inventory_bg_index = GetClosestPaletteIndex(RGB(84, 40, 0));

   InventoryResetFont();
   InventoryChangeColor();
}
/************************************************************************/
/*
 * InventoryBoxDestroy:  Destroy the inventory list box.
 */
void InventoryBoxDestroy(void)
{
   DestroyWindow(hwndInv);
   DestroyWindow(hwndInvDialog);
   DestroyWindow(hwndInvScroll);

   InventoryResetData();

//	DeleteObject( hbmpScrollBack );
	DeleteObject( hbrushScrollBack );
}
/************************************************************************/
/*
 * InventoryResetData:  Free item structures for all current items in the inventory.
 */
void InventoryResetData(void)
{
   items = list_destroy(items);
}
/************************************************************************/
/*
 * InventoryBoxResize:  Resize the inventory box when the main window is resized
 *   to (xsize, ysize).  view is the current grid area view.
 */
void InventoryBoxResize(int xsize, int ysize, AREA *view)
{
   int old_cols = cols;
   int old_rows = rows;

   int yMiniMap, iHeightAvailableForMapAndStats, iHeightMiniMap;

   /* Turn off highlight */
   DrawBorder(&inventory_area, inventory_bg_index, NULL);
   
   inventory_area.x = view->x + view->cx + LEFT_BORDER + 3 * HIGHLIGHT_THICKNESS;
   inventory_area.cx = xsize - inventory_area.x - 3 * HIGHLIGHT_THICKNESS - EDGETREAT_WIDTH;

   	yMiniMap = 2 * TOP_BORDER + USERAREA_HEIGHT + EDGETREAT_HEIGHT + MAPTREAT_HEIGHT;
	iHeightAvailableForMapAndStats = ysize - yMiniMap - 2 * HIGHLIGHT_THICKNESS - EDGETREAT_HEIGHT;
	iHeightMiniMap = (int)( iHeightAvailableForMapAndStats * PROPORTION_MINIMAP ) - HIGHLIGHT_THICKNESS - MAPTREAT_HEIGHT;

	inventory_area.y = yMiniMap + iHeightMiniMap + 3 * HIGHLIGHT_THICKNESS + MAPTREAT_HEIGHT + GROUPBUTTONS_HEIGHT + MAP_STATS_GAP_HEIGHT + 1;
	inventory_area.cy = ysize - EDGETREAT_HEIGHT - HIGHLIGHT_THICKNESS - inventory_area.y - STATS_BOTTOM_GAP_HEIGHT;

   InventoryComputeRowsCols();

   // If inventory gets bigger, go back to top
   if (cols > old_cols || rows > old_rows)
   {
      top_row = 0;
      SetScrollPos(hwndInvScroll, SB_CTL, 0, TRUE); 
   }

   InventoryDisplayScrollbar();
}
/************************************************************************/
/* 
 * InventoryDisplayScrollbar:  Determine whether a scrollbar is necessary,
 *   and draw it if so.  Also may involve moving the inventory box around.
 */
void InventoryDisplayScrollbar(void)
{
   ShowWindow(hwndInvDialog, SW_HIDE);  /* Hide scrollbar ugliness */
   ShowWindow(hwndInvScroll, SW_HIDE); 

   has_scrollbar = (num_items > rows * cols);

   InventoryComputeRowsCols();

   MoveWindow(hwndInvDialog, inventory_area.x, inventory_area.y, 
	      inventory_area.cx, inventory_area.cy,
	      FALSE);

   MoveWindow(hwndInv, 0, 0, 
	      inventory_area.cx - inventory_scrollbar_width, inventory_area.cy,
	      FALSE);

   MoveWindow(hwndInvScroll, inventory_area.cx - inventory_scrollbar_width,
	      0, inventory_scrollbar_width,
	      inventory_area.cy,
	      TRUE);

   InventoryScrollRange();
	if( StatsGetCurrentGroup() == STATS_INVENTORY )		//	ajw
	{
		ShowWindow(hwndInvDialog, SW_SHOWNORMAL);
		ShowWindow(hwndInvScroll, has_scrollbar ? SW_SHOWNORMAL : SW_HIDE);
	}
}
/************************************************************************/
/* 
 * InventoryComputeRowsCols:  Compute number of visible rows and columns in the
 *   inventory area, assuming that has_scrollbar has been set correctly.
 */
void InventoryComputeRowsCols(void)
{
   int width;

   rows = inventory_area.cy / INVENTORY_BOX_HEIGHT;

   width = inventory_area.cx;
   if (has_scrollbar)
      width -= inventory_scrollbar_width;

   cols = max(width / INVENTORY_BOX_WIDTH, 1);
}
/************************************************************************/
/*
 * InventoryScrollRange:  Set range of scroll bar to current # of items.
 */
void InventoryScrollRange(void)
{
   /* Max is when last item is on bottom of list */
   if (cols != 0)
      SetScrollRange(hwndInvScroll, SB_CTL, 0, (num_items + cols - 1) / cols - rows, TRUE);
}
/************************************************************************/
/*
 * InventoryVScroll:  User did something with inventory scroll bar.
 */
void InventoryVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
   int new_top;  // New top row index

   switch (code)
   {
   case SB_LINEUP:
      new_top = top_row - 1;
      break;

   case SB_LINEDOWN:
      new_top = top_row + 1;
      break;

   case SB_PAGEUP:
      new_top = top_row - rows;
      break;

   case SB_PAGEDOWN:
      new_top = top_row + rows;
      break;

   case SB_THUMBPOSITION:
      new_top = pos;
      break;

   case SB_THUMBTRACK:
      new_top = pos;
      break;

   case SB_BOTTOM:
      new_top = (num_items + cols - 1) / cols - rows;
      break;

   case SB_TOP:
      new_top = 0;
      break;
      
   default:
      // Pointless "SB_ENDSCROLL" added recently
      return;
   }
   new_top = max(new_top, 0);
   new_top = min(new_top, (num_items + cols - 1) / cols - rows);

   if (new_top != top_row)
   {
      top_row = new_top;
      InventoryRedraw();
      SetScrollPos(hwndInvScroll, SB_CTL, top_row, TRUE); 
   }
}
/************************************************************************/
/*
 * InventoryResetFont:  Called when a font has changed.
 */
void InventoryResetFont(void)
{
}
/************************************************************************/
/*
 * InventoryChangeColor:  Called when a color has changed.
 */
void InventoryChangeColor(void)
{
}
/************************************************************************/
void InventorySetFocus(Bool forward)
{
   SetFocus(hwndInv);
}
/************************************************************************/
/*
 * InventoryDialogProc:  Dialog procedure for inventory modeless dialog.
 */
INT_PTR CALLBACK InventoryDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_INITDIALOG:
      ShowWindow(hwnd, SW_HIDE);
      return FALSE;

   case WM_ERASEBKGND:
      return 1;

      HANDLE_MSG(hwnd, WM_VSCROLL, InventoryVScroll);

   case WM_DRAWITEM:
      InventoryRedraw();
      return TRUE;

   case WM_ACTIVATE:
     if (wParam == 0)
       *cinfo->hCurrentDlg = NULL;
     else *cinfo->hCurrentDlg = hwnd;
     return TRUE;
   }

   return FALSE;
}
/************************************************************************/
/*
 * InventoryProc:  Subclassed window procedure for inventory area.
 */
LRESULT CALLBACK InventoryProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   RECT r;
   HDC hdc;

   switch (message)
   {
   case WM_KEYDOWN:
      if (HANDLE_WM_KEYDOWN_BLAK(hwnd, wParam, lParam, InventoryKey))
      	 return 0;
   case WM_ERASEBKGND:
     // Draw outside of inventory items
     // XXX
     hdc = (HDC) wParam;
     r.left = cols * INVENTORY_BOX_WIDTH;
     r.right = inventory_area.cx;
     r.top = 0;
     r.bottom = inventory_area.cy;
     if (has_scrollbar)
       r.right -= inventory_scrollbar_width;
     DrawWindowBackgroundColor(&inventory_bkgnd, hdc, &r, r.left + inventory_area.x, r.top + inventory_area.y,
			       -1);
     
     r.left = 0;
     r.right = inventory_area.cx;
     if (has_scrollbar)
       r.right -= inventory_scrollbar_width;
     r.top = rows * INVENTORY_BOX_HEIGHT;
     r.bottom = inventory_area.cy;
     DrawWindowBackgroundColor(&inventory_bkgnd, hdc, &r, r.left + inventory_area.x, r.top + inventory_area.y,
			       -1);
     return 1;
      
      HANDLE_MSG(hwnd, WM_LBUTTONDOWN, InventoryLButtonDown);
      HANDLE_MSG(hwnd, WM_RBUTTONDOWN, InventoryRButton);
      HANDLE_MSG(hwnd, WM_LBUTTONDBLCLK, InventoryLButtonDown);
      HANDLE_MSG(hwnd, WM_LBUTTONUP, InventoryLButtonUp);

   case WM_SETFOCUS:
	  StatsDrawBorder();
      break;
   case WM_KILLFOCUS:
	  StatsDrawBorder();
      InventoryReleaseCapture();
      break;
   case WM_SYSKEYDOWN:
      // Prevent activation of the window's menu bar and propagate to parent
      if (wParam == VK_F10)
      {
          if (HANDLE_WM_KEYDOWN_BLAK(hwnd, wParam, lParam, InventoryKey) == True)
              return 0;
          break;
      }
      break;
   }

   return CallWindowProc(lpfnDefProc, hwnd, message, wParam, lParam);
}

/************************************************************************/
/*
 * InventoryDrawItem:  Handle WM_DRAWITEM messages for inventory.
 */
Bool InventoryDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
{
   InventoryRedraw();
   return True;
}
/************************************************************************/
/*
 * InventoryRedraw:  Redraw entire inventory.
 */
void InventoryRedraw(void)
{
   InvItem *item;
   int i, offset, row, col;
   list_type l;
   HDC hdc;
   RECT r;

   offset = top_row * cols;
   l = items;
   for (i=0; i < offset; i++)
     if (l != NULL)
       l = l->next;

   for (i = 0; i + offset < num_items; i++)
   {
      if (l == NULL)
	 break;

      item = (InvItem *) (l->data);

      row = i / cols;
      col = i % cols;

      if (row >= rows)
	 return;

      InventoryDrawSingleItem(item, i / cols, i % cols);
      
      l = l->next;
   }

   // Clear remaining empty part of inventory
   hdc = GetDC(hwndInv);
   for ( ; i < rows * cols; i++)
   {
      r.left   = (i % cols) * INVENTORY_BOX_WIDTH;
      r.top    = (i / cols) * INVENTORY_BOX_HEIGHT;
      r.right  = r.left + INVENTORY_BOX_WIDTH;
      r.bottom = r.top + INVENTORY_BOX_WIDTH;
      DrawWindowBackgroundColor(&inventory_bkgnd, hdc, &r, r.left + inventory_area.x, r.top + inventory_area.y,
				-1);
   }

     r.left = cols * INVENTORY_BOX_WIDTH;
     r.right = inventory_area.cx;
     r.top = 0;
     r.bottom = inventory_area.cy;
     if (has_scrollbar)
       r.right -= inventory_scrollbar_width;
     DrawWindowBackgroundColor(&inventory_bkgnd, hdc, &r, r.left + inventory_area.x, r.top + inventory_area.y,
			       -1);
     
     r.left = 0;
     r.right = inventory_area.cx;
     if (has_scrollbar)
       r.right -= inventory_scrollbar_width;
     r.top = rows * INVENTORY_BOX_HEIGHT;
     r.bottom = inventory_area.cy;
     DrawWindowBackgroundColor(&inventory_bkgnd, hdc, &r, r.left + inventory_area.x, r.top + inventory_area.y,
			       -1);

   ReleaseDC(hwndInv, hdc);
}
/************************************************************************/
/*
 * InventoryDrawSingleItem:  Draw given inventory item, at given row and col
 *   in relative coordinates.
 */
void InventoryDrawSingleItem(InvItem *item, int row, int col)
{
   HDC hdc;
   AREA area, obj_area;
   char temp[MAXAMOUNT + 1];
   Bool draw_cursor;
   
   area.x = col * INVENTORY_BOX_WIDTH;
   area.y =  row * INVENTORY_BOX_HEIGHT;
   area.cx = INVENTORY_BOX_WIDTH;
   area.cy = INVENTORY_BOX_HEIGHT;

   obj_area.x  = area.x + INVENTORY_OBJECT_BORDER;
   obj_area.y  = area.y + INVENTORY_OBJECT_BORDER;
   obj_area.cx = INVENTORY_OBJECT_WIDTH - 1;
   obj_area.cy = INVENTORY_OBJECT_HEIGHT - 1;

   hdc = GetDC(hwndInv);

   // See if we should draw cursor here
   draw_cursor = (GetFocus() == hwndInv && row == cursor_row - top_row && col == cursor_col);
   if (draw_cursor)
      OffscreenBitBlt(hdc, 0, 0, INVENTORY_BOX_WIDTH, INVENTORY_BOX_HEIGHT, 
		      cursor_bits, 0, 0, INVENTORY_BOX_WIDTH, OBB_FLIP);
   else OffscreenWindowBackground(&inventory_bkgnd, 
				  inventory_area.x + area.x, inventory_area.y + area.y, 
				  INVENTORY_BOX_WIDTH, INVENTORY_BOX_HEIGHT);

   if (item->is_using)
      OffscreenBitBlt(hdc, INVENTORY_OBJECT_BORDER, INVENTORY_OBJECT_BORDER, 
		      INVENTORY_OBJECT_WIDTH, INVENTORY_OBJECT_HEIGHT, 
		      inuse_bits, 0, 0, INVENTORY_OBJECT_WIDTH, OBB_FLIP | OBB_TRANSPARENT);

   DrawObject(hdc, item->obj, item->obj->animate->group, True, &obj_area, NULL, 
	      INVENTORY_OBJECT_BORDER, INVENTORY_OBJECT_BORDER, 0, False);

   OffscreenCopy(hdc, area.x, area.y, INVENTORY_BOX_WIDTH, INVENTORY_BOX_HEIGHT, 0, 0);

   // Draw numbers for number items
   if (IsNumberObj(item->obj->id) && cinfo->config->inventory_num)
   {
      sprintf(temp, "%d", item->obj->amount);

      SetBkMode(hdc, TRANSPARENT);
      SelectObject(hdc, GetFont(FONT_STATNUM));

      SetTextColor(hdc, GetColor(COLOR_INVNUMBGD));
      TextOut(hdc, obj_area.x + 1, obj_area.y + 1, temp, (int) strlen(temp));
      SetTextColor(hdc, GetColor(COLOR_INVNUMFGD));
      TextOut(hdc, obj_area.x, obj_area.y, temp, (int) strlen(temp));      
   }

   // Draw border around area to clear previous cursor (if any)
   if (!draw_cursor)
   {
      DrawWindowBackgroundBorder(&inventory_bkgnd, hdc, &obj_area, INVENTORY_OBJECT_BORDER, 
				 inventory_area.x + obj_area.x, inventory_area.y + obj_area.y, -1, NULL);
   }

   ReleaseDC(hwndInv, hdc);
}
/************************************************************************/
/*
 * InventoryRedrawSingleItem:  Redraw given object in inventory after it has changed.
 */
void InventoryRedrawSingleItem(InvItem *item)
{
   int index = 0;
   list_type l;

   for (l = items; l != NULL; l = l->next)
   {
      InvItem *temp = (InvItem *) (l->data);
      if (item->obj->id == temp->obj->id)
      {
	 int row = index / cols;
	 int col = index % cols;
	 
	 if (InventoryItemVisible(row, col))
	    InventoryDrawSingleItem(item, row - top_row, col);

	 return;
      }
      index++;
   }
}
/************************************************************************/
/*
 * InventoryLButtonDown:  User pressed left mouse button on inventory box.
 */
void InventoryLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   if (fDoubleClick)
   {
      InventoryKey(hwnd, VK_LDBLCLK, True, 0, keyFlags);
      return;
   }

   // Set cursor to correct place
   if (InventoryClick(hwnd, fDoubleClick, x, y, keyFlags))
   {
      // Handle user action
      InventoryKey(hwnd, VK_LBUTTON, True, 0, keyFlags);

      SetCapture(hwndInv);
      capture = True;
      SetMainCursor(LoadCursor(cinfo->hInst, MAKEINTRESOURCE(IDC_DROPCURSOR)));
   }
}
/************************************************************************/
/*
 * InventoryLButtonUp:  User released left mouse button on inventory box.
 */
void InventoryLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
   int temp_x, temp_y;
   room_contents_node *r;
   POINT mouse;
   AREA inventory_area;

   InventoryGetArea(&inventory_area);
   GetCursorPos(&mouse);
   ScreenToClient(cinfo->hMain, &mouse);

   if (!InventoryReleaseCapture())
      return;

   // If button was released in inventory area, move selected item to new location in inventory list
   if (IsInArea(&inventory_area, mouse.x, mouse.y))
      {
         InventoryMoveCurrentItem(x, y);
         return;
      }

   // See if mouse pointer is in main graphics area
   if (!MouseToRoom(&temp_x, &temp_y))
      return;

   // See if a container is under mouse pointer
   r = GetObjectByPosition(temp_x, temp_y, CLOSE_DISTANCE, OF_CONTAINER, 0);
   
   if (r == NULL)
      // Drop currently selected object (if any)
      InventoryDropCurrentItem(NULL);
   else
      InventoryDropCurrentItem(r);
}
/************************************************************************/
/*
 * InventoryRButton:  User pressed right mouse button on inventory box.
 */
void InventoryRButton(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   // Set cursor to correct place
   if (InventoryClick(hwnd, fDoubleClick, x, y, keyFlags))
   {
      // Handle user action
      InventoryKey(hwnd, VK_RBUTTON, True, 0, keyFlags);
   }
}
/************************************************************************/
/*
 * InventoryReleaseCapture:  Release mouse capture, if inventory has it.
 *   Return True iff inventory had mouse capture.
 */
Bool InventoryReleaseCapture(void)
{
   if (!capture)
      return False;

   ReleaseCapture();
   capture = False;
   GameWindowSetCursor();
   return True;
}
/************************************************************************/
/*
 * InventoryClick:  User clicked on inventory box.
 *   Return True iff processing of click should continue.
 */
Bool InventoryClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   int row, col, old_row, old_col, index;
   InvItem *item;

   // Find row and col in absolute coordinates
   col = x / INVENTORY_BOX_WIDTH;
   row = top_row + y / INVENTORY_BOX_HEIGHT;

   switch (GameGetState())
   {
   case GAME_PLAY:
      SetFocus(hwnd);
      if (!InventoryItemVisible(row, col))
	 return False;

      old_row = cursor_row;
      old_col = cursor_col;
      
      cursor_row = row;
      cursor_col = col;
      
      // Redraw cursor
      if (InventoryItemVisible(old_row, old_col))
      {
         item = (InvItem *) list_nth_item(items, old_row * cols + old_col);
         if (item != NULL)
            InventoryDrawSingleItem(item, old_row - top_row, old_col);
      }
      
      item = (InvItem *) list_nth_item(items, cursor_row * cols + cursor_col);
      if (item != NULL)
         InventoryDrawSingleItem(item, cursor_row - top_row, cursor_col);
      break;
      
   case GAME_SELECT:
      if (InventoryItemVisible(row, col))
      {
         /* User clicked on an object--get its id */
         index = row * cols + col;
         item = (InvItem *) list_nth_item(items, index);
         if (item != NULL)
            SelectedObject(item->obj->id);
      }
      return False;
   }
   return True;
}      
/************************************************************************/
/*
 * InventoryKey:  User pressed a key on the inventory list.  
 *   Return True iff key should NOT be passed on to Windows for default processing.
 */
Bool InventoryKey(HWND hwnd, UINT key, Bool fDown, int cRepeat, UINT flags)
{
   ID id;
   Bool held_down = (flags & 0x4000) ? True : False;  /* Is key being held down? */
   int action, params;
   InvItem *item;
   void *action_data;
   Bool inform;

   UserDidSomething();
   
   /* See if inventory handles this key specially */
   action = TranslateKey(key, inventory_key_table, &action_data); 
   if (action == A_NOACTION)
      return False;

   item = InventoryGetCurrentItem();
   if (item == NULL)
      id = INVALID_ID;
   else id = item->obj->id;

   // See if we should inform modules about event
   inform = True;
   switch (action)
   {
   case A_TABFWD:
   case A_TABBACK:
   case A_TOGGLEUSE:
   case A_USE:
   case A_UNUSE:
   case A_DROP:
   case A_APPLY:
   case A_LOOKINVENTORY:
   case A_SELECT:
      action_data = (void *) id;
      break;

   default:
      inform = False;   // Modules will be informed in PerformAction call below
   }

   // See if a module wants to handle this action
   if (inform)
      if (ModuleEvent(EVENT_USERACTION, action, action_data) == False)
	 return True;

   if (IsCursorAction(action))
   {
      InventoryCursorMove(action);
      return True;
   }

   switch (action)
   {
   case A_TABFWD:
		TextInputSetFocus(True);	//	ajw
		break;

   case A_TABBACK:
      SetFocus(cinfo->hMain);
      break;

   case A_TOGGLEUSE:
      if (id != INVALID_ID)
      {
	 if (item->obj->flags & OF_APPLYABLE)
	    StartApply(id);
	 else ToggleUse(id);
      }
      break;

   case A_USE:
      if (id != INVALID_ID)
	 RequestUse(id);
      break;

   case A_UNUSE:
      if (id != INVALID_ID)
	 RequestUnuse(id);
      break;
      
   case A_DROP:
      if (id != INVALID_ID)
	 InventoryDropCurrentItem(NULL);
      break;
      
   case A_APPLY:
      if (id != INVALID_ID)
	 StartApply(id);
      break;

   case A_LOOKINVENTORY:
      if (id == INVALID_ID)
	 break;

      params = DESC_DROP;
      if (item->is_using)
	 params |= DESC_UNUSE;
      else 
	 if (item->obj->flags & OF_APPLYABLE)
	    params |= DESC_APPLY;
	 else params |= DESC_USE;
      SetDescParams(cinfo->hMain, params);
      RequestLook(id);
      break;

   case A_SELECT:
      if (id != INVALID_ID)
	 SelectedObject(id);
      break;

   default:
      PerformAction(action, action_data);
      break;
   }
   return True;
}
/************************************************************************/
/*
 * InventoryCursorMove:  User requests the given cursor movement action.
 *   Move cursor.
 */
void InventoryCursorMove(int action)
{
   int dx = 0, dy = 0;
   int old_row, old_col, new_row, new_col;
   InvItem *item;

   switch (action)
   {
   case A_CURSORUP:         dy = -1; break;
   case A_CURSORDOWN:       dy = +1; break;
   case A_CURSORLEFT:       dx = -1; break;
   case A_CURSORRIGHT:      dx = +1; break;
   case A_CURSORUPLEFT:     dx = -1; dy = -1; break;
   case A_CURSORUPRIGHT:    dx = +1; dy = -1; break;
   case A_CURSORDOWNLEFT:   dx = -1; dy = +1; break;
   case A_CURSORDOWNRIGHT:  dx = +1; dy = +1; break;
   default: return;
   }

   new_col = (cursor_col + dx) % cols;
   if (new_col < 0)
      new_col += cols;

   new_row = max(0, cursor_row + dy);
   
   // See if we're going off end of inventory
   if (new_row * cols + new_col >= num_items)
      return;

   old_row = cursor_row;
   old_col = cursor_col;

   cursor_col = new_col;
   cursor_row = new_row;

   // Scroll inventory if necessary
   if (!InventoryItemVisible(cursor_row, cursor_col))
   {
      top_row += dy;
      InventoryRedraw();
      SetScrollPos(hwndInvScroll, SB_CTL, top_row, TRUE); 
   }
   else // Otherwise, redraw cursor to show movement
   {
      item = (InvItem *) list_nth_item(items, old_row * cols + old_col);
      if (item != NULL)
         InventoryDrawSingleItem(item, old_row - top_row, old_col);
      item = (InvItem *) list_nth_item(items, cursor_row * cols + cursor_col);
      if (item != NULL)
         InventoryDrawSingleItem(item, cursor_row - top_row, cursor_col);
   }
}

/************************************************************************/
/*
 * ToggleUse: Ask server to toggle usage state of given object.
 */
void ToggleUse(ID obj_id)
{
   InvItem *item;

   if (num_items == 0)
      return;

   item = (InvItem *) list_find_item(items, (void *) obj_id, InventoryCompareIdItem);
   if (item == NULL)
      return;  /* Not an error, since we try to unuse everything we drop */

   if (item->is_using)
      RequestUnuse(obj_id);
   else RequestUse(obj_id);
}
/************************************************************************/
/*
 * InventoryAddItem:  Add the given object to the inventory display.
 * // XXX Maybe should pass in "using" status
 */
void InventoryAddItem(object_node *obj)
{
   InvItem *new_item;

   new_item = (InvItem *) SafeMalloc(sizeof(InvItem));
   new_item->obj = obj;
   new_item->is_using = False;
   items = list_add_item(items, new_item);

   num_items++;
   InventoryScrollRange();
   /* See if we should add scroll bar */
   if (num_items == rows * cols + 1)
      InventoryDisplayScrollbar();

   InventoryRedrawSingleItem(new_item);
}
/************************************************************************/
/*
 * InventoryRemoveItem:  Remove the object with the given id from the
 *   inventory display.
 */
void InventoryRemoveItem(ID id)
{
   InvItem *item;
   
   WindowBeginUpdate(hwndInv);

   item = (InvItem *) list_find_item(items, (void *) id, InventoryCompareIdItem);
   if (item == NULL)
   {
      debug(("Tried to remove nonexistent object %ld from inventory list", id));
      return;
   }

   items = list_delete_item(items, (void *) id, InventoryCompareIdItem);
   // Object itself freed elsewhere
   SafeFree(item);

   num_items--;
   InventoryScrollRange();
   /* See if we should remove scroll bar */
   if (num_items == rows * cols)
   {
      InventoryDisplayScrollbar();
      top_row = 0;
   }

   WindowEndUpdate(hwndInv);

   InventoryRedraw();
}
/************************************************************************/
/*
 * InventoryDropCurrentItem:  Drop the item with the inventory cursor, if any.
 *   If container is not NULL, put item in the container.
 *   Ask user for amount of object if appropriate.
 *   Return True iff object dropped.
 */
Bool InventoryDropCurrentItem(room_contents_node *container)
{
   int x, y;
   InvItem *item;

   item = InventoryGetCurrentItem();
   if (item == NULL)
      return False;

   x = cursor_col * INVENTORY_BOX_WIDTH;
   y = (cursor_row - top_row + 1) * INVENTORY_BOX_HEIGHT;
   if (!GetAmount(cinfo->hMain, hwndInv, item->obj, x, y))
      return False;

   if (container == NULL)
      RequestDrop(item->obj);
   else RequestPut(item->obj, container->obj.id);
   return True;
}
/************************************************************************/
/*
 * InventoryMoveCurrentItem:  Move the selected item with the inventory cursor 
 *   to the item at the given (x, y) coordinates, if any. Returns True iff item moved.
 *   Coordinates (0,0) are at the top left corner of the inventory window,
 *   increasing as you go to the right and down.
 */
Bool InventoryMoveCurrentItem(int x, int y)
{
   InvItem *item = InventoryGetCurrentItem();
   if (item == NULL)
      return False;

   // Find row and col in absolute coordinates
   int row = top_row + y / INVENTORY_BOX_HEIGHT;
   int col = x / INVENTORY_BOX_WIDTH;

   InvItem *drop_position = (InvItem *) list_nth_item(items, row * cols + col);
   if (drop_position == NULL)
      return False;

   if (item->obj->id == drop_position->obj->id)
      return False;

   /* Before we send the move request to the server, we need to convert
   *   the move index arguments from 0-based to 1-based (Blakod).
   *   We also need to reverse the list indices because the client
   *   inventory list is reversed compared to the server Blakod list.
   */

   int pos_payload = list_get_position(items, (void *)item->obj->id, InventoryCompareIdItem);
   int pos_target = list_get_position(items, (void *)drop_position->obj->id, InventoryCompareIdItem);
   int length = list_length(items);

   /* add 1 to targetpos if target pos > payload pos because 
   *   the destination argument is the index of the item you
   *   want the moved item to go before.
   */
   if (pos_target > pos_payload)
      pos_target += 1;

   items = list_move_item(items, pos_payload, pos_target);
   InventoryRedraw();

   // Blakod indices are 1-based, and inventory list is reversed
   pos_payload = length - pos_payload;
   pos_target = length - pos_target + 1;

   RequestInventoryMove(pos_payload, pos_target);

   return True;
}
/************************************************************************/
void DisplayInventory(list_type inventory)
{
   list_type l;

   WindowBeginUpdate(hwndInv);

   num_items = 0;

   InventoryResetData();

   cursor_row = 0;
   cursor_col = 0;
   
   for (l = inventory; l != NULL; l = l->next)
   {
      object_node *inv_object = (object_node *) (l->data);
      InventoryAddItem(inv_object);
   }

   InventoryScrollRange();
   if (num_items > rows * cols)
      InventoryDisplayScrollbar();
   WindowEndUpdate(hwndInv);
}
/************************************************************************/
/*
 * DisplaySetUsing:  Change selections in inventory to reflect the fact that
 *   player is using (using = True) or not using (using = False) given item.
 */
void DisplaySetUsing(ID obj_id, Bool is_using)
{
   InvItem *item;
   
   item = (InvItem *) list_find_item(items, (void *) obj_id, InventoryCompareIdItem);
   if (item == NULL)
      return;  /* Not an error, since we try to unuse everything we drop */

   item->is_using = is_using;
   
   InventoryRedrawSingleItem(item);
}
/************************************************************************/
/*
 * DisplayUsing:  Change selections in inventory to reflect the fact that
 *   player is using exactly those items in given list.
 */
void DisplayUsing(list_type using_list)
{
   list_type l;

   /* Turn off all currently used items */
   for (l = items; l != NULL; l = l->next)
   {
      InvItem *item = (InvItem *) (l->data);
      item->is_using = False;
   }

   /* Select used items */
   for (l = using_list; l != NULL; l = l->next)
     DisplaySetUsing(reinterpret_cast<std::intptr_t>(l->data), True);
}
/************************************************************************/
/*
 * InventoryChangeItem:  Change given object in inventory to reflect new
 *   icon, name, or amount.
 */
void InventoryChangeItem(object_node *obj)
{
   InvItem *item;
   
   item = (InvItem *) list_find_item(items, (void *) obj->id, InventoryCompareIdItem);
   if (item == NULL)
      return;  /* Not an error, since we try to unuse everything we drop */

   InventoryRedrawSingleItem(item);
}
/************************************************************************/
/*
 * InventoryGetCurrentId:  Return id of object with cursor in inventory, or
 *   INVALID_ID if none.
 */
ID InventoryGetCurrentId(void)
{
   InvItem *item;

   item = InventoryGetCurrentItem();
   if (item == NULL)
      return INVALID_ID;
   return item->obj->id;
}
/************************************************************************/
/*
 * InventoryGetCurrentItem:  Return item with cursor in inventory, or NULL if none.
 */
InvItem *InventoryGetCurrentItem(void)
{
   int index = cursor_row * cols + cursor_col;
   return (InvItem *) list_nth_item(items, index);
}
/************************************************************************/
/*
 * InventoryItemVisible: Return True iff an item at the given (row, col) 
 *   in absolute coordinates is visible.
 */
Bool InventoryItemVisible(int row, int col)
{
   return (row >= top_row && row < top_row + rows) && (col >= 0 && col < cols) &&
      (row * cols + col < num_items);
}
/************************************************************************/
/*
 * InventoryCompareIdItem: Compare an id # with an inventory item; return True iff they
 *    have the same id #.
 */
Bool InventoryCompareIdItem(void *idnum, void *item)
{
   InvItem *temp = (InvItem *) item;
   return GetObjId(reinterpret_cast<std::intptr_t>(idnum)) == GetObjId(temp->obj->id);
}
/************************************************************************/
/*
 * InventoryGetArea: Copy inventory area to "area".
 */
void InventoryGetArea(AREA *area)
{
   memcpy(area, &inventory_area, sizeof(AREA));
}
/************************************************************************/
Bool InventoryMouseCaptured(void)
{
   return capture;
}


/************************************************************************/
/*
 * AnimateInventory: Animate inventory items.
 *   dt is number of milliseconds since last time animation timer went off.
 */
void AnimateInventory(int dt)
{
   Bool need_redraw;
   int index;
   list_type l;

   index = 0;
   for (l = items; l != NULL; l = l->next)
   {
      InvItem *item = (InvItem *) (l->data);
      
      need_redraw = AnimateObject(item->obj, dt);
      if (need_redraw)
      {
	 // Redraw object on screen if it's visible
	 int row = index / cols;
	 int col = index % cols;
	 
	 if (InventoryItemVisible(row, col))
	    InventoryDrawSingleItem(item, row - top_row, col);
      }
      index++;
   }
}

/************************************************************************/
/*
 * ShowInventory, HideInventory: Added by ajw.
 */
void ShowInventory( Bool bShow )
{
	ShowWindow( hwndInvDialog, bShow ? SW_SHOWNORMAL : SW_HIDE );
	ShowWindow( hwndInvScroll, bShow && has_scrollbar ? SW_SHOWNORMAL : SW_HIDE );
}

/************************************************************************/
HWND GetHwndInv()
{
	return hwndInv;
}

/************************************************************************/
RawBitmap* pinventory_bkgnd()
{
	return &inventory_bkgnd;
}
