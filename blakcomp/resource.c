// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * resource.c:  Write out resource file from symbol table information.
 */
#include "blakcomp.h"
#include "wrap.h"

#include <windows.h>

typedef struct
{
   HANDLE fh;    // Handle of file
   HANDLE mapfh; // Handle of mapping object
   int  length;  // Length of file
   char *mem;    // Start of file
   char *ptr;    // Current position in file
} file_node;

#define RSC_VERSION 3
static char rsc_magic[] = {0x52, 0x53, 0x43, 0x01};

// Encryption password
static char *password = "\x02F\x0C6\x046\x0DA\x020\x00E\x09F\x0F9\0x72";

char *GetStringFromResource(resource_type r);
int MappedFileWrite(file_node *f, void *buf, int num);
Bool MappedFileOpenWrite(char *filename, file_node *f, int length);
void MappedFileClose(file_node *f);
/******************************************************************************/
/*
 * write_resources: Write out resources to a .rsc file.  fname should be the 
 *    name of the .rsc file to receive the resources.  Resources are found by
 *    looping through all the classes in the symbol table.
 */
void write_resources(char *fname)
{
   list_type c, l;
   resource_type r;
   int num_resources, i, temp, max_length, response;
   char *main_pos;
   class_type cl;
   file_node f;
   char *str;
   BYTE byte;

   /* Count resources and compute length */
   num_resources = 0;
   max_length = 0;
   for (c = st.classes; c != NULL; c = c->next)
   {
      cl = (class_type) c->data;
      if (cl->is_new)
	 for (l = cl->resources; l != NULL; l = l->next)
	 {
	    r = (resource_type) (l->data);
	    
	    num_resources++;
	    str = GetStringFromResource(r);
	    max_length += 4 + strlen(str) + 1;
	 }
   }
   /* If no resources, do nothing */
   if (num_resources == 0)
      return;

   max_length += 100;  // For header, etc.; be on the safe side
   if (!MappedFileOpenWrite(fname, &f, max_length))
   {
      simple_error("Unable to open resource file %s!", fname);
      return;
   }
   
   /* Write out header information */
   for (i=0; i < 4; i++)
      MappedFileWrite(&f, &rsc_magic[i], 1);

   temp = RSC_VERSION;
   MappedFileWrite(&f, &temp, 4);
   MappedFileWrite(&f, &num_resources, 4);

   // Leave 8 bytes of space for encryption info
   f.ptr += 8;
   main_pos = f.ptr;
      
   /* Loop through classes in this source file, and then their resources */
   for (c = st.classes; c != NULL; c = c->next)
   {
      cl = (class_type) c->data;
      if (cl->is_new)
	 for (l = cl->resources; l != NULL; l = l->next)
	 {
	    r = (resource_type) (l->data);

	    // Write out id #
	    MappedFileWrite(&f, &r->lhs->idnum, 4);

	    // Write string
	    str = GetStringFromResource(r);
	    MappedFileWrite(&f, str, strlen(str));
	    byte = 0;
	    MappedFileWrite(&f, &byte, 1);
	 }
   }

   WrapInit();
   response = WrapEncrypt(main_pos, (f.ptr - main_pos), password, RSC_VERSION);
   WrapShutdown();

   // Write out length of encrypted section and response to challenge
   temp = (f.ptr - main_pos);
   memcpy(main_pos - 8, &temp, 4);
   memcpy(main_pos - 4, &response, 4);

   // Set length of file
   UnmapViewOfFile(f.mem);
   CloseHandle(f.mapfh);

   SetFilePointer(f.fh, (f.ptr - f.mem), NULL, FILE_BEGIN);
   SetEndOfFile(f.fh);

   CloseHandle(f.fh);
}
/***************************************************************************/
char *GetStringFromResource(resource_type r)
{
   switch (r->rhs->type)
   {
   case C_STRING:
      return r->rhs->value.stringval;

   case C_FNAME:
      return r->rhs->value.fnameval;

   default:
      simple_error("Unknown resource type (%d) encountered", r->rhs->type);
      return NULL;
   }
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
/*
 * MappedFileOpenWrite:  Open filename as a memory-mapped file for writing, and map a view
 *   of maximum size length.  Returns True on success, and fills in f.
 */
Bool MappedFileOpenWrite(char *filename, file_node *f, int length)
{
   f->fh = CreateFile(filename,GENERIC_READ | GENERIC_WRITE,0,NULL,
		      CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
   if (f->fh == INVALID_HANDLE_VALUE)
   {
      return False;
   }

   f->mapfh = CreateFileMapping(f->fh,NULL,PAGE_READWRITE,0,length,NULL);
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

   f->length = length;
   f->ptr = f->mem;
   return True;
}
/***************************************************************************/
void MappedFileClose(file_node *f)
{
   UnmapViewOfFile(f->mem);
   CloseHandle(f->mapfh);
   CloseHandle(f->fh);
}
