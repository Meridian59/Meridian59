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
void FreeRoom(roomdata_node *r);

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

   ret_val.v.tag = TAG_ROOM_DATA;
   room = roomdata;

   // Check if we have already allocated this room and are looking to make
   // a duplicate of it. Save memory/cycles by using the identical room data.
   while (room != NULL)
   {
      if (room->file_info.resource_id == resource_id)
      {
         ret_val.v.data = room->roomdata_id;

         return ret_val.int_val;
      }
      room = room->next;
   }

   if (!LoadRoomFile(r->resource_val[0],&file_info))
   {
      bprintf("LoadRoomData couldn't open %s!!!\n",r->resource_val[0]);
      return NIL;
   }

   room = (roomdata_node *)AllocateMemory(MALLOC_ID_ROOM,sizeof(roomdata_node));
   room->roomdata_id = num_roomdata++;
   room->file_info = file_info;
   room->file_info.resource_id = resource_id;

   room->next = roomdata;
   roomdata = room;

/*
   dprintf("LoadRoomData read room %i [%i,%i]\n",
      room->roomdata_id,room->file_info.rows,room->file_info.cols);
*/

   ret_val.v.data = room->roomdata_id;
   return ret_val.int_val;
}

/*
 * UnloadRoomData: Removes the roomdata_node from the roomdata list.
 *                 Calls FreeRoom to free the memory.
 */
void UnloadRoomData(roomdata_node *r)
{
   roomdata_node *room, *prev;

   if (r == NULL)
   {
      bprintf("UnloadRoomData called with NULL room!");

      return;
   }

   room = roomdata;

   // Check if we're unloading the first room in list.
   if (room->roomdata_id == r->roomdata_id)
   {
      roomdata = roomdata->next;
      FreeRoom(r);

      return;
   }

   prev = roomdata;
   room = roomdata->next;

   while (room != NULL)
   {
      if (room->roomdata_id == r->roomdata_id)
      {
         prev->next = room->next;
         // This function makes the calls to free the memory.
         FreeRoom(r);

         return;
      }
      prev = room;
      room = room->next;
   }

   // If we get to this point, we didn't find the room we wanted to unload.
   bprintf("Room %i not freed in UnloadRoomData!",r->roomdata_id);

   return;
}

/*
 * FreeRoom: Frees the memory associated with a room. Assumes
 *           node already removed from roomdata list.
 */
