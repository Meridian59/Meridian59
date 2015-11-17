// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * admindlg.c:  The guts of the admin dialog.
 */

#include "client.h"
#include "admin.h"

#define MAX_CONSTANT 40                  // Maximum length of a constant's name

HWND hAdminDlg, hAdminMoveDlg;
Bool hidden;         // True when dialog exists but is hidden

static RECT   dlg_rect;                  // Screen position of dialog
static HWND   hInput, hText, hUserList;  // Dialog controls
HWND   hObjectList;                      // Dialog controls
char   command[MAX_ADMIN + 1];           // Admin command typed by user
int    current_obj;                      // Object we're currently displaying, if any

static ChildPlacement admin_controls[] = {
{ IDC_ADMINTEXT,   RDI_ALL },
{ IDC_TELEPORT,    RDI_BOTTOM | RDI_LEFT },

{ IDC_USERFRAME,   RDI_BOTTOM | RDI_LEFT },
{ IDC_INVALIDATE,  RDI_BOTTOM | RDI_LEFT },
{ IDC_USERS,       RDI_BOTTOM | RDI_LEFT },
{ IDC_SHOW,        RDI_BOTTOM | RDI_LEFT },
{ IDC_GOTO,        RDI_BOTTOM | RDI_LEFT },
{ IDC_RESET,       RDI_BOTTOM | RDI_LEFT },
{ IDC_RESCUE,      RDI_BOTTOM | RDI_LEFT },
{ IDC_REFRESH,     RDI_BOTTOM | RDI_LEFT },
{ IDC_OBJNUM,      RDI_BOTTOM | RDI_LEFT },
{ IDC_OBJLABEL,    RDI_BOTTOM | RDI_LEFT },

{ IDC_OBJECTFRAME, RDI_BOTTOM | RDI_LEFT | RDI_RIGHT },
{ IDC_OBJECTLIST,  RDI_BOTTOM | RDI_LEFT | RDI_RIGHT },
{ IDC_SHOWOBJ,     RDI_BOTTOM | RDI_RIGHT },
{ IDC_GOTOOBJ,     RDI_BOTTOM | RDI_RIGHT },
{ IDC_REFRESHOBJ,  RDI_BOTTOM | RDI_RIGHT },
{ IDC_MOVE,        RDI_BOTTOM | RDI_RIGHT },
{ IDC_SEND,        RDI_BOTTOM | RDI_RIGHT },
{ IDC_OWNERBUTTON, RDI_BOTTOM | RDI_RIGHT },
{ IDC_OWNER,       RDI_BOTTOM | RDI_RIGHT },
{ 0,               0 },   // Must end this way
};

#define OBJECT_NONE -1  // Signals that we're not displaying an object
#define OWNER_NONE -1   // Signals that object has no owner
static int owner;       // Owner of currently displayed object

