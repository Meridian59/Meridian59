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
static WNDPROC lpfnDefComboProc;  // Default combo box message handler
static WNDPROC lpfnDefInputProc;  // Default text box message handler
static int inputHeight = TEXTINPUT_HEIGHT;

static keymap textin_key_table[] = {
{ VK_TAB,         KEY_NONE,             A_TABFWD,    (void *) IDC_TEXTINPUT },
{ VK_TAB,         KEY_SHIFT,            A_TABBACK,   (void *) IDC_TEXTINPUT },
{ VK_ESCAPE,      KEY_ANY,              A_GOTOMAIN },
};

// true when we should ignore next selection message (used to override default
// combo box behavior).
static bool skip_selection;       

extern HPALETTE hPal;
extern int border_index;

static LRESULT CALLBACK TextInputComboProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK TextInputProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static bool TextInputKey(HWND hwnd, UINT key, bool fDown, int cRepeat, UINT flags);

/************************************************************************/
/*
 * ChatAreaHasFocus:  Returns true if focus is on the chat log (rich edit)
 *   or the text input (combo box or its child edit control).
 */
static bool ChatAreaHasFocus(void)
{
   HWND hFocus = GetFocus();
   return hFocus == EditBoxWindow()
      || hFocus == hwndInput
      || IsChild(hwndInput, hFocus);
}
/************************************************************************/
/*
 * GetTextInputBorderColor:  Returns the border color for the chat frame.
 */
static COLORREF GetTextInputBorderColor(bool focused)
{
   return focused ? RGB(130, 130, 135) : RGB(75, 75, 79);
}
/************************************************************************/
/*
 * GetTextInputButtonColor:  Returns the history button fill color.
 */
static COLORREF GetTextInputButtonColor(bool focused, bool pressed)
{
   if (pressed)
      return RGB(80, 80, 84);
   if (focused)
      return RGB(70, 70, 74);
   return RGB(55, 55, 59);
}
/************************************************************************/
/*
 * DrawComboBorderRect:  Repaint the combo box border and history button
 *   with theme colors so the default Win32 chrome is not visible.
 */
