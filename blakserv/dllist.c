// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * dllist.c
 *

 This module keeps a linked list of files that have to be downloaded
 by the client.  It is stored in oldest file first order.
 The files to download are all added from the upload directory.
 referenced by any resources but must be sent to the client.  Because
 of the built in guys (all but dynarsc now), remember when resetting/initting this list to
 call AddBuiltInDLlist().

 */

#include "blakserv.h"

dllist_node *dllist;

/* local function prototypes */
void AddDLlist(char *filename,int time,int file_type,int file_size);
void AddOrUpdateDLlist(char *filename,int time,int file_type,int file_size);

void InitDLlist()
{
   dllist = NULL;
}

void ResetDLlist()
{
   dllist_node *dl,*temp;

   dl = dllist;
   while (dl != NULL)
   {
      temp = dl->next;
      FreeMemory(MALLOC_ID_DLLIST,dl,sizeof(dllist_node));
      dl = temp;
   }
   dllist = NULL;
}

void AddDLlist(char *filename,int time,int file_type, int file_size)
{
   dllist_node *temp,*prev,*new_node;

   if (time < 0)
      eprintf("AddDLlist got bad time %i for file %s\n",time,filename);

   new_node = (dllist_node *)AllocateMemory(MALLOC_ID_DLLIST,sizeof(dllist_node));
   strcpy(new_node->fname,filename);

   new_node->file_type = file_type;
   new_node->last_mod_time = time;
   new_node->file_size = file_size;

   /* insert in sorted increasing order by time */

   if (dllist == NULL || dllist->last_mod_time > new_node->last_mod_time)
   {
      new_node->next = dllist;
      dllist = new_node;
      return;
   }

   temp = dllist;
   do
   {
      prev = temp;
      temp = temp->next;
   } while (temp != NULL && temp->last_mod_time <= new_node->last_mod_time);
   
   new_node->next = temp;
   prev->next = new_node;
}

void AddOrUpdateDLlist(char *filename,int time,int file_type, int file_size)
{
   dllist_node *temp,*prev;

   /* dprintf("dl list %s, %i\n",filename,file_type); */
   
   /* check if first in list first */

   if (dllist == NULL)
   {
      AddDLlist(filename,time,file_type,file_size);
      return;
   }

   if (!stricmp(filename,dllist->fname))
   {
      temp = dllist;
      dllist = dllist->next;
      FreeMemory(MALLOC_ID_DLLIST,temp,sizeof(dllist_node));
      AddDLlist(filename,time,file_type,file_size);
      return;
   }

   /* so it's not the first node, at least */

   prev = dllist;
   temp = dllist->next;
   while (temp != NULL)
   {
      if (!stricmp(filename,temp->fname))
      {
	 prev->next = temp->next;
	 FreeMemory(MALLOC_ID_DLLIST,temp,sizeof(dllist_node));
	 AddDLlist(filename,time,file_type,file_size);
	 return;
      }
      prev = temp;
      temp = temp->next;
   }

   AddDLlist(filename,time,file_type,file_size);
}

dllist_node * GetDLNodeByFilename(char *filename)
{
   dllist_node *temp;

   temp = dllist;
   while (temp != NULL)
   {
      if (!stricmp(filename,temp->fname))
      {
	 return temp;
      }
      temp = temp->next;
   }

   return NULL;
}

int CountNewDLFile(session_node *s)
{
   dllist_node *temp;
   int count;

   count = 0;
   temp = dllist;

   /* skip over older ones */
   while (temp != NULL && temp->last_mod_time <= s->last_download_time)
      temp = temp->next;

   while (temp != NULL)
   {
      count++;
      temp = temp->next;
   }

   return count;
}

void ForEachNewDLFile(session_node *s,void (*callback_func)(char *str,int time,int type,int size))
{
   dllist_node *temp;

   temp = dllist;

   while (temp != NULL && temp->last_mod_time <= s->last_download_time)
      temp = temp->next;

   while (temp != NULL)
   {
      callback_func(temp->fname,temp->last_mod_time,temp->file_type,temp->file_size);
      temp = temp->next;
   }
}

void ForEachDLlist(void (*callback_func)(dllist_node *dl))
{
   dllist_node *dl;

   dl = dllist;

   while (dl != NULL)
   {
      callback_func(dl);
      dl = dl->next;
   }
}

#define MAX_PACKAGE_LINE 200

void AddBuiltInDLlist()
{
   char filename[MAX_PATH+FILENAME_MAX];
   FILE *packagefile;
   char line[MAX_PACKAGE_LINE+1];
   int lineno,time,type,size;
   
   char *t1,*t2,*t3,*t4;

   sprintf(filename,"%s%s",ConfigStr(PATH_PACKAGE_FILE),PACKAGE_FILE);
   
   if ((packagefile = fopen(filename,"rt")) == NULL)
   {
      eprintf("AddBuiltinDLlist can't open %s to load the packages!\n",
	      filename);
      return;
   }

   lineno = 0;
   while (fgets(line,MAX_PACKAGE_LINE,packagefile))
   {
      lineno++;

      t1 = strtok(line," \t\n");

      if (t1 == NULL)	/* ignore blank lines */
	 continue;

      if (*t1 == '#')   /* ignore commentns */
	 continue;

      t2 = strtok(NULL," \t\n");
      if (t2 == NULL || sscanf(t2,"%i",&time) != 1)
      {
	 eprintf("AddBuiltInDLlist (%i) can't read time\n",lineno);
	 continue;
      }
      t3 = strtok(NULL," \t\n");
      if (t3 == NULL || sscanf(t3,"%i",&type) != 1)
      {
	 eprintf("AddBuiltInDLlist (%i) can't read type\n",lineno);
	 continue;
      }
      t4 = strtok(NULL," \t\n");
      if (t4 == NULL || sscanf(t4,"%i",&size) != 1)
      {
	 size = 1024; // default file size
#if 0
	 eprintf("AddBuiltInDLlist (%i) can't read size\n",lineno);
	 continue;
#endif
      }

      AddOrUpdateDLlist(t1,time,type,size);
   }

   fclose(packagefile);
}






