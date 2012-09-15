/*
 * rscload.c:  Load a rsc file, and call a function for each loaded resource.
 */

#include <windows.h>

#include "memmap.h"
#include "rscload.h"
#include "wrap.h"

#define RSC_VERSION 3
static char rsc_magic[] = {0x52, 0x53, 0x43, 0x01};

// Encryption password
static char *password = "\x02F\x0C6\x046\x0DA\x020\x00E\x09F\x0F9\0x72";

static BOOL RscFileRead(char *fname, file_node *f, RscCallbackProc callback);
/***************************************************************************/
/*
 * RscFileLoad:  Open the given rsc file, and call the callback procedure
 *   for each resource in the file.
 *   Return TRUE iff every resource in the file is passed to the callback, FALSE otherwise.
 */
BOOL RscFileLoad(char *fname, RscCallbackProc callback)
{
   file_node f;

   if (callback == NULL)
      return FALSE;
   
   if (!MappedFileOpenCopy(fname, &f))
      return FALSE;

   if (!RscFileRead(fname, &f, callback))
   { 
      MappedFileClose(&f); 
      return FALSE; 
   }
      
   MappedFileClose(&f);
   return TRUE;
}
/***************************************************************************/
/*
 * RscFileRead:  Do real work of RscFileLoad.
 */
BOOL RscFileRead(char *fname, file_node *f, RscCallbackProc callback)
{
   int i, num_resources, version, len, response, rsc_num;
   BYTE byte;

   // Check magic number and version
   for (i = 0; i < 4; i++)
      if (MappedFileRead(f, &byte, 1) != 1 || byte != rsc_magic[i])
	 return FALSE;

   if (MappedFileRead(f, &version, 4) != 4 || version != RSC_VERSION)
      return FALSE;

   if (MappedFileRead(f, &num_resources, 4) != 4 || num_resources < 0)
      return FALSE;

   if (MappedFileRead(f, &len, 4) != 4 || MappedFileRead(f, &response, 4) != 4)
      return FALSE;
   
   // Decrypt file in place; won't be reflected in file since opened in write-copy mode
   // Make sure that file is actually this long
   if ((f->ptr - f->mem) + len > f->length)
      return FALSE;

   if (!WrapDecrypt(f->ptr, len, password, version, response))
      return FALSE;

   // Read each resource
   for (i=0; i < num_resources; i++)
   {
      if (MappedFileRead(f, &rsc_num, 4) != 4)
	 return FALSE;

      if (!(*callback)(fname, rsc_num, f->ptr))
	 return FALSE;
      f->ptr += strlen(f->ptr) + 1;
   }

   return TRUE;
}

