// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * offer.c:  Handle dialogs and messages that have to do with trades.
 */

#include "client.h"

static HWND hSendOfferDlg = NULL; /* Non-NULL if Send Offer dialog is up */
static HWND hRcvOfferDlg = NULL;  /* Non-NULL if Receive Offer dialog is up */
extern HWND hCurrentDlg;

static SendOfferDialogStruct *SendInfo;  /* Info on current offer */
static list_type receive_items = NULL;   /* List of items other guy offers */

static RcvOfferDialogStruct *RcvInfo;   /* Info on current offer */
static list_type send_items = NULL;     /* Items user is offering in return */

static WNDPROC lpfnDefListProc; /* Default list box window procedure */

/* local function prototypes */
void SendOfferEndDialog(HWND hDlg);
void RcvOfferEndDialog(HWND hDlg);
long CALLBACK OfferListProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

/************************************************************************/
BOOL CALLBACK SendOfferDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   static HWND hwndSend, hwndReceive;   /* Subwindows */

   switch (message)
   {
   case WM_INITDIALOG:
      hSendOfferDlg = hDlg;

      CenterWindow(hDlg, hMain);

      hwndSend = GetDlgItem(hDlg, IDC_OFFERSEND);
      hwndReceive = GetDlgItem(hDlg, IDC_OFFERRECEIVE);
      SendMessage(hDlg, BK_SETDLGFONTS, 0, 0);

      // Set up owner drawn boxes
      SetWindowLong(hwndSend, GWL_USERDATA, OD_DRAWOBJ);
      SetWindowLong(hwndReceive, GWL_USERDATA, OD_DRAWOBJ);

      SendInfo = (SendOfferDialogStruct *) lParam;
      
      /* Display offered items in list */
      ItemListSetContents(hwndSend, SendInfo->items, TRUE); // boolean is to list quantities

      /* Subclass list boxes */
      lpfnDefListProc = SubclassWindow(hwndSend, OfferListProc);
      SubclassWindow(hwndReceive, OfferListProc);

      /* Display receiver's name */
      Edit_SetText(GetDlgItem(hDlg, IDC_OFFEREDIT),
		   LookupNameRsc(SendInfo->receiver_name));

      /* Gray out "Accept" button until counteroffer arrives */
      ShowWindow(hwndReceive, SW_HIDE);
      EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);

      return TRUE;

   case WM_ACTIVATE:
      if (wParam == 0)
	 hCurrentDlg = NULL;
      else hCurrentDlg = hDlg;
      return TRUE;

   case BK_SETDLGFONTS:
      SetWindowFont(GetDlgItem(hDlg, IDC_OFFEREDIT), GetFont(FONT_LIST), TRUE);
      return TRUE;

   case BK_SETDLGCOLORS:
      InvalidateRect(hDlg, NULL, TRUE);
      return TRUE;

   case WM_COMPAREITEM:
      return ItemListCompareItem(hDlg, (const COMPAREITEMSTRUCT *) lParam);
   case WM_MEASUREITEM:
      ItemListMeasureItem(hDlg, (MEASUREITEMSTRUCT *) lParam);
      return TRUE;
   case WM_DRAWITEM:
      return ItemListDrawItemNoSelect(hDlg, (const DRAWITEMSTRUCT *)(lParam));

      HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORDLG, DialogCtlColor);

      HANDLE_MSG(hDlg, WM_INITMENUPOPUP, InitMenuPopupHandler);

   case BK_COUNTEROFFER:
      /* Update receive list */
      receive_items = (list_type) lParam;
      ItemListSetContents(hwndReceive, receive_items, True);

      /* Enable Accept button */
      ShowWindow(hwndReceive, SW_NORMAL);
      EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);

      SetWindowText(GetDlgItem(hDlg, IDC_MESSAGE), GetString(hInst, IDS_GOTOFFER));
      return TRUE;

   case WM_CLOSE:
      SendCancelOffer();
      /* fall through */

   case BK_OFFERDONE:
      SendOfferEndDialog(hDlg);
      break;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDC_OFFERSEND:
      case IDC_OFFERRECEIVE:
	 /* Double click ==> look at item */
	 if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_DBLCLK)
	 {
	    RequestLook(ItemListGetId(GET_WM_COMMAND_HWND(wParam, lParam)));
	    SetDescParams(hDlg, DESC_NONE);
	 }
	 return TRUE;

      case IDCANCEL:
	 SendCancelOffer();
	 SendOfferEndDialog(hDlg);
	 return TRUE;

      case IDOK:
	 SendAcceptOffer();
	 SendOfferEndDialog(hDlg);
	 return TRUE;
      }
      break;
   }
   return FALSE;
}
/************************************************************************/
/*
 * SendOfferEndDialog:  Perform appropriate cleanup when Send Offer dialog ends.
 */
