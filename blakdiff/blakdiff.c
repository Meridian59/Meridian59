// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * blakdiff.c
 *
 */

#include <stdio.h>

#include <windows.h>
#include <wincon.h>

typedef struct
{
   HANDLE fh;
   HANDLE mapfh;
   char *mem;
   int length;
} file_node;

/* local function prototypes */

BOOL MappedFileOpen(char *filename,file_node *f);
void MappedFileClose(file_node *f);

int main(int argc,char *argv[])
{
   file_node file1,file2;
   int i;

   if (argc != 3)
   {
      printf("Blakdiff compares 2 files specified on the command line, gives return code\n");
      return 2;
   }

   if (!MappedFileOpen(argv[1],&file1))
   {
      printf("Blakdiff can't open %s\n",argv[1]);
      return 1;
   }

   if (!MappedFileOpen(argv[2],&file2))
   {
      printf("Blakdiff can't open %s\n",argv[2]);
      return 1;
   }

   if (file1.length != file2.length)
      return 1;

   for (i=0;i<file1.length;i++)
      if (file1.mem[i] != file2.mem[i])
      {
	 MappedFileClose(&file1);
	 MappedFileClose(&file2);
	 return 1;
      }

   MappedFileClose(&file1);
   MappedFileClose(&file2);
   return 0;
} 

BOOL MappedFileOpen(char *filename,file_node *f)
{
   f->fh = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,
		      OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
   if (f->fh == INVALID_HANDLE_VALUE)
   {
      return FALSE;
   }

   f->length = GetFileSize(f->fh,NULL);
   
   f->mapfh = CreateFileMapping(f->fh,NULL,PAGE_READONLY,0,f->length,NULL);
   if (f->mapfh == NULL)
   {
      CloseHandle(f->fh);
      return FALSE;
   }

   f->mem = (char *) MapViewOfFile(f->mapfh,FILE_MAP_READ,0,0,0);
   if (f->mem == NULL)
   {
      CloseHandle(f->mapfh);
      CloseHandle(f->fh);
      return FALSE;
   }

   return TRUE;
}

void MappedFileClose(file_node *f)
{
   UnmapViewOfFile(f->mem);
   CloseHandle(f->mapfh);
   CloseHandle(f->fh);
}
