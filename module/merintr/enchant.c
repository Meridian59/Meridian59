// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * enchant.c:  Handle storing and drawing room and user enchantments.
 *
 * Player enchantments are temporary player modifications, such as super strength.
 * A small icon for each is drawn next to the player's head while the enchantment is in effect.
 *
 * Room enchantments are similar modifications to the room, such as darkness.
 * Icons for these are drawn near the main view window.
 */

#include "client.h"
#include "merintr.h"

typedef struct {
   object_node *obj;
   HWND        hwnd;                   // Window containing icon for enchantment
} Enchantment;

static list_type room_enchantments;    // List of current enchantments on the room
static list_type player_enchantments;  // List of current enchantments on the player

static WNDPROC lpfnDefEnchantmentProc; // Default window procedure

int player_enchant_x;           // X position of left side of first player enchantment
int player_enchant_y;           // X position of left side of first player enchantment
int player_enchant_right;       // Right side of player enchantment area
int player_enchant_bottom;       // bottom side of player enchantment area
int room_enchant_x;             // X position of right side of first room enchantment
int room_enchant_y;             // Y position of top of first room enchantment
int room_enchant_left;          // Left side of room enchantment area

static void EnchantmentsMove(void);
static void EnchantmentsMovePlayer(void);
static void EnchantmentsMoveRoom(void);
static list_type EnchantmentListDestroy(list_type list);
static Enchantment *EnchantmentDestroy(Enchantment *e);
static Bool CompareIdEnchantment(void *idnum, void *e);
static Enchantment *EnchantmentFindByWindow(HWND hwnd);
static long CALLBACK EnchantmentProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
/****************************************************************************/
/*
 * EnchantmentsInit:  Called at startup.
 */
void EnchantmentsInit(void)
{
   room_enchantments = NULL;
   player_enchantments = NULL;
   RequestEnchantments(ENCHANT_PLAYER);
}
/****************************************************************************/
/*
 * EnchantmentsDestroy:  Free memory for enchantments.
 */
void EnchantmentsDestroy(void)
{
   room_enchantments   = EnchantmentListDestroy(room_enchantments);
   player_enchantments = EnchantmentListDestroy(player_enchantments);
}
/****************************************************************************/
/*
 * EnchantmentAdd:  Add given enchantment of given type.
 */
void EnchantmentAdd(BYTE type, object_node *obj)
{
   Enchantment *e;

   e = (Enchantment *) SafeMalloc(sizeof(Enchantment));
   e->hwnd = CreateWindow("button", NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			  0, 0, 0, 0, cinfo->hMain, (HMENU) IDC_ENCHANTMENT, hInst, NULL);
   lpfnDefEnchantmentProc = SubclassWindow(e->hwnd, EnchantmentProc);

   TooltipAddWindow(e->hwnd, hInst, (int) LPSTR_TEXTCALLBACK);

   e->obj  = obj;
   switch (type)
   {
   case ENCHANT_PLAYER:
      player_enchantments = list_add_item(player_enchantments, e);
      EnchantmentsMovePlayer();
      break;

   case ENCHANT_ROOM:
      room_enchantments = list_add_item(room_enchantments, e);
      EnchantmentsMoveRoom();
      break;

   default:
      debug(("EnchantmentAdd got unknown enchantment type %d\n", type));
      EnchantmentDestroy(e);
      break;
   }
}
/****************************************************************************/
/*
 * EnchantmentRemove:  Remove given enchantment of given type.
 */
void EnchantmentRemove(BYTE type, ID obj_id)
{
   Enchantment *e;

   switch (type)
   {
   case ENCHANT_PLAYER:
      e = (Enchantment *)
         list_find_item(player_enchantments, (void *) obj_id, CompareIdEnchantment);
      if (e != NULL)
      {
         player_enchantments = list_delete_item(player_enchantments, (void *) obj_id,
                                                CompareIdEnchantment);
         EnchantmentDestroy(e);
         EnchantmentsMovePlayer();
      }
      break;

   case ENCHANT_ROOM:
      e = (Enchantment *)
         list_find_item(room_enchantments, (void *) obj_id, CompareIdEnchantment);
      if (e != NULL)
      {
         room_enchantments = list_delete_item(room_enchantments, (void *) obj_id,
                                              CompareIdEnchantment);
         EnchantmentDestroy(e);
         EnchantmentsMoveRoom();
      }
      break;

   default:
      debug(("EnchantmentRemove got unknown enchantment type %d\n", type));
      break;
   }
}
/************************************************************************/
/*
 * EnchantmentsResetData:  Ditch current enchantments when data reset.
 */
