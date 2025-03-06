// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * maindlg.c:  Main menu dialogs, not associated with the game.
 */

#include "client.h"

#define MINPASSWORD 6       // Minimum password length

static HWND hPasswdDialog = NULL;
static HWND hPreferencesDialog = NULL;

// XXX Would be nice to load this from resource file, but that doesn't seem to be possible
static char EXE_filter[] = "Programs (*.exe)\0*.exe\0All files (*.*)\0*.*\0\0";
/*****************************************************************************/
INT_PTR CALLBACK PasswordDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   static HWND hOldPasswd, hNewPasswd1, hNewPasswd2;
   char oldpasswd[MAXPASSWORD + 1], newpasswd1[MAXPASSWORD + 1], newpasswd2[MAXPASSWORD + 1];
   char buf1[ENCRYPT_LEN + 1], buf2[ENCRYPT_LEN + 1];

   switch (message)
   {
   case WM_INITDIALOG:
      if (hPasswdDialog)
      {
         DestroyWindow(hDlg);
         return TRUE;
      }
      
      CenterWindow(hDlg, GetParent(hDlg));
      
      hOldPasswd = GetDlgItem(hDlg, IDC_OLDPASSWD);
      hNewPasswd1 = GetDlgItem(hDlg, IDC_NEWPASSWD1);
      hNewPasswd2 = GetDlgItem(hDlg, IDC_NEWPASSWD2);
      
      Edit_LimitText(hOldPasswd, MAXPASSWORD);
      Edit_LimitText(hNewPasswd1, MAXPASSWORD);
      Edit_LimitText(hNewPasswd2, MAXPASSWORD);

      SetWindowFont(hOldPasswd, GetFont(FONT_INPUT), FALSE);
      SetWindowFont(hNewPasswd1, GetFont(FONT_INPUT), FALSE);
      SetWindowFont(hNewPasswd2, GetFont(FONT_INPUT), FALSE);
      hPasswdDialog = hDlg;
      return TRUE;
      
   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
         /* User has pressed return on one of the edit boxes */
         if (GetFocus() == hOldPasswd)
         {
            SetFocus(hNewPasswd1);
            return TRUE;
         }
         
         if (GetFocus() == hNewPasswd1)
         {
            SetFocus(hNewPasswd2);
            return TRUE;
         }
         
         if (GetFocus() == hNewPasswd2)
            PostMessage(hDlg, WM_COMMAND, IDC_OK, 0);
         return TRUE;
         
      case IDC_OK:
         /* Send results to server */	 
         Edit_GetText(hOldPasswd, oldpasswd, MAXPASSWORD + 1);
         Edit_GetText(hNewPasswd1, newpasswd1, MAXPASSWORD + 1);
         Edit_GetText(hNewPasswd2, newpasswd2, MAXPASSWORD + 1);
         
         if (0 != strcmp(newpasswd1, newpasswd2))
         {
            ClientError(hInst, hDlg, IDS_PASSWDMATCH);
            return TRUE;
         }
         
         if (strlen(newpasswd1) < MINPASSWORD)
         {
            ClientError(hInst, hDlg, IDS_PASSWDLENGTH, MINPASSWORD);
            return TRUE;
         }
         
         // Recall this was the last attempt to change a password.
         // It's just stopping the auto-nagging feature from popping
         // up a dialog box later.
         //
         // The config.password is checked elsewhere, such as by
         // modules that need confirmation for drastic features.
         //
         // To improve the security, we only update these if we *think*
         // they're right about the password.  Not perfect, but we
         // never get word from the server that the password was right.
         //
         if (0 == strcmp(oldpasswd, config.password))
         {
            config.lastPasswordChange = time(NULL);
            strcpy(config.password, newpasswd1);
         }

         // Encrypt old and new passwords for the server.
         // It's up to the server to check if we are allowed to change
         // the password, based on the correct old password.
         //
         MDString(oldpasswd, (unsigned char *) buf1);
         buf1[ENCRYPT_LEN] = 0;
         MDString(newpasswd1, (unsigned char *) buf2);
         buf2[ENCRYPT_LEN] = 0;
         RequestChangePassword(buf1, buf2);
         
         hPasswdDialog = NULL;
         EndDialog(hDlg, IDOK);
         return TRUE;
         
      case IDCANCEL:
         hPasswdDialog = NULL;
         EndDialog(hDlg, IDCANCEL);
         return TRUE;
      }
      break;
   }
   return FALSE;
}
/****************************************************************************/
void AbortPasswordDialog(void)
{
   if (hPasswdDialog != NULL)
     EndDialog(hPasswdDialog, IDCANCEL);
}
/*****************************************************************************/
void AbortPreferencesDialog(void)
{
   if (hPreferencesDialog != NULL)
     EndDialog(hPreferencesDialog, IDCANCEL);
}
/*****************************************************************************/

