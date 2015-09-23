// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * roomdata.c
 *

 This module maintains a linked list of loaded .roo files which are
 loaded by the Blakod (using the C function LoadRoom() in ccode.c).

 */

#include "blakserv.h"

room_node* first = NULL;
int        idcounter = 0;

void ResetRooms()
{
   room_node *node, *temp;

   node = first;
   while (node)
   {
      temp = node->next;

      BSPFreeRoom(&node->data);
      FreeMemory(MALLOC_ID_ROOM, node, sizeof(room_node));

      node = temp;
   }

   first = NULL;
   idcounter = 0;
}

int LoadRoom(int resource_id)
{
   val_type ret_val;
   resource_node* r;
   char s[MAX_PATH + FILENAME_MAX];

   /****************************************************************/

   r = GetResourceByID(resource_id);
   if (r == NULL)
   {
      bprintf("LoadRoomData can't find resource %i\n",resource_id);
      return NIL;
   }

   ret_val.v.tag = TAG_ROOM_DATA;

   /****************************************************************/

   // CASE 1: Reuse already loaded ROO
   room_node* node = GetRoomDataByResourceID(resource_id);
   if (node)
   {
      ret_val.v.data = node->data.roomdata_id;
      return ret_val.int_val;
   }

   /****************************************************************/

   // CASE 2: Load ROO
   room_node* newnode = (room_node*)AllocateMemory(MALLOC_ID_ROOM, sizeof(room_node));

   // combine path for roo and filename
   sprintf(s, "%s%s", ConfigStr(PATH_ROOMS), r->resource_val[0]);

   // try load it
   if (!BSPLoadRoom(s, &newnode->data))
   {
      FreeMemory(MALLOC_ID_ROOM, newnode, sizeof(room_node));
      bprintf("LoadRoomData couldn't open %s!!!\n",r->resource_val[0]);
      return NIL;
   }

   newnode->data.roomdata_id = idcounter++;
   newnode->data.resource_id = resource_id;
   newnode->next = NULL;

   if (!first)
	   first = newnode;

   else
   {
      node = first;
	  
	  // goto last element
      while (node->next)	  
         node = node->next;

	  node->next = newnode;
   }

   ret_val.v.data = newnode->data.roomdata_id;
   return ret_val.int_val;
}

void UnloadRoom(room_node *r)
{
   room_node* node     = NULL;
   room_node* previous = NULL;
   
   if (r == NULL)
   {
      bprintf("UnloadRoomData called with NULL room!");
      return;
   }

   node = first;
   while (node)
   {
      if (node->data.roomdata_id == r->data.roomdata_id)
      {
         // removing first element
         if (!previous)		  
            first = node->next;

         // removing not first element
         else		  
            previous->next = node->next;
		  
         // now cleanup node
         BSPFreeRoom(&node->data);
		 FreeMemory(MALLOC_ID_ROOM, node, sizeof(room_node));

         return;
      }

      previous = node;
      node = node->next;
   }

   // If we get to this point, we didn't find the room we wanted to unload.
   bprintf("Room %i not freed in UnloadRoomData!",r->data.roomdata_id);

   return;
}

room_node* GetRoomDataByID(int id)
{
   room_node *node;

   node = first;
   while (node)
   {
      if (node->data.roomdata_id == id)
         return node;
      node = node->next;
   }
   return NULL;
}

room_node* GetRoomDataByResourceID(int id)
{
   room_node *node;

   node = first;
   while (node)
   {
      if (node->data.resource_id == id)
         return node;
      node = node->next;
   }
   return NULL;
}