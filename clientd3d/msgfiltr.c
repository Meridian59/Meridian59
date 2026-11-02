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
#define WHOM_RESET  (WM_APP+6)

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

/*****************************************************************************/
/*
 * CopyTextToClipboard:  Place a null-terminated ANSI string on the clipboard.
 *   On success the clipboard owns the memory.
 */
static void CopyTextToClipboard(HWND hwnd, const char *text)
{
   if (text == NULL || text[0] == '\0')
      return;

   const size_t len = strlen(text) + 1;
   HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
   if (hMem == NULL)
      return;

   char *dest = (char *) GlobalLock(hMem);
   if (dest == NULL)
   {
      GlobalFree(hMem);
      return;
   }
   memcpy(dest, text, len);
   GlobalUnlock(hMem);

   if (!OpenClipboard(hwnd))
   {
      GlobalFree(hMem);
      return;
   }

   EmptyClipboard();
   if (SetClipboardData(CF_TEXT, hMem) == NULL)
      GlobalFree(hMem);
   CloseClipboard();
}

/*****************************************************************************/
/*
 * WhoListItemFromPoint:  Return the list index under the client point, or
 *   LB_ERR when the point is not on a player row.
 */
static int WhoListItemFromPoint(HWND hList, POINT clientPt)
{
   const DWORD result = (DWORD) SendMessage(hList, LB_ITEMFROMPOINT, 0, MAKELPARAM(clientPt.x, clientPt.y));
   if (HIWORD(result) != 0)
      return LB_ERR;

   const int index = (int) (short) LOWORD(result);
   if (index < 0)
      return LB_ERR;

   RECT rc;
   if (ListBox_GetItemRect(hList, index, &rc) == LB_ERR)
      return LB_ERR;
   if (!PtInRect(&rc, clientPt))
      return LB_ERR;

   return index;
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
      SetWindowLongPtr(hList, GWLP_USERDATA, OD_DRAWICON | OD_ONLYSEL | OD_COLORTEXT | OD_MAKEICONINDEX(0));
      
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

   case WM_CONTEXTMENU:
   {
      HWND hList = GetDlgItem(hDlg, IDC_USERLIST);
      if ((HWND) wParam != hList)
         break;

      POINT pt;
      pt.x = GET_X_LPARAM(lParam);
      pt.y = GET_Y_LPARAM(lParam);
      POINT client = pt;
      ScreenToClient(hList, &client);

      const int index = WhoListItemFromPoint(hList, client);
      if (index < 0)
         return TRUE;

      const LRESULT data = SendMessage(hList, LB_GETITEMDATA, (WPARAM) index, 0);
      
      if (data == LB_ERR || data == 0)
         return TRUE;
      
      object_node *obj = (object_node *) data;
      const ID playerId = obj->id;

      HMENU menu = CreatePopupMenu();
      if (menu == NULL)
         return TRUE;

      AppendMenu(menu, MF_STRING, IDS_WHO_COPYNAME, GetString(hInst, IDS_WHO_COPYNAME));
      
      const int cmd = TrackPopupMenu(menu,
                           TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                           pt.x, pt.y, 0, hDlg, NULL);
      DestroyMenu(menu);
      if (cmd == 0)
         return TRUE;

      obj = ItemListFindObject(hList, playerId);
      if (obj == NULL)
         return TRUE;

      if (cmd == IDS_WHO_COPYNAME)
      {
         char name[MAXSTRINGLEN];

         strncpy(name, LookupNameRsc(obj->name_res), sizeof(name) - 1);
         name[sizeof(name) - 1] = '\0';
         CopyTextToClipboard(hDlg, name);
      }
      
      return TRUE;
   }

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