void EnchantmentsResetData(void)
{
   room_enchantments   = EnchantmentListDestroy(room_enchantments);
   player_enchantments = EnchantmentListDestroy(player_enchantments);
   RequestEnchantments(ENCHANT_PLAYER);
}
/************************************************************************/
/*
 * EnchantmentsResize:  Called when the main window is resized.
 */
void EnchantmentsResize(int xsize, int ysize, AREA *view)
{
   RECT rcOffLimits;

   player_enchant_x     = view->x + view->cx + LEFT_BORDER + LEFT_BORDER + 2;
   player_enchant_right = xsize - RIGHT_BORDER - ENCHANT_SIZE;
   player_enchant_y = TOP_BORDER + EDGETREAT_HEIGHT + USERAREA_HEIGHT +
     ENCHANT_BORDER + MAPTREAT_HEIGHT;
   player_enchant_bottom = player_enchant_y +
     ((ENCHANT_SIZE + ENCHANT_BORDER) * 2);

   room_enchant_x = view->x + view->cx;
   if (cinfo->config->toolbar)
     room_enchant_y = TOOLBAR_Y + (TOOLBAR_BUTTON_HEIGHT - ENCHANT_SIZE) / 2;
   else room_enchant_y = ENCHANT_BORDER + EDGETREAT_HEIGHT;

   room_enchant_left = view->x;
   if (cinfo->config->toolbar)
   {
      ToolbarGetUnionRect(&rcOffLimits);
      room_enchant_left = rcOffLimits.right;
   }
   if (cinfo->config->lagbox)
   {
      Lagbox_GetRect(&rcOffLimits);
      room_enchant_left = rcOffLimits.right;
   }

	EnchantmentsMove();
}
/************************************************************************/
/*
 * EnchantmentsMove:  Move enchantment windows.
 */
void EnchantmentsMove(void)
{
   EnchantmentsMovePlayer();
   EnchantmentsMoveRoom();
}
/************************************************************************/
/*
 * EnchantmentsMovePlayer:  Move player enchantment windows.
 */
void EnchantmentsMovePlayer(void)
{
   list_type l;
   int x, y, width, height;

   x = player_enchant_x;
   y = player_enchant_y;
   width = ENCHANT_SIZE;
   height = ENCHANT_SIZE;
   for (l = player_enchantments; l != NULL; l = l->next)
   {
      Enchantment *e = (Enchantment *) (l->data);

      if (x + width > player_enchant_right)
      {
		  x = player_enchant_x;
		  y += height + ENCHANT_BORDER;
      }

	  if (y + height > player_enchant_bottom)
	  {
		  ShowWindow(e->hwnd, SW_HIDE);
		  continue;
	  }

      MoveWindow(e->hwnd, x, y, width, height, TRUE);

      x += width + ENCHANT_BORDER;
      ShowWindow(e->hwnd, SW_SHOWNORMAL);
   }
}
/************************************************************************/
/*
 * EnchantmentsMoveRoom:  Move room enchantment windows.
 */
void EnchantmentsMoveRoom(void)
{
   list_type l;
   int x, y, width, height;

   x = room_enchant_x - ENCHANT_SIZE;
   y = room_enchant_y;
   width  = ENCHANT_SIZE;
   height = ENCHANT_SIZE;
   for (l = room_enchantments; l != NULL; l = l->next)
   {
      Enchantment *e = (Enchantment *) (l->data);

      if (x < room_enchant_left)
      {
	 ShowWindow(e->hwnd, SW_HIDE);
	 continue;
      }
      
      MoveWindow(e->hwnd, x, y, width, height, TRUE);

      x -= width + ENCHANT_BORDER;
      ShowWindow(e->hwnd, SW_SHOWNORMAL);
   }
}
/****************************************************************************/
/*
 * EnchantmentsNewRoom:  Reset room enchantments when player changes rooms.
 */
void EnchantmentsNewRoom(void)
{
   room_enchantments   = EnchantmentListDestroy(room_enchantments);
}
/****************************************************************************/
/*
 * EnchantmentDrawItem:  Draw enchantment for given DRAWITEM structure.
 */
