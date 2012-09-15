/*
 * paldump.c:  Dump palette for given bitmap file.
 */

#include <windows.h>
#include <stdio.h>

#include "memmap.h"

#define NUM_COLORS 256

/*****************************************************************************************/
void DumpPalette(char *filename)
{
   file_node f;
   int i;
   BITMAPFILEHEADER fh;
   BITMAPINFOHEADER bi;
   RGBQUAD colors[NUM_COLORS];

   if (!MappedFileOpenRead(filename, &f))
   {
      printf("%12s can't open\n", filename);
      return;
   }

   MappedFileRead(&f, &fh, sizeof(BITMAPFILEHEADER));
   MappedFileRead(&f, &bi, sizeof(BITMAPINFOHEADER));

   for (i=0; i < NUM_COLORS; i++)
   {
      MappedFileRead(&f, &colors[i].rgbBlue, 1);
      MappedFileRead(&f, &colors[i].rgbGreen, 1);
      MappedFileRead(&f, &colors[i].rgbRed, 1);
      MappedFileRead(&f, &colors[i].rgbReserved, 1);

      printf("%d %d %d\n", colors[i].rgbRed, colors[i].rgbGreen, colors[i].rgbBlue);
   }

   MappedFileClose(&f);
}
/*****************************************************************************************/
int main (int argc, char **argv)
{
   DumpPalette(argv[1]);
}
