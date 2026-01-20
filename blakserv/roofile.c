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

static const int SF_SLOPED_FLOOR   = 0x00000400;
static const int SF_SLOPED_CEILING = 0x00000800;

// Macros to reduce repetitive error handling
#define CHECK_SEEK(expr, fname, desc) \
   do { \
      if ((expr) != 0) { \
         eprintf("Failed to seek %s in %s\n", desc, fname); \
         return false; \
      } \
   } while (0)

#define CHECK_READ_INT(expr, fname, desc) \
   do { \
      if ((expr) == -1) { \
         eprintf("Failed to read %s in %s\n", desc, fname); \
         return false; \
      } \
   } while (0)

#define CHECK_READ_SHORT(expr, fname, desc) \
   do { \
      if ((expr) == -1) { \
         eprintf("Failed to read %s in %s\n", desc, fname); \
         return false; \
      } \
   } while (0)

#define CHECK_READ_BYTE(expr, fname, desc) \
   do { \
      if ((expr) == -1) { \
         eprintf("Failed to read %s in %s\n", desc, fname); \
         return false; \
      } \
   } while (0)

#define CHECK_FREAD(expr, fname, desc) \
   do { \
      if ((expr) != 1) { \
         eprintf("Failed to read %s in %s\n", desc, fname); \
         return false; \
      } \
   } while (0)

static int readInt(FILE *fd)              /* 4-byte little-endian signed int   */
{
   int v;
   if (fread(&v, sizeof(int), 1, fd) != 1)
      return -1;
   return v;
}

static short readShort(FILE *fd)          /* 2-byte little-endian signed short */
{
   short v;
   if (fread(&v, sizeof(short), 1, fd) != 1)
      return -1;
   return v;
}

static int readByte(FILE *fd)             /* 1-byte unsigned, returns int for error checking */
{
   // Use unsigned char so byte value 0xFF doesn't collide with -1 error code
   unsigned char v = 0;
   if (fread(&v, 1, 1, fd) != 1)
      return -1;
   return (int)v;
}

static int readCoord(FILE *fd, bool as_float)
{
   char buf[4];
   if (fread(buf, 4, 1, fd) != 1)
      return 0;

   if (as_float)
      return (int)(*((float *)buf));

   // version ≤12 – bytes contain a plain little-endian int
   return *((int *)buf);
}

/*********************************************************************************************/
/*
 * LoadSectorPolygons:  Load the polygons for each sector from the BSP tree
 */
static bool LoadSectorPolygons(FILE *fd, long nodes_start, int num_nodes, room_type *room, const char *fname, bool coords_are_floats)
{
   if (fseek(fd, nodes_start, SEEK_SET) != 0)
   {
      eprintf("LoadSectorPolygons: Failed to seek to nodes_start in %s\n", fname);
      return false;
   }

   for (int n = 0; n < num_nodes; n++)
   {
      unsigned char type;
      CHECK_FREAD(fread(&type, 1, 1, fd), fname, "node type");
      CHECK_SEEK(fseek(fd, 16, SEEK_CUR), fname, "bbox");

      if (type == 1)
      {
         CHECK_SEEK(fseek(fd, 12 + 2 + 2 + 2, SEEK_CUR), fname, "type 1 node data");
      }
      else if (type == 2)
      {
         unsigned short sector_plus1 = readShort(fd);
         short num_pts = readShort(fd);

         if (sector_plus1 < 0 || num_pts < 0)
         {
            eprintf("LoadSectorPolygons: Invalid readShort result in %s (sector_plus1=%d, num_pts=%d)\n", 
                    fname, sector_plus1, num_pts);
            return false;
         }

         int idx = sector_plus1 - 1;
         
         if (idx >= 0 && (size_t)idx < room->sectors.size() && num_pts > 0)
         {
            server_sector *ss = &room->sectors[idx];

            server_polygon poly;
            poly.num_vertices = num_pts;
            poly.vertices_x = (int *) AllocateMemory(MALLOC_ID_ROOM, num_pts * sizeof(int));
            poly.vertices_y = (int *) AllocateMemory(MALLOC_ID_ROOM, num_pts * sizeof(int));
            
            for (int i = 0; i < num_pts; i++)
            {
               poly.vertices_x[i] = readCoord(fd, coords_are_floats);
               poly.vertices_y[i] = readCoord(fd, coords_are_floats);
            }

            ss->polygons.push_back(poly);
         }
         else
         {
            CHECK_SEEK(fseek(fd, num_pts * 8, SEEK_CUR), fname, "invalid polygon data");
         }
      }
      else
      {
         break;
      }
   }
   
   return true;
}

