// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
 /*
  * cache.c:  Implements bitmap cache.
  *
  * The bitmap caches are LRU caches of a fixed maximum size.  The maximum size is determined
  * upon initialization based on the amount of free memory.
  *
  * XXX BSP tree contains pointers directly to grid bitmaps, so can't evict them while in room
  */

 #include "client.h"

 typedef object_bitmap_type cache_bitmap;

 typedef struct {
   list_type bitmaps;				// List of cache_bitmaps
   unsigned int size;				// Memory taken up by bitmaps in this cache
   unsigned int max_size;			// Max size of cache
   unsigned int highwater_size;		// Max size cache has ever reached
 } Cache;

 // Largest amount of memory to use
 #define GEN_MEMORY (12*1024*1024)
 #define MAX_MEMORY (20*1024*1024)

 // Space to leave in memory for OS, client code, etc. depending on OS
 #define FREEMEM_WINNT   8000000
 #define FREEMEM_WIN95   8000000
 #define FREEMEM_UNKNOWN FREEMEM_WIN95

 #define OBJECT_CACHE_MIN_SIZE  6000000
 #define GRID_CACHE_MIN_SIZE    4000000

 static Cache grid_cache;
 static Cache object_cache;

 static Bool IdCacheBitmapCompare(void *idnum, void *b);
 static void CacheAddBitmap(Cache *cache, cache_bitmap bitmap);
 static cache_bitmap CacheFindBitmap(Cache *cache, ID id);
 static void CacheClear(Cache *cache);
 /************************************************************************/
 /*
 * IdCacheBitmapCompare: Compare a # and a cache bitmap; 
 *   return nonzero if # equals bitmap's id.
 */
 Bool IdCacheBitmapCompare(void *idnum, void *b)
 {  
    return *((ID *) idnum) == ((cache_bitmap) b)->idnum;
 }

 /******************************************************************************/
 /*
 * FindObjectBitmap:  Return object_bitmap_type pointer if bitmap for given resource
 *    # has already been loaded; NULL if not.
 */
 object_bitmap_type FindObjectBitmap(ID rsc)
 {
   return (object_bitmap_type) CacheFindBitmap(&object_cache, rsc);
 }
 /******************************************************************************/
 /*
 * FindGridBitmap:  Return grid_bitmap_type pointer if bitmap for given grid #
 *    has already been loaded; NULL if not.
 */
 grid_bitmap_type FindGridBitmap(WORD grid_num)
 {
   return (grid_bitmap_type) CacheFindBitmap(&grid_cache, (ID) grid_num);
 }

 /******************************************************************************/
 /*
 * CacheInitialize:  Set up object and grid bitmap caches.
 */
void CacheInitialize(void)
{
  MEMORYSTATUSEX statex;
  DWORDLONG mem_available;
  statex.dwLength = sizeof statex;
  GlobalMemoryStatusEx (&statex);
  
  // Set aside up to 1/2 of free memory for caches
  mem_available = statex.ullAvailPhys / 2;
  
  object_cache.max_size = max(mem_available / 2, config.ObjectCacheMin );
  grid_cache.max_size   = max(mem_available / 2, config.GridCacheMin );
  
  debug(("%12d bytes for object cache\n", object_cache.max_size));
  debug(("%12d bytes for grid cache\n", grid_cache.max_size));
}
/******************************************************************************/
/*
* CacheAddObject:  Add new object_bitmap_type structure to list of loaded bitmaps.
*/
void CacheAddObject(object_bitmap_type new_bmap)
{
	CacheAddBitmap(&object_cache, (cache_bitmap) new_bmap);
}
/******************************************************************************/
/*
* CacheAddGrid:  Add new grid_bitmap_type structure to list of loaded bitmaps.
*/
void CacheAddGrid(grid_bitmap_type new_bmap)
{
	CacheAddBitmap(&grid_cache, (cache_bitmap) new_bmap);
}
/******************************************************************************/
/*
* CacheAddBitmap:  Add given bitmap structure to given cache, evicting entries if necessary.
*/
void CacheAddBitmap(Cache *cache, cache_bitmap bitmap)
{
	cache_bitmap last_bmap;
	unsigned int new_size, old_size;
	
	new_size = BitmapsTotalSize(bitmap->bmaps);
	
	while (cache->size + new_size > cache->max_size && cache->bitmaps != NULL)
	{
		last_bmap = (cache_bitmap) list_last_item(cache->bitmaps);
		old_size = BitmapsTotalSize(last_bmap->bmaps);
		BitmapsFree(&last_bmap->bmaps);
		SafeFree(last_bmap);
		
		cache->bitmaps = list_delete_last(cache->bitmaps);
		cache->size   -= old_size;
	}
	
	cache->bitmaps = list_add_first(cache->bitmaps, bitmap);
	cache->size   += new_size;
	
	if (cache->size > cache->highwater_size)
		cache->highwater_size = cache->size;
}
/******************************************************************************/
/*
* CacheFindBitmap:  Find and return bitmap with given id in given cache.
*/
cache_bitmap CacheFindBitmap(Cache *cache, ID id)
{
	cache_bitmap bmap;
	
	cache->bitmaps = list_move_to_front(cache->bitmaps, &id, IdCacheBitmapCompare);
	
	/* Must be in first node or missing */
	bmap = (cache_bitmap) list_first_item(cache->bitmaps);
	if (bmap == NULL || id != bmap->idnum)
		return NULL;
	
	return bmap;
}
/******************************************************************************/
/*
* CacheClear:  Free memory in given bitmap cache.
*/
void CacheClear(Cache *cache)
{
	list_type l;
	
	for (l = cache->bitmaps; l != NULL; l = l->next)
	{
		cache_bitmap bmap = (cache_bitmap) (l->data);
		BitmapsFree(&bmap->bmaps);
	}
	
	cache->bitmaps = list_destroy(cache->bitmaps);
	cache->size = 0;
}
/******************************************************************************/
/*
* CacheClearAll:  Free bitmaps and associated lists.
*/
void CacheClearAll(void)
{
	CacheClear(&object_cache);
	CacheClear(&grid_cache);
	debug(("Graphic caches cleared.\n"));
}
/******************************************************************************/
/*
* CacheReport:  Status check on caches.
*/
void CacheReport(void)
{
	debug(("%12d bytes Object Cache (%12d highwater)\n", object_cache.size, object_cache.highwater_size));
	debug(("%12d bytes Grid Cache   (%12d highwater)\n", grid_cache.size, grid_cache.highwater_size));
}
/******************************************************************************/
/*
* CacheGetGridList:  Return list of all loaded grid bitmaps.
*/
list_type CacheGetGridList(void)
{
	return grid_cache.bitmaps;
}
