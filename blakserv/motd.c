// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * motd.c
 *

 This module maintains the message of the day.  It is stored in memory
 as a zero terminated string, but is read in from the memmap directory
 (for permanent storage), or if there's a new one, from the memmap dir.

 */

#include "blakserv.h"

/* variables */
char *motd;

/* local function prototypes */
Bool LoadMotdName(char *fname);

void InitMotd(void)
{
   motd = NULL;
}

void LoadMotd(void)
{
   char file_load_path[MAX_PATH+FILENAME_MAX];
   char file_copy_path[MAX_PATH+FILENAME_MAX];

   sprintf(file_load_path,"%s%s",ConfigStr(PATH_MOTD),MOTD_FILE);
   sprintf(file_copy_path,"%s%s",ConfigStr(PATH_MEMMAP),MOTD_FILE);

   /* if there's a new motd file, move it in */

   if (access(file_load_path, 0) != -1)
   {
      // Delete any existing file
      if (access(file_copy_path, 0) != -1)
         unlink(file_copy_path);
      if (!rename(file_load_path,file_copy_path))
         eprintf("LoadMotd can't move %s\n",MOTD_FILE);
   }

   LoadMotdName(file_copy_path);
/*
   if (motd != NULL)
      dprintf("LoadMotd loaded the message of the day\n");
   else
      dprintf("LoadMotd found no message of the day\n");
*/
}

void ResetLoadMotd()
{ 
   if (motd != NULL)
   {
      FreeMemory(MALLOC_ID_MOTD,motd,strlen(motd)+1);
      motd = NULL;
   }
}

Bool LoadMotdName(char *fname)
{
   FILE *file;
   int file_size;
   int num_read;
   
   file = fopen(fname, "rb");
   if (file == NULL)
      return False;

   // Get file size
   struct stat st;
   stat(fname, &st);
   file_size = st.st_size;
   
   motd = (char *)AllocateMemory(MALLOC_ID_MOTD,file_size + 1);
   num_read = fread(motd, 1, file_size, file);
   if (num_read != file_size)
   {
      FreeMemory(MALLOC_ID_MOTD,motd,file_size + 1);
      motd = NULL;
      fclose(file);
      return False;
   }

   motd[file_size] = 0; /* zero terminate string */

   fclose(file);

   return True;
}

void SetMotd(char *new_motd)
{
   int fh;
   char filename[MAX_PATH+FILENAME_MAX];

   /* write file to memmap directory */

   sprintf(filename,"%s%s",ConfigStr(PATH_MEMMAP),MOTD_FILE);

   fh = open(filename,O_CREAT | O_TRUNC | O_BINARY | O_WRONLY,S_IREAD | S_IWRITE);

   if (fh == -1)
   {
      eprintf("SetMotd error saving motd file to %s\n",filename);
      return;
   }

   if (write(fh,new_motd,strlen(new_motd)) != (int)strlen(new_motd))
   {
      eprintf("SetMotd error writing motd file to %s\n",filename);
      close(fh);
      return;
   }

   close(fh);

   ResetLoadMotd();
   LoadMotdName(filename);
}

int GetMotdLength(void)
{
   if (motd != NULL)
      return strlen(motd);
   else
      return strlen(ConfigStr(MOTD_DEFAULT));
}

char * GetMotd(void)
{
   if (motd != NULL)
      return motd;
   else
      return ConfigStr(MOTD_DEFAULT);
}
