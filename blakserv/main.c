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
static Bool in_main_loop = False;

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

Bool InMainLoop(void)
{
   return in_main_loop;
}

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
	
	lprintf("Starting %s\n",BlakServLongVersionString());
	
	InitClass();
	InitMessage();
	InitObject();
	InitList();
	InitTimer();
	InitSession();
	InitResource();
	InitRoomData();
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
	InitTable();
	AddBuiltInDLlist();
	
	LoadMotd();
	LoadBof();
	LoadRsc();
	LoadKodbase();
	
	LoadAdminConstants();
	
	PauseTimers();
	// Start Discord webhook worker
	
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

	
	//SetWindowText(hwndMain, ConfigStr(CONSOLE_CAPTION));

    StartupComplete(); /* for the interface to report no errors on startup */
    InterfaceUpdate();
    lprintf("Status: %i accounts\n",GetNextAccountID());

    lprintf("-------------------------------------------------------------------------------------\n");
    dprintf("-------------------------------------------------------------------------------------\n");
    eprintf("-------------------------------------------------------------------------------------\n");

    AsyncSocketStart();

    in_main_loop = True;

    RunMainLoop();
	/* returns if server terminated */
	
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
	
	ResetTable();
	ResetBufferPool();
	ResetSysTimer();
	ResetDLlist();
	ResetNameID();
	ResetAccount();
	ResetUser();
	ResetString();
	ResetRoomData();
	ResetResource();
	ResetTimer();
	ResetList();
	ResetObject();
	ResetMessage();
	ResetClass();
	
	ResetConfig();
	
	DeleteAllBlocks();
}

