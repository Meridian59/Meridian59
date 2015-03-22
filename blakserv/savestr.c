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

FILE *strfile;

/* local function prototypes */
void SaveEachString(string_node *snod,int string_id);

Bool SaveStrings(char *filename)
{
   int write_int;
   int written;

   strfile = fopen(filename, "wb");
   if (strfile == NULL)
   {
      eprintf("SaveStrings can't open %s to save strings!\n",filename);
      return False;
   }

   write_int = 1; /* version */
   written = fwrite(&write_int, 1, LEN_STR_VERSION, strfile);
   if (written != LEN_STR_VERSION)
      eprintf("SaveStrings 1 error writing to file!\n");
   
   write_int = GetNumStrings();
   written = fwrite(&write_int, 1, LEN_NUM_STRS, strfile);
   if (written != LEN_NUM_STRS)
      eprintf("SaveStrings 2 error writing to file!\n");

   ForEachString(SaveEachString);
   fclose(strfile);

   return True;
}

void SaveEachString(string_node *snod,int string_id)
{
   int written;

   written = fwrite(&string_id, 1, LEN_STR_ID, strfile);
   if (written != LEN_STR_ID)
      eprintf("SaveEachString 1 error writing to file!\n");

   written = fwrite(&snod->len_data, 1, LEN_STR_LEN, strfile);
   if (written != LEN_STR_LEN)
      eprintf("SaveEachString 2 error writing to file!\n");

   written = fwrite(snod->data, 1, snod->len_data, strfile);
   if (written != snod->len_data)
      eprintf("SaveEachString 3 error writing to file!\n");
}
