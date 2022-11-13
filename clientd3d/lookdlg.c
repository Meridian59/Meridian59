// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * lookdlg.c:  Handle dialog box that contains a list of items to select from.
 *             Only one such dialog can be in existence at any time.
 */

#include "client.h"

static LookDialogStruct *info = NULL;        /* Info on current dialog */

static list_type LookDialogRetval;

static HWND hwndLookDialog = NULL;  /* Non-NULL when Look dialog is displayed */

/* local function prototypes */
static void LookListSetContents(HWND hwndListBox, list_type contents, int flags);
static void LookListFreeContents(HWND hwndListBox);

static BOOL LookInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
static int  LookVkeyToItem(HWND hwnd, UINT key, HWND hwndListbox, int iCaret);
static void LookCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
static long CALLBACK LookProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
/*****************************************************************************/
/* 
 * LookListSetContents: Set contents & highlight of list box in Look dialog.
 */
void LookListSetContents(HWND hwndListBox, list_type contents, int flags)
{
   list_type l;
   object_node *obj;
   
   WindowBeginUpdate(hwndListBox);
   ListBox_ResetContent(hwndListBox);
   
   /* Fill in list box with stuff from passed-in list */
   for (l = contents; l != NULL; l = l->next)
   {
      /* We need to make a copy of this object, because the original object
         might be freed by the game before this dialog ends */
      obj = ObjectCopy((object_node *) (l->data));
      ItemListAddItem(hwndListBox, obj, -1, !(flags & LD_AMOUNTS));  /* Add to end */
   }
   WindowEndUpdate(hwndListBox);
   
   /* In single selection box, make 1st item default.  In multiple selection,
    * only highlight 1st item if there is just 1 item AND item doesn't need an amount */
   if ((flags & LD_MULTIPLESEL) && contents->next == NULL)
   {
      obj = (object_node *) contents->data;
      if (!(info->flags & LD_AMOUNTS) || !IsNumberObj(obj->id))
      {
         ListBox_SetSel(hwndListBox, TRUE, 0);
         info->selected[0] = True;
      }
   }
   else ListBox_SetCurSel(hwndListBox, 0); 
}
/****************************************************************************/
/*
 * AbortLookList:  Abort a look dialog.  If we lose the server connection or
 *   the server invalidates our room data.
 */
void AbortLookList(void)
{
   if (hwndLookDialog != NULL)
      SendMessage(hwndLookDialog, WM_COMMAND, IDCANCEL, 0);
}
/****************************************************************************/
/*
 * LookListFreeContents:  Free memory associated with items in Look dialog's
 *   list box.
 */
void LookListFreeContents(HWND hwndListBox)
{
   int i, num_entries;
   object_node *obj;

   /* Free previous list contents */
   num_entries = ListBox_GetCount(hwndListBox);
   
   for (i = 0; i < num_entries; i++)
   {
      obj = (object_node *) ListBox_GetItemData(hwndListBox, i);
      ObjectDestroyAndFree(obj);
   }
}
/*****************************************************************************/
/*
 * LookDialogProc:  Dialog procedure for user looking at a list of objects, and
 *   selecting one or more.
 *   Places a list of the objects selected ( or NULL if none ) into the global 
 *    variable LookDialogRetval.  This value is valid only if the return value
 *    of the dialog is IDOK (i.e. the user hit OK)
 *   lParam of the WM_INITDIALOG message should be a pointer to a LookDialogStruct.
 */
