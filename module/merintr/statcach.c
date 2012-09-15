// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statcach.c:  Maintain a cache of statistics, so that we don't have to ask the server
 *   for stats every time the user changes stat groups.
 *
 *   Cache entries are referenced by group numbers, which start at 1, although internally
 *   they are kept in an array which starts at 0.
 */

#include "client.h"
#include "merintr.h"

typedef struct {
   Bool      valid;                      // True when entry is valid
   list_type stats;                      // A group of stats
} StatCacheEntry;

typedef struct {
   int             size;                 // Number of cache entries
   StatCacheEntry *entries;
} StatCache;

static StatCache stat_cache;        // Cache stat groups we've retrieved from server already

/* local function prototypes */
static Bool CompareStats(void *s1, void *s2);
/************************************************************************/
void StatCacheCreate(void)
{
   stat_cache.size = 0;   
}
/************************************************************************/
void StatCacheDestroy(void)
{
   int i;

   for (i=0; i < stat_cache.size; i++)
      if (stat_cache.entries[i].valid)
	 list_destroy(stat_cache.entries[i].stats);

   if (stat_cache.entries != NULL)
      SafeFree(stat_cache.entries);

   stat_cache.size    = 0;
   stat_cache.entries = NULL;
}
/************************************************************************/
/*
 * CompareStats: Compare two statistics based on num field.
 */
Bool CompareStats(void *s1, void *s2)
{
   return ((Statistic *) s1)->num == ((Statistic *) s2)->num;
}
/************************************************************************/
/*
 * StatCacheGetEntry:  If cache contains a valid entry for given group,
 *   set stats to it and return True.
 *   Otherwise, return False.
 */
Bool StatCacheGetEntry(int group, list_type *stats)
{
   if (group > stat_cache.size)
      return False;

   group--;    // First group is array index 0
   if (!stat_cache.entries[group].valid)
      return False;

   *stats = stat_cache.entries[group].stats;
   return True;
}
/************************************************************************/
/*
 * StatCacheSetEntry:  Set the given group in the cache to the given list of stats.
 */
void StatCacheSetEntry(int group, list_type stats)
{
   if (group > stat_cache.size)
   {
      debug(("StatCacheSetEntry got entry too large %d; size is %d\n", group, stat_cache.size));
      return;
   }

   group--;    // First group is array index 0

   // Destroy previous stats, if any
   if (stat_cache.entries[group].valid)
      list_destroy(stat_cache.entries[group].stats);
   
   stat_cache.entries[group].stats = stats;
   stat_cache.entries[group].valid = True;
}
/************************************************************************/
/*
 * StatCacheSetSize:  Clear out stat cache and set its size
 */
void StatCacheSetSize(int size)
{
   int i;

   StatCacheDestroy();
   
   stat_cache.size = size;
   stat_cache.entries = (StatCacheEntry *) SafeMalloc(size * sizeof(StatCacheEntry));
   for (i=0; i < size; i++)
   {
      stat_cache.entries[i].valid = False;
      stat_cache.entries[i].stats = NULL;
   }
}
/************************************************************************/
/*
 * StatCacheUpdate:  Given stat in group has changed; update cache and return
 *   resultant Statistic structure, or NULL if stat not found.
 */
Statistic *StatCacheUpdate(BYTE group, Statistic *s)
{
   Statistic *old_stat;
   list_type stats;

   if (group > stat_cache.size || !stat_cache.entries[group-1].valid)
      return NULL;

   stats = stat_cache.entries[group-1].stats;

   /* Find old stat in list */
   old_stat = (Statistic *) list_find_item(stats, s, CompareStats);
   
   if (old_stat == NULL)
   {
      debug(("Tried to change nonexistent stat #%d in group #%d", s->num, (int) group));
      return NULL;
   }

   old_stat->name_res = s->name_res;
   old_stat->type     = s->type;
   switch (s->type)
   {
   case STATS_NUMERIC:
      memcpy(&old_stat->numeric, &s->numeric, sizeof(old_stat->numeric));
      break;

   case STATS_LIST:
      memcpy(&old_stat->list, &s->list, sizeof(old_stat->list));
      break;

   default:
      debug(("StatsCacheUpdate got illegal stat type %d\n", (int) s->type));
      break;
   }
   return old_stat;
}
