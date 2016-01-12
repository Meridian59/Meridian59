// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * roomdata.h
 *
 */

#ifndef _ROOMDATA_H
#define _ROOMDATA_H

#define INIT_ROOMTABLE_SIZE 400

typedef struct room_node
{
   room_type data;
   struct room_node *next;
} room_node;

void        InitRooms(void);
void        ExitRooms(void);
void        ResetRooms(void);
int         LoadRoom(int resource_id);
void        UnloadRoom(room_node *r);
room_node*  GetRoomDataByID(int id);
void PrintRoomTable();
void ForEachRoom(void(*callback_func)(room_node *r));

#endif
