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

static const unsigned char PAGE_BREAK_CHAR = 0xB6;	  /* U+00B6 PILCROW SIGN For multi-page descriptions */
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
	{ IDC_APPLY,       RDI_BOTTOM },
	{ IDC_ACTIVATE,    RDI_BOTTOM },
	{ IDC_URLLABEL,    RDI_BOTTOM },
	{ IDC_URLBUTTON,   RDI_BOTTOM },
	{ IDC_URL,         RDI_BOTTOM },
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
	char *pFullText;
	char *pStart;
	char *pEnd;
	char *pEndDescription;
	
	if (!buffer)
		return;
	
	buffer[0] = '\0';
	if (!info || !info->description)
		return;
	
	size_t lenDescription = strlen(info->description);
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

/*
 * ResizeEditToFitText:  Resizes the parent dialog so the edit control is
 *   tall enough to display all its content without a vertical scrollbar.
 */
static void ResizeEditToFitText(HWND hEdit, HFONT hFont)
{
	HWND hParent = GetParent(hEdit);
	
	RECT edit_rect, dlg_rect;
	int yincrease;
	
	if (!(GetWindowLong(hEdit, GWL_STYLE) & WS_VISIBLE))
		return;
	
	GetWindowRect(hEdit, &edit_rect);
	int origW = edit_rect.right - edit_rect.left;
	int origH = edit_rect.bottom - edit_rect.top;
	int textLen = GetWindowTextLength(hEdit);

	if (textLen == 0)
	{
		yincrease = GetFontHeight(hFont) - origH;
	}
	else
	{
		POINT editPos = { edit_rect.left, edit_rect.top };
		ScreenToClient(hParent, &editPos);

		/* Temporarily expand to screen height so EM_POSFROMCHAR returns
		 * unclipped coordinates. Width stays the same so text doesn't reflow. */
		int screenH = GetSystemMetrics(SM_CYSCREEN);
		MoveWindow(hEdit, editPos.x, editPos.y, origW, screenH, FALSE);

		// Get Y position of last character.
		POINTL pt;
		SendMessage(hEdit, EM_POSFROMCHAR, (WPARAM)&pt, textLen - 1);

		// Measure last line height from the distance between adjacent lines.
		int lastLineHeight = GetFontHeight(hFont);
		int lastLine = (int)SendMessage(hEdit, EM_LINEFROMCHAR, textLen - 1, 0);
		if (lastLine > 0)
		{
			int lastIdx = (int)SendMessage(hEdit, EM_LINEINDEX, lastLine, 0);
			int prevIdx = (int)SendMessage(hEdit, EM_LINEINDEX, lastLine - 1, 0);
			POINTL ptPrev, ptLast;
			SendMessage(hEdit, EM_POSFROMCHAR, (WPARAM)&ptPrev, prevIdx);
			SendMessage(hEdit, EM_POSFROMCHAR, (WPARAM)&ptLast, lastIdx);
			int measured = ptLast.y - ptPrev.y;
			if (measured > lastLineHeight)
				lastLineHeight = measured;
		}

		/* Content height = last char Y + last line height + padding.
		 * Padding covers dySpaceAfter on the last paragraph and text descent. */
		int padding = GetFontHeight(hFont);
		int contentBottom = pt.y + lastLineHeight + padding;

		// Account for formatting margins within the control.
		RECT fmtRect, clientRect;
		SendMessage(hEdit, EM_GETRECT, 0, (LPARAM)&fmtRect);
		GetClientRect(hEdit, &clientRect);
		int bottomMargin = std::max(0, (int)(clientRect.bottom - fmtRect.bottom));

		// Non-client height (borders) from the expanded state.
		RECT winRect;
		GetWindowRect(hEdit, &winRect);
		int nonClientH = (winRect.bottom - winRect.top)
			- (clientRect.bottom - clientRect.top);
		int neededWindowH = contentBottom + bottomMargin + nonClientH;

		// Restore original size.
		MoveWindow(hEdit, editPos.x, editPos.y, origW, origH, FALSE);

		yincrease = neededWindowH - origH;
	}

	GetWindowRect(hParent, &dlg_rect);
	yincrease = std::min(yincrease, (int)(GetSystemMetrics(SM_CYSCREEN)
		- dlg_rect.bottom + dlg_rect.top));
	yincrease = std::max(0, yincrease);
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
/************************************************************************/
/*
 * ResetRichEditFormatting:  Reset all character formatting in a Rich Edit
 *   control to plain text defaults (color, size, no bold/italic/underline).
 */
static void ResetRichEditFormatting(HWND hEdit)
{
	CHARFORMAT cformat;
	memset(&cformat, 0, sizeof(cformat));
	cformat.cbSize = sizeof(cformat);
	cformat.dwMask = CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_SIZE;
	cformat.crTextColor = GetColor(COLOR_EDITFGD);
	cformat.yHeight = MD_DEFAULT_FONT_TWIPS;
	cformat.dwEffects = 0;
	SendMessage(hEdit, EM_SETSEL, 0, -1);
	SendMessage(hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cformat);
	SendMessage(hEdit, EM_SETSEL, -1, -1);
}
/*****************************************************************************/
/*
* DescDialogProc:  Dialog procedure for dialog containing an
*   item's long description.
*/
INT_PTR CALLBACK DescDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DescDialogStruct *info;
	static HWND hwndBitmap;
	static bool changed;   // true when player has changed description
	static bool updatingText; // Guard flag to prevent EN_CHANGE during programmatic updates
	static char rawDescription[MAX_PAGE_DESCRIPTION_TEXT+1]; // Raw text with format codes
	HWND hEdit, hwndOK, hURL, hFixed;
	HDC hdc;
	HFONT hFont;
	RECT dlg_rect, edit_rect, fixed_rect;
	AREA area;
	char *str, url[MAX_URL + 1], *pPageBreak;
	DRAWITEMSTRUCT *lpdis;
	char descriptionBuffer[MAX_PAGE_DESCRIPTION_TEXT+1];
	int height;
	
	info = (DescDialogStruct *) GetWindowLongPtr(hDlg, DWLP_USER);
	
	switch (message)
	{
	case WM_INITDIALOG:
		info = (DescDialogStruct *) lParam;
		/* Store structure in dialog box's extra bytes */
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) info);
		
		hwndBitmap = GetDlgItem(hDlg, IDC_DESCBITMAP);
		hFixed = GetDlgItem(hDlg, IDC_DESCFIXED);
		hEdit = GetDlgItem(hDlg, IDC_DESCBOX);
		hwndOK = GetDlgItem(hDlg, IDOK);

		// Rich Edit controls require ENM_CHANGE to send EN_CHANGE notifications.
		SendMessage(hEdit, EM_SETEVENTMASK, 0, ENM_CHANGE);
		
		// Item Name.
		height = SetFontToFitText(info,GetDlgItem(hDlg, IDC_DESCNAME), 
			(int)FONT_TITLES, info->name.c_str());
		SetDlgItemText(hDlg, IDC_DESCNAME, info->name.c_str());
		hdc = GetDC(hDlg);
		SetTextColor(hdc,GetPlayerNameColor(info->obj->flags,info->name.c_str()));
		ReleaseDC(hDlg,hdc);
		
		// Item Description.
		hFont = GetFont(FONT_EDIT);
		SetWindowFont(hFixed, hFont, FALSE);
		SetWindowFont(hEdit, hFont, FALSE);

		// Rich Edit controls don't respond to WM_CTLCOLOREDIT; set colors directly.
		SendMessage(hEdit, EM_SETBKGNDCOLOR, FALSE, GetColor(COLOR_EDITBGD));
		SendMessage(hFixed, EM_SETBKGNDCOLOR, FALSE, GetSysColor(COLOR_3DFACE));
		{
			CHARFORMAT cformat;
			memset(&cformat, 0, sizeof(cformat));
			cformat.cbSize = sizeof(cformat);
			cformat.dwMask = CFM_COLOR;
			cformat.crTextColor = GetColor(COLOR_EDITFGD);
			SendMessage(hEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cformat);
			SendMessage(hFixed, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cformat);
		}
		
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
			strncpy(rawDescription, descriptionBuffer, MAX_PAGE_DESCRIPTION_TEXT);
			rawDescription[MAX_PAGE_DESCRIPTION_TEXT] = '\0';
			updatingText = true;
			RichEditSetMarkdownText(hEdit, descriptionBuffer,
				GetColor(COLOR_EDITFGD), MD_RENDER_FULL);
			updatingText = false;
		}
		
		// Show fixed string, if appropriate
		if (info->fixed_string != NULL)
		{
			updatingText = true;
			RichEditSetMarkdownText(GetDlgItem(hDlg, IDC_DESCFIXED),
				info->fixed_string, GetColor(COLOR_EDITFGD), MD_RENDER_FULL);
			updatingText = false;
		}
		
		// Can this player edit this object's description?
		if (info->flags & DF_EDITABLE)
		{
			HWND hCheckbox = GetDlgItem(hDlg, IDC_DESCEDIT);
			// Add WS_CLIPSIBLINGS so the owner-draw name control won't paint over us.
			SetWindowLong(hCheckbox, GWL_STYLE,
				GetWindowLong(hCheckbox, GWL_STYLE) | WS_CLIPSIBLINGS);
			SetWindowLong(GetDlgItem(hDlg, IDC_DESCNAME), GWL_STYLE,
				GetWindowLong(GetDlgItem(hDlg, IDC_DESCNAME), GWL_STYLE) | WS_CLIPSIBLINGS);
			// Bring checkbox to top of z-order and show it.
			SetWindowPos(hCheckbox, HWND_TOP,
				0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			/* Start in preview mode (checked + read-only) so the player
			 * sees their rendered description immediately. */
			CheckDlgButton(hDlg, IDC_DESCEDIT, BST_CHECKED);
			Edit_SetReadOnly(hEdit, TRUE);
		}
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
		SetFocus(hwndOK);
		hDescDialog = hDlg;
		changed = false;
		updatingText = false;
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
		   DrawText(lpdis->hDC, str, (int) strlen(str), &lpdis->rcItem, DT_LEFT | DT_VCENTER | DT_NOPREFIX);
		   OffsetRect(&lpdis->rcItem, -1, -1);
		   SetTextColor(lpdis->hDC, GetPlayerNameColor(info->obj->flags,info->name.c_str()));
		   DrawText(lpdis->hDC, str, (int) strlen(str), &lpdis->rcItem, DT_LEFT | DT_VCENTER | DT_NOPREFIX);
		   
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
			   strncpy(rawDescription, descriptionBuffer, MAX_PAGE_DESCRIPTION_TEXT);
			   rawDescription[MAX_PAGE_DESCRIPTION_TEXT] = '\0';
			   SetLookPageButtons(hDlg, info);
			   updatingText = true;
			   hEdit = GetDlgItem(hDlg, IDC_DESCBOX);
			   if (IsDlgButtonChecked(hDlg, IDC_DESCEDIT) == BST_CHECKED)
				   RichEditSetMarkdownText(hEdit,
					   descriptionBuffer, GetColor(COLOR_EDITFGD), MD_RENDER_FULL);
			   else
			   {
				   SetWindowText(hEdit, descriptionBuffer);
				   ResetRichEditFormatting(hEdit);
			   }
			   updatingText = false;
			   return TRUE;
			   
		   case IDC_NEXT:
			   if (info->currentPage < info->numPages-1)
				   info->currentPage++;
			   GetPageText(descriptionBuffer,info);
			   strncpy(rawDescription, descriptionBuffer, MAX_PAGE_DESCRIPTION_TEXT);
			   rawDescription[MAX_PAGE_DESCRIPTION_TEXT] = '\0';
			   updatingText = true;
			   hEdit = GetDlgItem(hDlg, IDC_DESCBOX);
			   if (IsDlgButtonChecked(hDlg, IDC_DESCEDIT) == BST_CHECKED)
				   RichEditSetMarkdownText(hEdit,
					   descriptionBuffer, GetColor(COLOR_EDITFGD), MD_RENDER_FULL);
			   else
			   {
				   SetWindowText(hEdit, descriptionBuffer);
				   ResetRichEditFormatting(hEdit);
			   }
			   updatingText = false;
			   SetLookPageButtons(hDlg, info);
			   return TRUE;
			   
		   case IDC_GET:
			   RequestPickup(info->obj->id);
			   EndDialog(hDlg, 0);
			   return TRUE;
			   
		   case IDC_DROP:
			   // Drop all of number items
			   info->obj->temp_amount = info->obj->amount;
			   {
			      list_type drop_list = list_add_item(NULL, info->obj);
			      RequestDrop(drop_list);
			      list_delete(drop_list);
			   }
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

		   case IDC_DESCEDIT:
			   hEdit = GetDlgItem(hDlg, IDC_DESCBOX);
			   updatingText = true;
			   if (IsDlgButtonChecked(hDlg, IDC_DESCEDIT) == BST_CHECKED)
			   {
				   // Switch to preview mode: render formatted text, read-only.
				   Edit_SetReadOnly(hEdit, TRUE);
				   RichEditSetMarkdownText(hEdit, rawDescription,
					   GetColor(COLOR_EDITFGD), MD_RENDER_FULL);
			   }
			   else
			   {
				   /* Switch to edit mode: show plain raw text.
				    * Clear link registry so stale links don't fire on raw text. */
				   MdClearLinks(hEdit);
				   Edit_SetReadOnly(hEdit, FALSE);
				   SetWindowText(hEdit, rawDescription);
				   ResetRichEditFormatting(hEdit);
			   }
			   updatingText = false;
			   return TRUE;

		   case IDC_DESCBOX:
			   if (GET_WM_COMMAND_CMD(wParam, lParam) != EN_CHANGE)
				   break;
			   if (updatingText)
				   return TRUE;
			   changed = true;
			   GetDlgItemText(hDlg, IDC_DESCBOX, rawDescription,
				   MAX_PAGE_DESCRIPTION_TEXT);
			   return TRUE;
			   
		   case IDC_URLBUTTON:
			   hURL = GetDlgItem(hDlg, IDC_URL);
			   Edit_GetText(hURL, url, MAX_URL);
			   WebLaunchBrowser(url);
			   return TRUE;
			   
		   case IDOK:
			   
			   /* If in preview mode (checked), rawDescription already has the latest raw text.
			    * If in edit mode (unchecked), grab current text from the control. */
			   if (IsDlgButtonChecked(hDlg, IDC_DESCEDIT) != BST_CHECKED)
			   {
				   GetDlgItemText(hDlg, IDC_DESCBOX, rawDescription,
					   MAX_PAGE_DESCRIPTION_TEXT);
			   }
			   
			   // Send new description if changed
			   if (changed)
			   {
				   RequestChangeDescription(info->obj->id, rawDescription);
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
	
	info = (DescDialogStruct *) GetWindowLongPtr(hDescDialog, DWLP_USER);
	obj = info->obj;
	
	old_group = obj->animate->group;
	
	if (AnimateObject(obj, dt) == true)
		SendMessage(hDescDialog, BK_ANIMATE, 0, 0);
}
/*****************************************************************************/
/*
* AmountDialogProc:  Dialog procedure for dialog asking user for a number.
*/
INT_PTR CALLBACK AmountDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
		
		snprintf(buf, sizeof(buf), "%u", info->amount);
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
                        char *fixed_string, char *url, item_rarity_grade rarity)
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
    char* nameCStr    = LookupNameRsc(obj->name_res);
    info.name         = std::string(nameCStr);
	info.description  = description;
	info.fixed_string = fixed_string;
	info.url          = url;
	info.rarity       = obj->rarity;
	
	std::string raritySuffix = GetRaritySuffix(info.rarity);

	if (!raritySuffix.empty())
	{
        info.name += " (" + raritySuffix + ")";
	}

	// Different dialog for players
	template_id = (obj->flags & OF_PLAYER) ? IDD_DESCPLAYER : IDD_DESC;
	
	SafeDialogBoxParam(hInst, MAKEINTRESOURCE(template_id), hDescParent,
                 DescDialogProc, reinterpret_cast<LPARAM>(&info));
	
	SetDescParams(NULL, DESC_NONE);
}
/************************************************************************/
/*
* GetRaritySuffix:  Provide a rarity suffix based on the given rarity type.
*/
std::string GetRaritySuffix(item_rarity_grade rarity)
{
	int stringID;

	switch(rarity)
	{
        case ITEM_RARITY_GRADE_NORMAL:
            return "";
        case ITEM_RARITY_GRADE_UNCOMMON:
            stringID = IDS_RARITY_GRADE_UNCOMMON;
			break;
        case ITEM_RARITY_GRADE_RARE:
            stringID = IDS_RARITY_GRADE_RARE;
			break;
        case ITEM_RARITY_GRADE_LEGENDARY:
            stringID = IDS_RARITY_GRADE_LEGENDARY;
			break;
        case ITEM_RARITY_GRADE_UNIDENTIFIED:
            stringID = IDS_RARITY_GRADE_UNIDENTIFIED;
			break;
        case ITEM_RARITY_GRADE_CURSED:
            stringID = IDS_RARITY_GRADE_CURSED;
			break;
		default:
			debug(("Unknown rarity grade %d\n", rarity));
            return ""; // Handle unknown rarity gracefully (no suffix)
	}

	return GetString(hInst, stringID);
}
/************************************************************************/
/*
* SetDialogFixedString:  Update the fixed string for the dialog that appears
* between the main name and description.
*/
void SetDialogFixedString(char* fixed_string)
{
	if (hDescDialog != NULL)
	{
		RichEditSetMarkdownText(GetDlgItem(hDescDialog, IDC_DESCFIXED),
			fixed_string, GetColor(COLOR_EDITFGD), MD_RENDER_FULL);
		InvalidateRect(GetDlgItem(hDescDialog, IDC_DESCFIXED), NULL, TRUE);
	}
}/************************************************************************/
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
/************************************************************************/
/*
* SafeDialogBoxParam:  A wrapper for DialogBoxParam that resets tooltips when a dialog is opened.
*   This is necessary because tooltips are not reset when a dialog is opened.
*/
INT_PTR SafeDialogBoxParam(HINSTANCE hInstance, LPCSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
    TooltipReset();
    return DialogBoxParam(hInstance, lpTemplate, hWndParent, lpDialogFunc, dwInitParam);
}
