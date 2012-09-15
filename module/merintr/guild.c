// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * guild.c:  Handle guild dialogs and commands.
 */

#include "client.h"
#include "merintr.h"

HWND hGuildConfigDialog;
HWND hGuildCreateDialog;
HWND hGuildHallsDialog;

GuildConfigDialogStruct *guild_info;

// Info for each tab of tabbed dialog
typedef struct {
   int     name;          // Resource id of name
   int     template_id;   // Resource id of dialog template
   DLGPROC dialog_proc;   // Dialog procedure for tab
} TabPage;

// Property sheet info, one per tab
TabPage tab_pages[] = {
   { IDS_GUILDMEMBERS, IDD_GUILDMEMBERS,       GuildMemberDialogProc, },
   { IDS_GUILDALLIES,  IDD_GUILDALLIES,        GuildAllyDialogProc, },
   { IDS_INVITE,       IDD_GUILDINVITE,        GuildInviteDialogProc, },
   { IDS_GUILDMASTER,  IDD_GUILDMASTER,        GuildMasterDialogProc, },
   { IDS_GUILDSHIELD,	IDD_GUILDSHIELD,		GuildShieldDialogProc, },
};

#define NUM_TAB_PAGES (sizeof(tab_pages) / sizeof(TabPage))

// Constants for indices into tab_pages
#define TAB_MEMBERS 0
#define TAB_ALLIES  1
#define TAB_INVITE  2
#define TAB_MASTER  3
#define TAB_SHIELD	4

static HWND hTab;      // Handle of tab control

static void GuildConfigCommand(HWND hDlg, int ctrl_id, HWND hwndCtl, UINT codeNotify);
static int CALLBACK GuildConfigSheetInit(HWND hDlg, UINT uMsg, LPARAM lParam);
/********************************************************************/
/*
 * GuildConfigInitInfo:  Got guild information from the server; display it in a dialog.
 */
void GuildConfigInitInfo(GuildConfigDialogStruct *info)
{
   int i;
   PROPSHEETHEADER psh;
   PROPSHEETPAGE psp[NUM_TAB_PAGES];

   guild_info = info;

   // Prepare property sheet header
   psh.dwSize = sizeof(PROPSHEETHEADER);
   psh.dwFlags = PSH_PROPSHEETPAGE | PSH_USECALLBACK | PSH_NOAPPLYNOW;
   psh.hwndParent = cinfo->hMain;
   psh.hInstance = hInst;
   psh.pszCaption = GetString(hInst, IDS_GUILDCONFIGTITLE);
   psh.nPages = NUM_TAB_PAGES;
   psh.nStartPage = 0;
   psh.ppsp = (LPCPROPSHEETPAGE) &psp;
   psh.pfnCallback = GuildConfigSheetInit;

   // Prepare property sheet pages
   for (i=0; i < NUM_TAB_PAGES; i++)
   {
      psp[i].dwSize = sizeof(PROPSHEETPAGE);
      psp[i].dwFlags = PSP_USETITLE;
      psp[i].hInstance = hInst;
      psp[i].pszTemplate = MAKEINTRESOURCE(tab_pages[i].template_id);
      psp[i].pfnDlgProc = tab_pages[i].dialog_proc;
      psp[i].pszTitle = MAKEINTRESOURCE(tab_pages[i].name);
      psp[i].lParam = i;
      psp[i].pfnCallback = NULL;
   }

   PropertySheet(&psh);   
}
/********************************************************************/
int CALLBACK GuildConfigSheetInit(HWND hDlg, UINT uMsg, LPARAM lParam)
{
   int flags, style;

   if (uMsg != PSCB_INITIALIZED)
      return 0;

   // Remove context-sensitive help button.
   //
   style = GetWindowLong(hDlg, GWL_EXSTYLE);
   SetWindowLong(hDlg, GWL_EXSTYLE, style & (~WS_EX_CONTEXTHELP));

   hGuildConfigDialog = hDlg;
   hTab = PropSheet_GetTabControl(hDlg);

   // Remove pages player shouldn't have.
   // Have to do this in right-to-left order, because it's by index.
   //
   flags = guild_info->flags;
   if (!(flags & GC_DISBAND))
   {
      PropSheet_RemovePage(hDlg, TAB_SHIELD, NULL);
      PropSheet_RemovePage(hDlg, TAB_MASTER, NULL);
   }
   if (!(flags & GC_INVITE))
   {
      PropSheet_RemovePage(hDlg, TAB_INVITE, NULL);
   }

   // Fiddle with property sheet buttons
   DestroyWindow(GetDlgItem(hDlg, IDOK));
 
   SetWindowText(GetDlgItem(hDlg, IDCANCEL), GetString(hInst, IDS_DONE));
   SetWindowText(hDlg, guild_info->name);

   return 0;
}

/*****************************************************************************/
/*
 * GuildResetData:  Called when client's data is reset; destroy guild config dialog
 *   since it has object numbers.
 */
void GuildResetData(void)
{
  AbortGuildDialogs();
}
/*****************************************************************************/
/*
 * AbortGuildDialogs:  Close guild dialogs when game is exited.
 */
void AbortGuildDialogs(void)
{
   if (hGuildConfigDialog != NULL)
   {
      PropSheet_PressButton(hGuildConfigDialog, PSBTN_CANCEL);
      // Hide in case another dialog pops up; for some reason the property sheet lingers awhile
      ShowWindow(hGuildConfigDialog, SW_HIDE);
   }

   if (hGuildCreateDialog != NULL)
      EndDialog(hGuildCreateDialog, IDCANCEL);   
   if (hGuildHallsDialog != NULL)
      EndDialog(hGuildHallsDialog, IDCANCEL);   
}
