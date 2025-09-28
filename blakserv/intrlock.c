// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * intrlock.c
 *
 
 This module maintains the locks for thread synchronization between
 the interface thread and the main thread.  There is a lock to do
 anything with the server and one to do quitting.

 */

#include "blakserv.h"

Mutex muxServer;

Mutex muxQuit;
bool quit;

void InitInterfaceLocks()
{
   muxServer = MutexCreate();

   quit = false;
   muxQuit = MutexCreate();
}

void EnterServerLock()
{
   MutexAcquire(muxServer);
}

bool TryEnterServerLock()
{
   return MutexAcquireWithTimeout(muxServer, 50);
}

void LeaveServerLock()
{
   MutexRelease(muxServer);
}

void SetQuit()
{
   MutexAcquire(muxQuit);   
   quit = true;

#ifdef BLAK_PLATFORM_WINDOWS
   PostThreadMessage(main_thread_id,WM_QUIT,0,0);
#endif
   MutexRelease(muxQuit);
}

bool GetQuit()
{
   bool copy_quit;

   MutexAcquire(muxQuit);   
   copy_quit = quit;
   MutexRelease(muxQuit);

   return copy_quit;
}

void SignalSession(int session_id)
{
	// doesn't seem to really be needed because each time through
	// the main loop every session is checked anyway
#ifdef BLAK_PLATFORM_WINDOWS
	PostThreadMessage(main_thread_id,WM_BLAK_MAIN_READ,0,session_id);
#endif
}
