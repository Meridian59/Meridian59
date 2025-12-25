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

static const int MAPFILE_VERSION = 1;
static const int MAPFILE_TOP_TABLE_SIZE = 100;      // # of entries in top level table
static const int MAPFILE_LOWER_TABLE_SIZE = 100;    // # of entries in a single offset table

static const char *map_directory = "mail";
static const char *map_filename  = "mail\\game.map";

static const char map_magic[] = {0x4D, 0x41, 0x50, 0x0F};

static FILE *mapfile = nullptr;     // Handle of map file

static bool MapFileVerifyHeader(FILE *file, bool file_exists);
static bool MapFileFindRoom(int security, bool create);
static int FileSize(FILE *file);
/*****************************************************************************/
/* 
 * MapFileInitialize:  Open map file when the game is started, or create a blank
 *   one if none exists.
 */
void MapFileInitialize(void)
{
   struct stat s;

   if (!MakeDirectory(map_directory))
      return;

   bool file_exists = (stat(map_filename, &s) == 0);
   if (file_exists)
   {
     mapfile = fopen(map_filename, "r+b");
   }
   else
   {
     // File doesn't exist; create a new one
     mapfile = fopen(map_filename, "w+b");
   }
   
   if (mapfile == nullptr)
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
   if (mapfile != nullptr)
   {
     fclose(mapfile);
     mapfile = nullptr;
   }
}
/*****************************************************************************/
/* 
 * MapFileVerifyHeader:  Check that given file is a valid map file.  If file_exists
 *   is false, write out a valid header to the file.
 *   Return true on success.
 */
bool MapFileVerifyHeader(FILE *file, bool file_exists)
{
   int i, version;
   BYTE byte;

   if (file_exists)
   {
     for (i=0; i < 4; i++)
       if (fread(&byte, 1, 1, file) != 1 || byte != map_magic[i])
         return false;
     if (fread(&version, 1, 4, file) != 4 || version > MAPFILE_VERSION)
     {
       debug(("Bad map file version %d; expecting %d\n", version, MAPFILE_VERSION));
       return false;
     }
   }
   else
   {
     if (fwrite(map_magic, 1, 4, file) != 4)
       return false;
     version = MAPFILE_VERSION;
     if (fwrite(&version, 1, 4, file) != 4)
       return false;
     
     // Write out empty header table
     int temp = 0;
     for (i=0; i < MAPFILE_TOP_TABLE_SIZE; i++)
       if (fwrite(&temp, 1, 4, file) != 4)
         return false;
   }
   return true;
}
/*****************************************************************************/
/*
 * MapFileLoadRoom:  Set the "seen" field of each wall in the room to the
 *   value in the map file, if possible.
 *   Return true on success.
 */
bool MapFileLoadRoom(room_type *room)
{
   int num_walls, i, offset, num_annotations;
   BYTE byte;

   if (mapfile == nullptr)
      return false;

   // See if we've been in this room before
   if (!MapFileFindRoom(room->security, false))
      return false;

   if (fread(&num_walls, 1, 4, mapfile) != 4)
      return false;

   if (num_walls != room->num_walls)
   {
      debug(("MapFileSetWalls found %d walls in room; map file has %d\n", num_walls, room->num_walls));
      return false;
   }

   for (i = 0; i < num_walls; i++)
   {
      offset = i % 8;
      if (offset == 0)
      {
         if (fread(&byte, 1, 1, mapfile) != 1)
            return false;
      }
      room->walls[i].seen = ((byte & (1 << offset)) != 0) ? true : false;
   }

   if (fread(&room->annotations_offset, 1, 4, mapfile) != 4)
      return false;

   // Read annotations if present
   if (room->annotations_offset != 0)
   {
      fseek(mapfile, room->annotations_offset, SEEK_SET);

      // Read # of annotations
      if (fread(&num_annotations, 1, 4, mapfile) != 4)
         return false;
      if(num_annotations == MAX_ANNOTATIONS_BAD)
      {
         // Corrupt annotations block. Only read one annotation.
         num_annotations = 1;
      }
      num_annotations = min(num_annotations, MAX_ANNOTATIONS);
      num_annotations = max(num_annotations, 0);

      for (i = 0; i < num_annotations; i++)
      {
         if (fread(&room->annotations[i].x, 1, 4, mapfile) != 4)
            return false;
         if (fread(&room->annotations[i].y, 1, 4, mapfile) != 4)
            return false;
         if (fread(room->annotations[i].text, 1, MAX_ANNOTATION_LEN, mapfile) != MAX_ANNOTATION_LEN)
            return false;
      }
   }

   return true;
}
/*****************************************************************************/
/*
 * MapFileSaveRoom:  Save the "seen" field of each wall in the room to the
 *   map file, if possible.
 *   Return true on success.
 */
