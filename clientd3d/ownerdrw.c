// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * ownerdrw.c:  Procedures for handling owner-drawn window controls.
 *
 * This file implements an owner-drawn list box or combo box control.  The control can 
 * display bitmaps of objects; its behavior is controlled by the OD_* constants that
 * the creator of the window should put in the GWL_USERDATA field of the control when
 * it's created.
 */

#include "client.h"

/* Size of objects drawn in list box */
#define LIST_OBJECT_HEIGHT 16
#define LIST_OBJECT_WIDTH 16

/* Borders around list objects */
#define LIST_OBJECT_LEFT_BORDER 3
#define LIST_OBJECT_TOP_BORDER 2
#define LIST_OBJECT_RIGHT_BORDER 3

static HIMAGELIST g_hImageList = NULL;

static void DrawOwnerListItem(const DRAWITEMSTRUCT *lpdis, Bool selected, Bool combo);
/*****************************************************************************/
/*
 * WindowBeginUpdate:  Turn off redrawing for a list box so that it won't
 *   flash as items are added.  WindowEndUpdate should then be called after 
 *   all items are adaded, to cause the window to redraw.
 */
void WindowBeginUpdate(HWND hwnd)
{
   SetWindowRedraw(hwnd, FALSE);
}
/*****************************************************************************/
/*
 * WindowEndUpdate:  Turn on redrawing for a window, and redraw it.
 */
void WindowEndUpdate(HWND hwnd)
{
   SetWindowRedraw(hwnd, TRUE);
   InvalidateRect(hwnd, NULL, TRUE);
}


/*****************************************************************************/
/*
 * NOTE: All the procedures below are meant to work on either owner-drawn list
 *       boxes or owner-drawn combo boxes, because the code is virtually identical
 *       except for Windows messages.  The last parameter to each function is 
 *       True for combo boxes, and False for list boxes.
 */
/*****************************************************************************/


/*****************************************************************************/
/*
 * OwnerListAddItem:  Add an item to an item list box; it gets placed at 
 *   given index.  If index is -1, item is added to the end if list isn't sorted;
 *   if the list is sorted, the object is added in sorted order.
 *   Returns index of added item.
 */
int OwnerListAddItem(HWND hwnd, object_node *obj, int index, Bool combo, Bool quan)
{
   char *name, desc[MAXNAME + 15];
   int pos;

   name = LookupNameRsc(obj->name_res);
   /* If item is a number object, add its amount after the item's name */
   if (quan && IsNumberObj(obj->id))
   {
      sprintf(desc, "%d %.*s", obj->amount, MAXNAME, name);
      name = desc;
   }
   /* If player using an object, say so */
   if (IsInUse(obj->id))
   {
      sprintf(desc, "%.*s %s", MAXNAME, name, GetString(hInst, IDS_INUSE));
      name = desc;
   }

   if (index ==  -1)
      pos = combo ? ComboBox_AddString(hwnd, name) 
	 : ListBox_AddString(hwnd, name);
   else pos = combo ? ComboBox_InsertString(hwnd, index, name) 
      : ListBox_InsertString(hwnd, index, name);

   combo ? ComboBox_SetItemData(hwnd, pos, obj) : ListBox_SetItemData(hwnd, pos, obj);
   return pos;
}
/*****************************************************************************/
/*
 * OwnerListSetContents:  Set the list box's contents to the items in the given
 *   list.  Calls WindowBegin/EndUpdate to prevent flashing.
 */
void OwnerListSetContents(HWND hwnd, list_type contents, Bool combo, Bool quan)
{
   list_type l;

   WindowBeginUpdate(hwnd);
   combo ? ComboBox_ResetContent(hwnd) : ListBox_ResetContent(hwnd);
   for (l = contents; l != NULL; l = l->next)
      OwnerListAddItem(hwnd, (object_node *) l->data, -1, combo, quan);  /* Add to end */
   WindowEndUpdate(hwnd);
}
/*****************************************************************************/
/*
 * OwnerListRemoveItem:  Remove the item from the list if it's there.
 */
void OwnerListRemoveItem(HWND hList, ID id, Bool combo)
{
   int i, num_items = combo ? ComboBox_GetCount(hList) : ListBox_GetCount(hList);
   object_node *obj;

   for (i = 0; i < num_items; i++)
   {
      obj = combo ? (object_node *) ComboBox_GetItemData(hList, i) :
	 (object_node *) ListBox_GetItemData(hList, i);

      if (GetObjId(obj->id) == GetObjId(id))
      {
	 if (combo)
	    ComboBox_DeleteString(hList, i);
	 else
	    ListBox_DeleteString(hList, i);
	 return;
      }
   }
}
/*****************************************************************************/
/*
 * OwnerListFindItem:  Return the index of the object with the given id in the
 *   given list box, or -1 if it's not there.
 */
