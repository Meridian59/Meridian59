// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * memmap.c:  Some utility procedures for memory-mapped files.
 */

#include "client.h"

/***************************************************************************/
/*
 * MappedFileOpenRead:  Open filename as a memory-mapped file for read-only access, 
 *   and map a view of the entire file.  Returns True on success, and fills in f.
 */
Bool CliMappedFileOpenRead(char *filename, file_node *f)
{
   f->fh = CreateFile(filename,GENERIC_READ,0,NULL,
		      OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
   if (f->fh == INVALID_HANDLE_VALUE)
   {
      return False;
   }

   f->length = GetFileSize(f->fh, NULL);

   f->mapfh = CreateFileMapping(f->fh,NULL,PAGE_READONLY,0,f->length,NULL);
   if (f->mapfh == NULL)
   {
      CloseHandle(f->fh);
      return False;
   }

   f->mem = (char *) MapViewOfFile(f->mapfh,FILE_MAP_READ,0,0,0);
   if (f->mem == NULL)
   {
      CloseHandle(f->mapfh);
      CloseHandle(f->fh);
      return False;
   }

   f->ptr = f->mem;
   return True;
}
/***************************************************************************/
/*
 * MappedFileOpenCopy:  Open filename as a memory-mapped file for write-on-copy, 
 *   and map a view of the entire file.  Returns True on success, and fills in f.
 */
Bool MappedFileOpenCopy(char *filename, file_node *f)
{
   f->fh = CreateFile(filename,GENERIC_READ | GENERIC_WRITE,0,NULL,
		      OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
   if (f->fh == INVALID_HANDLE_VALUE)
   {
      return False;
   }

   f->length = GetFileSize(f->fh, NULL);

   f->mapfh = CreateFileMapping(f->fh,NULL,PAGE_WRITECOPY,0,f->length,NULL);
   if (f->mapfh == NULL)
   {
      CloseHandle(f->fh);
      return False;
   }

   f->mem = (char *) MapViewOfFile(f->mapfh,FILE_MAP_COPY,0,0,0);
   if (f->mem == NULL)
   {
      CloseHandle(f->mapfh);
      CloseHandle(f->fh);
      return False;
   }

   f->ptr = f->mem;
   return True;
}
/***************************************************************************/
/*
 * MappedFileRead:  Copy num bytes from the memory-mapped file to buf.
 *   Increments the file pointer.  
 *   Returns num on success, or -1 on failure.
 */
int CliMappedFileRead(file_node *f, void *buf, int num)
{
   if ((f->ptr - f->mem) + num > f->length || f->ptr < f->mem || num <= 0)
      return -1;
      
   memcpy(buf, f->ptr, num);
   f->ptr += num;
   return num;
}
/***************************************************************************/
/*
 * MappedFileGoto:  Set file pointer of f to given position.
 *   Return True iff position is legal.
 */
Bool MappedFileGoto(file_node *f, int pos)
{
   if (pos < 0 || pos > f->length)
      return False;

   f->ptr = f->mem + pos;
   return True;
}
/***************************************************************************/
Bool MappedFileClose(file_node *f)
{
   UnmapViewOfFile(f->mem);
   CloseHandle(f->mapfh);
   CloseHandle(f->fh);
   return False;
}
