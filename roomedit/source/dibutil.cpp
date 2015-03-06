/*
 * dibutil.c:  Load a bgf file.
 */

#include "common.h"
#pragma hdrstop

#include "dibutil.h"

typedef struct
{
   HANDLE fh;    // Handle of file
   HANDLE mapfh; // Handle of mapping object
   int  length;  // Length of file
   char *mem;    // Start of file
   char *ptr;    // Current position in file
} file_node;

static BYTE magic[] = {0x42, 0x47, 0x46, 0x11};
#define BGF_VERSION 10

static int version;   // Version of file being loaded

/************* for dynamic zlib loading ******************/

// signature of uncompress method in zlib1.dll
typedef int(*uncompress_functype)(void* dest, unsigned long* destLen, const void* source, unsigned long sourceLen);

HMODULE             zlib_moduleptr = NULL;	// loaded dll
uncompress_functype uncompress     = NULL;	// callable function ptr

/*********************************************************/

int  MappedFileRead(file_node *f, void *buf, int num);
Bool MappedFileOpenRead(const char *filename, file_node *f);
Bool MappedFileClose(file_node *f);

static Bool DibReadHeader(file_node *f, Bitmaps *b, BYTE *shrink);
static void RotateBits(BYTE *b, int width, int height);
static Bool DibReadBits(file_node *f, PDIB pdib, int version);

/************************************************************************/
/*
* DibInitCompression: Dynamically loads the compression library (zlib1.dll)
*   Return TRUE on success or crashes with error message.
*/
Bool DibInitCompression()
{
   // try load the DLL into the process
   zlib_moduleptr = LoadLibraryEx("zlib1.dll", NULL, 0x00000200);

   // failed ...
   if (zlib_moduleptr == NULL)
   {
      Notify("Error loading zlib1.dll");

      // force crash
      assert(zlib_moduleptr != NULL);
   }

   // try get the entry point of 'uncompress'
   FARPROC uncompress_entryptr = GetProcAddress(zlib_moduleptr, "uncompress");

   // failed ...
   if (uncompress_entryptr == NULL)
   {
      Notify("Unable to find procedure 'uncompress' in zlib1.dll");

      // force crash
      assert(uncompress_entryptr != NULL);
   }

   // cast entry point to function
   uncompress = (uncompress_functype)uncompress_entryptr;

   return TRUE;
}
/************************************************************************/
/*
* DibCloseCompression: Dynamically unloads the compression library (zlib1.dll)
*   Return TRUE on success.
*/
Bool DibCloseCompression()
{
   if (zlib_moduleptr != NULL)
      FreeLibrary(zlib_moduleptr);

   zlib_moduleptr = NULL;
   uncompress = NULL;

   return TRUE;
}
/************************************************************************/
/*
 * DibOpenFile: Load the bitmaps in the file given by filename into the
 *   given bitmap structure.  Return TRUE on success.
 */
