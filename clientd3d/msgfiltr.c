// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * msgfiltr.c:  Allow player to filter spoken messages from other players.
 *
 * This file also contains the "who" dialog, which displays a list of logged on players
 * and allows message filtering settings to be changed.
 */

#include "client.h"

static HWND hWhoDialog = NULL;        /* Non-null if Who dialog is up */

extern player_info player;

#define WHOM_UPDATE (WM_APP+5)

Bool AddUserToIgnoreList(ID name)
{
  char *str = LookupNameRsc(name);
  if (str != NULL)
  {
    int i;
    for (i = 0; i < MAX_IGNORE_LIST; ++i)
      if (0 == config.ignore_list[i][0])
      {
        strncpy(config.ignore_list[i], str, MAX_CHARNAME);
        return True;
      }
  }

  // Do nothing.. ignore list is full.  Maybe should tell the user?
  return False;
}

void RemoveUserFromIgnoreList(ID name)
{
  char *str = LookupNameRsc(name);
  if (str != NULL)
  {
    int i;
    for (i = 0; i < MAX_IGNORE_LIST; ++i)
      if (0 == strcmp(config.ignore_list[i], str))
        config.ignore_list[i][0] = 0;
  }
}

Bool IsUserInIgnoreList(ID name)
{
  char *str = LookupNameRsc(name);
  if (str != NULL)
  {
    int i;
    for (i = 0; i < MAX_IGNORE_LIST; ++i)
      if (0 == strcmp(config.ignore_list[i], str))
        return True;
  }
  return False;
}

void UpdateWho(object_node* pUser, BOOL bAdded)
{
   debug(("UpdateWho\n"));
   if (hWhoDialog)
      SendMessage(hWhoDialog, WHOM_UPDATE, (WPARAM)bAdded, (LPARAM)pUser);
}

