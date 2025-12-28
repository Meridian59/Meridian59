//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
// Implements message hooks for catching F1 in the dialogs
//----------------------------------------------------------------------------
#include "bole.h"
#include "bolesvc.h"

extern "C" {
  #include "ole2ui.h"
}


// BOleHelpNotify is a C function which is exposed to the OLE2UI dialog
// boxes (but not to other DLLs) for them to call back to get help from
// the application.
//
// 2 minor hacks are used here to solve the language differences between
// the Bolero header files (C++ only) and OLE2UI (C, and I don't have the
// time to convert it to C++)
//
// 1. We're using the resource IDs of the dialog boxes to identify which
//    dialog box to provide help on since OLE2UI can't compile the
//    BOleDialogHelp enum.
//
// 2. The pCastToApp parameter is used because OLE2UI can't compile the
//    class definition for IBApplication.
//
extern "C" void BOleHelpNotify (DWORD pCastToApp, int dialogResId)
{
  BOleDialogHelp helpCode;
  switch (dialogResId) {
    case IDD_INSERTCONTROL: helpCode = BOLE_HELP_BROWSECONTROL;   break;
    case IDD_INSERTOBJECT : helpCode = BOLE_HELP_BROWSE;          break;
    case IDD_PASTESPECIAL : helpCode = BOLE_HELP_BROWSECLIPBOARD; break;
    case IDD_CONVERT      : helpCode = BOLE_HELP_CONVERT;         break;
    case IDD_EDITLINKS    : helpCode = BOLE_HELP_BROWSELINKS;     break;
    case IDD_CHANGEICON   : helpCode = BOLE_HELP_CHANGEICON;      break;
    case IDD_INSERTFILEBROWSE
                          : helpCode = BOLE_HELP_FILEOPEN;        break;
    case IDD_CHANGESOURCE : helpCode = BOLE_HELP_SOURCESET;       break;
    case IDD_CHANGEICONBROWSE
                          : helpCode = BOLE_HELP_ICONFILEOPEN;    break;
    default               : return;
  }

  PIBApplication pIApp = (PIBApplication) pCastToApp;
  pIApp->DialogHelpNotify (helpCode);
}


  // HELP for OLEUI standard dlgs


const TCHAR selectorPtr[] = TEXT("UnlikelyNameClash_HelpSelectorPtr");
const TCHAR offsetPtr[] = TEXT("UnlikelyNameClash_HelpOffsetPtr");


BOOL GetBOleHook (UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (msg == RegisterWindowMessage (SZOLEUI_MSG_HELP)) {
     return TRUE;
  } else
     return FALSE;
}

// MyWndProc is a MessageProc callback for hWndOwner to monitor for
// help messages for OleUI dialogs.
//
extern "C" LRESULT _loadds CALLBACK MyWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  WORD sel = (WORD)GetProp (hwnd, selectorPtr);
  WORD off = (WORD)GetProp (hwnd, offsetPtr);
  BOleService *pSvc = (BOleService *) MAKELONG(off,sel);

  if (GetBOleHook (msg, wParam, lParam))
    BOleHelpNotify ((DWORD) pSvc->GetApplication(), lParam);

  return ::CallWindowProc(pSvc->lpfnHelpWndProc, hwnd, msg, wParam, lParam);
}

// EnterBOleDialog has two basic jobs:
// 1. Install and remove the Windows hook which allows us to catch
//    F1 keystrokes while we're running a BOleUI dialog box.
// 2. Make sure we do the right enabling of modeless dialogs around
//    the system
//
void BOleService::ExitBOleDialog ()
{
  // If we're closing this dialog box, remove the Windows hook

  SetWindowLong (helpWnd, GWL_WNDPROC, (LONG)lpfnHelpWndProc);
  RemoveProp (helpWnd, selectorPtr);
  RemoveProp (helpWnd, offsetPtr);
  lpfnHelpWndProc = NULL;
  helpWnd = NULL;
  OnModalDialog (FALSE);
  return;
}

void BOleService::EnterBOleDialog (HWND hwndOwner, HHOOK *hHook, HTASK *hTask)
{
  if (lpfnHelpWndProc == NULL) {
     UINT    helpMsg = RegisterWindowMessage(SZOLEUI_MSG_HELP);
     WNDPROC wndProc = (WNDPROC)MyWndProc;
     helpWnd = hwndOwner;

     SetProp (helpWnd, selectorPtr, (HANDLE)HIWORD((DWORD)this));   //SELECTOROF(this));
     SetProp (helpWnd, offsetPtr, (HANDLE)LOWORD((DWORD)this));    //OFFSETOF(this));

     lpfnHelpWndProc =  (WNDPROC)GetWindowLong (helpWnd, GWL_WNDPROC);
     SetWindowLong (helpWnd, GWL_WNDPROC, (LONG)wndProc);
  }

  // Use the same thing Bolero clients do to en/disable modeless dialogs
  //
  OnModalDialog (TRUE);
}

