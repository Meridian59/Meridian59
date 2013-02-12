// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * guest.c:  Handle logging in guests.
 *
 * Guest logins attempt to find an available server.  The INI file contains a list
 * of possible servers; we try them in a random order and show the progress to the user
 * in a dialog.  The list of possible servers can be updated by a message from the
 * server.
 */

#include "client.h"

#define MAX_TRIES 25

static int num_tries = 0;      // # of login attempts we've made

static HWND hGuestDialog = NULL;

extern HWND hCurrentDlg;

static BOOL CALLBACK GuestDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static void AbortGuestDialog(void);
/********************************************************************/
/*
 * GuestGetLogin:  Set login parameters for a guest login.
 *   This includes setting the username, password, and server number.
 *   Return True iff login should proceed.
 */
Bool GuestGetLogin(void)
{
   int low, high;

   // Bring up dialog first time through; update bar on later attempts
   if (num_tries == 0)
      CreateDialog(hInst, MAKEINTRESOURCE(IDD_GUEST), hMain, GuestDialogProc);
   else SendMessage(hGuestDialog, BK_PROGRESS, 0, num_tries + 1);

   strcpy(config.username, GetString(hInst, IDS_GUEST));
   config.password[0] = 0;

   // Choose a new server to try
   // Try to re-login to previous server, if possible
   if (num_tries == 0 && config.server_guest != 0)
      config.comm.server_num = config.server_guest;
   else
   {
      low = config.server_low;
      high = config.server_high;
      if (high >= low)
	 config.comm.server_num = low + (rand() % (high - low + 1));
      else
      {
	 debug(("GuestLoggingIn got low server #%d > high server #%d\n", low, high));
	 config.comm.server_num = low;
      }
   }
   ConfigSetServerNameByNumber(config.comm.server_num);
   ConfigSetSocketPortByNumber(config.comm.server_num);

   num_tries++;

   if (num_tries > MAX_TRIES)
   {
      SendMessage(hGuestDialog, WM_COMMAND, IDCANCEL, 0);
      return False;
   }

   return True;
}
/********************************************************************/
/*
 * GuestConnectError:  Connection to server failed; try another server 
 *   or bail out.
 */
void GuestConnectError(void)
{
   OfflineConnect();
}
/********************************************************************/
/*
 * GuestLoggingIn:  Got a message from the server; update the available server
 *   range, and either log in or try another server.
 */
void GuestLoggingIn(BYTE status, int low, int high)
{
   config.server_low = low;
   config.server_high = high;

   // If we're allowed onto this server, we're done
   if (status == 0)
   {
      // Save this server as the last one we logged in to.
      config.server_guest = config.comm.server_num;
      AbortGuestDialog();
      return;
   }

   // If we're not allowed onto this server, try again
   OfflineConnect();
}

/*****************************************************************************/
/*
 * GuestDialogProc:  Display guest login dialog.
 */
BOOL CALLBACK GuestDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
   case WM_INITDIALOG:
      hGuestDialog = hDlg;
      CenterWindow(hDlg, GetParent(hDlg));

      // Set progress bar range
      SendDlgItemMessage(hDlg, IDC_GUESTBAR, PBM_SETRANGE, 
			 0, MAKELPARAM(0, MAX_TRIES));
      return TRUE;

   case BK_PROGRESS:
      // Set position of progress bar to lParam
      SendDlgItemMessage(hDlg, IDC_GUESTBAR, PBM_SETPOS, lParam, 0);
      return TRUE;
      
   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 MainSetState(STATE_OFFLINE);
	 num_tries = 0;
	 ClientError(hInst, hMain, IDS_GUESTFULL);
	 return TRUE;
      }
      break;

   case WM_ACTIVATE:
      if (wParam == 0)
	 hCurrentDlg = NULL;
      else hCurrentDlg = hDlg;
      return TRUE;

   case WM_DESTROY:
      hGuestDialog = NULL;
      num_tries = 0;
      return TRUE;
   }
   return FALSE;
}
/*****************************************************************************/
/*
 * GuestMove:  Called when main window moves.
 */
void GuestMove(HWND hwnd, int x, int y)
{
   if (hGuestDialog == NULL)
      return;

   CenterWindow(hGuestDialog, GetParent(hGuestDialog));
}
/*****************************************************************************/
/*
 * GuestResize:  Called when main window resizes.
 */
void GuestResize(HWND hwnd, UINT resize_flag, int xsize, int ysize)
{
   if (hGuestDialog == NULL)
      return;

   CenterWindow(hGuestDialog, GetParent(hGuestDialog));
}
/********************************************************************/
void AbortGuestDialog(void)
{
   if (hGuestDialog != NULL)
      DestroyWindow(hGuestDialog);
}
