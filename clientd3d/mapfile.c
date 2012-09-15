// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * mapfile.c:  Deal with loading and saving maps.
 *
 * All map information is kept in a single file, currently in the mail directory.
 * We store one bit for each wall in the room file; the bit is 1 if the player has seen
 * the wall, and so it should be shown in the future when he reenters the room.
 * 
 * In the map file, rooms are referenced by their security checksums, which we assume are
 * unique.  A top-level table in the file points to other tables, hashing on the last two
 * digits of the security value.  Lower tables contain (security, file offset) pairs.
 * Each table is of size 100, so this format could hold 10,000 room maps (assuming uniform
 * distribution).  
 *
 * Space is reserved in the lower-level tables for making a linked list of tables, to allow
 * an unlimited number of maps, but this hasn't been implemented yet.  
 *
 * Space is reserved in each map description for more information, such as map annotations,
 * but it currently isn't used.
 */

#include "client.h"

#define MAPFILE_VERSION 1
#define MAPFILE_TOP_TABLE_SIZE 100      // # of entries in top level table
#define MAPFILE_LOWER_TABLE_SIZE 100    // # of entries in a single offset table

static char *map_directory = "mail";
static char *map_filename  = "mail\\game.map";

static char map_magic[] = {0x4D, 0x41, 0x50, 0x0F};

static int mapfile = -1;     // Handle of map file

static Bool MapFileVerifyHeader(int file, Bool file_exists);
static Bool MapFileFindRoom(int security, Bool create);
/*****************************************************************************/
/* 
 * MapFileInitialize:  Open map file when the game is started, or create a blank
 *   one if none exists.
 */
void MapFileInitialize(void)
{
   Bool file_exists;
   struct stat s;

   if (!MakeDirectory(map_directory))
      return;

   file_exists = (stat(map_filename, &s) == 0);

   mapfile = open(map_filename, O_BINARY | O_RDWR | O_CREAT | O_RANDOM, S_IWRITE | S_IREAD);
   if (mapfile == -1)
   {
      debug(("Couldn't open map file\n"));
      return;
   }

   if (!MapFileVerifyHeader(mapfile, file_exists))
   {
      MapFileClose();
      return;
   }
}
/*****************************************************************************/
/* 
 * MapFileClose:  Close the map file when the game is ended.
 */
void MapFileClose(void)
{
   if (mapfile != -1)
   {
      close(mapfile);
      mapfile = -1;
   }
}
/*****************************************************************************/
/* 
 * MapFileVerifyHeader:  Check that given file is a valid map file.  If file_exists
 *   is False, write out a valid header to the file.
 *   Return True on success.
 */
Bool MapFileVerifyHeader(int file, Bool file_exists)
{
   int i, version, temp;
   BYTE byte;

   if (file_exists)
   {
      for (i=0; i < 4; i++)
	 if (read(file, &byte, 1) != 1 || byte != map_magic[i])
	    return False;
      if (read(file, &version, 4) != 4 || version > MAPFILE_VERSION)
      {
	 debug(("Bad map file version %d; expecting %d\n", version, MAPFILE_VERSION));
	 return False;
      }
   }
   else
   {
      for (i=0; i < 4; i++)
	 if (write(file, &map_magic[i], 1) != 1)
	    return False;
      version = MAPFILE_VERSION;
      if (write(file, &version, 4) != 4)
	 return False;

      // Write out empty header table
      temp = 0;
      for (i=0; i < MAPFILE_TOP_TABLE_SIZE; i++)
	 if (write(file, &temp, 4) != 4)
	    return False;
   }
   return True;
}
/*****************************************************************************/
/*
 * MapFileLoadRoom:  Set the "seen" field of each wall in the room to the
 *   value in the map file, if possible.
 *   Return True on success.
 */
Bool MapFileLoadRoom(room_type *room)
{
   int num_walls, i, offset, num_annotations;
   BYTE byte;

   if (mapfile == -1)
      return False;

   // See if we've been in this room before
   if (!MapFileFindRoom(room->security, False))
      return False;

   if (read(mapfile, &num_walls, 4) != 4)
      return False;

   if (num_walls != room->num_walls)
   {
      debug(("MapFileSetWalls found %d walls in room; map file has %d\n", 
	      num_walls, room->num_walls));
      return False;
   }

   for (i=0; i < num_walls; i++)
   {
      offset = i % 8;
      if (offset == 0)
      {
	 if (read(mapfile, &byte, 1) != 1)
	    return False;
      }
      room->walls[i].seen = ((byte & (1 << offset)) != 0) ? True : False;
   }

   if (read(mapfile, &room->annotations_offset, 4) != 4)
     return False;

   // Read annotations if present
   if (room->annotations_offset != 0)
   {
     lseek(mapfile, room->annotations_offset, SEEK_SET);

     // Read # of annotations
     if (read(mapfile, &num_annotations, 4) != 4)
       return False;
     num_annotations = min(num_annotations, MAX_ANNOTATIONS);

     for (i=0; i < num_annotations; i++)
     {
        if (read(mapfile, &room->annotations[i].x, 4) != 4)
	   return False;
        if (read(mapfile, &room->annotations[i].y, 4) != 4)
	   return False;
        if (read(mapfile, room->annotations[i].text, MAX_ANNOTATION_LEN) != MAX_ANNOTATION_LEN)
	   return False;
     }     
   }
   
   return True;
}
/*****************************************************************************/
/*
 * MapFileSaveRoom:  Save the "seen" field of each wall in the room to the
 *   map file, if possible.
 *   Return True on success.
 */
