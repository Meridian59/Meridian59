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

#define LEN_VERSION 4
#define LEN_NUM_RSCS 4
#define LEN_RSC_ID 4
#define LEN_RSC_TYPE 4
#define LEN_RSC_LEN 4

static char magic_num[] = { 'R', 'S', 'C', 1 };
#define RSC_MAGIC_LEN sizeof(magic_num)

int dynamic_rsc_count;

static FILE *rscfile;

/* local function prototypes */
void CountEachDynamicRsc(resource_node *r);
void SaveEachDynamicRsc(resource_node *r);

/* SaveDynamicRsc
   This function writes the dynamic rscs into one file, for save games */
Bool SaveDynamicRsc(char *filename)
{
   int write_int;
   int written;

   rscfile = fopen(filename, "wb");
   if (rscfile == NULL)
   {
      eprintf("SaveDynamicRsc can't open %s to save 'em!\n",filename);
      return False;
   }      

   written = fwrite(magic_num, 1, RSC_MAGIC_LEN, rscfile);
   if (written != RSC_MAGIC_LEN)
      eprintf("SaveDynamicRsc 1 error writing to file!\n");
   
   write_int = 1; /* version */
   written = fwrite(&write_int, 1, LEN_VERSION, rscfile);
   if (written != LEN_VERSION)
      eprintf("SaveDynamicRsc 2 error writing to file!\n");
   
   dynamic_rsc_count = 0;
   ForEachDynamicRsc(CountEachDynamicRsc);

   write_int = dynamic_rsc_count;
   written = fwrite(&write_int, 1, LEN_NUM_RSCS, rscfile);
   if (written != LEN_NUM_RSCS)
      eprintf("SaveDynamicRsc 3 error writing to file!\n");

   ForEachDynamicRsc(SaveEachDynamicRsc);

   fclose(rscfile);

   return True;
}

void CountEachDynamicRsc(resource_node *r)
{
   dynamic_rsc_count++;
}

void SaveEachDynamicRsc(resource_node *r)
{
   int written = fwrite(&r->resource_id, 1, LEN_RSC_ID, rscfile);
   if (written != LEN_RSC_ID)
      eprintf("SaveEachDynamicRsc 1 error writing to file!\n");
   
   int write_int = 0;
   written = fwrite(&write_int, 1, LEN_RSC_TYPE, rscfile);
   if (written != LEN_RSC_TYPE)
      eprintf("SaveEachDynamicRsc 2 error writing to file!\n");

   write_int = strlen(r->resource_val);
   written = fwrite(&write_int, 1, LEN_RSC_LEN, rscfile);
   if (written != LEN_RSC_LEN)
      eprintf("SaveEachDynamicRsc 3 error writing to file!\n");

   written = fwrite(r->resource_val, 1, strlen(r->resource_val), rscfile);
   if (written != (int)strlen(r->resource_val))
      eprintf("SaveEachDynamicRsc 4 error writing to file!\n");
}