static void DrawComboBorderRect(HWND hwnd)
{
   RECT rect;
   RECT edge;
   RECT window_rect;
   RECT button_rect;
   HDC hdc;
   HBRUSH brush;
   HBRUSH button_brush;
   HBRUSH arrow_brush;
   HPEN arrow_pen;
   HPEN old_pen;
   HBRUSH old_brush;
   COMBOBOXINFO combo_info;
   COLORREF border_color;
   bool focused;
   bool pressed;
   POINT points[3];
   int center_x;
   int center_y;

   if (config.theme != THEME_DARK)
      return;

   hdc = GetWindowDC(hwnd);
   if (hdc == NULL)
      return;

   focused = ChatAreaHasFocus();
   border_color = GetTextInputBorderColor(focused);
   brush = CreateSolidBrush(border_color);
   if (brush == NULL)
   {
      ReleaseDC(hwnd, hdc);
      return;
   }

   GetWindowRect(hwnd, &rect);
   OffsetRect(&rect, -rect.left, -rect.top);

   edge.left = 0;
   edge.top = 0;
   edge.right = rect.right;
   edge.bottom = HIGHLIGHT_THICKNESS;
   FillRect(hdc, &edge, brush);

   edge.left = 0;
   edge.top = 0;
   edge.right = HIGHLIGHT_THICKNESS;
   edge.bottom = rect.bottom;
   FillRect(hdc, &edge, brush);

   edge.left = rect.right - HIGHLIGHT_THICKNESS;
   edge.top = 0;
   edge.right = rect.right;
   edge.bottom = rect.bottom;
   FillRect(hdc, &edge, brush);

   edge.left = 0;
   edge.top = rect.bottom - HIGHLIGHT_THICKNESS;
   edge.right = rect.right;
   edge.bottom = rect.bottom;
   FillRect(hdc, &edge, brush);

   combo_info.cbSize = sizeof(combo_info);
   if (GetComboBoxInfo(hwnd, &combo_info))
   {
      RECT client_rect;
      GetWindowRect(hwnd, &window_rect);
      GetClientRect(hwnd, &client_rect);
      MapWindowPoints(hwnd, NULL, (LPPOINT)&client_rect, 2);

      int nc_left = client_rect.left - window_rect.left;
      int nc_top = client_rect.top - window_rect.top;

      button_rect = combo_info.rcButton;
      OffsetRect(&button_rect, nc_left, nc_top);

      pressed = (combo_info.stateButton & STATE_SYSTEM_PRESSED) != 0;
      button_brush = CreateSolidBrush(GetTextInputButtonColor(focused, pressed));
      if (button_brush != NULL)
      {
         FillRect(hdc, &button_rect, button_brush);
         DeleteObject(button_brush);
      }

      edge.left = button_rect.left;
      edge.top = button_rect.top;
      edge.right = button_rect.left + HIGHLIGHT_THICKNESS;
      edge.bottom = button_rect.bottom;
      FillRect(hdc, &edge, brush);

      center_x = (button_rect.left + button_rect.right) / 2;
      center_y = (button_rect.top + button_rect.bottom) / 2 + (pressed ? 1 : 0);
      points[0].x = center_x - 4;
      points[0].y = center_y - 2;
      points[1].x = center_x + 4;
      points[1].y = center_y - 2;
      points[2].x = center_x;
      points[2].y = center_y + 3;

      arrow_brush = CreateSolidBrush(RGB(212, 212, 212));
      arrow_pen = CreatePen(PS_SOLID, 1, RGB(212, 212, 212));
      if (arrow_brush != NULL && arrow_pen != NULL)
      {
         old_brush = (HBRUSH)SelectObject(hdc, arrow_brush);
         old_pen = (HPEN)SelectObject(hdc, arrow_pen);
         SetBkMode(hdc, TRANSPARENT);
         Polygon(hdc, points, 3);
         SelectObject(hdc, old_pen);
         SelectObject(hdc, old_brush);
      }

      if (arrow_pen != NULL)
         DeleteObject(arrow_pen);
      if (arrow_brush != NULL)
         DeleteObject(arrow_brush);
   }

   DeleteObject(brush);
   ReleaseDC(hwnd, hdc);
}

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

   // Subclass the combo box for theme border painting.
   lpfnDefComboProc = SubclassWindow(hwndInput, TextInputComboProc);

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
 * TextInputRetheme:  Redraw the combo and child edit so dark control border
 *   changes are visible immediately.
 */
