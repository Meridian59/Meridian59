// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * object.c:  Primitive utility procedures for use with game objects.
 */
#include "client.h"

extern player_info player;
extern room_type current_room;
extern int sector_depths[];

/************************************************************************/
/*
 * CompareId: Compare two id #s; return True iff they
 *    have the same id #.
 */
Bool CompareId(void *id1, void *id2)
{
   return (ID) id1 == (ID) id2;
}
/************************************************************************/
/*
 * CompareIdObject: Compare an id # with an object; return True iff they
 *    have the same id #.
 */
Bool CompareIdObject(void *idnum, void *obj)
{
   return GetObjId((ID) idnum) == GetObjId(((object_node *) obj)->id);
}
/************************************************************************/
/*
 * CompareIdRoomObject: Compare an id # with an object in a list of room contents; 
 *    return True iff they have the same id #.
 */
Bool CompareIdRoomObject(void *idnum, void *obj)
{
   return GetObjId((ID) idnum) == GetObjId(((room_contents_node *) obj)->obj.id);
}
/************************************************************************/
/*
 * CompareRoomObjectDistance:  Sorting comparison procedure for room objects
 *    based on distance.
 */
int CompareRoomObjectDistance(void *r1, void *r2)
{
   return ((room_contents_node *) r1)->distance - ((room_contents_node *) r2)->distance;
}
/*****************************************************************************/
/*
 * OverlayListDestroy:  Free memory associated with a list of overlays, and return NULL.
 */
list_type OverlayListDestroy(list_type overlays)
{
   // Method of freeing memory of overlays depends on method of allocation in 
   // ExtractOverlays (server.c) and in ObjectCopy.

   list_destroy(overlays);
   return NULL;
}
/*****************************************************************************/
/*
 * ObjectGetBlank:  Return a newly allocated object, with all fields set to 0.
 */
object_node *ObjectGetBlank(void)
{
   object_node *obj = (object_node *) SafeMalloc(sizeof(object_node));
   memset(obj, 0, sizeof(object_node));
   obj->overlays = &obj->normal_overlays;
   obj->animate  = &obj->normal_animate;
   return obj;
}
/*****************************************************************************/
/*
 * ObjectCopy:  Return a newly allocated copy of the given object.
 */
object_node *ObjectCopy(object_node *obj)
{
   object_node *temp;
   list_type l;
   Overlay *old_overlay, *new_overlay;

   temp = (object_node *) SafeMalloc(sizeof(object_node));
   memset(temp, 0, sizeof(object_node));
   temp->id = obj->id;
   temp->name_res = obj->name_res;
   temp->icon_res = obj->icon_res;
   temp->flags  = obj->flags;
   temp->drawingtype  = obj->drawingtype;
   temp->minimapflags  = obj->minimapflags;
   temp->namecolor = obj->namecolor;
   temp->objecttype = obj->objecttype;
   temp->moveontype = obj->moveontype;
   temp->amount = obj->amount;
   temp->temp_amount = obj->temp_amount;
   temp->translation = obj->translation;
   temp->normal_translation = obj->normal_translation;
   temp->secondtranslation = obj->secondtranslation;
   memcpy(&temp->normal_animate, &obj->normal_animate, sizeof(Animate));

   // Copy overlay structures
   for (l = obj->normal_overlays; l != NULL; l = l->next)
   {
      old_overlay = (Overlay *) (l->data);
      new_overlay = (Overlay *) SafeMalloc(sizeof(Overlay));
      memcpy(new_overlay, old_overlay, sizeof(Overlay));
      
      temp->normal_overlays = list_add_item(temp->normal_overlays, new_overlay);
   }

   temp->animate  = &temp->normal_animate;
   temp->overlays = &temp->normal_overlays;
   return temp;
}
/*****************************************************************************/
/*
 * ObjectDestroy:  Free memory associated with an object.
 */
void ObjectDestroy(object_node *obj)
{
   obj->normal_overlays = OverlayListDestroy(obj->normal_overlays);
}
/*****************************************************************************/
/*
 * ObjectDestroyAndFree:  Free memory associated with an object, and the object itself.
 *   Return NULL.
 */
