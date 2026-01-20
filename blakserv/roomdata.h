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

typedef struct roomdata_struct
{
 public:
   int GetSize(void) const
   {
      size_t total = sizeof *this;

      // Track vector's internal buffer capacity
      total += file_info.sectors.capacity() * sizeof(server_sector);

      // Track each sector's polygon vector buffer
      for (auto &sector : file_info.sectors)
      {
         total += sector.polygons.capacity() * sizeof(server_polygon);
      }

      return (int) total;
   }
   struct roomdata_struct *next;
   room_type file_info;
   blak_int roomdata_id;

 private:
} roomdata_node;

enum
{
   ROOM_FLAG_WALKABLE = 0x01
};

void InitRoomData(void);
void ResetRoomData(void);
bool CanMoveInRoom(roomdata_node *r,int from_row,int from_col,int to_row,int to_col);
bool CanMoveInRoomFine(roomdata_node *r,int from_row,int from_col,int to_row,int to_col);
blak_int LoadRoomData(int resource_id);
roomdata_node * GetRoomDataByID(int id);

#endif
