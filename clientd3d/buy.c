// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * buy.c:  Deal with buying objects
 */

#include "client.h"

#define MAX_PURCHASE 500

static BuyDialogStruct *info = NULL;

static HWND hwndBuyDialog = NULL;   /* Non-NULL when Buy dialog is displayed */

static char temp[MAXAMOUNT + 1];

static void BuyCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
static void WithdrawCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
static BOOL CALLBACK BuyDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static BOOL CALLBACK WithdrawalDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static BOOL BuyInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
static void UpdateCost(void);
static BOOL CostListDrawItem(const DRAWITEMSTRUCT *lpdis);
/****************************************************************************/
/*
 * AbortBuyDialog:  Close buy dialog
 */
void AbortBuyDialog(void)
{
   if (hwndBuyDialog == NULL)
      return;

   /* Simulate the user pressing cancel */
   SendMessage(hwndBuyDialog, WM_COMMAND, IDCANCEL, 0);
}

static BOOL DoDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
   /* Cheat--if item is about to be drawn, then perhaps list has been scrolled.
    * Tell ourselves to realign list boxes after we've finished drawing */
   PostMessage(hwnd, BK_ALIGNDLGS, 0, 0);
   /* Send message to appropriate list box handler */
   if (lpDrawItem->CtlID == IDC_ITEMLIST)
      return ItemListDrawItem(hwnd, lpDrawItem);
   else return CostListDrawItem(lpDrawItem);
}

static void DoAlignLists(void)
{
   /* Ensure that cost list shows same index at top that item list does */
   int index1 = ListBox_GetTopIndex(info->hwndItemList);
   int index2 = ListBox_GetTopIndex(info->hwndCostList);
   int index3 = ListBox_GetTopIndex(info->hwndQuanList);
   if ((index1 != index2) || (index1 != index3))
   {
      ListBox_SetTopIndex(info->hwndCostList, index1);
      ListBox_SetTopIndex(info->hwndQuanList, index1);
   }
}

/*****************************************************************************/
/*
 * BuyDialogProc:  Dialog procedure for user selecting one or more objects to buy
 *   from a list of objects.
 *   lParam of the WM_INITDIALOG message should be a pointer to a BuyDialogStruct.
 */
BOOL CALLBACK BuyDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
   HANDLE_MSG(hDlg, WM_INITDIALOG, BuyInitDialog);
   HANDLE_MSG(hDlg, WM_COMMAND, BuyCommand);
   HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, DialogCtlColor);
   HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, DialogCtlColor);
   HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, DialogCtlColor);
   HANDLE_MSG(hDlg, WM_CTLCOLORDLG, DialogCtlColor);
   HANDLE_MSG(hDlg, WM_DRAWITEM, DoDrawItem);

   case WM_COMPAREITEM:
      return ItemListCompareItem(hDlg, (const COMPAREITEMSTRUCT *) lParam);

   case WM_MEASUREITEM:
      ItemListMeasureItem(hDlg, (MEASUREITEMSTRUCT *) lParam);
      return TRUE;

   case BK_ALIGNDLGS:
      DoAlignLists();
      return TRUE;

   case WM_DESTROY:
      hwndBuyDialog = NULL;
      return TRUE;
   }

   return FALSE;
}
/*****************************************************************************/
/*
 * WithdrawalDialogProc:  Dialog procedure for user selecting one or more objects to 
 *   withdraw from a list of objects.
 *   lParam of the WM_INITDIALOG message should be a pointer to a BuyDialogStruct.
 */
BOOL CALLBACK WithdrawalDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
   HANDLE_MSG(hDlg, WM_INITDIALOG, BuyInitDialog); // does what we need!
   HANDLE_MSG(hDlg, WM_COMMAND, WithdrawCommand);
   HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, DialogCtlColor);
   HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, DialogCtlColor);
   HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, DialogCtlColor);
   HANDLE_MSG(hDlg, WM_CTLCOLORDLG, DialogCtlColor);
   HANDLE_MSG(hDlg, WM_DRAWITEM, DoDrawItem);

   case WM_COMPAREITEM:
      return ItemListCompareItem(hDlg, (const COMPAREITEMSTRUCT *) lParam);
   case WM_MEASUREITEM:
      ItemListMeasureItem(hDlg, (MEASUREITEMSTRUCT *) lParam);
      return TRUE;

   case BK_ALIGNDLGS:
      DoAlignLists();
      return TRUE;

   case WM_DESTROY:
      hwndBuyDialog = NULL;
      return TRUE;
   }

   return FALSE;
}
/*****************************************************************************/
/*
 * BuyInitDialog:  Handle WM_INITDIALOG messages.
 */ 
