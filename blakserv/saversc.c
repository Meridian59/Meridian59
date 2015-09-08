// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * saversc.c
 *

 This module saves dynamic resources to files.  One copy is kept in the upload
 directory, which are copied to the save game dir when the game is saved.

 Dynamic resources are weird.  The saved game has the times of the dynamic rsc
 files, which when loaded in are stored in loaded_drsc_per_file and loaded_drsc_time.
 Then once the resources are loaded, WriteAllDynamicResource is called, which writes
 all the dynamic resources to disk.  Then the saved times are used to set
 the file time of these files, as long as chunking size didn't change.  Phew.
 */

#include "blakserv.h"

static char magic_num[] = { 'R', 'S', 'C', 1 };
#define RSC_MAGIC_LEN sizeof(magic_num)

int dynamic_rsc_count;

static FILE *rscfile;

/* local function prototypes */
void CountEachDynamicRsc(resource_node *r);
void SaveEachDynamicRsc(resource_node *r);

// Functions for writing to the save rsc buffer.
void SaveRscCopyByteBuffer(char byte_buffer);
void SaveRscCopyIntBuffer(int int_buffer);
void SaveRscCopyStringBuffer(const char *string_buffer);
// Used to flush the buffer and write to file.
void SaveRscFlushBuffer();

// buffer is used for buffering data to write at one time, vs writing multiple
// times with small amount of data.
static char *buffer;

// buffer_position is the current position of buffer we're writing to, and the
// length that will be written to file if flushed.
static int buffer_position;

// buffer_size is the size of the currently allocated memory for buffer.
static int buffer_size;

// buffer_warning_size is used to check if we need to flush the buffer. Equal
// to 80% buffer_size.
static int buffer_warning_size;

/* SaveDynamicRsc
   This function writes the dynamic rscs into one file, for save games */
Bool SaveDynamicRsc(char *filename)
{
   // Buffer for buffering data to call fwrite() once. Used to speed up
   // saving for resources.
   buffer_position = 0;
   buffer = (char *)AllocateMemory(MALLOC_ID_SAVE_GAME, SAVEGAME_BUFFER);
   buffer_size = SAVEGAME_BUFFER;
   buffer_warning_size = (SAVEGAME_BUFFER / 10) * 8;

   rscfile = fopen(filename, "wb");
   if (rscfile == NULL)
   {
      eprintf("SaveDynamicRsc can't open %s to save 'em!\n",filename);
      return False;
   }      

   if (fwrite(magic_num, 1, RSC_MAGIC_LEN, rscfile) != RSC_MAGIC_LEN)
      eprintf("SaveDynamicRsc error writing to file!\n");
   
   // Write version
   SaveRscCopyIntBuffer(SAVE_RSC_VERSION);
   
   dynamic_rsc_count = 0;
   ForEachDynamicRsc(CountEachDynamicRsc);

   SaveRscCopyIntBuffer(dynamic_rsc_count);

   ForEachDynamicRsc(SaveEachDynamicRsc);

   // Flush buffer if it still contains data.
   if (buffer_position > 0)
      SaveRscFlushBuffer();

   fclose(rscfile);

   // Free buffer memory.
   FreeMemory(MALLOC_ID_SAVE_GAME, buffer, buffer_size);

   return True;
}

void CountEachDynamicRsc(resource_node *r)
{
   dynamic_rsc_count++;
}

void SaveEachDynamicRsc(resource_node *r)
{
   // Write resource ID.
   SaveRscCopyIntBuffer(r->resource_id);

   // Write language ID (English only for now).
   SaveRscCopyIntBuffer(0);

   // Save English resource string. Size is saved as int, then string.
   SaveRscCopyStringBuffer(r->resource_val[0]);
}

// Write 4 bytes to buffer.
void SaveRscCopyIntBuffer(int int_buffer)
{
   memcpy(&(buffer[buffer_position]), &int_buffer, 4);
   buffer_position += 4;

   // Flush buffer at 80%.
   if (buffer_position > buffer_warning_size)
      SaveRscFlushBuffer();
}

// Write 1 byte to buffer.
void SaveRscCopyByteBuffer(char byte_buffer)
{
   memcpy(&(buffer[buffer_position]), &byte_buffer, 1);
   buffer_position++;

   // Flush buffer at 80%.
   if (buffer_position > buffer_warning_size)
      SaveRscFlushBuffer();
}

// Write a string to buffer.
void SaveRscCopyStringBuffer(const char *string_buffer)
{
   size_t len_s;

   if (string_buffer == NULL)
   {
      eprintf("SaveRscCopyStringBuffer line %i can't save NULL string, invalid saved game\n", \
         __LINE__);
      len_s = 0;
      memcpy(&(buffer[buffer_position]), &len_s, 4);
      buffer_position += 4;
   }
   else
   {
      len_s = strlen(string_buffer);
      memcpy(&(buffer[buffer_position]), &len_s, 4);
      buffer_position += 4;
      memcpy(&(buffer[buffer_position]), string_buffer, len_s);
      buffer_position += len_s;
   }

   // Flush buffer at 80%.
   if (buffer_position > buffer_warning_size)
      SaveRscFlushBuffer();
}

// Write buffer to save rsc file, reset buffer position.
void SaveRscFlushBuffer()
{
   if (fwrite(buffer, buffer_position, 1, rscfile) != 1)
      eprintf("File %s Line %i error writing to file!\n", __FILE__, __LINE__);
   buffer_position = 0;
}