Bool MapFileSaveRoom(room_type *room)
{
   int i, offset, temp;
   BYTE byte;

   if (-1 == mapfile || !room || !room->tree || !room->nodes)
      return False;

   if (!MapFileFindRoom(room->security, True))
   {
      debug(("Couldn't make map file entry for room security %d\n", room->security));
      return False;
   }

   if (write(mapfile, &room->num_walls, 4) != 4)
      return False;

   byte = 0;
   for (i=0; i < room->num_walls; i++)
   {
      offset = i % 8;
      if (room->walls[i].seen)
	 byte |= (1 << offset);
      if (offset == 7)
      {
	 if (write(mapfile, &byte, 1) != 1)
	    return False;
	 byte = 0;
      }
   }

   // Write last byte if we ended partway through one
   if (offset != 7 && room->num_walls != 0)
      if (write(mapfile, &byte, 1) != 1)
	 return False;

   // Write out offset of annotations; write placeholder first in case at end of file
   if (write(mapfile, &temp, 4) != 4)
     return False;

   // See if adding annotations for the first time
   if (room->annotations_changed && room->annotations_offset == 0)
     room->annotations_offset = filelength(mapfile);

   lseek(mapfile, -4, SEEK_CUR);
   if (write(mapfile, &room->annotations_offset, 4) != 4)
      return False;

   // Write out annotations if they've changed
   if (room->annotations_changed)
   {
     lseek(mapfile, room->annotations_offset, SEEK_SET);
     temp = MAX_ANNOTATIONS;
     if (write(mapfile, &temp, 4) != 4)
       return False;
     
     for (i=0; i < MAX_ANNOTATIONS; i++)
     {
        if (write(mapfile, &room->annotations[i].x, 4) != 4)
	   return False;
        if (write(mapfile, &room->annotations[i].y, 4) != 4)
	   return False;
        if (write(mapfile, room->annotations[i].text, MAX_ANNOTATION_LEN) != MAX_ANNOTATION_LEN)
	   return False;
     }
   }

   return True;
}
/*****************************************************************************/
/* 
 * MapFileFindRoom:  Set file pointer of mapfile to start of entry for room with
 *   given security value.  
 *   If create is True, creates a new entry in the file if one doesn't exist.
 *   Return True on success.
 */
Bool MapFileFindRoom(int security, Bool create)
{
   int pos, next_table, i, file_security, temp;
   Bool found;

   if (mapfile == -1)
      return False;

   // Seek to header table entry
   pos = 8 + 4 * (abs(security) % MAPFILE_TOP_TABLE_SIZE);
   lseek(mapfile, pos, SEEK_SET);

   if (read(mapfile, &pos, 4) != 4) return False;

   // Create new offset table if none is there
   if (pos == 0)
   {
      if (!create)
	 return False;

      lseek(mapfile, -4, SEEK_CUR);
      pos = filelength(mapfile);
      if (write(mapfile, &pos, 4) != 4) return False;
      
      lseek(mapfile, pos, SEEK_SET);
      next_table = 0;
      if (write(mapfile, &next_table, 4) != 4) return False;

      // Fill in empty table, to reserve space in file
      for (i=0; i < MAPFILE_LOWER_TABLE_SIZE; i++)
      {
	 temp = 0;
	 if (write(mapfile, &temp, 4) != 4) return False;
	 if (write(mapfile, &temp, 4) != 4) return False;
      }
   }

   // Go to offset table
   lseek(mapfile, pos, SEEK_SET);

   if (read(mapfile, &next_table, 4) != 4) return False;

   // Search table linearly
   // XXX If not found, check next table or add table
   found = False;
   for (i=0; i < MAPFILE_LOWER_TABLE_SIZE; i++)
   {
      if (read(mapfile, &file_security, 4) != 4) return False;
      if (read(mapfile, &pos, 4) != 4) return False;
      if (file_security == 0)
	 break;
      if (file_security == security)
      {
	 found = True;
	 break;
      }
   }
   
   if (!found)
   {
      if (!create)
	 return False;

      if (i == MAPFILE_LOWER_TABLE_SIZE)
      {
	 debug(("MapFileFindRoom ran out of table space for security %d\n", security));
	 return False;
      }

      // Add map info entry
      lseek(mapfile, -8, SEEK_CUR);
      pos = filelength(mapfile);
      if (write(mapfile, &security, 4) != 4) return False;
      if (write(mapfile, &pos, 4) != 4) return False;
   }

   // Go to map info
   lseek(mapfile, pos, SEEK_SET);
   return True;
}
