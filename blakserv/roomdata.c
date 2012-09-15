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
 Currently the memory is not kept track of by memory.c because we use
 a common load function for the .roo file with the client.

 */

#include "blakserv.h"

roomdata_node *roomdata;
int num_roomdata;

/* local function prototypes */
Bool LoadRoomFile(char *fname,room_type *file_info);

#define signum(a) ((a)<0 ? -1 : ((a) > 0 ? 1 : 0))

enum
{
   MASK_NORTH = 1,
   MASK_NORTH_EAST = 1 << 1,
   MASK_EAST = 1 << 2,
   MASK_SOUTH_EAST = 1 << 3,
   MASK_SOUTH = 1 << 4,
   MASK_SOUTH_WEST = 1 << 5,
   MASK_WEST = 1 << 6,
   MASK_NORTH_WEST = 1 << 7,
};

void InitRoomData()
{
   roomdata = NULL;
   num_roomdata = 0;
}

void ResetRoomData()
{
   roomdata_node *room,*temp;

   room = roomdata;
   while (room != NULL)
   {
      temp = room->next;
      BSPRoomFreeServer(&(room->file_info));
      FreeMemory(MALLOC_ID_ROOM,room,sizeof(roomdata_node));
      room = temp;
   }
   roomdata = NULL;
   num_roomdata = 0;
}

int LoadRoomData(int resource_id)
{
   val_type ret_val;
   resource_node *r;
   roomdata_node *room;
   room_type file_info;

   r = GetResourceByID(resource_id);
   if (r == NULL)
   {
      bprintf("LoadRoomData can't find resource %i\n",resource_id);
      return NIL;
   }

   if (!LoadRoomFile(r->resource_val,&file_info))
   {
      bprintf("LoadRoomData couldn't open %s!!!\n",r->resource_val);
      return NIL;
   }

   room = (roomdata_node *)AllocateMemory(MALLOC_ID_ROOM,sizeof(roomdata_node));
   room->roomdata_id = num_roomdata++;
   room->file_info = file_info;

   room->next = roomdata;
   roomdata = room;

/*
   dprintf("LoadRoomData read room %i [%i,%i]\n",
	   room->roomdata_id,room->file_info.rows,room->file_info.cols);
*/

   ret_val.v.tag = TAG_ROOM_DATA;
   ret_val.v.data = room->roomdata_id;
   return ret_val.int_val;
}
      
roomdata_node * GetRoomDataByID(int id)
{
   roomdata_node *room;

   room = roomdata;
   while (room != NULL)
   {
      if (room->roomdata_id == id)
	 return room;
      room = room->next;
   }
   return NULL;
}

Bool CanMoveInRoom(roomdata_node *r,int from_row,int from_col,int to_row,int to_col)
{
   int dir_row,dir_col;
   Bool allow,debug;
   Bool bad_to;

   // Must support to_row/to_col which is outside the bounds of the grid.
   // Therefore, must not actually access the grid in those cases.

   debug = ConfigBool(DEBUG_CANMOVEINROOM);
   
   if (!r)
   {
      if (debug)
	 dprintf("-- invalid room, false\n");
      return False;
   }

   /* if not headed into room, don't access grid variables */

   bad_to = False;
   if (to_row < 0 || to_row >= r->file_info.rows)
   {
      if (debug)
	 dprintf("-- not going into room row, false\n");
      bad_to = True;
   }
   if (to_col < 0 || to_col >= r->file_info.cols)
   {
      if (debug)
	 dprintf("-- not going into room col, false\n");
      bad_to = True;
   }

   /* if it's inside a wall or an unwalkable floor, it's no good */
   
   if (debug)
      dprintf("room %i, from row %i, col %i to row %i, col %i\n",
	      r,from_row,from_col,to_row,to_col);
   if (!bad_to &&
       (r->file_info.flags[to_row][to_col] & ROOM_FLAG_WALKABLE) == 0)
   {
      if (debug)
	 dprintf("-- flag grid said no floor, false\n");
      return False;
   }
   
   /* if not currently in room, must be fine */
   if (from_row < 0 || from_row >= r->file_info.rows)
   {
      if (debug)
	 dprintf("-- not in current room row, true\n");
      return True;
   }
   if (from_col < 0 || from_col >= r->file_info.cols)
   {
      if (debug)
	 dprintf("-- not in current room col, true\n");
      return True;
   }

   /*
   dprintf("r%i c%i has data %02X",from_row,from_col,(r->file_info.grid[from_row][from_col]));
   */

   if (abs(to_row-from_row) > 1 || abs(to_col-from_col) > 1)
   {
      if (debug)
	 dprintf("-- allowing teleport\n");

      return True; /* teleport */
   }

   dir_row = signum(to_row-from_row);
   dir_col = signum(to_col-from_col);

   if (dir_row == 0 && dir_col == 0)
   {
      if (debug)
	 dprintf("-- not moving, true\n");

      return True; /* no move */
   }

   /* one of these cases WILL be true */

   switch (dir_row)
   {
   case -1 :
      switch (dir_col)
      {
      case -1 : allow = r->file_info.grid[from_row][from_col] & MASK_NORTH_WEST; break;
      case 0 : allow = r->file_info.grid[from_row][from_col] & MASK_NORTH; break;
      case 1 : allow = r->file_info.grid[from_row][from_col] & MASK_NORTH_EAST; break;
      default : eprintf("CanMoveInRoom got invalid direction %i, %i\n",dir_row,dir_col);
      }
      break;
   case 0 :
      switch (dir_col)
      {
      case -1 : allow = r->file_info.grid[from_row][from_col] & MASK_WEST; break;
      case 1 : allow = r->file_info.grid[from_row][from_col] & MASK_EAST; break;
      default : eprintf("CanMoveInRoom got invalid direction %i, %i\n",dir_row,dir_col);
      }
      break;
   case 1 :
      switch (dir_col)
      {
      case -1 : allow = r->file_info.grid[from_row][from_col] & MASK_SOUTH_WEST; break;
      case 0 : allow = r->file_info.grid[from_row][from_col] & MASK_SOUTH; break;
      case 1 : allow = r->file_info.grid[from_row][from_col] & MASK_SOUTH_EAST; break;
      default : eprintf("CanMoveInRoom got invalid direction %i, %i\n",dir_row,dir_col);
      }
      break;
   default : eprintf("CanMoveInRoom got invalid direction %i, %i\n",dir_row,dir_col);
   }
   /* allow is a bit, not necessarily 1 or 0, so need to make sure to make 1 or 0 here */
   if (debug)
      dprintf("-- using move grid, %s\n",allow ? "true" : "false");
   return (allow != 0);
}

