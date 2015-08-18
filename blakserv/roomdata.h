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
   int roomdata_id;
   room_type file_info;
   struct roomdata_struct *next;
} roomdata_node;

enum
{
   ROOM_FLAG_WALKABLE = 0x01
};

void InitRoomData(void);
void ResetRoomData(void);
Bool CanMoveInRoom(roomdata_node *r,int from_row,int from_col,int to_row,int to_col);
Bool CanMoveInRoomHighRes(roomdata_node *r,int from_row,int from_col,int from_finerow,int from_finecol,
						  int to_row,int to_col,int to_finerow,int to_finecol);
Bool CanMoveInRoomFine(roomdata_node *r,int from_row,int from_col,int to_row,int to_col);
int LoadRoomData(int resource_id);
void UnloadRoomData(roomdata_node *r);
roomdata_node * GetRoomDataByID(int id);
int GetHeight(roomdata_node *r,int row,int col,int finerow,int finecol);

int GetHeightFloorBSP(roomdata_node *r, int row, int col, int finerow, int finecol);
int GetHeightCeilingBSP(roomdata_node *r, int row, int col, int finerow, int finecol);
Bool LineOfSightBSP(roomdata_node *r, int from_row, int from_col, int from_finerow, int from_finecol,
                                      int to_row, int to_col, int to_finerow, int to_finecol);
#endif