/*****************************************************************************/
BOOL CALLBACK WhoDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   HWND hList;
   IDList users;
   int num, i;
   Bool selected, ignored;
   
   switch (message)
   {      
   case WM_INITDIALOG:
      CenterWindow(hDlg, GetParent(hDlg));
      users = (IDList) lParam;
      hList = GetDlgItem(hDlg, IDC_USERLIST);

      // Don't draw objects; too slow to load user bitmaps
      SetWindowLong(hList, GWL_USERDATA, OD_DRAWICON | OD_ONLYSEL | OD_COLORTEXT | OD_MAKEICONINDEX(0));
      
      /* Set contents of user list */
      ItemListSetContents(hList, users, False);

      // Highlight people we're not ignoring
      num = ListBox_GetCount(hList);
      for (i = 0; i < num; i++)
      {
        object_node *obj = (object_node *) ListBox_GetItemData(hList, i);
        if (obj == NULL)
        {
          debug(("WhoDialogProc got NULL user entry\n"));
          continue;
        }
        if (!IsUserInIgnoreList(obj->name_res))
          ListBox_SetSel(hList, TRUE, i);
      }
      
      SetDlgItemInt(hDlg, IDC_NUMPLAYERS, num, FALSE);

      CheckDlgButton(hDlg, IDC_BROADCAST, config.no_broadcast);
      CheckDlgButton(hDlg, IDC_IGNOREALL, config.ignore_all);

      if (config.ignore_all)
         SetWindowLong(hList, GWL_USERDATA, GetWindowLong(hList, GWL_USERDATA) | OD_NEXTICON);

      hWhoDialog = hDlg;
      return TRUE;

   case WHOM_UPDATE:
   {
     BOOL bAdded = (BOOL)wParam;
     object_node* pUser = (object_node*)lParam;
     hList = GetDlgItem(hDlg, IDC_USERLIST);
     
     if (!pUser)
     {
       debug(("WHOM_UPDATE NULL user\n"));
       return FALSE;
     }
     debug(("WHOM_UPDATE, %s user %i\n", (LPCTSTR)(bAdded?"adding":"removing"), pUser->id));
     if (bAdded)
     {
       i = ItemListAddItem(hList, pUser, -1, False);
       if (!IsUserInIgnoreList(pUser->name_res))
	       ListBox_SetSel(hList, TRUE, i);
     }
     else
     {
       ItemListRemoveItem(hList, pUser->id);
     }
     
     num = ListBox_GetCount(hList);
     SetDlgItemInt(hDlg, IDC_NUMPLAYERS, num, FALSE);
   }
   return TRUE;

   case WM_COMPAREITEM:
      return ItemListCompareItem(hDlg, (const COMPAREITEMSTRUCT *) lParam);
   case WM_MEASUREITEM:
      ItemListMeasureItem(hDlg, (MEASUREITEMSTRUCT *) lParam);
      return TRUE;
   case WM_DRAWITEM:
      return ItemListDrawItem(hDlg, (const DRAWITEMSTRUCT*)lParam);

   HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, DialogCtlColor);

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDC_IGNOREALL:
        hList = GetDlgItem(hDlg, IDC_USERLIST);
        if (IsDlgButtonChecked(hDlg, IDC_IGNOREALL))
          SetWindowLong(hList, GWL_USERDATA, GetWindowLong(hList, GWL_USERDATA) | OD_NEXTICON);
        else
          SetWindowLong(hList, GWL_USERDATA, GetWindowLong(hList, GWL_USERDATA) & ~OD_NEXTICON);
        InvalidateRect(hList, NULL, TRUE);
        return TRUE;
        
      case IDCANCEL:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
        
      case IDOK:
        hList = GetDlgItem(hDlg, IDC_USERLIST);
        
        // Modify ignore list to reflect changes
        num = ListBox_GetCount(hList);
        for (i=0; i < num; i++)
        {
          object_node *obj = (object_node *) ListBox_GetItemData(hList, i);
          
          if (obj == NULL)
          {
            debug(("WhoDialogProc got NULL user entry\n"));
            continue;
          }
          
          selected = (ListBox_GetSel(hList, i) > 0);
          ignored = IsUserInIgnoreList(obj->name_res);
          
          if (selected && ignored)
            RemoveUserFromIgnoreList(obj->name_res);
          
          if (!selected && !ignored)
            AddUserToIgnoreList(obj->name_res);
        }
        
        config.ignore_all   = IsDlgButtonChecked(hDlg, IDC_IGNOREALL);
        config.no_broadcast = IsDlgButtonChecked(hDlg, IDC_BROADCAST);
        
        RedrawAll();
        EndDialog(hDlg, IDOK);
        return TRUE;
      }
      break;
      
   case WM_DESTROY:
     hWhoDialog = NULL;
     return TRUE;
   }
   
   return FALSE;
}
/********************************************************************/
/*
 * MessageSaid:  Spoken text has arrived; determine if we should print it.
 *   say_type gives the type of the message (broadcast, said, etc.)
 *   sender gives name resource of sending object.
 */
void MessageSaid(ID sender_id, ID sender_name, BYTE say_type, char *message)
{
   // Always let non-player says through; check others for blocking
  if (say_type != SAY_RESOURCE && sender_id != player.id && 
      (config.ignore_all || IsUserInIgnoreList(sender_name) ||
       (say_type == SAY_EVERYONE && config.no_broadcast)))
  {
    // If we blocked a "send" to us, tell server so
    if (say_type == SAY_GROUP && 
        (IsUserInIgnoreList(sender_name) || config.ignore_all))
      SendSayBlocked(sender_id);
    return;
  }
   if (say_type == SAY_RESOURCE)
      DisplayServerMessage(message, GetColor(COLOR_BGD), 0);
   else
      DisplayServerMessage(message, GetColor(COLOR_MAINEDITFGD), 0);
}
/*****************************************************************************/
/*
 * AbortWhoDialog:  Close who dialog.
 */
void AbortWhoDialog(void)
{
   if (hWhoDialog != NULL)
      SendMessage(hWhoDialog, WM_COMMAND, IDCANCEL, 0);
}
