// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * gchannel.c:  The guts of the "G" Channel dialog.
 */

#include "client.h"
#include "dm.h"

static HWND hGChannelDlg = NULL;
static BOOL bGChannelHidden = FALSE;

static void OnGChannelCommand(HWND hDlg, int cmd_id, HWND hwndCtl, UINT codeNotify);
BOOL CALLBACK GChannelDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/****************************************************************************/

HWND GetGChannelDlg()
{
   return (bGChannelHidden? NULL : hGChannelDlg);
}

void ShowGChannelDlg()
{
   debug(("ShowGChannelDlg\n"));
   if (!hGChannelDlg)
   {
      debug(("ShowGChannelDlg needs to create the dialog\n"));
      hGChannelDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_GCHANNEL),
                                  cinfo->hMain, GChannelDialogProc);
   }

   if (hGChannelDlg)
   {
      debug(("ShowGChannelDlg needs to show the dialog\n"));
      ShowWindow(hGChannelDlg, SW_SHOWNORMAL);
      bGChannelHidden = FALSE;
   }
}

/****************************************************************************/

BOOL CALLBACK GChannelDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, cinfo->hMain);
      return TRUE;

   case WM_ACTIVATE:
      *cinfo->hCurrentDlg = (wParam == 0)? NULL : hDlg;
      return TRUE;

   case WM_COMMAND:
      {
	 UINT uNotifyCode = (UINT)HIWORD(wParam);
	 int nID = (int)LOWORD(wParam);
	 HWND hwndCtrl = (HWND)lParam;
	 OnGChannelCommand(hDlg, nID, hwndCtrl, uNotifyCode);
      }
      return TRUE;

   case DMDLGM_CLICKEDUSER:
      {
	 debug(("setting text to '%s'\n", (LPCTSTR)lParam));
	 SetDlgItemText(hDlg, IDC_EDIT1, (LPCTSTR)lParam);
      }
      return TRUE;

   case WM_DESTROY:
      hGChannelDlg = NULL;
      bGChannelHidden = FALSE;
      if (exiting)
	 PostMessage(cinfo->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
      return TRUE;
   }

   return FALSE;
}

/****************************************************************************/

void OnGChannelCommand(HWND hDlg, int cmd_id, HWND hwndCtl, UINT codeNotify)
{
   ID id;
   int index;

   switch(cmd_id)
   {
   case IDOK: // "submit"
      // verify all fields filled
      // submit log item to server
      // clear all fields
      break;
      
   case IDCANCEL: // "close"
      ShowWindow(hDlg, SW_HIDE);
      bGChannelHidden = TRUE;
      break;
   }
}
