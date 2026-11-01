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
#include "characternotes.h"

static HWND hWhoDialog = NULL;        /* Non-null if Who dialog is up */

extern player_info player;

#define WHOM_UPDATE (WM_APP+5)
#define WHOM_RESET  (WM_APP+6)
#define IDM_WRITE_CHARACTER_NOTE 1

/*****************************************************************************/
/*
 * IsAdmin:  Returns true when the player flags are an admin class.
 */
static bool IsAdmin(int flags)
{
  switch (GetPlayerFlags(flags))
  {
  case PF_DM:
  case PF_CREATOR:
  case PF_SUPER:
  case PF_EVENTCHAR:
    return true;
  default:
    return false;
  }
}

bool AddUserToIgnoreList(ID name)
{
  char *str = LookupNameRsc(name);
  if (str != NULL)
  {
    int i;
    for (i = 0; i < MAX_IGNORE_LIST; ++i)
      if (0 == config.ignore_list[i][0])
      {
        strncpy(config.ignore_list[i], str, MAX_CHARNAME);
        return true;
      }
  }

  // Do nothing.. ignore list is full.  Maybe should tell the user?
  return false;
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

bool IsUserInIgnoreList(ID name)
{
  char *str = LookupNameRsc(name);
  if (str != NULL)
  {
    int i;
    for (i = 0; i < MAX_IGNORE_LIST; ++i)
      if (0 == strcmp(config.ignore_list[i], str))
        return true;
  }
  return false;
}

/*****************************************************************************/
/*
 * ShouldHighlight:  Returns true when the who list should highlight this user.
 *   An admin is always highlighted.
 */
static bool ShouldHighlight(object_node *user)
{
  return IsAdmin(user->flags) || !IsUserInIgnoreList(user->name_res);
}

void UpdateWho(object_node* pUser, BOOL bAdded)
{
   debug(("UpdateWho\n"));
   if (hWhoDialog)
      SendMessage(hWhoDialog, WHOM_UPDATE, (WPARAM)bAdded, (LPARAM)pUser);
}

void ResetWhoList(list_type users)
{
   if (hWhoDialog)
      SendMessage(hWhoDialog, WHOM_RESET, 0, (LPARAM)users);
}

static LRESULT CALLBACK WhoListSubclassProc(HWND list, UINT message, WPARAM wParam,
                                            LPARAM lParam, UINT_PTR subclassId,
                                            DWORD_PTR refData)
{
   if (message == WM_CONTEXTMENU)
   {
      HWND dialog = (HWND)refData;
      POINT screenPoint;
      int item;

      screenPoint.x = GET_X_LPARAM(lParam);
      screenPoint.y = GET_Y_LPARAM(lParam);
      if (screenPoint.x == -1 && screenPoint.y == -1)
      {
         RECT itemRect;
         item = (int)SendMessage(list, LB_GETCARETINDEX, 0, 0);
         if (item == LB_ERR || SendMessage(list, LB_GETITEMRECT, item,
                                           (LPARAM)&itemRect) == LB_ERR)
            return 0;
         screenPoint.x = itemRect.left + (itemRect.right - itemRect.left) / 2;
         screenPoint.y = itemRect.top + (itemRect.bottom - itemRect.top) / 2;
         ClientToScreen(list, &screenPoint);
      }
      else
      {
         POINT clientPoint = screenPoint;
         DWORD hit;
         ScreenToClient(list, &clientPoint);
         hit = (DWORD)SendMessage(list, LB_ITEMFROMPOINT, 0,
                                  MAKELPARAM(clientPoint.x, clientPoint.y));
         if (HIWORD(hit))
            return 0;
         item = LOWORD(hit);
      }

      LRESULT itemData = SendMessage(list, LB_GETITEMDATA, item, 0);
      if (itemData == LB_ERR || itemData == 0)
         return 0;
      object_node *user = (object_node *)itemData;

      HMENU menu = CreatePopupMenu();
      if (!menu)
      {
         debug(("WhoListSubclassProc(): cannot create context menu\n"));
         return 0;
      }
      if (!AppendMenu(menu, MF_STRING, IDM_WRITE_CHARACTER_NOTE, "Write note"))
      {
         debug(("WhoListSubclassProc(): cannot add context menu item\n"));
         DestroyMenu(menu);
         return 0;
      }

      int command = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                                   screenPoint.x, screenPoint.y, 0, dialog, NULL);
      DestroyMenu(menu);
      if (command == IDM_WRITE_CHARACTER_NOTE)
      {
         std::string characterName = LookupNameRsc(user->name_res);
         EditCharacterNote(dialog, characterName.c_str());
         InvalidateRect(list, NULL, TRUE);
      }
      return 0;
   }

   return DefSubclassProc(list, message, wParam, lParam);
}

