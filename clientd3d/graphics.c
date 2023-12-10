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
#include <vector>
#include <numeric>
#include <chrono>

//	Duplicate of what is in merint\userarea.h.
#define USERAREA_HEIGHT 64

extern AREA	gD3DView;

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

extern room_type current_room;
extern int border_index;

int main_viewport_width;
int main_viewport_height;
extern float player_overlay_scaler;

// Variables to store frame times and FPS
static std::vector<double> fps_store;
static const int fps_window_size = 60;  // Number of fps values to consider in the rolling window.
static int average_fps = 0;
typedef std::chrono::time_point<std::chrono::steady_clock> steady_clock_time_point;
static steady_clock_time_point lastEndFrame;
// The clock to use for fps calculations - updating here will update throughout.
static auto& chrono_time_now = std::chrono::steady_clock::now;

static const int defaultMaxFps = 60;

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
   if (map)
      SetUserTargetID(INVALID_ID);

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

   if (client_x < view.x || client_x > view.x + view.cx ||
       client_y < view.y || client_y > view.y + view.cy)
      return False;

   *x = (client_x - view.x) / 2;
   *y = (client_y - view.y) / 2;

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

   int iHeightAvailableForMapAndStats;

   // Determine the height of the text area section of the client
   int text_area_size = ((float)config.text_area_size / 100.0) * ysize;
   int text_area_height = text_area_size + BOTTOM_BORDER + GetTextInputHeight() + TOP_BORDER + EDGETREAT_HEIGHT * 2;
   text_area_height += config.toolbar ? TOOLBAR_BUTTON_HEIGHT + MIN_TOP_TOOLBAR : MIN_TOP_NOTOOLBAR;

   // Calculate the largest possible viewport size keeping the classic client aspect ratio.
   new_ysize = ysize - text_area_height;
   new_xsize = new_ysize * MAXYX_ASPECT_RATIO;

   if (new_xsize > MAXX)
   {
       // Prevent larger resolutions from exceeding the maximum supported width.
       new_xsize = MAXX;
       new_ysize = new_xsize / MAXYX_ASPECT_RATIO;
   }

   if ((new_xsize + INVENTORY_MIN_WIDTH) > xsize) {
      new_xsize = xsize - INVENTORY_MIN_WIDTH;
      new_ysize = new_xsize * MAXXY_ASPECT_RATIO;
   }

   /* Make sizes divisible by 4.  Must be even for draw3d, and when 
    * stretchfactor = 2, need divisible by 4 so that room fits exactly in view */
   new_xsize &= ~3;
   new_ysize &= ~3;

   int inventory_width = xsize - new_xsize;

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

   // update main viewport and classic scaler (required for FOV calculations and equipment scaling)
   main_viewport_width = view.cx;
   main_viewport_height = view.cy;
   player_overlay_scaler = ((float)main_viewport_width) / CLASSIC_WIDTH;

   D3DRenderResizeDisplay(view.x, view.y, view.cx, view.cy);

   int minimap_width_height = (inventory_width + 3) & ~3;

   //	areaMiniMap added by ajw.
   areaMiniMap.x = view.x + view.cx + LEFT_BORDER + 2 * HIGHLIGHT_THICKNESS + MAPTREAT_WIDTH;
   areaMiniMap.cx = min( xsize - areaMiniMap.x - 2 * HIGHLIGHT_THICKNESS - EDGETREAT_WIDTH - MAPTREAT_WIDTH, minimap_width_height);

   areaMiniMap.y = 2 * TOP_BORDER + USERAREA_HEIGHT + EDGETREAT_HEIGHT + (MAPTREAT_HEIGHT * 2) - 1;

   iHeightAvailableForMapAndStats = ysize - areaMiniMap.y - 2 * HIGHLIGHT_THICKNESS - EDGETREAT_HEIGHT;

   areaMiniMap.cy = (int)( iHeightAvailableForMapAndStats * PROPORTION_MINIMAP ) - HIGHLIGHT_THICKNESS - MAPTREAT_HEIGHT;
   areaMiniMap.cy = min( areaMiniMap.cy, minimap_width_height);

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
   if (GameGetState() == GAME_INVALID || /*!need_redraw ||*/ IsIconic(hMain) ||
       view.cx == 0 || view.cy == 0 || current_room.rows == 0 || current_room.cols == 0)
   {
      need_redraw = False;
      return;
   }

   steady_clock_time_point startFrame = chrono_time_now();

   /* REVIEW: Clearing flag before draw phase allows draw phase to set flag.
    *         This is useful in rare circumstances when an effect should
    *         last only one frame, even if animation is off.
    */
   need_redraw = False;
   HDC hdc = GetDC(hMain);
   DrawRoom(hdc, view.x, view.y, &current_room, map);

   steady_clock_time_point endFrame = chrono_time_now();
   std::chrono::duration<double> elapsedTime = endFrame - startFrame;
   auto elapsedMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsedTime).count();
   auto elapsedMilliseconds = elapsedMicroseconds / 1000;
   msDrawFrame = elapsedMilliseconds;

   auto maxFPS = config.gpuEfficiency ? defaultMaxFps : config.maxFPS;
   fps = 1000 / max(1, elapsedMilliseconds);

   if (maxFPS)
   {
      if (fps > maxFPS)
      {
          // Clamp the fps to the maximum.
          int msSleep = (1000 / maxFPS) - elapsedMilliseconds;
          Sleep(msSleep);

          // Reclaulate the fps following the sleep.
          endFrame = chrono_time_now();
          elapsedTime = (endFrame - lastEndFrame);
          elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();
          fps = 1000 / max(1, elapsedMilliseconds);
      }
   }

   lastEndFrame = endFrame;

   if (config.showFPS)
   {
        // Add the fps to the rolling window.
        fps_store.push_back(fps);

        // Remove the oldest fps if the window is full.
        if (fps_store.size() > fps_window_size) {
            fps_store.erase(fps_store.begin());
        }

        // Calculate the average fps over the rolling window.
        auto sumFPS = std::accumulate(fps_store.begin(), fps_store.end(), 0.0);
        average_fps = sumFPS / fps_store.size();

        // Format and display the latest average fps value.
        RECT rc,lagBox;
        double milliseconds = static_cast<double>(elapsedMicroseconds) / 1000.0;
        char buffer[32];
        sprintf(buffer, "FPS=%d (%.1fms)        ", average_fps, milliseconds);
        ZeroMemory(&rc,sizeof(rc));

        rc.bottom = DrawText(hdc, buffer,-1,&rc,DT_SINGLELINE|DT_CALCRECT);
        Lagbox_GetRect(&lagBox);
        OffsetRect(&rc, main_viewport_width - 110, lagBox.top);
        DrawWindowBackground(hdc, &rc, rc.left, rc.top);
        int oldMode = SetBkMode(hdc,TRANSPARENT);
        DrawText(hdc, buffer,-1,&rc,DT_SINGLELINE);
        SetBkMode(hdc,oldMode);
        GdiFlush();
   }
   ReleaseDC(hMain, hdc);
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

   objects = GetObjects3D(x, y, CLOSE_DISTANCE, OF_GETTABLE | OF_CONTAINER, 0);
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
