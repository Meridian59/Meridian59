/*
 * memmap.h:  Header for memmap.c
 */

#ifndef _MEMMAP_H
#define _MEMMAP_H

typedef unsigned char Bool;
enum {False = 0, True = 1};

typedef struct
{
   HANDLE fh;    // Handle of file
   HANDLE mapfh; // Handle of mapping object
   int  length;  // Length of file
   char *mem;    // Start of file
   char *ptr;    // Current position in file
} file_node;

Bool MappedFileOpenRead(char *filename,file_node *f);
Bool MappedFileOpenWrite(char *filename, file_node *f, int length);
int  MappedFileOpenCopy(char *filename, file_node *f);
int  MappedFileRead(file_node *f, void *buf, int num);
int  MappedFileWrite(file_node *f, void *buf, int num);
void MappedFileClose(file_node *f);

#endif /* #ifndef _MEMMAP_H */
