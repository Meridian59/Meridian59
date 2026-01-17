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

#define SF_SLOPED_FLOOR   0x00000400
#define SF_SLOPED_CEILING 0x00000800

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

static unsigned char readByte(FILE *fd)   /* 1-byte unsigned                   */
{
   unsigned char v = 0;
   fread(&v, 1, 1, fd);
   return v;
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
      bprintf("LoadSectorPolygons: Failed to seek to nodes_start in %s\n", fname);
      return false;
   }

   for (int n = 0; n < num_nodes; n++)
   {
      unsigned char type;
      if (fread(&type, 1, 1, fd) != 1)
      {
         bprintf("LoadSectorPolygons: Failed to read node type in %s\n", fname);
         return false;
      }
      
      if (fseek(fd, 16, SEEK_CUR) != 0)
      {
         bprintf("LoadSectorPolygons: Failed to skip bbox in %s\n", fname);
         return false;
      }

      if (type == 1)
      {
         if (fseek(fd, 12 + 2 + 2 + 2, SEEK_CUR) != 0)
         {
            bprintf("LoadSectorPolygons: Failed to skip type 1 node data in %s\n", fname);
            return false;
         }
      }
      else if (type == 2)
      {
         unsigned short sector_plus1 = readShort(fd);
         short num_pts = readShort(fd);

         if (sector_plus1 < 0 || num_pts < 0)
         {
            bprintf("LoadSectorPolygons: Invalid readShort result in %s (sector_plus1=%d, num_pts=%d)\n", 
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
            if (fseek(fd, num_pts * 8, SEEK_CUR) != 0)
            {
               bprintf("LoadSectorPolygons: Failed to skip invalid polygon data in %s\n", fname);
               return false;
            }
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
   FILE *infile;
   int i, temp, roo_version;
   unsigned char byte;

   infile = fopen(fname, "rb");
   if (!infile)
   {
      bprintf("BSPRooFileLoadServer: Failed to open file %s\n", fname);
      return false;
   }

   // Header check
   for (i = 0; i < 4; i++)
      if (fread(&byte, 1, 1, infile) != 1 || byte != room_magic[i])
      {
         fclose(infile);
         return false;
      }

   if (fread(&roo_version, 4, 1, infile) != 1 || roo_version < ROO_VERSION)
   {
      fclose(infile);
      return false;
   }

   /* security (unused in loader) */
   if (fread(&room->security, 4, 1, infile) != 1)
   {
      fclose(infile);
      return false;
   }

   /* absolute offsets to main (client) and server sections */
   int main_off, server_off;
   if (fread(&main_off, 4, 1, infile) != 1)
   {
      fclose(infile);
      return false;
   }
   if (fread(&server_off, 4, 1, infile) != 1)
   {
      fclose(infile);
      return false;
   }

   // Client section
   // Sector metadata and BSP polygons
   fseek(infile, main_off, SEEK_SET);

   // Read in and ignore room width and height (not used on server)
   int room_w = readInt(infile);
   int room_h = readInt(infile);

   // Read in subsection offsets
   int node_off = readInt(infile);
   readInt(infile); /* cwall off   */
   readInt(infile); /* rwall off   */
   readInt(infile); /* sidedef off */

   int sector_off = readInt(infile);
   readInt(infile);

   // Sector metadata
   fseek(infile, sector_off, SEEK_SET);
   int num_sectors = readShort(infile);
   room->sectors.clear();

   if (num_sectors > 0)
   {
      room->sectors.reserve(num_sectors);

      for (i = 0; i < num_sectors; i++)
      {
         server_sector ss;

         ss.id = readShort(infile);
         readShort(infile); // floor_type
         readShort(infile); // ceiling_type
         readShort(infile); // xoffset
         readShort(infile); // yoffset
         readShort(infile); // floorh
         readShort(infile); // ceilh
         readByte(infile);  // light

         int flags = readInt(infile); // blak_flags
         readByte(infile);            // animate_speed

         if (flags & SF_SLOPED_FLOOR)
            fseek(infile, 46, SEEK_CUR); // 46-byte floor-slope record
         if (flags & SF_SLOPED_CEILING)
            fseek(infile, 46, SEEK_CUR); // 46-byte ceiling-slope record

         room->sectors.push_back(ss);
      }
   }

   // Sector polygons
   fseek(infile, node_off, SEEK_SET);
   int num_nodes = readShort(infile);

   bool coords_are_floats = (roo_version >= 13);
   if (!LoadSectorPolygons(infile, node_off + 2, num_nodes, room, fname, coords_are_floats))
   {
      fclose(infile);
      return false;
   }
   
   // Server section
   // Rows / cols / grids (unchanged)
   fseek(infile, server_off, SEEK_SET);

   if (fread(&temp, 4, 1, infile) != 1)
   {
      fclose(infile);
      return false;
   }
   
   room->rows = (short) temp;
   if (fread(&temp, 4, 1, infile) != 1)
   {
      fclose(infile);
      return false;
   }

   room->cols = (short) temp;

   room->grid = (unsigned char **) AllocateMemory(MALLOC_ID_ROOM, room->rows * sizeof(char *));
   room->flags = (unsigned char **) AllocateMemory(MALLOC_ID_ROOM, room->rows * sizeof(char *));

   for (i = 0; i < room->rows; i++)
   {
      room->grid[i] = (unsigned char *) AllocateMemory(MALLOC_ID_ROOM, room->cols);
      if (fread(room->grid[i], room->cols, 1, infile) != 1)
      {
         fclose(infile);
         return false;
      }
   }
   for (i = 0; i < room->rows; i++)
   {
      room->flags[i] = (unsigned char *) AllocateMemory(MALLOC_ID_ROOM, room->cols);
      if (fread(room->flags[i], room->cols, 1, infile) != 1)
      {
         fclose(infile);
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
         if (fread(room->monster_grid[i], room->cols, 1, infile) != 1)
         {
            fclose(infile);
            return false;
         }
      }
   }

   fclose(infile);
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
