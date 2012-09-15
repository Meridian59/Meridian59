// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * guildmtr.c:  Tabbed dialog page for guildmaster commands.
 */

#include "client.h"
#include "merintr.h"

extern HWND hGuildConfigDialog;
extern GuildConfigDialogStruct *guild_info;

static void GuildMasterCommand(HWND hDlg, int ctrl_id, HWND hwndCtl, UINT codeNotify);
/*****************************************************************************/
/*
 * GuildMasterDialogProc:  Dialog procedure for guildmaster commands.
 */
BOOL CALLBACK GuildMasterDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
  char password[MAX_GUILD_NAME + 1];
  HWND hEdit;

  switch (message)
  {
  case WM_INITDIALOG:

    if (!(guild_info->flags & GC_ABANDON))
      DestroyWindow(GetDlgItem(hDlg, IDC_ABANDON));

    if (guild_info->has_password)
      {
	hEdit = GetDlgItem(hDlg, IDC_GUILD_PASSWORD);
	Edit_SetText(hEdit, guild_info->password);
	Edit_LimitText(hEdit, MAX_GUILD_NAME);
	SetWindowFont(hEdit, GetFont(FONT_EDIT), FALSE);
	ShowWindow(GetDlgItem(hDlg, IDC_NOHALL), SW_HIDE);
      }
    else 
      {
	ShowWindow(GetDlgItem(hDlg, IDC_GUILD_PASSWORD), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_LABEL), SW_HIDE);
      }
    return TRUE;

   HANDLE_MSG(hDlg, WM_COMMAND, GuildMasterCommand);

  case WM_DESTROY:
    // See if password changed
    if (!guild_info->has_password)
      return TRUE;

    GetDlgItemText(hDlg, IDC_GUILD_PASSWORD, password, MAX_GUILD_NAME);
    if (stricmp(password, guild_info->password))
      RequestGuildPassword(password);
    return TRUE;
  }
  return FALSE;
}
/*****************************************************************************/
void GuildMasterCommand(HWND hDlg, int ctrl_id, HWND hwndCtl, UINT codeNotify)
{
  switch (ctrl_id)
    {
    case IDC_ABANDON:
       if (!AreYouSure(hInst, hDlg, NO_BUTTON, IDS_ABANDON))
          break;
       RequestAbandonHall();
       // Bring down dialog, because abandoning a hall can fail,
       // so we don't want to assume it succeeds and change the interface.
       PropSheet_PressButton(hGuildConfigDialog, PSBTN_CANCEL);
       break;
       
    case IDC_DISBAND:
       if (!AreYouSure(hInst, hDlg, NO_BUTTON, IDS_DISBAND))
          break;
       RequestDisband();
       PropSheet_PressButton(hGuildConfigDialog, PSBTN_CANCEL);
       break;
    }
}
