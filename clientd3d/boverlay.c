// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * boverlay.c:  Deal with background overlays.  These are bitmaps drawn on the flat background
 *   bitmap in the main graphics window.  There are separate protocol messages to add, remove,
 *   and change background overlays.  They are referenced by object IDs.
 */

#include "client.h"

RECT rcBackgroundOveray;
BOOL bg_overlayVisible;

extern room_type current_room;

static Bool CompareBackgroundOverlays(void *b1, void *b2);
static Bool CompareBackgroundOverlayId(void *id, void *b1);
/********************************************************************/
void BackgroundOverlaysReset(void)
{
   list_type l;

   for (l = current_room.bg_overlays; l != NULL; l = l->next)
   {
      BackgroundOverlay *overlay = (BackgroundOverlay *) (l->data);

      BackgroundOverlayDestroyAndFree(overlay);
   }
   current_room.bg_overlays = list_delete(current_room.bg_overlays);
}

/********************************************************************/
void BackgroundOverlayDestroyAndFree(BackgroundOverlay *overlay)
{
   ObjectDestroy(&overlay->obj);
   SafeFree(overlay);
}
/********************************************************************/
void BackgroundOverlayAdd(BackgroundOverlay *overlay)
{
   if (list_find_item(current_room.bg_overlays, overlay, CompareBackgroundOverlays))
   {
      debug(("BackgroundOverlayAdd got duplicate overlay id %d\n", overlay->obj.id));
      BackgroundOverlayDestroyAndFree(overlay);
      return;
   }

   current_room.bg_overlays = list_add_item(current_room.bg_overlays, overlay);
   RedrawAll();
}
/********************************************************************/
void BackgroundOverlayRemove(ID id)
{
   BackgroundOverlay *overlay;

   overlay = (BackgroundOverlay *) list_find_item(current_room.bg_overlays,
                                                  (void *) id, CompareBackgroundOverlayId);
   if (overlay == NULL)
   {
      debug(("BackgroundOverlayRemove couldn't find overlay id %d\n", id));
      return;
   }

   current_room.bg_overlays = 
      list_delete_item(current_room.bg_overlays, (void *) id, CompareBackgroundOverlayId);
   BackgroundOverlayDestroyAndFree(overlay);
   
   RedrawAll();
}
/********************************************************************/
void BackgroundOverlayChange(BackgroundOverlay *overlay)
{
   BackgroundOverlayRemove(overlay->obj.id);
   BackgroundOverlayAdd(overlay);
   RedrawAll();
}

/************************************************************************/
/*
 * CompareBackgroundOverlays: Compare two background overlays; return True iff they
 *    have the same id #.
 */
Bool CompareBackgroundOverlays(void *b1, void *b2)
{
   return ((BackgroundOverlay *) b1)->obj.id == ((BackgroundOverlay *) b2)->obj.id;
}
/************************************************************************/
/*
 * CompareBackgroundOverlayId: Compare a background overlay and an ID; return True iff they
 *    have the same id #.
 */
Bool CompareBackgroundOverlayId(void *id, void *b1)
{
   return ((BackgroundOverlay *) b1)->obj.id == (ID) id;
}