BOOL CALLBACK LookDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   int index1,index2;

   switch (message)
   {
      HANDLE_MSG(hDlg, WM_INITDIALOG, LookInitDialog);
      HANDLE_MSG(hDlg, WM_VKEYTOITEM, LookVkeyToItem);
      HANDLE_MSG(hDlg, WM_COMMAND, LookCommand);

   case WM_COMPAREITEM:
      return ItemListCompareItem(hDlg, (const COMPAREITEMSTRUCT *) lParam);
   case WM_MEASUREITEM:
      ItemListMeasureItem(hDlg, (MEASUREITEMSTRUCT *) lParam);
      return TRUE;
   case WM_DRAWITEM:
      /* Cheat--if item is about to be drawn, then perhaps list has been scrolled.
       * Tell ourselves to realign list boxes after we've finished drawing */
      PostMessage(hDlg, BK_ALIGNDLGS, 0, 0);
      return ItemListDrawItem(hDlg, (const DRAWITEMSTRUCT *)(lParam));

      HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORDLG, DialogCtlColor);

   case BK_ALIGNDLGS:
      /* Ensure that cost list shows same index at top that item list does */
      index1 = ListBox_GetTopIndex(info->hwndListBox);
      index2 = ListBox_GetTopIndex(info->hwndQuanList);
      if (index1 != index2)
      {
         ListBox_SetTopIndex(info->hwndQuanList, index1);
      }
      return TRUE;

   case WM_DESTROY:
      hwndLookDialog = NULL;
      return TRUE;
   }

   return FALSE;
}
/*****************************************************************************/
/*
 * LookInitDialog:  Handle WM_INITDIALOG messages.
 */ 
BOOL LookInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam) 
{
   DWORD style;
   int numItems, index;

   info = (LookDialogStruct *) lParam;

   CenterWindow(hDlg, GetParent(hDlg));
   info->hwndListBox = GetDlgItem(hDlg, IDC_ITEMLIST);
   info->hwndFind = GetDlgItem(hDlg, IDC_ITEMFIND);
   info->hwndQuanList = GetDlgItem(hDlg, IDC_QUANLIST);

   // Make list sorted if desired
   if (info->flags & LD_SORT)
   {
      style = GetWindowLong(info->hwndListBox, GWL_STYLE);
      SetWindowLong(info->hwndListBox, GWL_STYLE, style | LBS_SORT);
   }

   if (!(info->flags & LD_AMOUNTS))
   {
      ShowWindow(GetDlgItem(hDlg, IDC_STATIC1), SW_HIDE);
      ShowWindow(GetDlgItem(hDlg, IDC_QUANLIST), SW_HIDE);
   }
   
   // Draw objects in owner-drawn list box
   SetWindowLong(info->hwndListBox, GWL_USERDATA, OD_DRAWOBJ);
   
   SetWindowText(hDlg, info->title);  /* Set window's title */

   // Remember state of list box items
   info->selected = (Bool *) SafeMalloc(list_length(info->contents) * sizeof(Bool));
   memset(info->selected, 0, list_length(info->contents) * sizeof(Bool));
   
   LookListSetContents(info->hwndListBox, info->contents, info->flags);

   numItems = ListBox_GetCount(info->hwndListBox);
   for(index=0; index < numItems; index++)
   {
      ListBox_AddString(info->hwndQuanList," ");
      ListBox_SetItemData(info->hwndQuanList, index, 0);
   }
      
   Edit_LimitText(info->hwndFind, MAXNAME);
   SetWindowFont(info->hwndFind, GetFont(FONT_EDIT), TRUE);
   
   hwndLookDialog = hDlg;

   /* Subclass main list box */
   info->lpfnDefLookProc = SubclassWindow(info->hwndListBox, LookProc);

   return TRUE;
}
/*****************************************************************************/
/*
 * LookVkeyToItem:  Handle WM_VKTOITEM messages.
 */ 
int LookVkeyToItem(HWND hwnd, UINT key, HWND hwndListbox, int iCaret)
{
   char buf[MAXNAME + 1];
   int len;

   len = Edit_GetText(info->hwndFind, buf, MAXNAME);
   
   /* Backspace removes last character, if any */
   if (key == VK_BACK)
   {
      if (len != 0)
      {
	 buf[len - 1] = '\0';	    
	 Edit_SetText(info->hwndFind, buf);
      }
      return -2;
   }

   /* Add letter character to edit box */
   key = tolower(key);
   if (key >= 'a' && key <= 'z')
   {
      if (len < MAXNAME)
      {
	 buf[len + 1] = '\0';
	 buf[len] = tolower(key);	 
	 Edit_SetText(info->hwndFind, buf);
      }
      return -2;
   }

   return -1;
}
/*****************************************************************************/
/*
 * LookSelChange:  We received a LBN_SELCHANGE message for the list box; see
 *   if a number item is being selected, and thus we should prompt for an amount.
 */
