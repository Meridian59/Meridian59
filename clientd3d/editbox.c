// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * editbox.c:  Handle edit box for game messages on main window.
 */

#include "client.h"

#define MAX_TEXT  30000         /* Max # of bytes in edit box */

static HWND hwndText;           /* Edit box's handle */
static WNDPROC lpfnDefEditProc; /* Default edit box message handler */

/* Position & size of edit box */
static AREA edit_area;

// When adding text, keep track of whether the player has scrolled back, so that
// we shouldn't scroll the added text into view.
static Bool scrolled_back;  

static keymap editbox_key_table[] = {
{ VK_TAB,         KEY_NONE,             A_TABFWD,   (void *) IDC_MAINTEXT },
{ VK_TAB,         KEY_SHIFT,            A_TABBACK,  (void *) IDC_MAINTEXT },
{ VK_ESCAPE,      KEY_ANY,              A_GOTOMAIN },
{0,0,0}// charlie: since the program looks for this i thought i`d add it, how odd
};

extern HPALETTE hPal;

/* local function prototypes */
static long CALLBACK EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
/************************************************************************/
/*
 * EditBoxCreate:  Create the edit box.
 */
void EditBoxCreate(HWND hParent)
{
   /* Don't use WS_VISIBLE style here--create window large enough so that 
    * scroll bar will be drawn, then size for real & make visible in ResizeEditBox.
    */
   hwndText = CreateWindowEx(0, "richedit", NULL, 
			   WS_CHILD | WS_VSCROLL | WS_VISIBLE |
			   ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
			   0, 0, 0, 0, 
			   hParent, (HMENU) IDC_MAINTEXT, hInst, NULL);

   EditBoxChangeColor();

   lpfnDefEditProc = SubclassWindow(hwndText, EditProc);
   SetWindowFont(hwndText, GetFont(FONT_EDIT), FALSE);
}
/************************************************************************/
/*
 * EditBoxDestroy:  Destroy the edit box.
 */
void EditBoxDestroy(void)
{
   DestroyWindow(hwndText);
   hwndText = NULL;
}
/************************************************************************/
/*
 * EditBoxResize:  Resize the edit box when the main window is resized
 *   to (xsize, ysize).  view is the current grid area view.
 */
void EditBoxResize(int xsize, int ysize, AREA view)
{
   edit_area.x = view.x;
//   edit_area.y = view.y + view.cy + 2 * HIGHLIGHT_THICKNESS;
   edit_area.y = view.y + view.cy + EDITAREA_TOP_GAP;

   edit_area.cx = view.cx;
   edit_area.cy = input_area.y - edit_area.y; // ysize - edit_area.y - BOTTOM_BORDER - GetTextInputHeight() - EDGETREAT_HEIGHT;

   MoveWindow(hwndText, edit_area.x, edit_area.y, 
	      edit_area.cx, edit_area.cy,
	      TRUE);

   WindowBeginUpdate(hwndText);
   EditBoxScroll(hwndText, True);  /* Recalculate # of lines in edit box */
   WindowEndUpdate(hwndText);

   /* Highlight will be turned on later */
}
/************************************************************************/
void EditBoxSetFocus(Bool forward) 
{
   TextInputSetFocus(forward);
}
/************************************************************************/
void EditBoxDrawBorder(void)
{
   TextInputDrawBorder();
}
/************************************************************************/
void EditBoxGetArea(AREA *a)
{
   memcpy(a, &edit_area, sizeof(edit_area));
}
/************************************************************************/
/*
 * EditProc:  Subclassed window procedure for edit box.
 */
long CALLBACK EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   int action;
   void *action_data;

   switch (message)
   {
   case WM_ERASEBKGND:
     SelectPalette((HDC) wParam, hPal, FALSE);
     break;

   case WM_VSCROLL:
     // Causes window to redraw itself, which in turn selects correct palette in 
     // WM_ERASEBKGND above.
     InvalidateRect(hwnd, NULL, TRUE);
     break;

   case WM_KEYDOWN:
      action = TranslateKey(wParam, editbox_key_table, &action_data);

      if (action != A_NOACTION)
	 PerformAction(action, action_data);
      break;

   case WM_SETFOCUS:
   case WM_KILLFOCUS:
      EditBoxDrawBorder();
      break;
   }
   return CallWindowProc(lpfnDefEditProc, hwnd, message, wParam, lParam);
}


