/*
 * memmap.h:  Header for memmap.c
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

bool MappedFileOpenRead(char *filename,file_node *f);
bool MappedFileOpenWrite(char *filename, file_node *f, int length);
int  MappedFileRead(file_node *f, void *buf, int num);
int  MappedFileWrite(file_node *f, void *buf, int num);
void MappedFileClose(file_node *f);

#endif /* #ifndef _MEMMAP_H */
