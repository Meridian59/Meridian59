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

static HANDLE rscfile;

/* local function prototypes */
void CountEachDynamicRsc(resource_node *r);
void SaveEachDynamicRsc(resource_node *r);

/* SaveDynamicRsc
   This function writes the dynamic rscs into one file, for save games */
Bool SaveDynamicRsc(char *filename)
{
   int write_int;
   DWORD written;

   rscfile = CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,NULL);
   if (rscfile == INVALID_HANDLE_VALUE)
   {
      eprintf("SaveDynamicRsc can't open %s to save 'em!\n",filename);
      return False;
   }      
   
   if (!WriteFile(rscfile,magic_num,RSC_MAGIC_LEN,&written,NULL) || 
       written != RSC_MAGIC_LEN)
      eprintf("SaveDynamicRsc 1 error writing to file!\n");
   
   write_int = 1; /* version */
   if (!WriteFile(rscfile,&write_int,LEN_VERSION,&written,NULL) || 
       written != LEN_VERSION)
      eprintf("SaveDynamicRsc 2 error writing to file!\n");
   
   dynamic_rsc_count = 0;
   ForEachDynamicRsc(CountEachDynamicRsc);

   write_int = dynamic_rsc_count;

   if (!WriteFile(rscfile,&write_int,LEN_NUM_RSCS,&written,NULL) || 
       written != LEN_NUM_RSCS)
      eprintf("SaveDynamicRsc 3 error writing to file!\n");

   ForEachDynamicRsc(SaveEachDynamicRsc);

   CloseHandle(rscfile);

   return True;
}

void CountEachDynamicRsc(resource_node *r)
{
   dynamic_rsc_count++;
}

void SaveEachDynamicRsc(resource_node *r)
{
   int write_int;
   DWORD written;
   
   if (!WriteFile(rscfile,&r->resource_id,LEN_RSC_ID,&written,NULL) || 
       written != LEN_RSC_ID)
      eprintf("SaveEachDynamicRsc 1 error writing to file!\n");
   
   write_int = 0;
   if (!WriteFile(rscfile,&write_int,LEN_RSC_TYPE,&written,NULL) || 
       written != LEN_RSC_TYPE)
      eprintf("SaveEachDynamicRsc 2 error writing to file!\n");

   write_int = strlen(r->resource_val);

   if (!WriteFile(rscfile,&write_int,LEN_RSC_LEN,&written,NULL) || 
       written != LEN_RSC_LEN)
      eprintf("SaveEachDynamicRsc 3 error writing to file!\n");

   if (!WriteFile(rscfile,r->resource_val,strlen(r->resource_val),&written,NULL) || 
       written != (int)strlen(r->resource_val))
      eprintf("SaveEachDynamicRsc 4 error writing to file!\n");

}