int OwnerListFindItem(HWND hList, ID id, Bool combo)
{
   int i, num_items = combo ? ComboBox_GetCount(hList) : ListBox_GetCount(hList);
   object_node *obj;

   for (i=0; i < num_items; i++)
   {
      obj = combo ? (object_node *) ComboBox_GetItemData(hList, i) :
	 (object_node *) ListBox_GetItemData(hList, i);

      if (GetObjId(obj->id) == GetObjId(id))
	 return i;
   }
   return -1;
}
/*****************************************************************************/
/* 
 * OwnerListChangeItem:  Change attributes of given object in item list.
 *   We should reset icon & name string of the item in the list.
 */
void OwnerListChangeItem(HWND hwnd, object_node *obj, Bool combo, Bool quan)
{
   int index;

   if ((index = OwnerListFindItem(hwnd, obj->id, combo)) == -1)
      return;

   /* Delete old item; add new item */
   combo ? ComboBox_DeleteString(hwnd, index) : ListBox_DeleteString(hwnd, index);
   OwnerListAddItem(hwnd, obj, index, combo, quan);
}
/*****************************************************************************/
/*
 * OwnerListFindObject:  Return the object with the given id in the
 *   given list box, or NULL if it's not there.
 */
object_node *OwnerListFindObject(HWND hList, ID id, Bool combo)
{
   int i, num_items = combo ? ComboBox_GetCount(hList) : ListBox_GetCount(hList);
   object_node *obj;

   for (i=0; i < num_items; i++)
   {
      obj = combo ? (object_node *) ComboBox_GetItemData(hList, i) :
	 (object_node *) ListBox_GetItemData(hList, i);

      if (GetObjId(obj->id) == GetObjId(id))
	 return obj;
   }
   return NULL;
}
/*****************************************************************************/
/*
 * ItemListGetId:  Return the id of the object in the list box that currently
 *   has the focus (caret).
 * Returns INVALID_ID if no item has caret.
 * NOTE: This function does NOT work on combo boxes!!
 */
ID ItemListGetId(HWND hList)
{
   object_node *obj;
   int index;

   /* Get id of currently focused item */
   index = ListBox_GetCaretIndex(hList);
   if (index == LB_ERR)
      return INVALID_ID;
   
   obj = (object_node *) ListBox_GetItemData(hList, index);
   if (obj == NULL)
      return INVALID_ID;
   return obj->id; 
}
/*****************************************************************************/
/* 
 * OwnerListCompareItem:  Message handler for item list boxes.
 */
int OwnerListCompareItem(HWND hwnd, const COMPAREITEMSTRUCT *lpcis, Bool combo)
{
   char buf1[MAXNAME], buf2[MAXNAME];
   
   if (combo)
   {
      ComboBox_GetLBText(lpcis->hwndItem, lpcis->itemID1, buf1);
      ComboBox_GetLBText(lpcis->hwndItem, lpcis->itemID2, buf2);
   }
   else
   {
      ListBox_GetText(lpcis->hwndItem, lpcis->itemID1, buf1);
      ListBox_GetText(lpcis->hwndItem, lpcis->itemID2, buf2);
   }

   return stricmp(buf1, buf2);
}
/*****************************************************************************/
/* 
 * OwnerListMeasureItem:  Message handler for item list boxes.
 */
void OwnerListMeasureItem(HWND hwnd, MEASUREITEMSTRUCT *lpmis, Bool combo)
{
   int style, height;

   /* Height of item is maximum of:
    * 1) Bitmap height (if bitmap is present), plus borders
    * 2) Text height, plus borders
    */

   style = GetWindowLong(hwnd, GWL_USERDATA);

   height = GetFontHeight(GetFont(FONT_LIST));
   if (style & OD_DRAWOBJ)
      height = max(LIST_OBJECT_HEIGHT, height);
   lpmis->itemHeight = height + 2 * LIST_OBJECT_TOP_BORDER;
}
/*****************************************************************************/
/* 
 * OwnerListDrawItem:  Message handler for item list boxes.  Return TRUE iff
 *   message is handled.
 */
