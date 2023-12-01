// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statoffl.c:  Handle offline state, where user isn't connected to server.
 */

#include "client.h"

extern int connection;  /* Current connection to server: CON_NONE, CON_SERIAL, ... */
/****************************************************************************/
void OfflineInit(void)
{
   connection = CON_NONE;
   ModuleLoadByName("resource\\intro.dll");
   ResetSecurityToken();
}
/****************************************************************************/
void OfflineExit(void)
{
   // Exit intro DLL
   ModuleExitById(MODULE_OFFLINE_ID);
}
/****************************************************************************/
void OfflineSetFocus(HWND hwnd, HWND hwndOldFocus)
{
   ModuleEvent(EVENT_SETFOCUS);
}
/****************************************************************************/
void OfflineExpose(HWND hwnd)
{
   ModuleEvent(EVENT_REDRAW);
}
/****************************************************************************/
void OfflineResize(int xsize, int ysize)
{
   ModuleEvent(EVENT_RESIZE, xsize, ysize, NULL);
}
/****************************************************************************/
/*
 * OfflineConnect:  Attempt to connect to server using appropriate method.
 */
void OfflineConnect(void)
{
   if (!GetLogin())
      return;

   connection = CON_SOCKET;
   
   if (!OpenSocketConnection(config.comm.hostname, config.comm.sockport))
   {
      connection = CON_NONE;
      ClientError(hInst, hMain, IDS_SOCKETOPEN, config.comm.sockport, config.comm.hostname);
      return;
   }
   MainSetState(STATE_CONNECTING);
}