Bool EnchantmentDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
{
   Enchantment *e;
   AREA area;
   RECT r;
   POINT p;
   
   switch (lpdis->itemAction)
   {
   case ODA_SELECT:
   case ODA_DRAWENTIRE:
      e = EnchantmentFindByWindow(lpdis->hwndItem);
      if (e == NULL)
	 break;

      // Draw window background
      GetWindowRect(lpdis->hwndItem, &r);
      p.x = r.left;
      p.y = r.top;
      ScreenToClient(cinfo->hMain, &p);
      OffscreenWindowBackground(NULL, p.x, p.y, ENCHANT_SIZE, ENCHANT_SIZE);

      area.x = area.y = 0;
      area.cx = area.cy = ENCHANT_SIZE;
      DrawStretchedObjectGroup(lpdis->hDC, e->obj, e->obj->animate->group, &area, NULL);

      return True;
   }
   return False;
}
/****************************************************************************/
/*
 * EnchantmentListDestroy:  Free memory for list of enchantments.  Returns NULL.
 */
list_type EnchantmentListDestroy(list_type list)
{
   list_type l;

   for (l = list; l != NULL; l = l->next)
   {
      Enchantment *e = (Enchantment *) (l->data);
      EnchantmentDestroy(e);
   }
   
   list_destroy(list);
   return NULL;
}
/****************************************************************************/
/*
 * EnchantmentDestroy:  Free memory for given enchantment.  Returns NULL.
 */
Enchantment *EnchantmentDestroy(Enchantment *e)
{
   DestroyWindow(e->hwnd);
   ObjectDestroyAndFree(e->obj);
   return NULL;
}
/************************************************************************/
/*
 * CompareIdEnchantment: Compare an id # with an enchantment; return True iff they
 *    have the same id #.
 */
Bool CompareIdEnchantment(void *idnum, void *e)
{
   return GetObjId((ID) idnum) == GetObjId(((Enchantment *) e)->obj->id);
}
/************************************************************************/
/*
 * EnchantmentFindByWindow:  Return enchantment with given window, or NULL if none.
 */
Enchantment *EnchantmentFindByWindow(HWND hwnd)
{
   list_type l;

   for (l = player_enchantments; l != NULL; l = l->next)
   {
      Enchantment *e = (Enchantment *) (l->data);

      if (e->hwnd == hwnd)
	 return e;
   }
   for (l = room_enchantments; l != NULL; l = l->next)
   {
      Enchantment *e = (Enchantment *) (l->data);

      if (e->hwnd == hwnd)
	 return e;
   }
   return NULL;
}
/************************************************************************/
/* 
 * EnchantmentProc:  Subclass enchantment windows.
 */
long CALLBACK EnchantmentProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
   MSG msg;
   Enchantment *e;
   TOOLTIPTEXT *tt;
   
   msg.hwnd = hwnd;
   msg.message = message;
   msg.wParam = wParam;
   msg.lParam = lParam;

   TooltipForwardMessage(&msg);

   switch (message)
   {
   case WM_ERASEBKGND:
      return 1;

   case WM_RBUTTONDOWN:
      // Examine object
      e = EnchantmentFindByWindow(hwnd);
      if (e != NULL)
      {
	 SetDescParams(cinfo->hMain, DESC_NONE);
	 RequestLook(e->obj->id);
      }
      break;

   case WM_LBUTTONDOWN:
   case WM_LBUTTONUP:
      return TRUE;

   case WM_NOTIFY:
      tt = (TOOLTIPTEXT *) lParam;
      if (tt->hdr.code != TTN_NEEDTEXT)
	 break;
      

      e = EnchantmentFindByWindow(hwnd);
      if (e != NULL)
      {
	 tt->hinst = NULL;
	 tt->lpszText = LookupNameRsc(e->obj->name_res);
      }
      break;
   }

   return CallWindowProc(lpfnDefEnchantmentProc, hwnd, message, wParam, lParam);
}

/************************************************************************/
/*
 * AnimateEnchantments: Animate and redraw enchantment icons.
 *   dt is number of milliseconds since last time animation timer went off.
 */
void AnimateEnchantments(int dt)
{
   Bool need_redraw;
   Enchantment *e;
   list_type l;

   for (l = room_enchantments; l != NULL; l = l->next)
   {
      e = (Enchantment *) (l->data);
      
      need_redraw = AnimateObject(e->obj, dt);
      if (need_redraw)
	 InvalidateRect(e->hwnd, NULL, TRUE);
   }

   for (l = player_enchantments; l != NULL; l = l->next)
   {
      e = (Enchantment *) (l->data);
      
      need_redraw = AnimateObject(e->obj, dt);
      if (need_redraw)
	 InvalidateRect(e->hwnd, NULL, TRUE);
   }
}
