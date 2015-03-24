// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * bmparea.c:  Handle the main control that contains the drawn bitmaps.
 */

#include "bbgun.h"

// Action to do with current mouse capture
enum {
   CAPTURE_NONE,      // No mouse capture in effect
   CAPTURE_HOTSPOT,   // Move hotspot along with mouse
   CAPTURE_OFFSET,    // Move offset along with mouse
};

static int capture_type;

static WNDPROC lpfnDefButtonProc;
static int bwidth, bheight;   // Size of bitmap area

static PDIB pdib1;
static PDIB pdib2;

static int StaticX;
static int StaticY;

extern int CurrentBitmap;
extern int CurrentHotspot;
extern int CurrentBBG;
extern int NumBBGs;
extern int Zoom;

static void HideCursor(void);
static BOOL GetCursorWindowPos(HWND hwnd, int *x, int *y);
static BOOL GetPdib(Bitmap *b);
static BOOL ButtonKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
static void ScrollListbox(int id, int offset);
/************************************************************************/
/*
 * DrawAreaInit:  Called on startup.
 */
void DrawAreaInit(void)
{
   HWND hButton;
   RECT r;

   // Compute button size and subclass it
   hButton = GetDlgItem(hMain, IDBITMAP);
   GetClientRect(hButton, &r);
   bwidth  = r.right;
   bheight = r.bottom;
   
   lpfnDefButtonProc = SubclassWindow(hButton, MainButtonProc);
}
/************************************************************************/
void DoDraw(HDC hDC1, HWND hWnd)
{
   int OverX, OverY;
   int i;
   BOOL highlight;
   Bitmap *b, *anchor, *wanderer = NULL;

   ClearBitmap();
      
   if (!config.dual_mode && NumBBGs > 0 && BBGs[CurrentBBG].NumBitmaps > 0)
   {
      // Draw single bitmap
      /////////////////////
      
      if (GetPdib(&BBGs[CurrentBBG].Bitmaps[CurrentBitmap]) == FALSE)
	  return;

      pdib1 = BBGs[CurrentBBG].Bitmaps[CurrentBitmap].pdib;

      StaticX = (bwidth - DibWidth(pdib1) * Zoom) / 2;
      StaticY = (bheight - DibHeight(pdib1) * Zoom) / 2;
      // Draw at full size, regardless of shrink
      DrawBitmap(pdib1, StaticX, StaticY, 1.0f / Zoom, config.transparent);

      DisplayBitmaps(hDC1, 0,0, bwidth, bheight);
      if ( BBGs[CurrentBBG].Bitmaps[CurrentBitmap].NumHotspots > 0 && config.show_hotspots)
      {
	 for (i = 0; i < BBGs[CurrentBBG].Bitmaps[CurrentBitmap].NumHotspots; i++)
	 {
	    if (!config.all_hotspots)
	       i = BBGs[CurrentBBG].CurrentHotspot;

	    highlight = (i == BBGs[CurrentBBG].CurrentHotspot);

	    b = &BBGs[CurrentBBG].Bitmaps[CurrentBitmap];
	    DrawHotspot(hDC1, 
			StaticX + b->Hotspots[i].X * Zoom,
			StaticY + b->Hotspots[i].Y * Zoom,
			highlight);

	    if (!config.all_hotspots)
	       break;
	 }
      }
      
   }
   
   else {
      // Draw anchor + overlay
      ////////////////////////

      if ((Anchor.BBG == -1) || (WanderOne.BBG == -1))
	 return;

      anchor = &BBGs[Anchor.BBG].Bitmaps[BBGs[Anchor.BBG].CurrentBitmap];
      
      if (GetPdib(anchor) == FALSE)
	  return;

      pdib1 = anchor->pdib;

      if (WanderOne.BBG != -1)
      {
	 wanderer = &BBGs[WanderOne.BBG].Bitmaps[BBGs[WanderOne.BBG].CurrentBitmap];
	 if (GetPdib(wanderer) == FALSE)
	    return;
	 
	 pdib2 = wanderer->pdib;
      }
      
      StaticX = (bwidth - DibWidth(pdib1) * Zoom) / 2;
      StaticY = (bheight - DibHeight(pdib1) * Zoom) / 2;
      
      // Draw underlays
      if ((WanderOne.BBG != -1) &&
	  (anchor->Hotspots[BBGs[Anchor.BBG].CurrentHotspot].Lap < 0))
      {
	 OverX = StaticX
	    + (anchor->Hotspots[BBGs[Anchor.BBG].CurrentHotspot].X 
	       + wanderer->XOffset) * Zoom;
	 OverY = StaticY
	    + (anchor->Hotspots[BBGs[Anchor.BBG].CurrentHotspot].Y
	       + wanderer->YOffset) * Zoom;
	 
	 DrawBitmap(pdib2, OverX, OverY, 
		    ((float) BBGs[WanderOne.BBG].Shrink) / BBGs[Anchor.BBG].Shrink / Zoom,
		    config.transparent);
      }
      
      // Draw anchor at normal size
      DrawBitmap(pdib1, StaticX, StaticY, 1.0f / Zoom, config.transparent);  
      
      // Draw overlays
      if ((WanderOne.BBG != -1) && (anchor->Hotspots[BBGs[Anchor.BBG].CurrentHotspot].Lap > 0))
      {
	 OverX = StaticX
	    + (anchor->Hotspots[BBGs[Anchor.BBG].CurrentHotspot].X 
	       + wanderer->XOffset) * Zoom;
	 OverY = StaticY
	    + (anchor->Hotspots[BBGs[Anchor.BBG].CurrentHotspot].Y
	       + wanderer->YOffset) * Zoom;
	 
	 DrawBitmap(pdib2, OverX, OverY, 
		    ((float) BBGs[WanderOne.BBG].Shrink) / BBGs[Anchor.BBG].Shrink / Zoom, 
		    config.transparent);
      }
      
      DisplayBitmaps(hDC1, 0,0, bwidth, bheight);
      
      if (anchor->NumHotspots > 0 && config.show_hotspots)
	 for (i = 0; i < anchor->NumHotspots; i++)
	 {
	    if (!config.all_hotspots) 
	       i = BBGs[Anchor.BBG].CurrentHotspot;

	    highlight = (i == BBGs[Anchor.BBG].CurrentHotspot);

	    DrawHotspot(hDC1,
			StaticX + anchor->Hotspots[i].X * Zoom,
			StaticY + anchor->Hotspots[i].Y * Zoom,
			highlight);
	    
	    if (!config.all_hotspots)
	       break;
	 }
   }   
}
/************************************************************************/

