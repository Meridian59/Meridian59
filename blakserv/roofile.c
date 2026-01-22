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

#define CHECK_READ_INT(fd, out, fname, desc) \
   do { \
      if (!readInt(fd, out)) { \
         eprintf("Failed to read int %s in %s\n", desc, fname); \
         return false; \
      } \
   } while (0)

#define CHECK_READ_SHORT(fd, out, fname, desc) \
   do { \
      if (!readShort(fd, out)) { \
         eprintf("Failed to read short %s in %s\n", desc, fname); \
         return false; \
      } \
   } while (0)

#define CHECK_READ_BYTE(fd, out, fname, desc) \
   do { \
      if (!readByte(fd, out)) { \
         eprintf("Failed to read byte %s in %s\n", desc, fname); \
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

static bool readInt(FILE *fd, int *out) /* Reads 4-byte little-endian signed int */
{
   return fread(out, 4, 1, fd) == 1;
}

static bool readShort(FILE *fd, short *out) /* Reads 2-byte little-endian signed short */
{
   return fread(out, 2, 1, fd) == 1;
}

static bool readByte(FILE *fd, unsigned char *out) /* Reads 1-byte unsigned */ 
{ 
   return fread(out, 1, 1, fd) == 1; 
}

static bool readCoord(FILE *fd, int *out, bool as_float)
{
   char buf[4];
   if (fread(buf, 4, 1, fd) != 1)
      return false;

   if (as_float)
   {
      // version ≥13 – bytes contain a little-endian float
      *out = (int)(*((float *)buf));
   }
   else
   {
      // version ≤12 – bytes contain a plain little-endian int
      *out = *((int *)buf);
   }

   return true;
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
      CHECK_READ_BYTE(fd, &type, fname, "node type");
      CHECK_SEEK(fseek(fd, 16, SEEK_CUR), fname, "bbox");

      if (type == 1)
      {
         CHECK_SEEK(fseek(fd, 12 + 2 + 2 + 2, SEEK_CUR), fname, "type 1 node data");
      }
      else if (type == 2)
      {
         short sector_plus1;
         short num_pts;

         CHECK_READ_SHORT(fd, &sector_plus1, fname, "sector_plus1");
         CHECK_READ_SHORT(fd, &num_pts, fname, "num_pts");

         if (sector_plus1 < 0 || num_pts < 0) 
         {
            eprintf("LoadSectorPolygons: Failed to read sector_plus1 or num_pts in %s (sector_plus1=%d, num_pts=%d)\n", 
                    fname, sector_plus1, num_pts);
            return false;
         }

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
               int x, y;
               if (!readCoord(fd, &x, coords_are_floats) || !readCoord(fd, &y, coords_are_floats))
               {
                  FreeMemory(MALLOC_ID_ROOM, poly.vertices_x, num_pts * sizeof(int));
                  FreeMemory(MALLOC_ID_ROOM, poly.vertices_y, num_pts * sizeof(int));
                  eprintf("LoadSectorPolygons: Failed to read polygon coordinates in %s\n", fname);
                  return false;
               }
               poly.vertices_x[i] = x;
               poly.vertices_y[i] = y;
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

   FILE *f = fopen(fname, "rb");

   if (!f)
   {
      eprintf("BSPRooFileLoadServer: Failed to open file %s\n", fname);
      return false;
   }

   FileGuard infile(f);

   //
   // Header check
   //

   // Check magic number and version
   for (i = 0; i < 4; i++)
   {
      CHECK_READ_BYTE(infile.get(), &byte, fname, "magic number");
      if (byte != room_magic[i])
      {
         eprintf("BSPRooFileLoadServer: Invalid magic number %d at position %d in %s\n", byte, i, fname);
         return false;
      }
   }

   CHECK_READ_INT(infile.get(), &roo_version, fname, "roo_version");
   if (roo_version < ROO_VERSION)
   {
      eprintf("BSPRooFileLoadServer: Invalid roo_version %d (minimum %d) in %s\n", roo_version, ROO_VERSION, fname);
      return false;
   }

   // Read room security
   CHECK_FREAD(fread(&room->security, 4, 1, infile.get()), fname, "room->security");

   // Read absolute offsets to main (client) and server sections
   int main_off, server_off;
   CHECK_FREAD(fread(&main_off, 4, 1, infile.get()), fname, "main_off");
   CHECK_FREAD(fread(&server_off, 4, 1, infile.get()), fname, "server_off");

   //
   // Client section
   //

   // Seek to Sector metadata and BSP polygons
   CHECK_SEEK(fseek(infile.get(), main_off, SEEK_SET), fname, "main_off");

   int dummy;
   CHECK_READ_INT(infile.get(), &dummy, fname, "room_w");  // room width (ignore)
   CHECK_READ_INT(infile.get(), &dummy, fname, "room_h");  // room height (ignore)

   // Read in subsection offsets
   int node_off;
   CHECK_READ_INT(infile.get(), &node_off, fname, "node_off");

   CHECK_READ_INT(infile.get(), &dummy, fname, "cwall_off");    // client wall offset (ignore)
   CHECK_READ_INT(infile.get(), &dummy, fname, "rwall_off");    // roomedit wall offset (ignore)
   CHECK_READ_INT(infile.get(), &dummy, fname, "sidedef_off");  // sidedef offset (ignore)

   // Read in sector offset
   int sector_off;
   CHECK_READ_INT(infile.get(), &sector_off, fname, "sector_off");

   CHECK_READ_INT(infile.get(), &dummy, fname, "extra_off");  // extra offset (ignore)

   //
   // Sector metadata
   //

   CHECK_SEEK(fseek(infile.get(), sector_off, SEEK_SET), fname, "sector_off");

   short num_sectors;
   CHECK_READ_SHORT(infile.get(), &num_sectors, fname, "num_sectors");

   if (num_sectors < 0)
   {
      eprintf("BSPRooFileLoadServer: Invalid num_sectors %d (must be non-negative) in %s\n", num_sectors, fname);
      return false;
   }

   room->sectors.clear();

   if (num_sectors > 0)
   {
      room->sectors.reserve(num_sectors);

      for (i = 0; i < num_sectors; i++)
      {
         server_sector ss;
         short id_short;

         CHECK_READ_SHORT(infile.get(), &id_short, fname, "sector id");
         ss.id = id_short;

         short dummy_short;
         CHECK_READ_SHORT(infile.get(), &dummy_short, fname, "floor_type");    // floor_type (ignore)
         CHECK_READ_SHORT(infile.get(), &dummy_short, fname, "ceiling_type");  // ceiling_type (ignore)
         CHECK_READ_SHORT(infile.get(), &dummy_short, fname, "xoffset");       // xoffset (ignore)
         CHECK_READ_SHORT(infile.get(), &dummy_short, fname, "yoffset");       // yoffset (ignore)
         CHECK_READ_SHORT(infile.get(), &dummy_short, fname, "floorh");        // floorh (ignore)
         CHECK_READ_SHORT(infile.get(), &dummy_short, fname, "ceilh");         // ceilh (ignore)

         unsigned char dummy_byte;
         CHECK_READ_BYTE(infile.get(), &dummy_byte, fname, "light");  // light (ignore)

         int flags;
         CHECK_READ_INT(infile.get(), &flags, fname, "blak_flags");

         CHECK_READ_BYTE(infile.get(), &dummy_byte, fname, "animate_speed");  // animate_speed (ignore)

         if (flags & SF_SLOPED_FLOOR)
            CHECK_SEEK(fseek(infile.get(), 46, SEEK_CUR), fname, "floor-slope record");

         if (flags & SF_SLOPED_CEILING)
            CHECK_SEEK(fseek(infile.get(), 46, SEEK_CUR), fname, "ceiling-slope record");

         room->sectors.push_back(ss);
      }
   }

   // Sector polygons
   CHECK_SEEK(fseek(infile.get(), node_off, SEEK_SET), fname, "node_off");

   short num_nodes_short;
   CHECK_READ_SHORT(infile.get(), &num_nodes_short, fname, "num_nodes");
   int num_nodes = num_nodes_short;

   bool coords_are_floats = (roo_version >= 13);
   if (!LoadSectorPolygons(infile.get(), node_off + 2, num_nodes, room, fname, coords_are_floats))
   {
      eprintf("BSPRooFileLoadServer: Failed to load sector polygons in %s\n", fname);
      return false;
   }

   // Server section
   // Rows / cols / grids
   CHECK_SEEK(fseek(infile.get(), server_off, SEEK_SET), fname, "server_off");

   CHECK_READ_INT(infile.get(), &temp, fname, "temp (1)");

   room->rows = (short) temp;

   CHECK_READ_INT(infile.get(), &temp, fname, "temp (2)");

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
            for (int j = 0; j <= i; j++)
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
