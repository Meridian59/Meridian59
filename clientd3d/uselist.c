// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * uselist.c:  Deal with list of objects that player is currently using.
 */

#include "client.h"

static list_type use_list = NULL;

/************************************************************************/
void UseListSet(list_type new_using)
{
   list_delete(use_list);

   use_list = new_using;
   ModuleEvent(EVENT_INVENTORY, INVENTORY_USELIST, use_list);
}
/************************************************************************/
void UseListDelete(void)
{
   use_list = list_delete(use_list);
}
/************************************************************************/
/* 
 * UseListGetObjects:  Return a newly allocated list, which contains pointers
 *   into given list inv.  This new list contains those elements of inv that
 *    are being used.
 */
list_type UseListGetObjects(list_type inv)
{
   list_type l, new_list = NULL;
   object_node *obj;

   for (l = inv; l != NULL; l = l->next)
   {
      obj = (object_node *) l->data;

      if (IsInUse(obj->id))
	 new_list = list_add_item(new_list, l->data);
   }
   return new_list;
}
/************************************************************************/
void UseObject(ID obj_id)
{
   use_list = list_add_item(use_list, (void *) obj_id);
   ModuleEvent(EVENT_INVENTORY, INVENTORY_USE, obj_id);
}
/************************************************************************/
void UnuseObject(ID obj_id)
{
   if (list_find_item(use_list, (void *) obj_id, CompareId) == NULL)
      return;  /* Not an error, since we always try to unuse what we drop */

   use_list = list_delete_item(use_list, (void *) obj_id, CompareId);
   ModuleEvent(EVENT_INVENTORY, INVENTORY_UNUSE, obj_id);
}
/************************************************************************/
/*
 * IsInUse:  Return True iff the object with the given id is in use.
 */
Bool IsInUse(ID obj_id)
{
   return (list_find_item(use_list, (void *) obj_id, CompareId) != NULL);
}
