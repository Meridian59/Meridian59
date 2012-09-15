// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * loadstr.c
 *

 This module loads in the string database file and tells string.c each
 string it finds.  The format of the strings.sav file, which is a
 binary file, is not documented?

 */

#include "blakserv.h"

Bool LoadBlakodStrings(char *filename)
{
   HANDLE fh,mapfh;
   char *file_mem,*str_file_ptr;
   int file_size;
   
   int i,version,num_strs,len_str,str_id;

   fh = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,
		   OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
   if (fh == INVALID_HANDLE_VALUE)
   {
      eprintf("LoadBlakodStrings can't open %s to load the strings, none loaded\n",
	      filename);
      return False;
   }
   
   file_size = GetFileSize(fh,NULL);
   
   mapfh = CreateFileMapping(fh,NULL,PAGE_READONLY,0,file_size,NULL);
   if (mapfh == NULL)
   {
      CloseHandle(fh);
      return False;
   }

   file_mem = (char *) MapViewOfFile(mapfh,FILE_MAP_READ,0,0,0);
   if (file_mem == NULL)
   {
      CloseHandle(mapfh);
      CloseHandle(fh);
      return False;
   }

   str_file_ptr = file_mem;

   version = *(int *)str_file_ptr;
   str_file_ptr += LEN_STR_VERSION;
   num_strs = *(int *)str_file_ptr;
   str_file_ptr += LEN_NUM_STRS;

   for (i=0;i<num_strs;i++)
   {
      str_id = *(int *)str_file_ptr;
      str_file_ptr += LEN_STR_ID;

      len_str = *(int *)str_file_ptr;
      str_file_ptr += LEN_STR_LEN;

      if (!LoadBlakodString(str_file_ptr,len_str,str_id))
      {
	 UnmapViewOfFile(file_mem);
	 CloseHandle(mapfh);
	 CloseHandle(fh);
	 return False;
      }
      str_file_ptr += len_str;
   }

   UnmapViewOfFile(file_mem);
   CloseHandle(mapfh);
   CloseHandle(fh);

   /*
   dprintf("LoadBlakodStrings successfully loaded strings from %s\n",
	   STRING_FILE);
   */
   return True;
}