bool MapFileSaveRoom(room_type *room)
{
   int i, offset;
   BYTE byte;

   if (nullptr == mapfile || !room || !room->tree || !room->nodes)
      return false;

   if (!MapFileFindRoom(room->security, true))
   {
      debug(("Couldn't make map file entry for room security %d\n", room->security));
      return false;
   }

   if (fwrite(&room->num_walls, 1, 4, mapfile) != 4)
      return false;

   byte = 0;
   for (i = 0; i < room->num_walls; i++)
   {
      offset = i % 8;
      if (room->walls[i].seen)
         byte |= (1 << offset);
      if (offset == 7)
      {
         if (fwrite(&byte, 1, 1, mapfile) != 1)
            return false;
         byte = 0;
      }
   }

   // Write last byte if we ended partway through one
   if (offset != 7 && room->num_walls != 0)
      if (fwrite(&byte, 1, 1, mapfile) != 1)
         return false;

   if (!MapFileSaveRoomAnnotations(room))
   {
      return false;
   }

   return true;
}

bool MapFileSaveRoomAnnotations(room_type *room)
{
   int i, offset, num_annotations, temp = 0;
   bool createAnnotationsBlock = false;

   if (nullptr == mapfile || !room)
      return false;

   // Algorithm:
   //    Currently the file pointer is pointing at the 'annotations_offset' field within the room block (if written), which says where the annotations block is. A value of 0 means no annotations block.
   //    If inside of the annotations block, the number of annotations written is MAX_ANNOTATIONS_BAD, then we can only use the first one - all others are corrupt/missing.
   //    If the number of annotations is different than MAX_ANNOTATIONS, we need to rewrite the annotations block anyway.
   //   We also need to throw out the old annotations block, and rewrite a new one at the end of the file.

   // Get the position of the annotations_offset field
   offset = ftell(mapfile);

   // Write out offset of annotations; write placeholder first in case at end of file
   if (fwrite(&temp, 1, 4, mapfile) != 4)
      return false;

   // See if we are creating a new annotations block
   // - If we currently don't have one and our annotations have changed
   // - If we have one, but our old one is corrupt (MAX_ANNOTATIONS_BAD)
   if (room->annotations_offset == 0)
   {
      if (room->annotations_changed)
      {
         createAnnotationsBlock = true;
      }
   }
   else
   {
      // Read existing annotations block to see if corrupt (or different size)
      fseek(mapfile, room->annotations_offset, SEEK_SET);
      // Read # of annotations
      if (fread(&num_annotations, 1, 4, mapfile) != 4)
         return false;
      if (num_annotations == MAX_ANNOTATIONS_BAD || num_annotations != MAX_ANNOTATIONS)
      {
         // Corrupt (or resized) annotations block - need to rewrite
         createAnnotationsBlock = true;
      }
   }

   if(createAnnotationsBlock)
   {
      // New annotation block will start at the end of the file
      room->annotations_offset = FileSize(mapfile);

      // Move back to annotations_offset field and write the correct offset
      fseek(mapfile, offset, SEEK_SET);
      if (fwrite(&room->annotations_offset, 1, 4, mapfile) != 4)
         return false;
   }

   // Write out annotations if they've changed or are being created
   if (room->annotations_changed || createAnnotationsBlock)
   {
      // Write out number of annotations; write placeholder first in case at end of file - will rewrite later after success
      fseek(mapfile, room->annotations_offset, SEEK_SET);
      if (fwrite(&temp, 1, 4, mapfile) != 4)
         return false;

      for (i = 0; i < MAX_ANNOTATIONS; i++)
      {
         if (fwrite(&room->annotations[i].x, 1, 4, mapfile) != 4)
            return false;
         if (fwrite(&room->annotations[i].y, 1, 4, mapfile) != 4)
            return false;
         if (fwrite(room->annotations[i].text, 1, MAX_ANNOTATION_LEN, mapfile) != MAX_ANNOTATION_LEN)
            return false;
      }

      // Now that the annotations block is complete, go back and rewrite the number of annotations
      num_annotations = MAX_ANNOTATIONS;
      fseek(mapfile, room->annotations_offset, SEEK_SET);
      if (fwrite(&num_annotations, 1, 4, mapfile) != 4)
         return false;
   }

   return true;
}

