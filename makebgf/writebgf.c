// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * writebgf.c:  Routines for writing out a bgf file.
 */

#include "makebgf.h"

static const int BGF_VERSION = 10;
static BYTE magic[] = {0x42, 0x47, 0x46, 0x11};

typedef struct
{
   HANDLE fh;    // Handle of file
   HANDLE mapfh; // Handle of mapping object
   int  length;  // Length of file
   char *mem;    // Start of file
   char *ptr;    // Current position in file
} file_node;

static int  MappedFileRead(file_node *f, void *buf, int num);
static int  MappedFileWrite(file_node *f, void *buf, int num);
static Bool MappedFileOpenWrite(char *filename, file_node *f, int length);
static Bool MappedFileGoto(file_node *f, int pos);
static void MappedFileClose(file_node *f);

static int EstimateBGFFileSize(Bitmaps *b);
static BOOL WriteBitmap(file_node *f, PDIB pdib, Options *options);
/**************************************************************************/
/*
 * WriteBGFFile:  Write Bitmaps and Options structures out to given filename.
 *   Return FALSE on error.
 */
BOOL WriteBGFFile(Bitmaps *b, Options *opts, char *filename)
{
   int i, j, temp, max_indices, len;
   Group *g;
   file_node f;

   len = EstimateBGFFileSize(b);

   if (!MappedFileOpenWrite(filename, &f, len))
      return FALSE;

   // Write magic number
   for (i=0; i < 4; i++)
      if (MappedFileWrite(&f, &magic[i], 1) < 0) return FALSE;

   // Write version
   temp = BGF_VERSION;
   if (MappedFileWrite(&f, &temp, 4) < 0) return FALSE;

   // Write bitmap name
   if (MappedFileWrite(&f, &b->name, MAX_BITMAPNAME) < 0) return FALSE;

   // Write # of bitmaps
   if (MappedFileWrite(&f, &b->num_bitmaps, 4) < 0) return FALSE;

   // Write # of index groups
   if (MappedFileWrite(&f, &b->num_groups, 4) < 0) return FALSE;

   // Find most indices in a group
   max_indices = 0;
   for (i=0; i < b->num_groups; i++)
      if (b->groups[i].num_indices > max_indices)
	 max_indices = b->groups[i].num_indices;
   if (MappedFileWrite(&f, &max_indices, 4) < 0) return FALSE;   

   // Write shrink factor
   if (MappedFileWrite(&f, &opts->shrink, 4) < 0) return FALSE;      

   // Write out bitmaps
   for (i=0; i < b->num_bitmaps; i++)
   {
      PDIB pdib = b->bitmaps[i];

      // Write out bitmap size (swap width and height if rotated)
      if (opts->rotate)
      {
	 temp = DibHeight(pdib);
	 if (MappedFileWrite(&f, &temp, 4) < 0) return FALSE;
	 temp = DibWidth(pdib);
	 if (MappedFileWrite(&f, &temp, 4) < 0) return FALSE;
      }
      else
      {
	 temp = DibWidth(pdib);
	 if (MappedFileWrite(&f, &temp, 4) < 0) return FALSE;
	 temp = DibHeight(pdib);
	 if (MappedFileWrite(&f, &temp, 4) < 0) return FALSE;
      }

      // Write out x and y offsets
      temp = b->offsets[i].x;
      if (MappedFileWrite(&f, &temp, 4) < 0) return FALSE;
      temp = b->offsets[i].y;
      if (MappedFileWrite(&f, &temp, 4) < 0) return FALSE;

      // Write out hotspots
      if (MappedFileWrite(&f, &b->hotspots[i].num_hotspots, 1) < 0) return FALSE;
      for (j=0; j < b->hotspots[i].num_hotspots; j++)
      {
	 if (MappedFileWrite(&f, &b->hotspots[i].numbers[j], 1) < 0) return FALSE;
	 if (MappedFileWrite(&f, &b->hotspots[i].positions[j].x, 4) < 0) return FALSE;
	 if (MappedFileWrite(&f, &b->hotspots[i].positions[j].y, 4) < 0) return FALSE;
      }
      
      // Write out the bytes of the bitmap
      if (!WriteBitmap(&f, pdib, opts)) return FALSE;
   }

   // Write out indices
   for (i=0; i < b->num_groups; i++)
   {
      g = &b->groups[i];
      if (MappedFileWrite(&f, &g->num_indices, 4) < 0) return FALSE;
      for (j=0; j < g->num_indices; j++)
      if (MappedFileWrite(&f, &g->indices[j], 4) < 0) return FALSE;
   }
   
   UnmapViewOfFile(f.mem);
   CloseHandle(f.mapfh);

   SetFilePointer(f.fh, (f.ptr - f.mem), NULL, FILE_BEGIN);
   SetEndOfFile(f.fh);

   CloseHandle(f.fh);
   return TRUE;
}

