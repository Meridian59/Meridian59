// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * overlay.c:  Maintain and draw player overlays, which are first-person 
 *   pictures of the player or objects that the player is carrying.  
 *   These are not drawn in perspective.
 */

#include "client.h"

extern AREA area;                  /* size and position of view window */
extern player_info player;

// Main client windows current viewport area
extern int main_viewport_width;
extern int main_viewport_height;
float player_overlay_scaler = 1;

/* local function prototypes */
Bool ComputePlayerOverlayArea(PDIB pdib, char hotspot, AREA *obj_area);
static void DrawPlayerOverlayBitmap(PDIB pdib, AREA *obj_area, BYTE translation, BYTE secondtranslation, int flags);
static void DrawPlayerOverlayOverlays(PDIB pdib_obj, AREA *obj_area, list_type overlays,
			       Bool underlays, BYTE secondtranslation, int flags);
/************************************************************************/
void SetPlayerOverlay(char hotspot, object_node *poverlay)
{
   ID num;
   
   num = poverlay->id - 1;

   if (num < 0 || num >= NUM_PLAYER_OVERLAYS)
   {
      debug(("SetPlayerOverlay got illegal player overlay #%d\n", poverlay->id));
      ObjectDestroyAndFree(poverlay);
      return;
   }

   // Replace previous player overlay, if any
   if (player.poverlays[num].obj != NULL)
      ObjectDestroyAndFree(player.poverlays[num].obj);

   player.poverlays[num].obj     = poverlay;
   player.poverlays[num].hotspot = hotspot;
   RedrawAll();
}
/************************************************************************/
/*
 * PlayerOverlaysFree:  Free all player overlays
 */
void PlayerOverlaysFree(void)
{
   int i;

   for (i=0; i < NUM_PLAYER_OVERLAYS; i++)
   {
      if (player.poverlays[i].obj != NULL)
	 ObjectDestroyAndFree(player.poverlays[i].obj);
      player.poverlays[i].obj = NULL;
   }
}

/************************************************************************/
/*
 * DrawPlayerOverlays:  Draw first-person views of the player's hands or
 *  objects the player is holding.  
 */
void DrawPlayerOverlays(void)
{
   int i;
   PDIB pdib;
   AREA obj_area;
   object_node *obj;
   list_type overlays;
   room_contents_node *r;
   int flags;

   // Get player's object flags for special drawing effects
   r = GetRoomObjectById(player.id);
   if (r == NULL)
      flags = 0;
   else flags = r->obj.flags;
   
   for (i=0; i < NUM_PLAYER_OVERLAYS; i++)
   {
      PlayerOverlay *poverlay = &player.poverlays[i];

      if (poverlay->obj == NULL || poverlay->hotspot == 0)
	 continue;

      obj = poverlay->obj;

      pdib = GetObjectPdib(obj->icon_res, 0, obj->animate->group);
      if (pdib == NULL)
	 continue;

      ComputePlayerOverlayArea(pdib, poverlay->hotspot, &obj_area);

      // Draw underlays
      overlays = *(obj->overlays);
      if (overlays != NULL)
	 DrawPlayerOverlayOverlays(pdib, &obj_area, overlays, True, obj->secondtranslation, flags | (obj->effect << 20));

      DrawPlayerOverlayBitmap(pdib, &obj_area, obj->translation, obj->secondtranslation, flags | (obj->effect << 20));

      // Draw overlays
      if (overlays != NULL)
	 DrawPlayerOverlayOverlays(pdib, &obj_area, overlays, False, obj->secondtranslation, flags | (obj->effect << 20));
   }
}
/************************************************************************/
/*
 * DrawPlayerOverlayOverlays:  Draw overlays on the base player overlay whose bitmap is pdib_obj.
 *   object_area is the area the player overlay occupies on the screen.
 *   If underlays is True, draw only those overlays which should be drawn
 *     before the base player overlay is drawn.
 *   flags gives the object flags for special drawing effects.
 */