Bool DibOpenFile(const char *szFile, Bitmaps *b)
{
   file_node f;
   DWORD   dwLen, dwBits;
   DWORD   width, height, xoffset, yoffset;
	BYTE    num_hotspots, shrink;
   int     i, j, size, offset, num_indices;

   if (!MappedFileOpenRead(szFile, &f))
      return False;

   if (!DibReadHeader(&f, b, &shrink))
      return MappedFileClose(&f);
   
   // Allocate memory for bitmap pointers and indices
   size  = b->num_bitmaps * sizeof(PDIB) + b->num_groups * b->max_indices * sizeof(int);
   b->pdibs = (PDIB *) SafeMalloc(size);
   b->indices = (int *) ((BYTE *) b->pdibs + b->num_bitmaps * sizeof(PDIB));
   memset(b->pdibs, 0, size);
   
   // Read in bitmaps
   for (i=0; i < b->num_bitmaps; i++)
   {
      // Switch width and height since walls are rotated 90 degrees
      if (MappedFileRead(&f, &height, 4) != 4)
	 return MappedFileClose(&f);
      
      if (MappedFileRead(&f, &width, 4) != 4)
	 return MappedFileClose(&f);
      
      if (MappedFileRead(&f, &xoffset, 4) != 4)
	 return MappedFileClose(&f);
      
      if (MappedFileRead(&f, &yoffset, 4) != 4)
	 return MappedFileClose(&f);
      
      if (MappedFileRead(&f, &num_hotspots, 1) != 1)
	 return MappedFileClose(&f);
      
      /* How much memory do we need to hold the PDIB? */
      dwBits = width * height;
      dwLen  = dwBits + sizeof(DIBHEADER) + num_hotspots * (sizeof(POINT) + sizeof(BYTE));
      
      // Arrangement of PDIB in memory:
      // Header
      // bytes of bitmap
      // array of hotspot numbers
      // array of hotspot positions
      
      b->pdibs[i] = (PDIB) SafeMalloc(dwLen);
      
      b->pdibs[i]->width        = width;
      b->pdibs[i]->height       = height;
      b->pdibs[i]->xoffset      = xoffset;
      b->pdibs[i]->yoffset      = yoffset;
      b->pdibs[i]->num_hotspots = num_hotspots;
      b->pdibs[i]->numbers      = (char *) (DibPtr(b->pdibs[i]) + dwBits);
      b->pdibs[i]->hotspots     = (POINT *) (b->pdibs[i]->numbers + num_hotspots * sizeof(BYTE));
      b->pdibs[i]->shrink       = shrink;
      
      // read in the hotspots
      for (j=0; j < num_hotspots; j++)
      {
	 if ((MappedFileRead(&f, &b->pdibs[i]->numbers[j], 1) != 1) ||
	     (MappedFileRead(&f, &b->pdibs[i]->hotspots[j].x, 4) != 4) ||
	     (MappedFileRead(&f, &b->pdibs[i]->hotspots[j].y, 4) != 4))
	 {
	    BitmapsFree(b);
	    return MappedFileClose(&f);
	 }
      }
      
      /* read in the bits */
      if (!DibReadBits(&f, b->pdibs[i], version))
      {
	 BitmapsFree(b);
	 return MappedFileClose(&f);
      }
      RotateBits(DibPtr(b->pdibs[i]), height, width);
   }
   
   // Read in indices
   for (i=0; i < b->num_groups; i++)
   {
      // offset stores where we are in indices array
      offset = b->max_indices * i;
      if (MappedFileRead(&f, &num_indices, 4) != 4)
      {
	 BitmapsFree(b);
	 return MappedFileClose(&f);
      }
      b->indices[offset] = num_indices;
      for (j=0; j < num_indices; j++)
      {
	 offset++;
	 if (MappedFileRead(&f, &b->indices[offset], 4) != 4)
	 {
	    BitmapsFree(b);
	    return MappedFileClose(&f);
	 }
      }
   }
   
   MappedFileClose(&f);
   return True;
}
/************************************************************************/
/*
 * DibOpenFileSimple:  Read basic bitmap information from the given file into
 *   b, shrink, width, and height.  This does not load the actual bits of any bitmap.
 *   Return True on success.
 */
Bool DibOpenFileSimple(const char *szFile, Bitmaps *b, BYTE *shrink, int *width, int *height)
{
   file_node f;

   if (!MappedFileOpenRead(szFile, &f))
      return False;
   
   if (!DibReadHeader(&f, b, shrink))
      return MappedFileClose(&f);
   
   // Switch width and height since walls are rotated 90 degrees
   if (MappedFileRead(&f, height, 4) != 4)
      return MappedFileClose(&f);
   
   if (MappedFileRead(&f, width, 4) != 4)
      return MappedFileClose(&f);
   
   MappedFileClose(&f);
   return True;
}
/************************************************************************/
/*
 * DibReadHeader:  Read the initial bitmap information from the given file handle
 *   into the given Bitmaps structure.  Set shrink to the shrink factor.
 *   Return True on success.
 */