void LookSelChange(HWND hList)
{
   int i, count;
   Bool selected;
   
   count = ListBox_GetCount(hList);
   for (i=0; i < count; i++)
   {
      selected = (ListBox_GetSel(hList, i) > 0);
      
      if (info->flags & LD_AMOUNTS && !info->selected[i] && selected)
      {
         // Selecting item
         if (!GetAmountListBox(hList, i))
         {
            ListBox_SetSel(hList, FALSE, i);
            info->selected[i] = False;
            continue;
         }
      }
      
      info->selected[i] = selected;
   }
   
}
/*****************************************************************************/
/*
 * LookCommand:  Handle WM_COMMAND messages.
 */ 
void LookCommand(HWND hDlg, int ctrl_id, HWND hwndCtl, UINT codeNotify) 
{
   int index, num_entries, i, amount, currentAmount;
   list_type selection;
   object_node *obj;
   char buf[MAXNAME + 1], temp[16];

   switch(ctrl_id)
   {
   case IDC_ITEMFIND:
      if (codeNotify == EN_UPDATE)
      {
	 /* Go to object in list that user has named */
	 Edit_GetText(info->hwndFind, buf, MAXNAME);
	 index = ListBox_FindString(info->hwndListBox, -1, buf);

	 if (index != LB_ERR)
	    if (info->flags & LD_MULTIPLESEL)
	       ListBox_SetCaretIndex(info->hwndListBox, index);
	    else ListBox_SetCurSel(info->hwndListBox, index);
      }
      break;

   case IDC_ALL:
      /* In multiple selection box only, select all objects.  If we require that
       * user give amounts, don't select amount objects */
      num_entries = ListBox_GetCount(info->hwndListBox);

      ListBox_SetSel(info->hwndListBox, TRUE, -1);

      WindowBeginUpdate(info->hwndQuanList);
      // Select all for number items
      for (i=0; i < num_entries; i++)
      {
	 info->selected[i] = True;
	 obj = (object_node *) ListBox_GetItemData(info->hwndListBox, i);
	 if (IsNumberObj(obj->id))
	    amount = obj->amount;
	 else
	    amount = 1;
	 obj->temp_amount = amount;
	 ListBox_DeleteString(info->hwndQuanList,i);
	 sprintf(temp, "%d", amount);
	 ListBox_InsertString(info->hwndQuanList,i,temp);
	 ListBox_SetItemData(info->hwndQuanList,i,amount);
      }
      WindowEndUpdate(info->hwndQuanList);
      break;

   case IDC_ITEMLIST:
      switch (codeNotify)
      {
      case LBN_DBLCLK:
	 /* Look at item */
	 index = ListBox_GetCaretIndex(info->hwndListBox);
	 if (index != LB_ERR)
	 {
	    obj = (object_node *) ListBox_GetItemData(info->hwndListBox, index);
	    RequestLook(obj->id);
	    SetDescParams(hDlg, DESC_NONE);
	    ListBox_SetSel(info->hwndListBox, FALSE, index);
	    info->selected[index] = False;
	 }
	 break;

      case LBN_SELCHANGE:
	 index = ListBox_GetCurSel(info->hwndListBox);
	 obj = (object_node *) ListBox_GetItemData(info->hwndListBox, index);
	 WindowBeginUpdate(info->hwndQuanList);
	 ListBox_DeleteString(info->hwndQuanList,index);
	 if (ListBox_GetSel(info->hwndListBox,index))
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
	 ListBox_InsertString(info->hwndQuanList,index,temp);
	 ListBox_SetItemData(info->hwndQuanList,index,amount);
	 ListBox_SetSel(info->hwndListBox,amount > 0,index);
	 info->selected[index] = (amount > 0);
	 obj->temp_amount = amount;
	 ListBox_SetSel(info->hwndQuanList,FALSE,index);
	 WindowEndUpdate(info->hwndQuanList);

	 break;
      }
      break;

   case IDC_QUANLIST:
      if (codeNotify == LBN_SELCHANGE)
      {
	 char temp[16];
	 index = (int)ListBox_GetCurSel(info->hwndQuanList);
	 obj = (object_node *) ListBox_GetItemData(info->hwndListBox, index);
	 currentAmount = (int)ListBox_GetItemData(info->hwndQuanList, index);
	 amount = currentAmount;
	 if (ListBox_GetItemData(info->hwndQuanList, index) > 0)
	 {
	    if (IsNumberObj(obj->id))
	    {
	       MEASUREITEMSTRUCT m;
	       int startAmount = currentAmount;
	       if (currentAmount == 0)
		  startAmount = obj->amount;
	       /* Place amount dialog just beneath selected item */
	       ItemListMeasureItem(info->hwndQuanList, &m);
	       // Force highlight on (we are editing it)
	       ListBox_SetSel(info->hwndListBox,TRUE,index);
	       if(InputNumber(hDlg,info->hwndQuanList,
		  0,(index - ListBox_GetTopIndex(info->hwndQuanList) + 1) * (m.itemHeight - 1),
		  &amount,startAmount,1,obj->amount))
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
	       else
		  amount = currentAmount;
	       // reset highlight based on quantity - off if zero, on otherwise
	       ListBox_SetSel(info->hwndListBox,
		  (ListBox_GetItemData(info->hwndQuanList,index) > 0),
		  index);
	    }
	    else
	    {
	       ListBox_DeleteString(info->hwndQuanList,index);
	       if (currentAmount == 0)
	       {
		  amount = 1;
		  strcpy(temp,"1");
	       }
	       else
	       {
		  amount = 0;
		  strcpy(temp," ");
	       }
	       ListBox_InsertString(info->hwndQuanList,index,temp);
	       ListBox_SetItemData(info->hwndQuanList,index,amount);
	    }
	 }
	 ListBox_SetSel(info->hwndListBox,amount > 0,index);
	 info->selected[index] = (amount > 0);
	 obj->temp_amount = amount;
	 ListBox_SetCurSel(info->hwndQuanList,-1);
      }
      break;

   case IDOK:
      /* Get user's selection(s) */
      num_entries = ListBox_GetCount(info->hwndListBox);
      selection = NULL;
      
      for (i = 0; i < num_entries; i++)
      {
	 /* If item is selected, add to selection list, else free */
	 obj = (object_node *) ListBox_GetItemData(info->hwndListBox, i);
	 if (ListBox_GetSel(info->hwndListBox, i) > 0)
	    selection = list_add_item(selection, obj);
	 else 
	    ObjectDestroyAndFree(obj);
      }

      LookDialogRetval = selection;
      EndDialog(hDlg, IDOK);
      break;
      
   case IDCANCEL:
      LookListFreeContents(info->hwndListBox);
      
      LookDialogRetval = NULL;
      EndDialog(hDlg, IDCANCEL);
      break;
   }
}
/************************************************************************/
/*
 * InputNumber:  Input number from player.  Has default value, and minimum
 *   and maximum values for spin button.  Places the new dialog
 *   at (x, y) on hwnd. hParent is parent of dialog.
 *   Return True if ok.  False if cancel pressed.
 */
