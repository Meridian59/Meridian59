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

 This module supports hash tables in kod. It keeps a dynamically sized
 array of tables, which is GC'ed with the other kod data structures.
 Hash nodes are allocated as needed, and key/data vals are saved/loaded.
 Tables will resize when they become 50% full to avoid collisions.
 
 */

#include "blakserv.h"

table_node *tables;
int num_tables, max_num_tables;
static char buf0[LEN_MAX_CLIENT_MSG+1];

/* local function prototypes */

int AllocateTable(void);
hash_node * AllocateTableEntry(val_type key_val,val_type data_val);
void ResizeTable(int table_id);
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
   int hash_size = 0;

   if (num_tables == max_num_tables)
   {
      old_nodes = max_num_tables;
      max_num_tables = max_num_tables + (INIT_TABLE_NODES / 2);

      // Now have to get size of all table contents.
      for (int i = 0; i < num_tables; ++i)
         hash_size += (tables[i].size * sizeof(hash_node *));

      tables = (table_node *)ResizeMemory(MALLOC_ID_TABLE, tables,
         (old_nodes * sizeof(table_node)) + hash_size,
         (max_num_tables * sizeof(table_node)) + hash_size);
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
   if (size < MIN_TABLE_SIZE || size > MAX_TABLE_SIZE)
   {
      bprintf("CreateTable got out of bounds table size %i, resizing to %i\n",
         size, DEFAULT_TABLE_SIZE);
      size = DEFAULT_TABLE_SIZE;
   }
   tn->size = size;
   tn->num_entries = 0;
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
      tn->table[i] = NULL;
   }

   FreeMemory(MALLOC_ID_TABLE,tn->table,tn->size*sizeof(hash_node *));
   tn->size = 0;
   tn->num_entries = 0;
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

void ResizeTable(int table_id)
{
   int new_size, index;
   table_node *tn;
   hash_node *hn, *hn_t, **hn_new, *temp;

   tn = GetTableByID(table_id);
   if (tn == NULL)
      return;

   // Double table size + 1.
   new_size = tn->size * 2 + 1;
   if (new_size > MAX_TABLE_SIZE)
      new_size = MAX_TABLE_SIZE;
   // Allocate memory for the new hashes.
   hn_new = (hash_node **)AllocateMemoryCalloc(MALLOC_ID_TABLE, new_size,
      sizeof(hash_node *));

   // Look at each hash in the table and allocate it in the new one.
   // We also free memory as we go, to avoid doing the same work in
   // DeleteTable.
   for (int i = 0; i < tn->size; ++i)
   {
      hn = tn->table[i];
      while (hn != NULL)
      {
         // Save pointer to next hash node, if present.
         temp = hn->next;
         // Get the new hash.
         index = GetTableHash(hn->key_val) % new_size;
         // Allocate new hash node.
         hn_t = AllocateTableEntry(hn->key_val, hn->data_val);
         // Pointers to next node in hash.
         hn_t->next = hn_new[index];
         hn_new[index] = hn_t;
         // Free old hash.
         FreeMemory(MALLOC_ID_TABLE,hn,sizeof(hash_node));
         // Replace pointer to next, if present.
         hn = temp;
      }
   }

   // Free the old hash node pointer memory, since we're replacing it.
   FreeMemory(MALLOC_ID_TABLE,tn->table,tn->size*sizeof(hash_node *));
   tn->size = new_size;
   tn->table = hn_new;
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

   if (tn->size < 1)
   {
      bprintf("InsertTable can't insert into zero sized table %i\n", table_id);
      return;
   }

   // Dynamically resize table if it starts getting too many entries
   // to avoid collisions.
   if (tn->size < MAX_TABLE_SIZE && tn->num_entries * 2 > tn->size)
      ResizeTable(table_id);

   tn->num_entries++;
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
      tn->num_entries--;
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
         tn->num_entries--;
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

// These functions are for garbage collecting.

void MoveTable(int dest_id,int source_id)
{
   table_node *source,*dest;

   source = GetTableByID(source_id);
   if (source == NULL)
   {
      eprintf("MoveTable can't find source %i, total death end game\n",
         source_id);
      FlushDefaultChannels();
      return;
   }

   dest = GetTableByID(dest_id);
   if (dest == NULL)
   {
      eprintf("MoveTable can't find dest %i, total death end game\n",
         dest_id);
      FlushDefaultChannels();
      return;
   }

   // Don't change the dest id here--it is set to array index, correctly.
   dest->size = source->size;
   dest->num_entries = source->num_entries;
   dest->garbage_ref = source->garbage_ref;
   dest->table = source->table;
}

void SetNumTables(int new_num_tables)
{
   num_tables = new_num_tables;
}
