// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * bitmap.c:  Deal with loading bitmaps
 */

#include "client.h"

static object_bitmap_type bkgnd = NULL; /* Background bitmap */

static char *BITMAP_DIR       = "resource";
static char *BITMAP_PREFIX    = "grd";   /* Prefix of grid bitmap filenames */
static char *BITMAP_EXTENSION = "bgf";   /* Extension of grid bitmap files */

static grid_bitmap_type   LoadGridBitmap(ID gridnum);
static object_bitmap_type LoadObjectBitmap(ID rscnum);

static grid_bitmap_type GetDefaultGridBitmap(void);
static object_bitmap_type GetDefaultObjectBitmap(void);
static void GetDefaultBitmap(Bitmaps *b);
static void FreeBackgroundBitmap(void);
/************************************************************************/
/*
 * GetGridBitmap:  Get the given grid type's bitmap, loading
 *   it from disk if necessary.
 */
grid_bitmap_type GetGridBitmap(WORD type)
{
   grid_bitmap_type grid;
   
   /* See if bitmap has been loaded already */
   grid = FindGridBitmap(type);

   if (grid != NULL)
      return grid;

   /* Try to load bitmap from disk */
   debug(("Loading grid bitmap %d\n", type));
   grid = LoadGridBitmap(type);
   if (grid == NULL)
   {
      debug(("Couldn't load bitmap for grid type %d\n", type));
      grid = GetDefaultGridBitmap();
      grid->idnum = type;
   }
   
   CacheAddGrid(grid);

   return grid;   
}
/************************************************************************/
/*
 * GetObjectBitmap:  Get the given object's bitmap, loading
 *   it from disk if necessary.
 */
object_bitmap_type GetObjectBitmap(ID obj_id)
{
   object_bitmap_type obj;
   
   /* See if bitmap has been loaded already */
   obj = FindObjectBitmap(obj_id);

   if (obj != NULL)
      return obj;

   /* Try to load bitmap from disk */
   debug(("Loading object bitmap %ld\n", obj_id));
   obj = LoadObjectBitmap(obj_id);

   if (obj == NULL)
   {
      debug(("Couldn't load bitmap for object %d\n", obj_id));
      obj = GetDefaultObjectBitmap();
      
      obj->idnum = obj_id;
   }

   /* Add bitmap to list */
   CacheAddObject(obj);

   return obj;   
}
/******************************************************************************/
/*
 * GetBackgroundBitmap:  Return object_bitmap_type pointer if bitmap for given resource
 *    # has already been loaded; NULL if not.
 */
object_bitmap_type GetBackgroundBitmap(ID rsc)
{
   if (rsc == 0)
      return NULL;

   if (bkgnd != NULL)
      if (bkgnd->idnum == rsc) 
	 return bkgnd;
      else FreeBackgroundBitmap();  /* Free previous background */

   debug(("Loading background bitmap %ld\n", rsc));
   bkgnd = LoadObjectBitmap(rsc);
   if (bkgnd == NULL)
   {
      debug(("Couldn't load background bitmap %ld\n", rsc));
      return NULL;
   }

   return bkgnd;
}
/******************************************************************************/
/*
 * LoadGridBitmap: Load the bitmap for the grid background with the given #.
 *   Return a pointer to the bitmap data, or NULL if unsuccessful.
 */
grid_bitmap_type LoadGridBitmap(ID gridnum)
{
   grid_bitmap_type grid;
   char fname[MAX_PATH + FILENAME_MAX], game_path[MAX_PATH];

   GetGamePath( game_path );
   sprintf(fname, "%s%s\\%s%05ld.%s", game_path, BITMAP_DIR, BITMAP_PREFIX, gridnum, BITMAP_EXTENSION);
   
   grid = (grid_bitmap_type) SafeMalloc(sizeof(grid_bitmap_struct));

   if (DibOpenFile(fname, &grid->bmaps) == FALSE)
   {
      SafeFree(grid);
      return NULL;
   }

   grid->idnum = gridnum;

   return grid;
}
/******************************************************************************/
/*
 * LoadObjectBitmap: Load the bitmap for the background with the given
 * resource #.  Return a pointer to the bitmap's data, or NULL if
 * unsuccessful. 
 */
object_bitmap_type LoadObjectBitmap(ID rscnum)
{
   char fname[MAX_PATH + FILENAME_MAX], game_path[MAX_PATH], *basename;
   object_bitmap_type obj;

   /* First look up resource # in the table */
   if ((basename = LookupRsc(rscnum)) == NULL)
   {
      debug(("Couldn't find bitmap filename for resource #%d\n", rscnum));
      return NULL;
   }
   GetGamePath( game_path );
   sprintf(fname, "%s%s\\%.*s", game_path, BITMAP_DIR, FILENAME_MAX, basename);
   
   obj = (object_bitmap_type) SafeMalloc(sizeof(object_bitmap_struct));

   if (DibOpenFile(fname, &obj->bmaps) == FALSE)
   {
      SafeFree(obj);
      return NULL;
   }
   
   obj->idnum = rscnum;

   return obj;
}
/******************************************************************************/
/*
 * GetDefaultGridBitmap:  Return a default grid bitmap, for use when the 
 *   actual bitmap can't be loaded from disk.
 */
grid_bitmap_type GetDefaultGridBitmap(void)
{
   grid_bitmap_type grid = (grid_bitmap_type) SafeMalloc(sizeof(grid_bitmap_struct));
   GetDefaultBitmap(&grid->bmaps);
   return grid;
}
/******************************************************************************/
/*
 * GetDefaultObjectBitmap:  Return a default object bitmap, for use when the 
 *   actual bitmap can't be loaded from disk.
 */
object_bitmap_type GetDefaultObjectBitmap(void)
{
   object_bitmap_type obj = (object_bitmap_type) SafeMalloc(sizeof(object_bitmap_struct));
   GetDefaultBitmap(&obj->bmaps);
   return obj;
}
/******************************************************************************/
/*
 * GetDefaultBitmap:  Set given bitmap structure to contain a single 64x64 
 *   solid black bitmap.
 */
void GetDefaultBitmap(Bitmaps *b)
{
   int size, len;

   b->num_groups  = 1;
   b->num_bitmaps = 1;
   b->max_indices = 1;

   size = sizeof(PDIB) + 2 * sizeof(int);
   b->pdibs = (PDIB *) SafeMalloc(size);
   b->indices = (int *) ((BYTE *) b->pdibs + sizeof(PDIB));

   len = BITMAP_WIDTH * BITMAP_HEIGHT + sizeof(DIBHEADER);

   b->pdibs[0] = (PDIB) SafeMalloc(len);
   memset(b->pdibs[0], 0, len);
   b->pdibs[0]->width  = BITMAP_WIDTH;
   b->pdibs[0]->height = BITMAP_HEIGHT;
   b->pdibs[0]->shrink = 1;

   b->indices[0] = 1;
   b->indices[1] = 0;
   
   b->size = len + size;
}
/************************************************************************/
/*
 * FreeBackgroundBitmap
 */
void FreeBackgroundBitmap(void)
{
   if (bkgnd != NULL)
   {
      BitmapsFree(&bkgnd->bmaps);
      SafeFree(bkgnd);
   }
   bkgnd = NULL;
}
/************************************************************************/
/* 
 * FreeBitmaps:  Free memory for all loaded bitmaps.
 */
void FreeBitmaps(void)
{
   FreeBackgroundBitmap();
   CacheClearAll();
}
