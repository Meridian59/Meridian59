// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * savestr.c
 *

 This module saves the strings to a binary file.

 */

#include "blakserv.h"

HANDLE strfile;

/* local function prototypes */
void SaveEachString(string_node *snod,int string_id);

Bool SaveStrings(char *filename)
{
   int write_int;
   DWORD written;

   strfile = CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,NULL);
   if (strfile == INVALID_HANDLE_VALUE)
   {
      eprintf("SaveStrings can't open %s to save strings!\n",filename);
      return False;
   }

   write_int = 1; /* version */
   if (!WriteFile(strfile,&write_int,LEN_STR_VERSION,&written,NULL) || 
       written != LEN_STR_VERSION)
      eprintf("SaveStrings 1 error writing to file!\n");
   
   write_int = GetNumStrings();
   if (!WriteFile(strfile,&write_int,LEN_NUM_STRS,&written,NULL) || 
       written != LEN_NUM_STRS)
      eprintf("SaveStrings 2 error writing to file!\n");
   

   ForEachString(SaveEachString);
   CloseHandle(strfile);

   return True;
}

void SaveEachString(string_node *snod,int string_id)
{
   DWORD written;

   if (!WriteFile(strfile,&string_id,LEN_STR_ID,&written,NULL) || 
       written != LEN_STR_ID)
      eprintf("SaveEachString 1 error writing to file!\n");

   if (!WriteFile(strfile,&snod->len_data,LEN_STR_LEN,&written,NULL) || 
       written != LEN_STR_LEN)
      eprintf("SaveEachString 2 error writing to file!\n");

   if (!WriteFile(strfile,snod->data,snod->len_data,&written,NULL) ||
       written != snod->len_data)
      eprintf("SaveEachString 3 error writing to file!\n");
}
