// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * guildhal.c:  Show dialog with guild halls available for sale.
 */

#include "client.h"
#include "merintr.h"

static int num_halls;
static GuildHall *halls;

extern HWND hGuildHallsDialog;

static BOOL CALLBACK GuildHallsDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static Bool ListViewGetCurrentData(HWND hList, int *index, int *data);
static int CALLBACK GuildHallCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
/****************************************************************************/
/*
 * GuildHallsReceived:  Got list of guild halls for sale.
 */
void GuildHallsReceived(WORD init_num_halls, GuildHall *init_halls)
{
   num_halls = init_num_halls;
   halls = init_halls;
   DialogBox(hInst, MAKEINTRESOURCE(IDD_GUILDHALLS), cinfo->hMain, GuildHallsDialogProc);
   SafeFree(halls);
}
/*****************************************************************************/
/*
 * GuildHallsDialogProc:  Dialog procedure for guild halls dialog.
 */
BOOL CALLBACK GuildHallsDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   HWND hList; 
   int i, index, len;
   ID guild_id;
   char str[MAXAMOUNT + 1];
   LV_COLUMN lvcol;
   LV_ITEM lvitem;
   NM_LISTVIEW *nm;
   LV_HITTESTINFO lvhit;
   char password[MAX_GUILD_NAME + 1], name[MAXRSCSTRING + 1];

   switch (message)
   {
   case WM_INITDIALOG:
      hGuildHallsDialog = hDlg;

      hList = GetDlgItem(hDlg, IDC_GUILDLIST);
      SetWindowFont(hList, GetFont(FONT_LIST), FALSE);

      ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT,
                                          LVS_EX_FULLROWSELECT);

      // Add column headings
      lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
      lvcol.pszText = GetString(hInst, IDS_GHEADER3);
      lvcol.cx      = 190;
      ListView_InsertColumn(hList, 0, &lvcol);
      lvcol.pszText = GetString(hInst, IDS_GHEADER1);
      lvcol.cx      = 50;
      ListView_InsertColumn(hList, 1, &lvcol);
      lvcol.pszText = GetString(hInst, IDS_GHEADER2);
      lvcol.cx      = 70;
      ListView_InsertColumn(hList, 2, &lvcol);
	   
      for (i=0; i < num_halls; i++)
	{
	  lvitem.mask = LVIF_TEXT | LVIF_PARAM;
	  lvitem.iItem = ListView_GetItemCount(hList);
	  lvitem.lParam = halls[i].id;

	  lvitem.iSubItem = 0;
	  lvitem.pszText = LookupNameRsc(halls[i].name_res); 
	  ListView_InsertItem(hList, &lvitem);

	  // Add subitems
	  lvitem.mask = LVIF_TEXT;

	  lvitem.iSubItem = 1;
	  sprintf(str, "%d", halls[i].cost);
	  lvitem.pszText = str;
	  ListView_SetItem(hList, &lvitem);

	  lvitem.iSubItem = 2;
	  sprintf(str, "%d", halls[i].rent);
	  lvitem.pszText = str;
	  ListView_SetItem(hList, &lvitem);
	}

      // Sort items
      ListView_SortItems(hList, GuildHallCompareProc, hList);

      Edit_LimitText(GetDlgItem(hDlg, IDC_GUILD_PASSWORD), MAX_GUILD_NAME);
      SetWindowFont(GetDlgItem(hDlg, IDC_GUILD_PASSWORD), GetFont(FONT_EDIT), FALSE);
      SetWindowFont(GetDlgItem(hDlg, IDC_GUILDNAME), GetFont(FONT_LIST), FALSE);
      CenterWindow(hDlg, GetParent(hDlg));
      return TRUE;
      
   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
         hList = GetDlgItem(hDlg, IDC_GUILDLIST);
         if (!ListViewGetCurrentData(hList, &index, (int *) &guild_id))
            return TRUE;
         
         len = Edit_GetText(GetDlgItem(hDlg, IDC_GUILD_PASSWORD), password, MAX_GUILD_NAME);
         if (len == 0)
         {
            ClientError(hInst, hDlg, IDS_PASSWORDBLANK);
            return TRUE;
         }
         
         RequestGuildRent(guild_id, password);
         EndDialog(hDlg, IDOK);
         return TRUE;
         
      case IDCANCEL:
         EndDialog(hDlg, IDCANCEL);
         return TRUE;
      }
      
   case WM_NOTIFY:
      if (wParam != IDC_GUILDLIST)
         return TRUE;
      
      nm = (NM_LISTVIEW *) lParam;

      switch (nm->hdr.code)
      {
      case NM_CLICK:
	 // If you click on an item, select it--why doesn't control work this way by default?
	 hList = GetDlgItem(hDlg, IDC_GUILDLIST);
	 GetCursorPos(&lvhit.pt);
	 ScreenToClient(hList, &lvhit.pt);
	 lvhit.pt.x = 0;
	 index = ListView_HitTest(hList, &lvhit);
	 if (index == -1)
	    break;

	 ListView_SetItemState(hList, index, 
			       LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	 return TRUE;

      case LVN_ITEMCHANGED:
	 // New item selected; get its name
	 hList = GetDlgItem(hDlg, IDC_GUILDLIST);
	 lvitem.mask = LVIF_STATE | LVIF_TEXT;
	 lvitem.stateMask = LVIS_SELECTED;
	 lvitem.iItem = nm->iItem;
	 lvitem.iSubItem = 0;
	 lvitem.pszText = name;
	 lvitem.cchTextMax = MAXRSCSTRING;
	 ListView_GetItem(hList, &lvitem);

	 if (!(lvitem.state & LVIS_SELECTED))
	    break;

	 SetDlgItemText(hDlg, IDC_GUILDNAME, lvitem.pszText);
	 return TRUE;
      }
      break;

   case WM_DESTROY:
      hGuildHallsDialog = NULL;
      return TRUE;
   }
   return FALSE;
}
/****************************************************************************/
/*
 * ListViewGetCurrentData:  Set index to index of currently selected item 
 *   in given list view control, and data to its lParam value.
 * Return True if a selected item is found, False otherwise.
 */
