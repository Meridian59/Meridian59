// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * textin.c:  Deals with the text input area on the main window.
 */

#include "client.h"

/* Position & size of input box */
AREA input_area;

#define EDITBOX_HISTORY 20

static HWND hwndInput;            // Text input window handle
static WNDPROC lpfnDefInputProc;  // Default text box message handler
static int inputHeight = TEXTINPUT_HEIGHT;

static keymap textin_key_table[] = {
{ VK_TAB,         KEY_NONE,             A_TABFWD,    (void *) IDC_TEXTINPUT },
{ VK_TAB,         KEY_SHIFT,            A_TABBACK,   (void *) IDC_TEXTINPUT },
{ VK_ESCAPE,      KEY_ANY,              A_GOTOMAIN },
};

// True when we should ignore next selection message (used to override default
// combo box behavior).
static Bool skip_selection;       

extern HPALETTE hPal;
extern int border_index;

static long CALLBACK TextInputProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static Bool TextInputKey(HWND hwnd, UINT key, Bool fDown, int cRepeat, UINT flags);

static void CalculateWindowHeight(void)
{
   HDC hdc = GetDC(GetDesktopWindow());
   int oldMapMode = SetMapMode(hdc,MM_TEXT);
   HFONT hOldFont = (HFONT) SelectObject(hdc,GetFont(FONT_INPUT));
   TEXTMETRIC tm;

   GetTextMetrics(hdc,&tm);
   inputHeight = tm.tmHeight + tm.tmInternalLeading;
   SelectObject(hdc,hOldFont);
   SetMapMode(hdc,oldMapMode);
   ReleaseDC(GetDesktopWindow(),hdc);
}

int GetTextInputHeight(void)
{
   return inputHeight + 2*GetSystemMetrics(SM_CYEDGE);
}

/************************************************************************/
/*
 * TextInputCreate:  Create the text input box.
 */
void TextInputCreate(HWND hParent)
{
   HWND hwndEdit;

   CalculateWindowHeight();
   hwndInput = CreateWindow("combobox", NULL, 
			    WS_CHILD | WS_BORDER | WS_VISIBLE |
			    CBS_AUTOHSCROLL | CBS_DROPDOWN | WS_VSCROLL,
			    0, 0, 0, GetTextInputHeight(), 
			    hParent, (HMENU) IDC_TEXTINPUT, hInst, NULL);

   SetWindowFont(hwndInput, GetFont(FONT_INPUT), TRUE);
   CalculateWindowHeight();
   ComboBox_LimitText(hwndInput, MAXSAY);

   // The editbox is the first child of the combobox.
   // Subclass the editbox for key events.
   //
   hwndEdit = GetWindow(hwndInput, GW_CHILD);
   lpfnDefInputProc = SubclassWindow(hwndEdit, TextInputProc);
}
/************************************************************************/
/*
 * TextInputDestroy:  Destroy the edit box and the text buffer.
 */
void TextInputDestroy(void)
{
   DestroyWindow(hwndInput);
}

void TextInputResetFont(void)
{
   SetWindowFont(hwndInput, GetFont(FONT_INPUT), TRUE);
   CalculateWindowHeight();
}

/************************************************************************/
/*
 * TextInputResize:  Resize the text input box when the main window is resized
 *   to (xsize, ysize).  view is the current grid area view.
 */
void TextInputResize(int xsize, int ysize, AREA view)
{
   RECT rc;
   CalculateWindowHeight();
   input_area.x = view.x;
   input_area.y = ysize - EDGETREAT_HEIGHT - HIGHLIGHT_THICKNESS - GetTextInputHeight() - STATS_BOTTOM_GAP_HEIGHT;

   input_area.cx = view.cx + 2;					//	ajw added 2 pixels to line up with scrollbar
   input_area.cy = GetTextInputHeight();
   //MoveWindow(hwndEdit, input_area.x - 2, input_area.y,input_area.cx + 2, input_area.cy, TRUE);
   MoveWindow(hwndInput, input_area.x - 2, input_area.y,		//	ajw Moved left another 2 pixels...
	      input_area.cx + 2, input_area.cy * 6,
	      TRUE);
   SetRect(&rc,view.x-4,view.x + view.cx + 4,input_area.y - 2, input_area.y+input_area.cy+2);
   InvalidateRect(hMain,&rc,FALSE);
}

