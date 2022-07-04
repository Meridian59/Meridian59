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

 This module maintains a linked list of name/number pairs for the
 message names and parameter names from the kodbase.  

 */

#include "blakserv.h"

nameid_node *nameids;

/* local function prototypes */
nameid_node *AllocateNameIDNode();

void InitNameID()
{
   nameids = NULL;
}

void ResetNameID()
{
   nameid_node *nid,*temp;

   nid = nameids;

   while (nid != NULL)
   {
      temp = nid->next;
      FreeMemory(MALLOC_ID_KODBASE,nid->name,strlen(nid->name)+1);
      FreeMemory(MALLOC_ID_NAMEID,nid,sizeof(nameid_node));
      nid = temp;
   }

   nameids = NULL;
}

nameid_node *AllocateNameIDNode()
{
   nameid_node *nid;

   nid = (nameid_node *)AllocateMemory(MALLOC_ID_NAMEID,sizeof(nameid_node));
   return nid;
}

void CreateNameID(char *name,int id)
{
   nameid_node *nid;

   nid = AllocateNameIDNode();
   nid->name = (char *)AllocateMemory(MALLOC_ID_KODBASE,strlen(name)+1);
   strcpy(nid->name,name);
   nid->id = id;

   nid->next = nameids;
   nameids = nid;
}

int GetIDByName(const char *name)
{
   nameid_node *nid;

   nid = nameids;

   while (nid != NULL)
   {
      if (!stricmp(name,nid->name))
	 return nid->id;
      nid = nid->next;
   }
   return INVALID_ID;
}

const char * GetNameByID(int id)
{
   nameid_node *nid;

   nid = nameids;

   while (nid != NULL)
   {
      if (nid->id == id)
	 return nid->name;
      nid = nid->next;
   }
   return "Unknown";
}