void FreeRoom(roomdata_node *r)
{
   BSPRoomFreeServer(&(r->file_info));
   FreeMemory(MALLOC_ID_ROOM,r,sizeof(roomdata_node));

   return;
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

Bool CanMoveInRoomHighRes(roomdata_node *r,int from_row,int from_col,int from_finerow,int from_finecol,
						  int to_row,int to_col,int to_finerow, int to_finecol)
{
	int dir_row,dir_col,drow,dcol;
	int from_row_comb,from_col_comb,to_row_comb,to_col_comb;
	int from_fullrow,from_fullcol,to_fullrow,to_fullcol;
	unsigned int allow = 0;
	Bool debug;

	// enable debug output or not
	debug = ConfigBool(DEBUG_CANMOVEINROOMHIGHRES);

	// handle null room
	// don't allow move
	if (!r)
	{
		if (debug)
			dprintf("-- invalid room\n");

		return False;
	}

	/*************** CALCULATIONS *************************/

	// build a combined value in fine precision first
	// a row has 64 fine rows, a col has 64 fine cols
	// so a square has 4096 fine squares. 
	// << 6 (LSHIFT 6) is a faster variant of (*64)
	from_row_comb = (from_row << 6) + from_finerow;
	from_col_comb = (from_col << 6) + from_finecol;
	to_row_comb = (to_row << 6) + to_finerow;
	to_col_comb = (to_col << 6) + to_finecol;

	// scale to the highprecision scale
	// a row has 4 highprecision rows, a col has 4 highprecision cols.
	// so highres grid precision is NOT as good as fine precision!
	// a highres row still has 16 finerows.
	// >> 4 (RSHIFT 4) is faster variant of (/16)
	from_row_comb = from_row_comb >> 4;
	from_col_comb = from_col_comb >> 4;
	to_row_comb = to_row_comb >> 4;
	to_col_comb = to_col_comb >> 4;
	
	// get deltas
	drow = to_row_comb - from_row_comb;
	dcol = to_col_comb - from_col_comb;
	
	// here we build values which may add any
	// full row or coll within the fine values
	// to the major value (e.g. turns 1col, 64finecol into 2col)
	// >> 6 (RSHIFT 6) is faster variant of (/64)
	from_fullrow = from_row + (from_finerow >> 6);
	from_fullcol = from_col + (from_finecol >> 6);
	to_fullrow = to_row + (to_finerow >> 6);
	to_fullcol = to_col + (to_finecol >> 6);

	/*******************************************************/

	// Handle non existing highres_grid
	if (r->file_info.highres_grid == NULL)
	{				
		// in case the move was across a big grid square
		// hand it over to CanMoveInRoom (lowres-grid)
		if (abs(to_fullcol - from_fullcol) >= 1 ||
			abs(to_fullrow - from_fullrow) >= 1)
		{
			if (debug)			
				dprintf("room %i, from (%i/%i) to (%i/%i) (HANDOVER, NO HIGHRES)\n",
					r,from_row_comb,from_col_comb,to_row_comb,to_col_comb);
			
			return CanMoveInRoom(r, from_fullrow, from_fullcol, to_fullrow, to_fullcol);
		}

		// else allow small moves within big grid squares
		else
		{
			if (debug)			
				dprintf("room %i, from (%i/%i) to (%i/%i) (ALLOW, NO HIGHRES/SMALL)\n",
					r,from_row_comb,from_col_comb,to_row_comb,to_col_comb);

			return True;
		}
	}

	// don't allow if destination is oudside the grid bounds
	// or if dest square is marked not walkable (=not part of a BSP sector).
	// note: the new highres-flags still keep the walkable bit as first bit
	if (to_row_comb < 0 || to_row_comb >= r->file_info.rowshighres ||
		to_col_comb < 0 || to_col_comb >= r->file_info.colshighres ||
		((r->file_info.highres_grid[to_row_comb][to_col_comb] & ROOM_FLAG_WALKABLE) == 0))
	{
		if (debug)
		{
			dprintf("room %i, from (%i/%i) to (%i/%i) (DENY, DEST OUTSIDE/NON WALKABLE)\n",
				r,from_row_comb,from_col_comb,to_row_comb,to_col_comb);
		}

		return False;	   
	}
	
	// if someone called this with full square increments instead of
	// increments of 16 fineunits, hand it over to the lowres grid
	if (abs(drow) >= 4 || abs(dcol) >= 4)
	{
		if (debug)			
			dprintf("room %i, from (%i/%i) to (%i/%i) (HANDOVER, TOO BIG)\n",
				r,from_row_comb,from_col_comb,to_row_comb,to_col_comb);
			
		return CanMoveInRoom(r, from_fullrow, from_fullcol, to_fullrow, to_fullcol);
	}

	// else if the size is still too big, allow (= teleport of the object)
	// these cases would also (a) rather be part of a path finding solution
	// and (b) would even if linear done between source and destination
	// include major calculations (e.g. determine exit angle to determine direction)
	// if you use highres grid, you must make your moves in 16fine increments (=1 highres increment).
	else if (abs(drow) > 1 || abs(dcol) > 1)
	{
		if (debug)
			dprintf("room %i, from (%i/%i) to (%i/%i) (ALLOW, TELEPORT)\n",
				r,from_row_comb,from_col_comb,to_row_comb,to_col_comb);

		return True;
	}
	
	// allow if the source for whatever reason is on a non-walkable square
	// idea is to allow blocked objects get back in
	if (from_row_comb < 0 || from_row_comb >= r->file_info.rowshighres ||
		from_col_comb < 0 || from_col_comb >= r->file_info.colshighres)
	{
		if (debug)
			dprintf("room %i, from (%i/%i) to (%i/%i) (ALLOW, FROM OUTSIDE)\n",
				r,from_row_comb,from_col_comb,to_row_comb,to_col_comb);

		return True;
	}
	
	// now determine the direction
	// vertical/horizontal, return each in -1,0,1
	dir_row = signum(drow);
	dir_col = signum(dcol);

	// allow moves in no direction (=no move)
	if (dir_row == 0 && dir_col == 0)
	{
		if (debug)
			dprintf("room %i, from (%i/%i) to (%i/%i) (ALLOW NO-MOVE)\n",
				r,from_row_comb,from_col_comb,to_row_comb,to_col_comb);

		return True;
	}

	// now verify the direction against the flags of the source-square
	// note about the new highres-flags:
	// the old movement bits for direction N,NE,E,... are stored at bits 1-9
	// so the value is rightshifted by one to move them back in place
	switch (dir_row)
	{
	case -1 :
		switch (dir_col)
		{
		case -1 : allow = (r->file_info.highres_grid[from_row_comb][from_col_comb] >> 1) & (unsigned int)MASK_NORTH_WEST; break;
		case 0 : allow = (r->file_info.highres_grid[from_row_comb][from_col_comb] >> 1) & (unsigned int)MASK_NORTH; break;
		case 1 : allow = (r->file_info.highres_grid[from_row_comb][from_col_comb] >> 1) & (unsigned int)MASK_NORTH_EAST; break;
		default : eprintf("CanMoveInRoomHighRes got invalid direction %i, %i\n",dir_row,dir_col);
		}
		break;
	case 0 :
		switch (dir_col)
		{
		case -1 : allow = (r->file_info.highres_grid[from_row_comb][from_col_comb] >> 1) & (unsigned int)MASK_WEST; break;
		case 1 : allow = (r->file_info.highres_grid[from_row_comb][from_col_comb] >> 1) & (unsigned int)MASK_EAST; break;
		default : eprintf("CanMoveInRoomHighRes got invalid direction %i, %i\n",dir_row,dir_col);
		}
		break;
   case 1 :
		switch (dir_col)
		{
		case -1 : allow = (r->file_info.highres_grid[from_row_comb][from_col_comb] >> 1) & (unsigned int)MASK_SOUTH_WEST; break;
		case 0 : allow = (r->file_info.highres_grid[from_row_comb][from_col_comb] >> 1) & (unsigned int)MASK_SOUTH; break;
		case 1 : allow = (r->file_info.highres_grid[from_row_comb][from_col_comb] >> 1) & (unsigned int)MASK_SOUTH_EAST; break;
		default : eprintf("CanMoveInRoomHighRes got invalid direction %i, %i\n",dir_row,dir_col);
		}
		break;
	default : eprintf("CanMoveInRoomHighRes got invalid direction %i, %i\n",dir_row,dir_col);
	}

	if (debug)
		dprintf("room %i, from (%i/%i) to (%i/%i), DIR:(%i,%i) ALLOW:%i\n",
			r,from_row_comb,from_col_comb,to_row_comb,to_col_comb,dir_row,dir_col,allow);

	// allow is a bit, not necessarily 1 or 0, so need to make sure to make 1 or 0 here  
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

int GetHeight(roomdata_node *r,int row,int col,int finerow,int finecol)
{
	int highresrow,highrescol;
	
	// no room or no heightmap
	if (!r || r->file_info.highres_grid == NULL)
		return 0;

	// build a combined value in fine precision first
	// then scale it to highres precision
	// formulas see in CanMoveInRoomHighRes
	highresrow = ((row << 6) + finerow) >> 4;
	highrescol = ((col << 6) + finecol) >> 4;
	
	// outside
	if (highresrow >= r->file_info.rowshighres ||
		highrescol >= r->file_info.colshighres)
		return 0;

	// return height from upper 23 bit
	return (r->file_info.highres_grid[highresrow][highrescol] >> 9);
}


int GetHeightFloorBSP(roomdata_node *r, int row, int col, int finerow, int finecol)
{
   if (!r || r->file_info.TreeNodesCount == 0)
      return 0;

   V2 p;
   p.X = FINENESSKODTOROO((float)col * 64.0f + (float)finecol);
   p.Y = FINENESSKODTOROO((float)row * 64.0f + (float)finerow);

   // get floor height (1:1024) with depth modifier
   // (=height of 'object's feet' in water - BELOW water texture)
   float height = BSPGetHeight(&r->file_info, &p, true, true);

   // height of -MIN_KOD_INT indicates a location outside of the map
   // leave this value untouched, otherwise scale from ROO FINENESS to KOD
   // and box value into min/max kod integer
   return (height == (float)-MIN_KOD_INT ? -MIN_KOD_INT : FLOATTOKODINT(FINENESSROOTOKOD(height)));
}

int GetHeightCeilingBSP(roomdata_node *r, int row, int col, int finerow, int finecol)
{
   if (!r || r->file_info.TreeNodesCount == 0)
      return 0;

   V2 p;
   p.X = FINENESSKODTOROO((float)col * 64.0f + (float)finecol);
   p.Y = FINENESSKODTOROO((float)row * 64.0f + (float)finerow);

   // get ceiling height (1:1024)
   float height = BSPGetHeight(&r->file_info, &p, false, false);

   // height of -MIN_KOD_INT indicates a location outside of the map
   // leave this value untouched, otherwise scale from ROO FINENESS to KOD
   // and box value into min/max kod integer
   return (height == (float)-MIN_KOD_INT ? -MIN_KOD_INT : FLOATTOKODINT(FINENESSROOTOKOD(height)));
}

Bool LineOfSightBSP(roomdata_node *r, int from_row, int from_col, int from_finerow, int from_finecol,
                    int to_row, int to_col, int to_finerow, int to_finecol)
{
   if (!r || r->file_info.TreeNodesCount == 0)
      return false;

   V3 s;
   s.X = FINENESSKODTOROO((float)from_col * 64.0f + (float)from_finecol);
   s.Y = FINENESSKODTOROO((float)from_row * 64.0f + (float)from_finerow);
	
   // get floor height with depth modifier
   V2 s2d = { s.X, s.Y };
   s.Z = BSPGetHeight(&r->file_info, &s2d, true, true) + OBJECTHEIGHTROO;
	
   V3 e;
   e.X = FINENESSKODTOROO((float)to_col * 64.0f + (float)to_finecol);
   e.Y = FINENESSKODTOROO((float)to_row * 64.0f + (float)to_finerow);

   // get floor height with depth modifier
   V2 e2d = { e.X, e.Y };
   e.Z = BSPGetHeight(&r->file_info, &e2d, true, true) + OBJECTHEIGHTROO;

   bool los = BSPLineOfSight(&r->file_info, &s, &e);

#if DEBUGLOS
   dprintf("LOS:%i S:(%1.2f/%1.2f/%1.2f) E:(%1.2f/%1.2f/%1.2f)", los, s.X, s.Y, s.Z, e.X, e.Y, e.Z);
#endif

   return los;
}