Bool CanMoveInRoomFine(roomdata_node *r,int from_row,int from_col,int to_row,int to_col)
{
   int dir_row,dir_col;
   Bool allow,debug;
   Bool bad_to;

   // Must support to_row/to_col which is outside the bounds of the grid.
   // Therefore, must not actually access the grid in those cases.

   debug = ConfigBool(DEBUG_CANMOVEINROOM);
   
   if (!r)
   {
      if (debug)
	 dprintf("-- invalid room, false\n");
      return False;
   }

   /* if not headed into room, don't access grid variables */

   bad_to = False;
   if (to_row < 0 || to_row >= r->file_info.rows)
   {
      if (debug)
	 dprintf("-- not going into room row, false\n");
      bad_to = True;
   }
   if (to_col < 0 || to_col >= r->file_info.cols)
   {
      if (debug)
	 dprintf("-- not going into room col, false\n");
      bad_to = True;
   }

   /* if it's inside a wall or an unwalkable floor, it's no good */
   
   if (debug)
      dprintf("room %i, from row %i, col %i to row %i, col %i\n",
	      r,from_row,from_col,to_row,to_col);
   if (!bad_to &&
       (r->file_info.flags[to_row][to_col] & ROOM_FLAG_WALKABLE) == 0)
   {
      if (debug)
	 dprintf("-- flag grid said no floor, false\n");
      return False;
   }
   
   /* if not currently in room, must be fine */
   if (from_row < 0 || from_row >= r->file_info.rows)
   {
      if (debug)
	 dprintf("-- not in current room row, true\n");
      return True;
   }
   if (from_col < 0 || from_col >= r->file_info.cols)
   {
      if (debug)
	 dprintf("-- not in current room col, true\n");
      return True;
   }

   /*
   dprintf("r%i c%i has data %02X",from_row,from_col,(r->file_info.grid[from_row][from_col]));
   */

   if (abs(to_row-from_row) > 1 || abs(to_col-from_col) > 1)
   {
      if (debug)
	 dprintf("-- allowing teleport\n");

      return True; /* teleport */
   }

   dir_row = signum(to_row-from_row);
   dir_col = signum(to_col-from_col);

   if (dir_row == 0 && dir_col == 0)
   {
      if (debug)
	 dprintf("-- not moving, true\n");

      return True; /* no move */
   }

   if (r->file_info.monster_grid == NULL)
   {
	   bprintf("CanMoveInRoomFine has no monster grid for %i\n",
			   r->roomdata_id);
	   return True;
   }
   /* one of these cases WILL be true */

   switch (dir_row)
   {
   case -1 :
      switch (dir_col)
      {
      case -1 : allow = r->file_info.monster_grid[from_row][from_col] & MASK_NORTH_WEST; break;
      case 0 : allow = r->file_info.monster_grid[from_row][from_col] & MASK_NORTH; break;
      case 1 : allow = r->file_info.monster_grid[from_row][from_col] & MASK_NORTH_EAST; break;
      default : eprintf("CanMoveInRoomFine got invalid direction %i, %i\n",dir_row,dir_col);
      }
      break;
   case 0 :
      switch (dir_col)
      {
      case -1 : allow = r->file_info.monster_grid[from_row][from_col] & MASK_WEST; break;
      case 1 : allow = r->file_info.monster_grid[from_row][from_col] & MASK_EAST; break;
      default : eprintf("CanMoveInRoomFine got invalid direction %i, %i\n",dir_row,dir_col);
      }
      break;
   case 1 :
      switch (dir_col)
      {
      case -1 : allow = r->file_info.monster_grid[from_row][from_col] & MASK_SOUTH_WEST; break;
      case 0 : allow = r->file_info.monster_grid[from_row][from_col] & MASK_SOUTH; break;
      case 1 : allow = r->file_info.monster_grid[from_row][from_col] & MASK_SOUTH_EAST; break;
      default : eprintf("CanMoveInRoomFine got invalid direction %i, %i\n",dir_row,dir_col);
      }
      break;
   default : eprintf("CanMoveInRoomFine got invalid direction %i, %i\n",dir_row,dir_col);
   }
   /* allow is a bit, not necessarily 1 or 0, so need to make sure to make 1 or 0 here */
   if (debug)
      dprintf("-- using move grid, %s\n",allow ? "true" : "false");
   return (allow != 0);
}

Bool LoadRoomFile(char *fname,room_type *file_info)
{
   char s[MAX_PATH+FILENAME_MAX];

   sprintf(s,"%s%s",ConfigStr(PATH_ROOMS),fname);

   return BSPRooFileLoadServer(s,file_info);
}

