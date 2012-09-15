// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * guildmem.c:  Property page for guild membership.
 */

#include "client.h"
#include "merintr.h"

extern HWND hGuildConfigDialog;
extern GuildConfigDialogStruct *guild_info;

// # of members at rank 2 (there can only be MAX_RANK4 of these at a time)
static int num_rank4 = 0;
#define MAX_RANK4 2

static COLORREF logged_on_color  = PALETTERGB(255, 0, 0);

static void GuildMemberCommand(HWND hDlg, int ctrl_id, HWND hwndCtl, UINT codeNotify);
static BOOL GuildListDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);

/*****************************************************************************/
/*
 * GuildMemberDialogProc:  Dialog procedure for guild membership dialog.
 */
BOOL CALLBACK GuildMemberDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
  HWND hList;
  int i, index, flags;

  switch (message)
  {
  case WM_INITDIALOG:
    hList = GetDlgItem(hDlg, IDC_GUILDLIST);
    SetWindowFont(hList, GetFont(FONT_LIST), FALSE);
    SetWindowFont(GetDlgItem(hDlg, IDC_RANK), GetFont(FONT_LIST), FALSE);

    // Add guild members to list box
    num_rank4 = 0;
    for (i=0; i < guild_info->num_users; i++)
      {
	GuildMember *member = &guild_info->members[i];
	index = ListBox_AddString(hList, member->name);
	ListBox_SetItemData(hList, index, member);
	
	if (member->id == cinfo->player->id)
	  guild_info->rank = member->rank;
	
	if (member->id == guild_info->current_vote)
	  SetWindowText(GetDlgItem(hDlg, IDC_CURRENTVOTE), member->name);

	if (member->rank == 4)
	  num_rank4++;
      }

    if (guild_info->current_vote == 0)
      SetWindowText(GetDlgItem(hDlg, IDC_CURRENTVOTE), GetString(hInst, IDS_NOBODY));
    
    flags = guild_info->flags;
    
    // Destroy buttons for commands we don't have
    if (!(flags & GC_EXILE))
      DestroyWindow(GetDlgItem(hDlg, IDC_EXILE));
    
    if (!(flags & GC_RENOUNCE))
      DestroyWindow(GetDlgItem(hDlg, IDC_RENOUNCE));
    
    if (!(flags & GC_ABDICATE))
      {
	DestroyWindow(GetDlgItem(hDlg, IDC_ABDICATE));
	DestroyWindow(GetDlgItem(hDlg, IDC_ABDICATENAME));
      }
    
    if (!(flags & GC_VOTE))
      {
	DestroyWindow(GetDlgItem(hDlg, IDC_VOTE));
	DestroyWindow(GetDlgItem(hDlg, IDC_VOTENAME));
      }
    
    if (!(flags & GC_SET_RANK))
      DestroyWindow(GetDlgItem(hDlg, IDC_RANK));
    else DestroyWindow(GetDlgItem(hDlg, IDC_RANKDISPLAY));
    
    CenterWindow(hGuildConfigDialog, GetParent(hGuildConfigDialog));
    GuildShieldControl(GetDlgItem(hDlg, IDC_SHIELD));
    RequestGuildShieldSamples();
    RequestGuildShield();
    return TRUE;
    
   HANDLE_MSG(hDlg, WM_COMMAND, GuildMemberCommand);

	case WM_SHOWWINDOW:
		if (!(BOOL)wParam)
			RequestGuildShield();
		break;

   case WM_DRAWITEM:
      if (wParam == IDC_SHIELD)
      {
         GuildShieldControl(GetDlgItem(hDlg, IDC_SHIELD));
         GuildShieldDraw();
	 return TRUE;
      }
      return GuildListDrawItem(hDlg, (const DRAWITEMSTRUCT *)(lParam));
  }
  return FALSE;
}
/*****************************************************************************/
void GuildMemberCommand(HWND hDlg, int ctrl_id, HWND hwndCtl, UINT codeNotify)
{
  ID player_id;
  int i, index, flags;
  HWND hList, hCombo;
  char name[MAXUSERNAME];
  GuildMember *member;

  switch (ctrl_id)
    {
    case IDC_RENOUNCE:
      if (!AreYouSure(hInst, hDlg, NO_BUTTON, IDS_RENOUNCE))
	break;
      RequestRenounce();
      PropSheet_PressButton(hGuildConfigDialog, PSBTN_CANCEL);
      break;
            
    case IDC_EXILE:
      // Get currently selected user (if any), and send exile command
      hList = GetDlgItem(hDlg, IDC_GUILDLIST);
      index = ListBox_GetCurSel(hList);
      if (index == LB_ERR)
	break;
      
      member = (GuildMember *) ListBox_GetItemData(hList, index);
      
      player_id = member->id;
      ListBox_DeleteString(hList, index);
      RequestExile(player_id);
      break;
      
    case IDC_ABDICATE:
      // Get currently selected user (if any), and send abdicate command
      hList = GetDlgItem(hDlg, IDC_GUILDLIST);
      index = ListBox_GetCurSel(hList);
      if (index == LB_ERR)
	break;
      
      if (!AreYouSure(hInst, hDlg, NO_BUTTON, IDS_ABDICATE))
	break;
      
      member = (GuildMember *) ListBox_GetItemData(hList, index);
      player_id = member->id;
      RequestAbdicate(player_id);
      PropSheet_PressButton(hGuildConfigDialog, PSBTN_CANCEL);
      break;
      
    case IDC_VOTE:
      // Get currently selected user (if any), and send vote command
      hList = GetDlgItem(hDlg, IDC_GUILDLIST);
      index = ListBox_GetCurSel(hList);
      if (index == LB_ERR)
	break;
      
      member = (GuildMember *) ListBox_GetItemData(hList, index);
      player_id = member->id;
      RequestVote(player_id);
      
      // Display name of person voted for
      ListBox_GetText(hList, index, name);
      SetWindowText(GetDlgItem(hDlg, IDC_CURRENTVOTE), name);
      break;
      
    case IDC_GUILDLIST:
      if (codeNotify != LBN_SELCHANGE)
	break;
      
      hList = hwndCtl;
      index = ListBox_GetCurSel(hList);
      if (index == LB_ERR)
	break;
      
      member = (GuildMember *) ListBox_GetItemData(hList, index);
      player_id = member->id;
      flags = guild_info->flags;     
      
      if (flags & GC_EXILE)
	{
	  // Can't exile self, or someone at or above you
	  if (player_id == cinfo->player->id || member->rank >= guild_info->rank)
	    EnableWindow(GetDlgItem(hDlg, IDC_EXILE), FALSE);
	  else
	    EnableWindow(GetDlgItem(hDlg, IDC_EXILE), TRUE);	    
	}
      
      ListBox_GetText(hList, index, name);
      if (flags & GC_VOTE)
	SetWindowText(GetDlgItem(hDlg, IDC_VOTENAME), name);
      if (flags & GC_ABDICATE)
	SetWindowText(GetDlgItem(hDlg, IDC_ABDICATENAME), name);
      
      // If rank is settable, set up combo box, otherwise just display rank
      if (flags & GC_SET_RANK)
	{
	  // Add ranks to combo box
	  hCombo = GetDlgItem(hDlg, IDC_RANK);
	  ComboBox_ResetContent(hCombo);
	  for (i = 0; i < NUM_GUILD_RANKS; i++)
	    {
	      // Can't set people to ranks at or above your rank (1-based)
	      if (i == guild_info->rank - 1 && member->rank < guild_info->rank)
		break;
	      
	      if (member->gender == GUILD_MALE)
		ComboBox_AddString(hCombo, guild_info->male_ranks[i]);
	      else ComboBox_AddString(hCombo, guild_info->female_ranks[i]);
	    }
	  ComboBox_SetCurSel(hCombo, member->rank - 1);
	  
	  // Can't set rank of people at or above your rank
	  if (member->rank >= guild_info->rank)
	    EnableWindow(hCombo, FALSE);
	  else EnableWindow(hCombo, TRUE);
	}
      else
	{
	  if (member->gender == GUILD_MALE)
	    SetWindowText(GetDlgItem(hDlg, IDC_RANKDISPLAY), 
			  guild_info->male_ranks[member->rank - 1]);
	  else SetWindowText(GetDlgItem(hDlg, IDC_RANKDISPLAY), 
			     guild_info->female_ranks[member->rank - 1]);
	}
      
      break;
      
    case IDC_RANK:
      if (codeNotify != CBN_SELCHANGE)
	break;
      
      hList = GetDlgItem(hDlg, IDC_GUILDLIST);
      index = ListBox_GetCurSel(hList);
      if (index == LB_ERR)
	break;
      
      member = (GuildMember *) ListBox_GetItemData(hList, index);
      player_id = member->id;
      index = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_RANK));
      if (index == CB_ERR)
	break;

      // Can't have more than 2 players of rank 4
      if (index + 1 == 4)
	 if (num_rank4 >= MAX_RANK4)
	 {
	    index = 2;
	    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_RANK), index);
	 }
	 else num_rank4++;
      else 
	 if (member->rank == 4)
	    num_rank4--;

      member->rank = index + 1;
      RequestSetRank(player_id, index + 1);
      break;
    }
}
/*****************************************************************************/
/* 
 * GuildListDrawItem:  Message handler for item list boxes.  Return TRUE iff
 *   message is handled.
 */
