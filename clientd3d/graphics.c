// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * graphics.c: Routines for creating, moving, and resizing main graphics window.
 *   Also contains some functions for user interface of main graphics window.
 *   Doesn't handle actual drawing into window.
 */

#include "client.h"

//	Duplicate of what is in merint\userarea.h.
#define USERAREA_HEIGHT 64

extern AREA	gD3DView;
extern Bool				gD3DRedrawAll;

/* Set to True when the room should be redrawn at the first opportunity */
static Bool need_redraw = False;

static AREA view;			/* Client coordinates of room viewing rectangle */
static AREA areaMiniMap;	//	Coords of the MiniMap.

static DRAWBORDEREXCLUDE drawborderexcludeView = { 43, 43, 45, 45, 46, 48, 46, 48 };

Bool map = False;             // True if we should draw map instead of room view

static Bool capture = False;  // True when graphics area has mouse capture
static ID   drag_object;      // When capture = True, holds id of object being dragged
static int fps;
static int msDrawFrame;
DWORD fpsDrawTime = 0;
static int fpsCount = 0;
static int msDrawFrameCount = 0;

// XXX This isn't currently used at all
// # of pieces to divide graphics area horizontally & vertically for determining what
// user actions correspond to what parts of the graphics area.  For example, when
// SCREEN_UNIT = 12, boundaries of pieces are measured in multiples of 1/12 of the height
// and width of the graphics area.
#define SCREEN_UNIT 12

// Region of the graphics area that corresponds to a motion action
typedef struct {
   int left, right, top, bottom;      // In multiples of SCREEN_UNIT
   int  action;
} ScreenArea;

ScreenArea move_areas[] = {
{  0,  4,  0,  2, A_FORWARDTURNFASTLEFT },
{  4,  8,  0,  3, A_FORWARDFAST },
{  8, 12,  0,  2, A_FORWARDTURNFASTRIGHT },

{  0,  2,  2,  4, A_FORWARDTURNFASTLEFT },
{  2,  4,  2,  4, A_FORWARDTURNLEFT },
{  4,  8,  3,  6, A_FORWARD },
{  8, 10,  2,  4, A_FORWARDTURNRIGHT },
{ 10, 12,  2,  4, A_FORWARDTURNFASTRIGHT },

{  0,  2,  4,  8, A_TURNFASTLEFT },
{  2,  4,  4,  8, A_TURNLEFT },
{  8, 10,  4,  8, A_TURNRIGHT },
{ 10, 12,  4,  8, A_TURNFASTRIGHT },

{  0,  2,  8, 10, A_BACKWARDTURNFASTLEFT },
{  2,  4,  8, 10, A_BACKWARDTURNLEFT },
{  4,  8,  6,  9, A_BACKWARD },
{  8, 10,  8, 10, A_BACKWARDTURNRIGHT },
{ 10, 12,  8, 10, A_BACKWARDTURNFASTRIGHT },

{  0,  4, 10, 12, A_BACKWARDTURNFASTLEFT },
{  4,  8,  9, 12, A_BACKWARDFAST },
{  8, 12, 10, 12, A_BACKWARDTURNFASTRIGHT },
};

static int num_move_areas = (sizeof(move_areas) / sizeof(ScreenArea));

extern room_type current_room;
extern int border_index;

/************************************************************************/
/*
 * GraphicsAreaCreate:  Create main graphics view window.
 */
void GraphicsAreaCreate(HWND hParent)
{
}
/************************************************************************/
/*
 * GraphicsAreaDestroy:  Destroy main graphics view window.
 */
void GraphicsAreaDestroy(void)
{
   GraphicsReleaseCapture();
}
/************************************************************************/
/*
 * GraphicsToggleMap:  Toggle map display on and off.
 */
void GraphicsToggleMap(void)
{
   map = !map;

   RedrawAll();
}
/************************************************************************/
/*
 * ResizeAll:  Send main window a resize message so that subwindows
 *   resize themselves.
 */
void ResizeAll(void)
{
   RECT r;
   
   GetWindowRect(hMain, &r);
   //MoveWindow(hMain, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
   MoveWindow(hMain, 0, 0, 1024, 768, TRUE);

   GetClientRect(hMain, &r);
   MainResize(hMain, SIZE_RESTORED, r.right - r.left, r.bottom - r.top);
   InvalidateRect(hMain, NULL, TRUE);
}
/************************************************************************/
/*
 * TranslateToRoom:  Translate from client's (client_x, client_y) coordinates to 
 *    an (x, y) point in the room displayed in the graphics window.  
 *    If the graphics in the main window are not drawn 1:1, (x, y) are scaled
 *    as if the graphics were drawn 1:1.
 *    
 *    If (client_x, client_y) is in room view area, set (x, y) to corresponding
 *    point in room view and return True.  Otherwise return False.
 */
