// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

#include "blakcomp.h"

/*
 * A generic hash table.
 * hasher should return an integer i such that 0 <= i < TABLESIZE.
 * compare should return nonzero if two data elements are equal.
 *   The first argument to compare is an item from the table; the
 *   second is the item passed to table_insert or table_lookup.
 */

/***********************************************************************/ 
Table table_create(int size)
{
   int i;
   Table t;

   t.size = size;
   t.entries = (Entry *) SafeMalloc(size * sizeof(Entry));

   for (i=0; i < size; i++)
      t.entries[i] = NULL;
   return t;
}
/***********************************************************************/ 
/*
 * table_insert: Insert the given data into the symbol table.
 *               If the data is already there (as determined by compare),
 *               then data not inserted and nonzero returned.
 *               Otherwise data inserted and zero returned.
 */
int table_insert(Table t, void *data, HashProc hasher, CompareProc compare)
{
   int hashval = (*hasher)(data, t.size);
   Entry p = t.entries[hashval];
   
   /* Look for duplicate entry */
   while (p != NULL)
   {
     if ((*compare)(p->data, data))
       return 1;
     
     p = p->next;
   }

   t.entries[hashval] = list_add_item(t.entries[hashval], data);
   return 0;
}

/***********************************************************************/ 
/*
 * table_lookup: Return entry matching given data, or NULL if not in table.
 */
void *table_lookup(Table t, void *data, HashProc hasher, CompareProc compare)
{
   int hashval = (*hasher)(data, t.size);
   Entry p = t.entries[hashval];

   return list_find_item(p, data, compare);
}
/***********************************************************************/ 
/*
 * table_delete: Free memory associated with a table, and restore the
 *               table to a clean state.
 */
void table_delete(Table t)
{
   int i;

   for (i=0; i < t.size; i++)
     t.entries[i] = list_delete(t.entries[i]);
}
/***********************************************************************/ 
/*
 * table_delete_item: Delete the given item from the given table.
 *    Does nothing if item not in table.
 */
void table_delete_item(Table t, void *data, HashProc hasher, CompareProc compare)
{
   int hashval = (*hasher)(data, t.size);

   t.entries[hashval] = list_delete_item(t.entries[hashval], data, compare);
}
/***********************************************************************/ 
/*
 * table_get_all:  Return a list of all elements in the hash table, in
 *    no particular order.
 */
list_type table_get_all(Table t)
{
   int i;
   list_type p, l = NULL;

   for (i=0; i < t.size; i++)
      for (p = t.entries[i]; p != NULL; p = p->next)
	 l = list_add_item(l, p->data);
   return l;
}