/**************************************************************************/
/*
 * WriteBitmap:  Write bytes of given PDIB out to given file.
 *   Return FALSE on error.
 */
BOOL WriteBitmap(file_node *f, PDIB pdib, Options *options)
{
   int width, height, row, col, len, temp;
   BYTE *bits;
   BYTE *buf, *bufptr, byte;
      
   width = DibWidth(pdib);
   height = DibHeight(pdib);

   buf = (BYTE *) malloc(width * height);
   bufptr = buf;

   if (options->rotate)
   {
      for (col=0; col < width; col++)
      {
	 bits = (BYTE *) DibPtr(pdib) + col; 
	 
	 for (row = 0; row < height; row++)
	 {
	    *bufptr++ = *bits;
	    bits += DibWidthBytes(pdib);
	 }
      }
   }
   else
   {
      for (row=0; row < height; row++)
      {
	 // Watch out--rows are 4-bytes aligned
	 bits = (BYTE *) DibPtr(pdib) + row * DibWidthBytes(pdib); 
	 memcpy(bufptr, bits, width);
	 bufptr += width;
      }
   }

   // Leave space for # of bytes

   if (options->compress) 
   {
      len = compressBound(width * height);
      int retval = compress2((Bytef *) (f->ptr + 5), (uLongf *) &len, buf, width * height, Z_BEST_COMPRESSION);
      if (retval == Z_OK)
      {
         byte = 1;
         // Save compressed length
         MappedFileWrite(f, &byte, 1);
         MappedFileWrite(f, &len, 4);
         f->ptr += len;
      } else {
         return False;
      }
   }
   else len = -1;

   if (len < 0)
   {
      byte = 0;
      MappedFileWrite(f, &byte, 1);
      temp = 0;
      MappedFileWrite(f, &temp, 4);

      // Buffer is incompressible; just write out bits themselves
      MappedFileWrite(f, buf, width * height);
   }
   
   free(buf);
   return True;
}
/***************************************************************************/
/* 
 * EstimateBGFFileSize: Return an upper bound on the size of the BGF file
 *   that will result from saving the given bitmaps.
 */
int EstimateBGFFileSize(Bitmaps *b)
{
   Group *g;
   int len = 0, i;

   len += 56;   // header stuff

   // Space for hotspots and bitmaps
   for (i=0; i < b->num_bitmaps; i++)
   {
      PDIB pdib = b->bitmaps[i];

      len += 17 + 9 * b->hotspots[i].num_hotspots;
      len += DibHeight(pdib) * DibWidth(pdib);
   }

   // Write out indices
   for (i=0; i < b->num_groups; i++)
   {
      g = &b->groups[i];
      len += 4 + 4 * g->num_indices;
   }

   len += 100;  // just to be safe

   return len;
}

/***************************************************************************/
/*
 * MappedFileOpenWrite:  Open filename as a memory-mapped file for writing, 
 *   and map a view of the entire file.  Returns True on success, and fills in f.
 */
Bool MappedFileOpenWrite(char *filename, file_node *f, int length)
{
   f->fh = CreateFile(filename,GENERIC_READ | GENERIC_WRITE,0,NULL,
		      CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
   if (f->fh == INVALID_HANDLE_VALUE)
   {
      return False;
   }

   f->length = length;

   f->mapfh = CreateFileMapping(f->fh,NULL,PAGE_READWRITE,0,f->length,NULL);
   if (f->mapfh == NULL)
   {
      CloseHandle(f->fh);
      return False;
   }

   f->mem = (char *) MapViewOfFile(f->mapfh,FILE_MAP_WRITE,0,0,0);
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
void MappedFileClose(file_node *f)
{
   UnmapViewOfFile(f->mem);
   CloseHandle(f->mapfh);
   CloseHandle(f->fh);
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