BOOL GuildListDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
{
   char name[MAXUSERNAME];
   HBRUSH hbrush;
   RECT rcItem;

   switch (lpdis->itemAction)
   {
   case ODA_SELECT:
   case ODA_DRAWENTIRE:
      /* If box is empty, do nothing */
      if (lpdis->itemID == -1)
	 return TRUE;

      ListBox_GetText(lpdis->hwndItem, lpdis->itemID, name);

      // Draw logged on people in a different color
      if (FindPlayerByNameExact(name))
	SetTextColor(lpdis->hDC, logged_on_color);
      else if (lpdis->itemState & ODS_SELECTED)
	SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
      else
	SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));

      if (lpdis->itemState & ODS_SELECTED)
	hbrush = GetSysColorBrush(COLOR_HIGHLIGHT);
      else
	hbrush = GetSysColorBrush(COLOR_WINDOW);

      rcItem = lpdis->rcItem;

      FillRect(lpdis->hDC, &rcItem, hbrush);
      SetBkMode(lpdis->hDC, TRANSPARENT);

      rcItem.left += 2*GetSystemMetrics(SM_CXBORDER);
      DrawText(lpdis->hDC, name, strlen(name), &rcItem, DT_VCENTER | DT_LEFT);

      break;

   case ODA_FOCUS:
      DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
      break;
   }
   
   return TRUE;
}
