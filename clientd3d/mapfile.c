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
#include <filesystem>

static const int MAPFILE_VERSION_1 = 1;
static const int MAPFILE_VERSION = 2;
static const int MAPFILE_TOP_TABLE_SIZE = 100;      // # of entries in top level table
static const int MAPFILE_LOWER_TABLE_SIZE = 100;    // # of entries in a single offset table

static const char *map_directory = "mail";
static const char *map_filename  = "mail\\game.map";
static const char *map_filename_temp = "mail\\game.map.temp";  // Temporary file name for map file migration

static const char map_magic[] = {0x4D, 0x41, 0x50, 0x0F};

static FILE *mapfile = nullptr;     // Handle of map file

struct LowerTableEntry
{
   int security;
   int offset;
};

static bool MapFileVerifyHeader(FILE *file, bool file_exists);
static bool MapFileFindRoom(FILE* file, int security, bool create);
static bool WriteHeader(FILE *file);
static bool ReadRoomAnnotations(FILE *file, room_type *room, int max_annotations);
static bool SaveRoomAnnotations(FILE *file, room_type *room);
static int FileSize(FILE *file);
static bool MapFileMigrateVersion1ToVersion2();
static bool MapFileMigrateVersion1ToVersion2Data(FILE *destfile);

/*****************************************************************************/
/* 
 * MapFileInitialize:  Open map file when the game is started, or create a blank
 *   one if none exists.
 */
void MapFileInitialize(void)
{
   if (!MakeDirectory(map_directory))
      return;

   bool file_exists = std::filesystem::exists(map_filename);
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
      for (i = 0; i < 4; i++)
         if (fread(&byte, 1, 1, file) != 1 || byte != map_magic[i])
            return false;
      if (fread(&version, 1, 4, file) != 4)
         return false;
      if (version == MAPFILE_VERSION_1)
      {
         // Migrate old version 1 map file to version 2
         debug(("Migrating map file from version 1 to version 2\n"));
         if (!MapFileMigrateVersion1ToVersion2())
         {
            debug(("Map file migration failed!\n"));
            return false;
         }
      }
      else if (version != MAPFILE_VERSION)
      {
         debug(("Bad map file version %d; expecting %d\n", version, MAPFILE_VERSION));
         return false;
      }
   }
   else
   {
      WriteHeader(file);
   }
   return true;
}

bool WriteHeader(FILE *file)
{
   if (fwrite(map_magic, 1, 4, file) != 4)
      return false;
   int version = MAPFILE_VERSION;
   if (fwrite(&version, 1, 4, file) != 4)
      return false;

   // Write out empty header table
   int temp = 0;
   for (int i = 0; i < MAPFILE_TOP_TABLE_SIZE; i++)
      if (fwrite(&temp, 1, 4, file) != 4)
         return false;

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
   int num_walls, i, offset;
   BYTE byte;

   if (mapfile == nullptr)
      return false;

   // See if we've been in this room before
   if (!MapFileFindRoom(mapfile, room->security, false))
      return true;  // No map info for this room; all walls unseen

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
   if (!ReadRoomAnnotations(mapfile, room, MAX_ANNOTATIONS))
      return false;

   return true;
}