Bool ListViewGetCurrentData(HWND hList, int *index, int *data)
{
   int i, num;
   LV_ITEM lvitem;

   num = ListView_GetItemCount(hList);

   lvitem.mask = LVIF_STATE | LVIF_PARAM;
   lvitem.iSubItem = 0;
   lvitem.stateMask = LVIS_SELECTED;
   for (i=0; i < num; i++)
   {
      lvitem.iItem = i;
      ListView_GetItem(hList, &lvitem);
      
      if (lvitem.state & LVIS_SELECTED)
      {
	 *index = i;
	 *data = lvitem.lParam;
	 return True;
      }
   }
   return False;
}
/****************************************************************************/
/*
 * ListViewGetItemById:  Return index of item with given lParam value, or -1 if none.
 */
int ListViewGetItemById(HWND hList, int id)
{
  int i;
  LV_ITEM lvitem;

  for (i=0; i < ListView_GetItemCount(hList); i++)
    {
      lvitem.mask = LVIF_PARAM;
      lvitem.iItem = i;
      lvitem.iSubItem = 0;
      ListView_GetItem(hList, &lvitem);

      if (lvitem.lParam == id)
	return i;
    }
  return -1;
}
/****************************************************************************/
/*
 * GuildHallCompareProc:  Callback for sorting guild halls by price.
 */
int CALLBACK GuildHallCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  HWND hList;
  int index1, index2, cost1, cost2;
  char str[MAXAMOUNT + 1];

  hList = (HWND) lParamSort;
  
  index1 = ListViewGetItemById(hList, lParam1);
  index2 = ListViewGetItemById(hList, lParam2);
  if (index1 == -1 || index2 == -1)
    return 0;

  
  ListView_GetItemText(hList, index1, 1, str, MAXAMOUNT);
  cost1 = atoi(str);
  ListView_GetItemText(hList, index2, 1, str, MAXAMOUNT);
  cost2 = atoi(str);

  return (cost1 > cost2);
}
