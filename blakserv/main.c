// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* main.c
*

  Blakserv is the server program for Blakston.  This is a windows application,
  so we have a WinMain.  There is a dialog box interface in interfac.c.
  
	This module starts all of our "subsystems" and calls the timer loop,
	which executes until we terminate (either by the window interface or by
	a system administrator logging in to administrator mode.
	
*/

#include "blakserv.h"

/* local function prototypes */
void MainUsage();
void MainServer();
void MainExitServer();

DWORD main_thread_id;

#ifdef BLAK_PLATFORM_WINDOWS

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrev_instance,char *command_line,int how_show)
{
	main_thread_id = GetCurrentThreadId();
	
	StoreInstanceData(hInstance,how_show);

	MainServer();

	return 0;
}

#else

int main(int argc, char **argv)
{
	MainServer();
	return 0;
}

#endif

void MainServer()
{
	InitInterfaceLocks(); 
	
	InitInterface(); /* starts a thread with the window */

	InitMemory(); /* memory needs channels in general, but need to start before config,
	so just be careful. */
	
	InitConfig();
	LoadConfig();		/* must be nearly first since channels use it */

	InitDebug();
	
	InitChannelBuffer();
	
	OpenDefaultChannels();

	if (ConfigBool(MYSQL_ENABLED))
	{
		lprintf("Starting MySQL writer");
		MySQLInit(ConfigStr(MYSQL_HOST), ConfigStr(MYSQL_USERNAME), ConfigStr(MYSQL_PASSWORD), ConfigStr(MYSQL_DB));
	}

	lprintf("Starting %s\n",BlakServLongVersionString());
	
	InitClass();
	InitMessage();
	InitObject();
	InitList();
	InitTimer();
	InitSession();
	InitResource();
	InitRooms();
	InitString();
	InitUser();
	InitAccount();
	InitNameID();
	InitDLlist();   
	InitSysTimer();
	InitMotd();
	InitLoadBof();
	InitTime();
	InitGameLock();
	InitBkodInterpret();
	InitBufferPool();
	InitTables();
	AddBuiltInDLlist();
	
	LoadMotd();
	LoadBof();
	LoadRsc();
	LoadKodbase();
	
	LoadAdminConstants();
	
	PauseTimers();
	
	if (LoadAll() == True)
	{
	/* this loaded_game_msg tells it to disconnect all blakod info about sessions,
		* that were logged on when we saved */
		
		SendTopLevelBlakodMessage(GetSystemObjectID(),LOADED_GAME_MSG,0,NULL);
		DoneLoadAccounts();
	}
	
	/* these must be after LoadAll and ClearList */
	InitCommCli(); 
	InitParseClient(); 
	InitProfiling();
	InitAsyncConnections();
	
	UpdateSecurityRedbook();
	
	UnpauseTimers();

	

	ServiceTimers();
	/* returns if server termiated */
	
	MainExitServer();
}

void MainExitServer()
{
	lprintf("ExitServer terminating server\n");
	
	ExitAsyncConnections();
	
	CloseAllSessions(); /* gotta do this before anything, cause it uses kod, accounts */
	
	CloseDefaultChannels();
	
	ResetLoadMotd();
	ResetLoadBof();
	
	ResetTables();
	ResetBufferPool();
	ResetSysTimer();
	ResetDLlist();
	ResetNameID();
	ResetAccount();
	ResetUser();
	ResetString();
	// ExitRooms calls ResetRooms in addition to clearing the array memory.
	ExitRooms();
	ResetResource();
	ResetTimer();
	ResetList();
	ResetObject();
	ResetMessage();
	ResetClass();

	if (ConfigBool(MYSQL_ENABLED))
		MySQLEnd();
	
	ResetConfig();
	
	DeleteAllBlocks();
}

// This function keeps the necessary calls to reset and reinit game data
// in one place, to reduce the chance of errors if modifying it. Used by
// the interface and admin reload commands.
void MainReloadGameData()
{
   // Reset data.
   ResetAdminConstants();
   ResetUser();
   ResetString();
   ResetRooms();
   ResetLoadMotd();
   ResetLoadBof();
   ResetDLlist();
   ResetNameID();
   ResetResource();
   ResetTimer();
   ResetList();
   ResetTables();
   ResetObject();
   ResetMessage();
   ResetClass();

   // Reload data.
   InitNameID();
   LoadMotd();
   LoadBof();
   LoadRsc();
   LoadKodbase();

   UpdateSecurityRedbook();
   LoadAdminConstants();
}

char * GetLastErrorStr()
{
#ifdef BLAK_PLATFORM_WINDOWS

	char *error_str;
	
	error_str = "No error string"; /* in case the call  fails */
	
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,GetLastError(),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
		(LPTSTR) &error_str,0,NULL);
	return error_str;
   
#else

   return strerror(errno);
   
#endif
}
