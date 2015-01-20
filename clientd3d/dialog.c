// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* 
* dialog.c: Game dialog box procedures
*/

#include "client.h"

#define PAGE_BREAK_CHAR '¶'	  /* For multi-page descriptions */
#define MAX_PAGE_DESCRIPTION_TEXT MAXMESSAGE

static HWND hDescDialog = NULL;   /* Non-null if Description dialog is up */
static HWND hAmountDialog = NULL; /* Non-null if Amount dialog is up */

static HWND hDescParent = NULL;   /* Parent of next Description dialog to appear */

static int  desc_flags = 0;       /* Flags for how to display description dialog */

static ChildPlacement desc_controls[] = {
	{ IDC_DESCFIXED,   RDI_ALL },
	{ IDC_DESCBOX,     RDI_ALL },
	
	{ IDOK,            RDI_BOTTOM },
	{ IDC_GET,         RDI_BOTTOM },
	{ IDC_DROP,        RDI_BOTTOM },
	{ IDC_USE,         RDI_BOTTOM },
	{ IDC_UNUSE,       RDI_BOTTOM },
	{ IDC_INSIDE,      RDI_BOTTOM },
	{ IDC_ACTIVATE,    RDI_BOTTOM },
	{ IDC_URLLABEL,    RDI_BOTTOM },
	{ IDC_URLBUTTON,   RDI_BOTTOM },
	{ IDC_URL,         RDI_BOTTOM },
	{ IDC_APPLY,       RDI_BOTTOM },
	{ 0,               0 },   // Must end this way
};

extern player_info player;
extern HPALETTE hPal;

