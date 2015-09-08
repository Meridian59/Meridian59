// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * table.h
 *
 */

#ifndef _TABLE_H
#define _TABLE_H

#define INIT_TABLE_NODES (18000)
#define MIN_TABLE_SIZE (23)
#define MAX_TABLE_SIZE (19463)
#define DEFAULT_TABLE_SIZE (73)

typedef struct hash_struct
{
   val_type key_val;
   val_type data_val;
   struct hash_struct *next;
} hash_node;

typedef struct table_struct
{
   int size;
   int num_entries;
   hash_node **table;
   int garbage_ref;
} table_node;

void InitTables(void);
int GetTablesUsed(void);
void ResetTables(void);
int CreateTable(int size);
table_node * GetTableByID(int table_id);
void DeleteTable(int table_id);
void InsertTable(int table_id,val_type key_val,val_type data_val);
int GetTableEntry(int table_id,val_type key_val);
void DeleteTableEntry(int table_id,val_type key_val);

unsigned int GetBufferHash(const char *buf,unsigned int len_buf);
void ForEachTable(void (*callback_func)(table_node *t, int table_id));
void MoveTable(int dest_id,int source_id);
void SetNumTables(int new_num_tables);

#endif