/* subclassed window procedure for main bitmap area */
long CALLBACK MainButtonProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
   case WM_LBUTTONDOWN:
      if (capture_type != CAPTURE_NONE)
	 break;

      SetCapture(hwnd);
      capture_type = CAPTURE_HOTSPOT;
      HideCursor();
      SendMessage(hwnd, WM_MOUSEMOVE, wParam, lParam);
      break;

   case WM_LBUTTONUP:
      capture_type = CAPTURE_NONE;
      ReleaseCapture();
      ShowCursor(TRUE);
      break;

   case WM_RBUTTONDOWN:
      if (capture_type != CAPTURE_NONE || !config.dual_mode || NumBBGs < 2)
	 break;

      SetCapture(hwnd);
      capture_type = CAPTURE_OFFSET;
      HideCursor();
      SendMessage(hwnd, WM_MOUSEMOVE, wParam, lParam);
      break;

   case WM_RBUTTONUP:
      capture_type = CAPTURE_NONE;
      ReleaseCapture();
      ShowCursor(TRUE);
      break;

   case WM_MOUSEMOVE:
      switch (capture_type)
      {
      case CAPTURE_HOTSPOT:
	 HotspotDrag(hwnd);
	 break;

      case CAPTURE_OFFSET:
	 OffsetDrag(hwnd);
	 break;
      }
      break;

      HANDLE_MSG(hwnd, WM_KEYDOWN, ButtonKey);
   }
   return CallWindowProc(lpfnDefButtonProc, hwnd, message, wParam, lParam);
}
/************************************************************************/
/*
 * HotspotDrag:  User is dragging hotspot on hwnd.
 */
void HotspotDrag(HWND hwnd)
{
   Bitmap *anchor, *wanderer, *b;

   int mouse_x, mouse_y;

   if ((NumBBGs == 0) || (BBGs[CurrentBBG].NumBitmaps == 0)) 
      return;
      
   if (GetCursorWindowPos(hwnd, &mouse_x, &mouse_y) == FALSE)
      return;
   
   if (config.dual_mode)
   {
      if (Anchor.BBG == -1)
	 return;
      
      anchor   = &BBGs[Anchor.BBG].Bitmaps[BBGs[Anchor.BBG].CurrentBitmap];
      wanderer = &BBGs[WanderOne.BBG].Bitmaps[BBGs[WanderOne.BBG].CurrentBitmap];

      StaticX = (bwidth - DibWidth(pdib1) * Zoom) / 2;
      StaticY = (bheight - DibHeight(pdib1) * Zoom) / 2;
      anchor->Hotspots[BBGs[Anchor.BBG].CurrentHotspot].X = (mouse_x - StaticX) / Zoom;
      anchor->Hotspots[BBGs[Anchor.BBG].CurrentHotspot].Y = (mouse_y - StaticY) / Zoom;
      BBGs[Anchor.BBG].changed = TRUE;
   }
   else
   {
      b = &BBGs[CurrentBBG].Bitmaps[CurrentBitmap];
      
      StaticX = (bwidth - DibWidth(pdib1) * Zoom) / 2;
      StaticY = (bheight - DibHeight(pdib1) * Zoom) / 2;
      
      b->Hotspots[CurrentHotspot].X = (mouse_x - StaticX) / Zoom;
      b->Hotspots[CurrentHotspot].Y = (mouse_y - StaticY) / Zoom;
      BBGs[CurrentBBG].changed = TRUE;
   }

   UpdateHotspot();
   DrawIt();
}
/************************************************************************/
/*
 * OffsetDrag:  User is dragging bitmap offset on hwnd.
 */