static int SetFontToFitText(DescDialogStruct *info, HWND hwnd, int fontNum, const char *text)
{
	HFONT hOldFont;
	LOGFONT newFont;
	int height;
	RECT rcWindow,rcText;
	BOOL fit = FALSE;
	HDC hdc;
	
	hdc = GetDC(hwnd);
	if (info->hFontTitle && info->hFontTitle != GetFont(FONT_TITLES))
		DeleteObject(info->hFontTitle);
	info->hFontTitle = NULL;
	memcpy(&newFont,GetLogfont(fontNum),sizeof(LOGFONT));
	GetClientRect(hwnd,&rcWindow);
	ZeroMemory(&rcText,sizeof(rcText));
	while (!fit)
	{
		info->hFontTitle = CreateFontIndirect(&newFont);
		if (info->hFontTitle == NULL)
		{
			info->hFontTitle = CreateFontIndirect(GetLogfont(fontNum));
			break;
		}
		hOldFont = (HFONT) SelectObject(hdc,info->hFontTitle);
		height = DrawText(hdc,text,-1,&rcText,DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
		SelectObject(hdc,hOldFont);
		if (rcText.right <= rcWindow.right)
			fit = TRUE;
		else
		{
			if (newFont.lfHeight < -4)
			{
				DeleteObject(info->hFontTitle);
				info->hFontTitle = NULL;
				newFont.lfHeight++;
			}
			else if (newFont.lfHeight > 4)
			{
				DeleteObject(info->hFontTitle);
				info->hFontTitle = NULL;
				newFont.lfHeight--;
			}
			else
				fit = TRUE; // smallest font allowable
		}
	}
	SetWindowFont(hwnd,info->hFontTitle,FALSE);
	ReleaseDC(hwnd,hdc);
	return height;
}

static void GetPageText(char *buffer, DescDialogStruct *info)
{
	int page, len;
	int lenDescription;
	char *pFullText;
	char *pStart;
	char *pEnd;
	char *pEndDescription;
	
	if (!buffer)
		return;
	
	buffer[0] = '\0';
	if (!info || !info->description)
		return;
	
	lenDescription = strlen(info->description);
	pFullText = info->description;
	pStart = info->description;
	pEnd = strchr(pStart,PAGE_BREAK_CHAR);
	pEndDescription = info->description + lenDescription;
	
	if (NULL == pEnd)
		pEnd = pEndDescription;
	for (page = 0; page < info->currentPage; page++)
	{
		pStart = pEnd+1;
		pEnd = strchr(pStart,PAGE_BREAK_CHAR);
		if (NULL == pEnd)
			pEnd = pEndDescription;
	}
	len = pEnd - pStart;
	if (len > MAX_PAGE_DESCRIPTION_TEXT)
		len = MAX_PAGE_DESCRIPTION_TEXT;
	if (len > 0)
		memcpy(buffer,pStart,len);
	else if (len < 0)
		len = 0;
	buffer[len] = '\0'; // NULL terminate it.
}

static void ResizeEditToFitText(HWND hEdit, HFONT hFont)
{
	HWND hParent = GetParent(hEdit);
	
	RECT edit_rect, dlg_rect;
	int num_lines, yincrease;
	
	if (!(GetWindowLong(hEdit, GWL_STYLE) & WS_VISIBLE))
		return;
	
	GetWindowRect(hEdit, &edit_rect);
	num_lines = Edit_GetLineCount(hEdit) + 1;
	yincrease = GetFontHeight(hFont) * num_lines - (edit_rect.bottom - edit_rect.top);
	
	GetWindowRect(hParent, &dlg_rect);
	yincrease = min(yincrease, GetSystemMetrics(SM_CYSCREEN) - dlg_rect.bottom + dlg_rect.top);
	yincrease = max(0, yincrease);
	MoveWindow(hParent, dlg_rect.left, dlg_rect.top, dlg_rect.right - dlg_rect.left, 
		dlg_rect.bottom - dlg_rect.top + yincrease, FALSE);
}

static void SetLookPageButtons(HWND hDlg, DescDialogStruct *info)
{
	HWND hPrev = GetDlgItem(hDlg,IDC_PREV);
	HWND hNext = GetDlgItem(hDlg,IDC_NEXT);
	
	if (info->numPages < 2)
	{
		DestroyWindow(hPrev);
		DestroyWindow(hNext);
	}
	else
	{
		if (info->currentPage == 0)
		{
			if (hPrev == GetFocus())
				SetFocus(hNext);
			EnableWindow(hPrev,FALSE);
			EnableWindow(hNext,TRUE);
		}
		else if (info->currentPage == info->numPages-1)
		{
			if (hNext == GetFocus())
				SetFocus(hPrev);
			EnableWindow(hPrev,TRUE);
			EnableWindow(hNext,FALSE);
		}
		else
		{
			EnableWindow(hPrev,TRUE);
			EnableWindow(hNext,TRUE);
		}
	}
}

/*****************************************************************************/
/*
* DescDialogProc:  Dialog procedure for dialog containing an
*   item's long description.
*/
BOOL CALLBACK DescDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	DescDialogStruct *info;
	static HWND hwndBitmap;
	static Bool changed;   // True when player has changed description
	HWND hEdit, hwndOK, hURL, hFixed;
	HDC hdc;
	HFONT hFont;
	RECT dlg_rect, edit_rect, fixed_rect;
	AREA area;
	char *desc, *str, url[MAX_URL + 1], *pPageBreak;
	DRAWITEMSTRUCT *lpdis;
	char descriptionBuffer[MAX_PAGE_DESCRIPTION_TEXT+1];
	int height;
	
	info = (DescDialogStruct *) GetWindowLong(hDlg, DWL_USER);
	
	switch (message)
	{
	case WM_INITDIALOG:
		info = (DescDialogStruct *) lParam;
		/* Store structure in dialog box's extra bytes */
		SetWindowLong(hDlg, DWL_USER, (long) info);
		
		hwndBitmap = GetDlgItem(hDlg, IDC_DESCBITMAP);
		hFixed = GetDlgItem(hDlg, IDC_DESCFIXED);
		hEdit = GetDlgItem(hDlg, IDC_DESCBOX);
		hwndOK = GetDlgItem(hDlg, IDOK);
		
		// Item Name.
		height = SetFontToFitText(info,GetDlgItem(hDlg, IDC_DESCNAME), 
			(int)FONT_TITLES, info->name);
		SetDlgItemText(hDlg, IDC_DESCNAME, info->name);
		hdc = GetDC(hDlg);
		SetTextColor(hdc,GetPlayerNameColor(info->obj,info->name));
		ReleaseDC(hDlg,hdc);
		
		// Item Description.
		hFont = GetFont(FONT_EDIT);
		SetWindowFont(hFixed, hFont, FALSE);
		SetWindowFont(hEdit, hFont, FALSE);
		
		str = info->description;
		if (str)
		{
			pPageBreak = strchr(str,PAGE_BREAK_CHAR);
			while (pPageBreak)
			{
				info->numPages++;
				str = pPageBreak+1;
				pPageBreak = strchr(str,PAGE_BREAK_CHAR);
			}
			info->numPages++;
			GetPageText(descriptionBuffer,info);
			Edit_SetText(hEdit, descriptionBuffer);
		}
		
		// Show fixed string, if appropriate
		if (info->fixed_string != NULL)
		{
			SetDlgItemText(hDlg, IDC_DESCFIXED, info->fixed_string);
			SetWindowFont(GetDlgItem(hDlg, IDC_DESCFIXED), GetFont(FONT_EDIT), FALSE);
		}
		
		// Can this player edit this object's description?
		if (info->flags & DF_EDITABLE)
			Edit_SetReadOnly(hEdit, FALSE);
		Edit_LimitText(hEdit, MAX_DESCRIPTION);
		if (!(info->flags & (DF_EDITABLE | DF_INSCRIBED)) && !str)
		{
			GetWindowRect(hFixed, &fixed_rect);
			GetWindowRect(hEdit, &edit_rect);
			UnionRect(&fixed_rect, &fixed_rect, &edit_rect);
			ScreenToClient(hDlg, (LPPOINT)(&fixed_rect));
			ScreenToClient(hDlg, (LPPOINT)(&fixed_rect)+1);
			MoveWindow(hFixed, fixed_rect.left, fixed_rect.top,
				fixed_rect.right - fixed_rect.left,
				fixed_rect.bottom - fixed_rect.top, FALSE);
			ShowWindow(hEdit, SW_HIDE);
		}
		
		// Resize and move dialog controls
		GetWindowRect(hDlg, &dlg_rect);
		if (GetWindowLong(hEdit, GWL_STYLE) & WS_VISIBLE)
		{
			ShowWindow(hEdit, SW_HIDE);
			ResizeEditToFitText(hFixed, hFont);
			ResizeDialog(hDlg, &dlg_rect, desc_controls);
			ShowWindow(hEdit, SW_SHOW);
		}
		else
		{
			ResizeEditToFitText(hFixed, hFont);
			ResizeDialog(hDlg, &dlg_rect, desc_controls);
		}
		if (GetWindowLong(hEdit, GWL_STYLE) & WS_VISIBLE)
		{
			GetWindowRect(hFixed, &fixed_rect);
			GetWindowRect(hEdit, &edit_rect);
			height = fixed_rect.bottom-edit_rect.top;
			ScreenToClient(hDlg, (LPPOINT)(&edit_rect));
			ScreenToClient(hDlg, (LPPOINT)(&edit_rect)+1);
			ShowWindow(hFixed, SW_HIDE);
			OffsetRect(&edit_rect, 0, height);
			MoveWindow(hEdit, edit_rect.left, edit_rect.top,
				edit_rect.right - edit_rect.left,
				edit_rect.bottom - edit_rect.top, FALSE);
			GetWindowRect(hDlg, &dlg_rect);
			ResizeEditToFitText(hEdit, hFont);
			ResizeDialog(hDlg, &dlg_rect, desc_controls);
			ShowWindow(hFixed, SW_SHOW);
		}
		
		CenterWindow(hDlg, GetParent(hDlg));
		
		// Show URL, if appropriate
		if (info->url != NULL)
		{
			hURL = GetDlgItem(hDlg, IDC_URL);
			
			SetWindowText(hURL, info->url);
			SetWindowFont(hURL, GetFont(FONT_EDIT), FALSE);
			
			if (info->flags & DF_EDITABLE)
				Edit_SetReadOnly(hURL, FALSE);
			
			Edit_LimitText(hURL, MAX_URL);
		}
		
		// Show appropriate buttons
		if (!(desc_flags & DESC_GET))
			DestroyWindow(GetDlgItem(hDlg, IDC_GET));
		if (!(desc_flags & DESC_DROP))
			DestroyWindow(GetDlgItem(hDlg, IDC_DROP));
		if (!(desc_flags & DESC_USE))
			DestroyWindow(GetDlgItem(hDlg, IDC_USE));
		if (!(desc_flags & DESC_UNUSE))
			DestroyWindow(GetDlgItem(hDlg, IDC_UNUSE));
		if (!(desc_flags & DESC_INSIDE))
			DestroyWindow(GetDlgItem(hDlg, IDC_INSIDE));
		if (!(desc_flags & DESC_ACTIVATE))
			DestroyWindow(GetDlgItem(hDlg, IDC_ACTIVATE));
		if (!(desc_flags & DESC_APPLY))
			DestroyWindow(GetDlgItem(hDlg, IDC_APPLY));
		
		SetLookPageButtons(hDlg, info);
#if 0
		if (info->numPages < 2)
		{
			HWND hwnd = GetDlgItem(hDlg,IDC_NEXT);
			if (hwnd)
				DestroyWindow(GetDlgItem(hDlg, IDC_NEXT));
			hwnd = GetDlgItem(hDlg,IDC_PREV);
			if (hwnd)
				DestroyWindow(GetDlgItem(hDlg, IDC_PREV));
		}
		else 
		{
			HWND hwnd = GetDlgItem(hDlg,IDC_PREV);
			if (hwnd)
				EnableWindow(hwnd,FALSE);
		}
#endif
		SetFocus(hwndOK);
		hDescDialog = hDlg;
		changed = False;
		return FALSE;
		
   case WM_PAINT:
	   InvalidateRect(hwndBitmap, NULL, TRUE);
	   UpdateWindow(hwndBitmap);
	   /* fall through */
	   
   case BK_ANIMATE:
	   /* Draw object's bitmap */
	   hdc = GetDC(hwndBitmap);
	   GetClientRect(hwndBitmap, &dlg_rect);
	   
	   RectToArea(&dlg_rect, &area);
	   DrawStretchedObjectGroup(hdc, info->obj, info->obj->animate->group, &area, 
		   GetSysColorBrush(COLOR_3DFACE));
	   
	   ReleaseDC(hwndBitmap, hdc);
	   break;
	   
	   HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, DialogCtlColor);
	   HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, DialogCtlColor);
	   HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, DialogCtlColor);
	   HANDLE_MSG(hDlg, WM_CTLCOLORDLG, DialogCtlColor);
	   
   case WM_DESTROY:
	   hDescDialog = NULL;
	   return TRUE;   
	   
   case WM_DRAWITEM:
	   // Draw player name in color that reflects murderer status
	   lpdis = (DRAWITEMSTRUCT *) lParam;
	   switch (lpdis->itemAction)
       {
       case ODA_SELECT:
       case ODA_DRAWENTIRE:
		   SelectPalette(lpdis->hDC, hPal, FALSE);
		   hFont = info->hFontTitle;
		   SelectObject(lpdis->hDC, hFont);
		   SetBkMode(lpdis->hDC, TRANSPARENT);
		   str = LookupNameRsc(info->obj->name_res);
		   SetTextColor(lpdis->hDC, NAME_COLOR_NORMAL_BG);
		   DrawText(lpdis->hDC, str, strlen(str), &lpdis->rcItem, DT_LEFT | DT_VCENTER | DT_NOPREFIX);
		   OffsetRect(&lpdis->rcItem, -1, -1);
		   SetTextColor(lpdis->hDC, GetPlayerNameColor(info->obj, info->name));
		   DrawText(lpdis->hDC, str, strlen(str), &lpdis->rcItem, DT_LEFT | DT_VCENTER | DT_NOPREFIX);
		   
		   break;
       }
	   return TRUE;
	   
	   case WM_COMMAND:
		   switch(GET_WM_COMMAND_ID(wParam, lParam))
		   {
		   case IDC_PREV:
			   if (info->currentPage > 0)
				   info->currentPage--;
			   GetPageText(descriptionBuffer,info);
			   SetLookPageButtons(hDlg, info);
			   Edit_SetText(GetDlgItem(hDlg, IDC_DESCBOX), descriptionBuffer);
#if 0
			   EnableWindow(GetDlgItem(hDlg,IDC_PREV),info->currentPage > 0);
			   EnableWindow(GetDlgItem(hDlg,IDC_NEXT),info->currentPage < info->numPages-1);
#endif
			   return TRUE;
			   
		   case IDC_NEXT:
			   if (info->currentPage < info->numPages-1)
				   info->currentPage++;
			   GetPageText(descriptionBuffer,info);
			   Edit_SetText(GetDlgItem(hDlg, IDC_DESCBOX), descriptionBuffer);
			   SetLookPageButtons(hDlg, info);
#if 0
			   EnableWindow(GetDlgItem(hDlg,IDC_PREV),info->currentPage > 0);
			   EnableWindow(GetDlgItem(hDlg,IDC_NEXT),info->currentPage < info->numPages-1);
#endif
			   return TRUE;
			   
		   case IDC_GET:
			   RequestPickup(info->obj->id);
			   EndDialog(hDlg, 0);
			   return TRUE;
			   
		   case IDC_DROP:
			   // Drop all of number items
			   info->obj->temp_amount = info->obj->amount;
			   RequestDrop(info->obj);
			   EndDialog(hDlg, 0);
			   return TRUE;
			   
		   case IDC_USE:
			   // If player isn't holding the object (i.e. there's a Get button), pick object up first
			   if (IsWindowVisible(GetDlgItem(hDlg, IDC_GET)))
				   RequestPickup(info->obj->id);
			   RequestUse(info->obj->id);
			   EndDialog(hDlg, 0);
			   return TRUE;
			   
		   case IDC_UNUSE:
			   RequestUnuse(info->obj->id);
			   EndDialog(hDlg, 0);
			   return TRUE;
			   
		   case IDC_INSIDE:
			   RequestObjectContents(info->obj->id);
			   EndDialog(hDlg, 0);
			   return TRUE;
			   
		   case IDC_ACTIVATE:
			   RequestActivate(info->obj->id);
			   EndDialog(hDlg, 0);
			   return TRUE;
			   
		   case IDC_APPLY:
			   StartApply(info->obj->id);
			   EndDialog(hDlg, 0);
			   return TRUE;
			   
		   case IDC_DESCBOX:
			   if (GET_WM_COMMAND_CMD(wParam, lParam) != EN_CHANGE)
				   break;
			   changed = True;
			   return TRUE;
			   
		   case IDC_URLBUTTON:
			   hURL = GetDlgItem(hDlg, IDC_URL);
			   Edit_GetText(hURL, url, MAX_URL);
			   WebLaunchBrowser(url);
			   return TRUE;
			   
		   case IDOK:
			   
			   // Send new description if changed
			   if (changed)
			   {
				   desc = (char *) SafeMalloc(MAX_DESCRIPTION + 1);
				   GetDlgItemText(hDlg, IDC_DESCBOX, desc, MAX_DESCRIPTION);
				   RequestChangeDescription(info->obj->id, desc);
				   SafeFree(desc);
			   }
			   
			   // Send new URL if changed
			   if (info->url != NULL)
			   {
				   GetDlgItemText(hDlg, IDC_URL, url, MAX_URL);
				   if (strcmp(url, info->url))
					   RequestChangeURL(player.id, url);
			   }
			   
			   // FALLTHRU
			   
		   case IDCANCEL:
			   SetWindowFont(GetDlgItem(hDlg,IDC_DESCNAME), GetFont(FONT_TITLES), FALSE);
			   if (info->hFontTitle && info->hFontTitle != GetFont(FONT_TITLES))
				   DeleteObject(info->hFontTitle);
			   info->hFontTitle = NULL;
			   
			   EndDialog(hDlg, IDOK == GET_WM_COMMAND_ID(wParam, lParam));
			   return TRUE;
      }
      break;
   }
   
   return FALSE;
}
/************************************************************************/
/* 
* AnimateDescription:  Animate bitmap in description dialog.
*   dt is number of milliseconds since last time animation timer went off.
*/
void AnimateDescription(int dt)
{
	int old_group;
	DescDialogStruct *info;
	object_node *obj;
	
	if (hDescDialog == NULL)
		return;
	
	info = (DescDialogStruct *) GetWindowLong(hDescDialog, DWL_USER);
	obj = info->obj;
	
	old_group = obj->animate->group;
	
	if (AnimateObject(obj, dt) == True)
		SendMessage(hDescDialog, BK_ANIMATE, 0, 0);
}
/*****************************************************************************/
/*
* AmountDialogProc:  Dialog procedure for dialog asking user for a number.
*/
BOOL CALLBACK AmountDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	static AmountDialogStruct *info;
	static HWND hEdit;
	DWORD amount;
	RECT dlg_rect;
	RECT parent_rect;
	char buf[MAXAMOUNT + 1];
	HWND hwndSpin;
	
	switch (message)
	{
	case WM_INITDIALOG:
		hAmountDialog = hDlg;
		info = (AmountDialogStruct *) lParam;
		hEdit = GetDlgItem(hDlg, IDC_AMOUNTBOX);      
		
		GetWindowRect(hDlg, &dlg_rect);
		GetWindowRect(GetParent(hDlg), &parent_rect);
		
		// Position dialog.
		// Don't let it extend past the edges of the parent.
		//
		OffsetRect(&dlg_rect, info->x - dlg_rect.left, info->y - dlg_rect.top);
		if (dlg_rect.right > parent_rect.right)
			OffsetRect(&dlg_rect, parent_rect.right - dlg_rect.right, 0);
		if (dlg_rect.bottom > parent_rect.bottom)
			OffsetRect(&dlg_rect, 0, parent_rect.bottom - dlg_rect.bottom);
		if (dlg_rect.left < parent_rect.left)
			OffsetRect(&dlg_rect, parent_rect.left - dlg_rect.left, 0);
		if (dlg_rect.top < parent_rect.top)
			OffsetRect(&dlg_rect, 0, parent_rect.top - dlg_rect.top);
		MoveWindow(hDlg, dlg_rect.left, dlg_rect.top, dlg_rect.right - dlg_rect.left, 
			dlg_rect.bottom - dlg_rect.top, FALSE);
		
		sprintf(buf, "%u", info->amount);
		Edit_SetText(hEdit, buf);
		Edit_LimitText(hEdit, MAXAMOUNT);
		
		hwndSpin = GetDlgItem(hDlg, IDC_SPIN1);
		if (info->maxAmount > UD_MAXVAL)
			SendMessage(hwndSpin,UDM_SETRANGE,0,MAKELONG(UD_MAXVAL,info->minAmount));
		else
			SendMessage(hwndSpin,UDM_SETRANGE,0,MAKELONG(info->maxAmount,info->minAmount));
		
		return TRUE;
		
		HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, DialogCtlColor);
		HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, DialogCtlColor);
		HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, DialogCtlColor);
		HANDLE_MSG(hDlg, WM_CTLCOLORDLG, DialogCtlColor);
		
	case WM_COMMAND:
		switch(GET_WM_COMMAND_ID(wParam, lParam))
		{
		case IDOK:
			
			// changed from %d (long) to %u (DWORD) for 'amount'
			//   (1) avoids signed/unsigned mismatch
			//   (2) doesn't need special <= 0 case
			
			Edit_GetText(hEdit, buf, MAXAMOUNT);
			if (sscanf(buf, "%u", &amount) != 1)
				EndDialog(hDlg, IDCANCEL);
			else
			{
				if (amount < info->minAmount)
					info->amount = info->minAmount;
				else if (amount > info->maxAmount)
					info->amount = info->maxAmount;
				else
					info->amount = amount;
				EndDialog(hDlg, IDOK);
			}
			return TRUE;
			
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		break;
		
		case WM_DESTROY:
			hAmountDialog = NULL;
			return TRUE;
	}
	
	return FALSE;
}
/************************************************************************/
/*
* SetDescParams:  Set parameters which tell how to display description dialog.
*   This is necessary because the dialog is displayed in response to a 
*   server message, so the parent isn't known at the time of dialog creation.
*/
void SetDescParams(HWND hParent, int flags)
{
	hDescParent = hParent;
	desc_flags = flags;
}
/************************************************************************/
/*
* DisplayDescription:  Display given object's description.
*   Caller must ensure that obj is not deleted while dialog is up.
*   flags gives info on what user can do with dialog.
*   hDescParent global is used as parent for this dialog.
*   extra_string and url are used only in player descriptions.
*/
void DisplayDescription(object_node *obj, BYTE flags, char *description, 
                        char *extra_string, char *url)
{
	DescDialogStruct info;
	int template_id;
	
	if (hDescDialog != NULL)
	{
		debug(("Attempted to create multiple description dialogs\n"));
		return;
	}
	
	ZeroMemory(&info,sizeof(info));
	info.obj          = obj;
	info.flags        = flags;
	info.name         = LookupNameRsc(obj->name_res);
	info.description  = description;
	info.fixed_string = extra_string;
	info.url          = url;
	
	// Different dialog for players
	template_id = (obj->flags & OF_PLAYER) ? IDD_DESCPLAYER : IDD_DESC;
	
	DialogBoxParam(hInst, MAKEINTRESOURCE(template_id), hDescParent,
                 DescDialogProc, (LPARAM) &info);
	
	TooltipReset();
	SetDescParams(NULL, DESC_NONE);
}

/*****************************************************************************/
/*
* AbortGameDialogs:  Close modal dialogs, for example when we lose the server
*   connection.
*/
void AbortGameDialogs(void)
{
	AbortBuyDialog();
	if (hDescDialog != NULL)
		SendMessage(hDescDialog, WM_COMMAND, IDCANCEL, 0);
	if (hAmountDialog != NULL)
		SendMessage(hAmountDialog, WM_COMMAND, IDCANCEL, 0);
	AbortWhoDialog();
	AbortAnnotateDialog();
	AbortPasswordDialog();
	AbortPreferencesDialog();
}