Bool DibReadHeader(file_node *f, Bitmaps *b, BYTE *shrink)
{
   int i, temp;
   BYTE byte;
   
   // Check magic number
   for (i=0; i < 4; i++)
      if (MappedFileRead(f, &byte, 1) != 1 || byte != magic[i])
	 return False;
   
   // Check version
   if (MappedFileRead(f, &version, 4) != 4 || version < BGF_VERSION)
   {
      dprintf("Bad BGF version %d\n", version);
      return False;
   }

   // Read bitmap name
   if (MappedFileRead(f, &b->name, MAX_BITMAPNAME) != MAX_BITMAPNAME)
      return False;
   
   // Read # of bitmaps
   if (MappedFileRead(f, &b->num_bitmaps, 4) != 4)
      return False;
   
   // Read # of bitmap groups
   if (MappedFileRead(f, &b->num_groups, 4) != 4)
      return False;
   
   // Read max # of indices in a group
   if (MappedFileRead(f, &b->max_indices, 4) != 4)
      return False;
   b->max_indices++;  // Leave room to store the # of indices actually present in a group
   
   // Read "shrink factor" of bitmaps
   if (MappedFileRead(f, &temp, 4) != 4)
      return False;
   *shrink = (BYTE) temp;
   if (*shrink == 0)
      *shrink = 1;    // Avoid divide by zero errors elsewhere
   
   return True;
}
/************************************************************************/
/*
 * DibReadBits:  Read bits of an image into the given PDIB.  The file pointer
 *   for f must be positioned at the start of the image information.
 *   version is version of BGF file.
 *   Return True on success.
 */
Bool DibReadBits(file_node *f, PDIB pdib, int version)
{
   BYTE *bits, type;
   int length, temp, compressed_length, retval;
   unsigned long len;

   bits = DibPtr(pdib);
   length = DibWidth(pdib) * DibHeight(pdib);

   if (version < 10)
      return False;
   
   // See if image is compressed
   if (MappedFileRead(f, &type, 1) != 1) return False;

   switch (type)
   {
   case 0:
      if (MappedFileRead(f, &temp, 4) != 4) return False;  // Skip unused bytes
      if (MappedFileRead(f, bits, length) != length) return False;
      break;
   case 1:
      if (MappedFileRead(f, &compressed_length, 4) != 4) return False;

      len = length;
      retval = uncompress((void*)bits, &len, (const void*)f->ptr, (unsigned long)compressed_length);
      if (retval != 0) // 0 = Z_OK
      {
         dprintf("DibReadBits error during decompression\n");
         return False;
      }

      f->ptr += compressed_length;
      break;
   default:
      dprintf("DibReadBits got bad type byte %d\n", (int) type);
      return False;
   }
   return True;
}
/************************************************************************/
/*
 * Free memory associated with PDIB.
 */
void DibFree(PDIB pdib)
{
   SafeFree(pdib);
}
/************************************************************************/
/*
 * Free memory associated with bitmap structure.
 */
void BitmapsFree(Bitmaps *b)
{
   int i;

   // First free individual bitmaps, if they have been loaded
   for (i=0; i < b->num_bitmaps; i++)
      if (b->pdibs[i] != NULL)
	 DibFree(b->pdibs[i]);
   SafeFree(b->pdibs);
}
/************************************************************************/
/*
 * RotateBits:  Swap rows and columns of given memory.
 *   width and height give the size of the unrotated image.
 *   This is pretty inefficient, but it's only used while loading a bitmap from
 *   disk, so who cares?
 */
void RotateBits(BYTE *b, int width, int height)
{
   BYTE *temp;
   int i,j;

   temp = (BYTE *) SafeMalloc(width * height);

   for (i=0; i < height; i++)
      for (j=0; j < width; j++)
	 *(temp + j * height + i) = *(b + i * width + j);
      
   memcpy(b, temp, width * height);
   SafeFree(temp);
}

/***************************************************************************/
/*
 * MappedFileOpenRead:  Open filename as a memory-mapped file for read-only access, 
 *   and map a view of the entire file.  Returns True on success, and fills in f.
 */
Bool MappedFileOpenRead(const char *filename, file_node *f)
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
Bool MappedFileClose(file_node *f)
{
   UnmapViewOfFile(f->mem);
   CloseHandle(f->mapfh);
   CloseHandle(f->fh);
   return False;
}