/*****************************************************************************/
/* 
 * MapFileFindRoom:  Set file pointer of mapfile to start of entry for room with
 *   given security value.  
 *   If create is true, creates a new entry in the file if one doesn't exist.
 *   Return true on success.
 */
bool MapFileFindRoom(int security, bool create)
{
   int pos, next_table, i, file_security, temp;

   if (mapfile == nullptr)
      return false;

   // Seek to header table entry
   pos = 8 + 4 * (abs(security) % MAPFILE_TOP_TABLE_SIZE);
   fseek(mapfile, pos, SEEK_SET);

   if (fread(&pos, 1, 4, mapfile) != 4) return false;

   // Create new offset table if none is there
   if (pos == 0)
   {
      if (!create)
        return false;

      fseek(mapfile, -4, SEEK_CUR);
      pos = FileSize(mapfile);
      if (fwrite(&pos, 1, 4, mapfile) != 4) return false;
      
      fseek(mapfile, pos, SEEK_SET);
      next_table = 0;
      if (fwrite(&next_table, 1, 4, mapfile) != 4) return false;

      // Fill in empty table, to reserve space in file
      for (i=0; i < MAPFILE_LOWER_TABLE_SIZE; i++)
      {
        temp = 0;
        if (fwrite(&temp, 1, 4, mapfile) != 4) return false;
        if (fwrite(&temp, 1, 4, mapfile) != 4) return false;
      }
   }

   // Go to offset table
   fseek(mapfile, pos, SEEK_SET);

   if (fread(&next_table, 1, 4, mapfile) != 4) return false;

   // Search table linearly
   // XXX If not found, check next table or add table
   bool found = false;
   for (i=0; i < MAPFILE_LOWER_TABLE_SIZE; i++)
   {
     if (fread(&file_security, 1, 4, mapfile) != 4) return false;
     if (fread(&pos, 1, 4, mapfile) != 4) return false;
     if (file_security == 0)
       break;
     if (file_security == security)
     {
       found = true;
       break;
     }
   }
   
   if (!found)
   {
     if (!create)
       return false;
     
      if (i == MAPFILE_LOWER_TABLE_SIZE)
      {
        debug(("MapFileFindRoom ran out of table space for security %d\n", security));
        return false;
      }

      // Add map info entry
      fseek(mapfile, -8, SEEK_CUR);
      pos = FileSize(mapfile);
      if (fwrite(&security, 1, 4, mapfile) != 4) return false;
      if (fwrite(&pos, 1, 4, mapfile) != 4) return false;
   }

   // Go to map info
   fseek(mapfile, pos, SEEK_SET);
   return true;
}
/*****************************************************************************/
// Return size of given file, which must be open
static int FileSize(FILE *file) {
  int pos = ftell(file);
  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  fseek(file, pos, SEEK_SET);  // Restore original position
  return size;
}