void TextInputRetheme(void)
{
   HWND hwndEdit;

   if (hwndInput == NULL)
      return;

   RedrawWindow(hwndInput, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);

   hwndEdit = GetWindow(hwndInput, GW_CHILD);
   if (hwndEdit != NULL)
      RedrawWindow(hwndEdit, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
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
void TextInputSetFocus(bool forward)
{
   SetFocus(hwndInput);
   skip_selection = false;
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

   bool focused = ChatAreaHasFocus();

   if (config.theme == THEME_DARK)
   {
      /* Fill the panel area behind the chat controls so the game
         background texture doesn't bleed through the narrow gaps
         between the outer border frame and the child windows.
         Exclude child windows so the fill doesn't paint over their
         content (hMain may not have WS_CLIPCHILDREN). */
      HDC hdc = GetDC(hMain);
      if (hdc != NULL)
      {
         /* SaveDC/RestoreDC is required because hMain uses CS_OWNDC,
            so GetDC returns the same persistent DC every time and
            ReleaseDC does NOT reset its state.  Without this,
            ExcludeClipRect permanently corrupts the clip region,
            causing D3D render holes and tearing on resize. */
         int savedDC = SaveDC(hdc);

         RECT rcChild;
         HWND hwndEdit = EditBoxWindow();
         if (hwndEdit != NULL)
         {
            GetWindowRect(hwndEdit, &rcChild);
            MapWindowPoints(NULL, hMain, (LPPOINT)&rcChild, 2);
            ExcludeClipRect(hdc, rcChild.left, rcChild.top,
                            rcChild.right, rcChild.bottom);
         }
         if (hwndInput != NULL)
         {
            GetWindowRect(hwndInput, &rcChild);
            MapWindowPoints(NULL, hMain, (LPPOINT)&rcChild, 2);
            ExcludeClipRect(hdc, rcChild.left, rcChild.top,
                            rcChild.right, rcChild.bottom);
         }

         HBRUSH bg = CreateSolidBrush(GetColor(COLOR_MAINEDITBGD));
         if (bg != NULL)
         {
            RECT fill;
            fill.left = a.x;
            fill.top = a.y;
            fill.right = a.x + a.cx;
            fill.bottom = a.y + a.cy;
            FillRect(hdc, &fill, bg);
            DeleteObject(bg);
         }

         RestoreDC(hdc, savedDC);
         ReleaseDC(hMain, hdc);
      }

      if (focused)
         DrawBorder(&a, HIGHLIGHT_INDEX, NULL);
      else
         DrawBorderRGB(&a, GetColor(COLOR_BGD), NULL);
      return;
   }

   if (focused)
      DrawBorder(&a, HIGHLIGHT_INDEX, NULL);
   else 
      DrawBorder(&a, border_index, NULL);
}

/************************************************************************/
/*
 * TextInputComboProc:  Subclassed combo box procedure for painting a dark
 *   border and history button in dark theme.
 */
LRESULT CALLBACK TextInputComboProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   LRESULT result;

   result = CallWindowProc(lpfnDefComboProc, hwnd, message, wParam, lParam);

   switch (message)
   {
   case WM_NCPAINT:
   case WM_PAINT:
   case WM_SETFOCUS:
   case WM_KILLFOCUS:
   case WM_ENABLE:
      DrawComboBorderRect(hwnd);
      break;
   }

   return result;
}

/************************************************************************/
/*
 * TextInputSetText:  Set the contents of the text input box to the given string.
 *   If focus is true, set focus to text input box.
 */
void TextInputSetText(char *text, bool focus)
{
   int len;

   ComboBox_SetText(hwndInput, text);
   len = ComboBox_GetTextLength(hwndInput);

   if (focus)
      SetFocus(hwndInput);

   skip_selection = false;
   ComboBox_SetEditSel(hwndInput, len, len);  // Move caret to end of text
}
/************************************************************************/
/*
 * TextInputProc:  Subclassed window procedure for text box.
 */
LRESULT CALLBACK TextInputProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_KEYDOWN:
      if (HANDLE_WM_KEYDOWN_BLAK(hwnd, wParam, lParam, TextInputKey) == true)
      	 return 0;
      break;

   case WM_CHAR:   // Avoid beep when Tab or Return or Escape pressed
      if (wParam == VK_TAB || wParam == VK_RETURN || wParam == VK_ESCAPE)
	 return 0;
      break;

   case WM_SETFOCUS:
   case WM_KILLFOCUS:
      skip_selection = true;
      TextInputDrawBorder();
      TextInputRetheme();
      break;

   case EM_SETSEL:
      // Override default behavior of selecting all text when the control
      // gets the focus, or selecting (0, 0) when losing the focus.
      if (skip_selection)
      {
	 skip_selection = false;
	 return 0;
      }
      break;

   }
   return CallWindowProc(lpfnDefInputProc, hwnd, message, wParam, lParam);
}
/************************************************************************/
/*
 * TextInputKey:  User pressed a key on text input box.
 *   Return true iff key should NOT be passed on to Windows for default processing.
 */
bool TextInputKey(HWND hwnd, UINT key, bool fDown, int cRepeat, UINT flags)
{
   bool held_down = (flags & 0x4000) ? true : false;  /* Is key being held down? */
   char string[MAXSAY + 1];
   int action;
   BOOL bValid;
   const void *action_data;

   if (key == VK_RETURN && !held_down)
   {
      UserDidSomething();

      ComboBox_GetText(hwndInput, string, MAXSAY + 1);
      if (string[0] == 0)
	 return true;

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

      return true;
   }
   
   // Check for special keys
   action = TranslateKey(key, textin_key_table, &action_data);

   if (action == A_NOACTION)
      return false;

   PerformAction(action, action_data);

   return true;
}
