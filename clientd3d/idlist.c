// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * idlist.c:  Manage a list of server IDs.
 *
 * This is just a thin layer on the list implementation.
 */

#include "client.h"

/*****************************************************************************/
/*
 * IDListDelete:  Delete the given ID list and return NULL.
 */
IDList IDListDelete(IDList id_list)
{
   list_delete(id_list);
   return NULL;
}
/*****************************************************************************/
/*
 * IDListMember:  Return True iff given name resource is a member of the id list.
 */
Bool IDListMember(IDList id_list, ID name_res)
{
   return (list_find_item(id_list, (void *) name_res, CompareId) != NULL);
}
/*****************************************************************************/
/*
 * IDListAdd:  Add given name resource to list.  Assumes that it's not already in the list.
 *   Returns new list.
 */
IDList IDListAdd(IDList id_list, ID name_res)
{
   return list_add_item(id_list, (void *) name_res);
}
/*****************************************************************************/
/*
 * IDListRemove:  Remove given id from list, if it's present.
 *   Returns new list.
 */
IDList IDListRemove(IDList id_list, ID name_res)
{
   return list_delete_item(id_list, (void *) name_res, CompareId);
}