/* local function prototypes */
static ID AdminGetCurrentUser(HWND hList);
static void AdminDlgCommand(HWND hDlg, int cmd_id, HWND hwndCtl, UINT codeNotify);
void AdminDlgConvertString(char *s, int len_s);
static Bool AdminGetString(HWND hParent, char *buf);
static BOOL CALLBACK AdminMoveDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK AdminStringDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
/****************************************************************************/
BOOL CALLBACK AdminDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   char *name;
   char new_str[MAX_HISTORY];
   char *str_ptr = new_str;
   int txtlen, new_len, str_len, index, add;
   char temp_command[MAX_ADMIN + 1];
   list_type l;
   MINMAXINFO *lpmmi;

   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, c->hMain);
      hInput      = GetDlgItem(hDlg, IDC_COMBO);
      hText       = GetDlgItem(hDlg, IDC_ADMINTEXT);
      hUserList   = GetDlgItem(hDlg, IDC_USERS);
      hObjectList = GetDlgItem(hDlg, IDC_OBJECTLIST);
      SendMessage(hDlg, BK_SETDLGFONTS, 0, 0);
      ComboBox_LimitText(hInput, MAX_ADMIN);
      Edit_LimitText(hText, MAX_HISTORY);

      /* Store dialog rectangle in case of resize */
      GetWindowRect(hDlg, &dlg_rect);

      SendMessage(hDlg, BK_UPDATEUSERS, 0, 0);
      
      hAdminDlg = hDlg;
      owner = OWNER_NONE;
      current_obj = OBJECT_NONE;
      EnableWindow(GetDlgItem(hAdminDlg, IDC_MOVE), FALSE);
      EnableWindow(GetDlgItem(hAdminDlg, IDC_SEND), FALSE);
      return TRUE;

   HANDLE_MSG(hDlg, WM_COMMAND, AdminDlgCommand);
      
   case WM_SIZE:
      ResizeDialog(hDlg, &dlg_rect, admin_controls);
      return TRUE;

   case WM_GETMINMAXINFO:
      lpmmi = (MINMAXINFO *) lParam;
      lpmmi->ptMinTrackSize.x = 400;
      lpmmi->ptMinTrackSize.y = 300;
      return 0;

   case BK_GOTTEXT:   /* lParam is text to add to edit box */
      // Add new text to text window
      str_ptr = (char *)lParam;
      AdminDlgConvertString(str_ptr, sizeof(new_str));
      WindowBeginUpdate(hText);

      /* If box is full, remove some beginning text */
      str_len = strlen(str_ptr);
      txtlen = Edit_GetTextLength(hText);
      if (txtlen + str_len >= MAX_HISTORY)
      {
         Edit_SetSel(hText, 0, txtlen + str_len + 1 - MAX_HISTORY);
         Edit_ReplaceSel(hText, "");
      }

      new_len = GetWindowTextLength(hText);
      
      // Add new text to end of box
      Edit_SetSel(hText, new_len, new_len);
      Edit_ReplaceSel(hText, str_ptr);
      
      /* Scroll new string into view */
      EditBoxScroll(hText, False);
      
      WindowEndUpdate(hText);

      AdminNewLine(str_ptr);
      return TRUE;
      
   case BK_SENDCMD:   /* lParam is command to send to server */
      str_ptr = (char *)lParam;
      RequestAdminCommand(str_ptr);
      Edit_SetText(hInput, str_ptr);
      ComboBox_SetEditSel(hInput, 0, -1);

      add = False;
      /* Add command to combo list box, if not the same as the prev. command */
      if (ComboBox_GetCount(hInput) == 0)
         add = True;
      else
      {
         ComboBox_GetLBText(hInput, 0, temp_command);
         if (stricmp(str_ptr, temp_command))
            add = True;
      }
      if (add)
         ComboBox_InsertString(hInput, 0, str_ptr);
      return TRUE;

   case BK_SETDLGFONTS:
      SetWindowFont(hInput, GetFont(FONT_INPUT), TRUE);
      SetWindowFont(hText, GetFont(FONT_ADMIN), TRUE);
      SetWindowFont(hUserList, GetFont(FONT_LIST), TRUE);
      SetWindowFont(hObjectList, GetFont(FONT_ADMIN), TRUE);
      return TRUE;

      HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORDLG, DialogCtlColor);

      HANDLE_MSG(hDlg, WM_INITMENUPOPUP, InitMenuPopupHandler);

   case BK_UPDATEUSERS:
      // Put logged-on users in user listbox
      WindowBeginUpdate(hUserList);
      ListBox_ResetContent(hUserList);
      
      /* Add users to list box.  Itemdata is object id. */
      for (l = *(c->current_users); l != NULL; l = l->next)
      {
         object_node *obj = (object_node *) (l->data);

         name = LookupRsc(obj->name_res);
         if (name != NULL)
         {
            index = ListBox_AddString(hUserList, name);
            ListBox_SetItemData(hUserList, index, obj->id);
         }
      }
      WindowEndUpdate(hUserList);
      break;

   case BK_RESETDATA:
      // Clear all object numbers and info we have cached
      owner = OWNER_NONE;
      current_obj = OBJECT_NONE;
      EnableWindow(GetDlgItem(hAdminDlg, IDC_MOVE), FALSE);
      EnableWindow(GetDlgItem(hAdminDlg, IDC_SEND), FALSE);
      AdminDisplayOwner(0);

      ListBox_ResetContent(hObjectList);
      ListBox_ResetContent(hUserList);

      if (hAdminMoveDlg != NULL)
         SendMessage(hAdminMoveDlg, WM_COMMAND, IDCANCEL, 0);
      break;

   case WM_ACTIVATE:
      if (wParam == 0)
         *c->hCurrentDlg = NULL;
      else *c->hCurrentDlg = hDlg;
         return TRUE;

   case WM_DESTROY:
      hAdminDlg = NULL;
      if (exiting)
         PostMessage(c->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
      return TRUE;
   }

   return FALSE;
}
/****************************************************************************/
/*
 * AdminDlgConvertString:  Convert \n's to CR-LF pairs.
 */
