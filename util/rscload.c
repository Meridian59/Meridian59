/*
 * rscload.c:  Load a rsc file, and call a function for each loaded resource.
 */

#include <stdio.h>

#include "rscload.h"

static const int RSC_VERSION = 5;
static const int MAX_RSC_LEN = 20000;
static char rsc_magic[] = {0x52, 0x53, 0x43, 0x01};

static bool RscFileRead(char *fname, FILE *f, RscCallbackProc callback);
/***************************************************************************/
/*
 * RscFileLoad:  Open the given rsc file, and call the callback procedure
 *   for each resource in the file.
 *   Return true iff every resource in the file is passed to the callback, false otherwise.
 */
bool RscFileLoad(char *fname, RscCallbackProc callback)
{
   FILE *f;

   if (callback == NULL)
      return false;

   f = fopen(fname, "rb");
   if (f == NULL)
      return false;

   bool retval = RscFileRead(fname, f, callback);
   fclose(f); 
   return retval; 
}
/***************************************************************************/
/*
 * RscFileRead:  Do real work of RscFileLoad.
 */
bool RscFileRead(char *fname, FILE *f, RscCallbackProc callback)
{
   int i, num_resources, version, rsc_num, lang_id;
   unsigned char byte;
   
   // Check magic number and version
   for (i = 0; i < 4; i++)
      if (fread(&byte, 1, 1, f) != 1 || byte != rsc_magic[i])
         return false;
   
   if (fread(&version, 1, 4, f) != 4 || version != RSC_VERSION)
      return false;
   
   if (fread(&num_resources, 1, 4, f) != 4 || num_resources < 0)
      return false;

   // Read each resource
   for (i=0; i < num_resources; i++)
   {
      if (fread(&rsc_num, 1, 4, f) != 4)
         return false;

      // each resource has a language number and a string
      if (fread(&lang_id, 1, 4, f) != 4)
         return false;

      char str[MAX_RSC_LEN];
      int pos = 0;
      while (!feof(f) && fread(&str[pos], 1, 1, f) == 1)
      {
         // Too big for buffer?
         if (pos == MAX_RSC_LEN - 1)
            return false;
         
         // Reached end of string?
         if (str[pos] == '\0')
            break;
         pos++;
      }

      if (!(*callback)(fname, rsc_num, lang_id, str))
         return false;
   }

   return true;
}