Bool TranslateToRoom(int client_x, int client_y, int *x, int *y)
{
   int stretchfactor = config.large_area ? 2 : 1;

   if (client_x < view.x || client_x > view.x + view.cx ||
       client_y < view.y || client_y > view.y + view.cy)
      return False;

   *x = (client_x - view.x) / stretchfactor;
   *y = (client_y - view.y) / stretchfactor;

   return True;
}
/************************************************************************/
/*
 * MouseToRoom:  Translate from current mouse position to 
 *    the coordinates of the room.  Sets (x, y) to the room coordinate (pixel) 
 *    containing mouse pointer.  Returns True iff pointer is in grid area.
 */
Bool MouseToRoom(int *x, int *y)
{
   POINT mouse;

   if (map)
      return FALSE;

   GetCursorPos(&mouse);
   ScreenToClient(hMain, &mouse);

   return TranslateToRoom(mouse.x, mouse.y, x, y);
}
/************************************************************************/
/*
 * MouseToMiniMap:  Like MouseToRoom + TranslateToRoom, but for MiniMap. ajw
 *		Returns true and sets x and y to MiniMap relative coords if cursor
 *		is in MiniMap area.
 */
Bool MouseToMiniMap( int* x, int* y )
{
	POINT ptMouse;
	GetCursorPos( &ptMouse );
	ScreenToClient( hMain, &ptMouse );

	if( ptMouse.x < areaMiniMap.x || ptMouse.x > areaMiniMap.x + areaMiniMap.cx ||
		ptMouse.y < areaMiniMap.y || ptMouse.y > areaMiniMap.y + areaMiniMap.cy )
		return False;

	*x = ptMouse.x - areaMiniMap.x;
	*y = ptMouse.y - areaMiniMap.y;

	return True;
}

/************************************************************************/
/*
 * GraphicsAreaResize:  Set view variable to reflect size of window.
 *   Use when window is resized, and on startup.
 */
void GraphicsAreaResize(int xsize, int ysize)
{
   int new_xsize, new_ysize;  /* Need signed #s */
   Bool must_redraw = False;

   int max_width, max_height;
   int stretchfactor = config.large_area ? 2 : 1;

   int iHeightAvailableForMapAndStats;

   max_width  = stretchfactor * MAXX;
   max_height = stretchfactor * MAXY;

   new_xsize = min(xsize - INVENTORY_MIN_WIDTH, max_width);

   new_ysize = ysize - TEXT_AREA_MIN_HEIGHT - BOTTOM_BORDER - GetTextInputHeight() - TOP_BORDER - EDGETREAT_HEIGHT * 2;
   if (config.toolbar)
     new_ysize -= TOOLBAR_BUTTON_HEIGHT - MIN_TOP_TOOLBAR;
   else new_ysize -= MIN_TOP_NOTOOLBAR;
   new_ysize = min(new_ysize, max_height);   

   /* Make sizes divisible by 4.  Must be even for draw3d, and when 
    * stretchfactor = 2, need divisible by 4 so that room fits exactly in view */
   new_xsize &= ~3;
   new_ysize &= ~3;

   if (new_xsize < 0)
      new_xsize = 0;
   if (new_ysize < 0)
      new_ysize = 0;

   /* Move grid area to appropriate place */
   view.x = LEFT_BORDER + HIGHLIGHT_THICKNESS + EDGETREAT_WIDTH;
   view.y = HIGHLIGHT_THICKNESS;
   if (config.toolbar)
     view.y += TOOLBAR_Y + TOOLBAR_BUTTON_HEIGHT + MIN_TOP_TOOLBAR;
   else view.y += TOP_BORDER + MIN_TOP_NOTOOLBAR + EDGETREAT_HEIGHT;
   
   if (new_xsize != view.cx || new_ysize != view.cy)
      must_redraw = True;

   view.cx = new_xsize;
   view.cy = new_ysize;

   D3DRenderResizeDisplay(view.x, view.y, view.cx, view.cy);

   //	areaMiniMap added by ajw.
   areaMiniMap.x	= view.x + view.cx + LEFT_BORDER + 2 * HIGHLIGHT_THICKNESS + MAPTREAT_WIDTH;
   areaMiniMap.cx	= min( xsize - areaMiniMap.x - 2 * HIGHLIGHT_THICKNESS - EDGETREAT_WIDTH - MAPTREAT_WIDTH, MINIMAP_MAX_WIDTH );

   areaMiniMap.y	= 2 * TOP_BORDER + USERAREA_HEIGHT + EDGETREAT_HEIGHT + (MAPTREAT_HEIGHT * 2) - 1;

   iHeightAvailableForMapAndStats = ysize - areaMiniMap.y - 2 * HIGHLIGHT_THICKNESS - EDGETREAT_HEIGHT;

   areaMiniMap.cy	= (int)( iHeightAvailableForMapAndStats * PROPORTION_MINIMAP ) - HIGHLIGHT_THICKNESS - MAPTREAT_HEIGHT;
   areaMiniMap.cy	= min( areaMiniMap.cy, MINIMAP_MAX_HEIGHT );

   areaMiniMap.cy -= (TOOLBAR_BUTTON_HEIGHT + TOOLBAR_SEPARATOR_WIDTH) * 2;
   areaMiniMap.y += (TOOLBAR_BUTTON_HEIGHT + TOOLBAR_SEPARATOR_WIDTH) * 2;

   MapMiniSizeChanged(&areaMiniMap);

   //	Tell view edge treatment elements to reposition themselves.
   ViewElementsReposition( &view );

   if (must_redraw)
   {
      RedrawAll();
      RedrawForce();
   }
}
/************************************************************************/
/*
 * CopyCurrentView:  Put a copy of the current view structure in v.
 */