BOOL OwnerListDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis, Bool combo)
{
   {
      int iOldCaretItem = (int)GetProp(lpdis->hwndItem, "Caret");
      int iNewCaretItem = SendMessage(lpdis->hwndItem, LB_GETCARETINDEX, 0, 0L);
      if (GetFocus() != lpdis->hwndItem)
	 iNewCaretItem = -2;
      if (iOldCaretItem != iNewCaretItem)
	 InvalidateRect(lpdis->hwndItem, NULL, FALSE);
      SetProp(lpdis->hwndItem, "Caret", (HANDLE)iNewCaretItem);
   }

   switch (lpdis->itemAction)
   {
   case ODA_SELECT:
   case ODA_DRAWENTIRE:
      DrawOwnerListItem(lpdis, (Bool) (lpdis->itemState & ODS_SELECTED), combo);
      break;

   case ODA_FOCUS:
      // ignored; DrawOwnerListItem() does this itself due to bad XOR ordering
      break;
   }

   return TRUE;
}
/*****************************************************************************/
/* 
 * OwnerListDrawItemNoSelect:  Message handler for item list boxes.  Return TRUE
 *   message is handled.
 *   Doesn't draw selected objects differently from unselected ones, essentially
 *   disabling selections.
 */
BOOL OwnerListDrawItemNoSelect(HWND hwnd, const DRAWITEMSTRUCT *lpdis, Bool combo)
{
   {
      int iOldCaretItem = (int)GetProp(lpdis->hwndItem, "Caret");
      int iNewCaretItem = SendMessage(lpdis->hwndItem, LB_GETCARETINDEX, 0, 0L);
      if (GetFocus() != lpdis->hwndItem)
	 iNewCaretItem = -2;
      if (iOldCaretItem != iNewCaretItem)
	 InvalidateRect(lpdis->hwndItem, NULL, FALSE);
      SetProp(lpdis->hwndItem, "Caret", (HANDLE)iNewCaretItem);
   }

   switch (lpdis->itemAction)
   {
   case ODA_SELECT:
   case ODA_DRAWENTIRE:
      DrawOwnerListItem(lpdis, False, combo);
      break;

   case ODA_FOCUS:
      // ignored; DrawOwnerListItem() does this itself due to bad XOR ordering
      break;
   }

   return TRUE;
}
/*****************************************************************************/
/*
 * DrawOwnerListItem:  Draw an owner-draw list item, not including selection
 *   or focus.
 *   The item is drawn selected iff selected is True.
 *   (lpdis doesn't determine selection so that OwnerListDrawItemNoSelect can
 *    force item to be drawn without selection.)
 */
