// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * saveall.c
 *

 This module uses the three save modules, saveacco.c, savegame.c, and
 savestr.c to save the entire system.  If successful, it modifies a
 control file with the current date and time so that loadall.c will
 know which game to load in.  This control file has the time (as an
 integer, number of seconds) that was used to generate the filenames
 of the saved files.  Loadall.c reads this file, gets the integer, and
 then knows the filenames to load.

 */

#include "blakserv.h"

int SaveAll(void)
{
   Bool save_ok;
   int save_time;
   char save_name[MAX_PATH+FILENAME_MAX];
   char time_str[100];
   
   /* Note:  You must call GarbageCollect() right before SaveAll() */
   
/*
	 charlie: machine sets the local time from a time synch server
     its potentially dangerous, but only if the time has been changed
	 between either server startup or last save, which is unlikely to
	 have drifted by much, things will only start to freak out if the 
	 difference is more than say a minute behind, meaning the all the
	 timers will run for an extra minute.. its better this way as the 
	 time is going to be corrected every (currently) 4 hours , even
	 the worst PC clocks aren`t going to degrade that much in 4 hours
*/
     
   /* The current time is used as a suffix to the save filenames.  
      We make our own copy since the time functions use a static
      buffer. */
   save_time = GetTime();
   sprintf(time_str,"%i",save_time);
   
   save_ok = True;

   lprintf("Saving game (time stamp %s)...\n", time_str);
   
   sprintf(save_name,"%s%s%s",ConfigStr(PATH_LOADSAVE),GAME_FILE_SAVE,time_str);
   if (SaveGame(save_name) == False)
      save_ok = False;

   sprintf(save_name,"%s%s%s",ConfigStr(PATH_LOADSAVE),STRING_FILE_SAVE,time_str);
   if (SaveStrings(save_name) == False)
      save_ok = False;

   sprintf(save_name,"%s%s%s",ConfigStr(PATH_LOADSAVE),ACCOUNT_FILE_SAVE,time_str);
   if (SaveAccounts(save_name) == False)
      save_ok = False;

   sprintf(save_name,"%s%s%s",ConfigStr(PATH_LOADSAVE),DYNAMIC_RSC_FILE_SAVE,time_str);
   if (!SaveDynamicRsc(save_name))
      save_ok = False;

   if (save_ok)
      SaveControlFile(save_time);
   
   lprintf("Save game successful (time stamp %s).\n", time_str);

   if (save_ok)
      return save_time;

   return 0;
}


void SaveControlFile(int save_time)
{
   char save_name[MAX_PATH+FILENAME_MAX];
   FILE *savefile;

   sprintf(save_name,"%s%s",ConfigStr(PATH_LOADSAVE),SAVE_CONTROL_FILE);
   if ((savefile = fopen(save_name,"wt")) == NULL)
   {
      eprintf("SaveContrtolFile can't open %s to save date/time of successful save!!!\n",
	      save_name);
      return;
   }

   fprintf(savefile,"#\n");
   fprintf(savefile,"# Control file for last successful save\n");
   fprintf(savefile,"#\n");
   fprintf(savefile,"# Files written:\n");
   fprintf(savefile,"# %s%s%i\n",ConfigStr(PATH_LOADSAVE),GAME_FILE_SAVE,save_time);
   fprintf(savefile,"# %s%s%i\n",ConfigStr(PATH_LOADSAVE),ACCOUNT_FILE_SAVE,save_time);
   fprintf(savefile,"# %s%s%i\n",ConfigStr(PATH_LOADSAVE),STRING_FILE_SAVE,save_time);
   fprintf(savefile,"# %s%s%i\n",ConfigStr(PATH_LOADSAVE),DYNAMIC_RSC_FILE_SAVE,save_time);
   fprintf(savefile,"#\n");
   fprintf(savefile,"# Last successful save was at %s\n",TimeStr(save_time));
   fprintf(savefile,"#\n");
   fprintf(savefile,"\n");
   fprintf(savefile,"LASTSAVE %i\n",save_time);
   
   fclose(savefile);
}