void AdminDlgConvertString(char *s, int len_s)
{
   int i = 0;

   while (s[i])
   {
      if (i >= len_s - 2
         || s[i] == '\0')
         break;

      if (s[i] == '\n')
      {
         // newlines get a carriage return inserted
         memmove(s + i + 2, s + i + 1, len_s - i - 2);
         s[i++] = '\r';
         s[i++] = '\n';
      }
      else if (s[i] == '\r')
         // carriage returns in the original are stripped to avoid making \r\r\n's
         memmove(s + i, s + i + 1, len_s - i - 1);
      else
         i++;
   }
}
/****************************************************************************/
/*
 * AdminDlgCommand:  Handle WM_COMMAND messages in the main admin dialog.
 */
void AdminDlgCommand(HWND hDlg, int cmd_id, HWND hwndCtl, UINT codeNotify)
{
   ID id;
   int index;
   char constant_name[MAX_CONSTANT + 1];

   switch(cmd_id)
   {
   /*** Controls in "users" box ***/
   case IDC_SHOW:
      id = AdminGetCurrentUser(hUserList);
      if (id != 0)
      {
	 sprintf(command, "show object %d", id);
	 SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      }
      break;
      
   case IDC_GOTO:
      id = AdminGetCurrentUser(hUserList);
      if (id != 0)
      {
	 sprintf(command, "send object %d admingotoobject what object %d", c->player->id, id);
	 SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      }
      break;
      
   case IDC_RESCUE:
      id = AdminGetCurrentUser(hUserList);
      if (id != 0)
      {
	 sprintf(command, "send object %d admingotosafety", id);
	 SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      }
      break;
      
   case IDC_REFRESH:
      SendMessage(hDlg, BK_UPDATEUSERS, 0, 0);
      break;
      
   case IDC_USERS:
      if (codeNotify != LBN_SELCHANGE)
	 break;
      id = AdminGetCurrentUser(hUserList);
      if (id != 0)
      {
	 sprintf(command, "%d", id);
	 SetWindowText(GetDlgItem(hAdminDlg, IDC_OBJNUM), command);
      }
      break;

   /*** Controls in "objects" box ***/
   case IDC_OBJECTLIST:
      if (codeNotify != LBN_DBLCLK)
	 break;

      index = ListBox_GetCurSel(hwndCtl);
      if (index != LB_ERR)
	 DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADMINVALUE), hDlg, AdminValueDialogProc, index);
      break;

   case IDC_OWNERBUTTON:
      if (owner == OWNER_NONE)
	 break;
      
      sprintf(command, "show object %d", owner);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      break;

   case IDC_SEND:
      if (current_obj == OWNER_NONE)
	 break;

      sprintf(command, "send object %d ", current_obj);
      SetDlgItemText(hAdminDlg, IDC_COMBO, command);
      SetFocus(GetDlgItem(hAdminDlg, IDC_COMBO));
      SendDlgItemMessage(hAdminDlg, IDC_COMBO, CB_SETEDITSEL, 0, MAKELONG(9999, 9999));
      break;

   case IDC_MOVE:
      if (owner == OWNER_NONE || current_obj == OBJECT_NONE)
	 break;
      
      DialogBox(hInst, MAKEINTRESOURCE(IDD_ADMINMOVE), hDlg, AdminMoveDialogProc);
      break;

   case IDC_SHOWOBJ:
      index = ListBox_GetCurSel(hObjectList);
      if (index != LB_ERR)
	 AdminShowProperty(index);
      break;

   case IDC_GOTOOBJ:
      if (current_obj != OBJECT_NONE)
      {
	 sprintf(command, "send object %d admingotoobject what object %d", 
		 c->player->id, current_obj);
	 SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      }
      break;

   case IDC_REFRESHOBJ:
      if (current_obj != OBJECT_NONE)
      {
	 sprintf(command, "show object %d", current_obj);
	 SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      }
      break;


   /*** Controls in main part of window ***/      
   case IDC_TELEPORT:
      if (!AdminGetString(hDlg, constant_name))
	 break;
      sprintf(command, "send object %d teleportto rid int %s", c->player->id, constant_name);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      break;

   case IDC_INVALIDATE:
      sprintf(command, "send object %d invalidatedata", c->player->id);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      break;

   case IDOK:
      Edit_GetText(hInput, command, MAX_ADMIN);
      
      if (!stricmp("quit", command))
      {
	 DestroyWindow(hDlg);
	 break;
      }
      
      SendMessage(hDlg, BK_SENDCMD, 0, (LPARAM) command);
      break;
      
   case IDCANCEL:
      ShowWindow(hDlg, SW_HIDE);
      hidden = True;
      break;
   }
}
/****************************************************************************/
/*
 * AdminGetCurrentUser:  Return ID of user currently selected in given listbox,
 *   or 0 if none.
 */
