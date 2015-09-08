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
   FILE *f;
   
   int i,version,num_strs,len_str,str_id;

   f = fopen(filename, "rb");
   if (f == NULL)
   {
      eprintf("LoadBlakodStrings can't open %s to load the strings, none loaded\n",
              filename);
      return False;
   }
   
   if (fread(&version, 1, 4, f) != 4 ||
       fread(&num_strs, 1, 4, f) != 4)
   {
      fclose(f);
      return False;
   }
   
   for (i=0;i<num_strs;i++)
   {
      if (fread(&str_id, 1, 4, f) != 4 ||
          fread(&len_str, 1, 4, f) != 4)
      {
         fclose(f);
         return False;
      }
      
      if (!LoadBlakodString(f,len_str,str_id))
      {
         fclose(f);
         return False;
      }
   }

   fclose(f);

   return True;
}