object_node *ObjectDestroyAndFree(object_node *obj)
{
   obj->normal_overlays = OverlayListDestroy(obj->normal_overlays);
   SafeFree(obj);
   return NULL;
}
/*****************************************************************************/
/*
 * ObjectListDestroy:  Free memory associated with a list of object_nodes.
 *   Returns NULL.
 */
list_type ObjectListDestroy(list_type obj_list)
{
   list_type l;

   for (l = obj_list; l != NULL; l = l->next)
      ObjectDestroy((object_node *) l->data);

   list_destroy(obj_list);
   return NULL;
}
/*****************************************************************************/
/*
 * RoomObjectDestroy:  Free memory associated with a room object.  Returns NULL.
 */
room_contents_node *RoomObjectDestroy(room_contents_node *r)
{
   OverlayListDestroy(r->motion.overlays);
   ObjectDestroy(&r->obj);   
   return NULL;
}
/*****************************************************************************/
/*
 * RoomObjectDestroyAndFree:  Free memory associated with a room object, and the
 *   room object itself.  Returns NULL.
 */
room_contents_node *RoomObjectDestroyAndFree(room_contents_node *r)
{
   RoomObjectDestroy(r);   
   SafeFree(r);
   return NULL;
}
/*****************************************************************************/
/*
 * RoomObjectListDestroy:  Free memory associated with a list of room_contents_nodes.
 *   Returns NULL.
 */
list_type RoomObjectListDestroy(list_type obj_list)
{
   list_type l;

   for (l = obj_list; l != NULL; l = l->next)
   {
      room_contents_node *r = (room_contents_node *) (l->data);
      RoomObjectDestroy(r);
   }

   list_destroy(obj_list);
   return NULL;
}

/*****************************************************************************/
/*
 * RoomObjectSetHeight:  Set z coordinate of object to floor height at object's position.
 * NOTE:  If it is a hanging object we have to attach to the ceiling height minus the
 * object height.
 */
void RoomObjectSetHeight(room_contents_node *r)
{
   int floor,ceiling;

   GetPointHeights(r->motion.x,r->motion.y,&floor,&ceiling);
   if (r->obj.flags & OF_HANGING)
   {
      if (r->obj.boundingHeight == 0)
      {
	 int width,height;
	 if(GetObjectSize(r->obj.icon_res, r->obj.animate->group, 0, *(r->obj.overlays), 
			 &width, &height))
	 {
	    r->obj.boundingHeight = height;
	    r->obj.boundingWidth = width;
	 }
      }
      r->motion.z = ceiling - r->obj.boundingHeight;
   }
   else if (ROOM_OVERRIDE_MASK & GetRoomFlags())
   {
      int height = floor;
      int depth = GetPointDepth(r->motion.x, r->motion.y);
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
      r->motion.z = height;
   }
   else
   {
      int depth = GetPointDepth(r->motion.x, r->motion.y);
      r->motion.z = floor - sector_depths[depth];
   }
}

/*****************************************************************************/
/*
 * ObjectStopAnimation:  Stop animation for object and its overlays.
 */
void ObjectStopAnimation(object_node *obj)
{
   list_type l;

   AnimateStop(obj->animate);

   for (l = *(obj->overlays); l != NULL; l = l->next)
   {
      Overlay *overlay = (Overlay *) (l->data);
      AnimateStop(&overlay->animate);
   }
}
/*****************************************************************************/
/*
 * RoomObjectSetAnimation:  Set animation for given room object to move animation
 *   if "move" is True, and normal animation otherwise.
 */
void RoomObjectSetAnimation(room_contents_node *r, Bool move)
{
  if (move)
  {
     r->obj.animate  = &r->motion.animate;
     r->obj.overlays = &r->motion.overlays; 
     r->obj.translation = r->motion.translation;
  }
  else
  {
     r->obj.animate  = &r->obj.normal_animate;
     r->obj.overlays = &r->obj.normal_overlays; 
     r->obj.translation = r->obj.normal_translation;
  }
}