void DrawPlayerOverlayOverlays(PDIB pdib_obj, AREA *obj_area, list_type overlays, Bool underlays,
			       BYTE secondtranslation, int flags)
{
   list_type l;
   AREA overlay_area;
   int angle = 0;
   int pass;  // Which pass of overlays are we on; 0 = under; 1 = normal; 2 = over
   int depth; // Current overlay depth to match

   for (pass = 0; pass < 3; pass++)
   {
      if (underlays)
	 switch (pass)
	 {
	 case 0: depth = HOTSPOT_UNDERUNDER;  break;
	 case 1: depth = HOTSPOT_UNDER;       break;
	 case 2: depth = HOTSPOT_UNDEROVER;   break;
	 }
      else
	 switch (pass)
	 {
	 case 0: depth = HOTSPOT_OVERUNDER;  break;
	 case 1: depth = HOTSPOT_OVER;       break;
	 case 2: depth = HOTSPOT_OVEROVER;   break;
	 }

      for (l = overlays; l != NULL; l = l->next)
      {
	 Overlay *overlay = (Overlay *) (l->data);
	 PDIB pdib_ov;
	 
	 pdib_ov = GetObjectPdib(overlay->icon_res, angle, overlay->animate.group);
	 if (pdib_ov == NULL)
	    continue;
	 
	 if (!FindOverlayArea(pdib_ov, angle, overlay->hotspot, pdib_obj, overlays, 
			      depth, &overlay_area))
	    continue;
	 
	 // Scale offset, and place on base bitmap
	 overlay_area.x = overlay_area.x / OVERLAY_FACTOR + obj_area->x;
	 overlay_area.y = overlay_area.y / OVERLAY_FACTOR + obj_area->y;
	 DrawPlayerOverlayBitmap(pdib_ov, &overlay_area, overlay->translation, secondtranslation, flags | (overlay->effect << 20));
      }
   }
}
/************************************************************************/
/*
 * DrawPlayerOverlayBitmap:  Draw the given bitmap as a player overlay in the 
 *   given area on the screen.
 *   translation gives the palette translation type.
 *   flags gives the object flags for special drawing effects.
 */
void DrawPlayerOverlayBitmap(PDIB pdib, AREA *obj_area, BYTE translation, BYTE secondtranslation, int flags)
{
   room_contents_node *pPlayer;
   DrawObjectInfo dos;
   ViewCone c;

   pPlayer = GetRoomObjectById(player.id);
   c.leftedge = 0;
   c.rightedge = area.cx-1;
   c.top_a = 1;
   c.top_b = 0;
   c.top_d = 0;
   c.bot_a = 1;
   c.bot_b = 0;
   c.bot_d = area.cy-1;

   ZeroMemory(&dos,sizeof(dos));
   dos.pdib     = pdib;
   dos.light    = KOD_LIGHT_LEVELS - 1;
   dos.draw     = True;
   dos.flags    = flags;
   dos.cone     = &c;
   dos.distance = 1;
   dos.cutoff   = main_viewport_height;
   dos.translation = translation;
   dos.secondtranslation = secondtranslation;
   dos.obj      = pPlayer;
   DrawObjectBitmap( &dos, obj_area, False );
}
/************************************************************************/
/*
 * ComputePlayerOverlayArea:  A player overlay with the given PDIB is to be placed
 *   on the given hotspot.  Set area to the area the overlay should occupy.
 *   Return True iff hotspot # is legal.
 */
Bool ComputePlayerOverlayArea(PDIB pdib, char hotspot, AREA *obj_area)
{
   if (hotspot < 1 || hotspot > HOTSPOT_PLAYER_MAX)
   {
      debug(("ComputePlayerOverlayArea found hotspot out of range (%d).\n", (int) hotspot));
      return False;
   }

   float scaler = player_overlay_scaler * 0.5f;
   int dib_width = DibWidth(pdib) * scaler;
   int dib_height = DibHeight(pdib) * scaler;
   int dib_x_offset = DibXOffset(pdib) * scaler;
   int dib_y_offset = DibYOffset(pdib) * scaler;

   // Find x position
   switch (hotspot)
   {
   case HOTSPOT_NW:
   case HOTSPOT_W:
   case HOTSPOT_SW:
      obj_area->x = 0;
      break;

   case HOTSPOT_SE:
   case HOTSPOT_E:
   case HOTSPOT_NE:
      obj_area->x = area.cx - dib_width;
      break;

   case HOTSPOT_N:
   case HOTSPOT_S:
   case HOTSPOT_CENTER:
      obj_area->x = (area.cx - dib_width) / 2;
      break;
   }

   // Find y position
   switch (hotspot)
   {
   case HOTSPOT_NW:
   case HOTSPOT_N:
   case HOTSPOT_NE:
      obj_area->y = 0;
      break;

   case HOTSPOT_SW:
   case HOTSPOT_S:
   case HOTSPOT_SE:
      obj_area->y = area.cy - dib_height;
      break;

   case HOTSPOT_W:
   case HOTSPOT_E:
   case HOTSPOT_CENTER:
      obj_area->y = (area.cy - (dib_height) / 2);
      break;
   }

   obj_area->x += dib_x_offset;
   obj_area->y += dib_y_offset;
   obj_area->cx = dib_width;
   obj_area->cy = dib_height;
   return True;
}

