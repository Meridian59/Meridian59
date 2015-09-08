// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * savestr.c
 *

 This module saves the strings to a binary file.

 */

#include "blakserv.h"

FILE *strfile;

/* local function prototypes */
void SaveEachString(string_node *snod,int string_id);

// Functions for writing to the save game buffer.
void SaveStrCopyIntBuffer(int int_buffer);
void SaveStrCopySnodBuffer(string_node *snod);
// Used to flush the buffer and write to file.
void SaveStrFlushBuffer();

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

Bool SaveStrings(char *filename)
{
   strfile = fopen(filename, "wb");
   if (strfile == NULL)
   {
      eprintf("SaveStrings can't open %s to save strings!\n",filename);
      return False;
   }

   // Buffer for buffering data to call fwrite() once. Used to speed up
   // saving for strings.
   buffer_position = 0;
   buffer = (char *)AllocateMemory(MALLOC_ID_SAVE_GAME, SAVEGAME_BUFFER);
   buffer_size = SAVEGAME_BUFFER;
   buffer_warning_size = (SAVEGAME_BUFFER / 10) * 8;

   // Write version
   SaveStrCopyIntBuffer(SAVE_STR_VERSION);

   // Write number of strings.
   SaveStrCopyIntBuffer(GetNumStrings());

   ForEachString(SaveEachString);

   // Flush buffer if it still contains data.
   if (buffer_position > 0)
      SaveStrFlushBuffer();

   fclose(strfile);

   // Free buffer memory.
   FreeMemory(MALLOC_ID_SAVE_GAME, buffer, buffer_size);

   return True;
}

void SaveEachString(string_node *snod, int string_id)
{
   // Write string ID
   SaveStrCopyIntBuffer(string_id);

   // Write string length/data.
   SaveStrCopySnodBuffer(snod);
}

// Write 4 bytes to buffer.
void SaveStrCopyIntBuffer(int int_buffer)
{
   memcpy(&(buffer[buffer_position]), &int_buffer, 4);
   buffer_position += 4;

   // Flush buffer at 80%.
   if (buffer_position > buffer_warning_size)
      SaveStrFlushBuffer();
}

// Write a string node (length and data) to buffer.
void SaveStrCopySnodBuffer(string_node *snod)
{
   size_t len_s;

   if (snod == NULL)
   {
      eprintf("SaveStrCopyStringBuffer line %i can't save NULL string, invalid saved game\n", \
         __LINE__);
      len_s = 0;
      memcpy(&(buffer[buffer_position]), &len_s, 4);
      buffer_position += 4;
   }
   else
   {
      memcpy(&(buffer[buffer_position]), &(snod->len_data), 4);
      buffer_position += 4;
      memcpy(&(buffer[buffer_position]), snod->data, snod->len_data);
      buffer_position += snod->len_data;
   }

   // Flush buffer at 80%.
   if (buffer_position > buffer_warning_size)
      SaveStrFlushBuffer();
}

// Write buffer to save rsc file, reset buffer position.
void SaveStrFlushBuffer()
{
   if (fwrite(buffer, buffer_position, 1, strfile) != 1)
      eprintf("File %s Line %i error writing to file!\n", __FILE__, __LINE__);
   buffer_position = 0;
}