/*********************************************************************************************/
/*
 * BSPRooFileLoadServer:  Load the room data from the given roo file
 */
bool BSPRooFileLoadServer(char *fname, room_type *room)
{
   int i, temp, roo_version;
   unsigned char byte;

   FILE *f = fopen(fname, "rb");;
   if (!f)
   {
      eprintf("BSPRooFileLoadServer: Failed to open file %s\n", fname);
      return false;
   }

   FileGuard infile(f);

   //
   // Header check
   //
   for (i = 0; i < 4; i++)
      if (fread(&byte, 1, 1, infile.get()) != 1 || byte != room_magic[i])
      {
         eprintf("BSPRooFileLoadServer: Failed to read room_magic in %s\n", fname);
         return false;
      }

   //
   // Read room version
   //
   if (fread(&roo_version, 4, 1, infile.get()) != 1 || roo_version < ROO_VERSION)
   {
      eprintf("BSPRooFileLoadServer: Failed to read roo_version in %s\n", fname);
      return false;
   }

   //
   // Read room security (unused by loader)
   //
   CHECK_FREAD(fread(&room->security, 4, 1, infile.get()), fname, "room->security");

   //
   // Read absolute offsets to main (client) and server sections
   //
   int main_off, server_off;
   CHECK_FREAD(fread(&main_off, 4, 1, infile.get()), fname, "main_off");
   CHECK_FREAD(fread(&server_off, 4, 1, infile.get()), fname, "server_off");

   //
   // Client section
   //

   //
   // Seek to Sector metadata and BSP polygons
   //
   CHECK_SEEK(fseek(infile.get(), main_off, SEEK_SET), fname, "main_off");

   //
   // Read in and ignore room width and height (not used on server)
   //
   CHECK_READ_INT(readInt(infile.get()), fname, "room_w");
   CHECK_READ_INT(readInt(infile.get()), fname, "room_h");

   //
   // Read in subsection offsets
   //
   int node_off;
   CHECK_READ_INT(node_off = readInt(infile.get()), fname, "node_off");
   
   //
   // Read in and ignore cwall offset
   //
   CHECK_READ_INT(readInt(infile.get()), fname, "cwall_off");
   
   //
   // Read in and ignore rwall offset
   //
   CHECK_READ_INT(readInt(infile.get()), fname, "rwall_off");
   
   //
   // Read in and ignore sidedef offset
   //
   CHECK_READ_INT(readInt(infile.get()), fname, "sidedef_off");

   //
   // Read in sector offset
   //
   int sector_off;
   CHECK_READ_INT(sector_off = readInt(infile.get()), fname, "sector_off");
   
   //
   // Read in and ignore extra offset
   //
   CHECK_READ_INT(readInt(infile.get()), fname, "extra_off");

   //
   // Sector metadata
   //

   CHECK_SEEK(fseek(infile.get(), sector_off, SEEK_SET), fname, "sector_off");

   int num_sectors;
   CHECK_READ_SHORT(num_sectors = readShort(infile.get()), fname, "num_sectors");
   
   room->sectors.clear();

   if (num_sectors > 0)
   {
      room->sectors.reserve(num_sectors);

      for (i = 0; i < num_sectors; i++)
      {
         server_sector ss;

         ss.id = readShort(infile.get());

         CHECK_READ_SHORT(readShort(infile.get()), fname, "floor_type");      // floor_type (ignore)
         CHECK_READ_SHORT(readShort(infile.get()), fname, "ceiling_type");    // ceiling_type (ignore)
         CHECK_READ_SHORT(readShort(infile.get()), fname, "xoffset");         // xoffset (ignore)
         CHECK_READ_SHORT(readShort(infile.get()), fname, "yoffset");         // yoffset (ignore)
         CHECK_READ_SHORT(readShort(infile.get()), fname, "floorh");          // floorh (ignore)
         CHECK_READ_SHORT(readShort(infile.get()), fname, "ceilh");           // ceilh (ignore)
         CHECK_READ_BYTE(readByte(infile.get()), fname, "light");             // light (ignore)

         int flags;
         CHECK_READ_INT(flags = readInt(infile.get()), fname, "blak_flags");

         CHECK_READ_BYTE(readByte(infile.get()), fname, "animate_speed");      // animate_speed (ignore)

         if (flags & SF_SLOPED_FLOOR)
            CHECK_SEEK(fseek(infile.get(), 46, SEEK_CUR), fname, "floor-slope record");

         if (flags & SF_SLOPED_CEILING)
            CHECK_SEEK(fseek(infile.get(), 46, SEEK_CUR), fname, "ceiling-slope record");

         room->sectors.push_back(ss);
      }
   }

   // Sector polygons
   CHECK_SEEK(fseek(infile.get(), node_off, SEEK_SET), fname, "node_off");

   int num_nodes;
   CHECK_READ_SHORT(num_nodes = readShort(infile.get()), fname, "num_nodes");

   bool coords_are_floats = (roo_version >= 13);
   if (!LoadSectorPolygons(infile.get(), node_off + 2, num_nodes, room, fname, coords_are_floats))
   {
      eprintf("BSPRooFileLoadServer: Failed to load sector polygons in %s\n", fname);
      return false;
   }
   
   // Server section
   // Rows / cols / grids (unchanged)
   CHECK_SEEK(fseek(infile.get(), server_off, SEEK_SET), fname, "server_off");

   CHECK_READ_INT(temp = readInt(infile.get()), fname, "temp (1)");
   
   room->rows = (short) temp;
   
   CHECK_READ_INT(temp = readInt(infile.get()), fname, "temp (2)");

   room->cols = (short) temp;

   room->grid = (unsigned char **) AllocateMemory(MALLOC_ID_ROOM, room->rows * sizeof(char *));
   for (i = 0; i < room->rows; i++)
   {
      room->grid[i] = (unsigned char *) AllocateMemory(MALLOC_ID_ROOM, room->cols);
      if (fread(room->grid[i], room->cols, 1, infile.get()) != 1)
      {
         for (int j = 0; j <= i; j++)
            FreeMemory(MALLOC_ID_ROOM, room->grid[j], room->cols);
         FreeMemory(MALLOC_ID_ROOM, room->grid, room->rows * sizeof(char *));
         return false;
      }
   }

   room->flags = (unsigned char **) AllocateMemory(MALLOC_ID_ROOM, room->rows * sizeof(char *));
   for (i = 0; i < room->rows; i++)
   {
      room->flags[i] = (unsigned char *) AllocateMemory(MALLOC_ID_ROOM, room->cols);
      if (fread(room->flags[i], room->cols, 1, infile.get()) != 1)
      {
         for (int j = 0; j <= i; j++)
            FreeMemory(MALLOC_ID_ROOM, room->flags[j], room->cols);
         FreeMemory(MALLOC_ID_ROOM, room->flags, room->rows * sizeof(char *));
         return false;
      }
   }

   /* optional monster grid for v12+ */
   room->monster_grid = NULL;
   if (roo_version >= 12)
   {
      room->monster_grid = (unsigned char **) AllocateMemory(MALLOC_ID_ROOM, room->rows * sizeof(char *));
      for (i = 0; i < room->rows; i++)
      {
         room->monster_grid[i] = (unsigned char *) AllocateMemory(MALLOC_ID_ROOM, room->cols);
         if (fread(room->monster_grid[i], room->cols, 1, infile.get()) != 1)
         {
            for (int j=0; j<=i; j++)
               FreeMemory(MALLOC_ID_ROOM, room->monster_grid[j], room->cols);
            FreeMemory(MALLOC_ID_ROOM, room->monster_grid, room->rows * sizeof(char *));
            eprintf("BSPRooFileLoadServer: Failed to read monster grid in %s\n", fname);
            return false;
         }
      }
   }
   
   return true;
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

   for (size_t i = 0; i < room->sectors.size(); i++)
   {
      for (size_t p = 0; p < room->sectors[i].polygons.size(); p++)
      {
         if (room->sectors[i].polygons[p].vertices_x)
            FreeMemory(MALLOC_ID_ROOM, room->sectors[i].polygons[p].vertices_x, 
                     room->sectors[i].polygons[p].num_vertices * sizeof(int));
         if (room->sectors[i].polygons[p].vertices_y)
            FreeMemory(MALLOC_ID_ROOM, room->sectors[i].polygons[p].vertices_y, 
                     room->sectors[i].polygons[p].num_vertices * sizeof(int));
      }
   }

   room->grid = NULL;
   room->flags = NULL;
   room->monster_grid = NULL;
   room->rows = 0;
   room->cols = 0;
}