BOOL BuyInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam) 
{
   list_type l;
   int index;

   info = (BuyDialogStruct *) lParam;

   CenterWindow(hDlg, GetParent(hDlg));
   info->hwndItemList = GetDlgItem(hDlg, IDC_ITEMLIST);
   info->hwndCostList = GetDlgItem(hDlg, IDC_COSTLIST);
   info->hwndQuanList = GetDlgItem(hDlg, IDC_QUANLIST);
   info->hwndCost = GetDlgItem(hDlg, IDC_COST);

   // Draw objects in owner-drawn list box
   SetWindowLong(info->hwndItemList, GWL_USERDATA, OD_DRAWOBJ);
   SetWindowLong(info->hwndCostList, GWL_USERDATA, OD_DRAWOBJ);
   SetWindowLong(info->hwndQuanList, GWL_USERDATA, OD_DRAWOBJ);

   /* Add items & costs to list boxes */
   WindowBeginUpdate(info->hwndItemList);
   WindowBeginUpdate(info->hwndCostList);
   WindowBeginUpdate(info->hwndQuanList);
   for (l = info->items; l != NULL; l = l->next)
   {
      DWORD amount = 1;
      DWORD cost;
      buy_object *buy_obj = (buy_object *) l->data;
      index = ItemListAddItem(info->hwndItemList, &buy_obj->obj, -1, False);
      if (IsNumberObj(buy_obj->obj.id))
	 amount = buy_obj->obj.amount;
      else
	 amount = 1;
      cost = buy_obj->cost; // this WAS for a group: buy_obj->cost / amount;
      sprintf(temp, "%d", cost);
      ListBox_InsertString(info->hwndCostList, index, temp);
      ListBox_SetItemData(info->hwndCostList, index, cost);
      ListBox_InsertString(info->hwndQuanList, index, " ");
      ListBox_SetItemData(info->hwndQuanList, index, 0);
   }
   WindowEndUpdate(info->hwndCostList);
   WindowEndUpdate(info->hwndItemList);
   WindowEndUpdate(info->hwndQuanList);

   /* Set seller's name */
   Edit_SetText(GetDlgItem(hDlg, IDC_SELLER), 
		LookupNameRsc(info->seller_name));
   SetWindowFont(GetDlgItem(hDlg, IDC_SELLER), GetFont(FONT_LIST), FALSE);

   hwndBuyDialog = hDlg;
   
   return TRUE;
}

static void ClickOnQuantity(HWND hDlg, BOOL bLimitToAmount)
{
   int amount;
   char temp[16];

   int index = (int)ListBox_GetCurSel(info->hwndQuanList);
   object_node *obj = (object_node *) ListBox_GetItemData(info->hwndItemList, index);
   int currentAmount = (int)ListBox_GetItemData(info->hwndQuanList, index);
   if (ListBox_GetItemData(info->hwndQuanList, index) > 0)
   {
      //WindowBeginUpdate(info->hwndItemList);
      //WindowBeginUpdate(info->hwndQuanList);
      if (IsNumberObj(obj->id))
      {
	 MEASUREITEMSTRUCT m;
	 int startAmount = currentAmount;
	 int maxAmount = MAX_PURCHASE;
	 if (bLimitToAmount)
	    maxAmount = obj->amount;
	 if (currentAmount == 0)
	    startAmount = obj->amount;
	 /* Place amount dialog just beneath selected item */
	 ItemListMeasureItem(info->hwndQuanList, &m);
	 // Force highlight on (we are editing it)
	 ListBox_SetSel(info->hwndItemList,TRUE,index);
	 if(InputNumber(hDlg,info->hwndQuanList,
	    0,(index - ListBox_GetTopIndex(info->hwndQuanList) + 1) * (m.itemHeight - 1),
	    &amount,startAmount,1,maxAmount))
	 {
	    ListBox_DeleteString(info->hwndQuanList,index);
	    if (amount > 0)
	    {
	       sprintf(temp, "%d", amount);
	       ListBox_InsertString(info->hwndQuanList, index, temp);
	    }
	    else
	    {
	       ListBox_InsertString(info->hwndQuanList,index," ");
	    }
	    ListBox_SetItemData(info->hwndQuanList, index, amount);
	 }
	 // reset highlight based on quanity - off if zero, on otherwise
	 ListBox_SetSel(info->hwndItemList,
	    (ListBox_GetItemData(info->hwndQuanList,index) > 0),
	    index);
      }
      else
      {
	 ListBox_DeleteString(info->hwndQuanList,index);
	 if (currentAmount == 0)
	 {
	    amount = 1;
	    ListBox_InsertString(info->hwndQuanList,index,"1");
	 }
	 else
	 {
	    amount = 0;
	    ListBox_InsertString(info->hwndQuanList,index," ");
	 }
	 ListBox_SetItemData(info->hwndQuanList,index,amount);
	 ListBox_SetSel(info->hwndItemList,(amount > 0),index);
	 ListBox_SetCurSel(info->hwndQuanList,-1);
      }
      //WindowEndUpdate(info->hwndItemList);
      //WindowEndUpdate(info->hwndQuanList);
      UpdateCost();
   }
}

