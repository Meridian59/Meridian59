// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * memmap.h:  Header file for memmap.c
 * TODO: Delete this file and instead use the memmap.c from the util directory.
 */

#ifndef _MEMMAP_H
#define _MEMMAP_H

typedef struct
{
   HANDLE fh;    // Handle of file
   HANDLE mapfh; // Handle of mapping object
   int  length;  // Length of file
   char *mem;    // Start of file
   char *ptr;    // Current position in file
} file_node;

// Rename to avoid naming conflicts
int  CliMappedFileRead(file_node *f, void *buf, int num);
Bool CliMappedFileOpenRead(char *filename, file_node *f);
Bool MappedFileOpenCopy(char *filename, file_node *f);
Bool MappedFileGoto(file_node *f, int pos);
Bool MappedFileClose(file_node *f);

#endif /* #ifndef _MEMMAP_H */
