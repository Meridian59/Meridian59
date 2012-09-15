/*
 * bmpdump.c:  Dump some basic info about bmp files.
 */

#include <windows.h>
#include <stdio.h>

#include "memmap.h"

static int total_size;

/*****************************************************************************************/
void DumpBitmap(char *filename)
{
   file_node f;
   int width, height;

   if (!MappedFileOpenRead(filename, &f))
   {
      printf("%12s can't open\n", filename);
      return;
   }

   memcpy(&width, f.ptr + 18, 4);
   memcpy(&height, f.ptr + 22, 4);
   
   printf("%12s: %3d x %3d  size = %7d bytes\n", filename, width, height, width * height);

   total_size += width * height;

   MappedFileClose(&f);
}
/*****************************************************************************************/
int main (int argc, char **argv)
{
   int i;

   total_size = 0;

   for (i = 1; i < argc; i++)
      DumpBitmap(argv[i]);

   printf("total size = %d bytes\n", total_size);
}
