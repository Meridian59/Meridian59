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

INT_PTR CALLBACK ProfanityDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

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
INT_PTR CALLBACK PreferencesDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   Bool toolbar_changed, lagbox_changed, temp;
   CommSettings *comm = &config.comm;
   int new_val;

   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, GetParent(hDlg));
      if (hPreferencesDialog != NULL)
      {
         EndDialog(hDlg, IDCANCEL);
         return FALSE;
      }
      hPreferencesDialog = hDlg;
      
      CheckDlgButton(hDlg, IDC_SCROLLLOCK, config.scroll_lock);
      CheckDlgButton(hDlg, IDC_DRAWNAMES, config.draw_names);
      CheckDlgButton(hDlg, IDC_TOOLTIPS, config.tooltips);
      CheckDlgButton(hDlg, IDC_PAIN, config.pain);
      CheckDlgButton(hDlg, IDC_INVNUM, config.inventory_num);
      CheckDlgButton(hDlg, IDC_SAFETY, config.aggressive);
      CheckDlgButton(hDlg, IDC_BOUNCE, config.bounce);
      CheckDlgButton(hDlg, IDC_TOOLBAR, config.toolbar);
      CheckDlgButton(hDlg, IDS_LATENCY0, config.lagbox);
      CheckDlgButton(hDlg, IDC_PROFANE, config.antiprofane);
      CheckDlgButton(hDlg, IDC_DRAWMAP, config.drawmap);
      CheckDlgButton(hDlg, IDC_MAP_ANNOTATIONS, config.map_annotations);

      CheckDlgButton(hDlg, IDC_MUSIC, config.play_music);
      CheckDlgButton(hDlg, IDC_SOUNDFX, config.play_sound);
      CheckDlgButton(hDlg, IDC_LOOPSOUNDS, config.play_loop_sounds);
      CheckDlgButton(hDlg, IDC_RANDSOUNDS, config.play_random_sounds);

      EnableWindow(GetDlgItem(hDlg, IDC_LOOPSOUNDS), IsDlgButtonChecked(hDlg, IDC_SOUNDFX));
      EnableWindow(GetDlgItem(hDlg, IDC_RANDSOUNDS), IsDlgButtonChecked(hDlg, IDC_SOUNDFX));

      CheckRadioButton(hDlg, IDC_TARGETHALO1, IDC_TARGETHALO3, config.halocolor + IDC_TARGETHALO1);

      CheckDlgButton(hDlg, IDC_COLORCODES, config.colorcodes);

      Trackbar_SetRange(GetDlgItem(hDlg, IDC_SOUND_VOLUME), 0, CONFIG_MAX_VOLUME, FALSE);
      Trackbar_SetRange(GetDlgItem(hDlg, IDC_MUSIC_VOLUME), 0, CONFIG_MAX_VOLUME, FALSE);
      Trackbar_SetPos(GetDlgItem(hDlg, IDC_SOUND_VOLUME), config.sound_volume);
      Trackbar_SetPos(GetDlgItem(hDlg, IDC_MUSIC_VOLUME), config.music_volume);
      
      return TRUE;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDCANCEL:
         EndDialog(hDlg, IDCANCEL);
         return TRUE;
         
      case IDC_SOUNDFX:
         EnableWindow(GetDlgItem(hDlg, IDC_LOOPSOUNDS), IsDlgButtonChecked(hDlg, IDC_SOUNDFX));
         EnableWindow(GetDlgItem(hDlg, IDC_RANDSOUNDS), IsDlgButtonChecked(hDlg, IDC_SOUNDFX));
         return TRUE;
         
      case IDC_PROFANESETTINGS:
         if (IDOK == DialogBox(hInst, MAKEINTRESOURCE(IDC_PROFANESETTINGS), hDlg, ProfanityDialogProc))
            CheckDlgButton(hDlg, IDC_PROFANE, TRUE);
         return TRUE;
         
      case IDOK:
         config.scroll_lock   = IsDlgButtonChecked(hDlg, IDC_SCROLLLOCK);
         config.draw_names    = IsDlgButtonChecked(hDlg, IDC_DRAWNAMES);
         config.tooltips      = IsDlgButtonChecked(hDlg, IDC_TOOLTIPS);
         config.pain          = IsDlgButtonChecked(hDlg, IDC_PAIN);
         config.inventory_num = IsDlgButtonChecked(hDlg, IDC_INVNUM);
         config.aggressive    = IsDlgButtonChecked(hDlg, IDC_SAFETY);
         config.bounce        = IsDlgButtonChecked(hDlg, IDC_BOUNCE);
         config.antiprofane   = IsDlgButtonChecked(hDlg, IDC_PROFANE);
         config.drawmap	      = IsDlgButtonChecked(hDlg, IDC_DRAWMAP);
         config.map_annotations = IsDlgButtonChecked(hDlg, IDC_MAP_ANNOTATIONS);
         temp                 = IsDlgButtonChecked(hDlg, IDC_TOOLBAR);
         toolbar_changed = (temp != config.toolbar);
         config.toolbar = temp;
         temp                 = IsDlgButtonChecked(hDlg, IDS_LATENCY0);
         lagbox_changed = (temp != config.lagbox);
         config.lagbox = temp;
         
         if (IsDlgButtonChecked(hDlg, IDC_MUSIC) != config.play_music)
            UserToggleMusic(config.play_music);
         config.play_music = IsDlgButtonChecked(hDlg, IDC_MUSIC);
         
         config.play_sound = IsDlgButtonChecked(hDlg, IDC_SOUNDFX);
         config.play_loop_sounds = IsDlgButtonChecked(hDlg, IDC_LOOPSOUNDS);
         config.play_random_sounds = IsDlgButtonChecked(hDlg, IDC_RANDSOUNDS);
         if (!config.play_sound)
            SoundAbort();

         new_val = Trackbar_GetPos(GetDlgItem(hDlg, IDC_MUSIC_VOLUME));
         if (new_val != config.music_volume)
         {
            config.music_volume = new_val;
            ResetMusicVolume();
         }

         // Don't need to dynamically update sound volume, because
         // looping sounds are updated as player moves around.
         config.sound_volume = Trackbar_GetPos(GetDlgItem(hDlg, IDC_SOUND_VOLUME));

         if( IsDlgButtonChecked( hDlg, IDC_TARGETHALO1 ) == BST_CHECKED )
            config.halocolor = 0;
         else if( IsDlgButtonChecked( hDlg, IDC_TARGETHALO2 ) == BST_CHECKED )
            config.halocolor = 1;
         else
            config.halocolor = 2;
         
         config.colorcodes = IsDlgButtonChecked(hDlg, IDC_COLORCODES);
         
         // Redraw main window to reflect new settings
         if (toolbar_changed || lagbox_changed)
         {
            ResizeAll();
         }
         else
         {
            InvalidateRect(hMain, NULL, TRUE);
            RedrawAll();
         }
         
         EditBoxSetNormalFormat();
         
         EndDialog(hDlg, IDOK);
         return TRUE;
      }
      break;
      
   case WM_DESTROY:
      hPreferencesDialog = NULL;
      return TRUE;
   }
   return FALSE;
}
/*****************************************************************************/
INT_PTR CALLBACK ProfanityDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   char term[MAXPROFANETERM+1];

   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, GetParent(hDlg));
      Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT1), MAXPROFANETERM);
      CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1+!config.ignoreprofane);
      CheckDlgButton(hDlg, IDC_CHECK1, config.extraprofane);
      SetFocus(GetDlgItem(hDlg, IDC_RADIO1+!config.ignoreprofane));
      return FALSE; // return TRUE unless we set the focus

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDC_BUTTON1:
	 GetDlgItemText(hDlg, IDC_EDIT1, term, sizeof(term));
	 AddProfaneTerm(term);
	 SetDlgItemText(hDlg, IDC_EDIT1, "");
	 SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
	 return TRUE;

      case IDC_BUTTON2:
	 GetDlgItemText(hDlg, IDC_EDIT1, term, sizeof(term));
	 RemoveProfaneTerm(term);
	 SetDlgItemText(hDlg, IDC_EDIT1, "");
	 SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
	 return TRUE;

      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;

      case IDOK:
	 config.ignoreprofane = IsDlgButtonChecked(hDlg, IDC_RADIO1);
	 config.extraprofane = IsDlgButtonChecked(hDlg, IDC_CHECK1);
	 RecompileAllProfaneExpressions();
	 EndDialog(hDlg, IDOK);
	 return TRUE;
      }
      break;
   }

   return FALSE;
}
