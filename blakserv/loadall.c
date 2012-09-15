// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * loadall.c
 *

  This module uses the three load modules loadacco.c, loadgame.c, and
  loadstr.c to load the entire system.  It reads the last save time
  from the save control file to determine the filenames of the saved
  game.  The file is a text file, with comment lines started with a
  pound sign (#).  The important line starts with "LOADSAVE", followed
  by an integer that represents the time.
  
*/

#include "blakserv.h"

#define MAX_SAVE_CONTROL_LINE 200

/* local function prototypes */
Bool LoadAllButAccountAtTime(char *time_str);
Bool LoadControlFile(int *last_save_time);

/* LoadAll
Can't do this if any sessions are logged in because sessions have
pointers to accounts
*/
Bool LoadAll(void)
{
	char load_name[MAX_PATH+FILENAME_MAX];
	char time_str[100];
	int last_save_time;
	

	/* ban all the naughty children */
	BuildBannedIPBlocks("banned.txt");

	if (LoadControlFile(&last_save_time) == False)
	{
		lprintf("LoadAll initializing a new game\n");
		SetSystemObjectID(CreateObject(SYSTEM_CLASS,0,NULL));
		CreateBuiltIn();
		return False;
	}
	
	sprintf(time_str,"%i",last_save_time);
	
	sprintf(load_name,"%s%s%s",ConfigStr(PATH_LOADSAVE),ACCOUNT_FILE_SAVE,time_str);
	if (LoadAccounts(load_name) == False)
	{
		lprintf("LoadAll error loading accounts, initializing a new game\n");
		SetSystemObjectID(CreateObject(SYSTEM_CLASS,0,NULL));
		CreateBuiltIn();
		return False;
	}
	
	if (LoadAllButAccountAtTime(time_str) == False)
		return False;
	
	/* can't use TimeStr() in an xprintf because it uses TimeStr() too */
	strcpy(time_str,TimeStr(last_save_time));
	lprintf("LoadAll loaded game saved at %s\n",time_str);
	
	return True;
}

Bool LoadAllButAccount(void)
{
	char time_str[100];
	int last_save_time;
	
	if (LoadControlFile(&last_save_time) == False)
	{
		/* couldn't load anything in, so system is dead */
		SetSystemObjectID(CreateObject(SYSTEM_CLASS,0,NULL));
		return False;
	}
	
	sprintf(time_str,"%i",last_save_time);
	
	return LoadAllButAccountAtTime(time_str);
}

Bool LoadAllButAccountAtTime(char *time_str)
{
	Bool load_ok;
	char load_name[MAX_PATH+FILENAME_MAX];
	
	load_ok = True;
	
	sprintf(load_name,"%s%s%s",ConfigStr(PATH_LOADSAVE),STRING_FILE_SAVE,time_str);
	if (LoadBlakodStrings(load_name) == False)
		load_ok = False;
	
	sprintf(load_name,"%s%s%s",ConfigStr(PATH_LOADSAVE),GAME_FILE_SAVE,time_str);
	if (!LoadGame(load_name)) 
	{
	/* If loadgame failed, create a system object which basically starts
	a new game.  This is good when you want to use an old account file
		and start a new game. */
		
		ClearObject();
		ClearList(); 
		ClearTimer();
		ClearUser();
		SetSystemObjectID(CreateObject(SYSTEM_CLASS,0,NULL));
	}
	
	sprintf(load_name,"%s%s%s",ConfigStr(PATH_LOADSAVE),DYNAMIC_RSC_FILE_SAVE,time_str);
	LoadDynamicRsc(load_name);
	
	return load_ok;
}

Bool LoadControlFile(int *last_save_time)
{
	FILE *loadfile;
	char line[MAX_SAVE_CONTROL_LINE+1];
	char *t1,*t2;
	char load_name[MAX_PATH+FILENAME_MAX];
	int lineno;
	Bool found_lastsave;
	
	sprintf(load_name,"%s%s",ConfigStr(PATH_LOADSAVE),SAVE_CONTROL_FILE);
	if ((loadfile = fopen(load_name,"rt")) == NULL)
		return False;
	
	found_lastsave = False;
	
	lineno = 0;
	while (fgets(line,MAX_SAVE_CONTROL_LINE,loadfile))
	{
		lineno++;
		
		t1 = strtok(line," \n");
		t2 = strtok(NULL," \n");
		
		if (t1 == NULL)	/* ignore blank lines */
			continue;
		
		if (*t1 == '#')
			continue;
		
		if (stricmp(t1,"LASTSAVE") == 0)
			if (sscanf(t2,"%i",last_save_time) == 1)
			{
				found_lastsave = True;
				continue;
			}
			
			eprintf("LoadControl file invalid data line %s (%i)\n",
				load_name,lineno);
			fclose(loadfile);
			return False;
			
	}
	
	fclose(loadfile);
	
	return found_lastsave;
}