/*****************************************************************************/
INT_PTR CALLBACK WhoDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   HWND hList;
   IDList users;
   int num, i;
   bool selected, ignored;
   
   switch (message)
   {      
   case WM_INITDIALOG:
      CenterWindow(hDlg, GetParent(hDlg));
      users = (IDList) lParam;
      hList = GetDlgItem(hDlg, IDC_USERLIST);

      // Don't draw objects; too slow to load user bitmaps
      SetWindowLongPtr(hList, GWLP_USERDATA, OD_DRAWICON | OD_ONLYSEL | OD_COLORTEXT |
                       OD_CHARACTER_NOTES | OD_MAKEICONINDEX(0));
      
      /* Set contents of user list */
      ItemListSetContents(hList, users, false);

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
        if (ShouldHighlight(obj))
          ListBox_SetSel(hList, TRUE, i);
      }
      
      SetDlgItemInt(hDlg, IDC_NUMPLAYERS, num, FALSE);

      CheckDlgButton(hDlg, IDC_BROADCAST, config.no_broadcast);
      CheckDlgButton(hDlg, IDC_IGNOREALL, config.ignore_all);

      if (config.ignore_all)
         SetWindowLongPtr(hList, GWLP_USERDATA, GetWindowLongPtr(hList, GWLP_USERDATA) | OD_NEXTICON);

      hWhoDialog = hDlg;
      if (!SetWindowSubclass(hList, WhoListSubclassProc, 1, (DWORD_PTR)hDlg))
         debug(("WhoDialogProc(): could not install player list context menu\n"));
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
       i = ItemListAddItem(hList, pUser, -1, false);
       if (i != LB_ERR && ShouldHighlight(pUser))
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

   case WHOM_RESET:
   {
     list_type users = (list_type)lParam;
     hList = GetDlgItem(hDlg, IDC_USERLIST);
     ItemListSetContents(hList, users, false);
     num = ListBox_GetCount(hList);
     for (i = 0; i < num; i++)
     {
       object_node *obj = (object_node *) ListBox_GetItemData(hList, i);
       if (obj != NULL && ShouldHighlight(obj))
         ListBox_SetSel(hList, TRUE, i);
     }
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
          SetWindowLongPtr(hList, GWLP_USERDATA, GetWindowLongPtr(hList, GWLP_USERDATA) | OD_NEXTICON);
        else
          SetWindowLongPtr(hList, GWLP_USERDATA, GetWindowLongPtr(hList, GWLP_USERDATA) & ~OD_NEXTICON);
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
          
          // An admin is never added to the list
          if (IsAdmin(obj->flags))
            continue;

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
     RemoveWindowSubclass(GetDlgItem(hDlg, IDC_USERLIST), WhoListSubclassProc, 1);
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
  // The ignore list holds true names, which a message does not always carry.
  // A sender that is not a logged on user keeps the name in the message.
  object_node *sender = CurrentUser(sender_id);
  ID ignore_name = (sender == NULL) ? sender_name : sender->name_res;
  bool from_admin = (sender != NULL) && IsAdmin(sender->flags);

   // Always let non-player says through; check others for blocking
  if (say_type != SAY_RESOURCE && sender_id != player.id && 
      !from_admin &&
      (config.ignore_all || IsUserInIgnoreList(ignore_name) ||
       (say_type == SAY_EVERYONE && config.no_broadcast)))
  {
    // If we blocked a "send" to us, tell server so
    if (say_type == SAY_GROUP && 
        (IsUserInIgnoreList(ignore_name) || config.ignore_all))
      SendSayBlocked(sender_id);
    return;
  }
  
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
