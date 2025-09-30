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
bool LoadAllButAccountAtTime(char *time_str);
bool LoadControlFile(int *last_save_time);

/* LoadAll
Can't do this if any sessions are logged in because sessions have
pointers to accounts
*/
bool LoadAll(void)
{
	char load_name[MAX_PATH+FILENAME_MAX];
	char time_str[100];
	int last_save_time;
	

	/* ban all the naughty children */
	BuildBannedIPBlocks("banned.txt");

	if (LoadControlFile(&last_save_time) == false)
	{
		lprintf("LoadAll initializing a new game\n");
		SetSystemObjectID(CreateObject(SYSTEM_CLASS,0,NULL));
		CreateBuiltIn();
		return false;
	}
	
	snprintf(time_str, sizeof(time_str), "%i",last_save_time);
	
	snprintf(load_name, sizeof(load_name), "%s%s%s",ConfigStr(PATH_LOADSAVE),ACCOUNT_FILE_SAVE,time_str);
	if (LoadAccounts(load_name) == false)
	{
		lprintf("LoadAll error loading accounts, initializing a new game\n");
		SetSystemObjectID(CreateObject(SYSTEM_CLASS,0,NULL));
		CreateBuiltIn();
		return false;
	}
	
	if (LoadAllButAccountAtTime(time_str) == false)
		return false;
	
	/* can't use TimeStr() in an xprintf because it uses TimeStr() too */
	strncpy(time_str,TimeStr(last_save_time).c_str(), sizeof(time_str));
	lprintf("LoadAll loaded game saved at %s\n",time_str);
	
	return true;
}

bool LoadAllButAccount(void)
{
	char time_str[100];
	int last_save_time;
	
	if (LoadControlFile(&last_save_time) == false)
	{
		/* couldn't load anything in, so system is dead */
		SetSystemObjectID(CreateObject(SYSTEM_CLASS,0,NULL));
		return false;
	}
	
	snprintf(time_str, sizeof(time_str), "%i",last_save_time);
	
	return LoadAllButAccountAtTime(time_str);
}

bool LoadAllButAccountAtTime(char *time_str)
{
	bool load_ok;
	char load_name[MAX_PATH+FILENAME_MAX];
	
	load_ok = true;
	
	snprintf(load_name, sizeof(load_name), "%s%s%s",ConfigStr(PATH_LOADSAVE),STRING_FILE_SAVE,time_str);
	if (LoadBlakodStrings(load_name) == false)
		load_ok = false;
	
	snprintf(load_name, sizeof(load_name), "%s%s%s",ConfigStr(PATH_LOADSAVE),GAME_FILE_SAVE,time_str);
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
	
	snprintf(load_name, sizeof(load_name), "%s%s%s",ConfigStr(PATH_LOADSAVE),DYNAMIC_RSC_FILE_SAVE,time_str);
	LoadDynamicRsc(load_name);
	
	return load_ok;
}

bool LoadControlFile(int *last_save_time)
{
	FILE *loadfile;
	char line[MAX_SAVE_CONTROL_LINE+1];
	char *t1,*t2;
	char load_name[MAX_PATH+FILENAME_MAX];
	int lineno;
	bool found_lastsave;
	
	snprintf(load_name, sizeof(load_name), "%s%s",ConfigStr(PATH_LOADSAVE),SAVE_CONTROL_FILE);
	if ((loadfile = fopen(load_name,"rt")) == NULL)
		return false;
	
	found_lastsave = false;
	
	lineno = 0;
	while (fgets(line,MAX_SAVE_CONTROL_LINE,loadfile))
	{
		lineno++;

		t1 = strtok(line," \r\n");
		t2 = strtok(NULL," \r\n");
		
		if (t1 == NULL)	/* ignore blank lines */
			continue;
		
		if (*t1 == '#')
			continue;
		
		if (stricmp(t1,"LASTSAVE") == 0)
			if (sscanf(t2,"%i",last_save_time) == 1)
			{
				found_lastsave = true;
				continue;
			}
			
			eprintf("LoadControl file invalid data line %s (%i)\n",
					load_name,lineno);
			fclose(loadfile);
			return false;
			
	}
	
	fclose(loadfile);
	
	return found_lastsave;
}

