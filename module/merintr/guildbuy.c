// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * guildbuy.c:  Handle dialog for creating a guild.
 */

#include "client.h"
#include "merintr.h"

extern HWND hGuildCreateDialog;

typedef struct {
   int ctrl;       // Window control id
   int name;       // String resource id of rank name to set
} DefaultRankName;

static DefaultRankName default_ranks[] = {
{ IDC_MRANK1, IDS_MRANK1},
{ IDC_MRANK2, IDS_MRANK2},
{ IDC_MRANK3, IDS_MRANK3},
{ IDC_MRANK4, IDS_MRANK4},
{ IDC_MRANK5, IDS_MRANK5},
{ IDC_FRANK1, IDS_FRANK1},
{ IDC_FRANK2, IDS_FRANK2},
{ IDC_FRANK3, IDS_FRANK3},
{ IDC_FRANK4, IDS_FRANK4},
{ IDC_FRANK5, IDS_FRANK5},
};

static int num_default_ranks = (sizeof(default_ranks) / sizeof(DefaultRankName));

static int secret_cost, non_secret_cost;   // Cost of guilds with and without secret flag

static BOOL CALLBACK GuildCreateDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
/*****************************************************************************/
/*
 * GuildCreate:  Ask user for parameters of a newly created guild.
 */
void GuildCreate(int cost1, int cost2)
{
  non_secret_cost = cost1;
  secret_cost = cost2;
  DialogBox(hInst, MAKEINTRESOURCE(IDD_GUILDCREATE), cinfo->hMain, GuildCreateDialogProc);
}
/*****************************************************************************/
/*
 * GuildCreateDialogProc:  Dialog procedure for guild creation dialog.
 */
BOOL CALLBACK GuildCreateDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   int i, len;
   HWND hEdit;
   char name[MAX_GUILD_NAME + 1];
   char male_ranks[NUM_GUILD_RANKS][MAX_RANK_LENGTH + 1];
   char female_ranks[NUM_GUILD_RANKS][MAX_RANK_LENGTH + 1];

   switch (message)
   {
   case WM_INITDIALOG:
      hGuildCreateDialog = hDlg;

      hEdit = GetDlgItem(hDlg, IDC_GUILDNAME);
      Edit_LimitText(hEdit, MAX_GUILD_NAME);
      SetWindowFont(hEdit, GetFont(FONT_EDIT), FALSE);

      for (i=0; i < num_default_ranks; i++)
      {
	 hEdit = GetDlgItem(hDlg, default_ranks[i].ctrl);
	 SetWindowText(hEdit, GetString(hInst, default_ranks[i].name));
	 SetWindowFont(hEdit, GetFont(FONT_EDIT), FALSE);
	 Edit_LimitText(hEdit, MAX_RANK_LENGTH);
      }

      SetDlgItemInt(hDlg, IDC_GUILD_COST, non_secret_cost, FALSE);

      CenterWindow(hDlg, GetParent(hDlg));
      return TRUE;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDC_SECRET:
	if (IsDlgButtonChecked(hDlg, IDC_SECRET))
	  SetDlgItemInt(hDlg, IDC_GUILD_COST, secret_cost, TRUE);
	else SetDlgItemInt(hDlg, IDC_GUILD_COST, non_secret_cost, TRUE);
	return TRUE;
	
      case IDOK:
	 hEdit = GetDlgItem(hDlg, IDC_GUILDNAME);
	 len = Edit_GetText(hEdit, name, MAX_GUILD_NAME);

	 if (len == 0)
	 {
	    ClientError(hInst, hDlg, IDS_GUILDBLANK);
	    return TRUE;
	 }

	 for (i=0; i < NUM_GUILD_RANKS; i++)
	 {
	    hEdit = GetDlgItem(hDlg, IDC_MRANK1 + i);
	    Edit_GetText(hEdit, male_ranks[i], MAX_RANK_LENGTH);

	    hEdit = GetDlgItem(hDlg, IDC_FRANK1 + i);
	    Edit_GetText(hEdit, female_ranks[i], MAX_RANK_LENGTH);
	 }

	 RequestCreateGuild(name, male_ranks[0], female_ranks[0], 
			    male_ranks[1], female_ranks[1], male_ranks[2], female_ranks[2], 
			    male_ranks[3], female_ranks[3], male_ranks[4], female_ranks[4],
			    IsDlgButtonChecked(hDlg, IDC_SECRET));

	 EndDialog(hDlg, IDOK);
	 return TRUE;

      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }

   case WM_DESTROY:
      hGuildCreateDialog = NULL;
      return TRUE;
   }
   return FALSE;
}
