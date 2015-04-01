// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * client3d.c:  Assorted support routines related to 3D drawing.
 */

#include "client.h"

extern room_type current_room;
extern BOOL bg_overlayVisible;
extern player_info player;

/* Visible object info is filled in by draw3d.c */
extern int num_visible_objects;
extern ObjectRange visible_objects[];  /* Where objects are on screen */

extern int sector_depths[];
extern RECT rcBackgroundOveray;

/************************************************************************/
/*
 * GetObjects3D:  Return a list of all objects that cover the point (x, y),
 *   in order of increasing distance.
 *   This point is measured from the origin of the visible window.
 *   If x and y are the value NO_COORD_CHECK, the coordinates of each object
 *   are not checked.
 *   Includes only objects closer than "distance" from viewer (in FINENESS units),
 *   unless distance = 0, in which case distance isn't taken into account.
 *   Includes only objects that have at least one of "pos_flags" set, 
 *   and none of "neg_flags" unset.
 *   The object drawing effects are handled separately:  if any drawing effect
 *   is set in pos_flags, then the object will only be included in the list
 *   if it has exactly the same drawing effect.
 *   The list is newly allocated and should be freed by the caller.
 *   The "distance" field of each element in the list is also filled in.
 */
list_type GetObjects3D(int x, int y, int distance, int pos_flags, int neg_flags,
                       BYTE pos_drawingtype, BYTE neg_drawingtype)
{
   static room_contents_node sunNode;
   list_type new_list = NULL;
   room_contents_node *r;
   int i, obj_flags, obj_drawingtype;
   extern Bool map;

   if (IsBlind())
      return NULL;

   if (map)
      return NULL;

   for (i=0; i < num_visible_objects; i++)
   {
      if (distance > 0 && visible_objects[i].distance > distance)
         continue;

      r = GetRoomObjectById(visible_objects[i].id);
      if (r == NULL)
         continue;

      if (visible_objects[i].id != r->obj.id)
         continue;

      // Check screen area, if desired
      if (x != NO_COORD_CHECK && y != NO_COORD_CHECK)
      {
         if (visible_objects[i].left_col > x ||
         visible_objects[i].right_col < x ||
         visible_objects[i].top_row > y ||
         visible_objects[i].bottom_row < y)
            continue;
      }

      obj_flags = r->obj.flags;
      obj_drawingtype = r->obj.drawingtype;

      if ((pos_drawingtype != 0 && obj_drawingtype != pos_drawingtype)
      || (neg_drawingtype != 0 && obj_drawingtype == neg_drawingtype))
         continue;

      if ((pos_flags == 0 || (obj_flags & pos_flags) != 0) && (obj_flags & neg_flags) == 0)
      {
         r->distance = visible_objects[i].distance;
         new_list = list_add_sorted_item(new_list, r, CompareRoomObjectDistance);
      }
   }

   if (NULL == new_list)
   {
      list_type l;
      for (l = current_room.bg_overlays; l != NULL; l = l->next)
      {
         BackgroundOverlay *overlay = (BackgroundOverlay *)(l->data);
         if (overlay->drawn)
         {
            POINT pt;
            pt.x = x;
            pt.y = y;
            if (PtInRect(&overlay->rcScreen,pt))
            {
               memset(&sunNode,0,sizeof(sunNode));
               memcpy(&sunNode.obj,&overlay->obj,sizeof(object_node));
               new_list = list_add_item(new_list,&sunNode);
            }
         }
      }
   }
   return new_list;
}
/************************************************************************/
/*
 * GetObjectByPosition:  Return the room_contents_node for the closest object
 *   under (x, y), or NULL if none.  Arguments are as in GetObjects3D above,
 *   except that function also handles drawingtype and this one does not.
 */
room_contents_node *GetObjectByPosition(int x, int y, int distance, int pos_flags, int neg_flags)
{
   room_contents_node *r;
   extern Bool map;
   int i;

   if (IsBlind())
      return NULL;

   if (map)
      return NULL;

   for (i=0; i < num_visible_objects; i++)
   {
      if (distance > 0 && visible_objects[i].distance > distance)
	 continue;

      r = GetRoomObjectById(visible_objects[i].id);
      if (r == NULL)
	 continue;

      if (visible_objects[i].id == r->obj.id &&
	  visible_objects[i].left_col <= x && 
	  visible_objects[i].right_col >= x &&
	  visible_objects[i].top_row <= y &&
	  visible_objects[i].bottom_row >= y &&
	  (pos_flags == 0 || ((r->obj.flags & pos_flags) != 0)) &&
	  ((r->obj.flags & neg_flags) == 0))
      {
	 r->distance = visible_objects[i].distance;
	 return r;
      }
   }

   return NULL;
}
/************************************************************************/
/*
 * GetVisibleObjects: Set objs to point to an array of all visible object IDs.
 *   Return the number of objects in the array.
 */