void CopyCurrentView(AREA *v)
{
   memcpy(v, &view, sizeof(view));
}
/************************************************************************/
/*
 * CopyCurrentAreaMiniMap:  Put a copy of the current areaMiniMap structure in v.
 */
void CopyCurrentAreaMiniMap( AREA* v )
{
	memcpy( v, &areaMiniMap, sizeof( areaMiniMap ) );
}
/************************************************************************/
/*
 * DrawGridBorder:  Draw the border around the current room area.
 */
void DrawGridBorder(void)
{
	if (state == STATE_GAME && (GameGetState() == GAME_PLAY || GameGetState() == GAME_SELECT))
		if (GetFocus() == hMain)
			DrawBorder(&view, HIGHLIGHT_INDEX, &drawborderexcludeView);
		else
			//DrawBorder(view, border_index, &drawborderexcludeView);
			DrawBorder(&view, BORDER_INDEX, &drawborderexcludeView);
}
/************************************************************************/
/*
 * MapVisible:  Return True iff we are currently showing the map in the main
 *   graphics area.
 */
Bool MapVisible(void)
{
   return map;
}

int GetFPS(void)
{
   return fps;
}

int GetMSDrawFrame(void)
{
   return msDrawFrame;
}

/************************************************************************/
/* 
 * Functions should call RedrawAll if they need the room to be redrawn.
 * This sets a flag, and the next time an opportunity arises, the room
 * is redrawn via a call to RedrawForce.
 */
void RedrawForce(void)
{
   HDC hdc;
   static DWORD lastEndFrame = 0;
   DWORD endFrame, startFrame;
   int totalFrameTime, oldMode;
   char buffer[32];

   if (GameGetState() == GAME_INVALID || /*!need_redraw ||*/ IsIconic(hMain) ||
       view.cx == 0 || view.cy == 0 || current_room.rows == 0 || current_room.cols == 0)
   {
      need_redraw = False;
      return;
   }

   timeBeginPeriod(1);
   startFrame = timeGetTime();

   /* REVIEW: Clearing flag before draw phase allows draw phase to set flag.
    *         This is useful in rare circumstances when an effect should
    *         last only one frame, even if animation is off.
    */
   need_redraw = False;
   hdc = GetDC(hMain);
   DrawRoom(hdc, view.x, view.y, &current_room, map);

   endFrame = timeGetTime();
   msDrawFrame = (int)(endFrame - startFrame);
   totalFrameTime = (int)(endFrame - lastEndFrame);

   // if totalFrameTime is less than one, clamp to 1 so we don't divide by 0 or get negative fps
   if (1 > totalFrameTime)
	   totalFrameTime = 1;
   fps = 1000 / (int)totalFrameTime;
   if (timeGetTime() >= fpsDrawTime + 250)
   {
      fpsCount = fps;
      msDrawFrameCount = msDrawFrame;
      fpsDrawTime = timeGetTime();
   }

   if (config.maxFPS)
   {
      if (fps > config.maxFPS)
      {
	 int msSleep = (1000 / config.maxFPS) - totalFrameTime;
	 Sleep(msSleep);
      }
   }
   lastEndFrame = endFrame;
   timeEndPeriod(1);

   // Try to redraw new graphics
   //gD3DRedrawAll |= D3DRENDER_REDRAW_ALL;

   if (config.showFPS)
   {
      RECT rc,lagBox;
      wsprintf(buffer, "FPS = %d (%dms)        ", fpsCount, msDrawFrameCount);
      ZeroMemory(&rc,sizeof(rc));
      rc.bottom = DrawText(hdc, buffer, -1, &rc, DT_SINGLELINE | DT_CALCRECT | DT_NOCLIP);
      Lagbox_GetRect(&lagBox);
      OffsetRect(&rc,lagBox.right + TOOLBAR_SEPARATOR_WIDTH,lagBox.top);
      DrawWindowBackground(hdc, &rc, rc.left, rc.top);
      oldMode = SetBkMode(hdc,TRANSPARENT);
      DrawText(hdc, buffer, -1, &rc, DT_SINGLELINE | DT_NOCLIP);
      SetBkMode(hdc,oldMode);
      GdiFlush();
   }
   ReleaseDC(hMain, hdc);

   GameWindowSetCursor();   // We may have moved; reset cursor
}
/************************************************************************/
void RedrawAll(void)
{
   need_redraw = True;
}

