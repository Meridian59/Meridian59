// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * vault.c:  Sort vaults. 
 */

#include "client.h"

static BOOL CALLBACK VaultDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static BOOL VaultInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
static void VaultCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);

static void HandleSelectionChange(HWND hdlg);
static void HandleNoItemSelected(HWND hDlg);
static void HandleFirstItemSelected(HWND hdlg);
static void HandleIntermediateItemSelected(HWND hdlg);
static void HandleLastItemSelected(HWND hdlg);

static void MoveToFirstPosition(HWND hDlg);
static void MoveToPreviousPosition(HWND hDlg);
static void MoveToNextPosition(HWND hDlg);
static void MoveToLastPosition(HWND hDlg);
static void MoveItem(int fromIndex, int toIndex);

static list_type GetItems(void);

static BOOL DoDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);


static VaultDialogStruct *info = NULL;

static HWND hwndDialog = NULL;   /* Non-NULL when vault dialog is displayed */

/*
 * Server just sent us list of objects that can be sorted. Bring up vault dialog.
 *   items is list of objects to sort (each of type item_object)
 */
void VaultList(object_node vault, list_type items)
{
   VaultDialogStruct dlg_info { vault.id, vault.name_res, items };
   if (hwndDialog == NULL)
   {
      DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VAULT), hMain, VaultDialogProc, (LPARAM)&dlg_info);
   }
   ObjectListDestroy(items);
}

/* Abort the vault dialog in case we are being disconnected due to inactivity. */
void AbortVaultDialog(void)
{
   if (hwndDialog == NULL)
   {
      return;
   }
   SendMessage(hwndDialog, WM_COMMAND, IDCANCEL, 0);
}

/*
 * Dialog procedure for user sorting items in a vault.
 *   lParam of the WM_INITDIALOG message should be a pointer to a VaultDialogStruct.
 */
BOOL CALLBACK VaultDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
      HANDLE_MSG(hDlg, WM_INITDIALOG, VaultInitDialog);
      HANDLE_MSG(hDlg, WM_COMMAND, VaultCommand);
      HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORDLG, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_DRAWITEM, DoDrawItem);

      case WM_COMPAREITEM:
         return ItemListCompareItem(hDlg, (const COMPAREITEMSTRUCT *)lParam);

      case WM_MEASUREITEM:
         ItemListMeasureItem(hDlg, (MEASUREITEMSTRUCT *)lParam);
         return TRUE;

      case WM_DESTROY:
         hwndDialog = NULL;
         return TRUE;
   }
   return FALSE;
}

/* Initialize dialog. Handle WM_INITDIALOG messages. */
BOOL VaultInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
   info = (VaultDialogStruct *)lParam;

   /* Draw objects. */
   CenterWindow(hDlg, GetParent(hDlg));
   info->hwndItemList = GetDlgItem(hDlg, IDC_ITEMLIST);
   SetWindowLong(info->hwndItemList, GWL_USERDATA, OD_DRAWOBJ);

   /* Add items to list box. */
   WindowBeginUpdate(info->hwndItemList);
   for (list_type node = info->vault_items; node != NULL; node = node->next)
   {
      item_object *item_obj = (item_object *)node->data;
      ItemListAddItem(info->hwndItemList, &item_obj->obj, -1, False);
   }
   WindowEndUpdate(info->hwndItemList);

   /* Set vault's name. */
   Edit_SetText(GetDlgItem(hDlg, IDC_VAULT), LookupNameRsc(info->vault_name));
   SetWindowFont(GetDlgItem(hDlg, IDC_VAULT), GetFont(FONT_LIST), FALSE);

   HandleNoItemSelected(hDlg);
   hwndDialog = hDlg;
   return TRUE;
}

/* Handle user interaction / WM_COMMAND messages. */
void VaultCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
   switch (id)
   {
      case IDC_ITEMLIST:
         if (codeNotify == LBN_DBLCLK)
         {
            RequestLook(ItemListGetId(hwndCtl));
            SetDescParams(hDlg, DESC_NONE);
         }
         else if (codeNotify == LBN_SELCHANGE)
         {
            HandleSelectionChange(hDlg);
         }
         break;

      case IDC_FIRST:
         MoveToFirstPosition(hDlg);
         break;

      case IDC_PREV:
         MoveToPreviousPosition(hDlg);
         break;

      case IDC_NEXT:
         MoveToNextPosition(hDlg);
         break;

      case IDC_LAST:
         MoveToLastPosition(hDlg);
         break;

      case IDOK:
      {
         list_type items = GetItems();
         if (items != NULL)
         {
            RequestVaultItems(info->vault_id, items);
            list_delete(items);
         }
         EndDialog(hDlg, IDOK);
         break;
      }

      case IDCANCEL:
         EndDialog(hDlg, IDCANCEL);
         break;
      }
}