int GetVisibleObjects(ObjectRange **objs)
{
   *objs = visible_objects;
   return num_visible_objects;
}
/************************************************************************/
/*
 * FindVisibleObjectById:  Return the ObjectRange structure for the given object id,
 *   or NULL if it isn't present.
 */
ObjectRange *FindVisibleObjectById(ID obj_id)
{
   int i;

   for (i=0; i < num_visible_objects; i++)
     if (visible_objects[i].id == obj_id)
       return &visible_objects[i];
   return NULL;
}
/************************************************************************/
/*
 * GetPointFloor:  Return height of floor at (x, y).  If (x, y) is not in
 *   a leaf node, return -1.
 */
int GetPointFloor(int x, int y)
{
   BSPleaf *leaf = BSPFindLeafByPoint(current_room.tree, x, y);

   if (leaf == NULL || leaf->sector == NULL)
      return -1;

   return GetFloorHeight(x, y, leaf->sector);
}
/************************************************************************/
/*
 * GetPointCeiling:  Return sector height at (x, y).  If (x, y) is not in
 *   a leaf node, return -1.
 */
int GetPointCeiling(int x, int y)
{
   BSPleaf *leaf = BSPFindLeafByPoint(current_room.tree, x, y);

   if (leaf == NULL || leaf->sector == NULL)
      return -1;

   return GetCeilingHeight(x, y, leaf->sector);
}
/************************************************************************/
/*
 * GetPointFloor:  Set floor to height of floor at (x, y), and ceiling to height
 *   of ceiling.  If (x, y) is not in a leaf node, return False; otherwise return True.
 */
Bool GetPointHeights(int x, int y, int *floor, int *ceiling)
{
   BSPleaf *leaf = BSPFindLeafByPoint(current_room.tree, x, y);

   if (leaf == NULL || leaf->sector == NULL)
      return False;

   *floor   = GetFloorHeight(x, y, leaf->sector);
   *ceiling = GetCeilingHeight(x, y, leaf->sector);
   return True;
}
/************************************************************************/
PDIB GetPointCeilingTexture(int x, int y)
{
	BSPleaf *leaf = BSPFindLeafByPoint(current_room.tree, x, y);

	if (leaf == NULL || leaf->sector == NULL)
		return NULL;

	return leaf->sector->ceiling;
}

PDIB GetPointFloorTexture(int x, int y)
{
	BSPleaf *leaf = BSPFindLeafByPoint(current_room.tree, x, y);

	if (leaf == NULL || leaf->sector == NULL)
		return NULL;

	return leaf->sector->floor;
}

int GetPointDepth(int x, int y)
{
	BSPleaf *leaf = BSPFindLeafByPoint(current_room.tree, x, y);

	if (leaf == NULL || leaf->sector == NULL)
		return SF_DEPTH0;

	// returns one of SF_DEPTH0 - SF_DEPTH3
	return SectorDepth(leaf->sector->flags);
}
/************************************************************************/
/*
 * GetFloorBase:  Return height of floor at (x, y), including floor "depth" as indicated
 *   by sector flags.
 * If (x, y) is not in a leaf node, return -1.
 */
int GetFloorBase(int x, int y)
{
   Sector *s;
   BSPleaf *leaf = BSPFindLeafByPoint(current_room.tree, x, y);
   int height, depth;

   if (leaf == NULL || leaf->sector == NULL)
      return -1;

   s = leaf->sector;
   depth = SectorDepth(s->flags);
   height = GetFloorHeight(x, y, s) - sector_depths[depth];
   if (ROOM_OVERRIDE_MASK & GetRoomFlags())
   {
      switch (depth) {
      case SF_DEPTH1:
	 if (ROOM_OVERRIDE_DEPTH1 & GetRoomFlags())
	    height = GetOverrideRoomDepth(SF_DEPTH1);
	 break;
      case SF_DEPTH2:
	 if (ROOM_OVERRIDE_DEPTH2 & GetRoomFlags())
	    height = GetOverrideRoomDepth(SF_DEPTH2);
	 break;
      case SF_DEPTH3:
	 if (ROOM_OVERRIDE_DEPTH3 & GetRoomFlags())
	    height = GetOverrideRoomDepth(SF_DEPTH3);
	 break;
      }
   }
   return height;
}