static void HandleSelectionChange(void)
{
   int amount;
   char temp[16];

   int index = (int)ListBox_GetCurSel(info->hwndItemList);
   object_node *obj = (object_node *) ListBox_GetItemData(info->hwndItemList, index);

   WindowBeginUpdate(info->hwndQuanList);
   ListBox_DeleteString(info->hwndQuanList,index);
   if (ListBox_GetSel(info->hwndItemList,index))
   {
      if (IsNumberObj(obj->id))
	 amount = obj->amount;
      else
	 amount = 1;
      sprintf(temp, "%d", amount);
   }
   else
   {
      amount = 0;
      strcpy(temp," ");
   }
   DoAlignLists();
   ListBox_InsertString(info->hwndQuanList,index,temp);
   ListBox_SetItemData(info->hwndQuanList,index,amount);
   WindowEndUpdate(info->hwndQuanList);
   UpdateCost();
}

static list_type GetItems(void)
{
   /* Get user's selection(s) */
   int num_entries = ListBox_GetCount(info->hwndItemList);
   list_type selection = NULL;
   int index;
   
   for (index = 0; index < num_entries; index++)
   {
      /* If item is selected, add to selection list, else free */
      object_node *obj = (object_node *) ListBox_GetItemData(info->hwndItemList, index);
      if (ListBox_GetSel(info->hwndItemList, index) > 0)
      {
	 DWORD amount = (DWORD)ListBox_GetItemData(info->hwndQuanList,index);
	 obj->temp_amount = amount;
	 selection = list_add_item(selection, obj);
      }
   }
   return selection;
}

/*****************************************************************************/
/*
 * BuyCommand:  Handle WM_COMMAND messages.
 */ 
void BuyCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify) 
{
   list_type selection;

   switch(id)
   {
   case IDC_QUANLIST:
      if (codeNotify == LBN_SELCHANGE)
	 ClickOnQuantity(hDlg,FALSE);
      break;
   case IDC_ITEMLIST:
      if (codeNotify == LBN_DBLCLK)
      {
	 RequestLook(ItemListGetId(hwndCtl));
	 SetDescParams(hDlg, DESC_NONE);
      }
      if (codeNotify == LBN_SELCHANGE)
	 HandleSelectionChange();
      break;

   case IDOK:
      selection = GetItems();
      /* Send list of selected items to server */
      if (selection != NULL)
	 RequestBuyItems(info->seller_id, selection);
	 
      list_delete(selection);
      EndDialog(hDlg, IDOK);
      break;
      
   case IDCANCEL:
      EndDialog(hDlg, IDCANCEL);
      break;
   }
}
/*****************************************************************************/
/*
 * WithdrawCommand:  Handle WM_COMMAND messages.
 */ 
void WithdrawCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify) 
{
   list_type selection;

   switch(id)
   {
   case IDC_QUANLIST:
      if (codeNotify == LBN_SELCHANGE)
	 ClickOnQuantity(hDlg,TRUE);
      break;
   case IDC_ITEMLIST:
      if (codeNotify == LBN_DBLCLK)
      {
	 RequestLook(ItemListGetId(hwndCtl));
	 SetDescParams(hDlg, DESC_NONE);
      }
      if (codeNotify == LBN_SELCHANGE)
	 HandleSelectionChange();
      break;

   case IDOK:
      selection = GetItems();
      /* Send list of selected items to server */
      if (selection != NULL)
	 RequestWithdrawalItems(info->seller_id, selection);
	 
      list_delete(selection);
      EndDialog(hDlg, IDOK);
      break;
      
   case IDCANCEL:
      EndDialog(hDlg, IDCANCEL);
      break;
   }
}
/************************************************************************/
/*
 * UpdateCost:  Recalculate total cost of bought items when item an item
 *   changes selection state.
 */
void UpdateCost(void)
{
   int num, i;

   info->cost = 0;
   num = ListBox_GetCount(info->hwndItemList);
   for (i=0; i < num; i++)
   {
      int quantity = (int)ListBox_GetItemData(info->hwndQuanList,i);
      int cost = (int)ListBox_GetItemData(info->hwndCostList,i);
      info->cost += quantity * cost;
   }

   /* Draw new total cost */
   sprintf(temp, "%d", info->cost);
   SetWindowText(info->hwndCost, temp);
}
/************************************************************************/
/*
 * CostListDrawItem:  Handle WM_DRAWITEM messages for cost list box.
 */
BOOL CostListDrawItem(const DRAWITEMSTRUCT *lpdis)
{
   HBRUSH hbrush;
   Bool selected = False; /* Never draw highlight */
   int dc_state;

   /* If box is empty, do nothing */
   if (lpdis->itemID == -1)
      return TRUE;

   dc_state = SaveDC(lpdis->hDC);
   SetBkMode(lpdis->hDC, TRANSPARENT);

   switch (lpdis->itemAction)
   {
   case ODA_SELECT:
   case ODA_DRAWENTIRE:
      /* Always draw unselected */
      hbrush = GetBrush(COLOR_LISTBGD);

      FillRect(lpdis->hDC, &lpdis->rcItem, hbrush);
      
      SelectObject(lpdis->hDC, GetFont(FONT_LIST));
      if (lpdis->itemState & ODS_DISABLED)
	 SetTextColor(lpdis->hDC, GetSysColor(COLOR_GRAYTEXT));
      else SetTextColor(lpdis->hDC, GetColor(COLOR_LISTFGD));
      ListBox_GetText(lpdis->hwndItem, lpdis->itemID, temp);
      DrawText(lpdis->hDC, temp, strlen(temp), 
	       &((DRAWITEMSTRUCT *) lpdis)->rcItem, DT_VCENTER | DT_CENTER | DT_NOPREFIX);
      break;

   case ODA_FOCUS:
      DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
      break;
   }
   RestoreDC(lpdis->hDC, dc_state);

   return TRUE;
}

/*****************************************************************************/
/*
 * BuyList:  Server just sent us list of objects that can be bought.
 *   Bring up buy dialog and get items from user.
 *   seller is object that we are buying from
 *   items is list of objects to buy (each of type buy_object)
 */
void BuyList(object_node seller, list_type items)
{
   BuyDialogStruct dlg_info;

   dlg_info.items = items;
   dlg_info.seller_id = seller.id;
   dlg_info.seller_name = seller.name_res;
   dlg_info.cost = 0;

   if (hwndBuyDialog == NULL)
      /* Give user list of things to select from */
      DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_BUY), hMain, BuyDialogProc, (LPARAM) &dlg_info);

   ObjectListDestroy(items);
}
/*****************************************************************************/
/*
 * WithdrawList:  Server just sent us list of objects that can be withdrawn.
 *   Bring up buy dialog and get items from user.
 *   seller is object that we are buying from
 *   items is list of objects to buy (each of type buy_object)
 */
void WithdrawalList(object_node seller, list_type items)
{
   BuyDialogStruct dlg_info;

   dlg_info.items = items;
   dlg_info.seller_id = seller.id;
   dlg_info.seller_name = seller.name_res;
   dlg_info.cost = 0;

   if (hwndBuyDialog == NULL)
      /* Give user list of things to select from */
      DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_WITHDRAWAL), hMain, WithdrawalDialogProc, (LPARAM) &dlg_info);

   ObjectListDestroy(items);
}
