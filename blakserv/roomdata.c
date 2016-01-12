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
 This module maintains an array of pointers to room_node (all the room data)
 and also an array of pointers to room_rsc_node (used for fast room ID lookup
 by resource id). The arrays have a hardcoded size of 400, and each element
 is a linked list of pointers indexed by room_id % 400 for room_nodes, and
 resource_id % 400 for room_rsc_nodes. This allows a much smaller size to be
 set for the array, but can still handle an unforseen case where more than
 400 rooms get created. The hardcoded size (INIT_ROOMTABLE_SIZE in roomdata.h)
 should be increased if the 'normal' number of rooms on the server is ever over
 400 (currently ~270). .roo files loaded by the C function LoadRoom(), called
 from blakod.

 */

#include "blakserv.h"

// Next available room ID.
int           idcounter = 0;
// Array of pointers for room data storage.
room_node     **rooms;

void InitRooms()
{
   rooms = (room_node **)AllocateMemoryCalloc(MALLOC_ID_ROOM,
      INIT_ROOMTABLE_SIZE, sizeof(room_node*));
   idcounter = 0;
}

void ExitRooms()
{
   ResetRooms();
   FreeMemory(MALLOC_ID_ROOM, rooms, INIT_ROOMTABLE_SIZE * sizeof(room_node*));
}

void ResetRooms()
{
   room_node *room, *temp;

   for (int i = 0; i < INIT_ROOMTABLE_SIZE; ++i)
   {
      if (rooms)
      {
         // Free memory from rooms.
         room = rooms[i % INIT_ROOMTABLE_SIZE];
         while (room)
         {
            temp = room->next;
            BSPFreeRoom(&room->data);
            FreeMemory(MALLOC_ID_ROOM, room, sizeof(room_node));
            room = temp;
         }
         rooms[i % INIT_ROOMTABLE_SIZE] = NULL;
      }
   }

   idcounter = 0;
}

int LoadRoom(int resource_id)
{
   val_type ret_val;
   resource_node* r;
   char s[MAX_PATH + FILENAME_MAX];

   /****************************************************************/

   r = GetResourceByID(resource_id);
   if (!r)
   {
      bprintf("LoadRoomData can't find resource %i\n",resource_id);
      return NIL;
   }

   ret_val.v.tag = TAG_ROOM_DATA;

   /****************************************************************/

   // Load ROO
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

   // Add this room_node to the rooms table.
   newnode->data.roomdata_id = idcounter++;
   newnode->data.resource_id = resource_id;
   newnode->next = rooms[newnode->data.roomdata_id % INIT_ROOMTABLE_SIZE];
   rooms[newnode->data.roomdata_id % INIT_ROOMTABLE_SIZE] = newnode;

   ret_val.v.data = newnode->data.roomdata_id;
   return ret_val.int_val;
}

void UnloadRoom(room_node *r)
{
   room_node *room, *temp;
   int room_hash;
   if (!r)
   {
      bprintf("UnloadRoomData called with NULL room!");

      return;
   }

   if (!rooms)
   {
      bprintf("UnloadRoomData couldn't get room table!");

      return;
   }

   room_hash = r->data.roomdata_id % INIT_ROOMTABLE_SIZE;

   // Get rooms occupying this position in rooms table.
   room = rooms[room_hash];

   if (!room)
   {
      bprintf("UnloadRoomData got NULL data for room %i at room table entry %i!",
         r->data.roomdata_id, room_hash);

      return;
   }

   // If the room we want to free is first, set rooms at this position
   // to the next room in this list.
   if (room->data.roomdata_id == r->data.roomdata_id)
   {
      rooms[room_hash] = room->next;
      BSPFreeRoom(&room->data);
      FreeMemory(MALLOC_ID_ROOM, room, sizeof(room_node));
      room = NULL;

      return;
   }

   // Otherwise check the next room in list.
   while (room->next)
   {
      if (room->next->data.roomdata_id == r->data.roomdata_id)
      {
         // Matched, set temp to the room to be freed.
         temp = room->next;
         // Set current room's next pointer to the next pointer
         // of the room we're freeing.
         room->next = room->next->next;
         BSPFreeRoom(&temp->data);
         FreeMemory(MALLOC_ID_ROOM, temp, sizeof(room_node));

         return;
      }
      room = room->next;
   }

   // If we get to this point, we didn't find the room we wanted to unload.
   bprintf("Room %i not freed in UnloadRoomData!", r->data.roomdata_id);

   return;
}

room_node * GetRoomDataByID(int id)
{
   room_node *room;

   if (!rooms)
      return NULL;

   room = rooms[id % INIT_ROOMTABLE_SIZE];
   while (room)
   {
      if (room->data.roomdata_id == id)
         return room;
      room = room->next;
   }

   return NULL;
}

// Prints the rooms in rooms table to admin log.
void PrintRoomTable()
{
   room_node *room;

   if (!rooms)
   {
      aprintf("No rooms table loaded.\n");

      return;
   }

   for (int i = 0; i < INIT_ROOMTABLE_SIZE; i++)
   {
      room = rooms[i];
      while (room)
      {
         aprintf("Room at position %i, roomdata %i, resource %s\n",
            i, room->data.roomdata_id, GetResourceStrByLanguageID(room->data.resource_id,0));
         room = room->next;
      }
   }
}

void ForEachRoom(void(*callback_func)(room_node *r))
{
   room_node *node;

   for (int i = 0; i < INIT_ROOMTABLE_SIZE; i++)
   {
      node = rooms[i];
      while (node)
      {
         callback_func(node);
         node = node->next;
      }
   }
}
