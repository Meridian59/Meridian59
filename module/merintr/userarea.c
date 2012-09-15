// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * userarea.c:  Handle the part of the main window where user faces are displayed.
 */

#include "client.h"
#include "merintr.h"

#define MIN_FACE 1          // Minimum hotspot to which face parts are attached
#define MAX_FACE 20         // Maximum hotspot to which face parts are attached

#define MIN_PLAYER_OVERLAYS 7 // The minimum # of overlays a player will have

static HWND hUser;          // User window
static WNDPROC lpfnDefUserProc;
static AREA user_area;      // Screen area of user bitmap

extern BYTE *selftrgt_bits;        // Bitmap for in-use highlight

static long CALLBACK UserAreaProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
/************************************************************************/
/*
 * UserAreaCreate:  Create the user display area.
 */
void UserAreaCreate(void)
{
   hUser = CreateWindow("button", NULL,
			WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			0, 0, 0, 0, cinfo->hMain, (HMENU) IDC_USERAREA, hInst, NULL);
   lpfnDefUserProc = SubclassWindow(hUser, UserAreaProc);
}
/************************************************************************/
/*
 * UserAreaDestroy:  Destroy the user display area.
 */
void UserAreaDestroy(void)
{
   DestroyWindow(hUser);
   hUser = NULL;
}
/************************************************************************/
/*
 * UserAreaRedraw:  Redraw the user display area.
 */
void UserAreaRedraw(void)
{
   HDC hdc = GetDC(hUser);
   room_contents_node *r;
   AREA area;

   r = GetRoomObjectById(cinfo->player->id);
   
   if (r == NULL || hUser == NULL)
      debug(("DrawUserBitmap got NULL player object or area\n"));
   else
   {
      // Set up window background under user bitmap
      OffscreenWindowBackground(NULL, user_area.x, user_area.y, user_area.cx, user_area.cy);

	  if (GetUserTargetID() == GetPlayer()->id)
		OffscreenStretchBlt(hdc, 0, 0, user_area.cx, user_area.cy,
			selftrgt_bits, 0, 0, 64, 64,
			OBB_FLIP | OBB_TRANSPARENT);

      memcpy(&area, &user_area, sizeof(AREA));
      area.x = 0;
	  area.y = 1;
      // Try to tell if we're a player--if not, we want to draw the entire object, and not
      // just the face, because most objects don't have separate face overlays
      if (list_length(*r->obj.overlays) >= MIN_PLAYER_OVERLAYS)
	 // Only draw face overlays
	 DrawStretchedOverlayRange(hdc, &r->obj, &area, NULL, MIN_FACE, MAX_FACE);
      else DrawStretchedObjectDefault(hdc, &r->obj, &area, NULL);
   }
   ReleaseDC(hUser, hdc);
}
/************************************************************************/
/*
 * UserAreaResize:  Called when the main window is resized.
 */
void UserAreaResize(int xsize, int ysize, AREA *view)
{
   user_area.x  = view->x + view->cx + LEFT_BORDER + RIGHT_BORDER + MAPTREAT_WIDTH;
   user_area.y  = TOP_BORDER + EDGETREAT_HEIGHT;
   user_area.cx = USERAREA_WIDTH;
   user_area.cy = USERAREA_HEIGHT;

   MoveWindow(hUser, user_area.x, user_area.y, user_area.cx, user_area.cy, FALSE);
}

/************************************************************************/
/* 
 * UserAreaProc:  Window procedure for user area window.
 */
long CALLBACK UserAreaProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
   case WM_ERASEBKGND:
      return 1;

   case WM_LBUTTONDOWN:
      // See if selecting user as target
      if (GameGetState() == GAME_SELECT)
	 PerformAction(A_SELECT, (void *) cinfo->player->id);
      return 0;

   case WM_RBUTTONDOWN:
      // Examine self
      SetDescParams(cinfo->hMain, DESC_NONE);
      RequestLook(cinfo->player->id);
      return 0;
   }

   return CallWindowProc(lpfnDefUserProc, hwnd, message, wParam, lParam);
}
