// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * table.c
 *

 This module supports hash tables in kod.

 It is currently quite primitive--doesn't work over garbage collects,
 and is not saved.
 
 */

#include "blakserv.h"

table_node *tables;
int num_tables, max_num_tables;
static char buf0[LEN_MAX_CLIENT_MSG+1];

/* local function prototypes */

int AllocateTable(void);
hash_node * AllocateTableEntry(val_type key_val,val_type data_val);

Bool EqualTableEntry(val_type s1_val,val_type s2_val);
unsigned int GetTableHash(val_type val);


void InitTables()
{
   num_tables = 0;
   max_num_tables = INIT_TABLE_NODES;
   tables = (table_node *)AllocateMemory(MALLOC_ID_TABLE,
                              max_num_tables * sizeof(table_node));
}

int GetTablesUsed(void)
{
   return num_tables;
}

int AllocateTable(void)
{
   int old_nodes;

   if (num_tables == max_num_tables)
   {
      old_nodes = max_num_tables;
      max_num_tables = max_num_tables + (INIT_TABLE_NODES / 2);

      tables = (table_node *)ResizeMemory(MALLOC_ID_TABLE, tables,
         old_nodes * sizeof(table_node), max_num_tables * sizeof(table_node));
      lprintf("AllocateTable resized to %i tables\n",max_num_tables);
   }

   return num_tables++;
}

void ResetTables()
{
   int old_nodes;

   for (int i = 0; i < num_tables; ++i)
      DeleteTable(i);

   num_tables = 0;
   old_nodes = max_num_tables;
   max_num_tables = INIT_TABLE_NODES;
   tables = (table_node *)ResizeMemory(MALLOC_ID_TABLE, tables,
      old_nodes * sizeof(table_node), max_num_tables * sizeof(table_node));
}

int CreateTable(int size)
{
   table_node *tn;
   int table_id;

   table_id = AllocateTable();
   tn = GetTableByID(table_id);
   tn->size = size;
   tn->table = (hash_node **)AllocateMemoryCalloc(MALLOC_ID_TABLE, size,
                                 sizeof(hash_node *));

   return table_id;
}

void DeleteTable(int table_id)
{
   hash_node *hn,*temp;
   table_node *tn;

   tn = GetTableByID(table_id);

   for (int i = 0; i < tn->size; ++i)
   {
      hn = tn->table[i];
      while (hn != NULL)
      {
         temp = hn->next;
         FreeMemory(MALLOC_ID_TABLE,hn,sizeof(hash_node));
         hn = temp;
      }
   }

   FreeMemory(MALLOC_ID_TABLE,tn->table,tn->size*sizeof(hash_node *));
}

table_node * GetTableByID(int table_id)
{
   if (table_id < 0 || table_id >= num_tables)
   {
      eprintf("GetTableByID can't retrieve invalid table %i\n",table_id);

      return NULL;
   }

   return &tables[table_id];
}

hash_node * AllocateTableEntry(val_type key_val,val_type data_val)
{
   hash_node *hn;

   hn = (hash_node *)AllocateMemory(MALLOC_ID_TABLE,sizeof(hash_node));

   hn->key_val = key_val;
   hn->data_val = data_val;

   hn->next = NULL;

   return hn;
}

void InsertTable(int table_id,val_type key_val,val_type data_val)
{
   table_node *tn;
   hash_node *hn;
   unsigned int index;

   tn = GetTableByID(table_id);
   if (tn == NULL)
   {
      bprintf("InsertTable can't find table %i\n",table_id);
      return;
   }

   index = GetTableHash(key_val) % tn->size;

   if (ConfigBool(DEBUG_HASH) == True)
      dprintf("Insert tbl %i, index %i, key %i,%i\n",table_id,index,key_val.v.tag,key_val.v.data);
   
   /* insert in front of list */
   hn = AllocateTableEntry(key_val,data_val);
   hn->next = tn->table[index];
   tn->table[index] = hn;
}

int GetTableEntry(int table_id,val_type key_val)
{
   table_node *tn;
   hash_node *hn;
   unsigned int index;

   tn = GetTableByID(table_id);
   if (tn == NULL)
   {
      bprintf("GetTableEntry can't find table %i\n",table_id);
      return NIL;
   }

   index = GetTableHash(key_val) % tn->size;

   hn = tn->table[index];
   while (hn != NULL)
   {
      if (EqualTableEntry(hn->key_val,key_val))
         return hn->data_val.int_val;
      hn = hn->next;
   }
   return NIL;
}

void DeleteTableEntry(int table_id,val_type key_val)
{
   table_node *tn;
   hash_node *hn,*temp;
   unsigned int index;

   tn = GetTableByID(table_id);
   if (tn == NULL)
   {
      bprintf("DeleteTableEntry can't find table %i\n",table_id);
      return;
   }

   index = GetTableHash(key_val) % tn->size;

   if (tn->table[index] == NULL)
   {
      dprintf("DeleteTableEntry can't delete %i,%i from table %i\n",key_val.v.tag,
         key_val.v.data,table_id);
      return;
   }

   if (EqualTableEntry(tn->table[index]->key_val,key_val))
   {
      hn = tn->table[index]->next;
      FreeMemory(MALLOC_ID_TABLE,tn->table[index],sizeof(hash_node));
      tn->table[index] = hn;
      return;
   }

   hn = tn->table[index];
   
   while (hn->next != NULL)
   {
      if (EqualTableEntry(hn->next->key_val,key_val))
      {
         temp = hn->next;
         hn->next = hn->next->next;
         FreeMemory(MALLOC_ID_TABLE,temp,sizeof(hash_node));
         return;
      }
      hn = hn->next;
   }
   dprintf("DeleteTableEntry can't delete %i,%i from table %i\n",key_val.v.tag,
      key_val.v.data,table_id);
}