/************************************************************************/
void TextInputSetFocus(Bool forward)
{
   SetFocus(hwndInput);
   ComboBox_SetEditSel(hwndInput, 0, -1);  // select all text
}
/************************************************************************/
void TextInputDrawBorder(void)
{
   // Put border around edit box and text input box
   AREA a; //, edit_area;

//   EditBoxGetArea(&edit_area);
//   UnionArea(&a, &edit_area, &input_area);
   EditBoxGetArea(&a);
   a.cy += GetTextInputHeight();

//	Add space for edit treatment. Assuming same size as HIGHLIGHT_THICKNESS.
   a.x -= HIGHLIGHT_THICKNESS;
   a.y -= HIGHLIGHT_THICKNESS;
   a.cx += 2 * HIGHLIGHT_THICKNESS;
   a.cy += 2 * HIGHLIGHT_THICKNESS - 1; //There is no bottom treatment on editbox.

   if (IsChild(hwndInput, GetFocus()))
      DrawBorder(&a, HIGHLIGHT_INDEX, NULL);
   else 
      DrawBorder(&a, border_index, NULL);
}

/************************************************************************/
/*
 * TextInputSetText:  Set the contents of the text input box to the given string.
 *   If focus is True, set focus to text input box.
 */
void TextInputSetText(char *text, Bool focus)
{
   int len;

   ComboBox_SetText(hwndInput, text);
   len = ComboBox_GetTextLength(hwndInput);

   if (focus)
      SetFocus(hwndInput);

   ComboBox_SetEditSel(hwndInput, len, len);  // Move caret to end of text
}
/************************************************************************/
/*
 * TextInputProc:  Subclassed window procedure for text box.
 */
long CALLBACK TextInputProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_KEYDOWN:
      if (HANDLE_WM_KEYDOWN_BLAK(hwnd, wParam, lParam, TextInputKey) == True)
      	 return 0;
      break;

   case WM_CHAR:   // Avoid beep when Tab or Return or Escape pressed
      if (wParam == VK_TAB || wParam == VK_RETURN || wParam == VK_ESCAPE)
	 return 0;
      break;

   case WM_SETFOCUS:
   case WM_KILLFOCUS:
      skip_selection = True;
      TextInputDrawBorder();
      break;

   case EM_SETSEL:
      // Override default behavior of selecting all text when the control
      // gets the focus, or selecting (0, 0) when losing the focus.
      if (skip_selection)
      {
	 skip_selection = False;
	 return 0;
      }
      break;

   }
   return CallWindowProc(lpfnDefInputProc, hwnd, message, wParam, lParam);
}
/************************************************************************/
/*
 * TextInputKey:  User pressed a key on text input box.
 *   Return True iff key should NOT be passed on to Windows for default processing.
 */
Bool TextInputKey(HWND hwnd, UINT key, Bool fDown, int cRepeat, UINT flags)
{
   Bool held_down = (flags & 0x4000) ? True : False;  /* Is key being held down? */
   char string[MAXSAY + 1];
   int action;
   BOOL bValid;
   void *action_data;

   if (key == VK_RETURN && !held_down)
   {
      UserDidSomething();

      ComboBox_GetText(hwndInput, string, MAXSAY + 1);
      if (string[0] == 0)
	 return True;

      SetFocus(hMain);
      bValid = ParseGotText(string);

      // Add it to the history.
      if (*string && bValid)
      {
	 BOOL bAdd = TRUE;
	 int iCount;
	 char achHead[MAXSAY+1];
	 iCount = ComboBox_GetCount(hwndInput);
	 if (iCount > 0)
	 {
	    ComboBox_GetLBText(hwndInput, 0, achHead);
	    if (0 == strcmp(achHead, string))
	       bAdd = FALSE;
	 }
	 if (bAdd)
	 {
	    ComboBox_InsertString(hwndInput, 0, string);
	    if (iCount > EDITBOX_HISTORY)
	       ComboBox_DeleteString(hwndInput, iCount);
	 }
      }

      return True;
   }
   
   // Check for special keys
   action = TranslateKey(key, textin_key_table, &action_data);

   if (action == A_NOACTION)
      return False;

   PerformAction(action, action_data);

   return True;
}
