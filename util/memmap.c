/*
 * memmap.c:  Utility functions for use with memory-mapped files.
 */

#include <windows.h>
#include "memmap.h"

/***************************************************************************/
/*
 * MappedFileOpenRead:  Open filename as a memory-mapped file for reading, and map the entire file
 *   into memory.  Returns true on success, and fills in f.
 */
bool MappedFileOpenRead(char *filename,file_node *f)
{
   f->fh = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,
		      OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
   if (f->fh == INVALID_HANDLE_VALUE)
   {
      return false;
   }

   f->length = GetFileSize(f->fh,NULL);
   
   f->mapfh = CreateFileMapping(f->fh,NULL,PAGE_READONLY,0,f->length,NULL);
   if (f->mapfh == NULL)
   {
      CloseHandle(f->fh);
      return false;
   }

   f->mem = (char *)MapViewOfFile(f->mapfh,FILE_MAP_READ,0,0,0);
   if (f->mem == NULL)
   {
      CloseHandle(f->mapfh);
      CloseHandle(f->fh);
      return false;
   }

   f->ptr = f->mem;
   return true;
}
/***************************************************************************/
/*
 * MappedFileOpenWrite:  Open filename as a memory-mapped file for writing, and map a view
 *   of maximum size length.  Returns true on success, and fills in f.
 */
bool MappedFileOpenWrite(char *filename, file_node *f, int length)
{
   f->fh = CreateFile(filename,GENERIC_READ | GENERIC_WRITE,0,NULL,
		      CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
   if (f->fh == INVALID_HANDLE_VALUE)
   {
      return false;
   }

   f->mapfh = CreateFileMapping(f->fh,NULL,PAGE_READWRITE,0,length,NULL);
   if (f->mapfh == NULL)
   {
      CloseHandle(f->fh);
      return false;
   }

   f->mem = (char *)MapViewOfFile(f->mapfh,FILE_MAP_WRITE,0,0,0);
   if (f->mem == NULL)
   {
      CloseHandle(f->mapfh);
      CloseHandle(f->fh);
      return false;
   }

   f->length = length;
   f->ptr = f->mem;
   return true;
}
/***************************************************************************/
/*
 * MappedFileRead:  Copy num bytes from the memory-mapped file to buf.
 *   Increments the file pointer.  
 *   Returns num on success, or -1 on failure.
 */
int MappedFileRead(file_node *f, void *buf, int num)
{
   if ((f->ptr - f->mem) + num > f->length || f->ptr < f->mem || num <= 0)
      return -1;
      
   memcpy(buf, f->ptr, num);
   f->ptr += num;
   return num;
}
/***************************************************************************/
/* 
 * MappedFileWrite: Write num bytes from buf to f, and increment the file pointer.
 *   Return num on success, -1 on failure.
 */
int MappedFileWrite(file_node *f, void *buf, int num)
{
   if ((f->ptr - f->mem) + num > f->length || num <= 0)
      return -1;
   
   memcpy(f->ptr, buf, num);
   f->ptr += num;

   return num;
}
/***************************************************************************/
void MappedFileClose(file_node *f)
{
   UnmapViewOfFile(f->mem);
   CloseHandle(f->mapfh);
   CloseHandle(f->fh);
}