Bool InputNumber(HWND hParent, HWND hwnd, int x, int y, int *returnValue, int startValue, int minValue, int maxValue)
{
   RECT r;
   AmountDialogStruct dlg_info;

   GetWindowRect(hwnd, &r);

   dlg_info.x = r.left + x;
   dlg_info.y = r.top + y;
   dlg_info.amount = startValue;
   dlg_info.minAmount = minValue;
   dlg_info.maxAmount = maxValue;
   
   if (IDCANCEL == DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_AMOUNT), hParent,
      AmountDialogProc, (LPARAM) &dlg_info))
      return False;  /* Don't select item */

   *returnValue = dlg_info.amount;
   return True;
}

/************************************************************************/
/*
 * GetAmount:  If appropriate, display amount dialog to get amount
 *   of given object at (x, y) on hwnd.
 *   hParent is parent of dialog.
 *   Return True if item isn't a number item, or user entered valid amount.
 */
Bool GetAmount(HWND hParent, HWND hwnd, object_node *obj, int x, int y)
{
   int value;

   if (!IsNumberObj(obj->id))
      return True;

   if (!InputNumber(hParent,hwnd,x,y,&value,(int)obj->amount,1,(int)obj->amount))
   {
      return False;
   }
   if (value == 0)
      return False;

   obj->temp_amount = value;
   return True;
#if 0
   RECT r;
   AmountDialogStruct dlg_info;

   if (!IsNumberObj(obj->id))
      return True;

   GetWindowRect(hwnd, &r);

   dlg_info.x = r.left + x;
   dlg_info.y = r.top + y;
   dlg_info.amount = obj->amount;
   
   if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_AMOUNT), hParent,
		      AmountDialogProc, (LPARAM) &dlg_info)
       == IDCANCEL)
      return False;  /* Don't select item */

   obj->temp_amount = dlg_info.amount;

   if (dlg_info.amount == 0)
      return False;

   return True;
