// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * statconn.c:  Handle STATE_CONNECTING, which is active when we are trying to connect
 *   to the server.
 */

#include "client.h"

/********************************************************************/
/*
 * ConnectingInit:  Enter STATE_CONNECTING.
 */
void ConnectingInit(void)
{
   ResetSecurityToken();
}
/********************************************************************/
/*
 * ConnectingExit:  Leave STATE_CONNECTING.
 */
void ConnectingExit(void)
{
}
/********************************************************************/
/*
 * ConnectingDone:  Status of connect request is known; error gives
 *   WSA error number (0 if success).
 */
void ConnectingDone(int error)
{
   if (error != 0)
   {
      debug(("Connect error %d\n", error));
      
      ClientError(hInst, hMain, IDS_SOCKETOPEN, config.comm.sockport, config.comm.hostname);
      MainSetState(STATE_OFFLINE);
      return;
   }
   MainSetState(STATE_LOGIN);
}
