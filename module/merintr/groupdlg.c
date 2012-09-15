// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * groupdlg.c:  Handle group editing dialog.
 */

#include "client.h"
#include "merintr.h"

extern int num_groups;
extern char groups[MAX_NUMGROUPS][MAX_GROUPNAME + 1];       // Pointers to group names

// Colors for player names
static COLORREF logged_on_color  = PALETTERGB(255, 0, 0);

static HWND hGroupDialog;

static void GroupCommand(HWND hDlg, int ctrl_id, HWND hwndCtl, UINT codeNotify);
static Bool GetCurrentGroupName(HWND hDlg, char *group_name);
static BOOL CALLBACK GroupDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static long CALLBACK GroupEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static long CALLBACK PlayerEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static BOOL GroupListDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);
static void QuotePlayerName(char *name, char *buf);
/****************************************************************************/
/*
 * GroupConfigure:  Bring up dialog to configure groups.
 */
void GroupConfigure(void)
{
   DialogBox(hInst, MAKEINTRESOURCE(IDD_GROUP), cinfo->hMain, GroupDialogProc);
}
/*****************************************************************************/
/*
 * GroupDialogProc:  Dialog procedure for group dialog.
 */
BOOL CALLBACK GroupDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   int i, index;
   HWND hList, hCombo;
   list_type l;

   switch (message)
   {
   case WM_INITDIALOG:
      // Add groups to list box
      hCombo = GetDlgItem(hDlg, IDC_GROUPS);
      SetWindowFont(hCombo, GetFont(FONT_LIST), FALSE);
      for (i=0; i < num_groups; i++)
	 index = ComboBox_AddString(hCombo, groups[i]);

      // Add logged on users to list box
      hList = GetDlgItem(hDlg, IDC_LOGGEDON);
      SetWindowFont(hList, GetFont(FONT_LIST), FALSE);
      for (l = *(cinfo->current_users); l != NULL; l = l->next)
      {
	object_node *obj = (object_node *) (l->data);
	ListBox_AddString(hList, LookupNameRsc(obj->name_res));
      }

      PostMessage(hDlg, BK_CREATED, 0, 0);

      SetWindowFont(GetDlgItem(hDlg, IDC_GROUPMEMBERS), GetFont(FONT_LIST), FALSE);
      SetWindowFont(GetDlgItem(hDlg, IDC_ADDNAME), GetFont(FONT_EDIT), FALSE);
      SetWindowFont(GetDlgItem(hDlg, IDC_NEWGROUP), GetFont(FONT_EDIT), FALSE);
      SetWindowFont(GetDlgItem(hDlg, IDC_GROUPTELL), GetFont(FONT_EDIT), FALSE);

      Edit_LimitText(GetDlgItem(hDlg, IDC_NEWGROUP), MAX_GROUPNAME);
      Edit_LimitText(GetDlgItem(hDlg, IDC_ADDNAME), MAX_CHARNAME);
      Edit_LimitText(GetDlgItem(hDlg, IDC_GROUPTELL), MAXSAY);

       if (num_groups >= MAX_NUMGROUPS)
	 EnableWindow(GetDlgItem(hDlg, IDC_NEWGROUP), FALSE);

      hGroupDialog = hDlg;
      CenterWindow(hDlg, GetParent(hDlg));
      return TRUE;
      
   case BK_CREATED:
     hCombo = GetDlgItem(hDlg, IDC_GROUPS);
     ComboBox_SetCurSel(hCombo, 0);
     // Need this for some reason to simulate WM_COMMAND
     GroupCommand(hDlg, IDC_GROUPS, hCombo, CBN_SELCHANGE);
     return TRUE;

   HANDLE_MSG(hDlg, WM_COMMAND, GroupCommand);
   case WM_DRAWITEM:     // windowsx.h macro always returns FALSE
      return GroupListDrawItem(hDlg, (const DRAWITEMSTRUCT *)(lParam));

   case WM_DESTROY:
      hGroupDialog = NULL;
      return TRUE;
   }

   return FALSE;
}
/*****************************************************************************/
void GroupCommand(HWND hDlg, int ctrl_id, HWND hwndCtl, UINT codeNotify)
{
   int index, i;
   char group_name[MAX_GROUPNAME + 1], player_name[MAX_CHARNAME + 1], quoted_name[MAX_CHARNAME + 3];
   char say_string[MAXSAY + 1];
   UserGroup g;
   HWND hList, hwndFocus, hCombo;
   Bool legal_index;

   switch(ctrl_id)
   {
   case IDC_GROUPS:
     if (codeNotify != CBN_SELCHANGE)
       break;

     legal_index = GetCurrentGroupName(hDlg, group_name);

     EnableWindow(GetDlgItem(hDlg, IDC_DELGROUP), legal_index);
     EnableWindow(GetDlgItem(hDlg, IDC_ADDMEMBER), legal_index);
     EnableWindow(GetDlgItem(hDlg, IDC_DELMEMBER), legal_index);
     EnableWindow(GetDlgItem(hDlg, IDC_ADDNAME), legal_index);
     EnableWindow(GetDlgItem(hDlg, IDC_ADDNAMETEXT), legal_index);

     if (!legal_index)
       break;

     if (GroupLoad(group_name, &g) != GROUP_MATCH)
     {
       debug(("GroupCommand unable to load selected group %s\n", group_name));
       break;
     }

     hList = GetDlgItem(hDlg, IDC_GROUPMEMBERS);
     ListBox_ResetContent(hList);
     for (i=0; i < g.num_users; i++)
       ListBox_AddString(hList, g.names[i]);
     SetDlgItemText(hDlg, IDC_GROUPSTATUS, "");
     break;

   case IDC_DELGROUP:
     hCombo = GetDlgItem(hDlg, IDC_GROUPS);
     index = ComboBox_GetCurSel(hCombo);
     
     if (index == CB_ERR)
       break;
     
     ComboBox_GetText(hCombo, group_name, MAX_GROUPNAME);

     if (GroupDelete(group_name))
     {
       ComboBox_DeleteString(hCombo, index);
       if (ComboBox_GetCount(hCombo) == 0)
       {
	  ComboBox_ResetContent(hCombo);
	  ListBox_ResetContent(GetDlgItem(hDlg, IDC_GROUPMEMBERS));
       }
       else ComboBox_SetCurSel(hCombo, min(index, ComboBox_GetCount(hCombo) - 1));

       FORWARD_WM_COMMAND(hDlg, IDC_GROUPS, hCombo, CBN_SELCHANGE, GroupDialogProc);
 
       if (num_groups < MAX_NUMGROUPS)
	 EnableWindow(GetDlgItem(hDlg, IDC_NEWGROUP), TRUE);
       SetDlgItemText(hDlg, IDC_GROUPSTATUS, "");
     }
     break;

   case IDC_ADDMEMBER:
     if (!GetCurrentGroupName(hDlg, group_name))
       break;

     hList = GetDlgItem(hDlg, IDC_LOGGEDON);
     index = ListBox_GetCurSel(hList);
     if (index == LB_ERR)
       break;

     ListBox_GetText(hList, index, player_name);

     // Check for group too full
     if (ListBox_GetCount(GetDlgItem(hDlg, IDC_GROUPMEMBERS)) >= MAX_GROUPSIZE)
     {
        SetDlgItemText(hDlg, IDC_GROUPSTATUS, GetString(hInst, IDS_GROUPFULLDLG));
        break;
     }

     // Add quotes around player to take care of spaces
     QuotePlayerName(player_name, quoted_name);

     if (GroupAdd(group_name, quoted_name) == 0)
     {
        SetDlgItemText(hDlg, IDC_GROUPSTATUS, GetString(hInst, IDS_CANTADDNAMEDLG));
        break;
     }

     hList = GetDlgItem(hDlg, IDC_GROUPMEMBERS);
     ListBox_AddString(hList, player_name);     
     SetDlgItemText(hDlg, IDC_GROUPSTATUS, "");
     break;

   case IDC_DELMEMBER:
     if (!GetCurrentGroupName(hDlg, group_name))
       break;

     hList = GetDlgItem(hDlg, IDC_GROUPMEMBERS);
     index = ListBox_GetCurSel(hList);
     if (index == LB_ERR)
       break;

     ListBox_GetText(hList, index, player_name);

     QuotePlayerName(player_name, quoted_name);

     if (GroupDeleteNames(group_name, quoted_name) != 1)
     {
       debug(("Deleting name %s from group %s failed\n", player_name, group_name));
       break;
     }
     
     ListBox_DeleteString(hList, index);
     ListBox_SetCurSel(hList, min(index, ListBox_GetCount(hList) - 1));
     SetDlgItemText(hDlg, IDC_GROUPSTATUS, "");
     break;

   case IDOK:
     // Sent when Enter pressed on edit box
     hwndFocus = GetFocus();

     if (hwndFocus == GetDlgItem(hDlg, IDC_ADDNAME))
     {
       if (Edit_GetText(hwndFocus, player_name, MAX_CHARNAME) <= 0)
	 break;

       Edit_SetSel(hwndFocus, 0, -1);

       if (!GetCurrentGroupName(hDlg, group_name))
	 break;

       // Check for group too full
       if (ListBox_GetCount(GetDlgItem(hDlg, IDC_GROUPMEMBERS)) >= MAX_GROUPSIZE)
       {
	 SetDlgItemText(hDlg, IDC_GROUPSTATUS, GetString(hInst, IDS_GROUPFULLDLG));
	 break;
       }

       QuotePlayerName(player_name, quoted_name);

       if (GroupAdd(group_name, quoted_name) == 0)
       {
	 SetDlgItemText(hDlg, IDC_GROUPSTATUS, GetString(hInst, IDS_CANTADDNAMEDLG));
	 break;
       }
       
       hList = GetDlgItem(hDlg, IDC_GROUPMEMBERS);
       ListBox_AddString(hList, player_name);     
       SetDlgItemText(hDlg, IDC_GROUPSTATUS, "");
     }
     else if (hwndFocus == GetDlgItem(hDlg, IDC_NEWGROUP))
     {
       if (Edit_GetText(hwndFocus, group_name, MAX_GROUPNAME) <= 0)
	 break;

       Edit_SetSel(hwndFocus, 0, -1);

       if (!GroupNew(group_name))
       {	 
	 SetDlgItemText(hDlg, IDC_GROUPSTATUS, GetString(hInst, IDS_GROUPFAILEDDLG));
	 break;
       }
       hCombo = GetDlgItem(hDlg, IDC_GROUPS);
       index = ComboBox_AddString(hCombo, group_name);
       ComboBox_SetCurSel(hCombo, index);

       if (num_groups >= MAX_NUMGROUPS)
	 EnableWindow(GetDlgItem(hDlg, IDC_NEWGROUP), FALSE);

       FORWARD_WM_COMMAND(hDlg, IDC_GROUPS, hCombo, CBN_SELCHANGE, GroupDialogProc);
       SetDlgItemText(hDlg, IDC_GROUPSTATUS, "");
     }
     else if (hwndFocus == GetDlgItem(hDlg, IDC_GROUPTELL))
     {
       if (Edit_GetText(hwndFocus, say_string, MAXSAY) <= 0)
	 break;
       
       if (!GetCurrentGroupName(hDlg, group_name))
	 break;
       
       TellGroup(group_name, say_string);
       // Prevent auto-repeat from causing lots of messages to be sent
       Edit_SetText(hwndFocus, "");
     }
     break;

   case IDC_GROUP_DONE:
   case IDCANCEL:
     EndDialog(hDlg, IDOK);
     break;
   }
}
/****************************************************************************/
/*
 * GetCurrentGroupName:  Fill in group_name with name of currently selected
 *   group in group list box and return True, if a group is selected.
 *   Otherwise return False.
 */
Bool GetCurrentGroupName(HWND hDlg, char *group_name)
{
   int index;
   HWND hCombo;

   hCombo = GetDlgItem(hDlg, IDC_GROUPS);
   index = ComboBox_GetCurSel(hCombo);
   
   if (index == CB_ERR)
     return False;
   
   ComboBox_GetText(hCombo, group_name, MAX_GROUPNAME);
   return True;
}
/****************************************************************************/
void AbortGroupDialog(void)
{
   if (hGroupDialog != NULL)
     EndDialog(hGroupDialog, IDCANCEL);   
}
/*****************************************************************************/
/* 
 * GroupListDrawItem:  Message handler for item list boxes.  Return TRUE iff
 *   message is handled.
 */
BOOL GroupListDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
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
/*****************************************************************************/
/*
 * QuotePlayerName:  Return a quoted version of name in buf.  buf must be at least
 *   2 characters longer than name.
 */
void QuotePlayerName(char *name, char *buf)
{
  memcpy(buf + 1, name, strlen(name) + 1);
  buf[0] = '\"';
  strcat(buf, "\"");
}
