// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * guildaly.c:  Property page for guild alliances.
 */

#include "client.h"
#include "merintr.h"

extern HWND hGuildConfigDialog;
extern GuildConfigDialogStruct *guild_info;

static HWND hGuildAllyDialog;

static IDList allies;   // Guilds which consider us an ally
static IDList enemies;  // Guilds which consider us an enemy

static void GuildAllyShowStatus(HWND hDlg, HWND hList);
static void GuildAllyCommand(HWND hDlg, int ctrl_id, HWND hwndCtl, UINT codeNotify);
static void UpdateAllyLists(HWND hDlg);
/*****************************************************************************/
/*
 * GuildAllyDialogProc:  Dialog procedure for guild alliances dialog.
 */
BOOL CALLBACK GuildAllyDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
  int flags;

  switch (message)
  {
  case WM_INITDIALOG:
    hGuildAllyDialog = hDlg;
    flags = guild_info->flags;

    if (!(flags & GC_MAKE_ALLIANCE))
      {
	DestroyWindow(GetDlgItem(hDlg, IDC_ALLY));
	DestroyWindow(GetDlgItem(hDlg, IDC_ALLYLABEL));
      }
    
    if (!(flags & GC_END_ALLIANCE))
      DestroyWindow(GetDlgItem(hDlg, IDC_UNALLY));
    
    if (!(flags & GC_DECLARE_ENEMY))
      DestroyWindow(GetDlgItem(hDlg, IDC_ENEMY));
    
    if (!(flags & GC_END_ENEMY))
      DestroyWindow(GetDlgItem(hDlg, IDC_UNENEMY));

    SetWindowFont(GetDlgItem(hDlg, IDC_GUILDS), GetFont(FONT_LIST), FALSE);
    SetWindowFont(GetDlgItem(hDlg, IDC_ALLYGUILDS), GetFont(FONT_LIST), FALSE);
    SetWindowFont(GetDlgItem(hDlg, IDC_ENEMYGUILDS), GetFont(FONT_LIST), FALSE);

    RequestGuildList();
    return TRUE;
    
   HANDLE_MSG(hDlg, WM_COMMAND, GuildAllyCommand);

   case WM_DESTROY:
      hGuildAllyDialog = NULL;
      allies = IDListDelete(allies);
      enemies = IDListDelete(enemies);
      return TRUE;
  }
  return FALSE;
}
/*****************************************************************************/
void GuildAllyCommand(HWND hDlg, int ctrl_id, HWND hwndCtl, UINT codeNotify)
{
  int index, guild_id;
  HWND hList;

  switch (ctrl_id)
    {
    case IDC_ALLY:
      hList = GetDlgItem(hDlg, IDC_GUILDS);
      index = ListBox_GetCurSel(hList);
      if (index == LB_ERR)
	break;
      
      guild_id = ListBox_GetItemData(hList, index);
      RequestAlly(guild_id);

      UpdateAllyLists(hDlg);
      break;
      
    case IDC_UNALLY:
      hList = GetDlgItem(hDlg, IDC_ALLYGUILDS);
      index = ListBox_GetCurSel(hList);
      if (index == LB_ERR)
	break;
      
      guild_id = ListBox_GetItemData(hList, index);
      RequestEndAlly(guild_id);
      
      UpdateAllyLists(hDlg);
      break;
      
    case IDC_ENEMY:
      hList = GetDlgItem(hDlg, IDC_GUILDS);
      index = ListBox_GetCurSel(hList);
      if (index == LB_ERR)
	break;
      
      guild_id = ListBox_GetItemData(hList, index);
      RequestEnemy(guild_id);
      
      UpdateAllyLists(hDlg);
      break;
      
    case IDC_UNENEMY:
      hList = GetDlgItem(hDlg, IDC_ENEMYGUILDS);
      index = ListBox_GetCurSel(hList);
      if (index == LB_ERR)
	break;
      
      guild_id = ListBox_GetItemData(hList, index);
      RequestEndEnemy(guild_id);
      
      UpdateAllyLists(hDlg);
      break;
      
    case IDC_GUILDS:
    case IDC_ALLYGUILDS:
    case IDC_ENEMYGUILDS:
      if (codeNotify != LBN_SELCHANGE)
	break;
      
      GuildAllyShowStatus(hDlg, hwndCtl);
      break;
      
    }
}
/*****************************************************************************/
/*
 * GuildAllyShowStatus:  Given guild dialog and list box, display text
 *   showing how currently selected guild is allied with our guild.
 */
void GuildAllyShowStatus(HWND hDlg, HWND hList)
{
  int index, guild_id;
  HWND hStatus;
  char buf[MAX_GUILD_NAME + 100], guild_name[MAX_GUILD_NAME + 1];
  
  hStatus = GetDlgItem(hDlg, IDC_GUILDSTATUS);
  index = ListBox_GetCurSel(hList);
  if (index == LB_ERR)
    {
      SetWindowText(hStatus, "");
      return;
    }

  guild_id = ListBox_GetItemData(hList, index);
  ListBox_GetText(hList, index, guild_name);
  if (IDListMember(allies, guild_id))
     sprintf(buf, GetString(hInst, IDS_GUILDALLY), guild_name);
  else if (IDListMember(enemies, guild_id))
     sprintf(buf, GetString(hInst, IDS_GUILDENEMY), guild_name);
  else sprintf(buf, GetString(hInst, IDS_GUILDUNALIGNED), guild_name);
  SetWindowText(hStatus, buf);
}
/*****************************************************************************/
/*
 * GuildGotList:  Got list of all guilds in the system; show in dialog box.
 */
void GuildGotList(list_type guilds, IDList ally_guilds, IDList enemy_guilds, IDList other_allies,
		  IDList other_enemies)
{
   HWND hList;
   list_type l;
   int index;

   if (hGuildAllyDialog != NULL)
   {
      // Add guild names to list boxes
      for (l = guilds; l != NULL; l = l->next)
      {
	 GuildInfo *info = (GuildInfo *) (l->data);

	 // Skip our guild
	 if (info->id == guild_info->guild_id)
	   continue;

	 hList = GetDlgItem(hGuildAllyDialog, IDC_GUILDS);
	 
	 // Remove allies and enemies from this list of unaligned guilds
	 if (IDListMember(ally_guilds, info->id))
	    hList = GetDlgItem(hGuildAllyDialog, IDC_ALLYGUILDS);

	 if (IDListMember(enemy_guilds, info->id))
	    hList = GetDlgItem(hGuildAllyDialog, IDC_ENEMYGUILDS);
	    
	 index = ListBox_AddString(hList, info->name);
	 ListBox_SetItemData(hList, index, info->id);
      }
   }
   list_destroy(guilds);
   IDListDelete(ally_guilds);
   IDListDelete(enemy_guilds);
   allies = other_allies;
   enemies = other_enemies;
}

/*****************************************************************************/
/*
 * UpdateAllyLists:  Helper function to clear and update the alliance info
 */
void UpdateAllyLists(HWND hDlg)
{
   ListBox_ResetContent(GetDlgItem(hDlg, IDC_ALLYGUILDS));
   ListBox_ResetContent(GetDlgItem(hDlg, IDC_GUILDS));
   ListBox_ResetContent(GetDlgItem(hDlg, IDC_ENEMYGUILDS));
   RequestGuildList();

   return;
}