void DrawOwnerListItem(const DRAWITEMSTRUCT *lpdis, Bool selected, Bool combo)
{
   char buf[MAXNAME];
   object_node *obj;
   int dc_state, style;
   WORD y;
   RECT r;
   AREA area;
   COLORREF crColorText;
   HBRUSH hColorBg;

   if (!lpdis || !IsWindow(lpdis->hwndItem) || !IsWindowVisible(lpdis->hwndItem))
      return;

   style = GetWindowLong(lpdis->hwndItem, GWL_USERDATA);

   /* Set text mode of DC to transparent */
   dc_state = SaveDC(lpdis->hDC);
   SetBkMode(lpdis->hDC, OPAQUE);
   obj = (object_node*)lpdis->itemData;

   hColorBg = GetBrush(GetItemListColor(lpdis->hwndItem, (selected? SEL_BGD : UNSEL_BGD), NULL));
   if ((style & OD_ONLYSEL) && (style & (OD_DRAWOBJ | OD_DRAWICON)))
      hColorBg = GetBrush(GetItemListColor(lpdis->hwndItem, UNSEL_BGD, NULL));

   FillRect(lpdis->hDC, &lpdis->rcItem, hColorBg);
   SetBkMode(lpdis->hDC, TRANSPARENT);

   /* Send object flags for objects with icons in lists for coloring magic weapons.
      Character select screen causes a client crash because the character name is
      obj; this statement causes NULL to be sent in that case */
   if (style & (OD_DRAWOBJ | OD_DRAWICON))
   {
      crColorText = GetColor(GetItemListColor(lpdis->hwndItem, (selected? SEL_FGD : UNSEL_FGD), obj->flags));
   }
   else
   {
      crColorText = GetColor(GetItemListColor(lpdis->hwndItem, (selected? SEL_FGD : UNSEL_FGD), NULL));
   }
   
   if ((style & OD_ONLYSEL) && (style & (OD_DRAWOBJ | OD_DRAWICON)))
   crColorText = GetColor(GetItemListColor(lpdis->hwndItem, UNSEL_FGD, obj->flags));


   if (lpdis->itemState & ODS_DISABLED)
   crColorText = GetSysColor(COLOR_GRAYTEXT);

   if (style & OD_DRAWOBJ)
   {
   /* Draw small version of object's icon, centered vertically */
   y = (lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top) / 2)
      - LIST_OBJECT_HEIGHT / 2;
   
   area.x  = LIST_OBJECT_LEFT_BORDER;
   area.y  = y;
   area.cx = LIST_OBJECT_WIDTH;
   area.cy = LIST_OBJECT_HEIGHT;

   if (!(style & OD_ONLYSEL) || selected)
   {
      DrawStretchedObjectDefault(lpdis->hDC, obj, &area, hColorBg);
   }
   }

	if (style & OD_DRAWICON)
	{
		if (!g_hImageList)
		{
			// set up image list first time we use it
			HBITMAP hbmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_IMAGELIST));
			g_hImageList = ImageList_Create(16, 16, ILC_COLOR | ILC_MASK, 0, 1);
			ImageList_AddMasked(g_hImageList, hbmp, RGB(0, 128, 128));
			DeleteObject(hbmp);
			//REVIEW: never doing a ImageList_Destroy(g_hImageList);
		}

		/* Draw chosen icon from image list, centered vertically */
		if (!(style & OD_ONLYSEL) || selected)
		{
			int iIcon = OD_GETICONINDEX(style);
			if (style & OD_NEXTICON)
				iIcon++;
			ImageList_Draw(g_hImageList, iIcon,
				lpdis->hDC,
				lpdis->rcItem.left+1, lpdis->rcItem.top+1,
				ILD_TRANSPARENT);
		}
	}

   /* Draw name of object */
   r.top = lpdis->rcItem.top + LIST_OBJECT_TOP_BORDER;
   r.bottom = lpdis->rcItem.bottom - LIST_OBJECT_TOP_BORDER;
   r.right = lpdis->rcItem.right;
   if (style & (OD_DRAWOBJ | OD_DRAWICON))
   {
      r.left = lpdis->rcItem.left + LIST_OBJECT_LEFT_BORDER + LIST_OBJECT_WIDTH + 
	 LIST_OBJECT_RIGHT_BORDER;
   }
   else
   {
      r.left = lpdis->rcItem.left + LIST_OBJECT_LEFT_BORDER;
   }

   if (combo)
      ComboBox_GetLBText(lpdis->hwndItem, lpdis->itemID, buf);
   else
      ListBox_GetText(lpdis->hwndItem, lpdis->itemID, buf);

   SelectObject(lpdis->hDC, GetFont(FONT_LIST));

   if (style & OD_COLORTEXT)
   {
   	// get the color we'd prefer for this particular obj
   	crColorText = GetPlayerWhoNameColor(obj,NULL);
   	
	// draw a black halo around the text just to ensure it is visible
	SetTextColor(lpdis->hDC, RGB(0, 0, 0));
        OffsetRect(&r, 1, 0);
	DrawText(lpdis->hDC, buf, strlen(buf), &r, DT_VCENTER | DT_LEFT | DT_NOPREFIX);
        OffsetRect(&r, -2, 0);
	DrawText(lpdis->hDC, buf, strlen(buf), &r, DT_VCENTER | DT_LEFT | DT_NOPREFIX);
        OffsetRect(&r, 1, 1);
	DrawText(lpdis->hDC, buf, strlen(buf), &r, DT_VCENTER | DT_LEFT | DT_NOPREFIX);
        OffsetRect(&r, 0, -2);
	DrawText(lpdis->hDC, buf, strlen(buf), &r, DT_VCENTER | DT_LEFT | DT_NOPREFIX);
        OffsetRect(&r, 0, 1);
   }

   SetTextColor(lpdis->hDC, crColorText);
   DrawText(lpdis->hDC, buf, strlen(buf), &r, DT_VCENTER | DT_LEFT | DT_NOPREFIX);

   RestoreDC(lpdis->hDC, dc_state);

   if (!combo &&
       (GetFocus() == lpdis->hwndItem) &&
       (SendMessage(lpdis->hwndItem, LB_GETCARETINDEX, 0, 0L) == (LRESULT)lpdis->itemID))
   {
      DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
   }
}
