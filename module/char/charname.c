// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * charname.c:  Manage character name tabbed dialog.
 */

#include "client.h"
#include "char.h"

static HWND hTabPage;   // Handle of this tabbed dialog

extern HWND hMakeCharDialog;

/* Legal characters for character names */
static char legal_chars[] = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_ '!@$^&*()+=:[]{};/?|<>";

/********************************************************************/
BOOL CALLBACK CharNameDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   HWND hName, hDesc;

   switch (message)
   {
   case WM_INITDIALOG:
      // Fiddle with "name" edit box 
      hName = GetDlgItem(hDlg, IDC_NAME);
      SetWindowFont(hName, GetFont(FONT_INPUT), FALSE);
      Edit_LimitText(hName, MAX_CHARNAME);
      
      // Fiddle with "description" edit box 
      hDesc = GetDlgItem(hDlg, IDC_DESCRIPTION);
      SetWindowFont(hDesc, GetFont(FONT_INPUT), FALSE);
      Edit_LimitText(hDesc, MAX_DESCRIPTION - 1);

      hTabPage = hDlg;

      SetFocus(hName);

      CenterWindow(hMakeCharDialog, GetParent(hMakeCharDialog));
      
      break;

      HANDLE_MSG(hDlg, WM_COMMAND, CharTabPageCommand);      

   case WM_NOTIFY:
      switch (((LPNMHDR) lParam)->code)
      {
      case PSN_SETACTIVE:
	 SetFocus(GetDlgItem(hDlg, IDC_NAME));
	 break;

      case PSN_APPLY:
	 VerifySettings();
	 // Don't quit dialog until we hear result from server
	 SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
	 break;

      }
      return TRUE;      
   }
   return FALSE;
}
/********************************************************************/
/*
 * CharNameGetChoices:  Get name and description from dialog.
 */
void CharNameGetChoices(char *name, char *desc)
{
   GetDlgItemText(hTabPage, IDC_NAME, name, MAX_CHARNAME);
   GetDlgItemText(hTabPage, IDC_DESCRIPTION, desc, MAX_DESCRIPTION);
}
/********************************************************************/
/*
 * VerifyCharName:  Given a candidate character name, return a legal
 *   version of the name if possible, or NULL if the name is illegal.
 */
char *VerifyCharName(char *name)
{
   char *ptr;
   int len;
   
   // Skip leading whitespace
   while (*name == ' ')
      name++;

   // Erase tailing whitespace
   ptr = name + strlen(name) - 1;
   while (ptr > name && *ptr == ' ')
      ptr--;
   *(ptr + 1) = 0;

   len = strlen(name);
   if (len < MIN_CHARNAME || len > MAX_CHARNAME)
      return NULL;

   // Check that name is made up of legal characters
   if ((int) strspn(name, legal_chars) != len)
      return NULL;

   return name;
}