Bool EqualTableEntry(val_type s1_val,val_type s2_val)
{
   char *s1,*s2;
   int len1,len2;
   resource_node *r;
   string_node *snod;

   s1 = NULL;
   switch (s1_val.v.tag)
   {
      case TAG_RESOURCE :
         r = GetResourceByID(s1_val.v.data);
         if (r == NULL)
         {
            bprintf("EqualTableEntry can't find resource %i,%i\n",
            s1_val.v.tag,s1_val.v.data);
            eprintf("%s\n",BlakodStackInfo());
            return False;
         }
         s1 = r->resource_val[0];
         len1 = strlen(r->resource_val[0]);
         break;

      case TAG_STRING :
         snod = GetStringByID(s1_val.v.data);
         if (snod == NULL)
         {
            bprintf("EqualTableEntry can't find string %i,%i\n",
            s1_val.v.tag,s1_val.v.data);
            eprintf("%s\n",BlakodStackInfo());
            return False;
         }
         s1 = snod->data;
         len1 = snod->len_data;
         break;

      case TAG_TEMP_STRING :
         snod = GetTempString();
         s1 = snod->data;
         len1 = snod->len_data;
         break;
   }

   s2 = NULL;
   switch (s2_val.v.tag)
   {
      case TAG_RESOURCE :
         r = GetResourceByID(s2_val.v.data);
         if (r == NULL)
         {
            bprintf("EqualTableEntry can't find resource %i,%i\n",
            s2_val.v.tag,s2_val.v.data);
            eprintf("%s\n",BlakodStackInfo());
            return False;
         }
         s2 = r->resource_val[0];
         len2 = strlen(r->resource_val[0]);
         break;

      case TAG_STRING :
         snod = GetStringByID(s2_val.v.data);
         if (snod == NULL)
         {
            bprintf("EqualTableEntry can't find string %i,%i\n",
            s2_val.v.tag,s2_val.v.data);
            eprintf("%s\n",BlakodStackInfo());
            return False;
         }
         s2 = snod->data;
         len2 = snod->len_data;
         break;

      case TAG_TEMP_STRING :
         snod = GetTempString();
         s2 = snod->data;
         len2 = snod->len_data;
         break;
   }

   /* neither strings */
   if (s1 == NULL && s2 == NULL)
      return s1_val.int_val == s2_val.int_val;

   /* mismatch */
   if (s1 == NULL || s2 == NULL)
   {
      eprintf("EqualTableEntry can't match %i,%i with %i,%i\n",
         s1_val.v.tag,s1_val.v.data,s2_val.v.tag,s2_val.v.data);
      eprintf("%s\n",BlakodStackInfo());
      return False;
   }

   return FuzzyBufferEqual(s1,len1,s2,len2);
}

unsigned int GetTableHash(val_type val)
{
   resource_node *r;
   string_node *snod;
   char* s = NULL;
   int len = 0;

   switch (val.v.tag)
   {
      case TAG_RESOURCE :
         r = GetResourceByID(val.v.data);
         if (r == NULL)
         {
            bprintf("GetTableHash can't find resource %i\n",val.v.data);
            eprintf("%s\n",BlakodStackInfo());
            return 0;
         }
         s = r->resource_val[0];
         len = strlen(r->resource_val[0]);
         break;

      case TAG_STRING :
         snod = GetStringByID(val.v.data);
         if (snod == NULL)
         {
            bprintf("GetTableHash can't find string %i\n",val.v.data);
            eprintf("%s\n",BlakodStackInfo());
            return 0;
         }
         s = snod->data;
         len = snod->len_data;
         break;

      case TAG_TEMP_STRING :
         snod = GetTempString();
         if (snod == NULL)
         {
            bprintf("GetTableHash can't find temp string\n");
            return 0;
         }
         s = snod->data;
         len = snod->len_data;
         break;

      default:
         return GetBufferHash((char *)&val.int_val,4);
   }

   if (!s || len <= 0)
      return 0;

   FuzzyCollapseString(buf0,s,len);

   return GetBufferHash(buf0,strlen(buf0));
}

unsigned int GetBufferHash(const char *buf,unsigned int len_buf)
{
   unsigned int g,h,i;

   /* stolen from ELF file format */

   h = 0;
   for (i=0;i<len_buf;i++)
   {
      h = (h << 4) + (unsigned char)(toupper(buf[i]));
      if (g = h & 0xF0000000)
         h ^= g >> 24;
      h &= ~g;
   }

   return h;
}

void ForEachTable(void (*callback_func)(table_node *t,int table_id))
{
   int i;

   for (i=0;i<num_tables;i++)
      callback_func(&tables[i],i);
}
