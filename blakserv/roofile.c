// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * roofile.c
 * 
 
 Utility routine to load a room file, written by Andrew.

 */

#include "blakserv.h"

#define ROO_VERSION 4

static unsigned char room_magic[] = { 0x52, 0x4F, 0x4F, 0xB1 };

/*********************************************************************************************/
/*
 * BSPRooFileLoadServer:  Fill "room" with server-relevant data from given roo file.
 *   Return True on success.
 */
Bool BSPRooFileLoadServer(char *fname, room_type *room)
{
   int infile, i, j, temp,roo_version;
   unsigned char byte;

   infile = open(fname, O_BINARY | O_RDONLY);
   if (infile < 0)
      return False;

   // Check magic number and version
   for (i = 0; i < 4; i++)
      if (read(infile, &byte, 1) != 1 || byte != room_magic[i])
      { close(infile); return False; }

   if (read(infile, &roo_version, 4) != 4 || roo_version < ROO_VERSION)
   { close(infile); return False; }

   // Read room security
   if (read(infile, &room->security, 4) != 4)
   { close(infile); return False; }

   // Skip pointer to client info
   if (read(infile, &temp, 4) != 4)
   { close(infile); return False; }

   // Read pointer to server info and seek there
   if (read(infile, &temp, 4) != 4)
   { close(infile); return False; }
   lseek(infile, temp, SEEK_SET);

   // Read size of room
   if (read(infile, &room->rows, 4) != 4)
   { close(infile); return False; }

   if (read(infile, &room->cols, 4) != 4)
   { close(infile); return False; }

   // Allocate and read movement grid
   room->grid = (unsigned char **)AllocateMemory(MALLOC_ID_ROOM,room->rows * sizeof(char *));
   for (i=0; i < room->rows; i++)
   {
      room->grid[i] = (unsigned char *)AllocateMemory(MALLOC_ID_ROOM,room->cols);
      if (read(infile, room->grid[i], room->cols) != room->cols)
      {
	 for (j=0; j <= i; j++)
	    FreeMemory(MALLOC_ID_ROOM,room->grid[i],room->cols);
	 FreeMemory(MALLOC_ID_ROOM,room->grid,room->rows * sizeof(char *));

	 close(infile);
	 return False;
      }
   }

   // Allocate and read flag grid
   room->flags = (unsigned char **)AllocateMemory(MALLOC_ID_ROOM,room->rows * sizeof(char *));
   for (i=0; i < room->rows; i++)
   {
      room->flags[i] = (unsigned char *)AllocateMemory(MALLOC_ID_ROOM,room->cols);
      if (read(infile, room->flags[i], room->cols) != room->cols)
      {
	 for (j=0; j <= i; j++)
	    FreeMemory(MALLOC_ID_ROOM,room->flags[i],room->cols);
	 FreeMemory(MALLOC_ID_ROOM,room->flags,room->rows * sizeof(char *));

	 close(infile);
	 return False;
      }
   }

   room->monster_grid = NULL;
   
   if (roo_version >= 12)
   {   
	   /* dprintf("found a new room version 12\n");*/
	   // Allocate and read monster movement grid
	   room->monster_grid = (unsigned char **)AllocateMemory(MALLOC_ID_ROOM,room->rows * sizeof(char *));
	   for (i=0; i < room->rows; i++)
	   {
		   room->monster_grid[i] = (unsigned char *)AllocateMemory(MALLOC_ID_ROOM,room->cols);
		   if (read(infile, room->monster_grid[i], room->cols) != room->cols)
		   {
			   for (j=0; j <= i; j++)
				   FreeMemory(MALLOC_ID_ROOM,room->monster_grid[i],room->cols);
			   FreeMemory(MALLOC_ID_ROOM,room->monster_grid,room->rows * sizeof(char *));
			   
			   close(infile);
			   return False;
		   }
	   }
	   /*
	   dprintf("%s: %d rows, %d cols\n", fname, room->rows, room->cols);
	   dprintf("monster grid:\n");
	   for (i=0; i < room->rows; i++)
		   for (j=0; j < room->cols; j++)
			   dprintf("%2x ", room->monster_grid[i][j]);
	   */
   }
   
#if 0
   dprintf("%s: %d rows, %d cols\n", fname, room->rows, room->cols);
   for (i=0; i < room->rows; i++)
      for (j=0; j < room->cols; j++)
	 dprintf("%2x ", room->grid[i][j]);
#endif
	 
   close(infile);

   return True;
}
/*********************************************************************************************/
/*
 * BSPRoomFreeServer:  Free the parts of a room structure used by the server.
 */
void BSPRoomFreeServer(room_type *room)
{
   int i;

   for (i=0; i < room->rows; i++)
      FreeMemory(MALLOC_ID_ROOM,room->grid[i],room->cols);
   FreeMemory(MALLOC_ID_ROOM,room->grid,room->rows * sizeof(char *));
   
   for (i=0; i < room->rows; i++)
      FreeMemory(MALLOC_ID_ROOM,room->flags[i],room->cols);
   FreeMemory(MALLOC_ID_ROOM,room->flags,room->rows * sizeof(char *));

   if (room->monster_grid != NULL)
   {
	   for (i=0; i < room->rows; i++)
		   FreeMemory(MALLOC_ID_ROOM,room->monster_grid[i],room->cols);
	   FreeMemory(MALLOC_ID_ROOM,room->monster_grid,room->rows * sizeof(char *));
   }
   room->grid = NULL;
   room->flags = NULL;
   room->monster_grid = NULL;
   room->rows = 0;
   room->cols = 0;
}