/************************************************************************/
Bool NeedRedraw(void)
{
   return need_redraw;
}

/* called from windows events */
/************************************************************************/
/* 
 * GraphicsAreaRedraw:  Called whenever we get an expose event.
 *   hdc should be the DC from the PAINTSTRUCT structure of the expose event.
 */
void GraphicsAreaRedraw(HDC hdc)
{
	// If in GAME_INVALID, leave background color; if in game, copy drawn room
	if (GameGetState() != GAME_INVALID)
	{
		RecopyRoom3D( hdc, view.x, view.y, view.cx, view.cy, FALSE );
		RecopyRoom3D( hdc, areaMiniMap.x, areaMiniMap.y, areaMiniMap.cx, areaMiniMap.cy, TRUE );
	}
}

/************************************************************************/
/* 
 * UserMouseMove:  User wants to use mouse to move; check position of mouse
 *   and move user accordingly.
 */
void UserMouseMove(void)
{
   int x, y, xunit, yunit, i;
   int stretchfactor = config.large_area ? 2 : 1;

   if (!MouseToRoom(&x, &y) || view.cx == 0 || view.cy == 0)
      return;

   // Find action that corresponds to this part of the graphics area
   xunit = x * SCREEN_UNIT * stretchfactor / view.cx;
   yunit = y * SCREEN_UNIT * stretchfactor / view.cy;

   for (i=0; i < num_move_areas; i++)
   {
      if (xunit >= move_areas[i].left && xunit < move_areas[i].right &&
	  yunit >= move_areas[i].top  && yunit < move_areas[i].bottom)
      {
	 PerformAction(move_areas[i].action, NULL);
	 break;
      }
   }
}
/************************************************************************/
/*
 * UserStartDrag:  User wants to drag object under mouse pointer.  If there is no
 *   object there, do nothing.  If there is more than one object, ask user which
 *   one to pick up.  If there is exactly one, begin dragging object.
 */
void UserStartDrag(void)
{
   list_type objects, sel_list, l;
   int x, y;
   object_node *obj;

   /* Find out where in the the room the user clicked on, if anywhere */
   if (!MouseToRoom(&x, &y))
      return;

   objects = GetObjects3D(x, y, CLOSE_DISTANCE, OF_GETTABLE | OF_CONTAINER, 0, 0, 0);
   if (objects == NULL)
      return;

   if (objects->next == NULL)
   {
      SetCapture(hMain);
      capture = True;
      obj = (object_node *) objects->data;
      drag_object = obj->id;
      SetMainCursor(LoadCursor(hInst, MAKEINTRESOURCE(IDC_GETCURSOR)));
   }
   else
   {
      sel_list = DisplayLookList(hMain, GetString(hInst, IDS_GET), objects, LD_SINGLEAUTO);
      
      for (l = sel_list; l != NULL; l = l->next)
	 RequestPickup(((room_contents_node *) (l->data))->obj.id);

      ObjectListDestroy(sel_list);
   }

   list_delete(objects);
}
/************************************************************************/
/*
 * UserEndDrag:  User has ended drag of an object.
 */
void UserEndDrag(void)
{
   if (!GraphicsReleaseCapture())
      return;

   PerformAction(A_ENDDRAG, (void *) drag_object);
}
/************************************************************************/
/*
 * GraphicsReleaseCapture:  Release mouse capture, if main window has it.
 *   Return True iff main window had mouse capture.
 */
Bool GraphicsReleaseCapture(void)
{
   if (!capture)
      return False;

   ReleaseCapture();
   capture = False;
   GameWindowSetCursor();
   return True;
}
/************************************************************************/
Bool GraphicsMouseCaptured(void)
{
   return capture;
}