#endif
}
/************************************************************************/
/*
 * GetAmountListBox:  If appropriate, display amount dialog to get amount
 *   of object at given index in given list box.
 *   Return True iff item should be selected (i.e. user entered valid #)
 */
Bool GetAmountListBox(HWND hList, int index)
{
   object_node *obj;
   MEASUREITEMSTRUCT m;

   /* See if item requires an amount */
   obj = (object_node *) ListBox_GetItemData(hList, index);
   if (obj == NULL || !IsNumberObj(obj->id))
      return True;

   /* Place amount dialog just beneath selected item */
   ItemListMeasureItem(hList, &m);

   return GetAmount(GetParent(hList), hList, obj, 0, 
		    (index - ListBox_GetTopIndex(hList) + 1) * (m.itemHeight - 1));
}
/************************************************************************/
/*
 * LookProc:  Subclassed window procedure for list box.
 */
long CALLBACK LookProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
   case WM_CHAR:
      /* 
       * Throw out WM_CHAR messages.  If we don't, list box automatically moves
       * caret to item beginning with the letter typed, which we don't want since
       * we move the caret ourselves.  There doesn't appear to be a real way to
       * turn off this default Windows behavior.
       */
      return 0;
   }
   return CallWindowProc(info->lpfnDefLookProc, hwnd, message, wParam, lParam);
}

/************************************************************************/
/*
 * DisplayLookList:  Display a list of items in a popup dialog box.
 *   Returns a list of the objects the user selected, or NULL if he hit
 *     cancel.  These object are actually copies of those in the given list, so the
 *     caller should free the list.
 *   hParent is the parent for the dialog box.
 *   title is the title for the dialog box.
 *   flags affects behavior of dialog box:
 *   If LD_MULTIPLESEL is set, the user may select more than one object from
 *     the list of items; otherwise he may select at most one.
 *   If LD_AMOUNTS is set, ask user for amount of object if appropriate.  amount field
 *     will be set in object in the returned list.
 *   If LD_SINGLEAUTO is set and l contains only one object, the dialog won't be
 *     displayed and the object will be automatically selected.
 *   Note:  The returned list is created by the dialog procedure, and so should
 *          be freed by the caller.
 */
list_type DisplayLookList(HWND hParent, char *title, list_type l, int flags)
{
   LookDialogStruct dlg_info;
   Bool valid;

   /* Do nothing if list is empty, or if dialog is already up */
   if (l == NULL || hwndLookDialog != NULL)
      return NULL;

   /* Even if there's just 1 object, need to copy it to be consistent with general case */
   if ((flags & LD_SINGLEAUTO) && l->next == NULL)
   {
      // Ask for amount for single number items
      object_node *obj = (object_node *) (l->data);
      if (!((flags & LD_AMOUNTS) && IsNumberObj(obj->id)))
	 return list_add_item(NULL, ObjectCopy((object_node *) (l->data)));
   }

   ZeroMemory(&dlg_info,sizeof(dlg_info));
   dlg_info.title = title;
   dlg_info.contents = l;
   dlg_info.flags = flags;

   /* If multiple selections allowed, make list box multiple select */
   if (flags & LD_MULTIPLESEL)
      valid = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ITEMLISTMULTIPLE), hParent,
			     LookDialogProc, (LPARAM) &dlg_info);
   else 
      if (flags & LD_SORT)
	 valid = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ITEMLISTSORTED), hParent,
				LookDialogProc, (LPARAM) &dlg_info);
      else
	 valid = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ITEMLISTSINGLE), hParent,
				LookDialogProc, (LPARAM) &dlg_info);

   if( info != NULL )
   {
       SafeFree(info->selected);
   }
   
   /* In Windows, DialogBox returns an int, which can't fit a pointer.  So
    * we must put the return value in a global variable.  Death to Windows!! */
   if (valid == IDOK)
      return LookDialogRetval;
   else return NULL;
}
