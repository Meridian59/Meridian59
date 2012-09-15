// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * qeditor.c:  The guts of the "Q" Quest Editor dialog.
 */

#include "client.h"
#include "dm.h"

static HWND hQEditorDlg = NULL;
static BOOL bQEditorHidden = FALSE;

static void OnQEditorCommand(HWND hDlg, int cmd_id, HWND hwndCtl, UINT codeNotify);
BOOL CALLBACK QEditorDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/****************************************************************************/

HWND GetQEditorDlg()
{
   return (bQEditorHidden? NULL : hQEditorDlg);
}

void ShowQEditorDlg()
{
   if (!hQEditorDlg)
   {
      hQEditorDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_QEDITOR),
                                  cinfo->hMain, QEditorDialogProc);
   }

   if (hQEditorDlg)
   {
      ShowWindow(hQEditorDlg, SW_SHOWNORMAL);
      bQEditorHidden = FALSE;
   }
}

/****************************************************************************/

BOOL CALLBACK QEditorDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
	 OnQEditorCommand(hDlg, nID, hwndCtrl, uNotifyCode);
      }
      return TRUE;

   case WM_DESTROY:
      hQEditorDlg = NULL;
      bQEditorHidden = FALSE;
      if (exiting)
	 PostMessage(cinfo->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
      return TRUE;
   }

   return FALSE;
}

/****************************************************************************/

void OnQEditorCommand(HWND hDlg, int cmd_id, HWND hwndCtl, UINT codeNotify)
{
   switch(cmd_id)
   {
   case IDOK: // "submit"
      // verify all fields filled
      // submit log item to server
      // clear all fields
      break;
      
   case IDCANCEL: // "close"
      ShowWindow(hDlg, SW_HIDE);
      bQEditorHidden = TRUE;
      break;
   }
}
