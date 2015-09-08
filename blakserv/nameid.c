// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * nameid.c
 *

 This module maintains tables of name/number pairs for the
 message names and parameter names from the kodbase.  

 */

#include "blakserv.h"

static nameid_node **nameids;
static int nameids_size;
static int num_nameids;

static sihash_type nameid_name_map;

void InitNameID()
{
   // Table for fast nameid ID lookups.
   nameids_size = INIT_NAMEID_NODES;
   // calloc, so memory is zeroed.
   nameids = (nameid_node **)AllocateMemoryCalloc(MALLOC_ID_NAMEID, nameids_size, sizeof(nameid_node*));
   num_nameids = 0;

   // Table for fast nameid name lookups.
   nameid_name_map = CreateSIHash(99971);
}

void ResetNameID()
{
   nameid_node *n, *temp;

   for (int i = 0; i < nameids_size; ++i)
   {
      n = nameids[i];
      while (n != NULL)
      {
         temp = n->next;

         if (n->name)
            FreeMemory(MALLOC_ID_KODBASE, n->name, strlen(n->name) + 1);
         FreeMemory(MALLOC_ID_NAMEID, n, sizeof(nameid_node));

         n = temp;
      }
      nameids[i] = NULL;
   }

   FreeMemory(MALLOC_ID_NAMEID, nameids, nameids_size * sizeof(nameid_node*));
   FreeSIHash(nameid_name_map);
}

void CreateNameID(char *name, int id)
{
   int hash_num;
   bool new_nameid = false;
   nameid_node *n;

   // Throw error if we haven't allocated anything for nameids.
   if (nameids == NULL)
   {
      eprintf("No memory allocated for name IDs, system death.\n");
      return;
   }

   n = GetNameIDNode(id);
   if (n == NULL)
   {
      n = (nameid_node *)AllocateMemory(MALLOC_ID_NAMEID, sizeof(nameid_node));
      n->id = id;
      n->name = NULL;
      new_nameid = true;
   }
   else
   {
      if (!stricmp(name, n->name))
      {
         // Names match, check ID.
         if (n->id == id)
         {
            // Don't add duplicate entries.
            return;
         }
      }
      else
      {
         // Names didn't match, check ID.
         if (n->id == id)
         {
            eprintf("CreateNameID got different string names for name ID %i!\n", id);
            return;
         }
         // If ID different we have a table collision, which we should record.
         eprintf("CreateNameID got table collision for id %i, consider resizing.\n", id);
      }
   }

   n->name = (char *)AllocateMemory(MALLOC_ID_KODBASE, strlen(name) + 1);
   strcpy(n->name, name);

   // Add to nameid tables.
   hash_num = n->id % nameids_size;
   if (new_nameid)
      n->next = nameids[hash_num];
   nameids[hash_num] = n;
   SIHashInsert(nameid_name_map, name, id);
   ++num_nameids;
}

nameid_node * GetNameIDNode(int id)
{
   nameid_node *n;

   n = nameids[id % nameids_size];
   while (n != NULL)
   {
      if (n->id == id)
         return n;
      n = n->next;
   }

   return NULL;
}

int GetIDByName(const char *name)
{
   int name_id;
   Bool found;

   found = SIHashFind(nameid_name_map, name, &name_id);
   if (found)
      return name_id;

   return INVALID_ID;
}

char * GetNameByID(int id)
{
   nameid_node *n;

   n = nameids[id % nameids_size];
   while (n != NULL)
   {
      if (n->id == id)
         return n->name;
      n = n->next;
   }

   return "Unknown";
}

int GetMaxNameID()
{
   return num_nameids;
}

void ForEachNameID(void(*callback_func)(nameid_node *n))
{
   nameid_node *n;

   for (int i = 0; i < nameids_size; ++i)
   {
      n = nameids[i];
      while (n != NULL)
      {
         callback_func(n);
         n = n->next;
      }
   }
}
