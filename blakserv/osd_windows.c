// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * osd_windows.c
 *

 This contains a windows implementation of some os-dependent functions.

 */

#include "blakserv.h"

void RunMainLoop(void)
{
   MSG msg;
   INT64 ms;

   for (;;)
   {
	   ms = GetMainLoopWaitTime();
      
	   if (MsgWaitForMultipleObjects(0,NULL,0,(DWORD)ms,QS_ALLINPUT) == WAIT_OBJECT_0)
	   {
		   while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		   {
			   if (msg.message == WM_QUIT)
			   {
				   lprintf("ServiceTimers shutting down the server\n");   
				   return;
			   }
	    
			   switch (msg.message)
			   {
			   case WM_BLAK_MAIN_READ :
				   EnterServerLock();
	       
				   PollSession((int) msg.lParam);
				   TimerActivate();
	       
				   LeaveServerLock();
				   break;
			   case WM_BLAK_MAIN_RECALIBRATE :
				   /* new soonest timer, so we should recalculate our time left... 
					  so we just need to restart the loop! */
				   break;
			   case WM_BLAK_MAIN_DELETE_ACCOUNT :
				   EnterServerLock();
				   DeleteAccountAndAssociatedUsersByID((int) msg.lParam);
				   LeaveServerLock();
				   break;

			   case WM_BLAK_MAIN_VERIFIED_LOGIN :
				   EnterServerLock();
				   VerifiedLoginSession((int) msg.lParam);
				   LeaveServerLock();
				   break;

			   default :
				   dprintf("ServiceTimers got unknown message %i\n",msg.message);
				   break;
			   }
		   }
	   }
	   else
	   {
		   /* a Blakod timer is ready to go */
	 
		   EnterServerLock();
		   PollSessions(); /* really just need to check session timers */
		   TimerActivate();
		   LeaveServerLock();
	   }
   }
}

char * GetLastErrorStr()
{
	char *error_str;
	
	error_str = "No error string"; /* in case the call  fails */
	
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,GetLastError(),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
		(LPTSTR) &error_str,0,NULL);
	return error_str;
}

bool FindMatchingFiles(const char *path, const char *extension, std::vector<std::string> *files)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA search_data;
	
	std::string path_spec(path);
	path_spec.append("\\*");
	path_spec.append(extension);
	files->clear();
	hFindFile = FindFirstFile(path_spec.c_str(), &search_data);
	if (hFindFile == INVALID_HANDLE_VALUE)
		return false;
   
	do
	{
		files->push_back(search_data.cFileName);
	} while (FindNextFile(hFindFile,&search_data));
	FindClose(hFindFile);
	
	return true;
}

bool BlakMoveFile(const char *source, const char *dest)
{
   if (!CopyFile(source,dest,FALSE))
   {
      eprintf("BlakMoveFile error moving %s to %s (%s)\n",source,dest,GetLastErrorStr());
      return false;
   }
   if (!DeleteFile(source))
   {
      eprintf("BlakMoveFile error deleting %s (%s)\n",source,GetLastErrorStr());
      return false;
   }
   return true;
}