void SendOfferEndDialog(HWND hDlg)
{
   /* Destroy SendInfo->items, and receive_items, since both came from server.
    */
   SendInfo->items = ObjectListDestroy(SendInfo->items);
   receive_items = ObjectListDestroy(receive_items);
   
   DestroyWindow(hDlg);
   hSendOfferDlg = NULL;
}


/************************************************************************/
BOOL CALLBACK RcvOfferDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   static HWND hwndSend, hwndReceive;

   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, hMain);

      hwndSend = GetDlgItem(hDlg, IDC_OFFERSEND);
      hwndReceive = GetDlgItem(hDlg, IDC_OFFERRECEIVE);
      SendMessage(hDlg, BK_SETDLGFONTS, 0, 0);

      // Set up owner drawn boxes
      SetWindowLong(hwndSend, GWL_USERDATA, OD_DRAWOBJ);
      SetWindowLong(hwndReceive, GWL_USERDATA, OD_DRAWOBJ);

      RcvInfo = (RcvOfferDialogStruct *) lParam;

      /* Display offered items in list */
      ItemListSetContents(hwndReceive, RcvInfo->items, True);

      /* Subclass list boxes */
      lpfnDefListProc = (WNDPROC) GetWindowLong(hwndSend, GWL_WNDPROC);
      SubclassWindow(hwndSend, OfferListProc);
      SubclassWindow(hwndReceive, OfferListProc);

      /* Display receiver's name */
      Edit_SetText(GetDlgItem(hDlg, IDC_OFFEREDIT),
		   LookupNameRsc(RcvInfo->sender_name));

      ShowWindow(hDlg, SW_HIDE);

      hRcvOfferDlg = hDlg;
      return TRUE;

   case WM_ACTIVATE:
      if (wParam == 0)
	 hCurrentDlg = NULL;
      else hCurrentDlg = hDlg;
      return TRUE;

   case BK_SETDLGFONTS:
      SetWindowFont(GetDlgItem(hDlg, IDC_OFFEREDIT), GetFont(FONT_LIST), TRUE);
      return TRUE;

   case BK_SETDLGCOLORS:
      InvalidateRect(hDlg, NULL, TRUE);
      return TRUE;

   case WM_COMPAREITEM:
      return ItemListCompareItem(hDlg, (const COMPAREITEMSTRUCT *) lParam);
   case WM_MEASUREITEM:
      ItemListMeasureItem(hDlg, (MEASUREITEMSTRUCT *) lParam);
      return TRUE;
   case WM_DRAWITEM:
      return ItemListDrawItemNoSelect(hDlg, (const DRAWITEMSTRUCT *)(lParam));

      HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, DialogCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORDLG, DialogCtlColor);

      HANDLE_MSG(hDlg, WM_INITMENUPOPUP, InitMenuPopupHandler);

   case WM_CLOSE:
      SendCancelOffer();
      /* fall through */

   case BK_OFFERDONE:
      RcvOfferEndDialog(hDlg);
      return TRUE;

   case BK_COUNTEROFFERED:
      /* Display items we counteroffered */
      send_items = (list_type) lParam;
      ItemListSetContents(hwndSend, send_items, True);      
      return TRUE;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDC_OFFERSEND:
      case IDC_OFFERRECEIVE:
	 /* Double click ==> look at item */
	 if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_DBLCLK)
	 {
	    RequestLook(ItemListGetId(GET_WM_COMMAND_HWND(wParam, lParam)));
	    SetDescParams(hDlg, DESC_NONE);
	 }
	 return TRUE;

      case IDC_SETITEMS:
	 /* Close any other look dialogs that are up */
	 AbortLookList();

	 /* Get list of items from inventory & tell server */
	 send_items = UserInventoryList(hDlg, GetString(hInst, IDS_OFFERITEMS));

	 /* fall through */

      case IDOK:
	 /* Send counteroffer */
	 RequestCounteroffer(send_items);
	 send_items = ObjectListDestroy(send_items);

	 EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
	 EnableWindow(GetDlgItem(hDlg, IDC_SETITEMS), FALSE);

	 SetWindowText(GetDlgItem(hDlg, IDC_MESSAGE), GetString(hInst, IDS_WAITRESPONSE));
	 return TRUE;

      case IDCANCEL:
	 SendCancelOffer();
	 RcvOfferEndDialog(hDlg);
	 return TRUE;
      }
      break;
   }
   return FALSE;
}
/************************************************************************/
/*
 * RcvOfferEndDialog:  Perform appropriate cleanup when Receive Offer dialog ends.
 */