/************************************************************************/
/*
 * EditBoxStartAdd:  Prepare to add text to the edit box.
 *   This function MUST be called in conjunction with EditBoxEndAdd.
 */
void EditBoxStartAdd(void)
{
   CHARFORMAT cformat;
   int txtlen;

   WindowBeginUpdate(hwndText);

   // See if user is looking at earlier text
   scrolled_back = !EditBoxLastVisible(hwndText);

   // Turn off current styles
   txtlen = Edit_GetTextLength(hwndText);

   // Add newline if not at start
   if (txtlen > 0)
   {
      Edit_SetSel(hwndText, txtlen, txtlen);
      Edit_ReplaceSel(hwndText, "\r\n");
   }

   txtlen = Edit_GetTextLength(hwndText);
   Edit_SetSel(hwndText, txtlen, txtlen);
   memset(&cformat, 0, sizeof(cformat));
   cformat.cbSize = sizeof(cformat);
   cformat.dwMask    |= CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;  // Turn these off
   SendMessage(hwndText, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cformat);
}
/************************************************************************/
/*
 * EditBoxEndAdd:  Done adding text to the edit box.
 *   This function MUST be called in conjunction with EditBoxStartAdd.
 */
void EditBoxEndAdd(void)
{
   /* Scroll new string into view */
   if (!scrolled_back || !config.scroll_lock)
      EditBoxScroll(hwndText, True);

   WindowEndUpdate(hwndText);
}
/************************************************************************/
/*
 * EditBoxAddText:  Add a message from the server to the text edit box.
 *   Message is displayed with given color and given style.
 */
void EditBoxAddText(char *message, int color, int style)
{
   int txtlen, msglen;

   txtlen = Edit_GetTextLength(hwndText);
   msglen = strlen(message);

   /* If box is full, get as much as we can fit.  +2 for CR/LF */
   if (txtlen + msglen + 2 > MAX_TEXT)
   {
      // XXX Split off at newline
      Edit_SetSel(hwndText, 0, txtlen + msglen + 2 - MAX_TEXT);
      Edit_ReplaceSel(hwndText, "");
      txtlen = Edit_GetTextLength(hwndText);
   }

   /* Append new message */
   Edit_SetSel(hwndText, txtlen, txtlen);

   if (config.colorcodes)
   {
      CHARFORMAT cformat;

      // Draw text in given color and style
      memset(&cformat, 0, sizeof(cformat));
      cformat.cbSize = sizeof(cformat);
      cformat.dwMask = CFM_COLOR;
      cformat.crTextColor = color;

      if (style == STYLE_NORMAL)
	 cformat.dwMask    |= CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;

      if (style & STYLE_BOLD)
      {
	 cformat.dwMask    |= CFM_BOLD;
	 cformat.dwEffects |= CFE_BOLD;
      }

      if (style & STYLE_ITALIC)
      {
	 cformat.dwMask    |= CFM_ITALIC;
	 cformat.dwEffects |= CFE_ITALIC;
      }

      if (style & STYLE_UNDERLINE)
      {
	 cformat.dwMask    |= CFM_UNDERLINE;
	 cformat.dwEffects |= CFE_UNDERLINE;
      }

      SendMessage(hwndText, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cformat);
   }

   Edit_ReplaceSel(hwndText, message);
}
/************************************************************************/
void EditBoxSetNormalFormat()
{
   CHARFORMAT cformat;

   if (!hwndText)
      return;

   // Draw text in given color and style
   memset(&cformat, 0, sizeof(cformat));
   cformat.cbSize = sizeof(cformat);
   cformat.crTextColor = RGB(0,0,0);
   cformat.dwMask |= CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;

   SendMessage(hwndText, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cformat);
}
/************************************************************************/
/*
 * EditBoxResetFont:  Set the main edit box's font, and recalculate the # of lines
 *   it contains.
 */
void EditBoxResetFont(void)
{
   SetWindowFont(hwndText, GetFont(FONT_EDIT), TRUE);

   /* Rescroll, since bigger font means fewer lines fit. */
   WindowBeginUpdate(hwndText);
   EditBoxScroll(hwndText, True);
   WindowEndUpdate(hwndText);
}
/************************************************************************/
/*
 * EditBoxChangeColor:  Called when edit box needs to reset its background color.
 */
void EditBoxChangeColor(void)
{
   SendMessage(hwndText, EM_SETBKGNDCOLOR, FALSE, GetColor(COLOR_MAINEDITBGD));
}

HWND EditBoxWindow(void)
{
   return hwndText;
}