void OffsetDrag(HWND hwnd)
{   
   Bitmap *anchor, *wanderer;
   int mouse_x, mouse_y;

   // Allow cursor to be outside of window, for greater placement range
   GetCursorWindowPos(hwnd, &mouse_x, &mouse_y);

   anchor   = &BBGs[Anchor.BBG].Bitmaps[BBGs[Anchor.BBG].CurrentBitmap];
   wanderer = &BBGs[WanderOne.BBG].Bitmaps[BBGs[WanderOne.BBG].CurrentBitmap];
   
   StaticX = (bwidth - DibWidth(pdib1) * Zoom) / 2;
   StaticY = (bheight - DibHeight(pdib1) * Zoom) / 2;
   
   wanderer->XOffset =
      (mouse_x - StaticX - anchor->Hotspots[BBGs[Anchor.BBG].CurrentHotspot].X) / Zoom;
   
   wanderer->YOffset =
      (mouse_y - StaticY - anchor->Hotspots[BBGs[Anchor.BBG].CurrentHotspot].Y) / Zoom;

   UpdateOffset2();
   DrawIt();
   BBGs[WanderOne.BBG].changed = TRUE;
}
/************************************************************************/
/*
 * GetCursorWindowPos:  Set (x, y) to cursor's position.
 *   If cursor is in given window, return TRUE.  Otherwise return FALSE.
 */
BOOL GetCursorWindowPos(HWND hwnd, int *x, int *y)
{
   RECT r;
   POINT mouse;

   GetWindowRect(hwnd, &r);
   GetCursorPos(&mouse);   

   mouse.x -= r.left;
   mouse.y -= r.top;

   *x = mouse.x;
   *y = mouse.y;

   if (mouse.x < 0 || mouse.y < 0 || 
       mouse.x > r.right - r.left || mouse.y > r.bottom - r.top)
      return FALSE;

   return TRUE;
}
/************************************************************************/
/*
 * HideCursor:  Make mouse pointer go away.
 */
void HideCursor(void)
{
   while (ShowCursor(FALSE) >= 0)
      ;
}
/************************************************************************/
/*
 * GetPdib:  Set pdib field of given Bitmap structure, loading bitmap if
 *   necessary.  Returns TRUE iff pdib field is non-NULL on return.
 */
BOOL GetPdib(Bitmap *b)
{
   char temp[100 + MAX_PATH];

   if (b->pdib != NULL)
      return TRUE;
   
   // Load bitmaps
   b->pdib = DibOpenFile(b->Title);
   if (b->pdib == NULL)
   {
      wsprintf(temp, "Couldn't load bitmap %s", b->Title);
      MessageBox(hMain, temp, szAppName, MB_OK);
      return FALSE;
   }

   return TRUE;
}

/************************************************************************/
BOOL ButtonKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
   switch (vk)
   {
   case VK_INSERT:
      ScrollListbox(IDC_FILELIST, -1);
      break;

   case VK_DELETE:
      ScrollListbox(IDC_FILELIST, +1);
      break;

   case VK_HOME:
      ScrollListbox(IDC_FILELIST2, -1);
      break;

   case VK_END:
      ScrollListbox(IDC_FILELIST2, +1);
      break;
   }
   return FALSE;
}
/************************************************************************/
/*
 * ScrollListbox:  Fake Windows message to scroll given listbox by given amount, if possible.
 */
void ScrollListbox(int id, int offset)
{
   HWND hList;
   int count, pos;

   hList = GetDlgItem(hMain, id);
   count = ListBox_GetCount(hList);
   pos = ListBox_GetCurSel(hList);

   if (count == 0 || pos == LB_ERR)
      return;

   if (pos + offset < 0 || pos + offset >= count)
      return;

   ListBox_SetCurSel(hList, pos + offset);

   FORWARD_WM_COMMAND(hMain, id, hList, LBN_SELCHANGE, WndProc);
}