bool ReadRoomAnnotations(FILE *file, room_type *room, int max_annotations)
{
   if (room->annotations_offset != 0)
   {
      if (fseek(file, room->annotations_offset, SEEK_SET))
         return false;

      // Read # of annotations
      int num_annotations = 0;
      if (fread(&num_annotations, 1, 4, file) != 4)
         return false;
      num_annotations = std::clamp(num_annotations, 0, max_annotations);

      // Read through each annotation
      for (int i = 0; i < num_annotations; i++)
      {
         if (fread(&room->annotations[i].x, 1, 4, file) != 4)
            return false;
         if (fread(&room->annotations[i].y, 1, 4, file) != 4)
            return false;
         if (fread(room->annotations[i].text, 1, MAX_ANNOTATION_LEN, file) != MAX_ANNOTATION_LEN)
            return false;
         room->annotations[i].text[MAX_ANNOTATION_LEN - 1] = 0;  // Ensure null-termination
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

   if (!MapFileFindRoom(mapfile, room->security, true))
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

   if (!SaveRoomAnnotations(mapfile, room))
   {
      return false;
   }

   return true;
}

bool SaveRoomAnnotations(FILE* file, room_type *room)
{
   int i, offset, num_annotations, temp = 0;

   if (nullptr == file || !room)
      return false;

   // Currently the file pointer is pointing at the 'annotations_offset' field within
   // the room block (if written), which says where the annotations block is. A value
   // of 0 means no annotations block.

   // Get the position of the annotations_offset field
   offset = ftell(file);

   // Write out offset of annotations; write placeholder first in case at end of file
   if (fwrite(&temp, 1, 4, file) != 4)
      return false;

   if (room->annotations_changed && room->annotations_offset == 0)
   {
      // New annotation block will start at the end of the file
      room->annotations_offset = FileSize(file);
   }

   if (room->annotations_offset == 0)
   {
      // No annotations block to write
      return true;
   }

   // Move back to annotations_offset field and write the correct offset
   if (fseek(file, offset, SEEK_SET))
      return false;
   if (fwrite(&room->annotations_offset, 1, 4, file) != 4)
      return false;

   // Write out annotations if they've changed or are being created
   if (room->annotations_changed)
   {
      // Write out number of annotations; write placeholder first in case at end of file - will rewrite later after success
      if (fseek(file, room->annotations_offset, SEEK_SET))
         return false;
      if (fwrite(&temp, 1, 4, file) != 4)
         return false;

      for (i = 0; i < MAX_ANNOTATIONS; i++)
      {
         if (fwrite(&room->annotations[i].x, 1, 4, file) != 4)
            return false;
         if (fwrite(&room->annotations[i].y, 1, 4, file) != 4)
            return false;
         if (fwrite(room->annotations[i].text, 1, MAX_ANNOTATION_LEN, file) != MAX_ANNOTATION_LEN)
            return false;

         if (strlen(room->annotations[i].text) > 0)
         {
            debug(("Wrote annotation '%s'\n", room->annotations[i].text));
         }
      }

      // Now that the annotations block is complete, go back and rewrite the number of annotations
      num_annotations = MAX_ANNOTATIONS;
      if (fseek(file, room->annotations_offset, SEEK_SET))
         return false;
      if (fwrite(&num_annotations, 1, 4, file) != 4)
         return false;
      // Move back to end of annotations block
      if (fseek(file, MAX_ANNOTATIONS * (4 + 4 + MAX_ANNOTATION_LEN), SEEK_CUR))
         return false;
   }

   return true;
}

/*****************************************************************************/
/* 
 * MapFileFindRoom:  Set file pointer of file to start of entry for room with
 *   given security value.  
 *   If create is true, creates a new entry in the file if one doesn't exist.
 *   Return true on success.
 */
bool MapFileFindRoom(FILE *file, int security, bool create)
{
   int pos, next_table, i, file_security, temp;

   if (file == nullptr)
      return false;

   // Seek to header table entry
   pos = 8 + 4 * (abs(security) % MAPFILE_TOP_TABLE_SIZE);
   if (fseek(file, pos, SEEK_SET))
      return false;

   if (fread(&pos, 1, 4, file) != 4)
      return false;

   // Create new offset table if none is there
   if (pos == 0)
   {
      if (!create)
         return false;

      if (fseek(file, -4, SEEK_CUR))
         return false;
      pos = FileSize(file);
      if (fwrite(&pos, 1, 4, file) != 4)
         return false;

      if (fseek(file, pos, SEEK_SET))
         return false;
      next_table = 0;
      if (fwrite(&next_table, 1, 4, file) != 4)
         return false;

      // Fill in empty table, to reserve space in file
      for (i = 0; i < MAPFILE_LOWER_TABLE_SIZE; i++)
      {
         temp = 0;
         if (fwrite(&temp, 1, 4, file) != 4)
            return false;
         if (fwrite(&temp, 1, 4, file) != 4)
            return false;
      }
   }

   // Go to offset table
   if (fseek(file, pos, SEEK_SET))
      return false;

   if (fread(&next_table, 1, 4, file) != 4)
      return false;

   // Search table linearly
   // XXX If not found, check next table or add table
   bool found = false;
   for (i = 0; i < MAPFILE_LOWER_TABLE_SIZE; i++)
   {
      if (fread(&file_security, 1, 4, file) != 4)
         return false;
      if (fread(&pos, 1, 4, file) != 4)
         return false;
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
      if (fseek(file, -8, SEEK_CUR))
         return false;
      pos = FileSize(file);
      if (fwrite(&security, 1, 4, file) != 4)
         return false;
      if (fwrite(&pos, 1, 4, file) != 4)
         return false;
   }

   // Go to map info
   if (fseek(file, pos, SEEK_SET))
      return false;
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

bool MapFileMigrateVersion1ToVersion2()
{
   if (mapfile == nullptr)
      return false;

   // Create an empty temp file
   FILE* destfile = fopen(map_filename_temp, "w+b");
   if (destfile == nullptr)
   {
      debug(("Couldn't open temp map file (%s) for migration\n", map_filename_temp));
      return false;
   }

   // Write new header
   if (!WriteHeader(destfile))
   {
      fclose(destfile);
      return false;
   }

   // Iterate through all rooms in old file and copy them to new file
   if (!MapFileMigrateVersion1ToVersion2Data(destfile))
   {
      fclose(destfile);
      return false;
   }

   // Close both files, delete the old one, and rename the new one
   fclose(destfile);
   fclose(mapfile);
   if (remove(map_filename) != 0)
   {
      debug(("Failed to delete old map file during migration\n"));
   }
   if (rename(map_filename_temp, map_filename) != 0)
   {
      debug(("Failed to rename temp map file to main map file during migration\n"));
   }

   // Reopen the new file as the main map file
   mapfile = fopen(map_filename, "r+b");
   if (!MapFileVerifyHeader(mapfile, true))
   {
      MapFileClose();
      return false;
   }

   return true;
}

/*
 This is the fancy version of the migration function, which verifies each room and annotation block,
 ensuring that any bad data is avoided. It's slower, but safer.

 TODO: Remove this fancy version once we are confident that map files are not corrupt in the wild.
       Do this by deleting this function and MapFileMigrateVersion1ToVersion2(). Then update 
       MapFileVerifyHeader() to accept either version 1 or version 2 files, without any migration.
 Added by Bill Carlson, 12/27/2025
*/
bool MapFileMigrateVersion1ToVersion2Data(FILE* destfile)
{
   int throwaway;
   int file_size = 0;

   if (mapfile == nullptr)
      return false;

   // Get file size to know when we reach the end of the file
   if (fseek(mapfile, 0, SEEK_END))
      return false;
   file_size = ftell(mapfile);

   // Seek past the first 8 bytes (header)
   if (fseek(mapfile, 8, SEEK_SET))
      return false;
   
   // There are 100 top-level entries, each with 100 lower-level entries, which means 10,000 possible rooms
   // All of that room data is too much to hold in memory, but we can keep "pointers" (file offset) to each room and
   // annotations to quickly confirm whether the map annotation is potentially corrupt.
   std::vector<LowerTableEntry> rooms;
   std::vector<int> mapAnnotationOffsets;

   // The 'top table' is the next MAPFILE_TOP_TABLE_SIZE * 4 bytes
   int topTable[MAPFILE_TOP_TABLE_SIZE];
   if (fread(topTable, 4, MAPFILE_TOP_TABLE_SIZE, mapfile) != MAPFILE_TOP_TABLE_SIZE)
      return false;

   // FIRST PASS - Find all valid rooms and map annotation blocks, store their offsets
   for (int i = 0; i < MAPFILE_TOP_TABLE_SIZE; i++)
   {
      if (topTable[i] == 0)
         continue;

      // Go to lower table and read it fully
      if (fseek(mapfile, topTable[i], SEEK_SET))
         return false;
      LowerTableEntry lowerTable[MAPFILE_LOWER_TABLE_SIZE];
      if (fread(&throwaway, 1, 4, mapfile) != 4)  // "Next table pointer" - unused
         return false;
      if (fread(lowerTable, sizeof(LowerTableEntry), MAPFILE_LOWER_TABLE_SIZE, mapfile) != MAPFILE_LOWER_TABLE_SIZE)
         return false;

      // Iterate through each entry in the lower table
      for (int j = 0; j < MAPFILE_LOWER_TABLE_SIZE; j++)
      {
         if (lowerTable[j].security == 0 && lowerTable[j].offset == 0)
            continue;  // Empty entry

         bool off_file = (lowerTable[j].offset <= 0 || lowerTable[j].offset > file_size);
         if (off_file)
         {
            debug(("MapFileMigrateVersion1ToVersion2Data: Found invalid table entry (offset OOB).\n"));
            continue;
         }

         if (lowerTable[j].security == 0)
         {
            debug(("MapFileMigrateVersion1ToVersion2Data: Found invalid table entry (security invalid).\n"));
            continue;
         }

         if (lowerTable[j].offset == 0)
         {
            debug(("MapFileMigrateVersion1ToVersion2Data: Found invalid table entry (offset invalid).\n"));
            continue;
         }

         // Process the room entry here
         if (!MapFileFindRoom(mapfile, lowerTable[j].security, false))
         {
            debug(("MapFileMigrateVersion1ToVersion2Data: Failed to load room with security %d\n", lowerTable[j].security));
            continue;
         }

         if (ftell(mapfile) != lowerTable[j].offset)
         {
            debug(("MapFileMigrateVersion1ToVersion2Data: File position mismatch for room with security %d\n", lowerTable[j].security));
            continue;
         }

         // Process the room entry here - skip past walls, find map annotation offset
         int num_walls = 0;
         if (fread(&num_walls, 1, 4, mapfile) != 4)
            continue;

         // 'num_walls' is the number of bits, so round up, divide by 8, and skip that number of bytes
         if (fseek(mapfile, (num_walls + 7) / 8, SEEK_CUR))
            continue;

         // Read the location of the annotations block
         int annotations_offset = 0;
         if (fread(&annotations_offset, 1, 4, mapfile) != 4)
            continue;

         if (annotations_offset < 0 || annotations_offset > file_size)
         {
            debug(("MapFileMigrateVersion1ToVersion2Data: Found invalid map annotation entry (offset OOB).\n"));
            continue;
         }
         
         // Don't push the room info until after we've validated the room
         LowerTableEntry roomEntry;
         roomEntry.security = lowerTable[j].security;
         roomEntry.offset = lowerTable[j].offset;
         rooms.push_back(roomEntry);

         if (annotations_offset > 0)
         {
            mapAnnotationOffsets.push_back(annotations_offset);
         }
      }
   }

   // SECOND PASS - Copy each room's data to the new file
   byte wallData[4096];
   for (size_t i = 0; i < rooms.size(); i++)
   {
      // Find the source room
      if (!MapFileFindRoom(mapfile, rooms[i].security, false))
      {
         debug(("MapFileMigrateVersion1ToVersion2Data: Failed to load source room with security %d\n", rooms[i].security));
         continue;
      }
      // Find the destination room
      if (!MapFileFindRoom(destfile, rooms[i].security, true))
      {
         debug(("MapFileMigrateVersion1ToVersion2Data: Failed to load destination room with security %d\n", rooms[i].security));
         continue;
      }

      // Read the source room data
      int num_walls = 0;
      if (fread(&num_walls, 1, 4, mapfile) != 4)
         return false;
      int num_wall_bytes = (num_walls + 7) / 8;
      if (num_wall_bytes > sizeof(wallData))
      {
         debug(("MapFileMigrateVersion1ToVersion2Data: Room with security %d has too many walls (%d)\n", rooms[i].security, num_walls));
         return false;
      }
      if (fread(wallData, 1, num_wall_bytes, mapfile) != num_wall_bytes)
         return false;

      // Write the room data to the destination file
      if (fwrite(&num_walls, 1, 4, destfile) != 4)
         return false;
      if (fwrite(wallData, 1, num_wall_bytes, destfile) != num_wall_bytes)
         return false;

      // Read the annotation offset from the source file
      room_type temp_room;
      memset(&temp_room, 0, sizeof(room_type));
      if (fread(&temp_room.annotations_offset, 1, 4, mapfile) != 4)
         return false;

      if (temp_room.annotations_offset > 0)
      {
         // Calculate max annotations based on next known data offset
         // Start from file_size (end of file), look through all known rooms and annotation blocks to find the next
         // known data offset after temp_room.annotations_offset
         int next_data = file_size;
         for (size_t j = 0; j < MAPFILE_LOWER_TABLE_SIZE * MAPFILE_TOP_TABLE_SIZE; ++j)
         {
            if (j > rooms.size() && j > mapAnnotationOffsets.size())
               break;
            if (j < rooms.size())
            {
               int offset = rooms[j].offset;
               if (offset > temp_room.annotations_offset && offset < next_data)
               {
                  next_data = offset;
               }
            }
            if (j < mapAnnotationOffsets.size())
            {
               int offset = mapAnnotationOffsets[j];
               if (offset > temp_room.annotations_offset && offset < next_data)
               {
                  next_data = offset;
               }
            }
         }

         int num_annotations = MAX_ANNOTATIONS;
         int max_annotations = (next_data - temp_room.annotations_offset - 4) / (4 + 4 + MAX_ANNOTATION_LEN);
         max_annotations = std::clamp(max_annotations, 0, MAX_ANNOTATIONS);
         if (max_annotations < num_annotations)
         {
            num_annotations = min(num_annotations, max_annotations);
            // In MOST cases we know that only the first annotation is going to be valid, due to the bug that caused it
            num_annotations = min(num_annotations, 1);
            debug(("Detected corrupt map annotations block - reducing number of annotations to %d.\n", num_annotations));
         }

         if (!ReadRoomAnnotations(mapfile, &temp_room, max_annotations))
            return false;
         temp_room.annotations_changed = true;  // Force write of annotations
         temp_room.annotations_offset = 0;      // Force new annotations block (value is also from the old file)
      }

      if (!SaveRoomAnnotations(destfile, &temp_room))
         return false;
   }

   return true;
}

