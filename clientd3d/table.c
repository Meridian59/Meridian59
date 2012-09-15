// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* 
 * table.c:  A generic hash table.
 */

#include "client.h"

/*
 * hasher should return an integer i such that 0 <= i < TABLESIZE.
 * compare should return nonzero if two data elements are equal.
 *   The first argument to compare is an item from the table; the
 *   second is the item passed to table_insert or table_lookup.
 */

/*
	Charlie: This should be returning a pointer to the created table,
			not allocating a table, and then returning a copy of the created
			table, and then not freeing the original ...
*/

/***********************************************************************/ 
Table *table_create(DWORD size)
{
   DWORD i;
   Table *t;

	t = (Table  *)SafeMalloc( sizeof ( Table ) );

   t->size = size;
   t->entries = (Entry *) SafeMalloc((UINT) size * sizeof(Entry));

   for (i=0; i < size; i++)
      t->entries[i] = NULL;

   return t;
}
/***********************************************************************/ 
/*
 * table_insert: Insert the given data into the symbol table.
 *               If the data is already there (as determined by compare),
 *               then data not inserted and nonzero returned.
 *               Otherwise data inserted and zero returned.
 */
int table_insert(Table *t, void *data, HashProc hasher, CompareProc compare)
{
   int hashval = (*hasher)(data, t->size);
   Entry p = t->entries[hashval];
   
   /* Look for duplicate entry */
   while (p != NULL)
   {
      if ((*compare)(p->data, data))
	 return 1;
      
      p = p->next;
   }

   t->entries[hashval] = list_add_item(t->entries[hashval], data);
   return 0;
}

/***********************************************************************/ 
/*
 * table_lookup: Return entry matching given data, or NULL if not in table.
 */
void *table_lookup(Table *t, void *data, HashProc hasher, CompareProc compare)
{
   DWORD hashval = (*hasher)(data, t->size);
   Entry p = t->entries[hashval];

   return list_find_item(p, data, compare);
}
/***********************************************************************/ 
/*
 * table_destroy: Totally destroy a table, releasing all memory 
 *    associated with it.
 *    The callback procedure is called for each entry in the table,
 *    so that memory associated with the entry can be freed.
 */
void table_destroy(Table *t, TableDestroyProc destructor)
{
   DWORD i;
   list_type l;

   for (i=0; i < t->size; i++)
   {
      for (l = t->entries[i]; l != NULL; l = l->next)
	 (*destructor)(l->data);
      t->entries[i] = list_delete(t->entries[i]);
   }

   SafeFree(t->entries);
	SafeFree( t );

}
/***********************************************************************/ 
/*
 * table_delete: Free memory associated with a table's items, and restore the
 *               table to a clean state.
 */
void table_delete(Table *t)
{
   DWORD i;

   for (i=0; i < t->size; i++)
      t->entries[i] = list_delete(t->entries[i]);
}
/***********************************************************************/ 
/*
 * table_delete_item: Delete the given item from the given table.
 *    Does nothing if item not in table.
 */
void table_delete_item(Table *t, void *data, HashProc hasher, CompareProc compare)
{
   DWORD hashval = (*hasher)(data, t->size);

   t->entries[hashval] = list_delete_item(t->entries[hashval], data, compare);
}
/***********************************************************************/ 
/*
 * table_get_all:  Return a list of all elements in the hash table, in
 *    no particular order.
 */
list_type table_get_all(Table *t)
{
   DWORD i;
   list_type p, l = NULL;

   for (i=0; i < t->size; i++)
      for (p = t->entries[i]; p != NULL; p = p->next)
	 l = list_add_item(l, p->data);
   return l;
}