/* User selected a different item in the item list. Enable or disable buttons appropriately. */
void HandleSelectionChange(HWND hDlg)
{
   int index = (int)ListBox_GetCurSel(info->hwndItemList);

   if (ListBox_GetCount(info->hwndItemList) < 2 || index < 0) // Not enough items or no item selected.
   {
      HandleNoItemSelected(hDlg);
   }
   else if (index == 0)
   {
      HandleFirstItemSelected(hDlg);
   }
   else if (index >= ListBox_GetCount(info->hwndItemList) - 1)
   {
      HandleLastItemSelected(hDlg);
   }
   else
   {
      HandleIntermediateItemSelected(hDlg);
   }
}

/* Disable all buttons. */
void HandleNoItemSelected(HWND hDlg)
{
   Button_Enable(GetDlgItem(hDlg, IDC_FIRST), false);
   Button_Enable(GetDlgItem(hDlg, IDC_PREV), false);
   Button_Enable(GetDlgItem(hDlg, IDC_NEXT), false);
   Button_Enable(GetDlgItem(hDlg, IDC_LAST), false);
}

/* Item can be moved downwards. */
void HandleFirstItemSelected(HWND hDlg)
{
   Button_Enable(GetDlgItem(hDlg, IDC_FIRST), false);
   Button_Enable(GetDlgItem(hDlg, IDC_PREV), false);
   Button_Enable(GetDlgItem(hDlg, IDC_NEXT), true);
   Button_Enable(GetDlgItem(hDlg, IDC_LAST), true);
}

/* Item can be moved upwards or downwards. */
void HandleIntermediateItemSelected(HWND hDlg)
{
   Button_Enable(GetDlgItem(hDlg, IDC_FIRST), true);
   Button_Enable(GetDlgItem(hDlg, IDC_PREV), true);
   Button_Enable(GetDlgItem(hDlg, IDC_NEXT), true);
   Button_Enable(GetDlgItem(hDlg, IDC_LAST), true);
}

/* Item can be moved upwards. */
void HandleLastItemSelected(HWND hDlg)
{
   Button_Enable(GetDlgItem(hDlg, IDC_FIRST), true);
   Button_Enable(GetDlgItem(hDlg, IDC_PREV), true);
   Button_Enable(GetDlgItem(hDlg, IDC_NEXT), false);
   Button_Enable(GetDlgItem(hDlg, IDC_LAST), false);
}

/* Move selected item to the top of the list. Do not select it again since the user probably wants to move more items to the top. */
void MoveToFirstPosition(HWND hDlg)
{
   MoveItem(ListBox_GetCurSel(info->hwndItemList), 0);
   HandleSelectionChange(hDlg);
}

/* Move selected item one position upwards. */
void MoveToPreviousPosition(HWND hDlg)
{
   int index = ListBox_GetCurSel(info->hwndItemList);
   MoveItem(index, index - 1);
   ListBox_SetCurSel(info->hwndItemList, index - 1);
   HandleSelectionChange(hDlg);
}

/* Move selected item one position downwards. */
void MoveToNextPosition(HWND hDlg)
{
   int index = ListBox_GetCurSel(info->hwndItemList);
   MoveItem(index, index + 1);
   ListBox_SetCurSel(info->hwndItemList, index + 1);
   HandleSelectionChange(hDlg);
}

/* Move selected item to the bottom of the list. Do not select it again since the user probably wants to move more items to the bottom. */
void MoveToLastPosition(HWND hDlg)
{
   MoveItem(ListBox_GetCurSel(info->hwndItemList), ListBox_GetCount(info->hwndItemList) - 1);
   HandleSelectionChange(hDlg);
}

/* Move item from one position to the other. */
void MoveItem(int fromIndex, int toIndex)
{
   object_node *obj = (object_node *)ListBox_GetItemData(info->hwndItemList, fromIndex);
   WindowBeginUpdate(info->hwndItemList);
   ItemListRemoveItem(info->hwndItemList, obj->id);
   ItemListAddItem(info->hwndItemList, obj, toIndex, False);
   WindowEndUpdate(info->hwndItemList);
}

/* Retrieve the item list sorted by the user. */
list_type GetItems(void)
{
   int num_entries = ListBox_GetCount(info->hwndItemList);
   list_type selection = NULL;

   // The list appears in reverse order on the server. Reverse it here, too, so that the order is correct.
   for (int index = num_entries - 1; index >= 0; --index)
   {
      object_node *obj = (object_node *)ListBox_GetItemData(info->hwndItemList, index);
      obj->temp_amount = obj->amount;  // Items without temp_amount are not sent to the server.
      selection = list_add_item(selection, obj);
   }
   return selection;
}

/* Draw an item. */
BOOL DoDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
   return ItemListDrawItem(hwnd, lpDrawItem);
}
