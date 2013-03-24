// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* table.h
 * Header file for table.c
 */
#ifndef _TABLE_H
#define _TABLE_H

/* Use generic lists to represent hash table entries */
typedef list_type Entry;
typedef list_struct EntryStruct;

typedef struct {
   int size;
   Entry *entries;
} Table;

typedef int (*HashProc)(const void *, int);
typedef int (*CompareProc)(void *, void *);

Table table_create(int size);
int table_insert(Table t, void *data, HashProc hasher, CompareProc compare);
void *table_lookup(Table t, void *data, HashProc hasher, CompareProc compare);
void table_delete_item(Table t, void *data, HashProc hasher, CompareProc compare);
void table_delete(Table t);
list_type table_get_all(Table t);


#endif /* #ifndef _TABLE_H */