ID AdminGetCurrentUser(HWND hList)
{
   int index;
   
   index = ListBox_GetCurSel(hList);
   if (index == LB_ERR)
      return 0;

   return (ID) ListBox_GetItemData(hList, index);
}
/****************************************************************************/
/*
 * AdminDisplayOwner:  Display given object number as owner of current object,
 *   or a dash if the owner is nil.
 */
void AdminDisplayOwner(int num)
{
   char temp[MAXAMOUNT];
   HWND hwnd = GetDlgItem(hAdminDlg, IDC_OWNER);

   if (num != 0)
   {
      sprintf(temp, "%d", num);
      SetWindowText(hwnd, temp);
      owner = num;
   }
   else 
   {
      owner = OWNER_NONE;
      SetWindowText(hwnd, "---");
   }
   EnableWindow(GetDlgItem(hAdminDlg, IDC_MOVE), (num <= 0) ? FALSE : TRUE);
}
/****************************************************************************/
/*
 * AdminDisplayObject:  Set and display current object number and class.
 */
void AdminDisplayObject(int num, char *class_name)
{
   char temp[MAX_PROPERTYLEN];
   HWND hwnd = GetDlgItem(hAdminDlg, IDC_OBJECTFRAME);

   sprintf(temp, "Object %d (%s)", num, class_name);
   SetWindowText(hwnd, temp);
   current_obj = num;

   EnableWindow(GetDlgItem(hAdminDlg, IDC_SEND), (num <= 0) ? FALSE : TRUE);
}

/****************************************************************************/
/*
 * AdminMoveDialogProc:  Dialog procedure for moving an object.
 */
