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

typedef struct hash_struct
{
   val_type key_val;
   val_type data_val;
   struct hash_struct *next;
} hash_node;

typedef struct table_struct
{
   int table_id;
   int size;
   hash_node **table;
   struct table_struct *next;
} table_node;

void InitTable(void);
void ResetTable(void);
int CreateTable(int size);
table_node * GetTableByID(int table_id);
void DeleteTable(int table_id);
void InsertTable(int table_id,val_type key_val,val_type data_val);
int GetTableEntry(int table_id,val_type key_val);
void DeleteTableEntry(int table_id,val_type key_val);

unsigned int GetBufferHash(const char *buf,unsigned int len_buf);

#endif