void RcvOfferEndDialog(HWND hDlg)
{
   /* Destroy RcvInfo->items, since it comes from server.
    * Delete send_items, since it came from inventory.
    */
   RcvInfo->items = ObjectListDestroy(RcvInfo->items);
   send_items = ObjectListDestroy(send_items);
   
   DestroyWindow(hDlg);
   hRcvOfferDlg = NULL;
}
/************************************************************************/
/*
 * OfferListProc:  Subclassed window procedure for offer list boxes.
 */
long CALLBACK OfferListProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   ID id;

   switch (message)
   {
   case WM_KEYDOWN:
      /* Return ==> look at item */
      if (wParam == VK_SPACE) 
      {
	 id = ItemListGetId(hwnd);
	 if (id == INVALID_ID)
	    return 0;

	 RequestLook(id);
	 SetDescParams(GetParent(hwnd), DESC_NONE); /* Parent = dialog box */
	 return 0;
      }

      if (wParam == VK_DOWN)
      {
	 id = ListBox_GetCaretIndex(hwnd);
	 debug(("Old index = %ld\n", id));
      }

      break;

   case WM_CHAR:
      return 0;        /* Disable default Windows list box behavior */
   }
   return CallWindowProc(lpfnDefListProc, hwnd, message, wParam, lParam);
}


/************************************************************************/
void OfferChangeFonts(void)
{
   if (hSendOfferDlg != NULL)
      SendMessage(hSendOfferDlg, BK_SETDLGFONTS, 0, 0);
   if (hRcvOfferDlg != NULL)
      SendMessage(hRcvOfferDlg, BK_SETDLGFONTS, 0, 0);
}
/************************************************************************/
void OfferChangeColor(void)
{
   if (hSendOfferDlg != NULL)
      SendMessage(hSendOfferDlg, BK_SETDLGCOLORS, 0, 0);
   if (hRcvOfferDlg != NULL)
      SendMessage(hRcvOfferDlg, BK_SETDLGCOLORS, 0, 0);
}
/************************************************************************/
void RaiseOfferWindow(void)
{
   if (hSendOfferDlg != NULL)
   {
      ShowWindow(hSendOfferDlg, SW_SHOWNORMAL);
      SetFocus(hSendOfferDlg);
   }
   if (hRcvOfferDlg != NULL)
   {
      ShowWindow(hRcvOfferDlg, SW_SHOWNORMAL);
      SetFocus(hRcvOfferDlg);
   }
}
/************************************************************************/
void OfferClose(void)
{
   if (hSendOfferDlg != NULL)
      SendMessage(hSendOfferDlg, WM_CLOSE, 0, 0);
   if (hRcvOfferDlg != NULL)
      SendMessage(hRcvOfferDlg, WM_CLOSE, 0, 0);
}


/************************************************************************/
/*
 * ReceiveOffer:  Player has received an offer from another player.
 *   Bring up receive offer dialog, if one isn't already up.
 */
void ReceiveOffer(ID obj_id, ID icon_res, ID name_res, list_type items)
{
   static RcvOfferDialogStruct info;
   HWND hwndFocus;

   if (hSendOfferDlg != NULL || hRcvOfferDlg != NULL)
      return;

   info.sender_name = name_res;
   info.items = items;

   /* Create offer dialog hidden, but restore focus to original window */
   hwndFocus = GetFocus();
   CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_OFFERRECEIVE), 
		     NULL, RcvOfferDialogProc, (LPARAM) &info);
   SetFocus(hwndFocus);
}
/************************************************************************/
/*
 * Counteroffer:  Player has received a response to his offer.
 *   Update offer dialog, if one is up.
 */
void Counteroffer(list_type items)
{
   if (hSendOfferDlg == NULL)
   {
      ObjectListDestroy(items);
      return;
   }
   SendMessage(hSendOfferDlg, BK_COUNTEROFFER, 0, (LPARAM) items);
}
/************************************************************************/
/* 
 * Counteroffered:  Player made a counteroffer; now display list of items
 *   in Receive Offer dialog.
 */ 
void Counteroffered(list_type items)
{
   if (hRcvOfferDlg == NULL)
   {
      ObjectListDestroy(items);
      return;
   }
   SendMessage(hRcvOfferDlg, BK_COUNTEROFFERED, 0, (LPARAM) items);
}
/************************************************************************/
/*
 * OfferCanceled:  Other guy has canceled offer.
 */
void OfferCanceled(void)
{
   if (hSendOfferDlg != NULL)
      SendMessage(hSendOfferDlg, BK_OFFERDONE, 0, 0);
   if (hRcvOfferDlg != NULL)
      SendMessage(hRcvOfferDlg, BK_OFFERDONE, 0, 0);
}
/************************************************************************/
/*
 * OfferInProgress:  Return True iff one of the offer dialogs is up.
 */
Bool OfferInProgress(void)
{
   return hSendOfferDlg != NULL || hRcvOfferDlg != NULL;
}