BOOL CALLBACK AdminMoveDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   char temp[MAXAMOUNT];
   char row[MAXAMOUNT + 1], col[MAXAMOUNT + 1], fine_row[MAXAMOUNT + 1], fine_col[MAXAMOUNT + 1];
   static HWND hRow, hCol, hFineRow, hFineCol;
   static int old_angle;
   int angle = 0;

   switch (message)
   {
   case WM_INITDIALOG:
      hAdminMoveDlg = hDlg;

      hRow = GetDlgItem(hDlg, IDC_ROWEDIT);
      hCol = GetDlgItem(hDlg, IDC_COLEDIT);
      hFineRow = GetDlgItem(hDlg, IDC_FINEROWEDIT);
      hFineCol = GetDlgItem(hDlg, IDC_FINECOLEDIT);
      Edit_LimitText(hRow, MAXAMOUNT);
      Edit_LimitText(hCol, MAXAMOUNT);
      Edit_LimitText(hFineRow, MAXAMOUNT);
      Edit_LimitText(hFineCol, MAXAMOUNT);

      sprintf(temp, "%d", current_obj);
      SetDlgItemText(hDlg, IDC_OBJNUM, temp);

      // Get object's coordinates and angle
      sprintf(command, "send object %d getrow", current_obj);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      sprintf(command, "send object %d getcol", current_obj);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      sprintf(command, "send object %d getangle", current_obj);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      sprintf(command, "send object %d getfinerow", current_obj);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      sprintf(command, "send object %d getfinecol", current_obj);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);

      CenterWindow(hDlg, GetParent(hDlg));
      break;

   case BK_GOTROW:
      SetWindowText(hRow, (char *) lParam);
      Edit_SetSel(hRow, 0, -1);

      SetDlgItemText(hDlg, IDC_ROW, (char *) lParam);
      break;

   case BK_GOTCOL:
      SetWindowText(hCol, (char *) lParam);
      Edit_SetSel(hCol, 0, -1);

      SetDlgItemText(hDlg, IDC_COL, (char *) lParam);
      break;

   case BK_GOTFINEROW:
      SetWindowText(hFineRow, (char *) lParam);
      Edit_SetSel(hFineRow, 0, -1);
      break;

   case BK_GOTFINECOL:
      SetWindowText(hFineCol, (char *) lParam);
      Edit_SetSel(hFineCol, 0, -1);
      break;

   case BK_GOTANGLE:
      old_angle = atoi((char *) lParam) * 8 / MAX_ANGLE;
      
      switch (old_angle)
      {
      case 0:  CheckDlgButton(hDlg, IDC_EAST, 1);         break;
      case 1:  CheckDlgButton(hDlg, IDC_SOUTHEAST, 1);    break;
      case 2:  CheckDlgButton(hDlg, IDC_SOUTH, 1);        break;
      case 3:  CheckDlgButton(hDlg, IDC_SOUTHWEST, 1);    break;
      case 4:  CheckDlgButton(hDlg, IDC_WEST, 1);         break;
      case 5:  CheckDlgButton(hDlg, IDC_NORTHWEST, 1);    break;
      case 6:  CheckDlgButton(hDlg, IDC_NORTH, 1);        break;
      case 7:  CheckDlgButton(hDlg, IDC_NORTHEAST, 1);    break;
      }
      break;

   case WM_COMMAND:
      switch (GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
	 // Set coordinates
	 Edit_GetText(hRow, row, MAXAMOUNT);
	 Edit_GetText(hCol, col, MAXAMOUNT);
	 Edit_GetText(hFineRow, fine_row, MAXAMOUNT);
	 Edit_GetText(hFineCol, fine_col, MAXAMOUNT);

	 sprintf(command, "send object %d somethingmoved what object %d "
		 "new_row int %s new_col int %s fine_row int %s fine_col int %s", 
		 owner, current_obj, row, col, fine_row, fine_col);
	 SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);

	 // Set angle if needed
	 if (IsDlgButtonChecked(hDlg, IDC_EAST))      angle = 0;
	 if (IsDlgButtonChecked(hDlg, IDC_SOUTHEAST)) angle = 1;
	 if (IsDlgButtonChecked(hDlg, IDC_SOUTH))     angle = 2;
	 if (IsDlgButtonChecked(hDlg, IDC_SOUTHWEST)) angle = 3;
	 if (IsDlgButtonChecked(hDlg, IDC_WEST))      angle = 4;
	 if (IsDlgButtonChecked(hDlg, IDC_NORTHWEST)) angle = 5;
	 if (IsDlgButtonChecked(hDlg, IDC_NORTH))     angle = 6;
	 if (IsDlgButtonChecked(hDlg, IDC_NORTHEAST)) angle = 7;

	 if (angle != old_angle)
	 {
	    sprintf(command, "send object %d somethingturned what object %d "
		    "new_angle int %d", owner, current_obj, angle * MAX_ANGLE / 8);
	    SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
	 }
	 

	 EndDialog(hDlg, IDOK);
	 return TRUE;

      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
      break;

   case WM_DESTROY:
      hAdminMoveDlg = NULL;
      return TRUE;
   }
   return FALSE;
}

/****************************************************************************/
/*
 * AdminGetString:  Bring up a dialog for user to enter a string.
 *   Return True iff user enters a string.
 *   hParent gives parent window for dialog.
 *   buf gets filled in with string; must have length at least MAX_CONSTANT.
 */
Bool AdminGetString(HWND hParent, char *buf)
{
   int retval;
   retval = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADMINSTRING), hParent, 
			   AdminStringDialogProc, (LPARAM) buf);
   
   if (retval == IDCANCEL)
      return False;
   return True;
}

/****************************************************************************/
/*
 * AdminStringDialogProc:  Dialog procedure for entering a string.
 *   Initial lParam is buffer to fill in.
 */
BOOL CALLBACK AdminStringDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   static char *buf;
   HWND hEdit;

   switch (message)
   {
   case WM_INITDIALOG:
      buf = (char *) lParam;

      hEdit = GetDlgItem(hDlg, IDC_STRING);
      Edit_LimitText(hEdit, MAX_CONSTANT);
      SetFocus(hEdit);
      CenterWindow(hDlg, GetParent(hDlg));
      break;
      
   case WM_COMMAND:
      hEdit = GetDlgItem(hDlg, IDC_STRING);
      switch (GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
	 Edit_GetText(hEdit, buf, MAX_CONSTANT);
	 EndDialog(hDlg, IDOK);
	 break;

      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
   }
   return FALSE;
}
