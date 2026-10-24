/*
 * ctl_edit.c:  Wine's Edit on the layer: text round trips with Latin-1, typing,
 *   EN_CHANGE/EN_UPDATE/EN_SETFOCUS to an ANSI parent, limits, selection and
 *   replacement, multi-line lines and scrolling, read-only, passwords, the
 *   clipboard, the caret, an ANSI subclass, rendering of text and selection, and
 *   keys typed ahead of the message loop, and the key state they leave.
 */

#include "tests/ctl_common.h"

static WNDPROC old_edit_proc;
static int subclass_chars;
static WPARAM subclass_last_char;

static LRESULT CALLBACK EditSubclass(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_CHAR)
   {
      subclass_chars++;
      subclass_last_char = wparam;
      if (wparam == '#')
         return 0;   // eaten, as the client's textin.c does with some keys
   }
   return CallWindowProcA(old_edit_proc, hwnd, msg, wparam, lparam);
}

// The text when Return goes down, as the client's text bar reads it.
static char text_at_return[64];

static LRESULT CALLBACK ReturnSubclass(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_KEYDOWN && wparam == VK_RETURN)
      GetWindowTextA(hwnd, text_at_return, sizeof(text_at_return));
   return CallWindowProcA(old_edit_proc, hwnd, msg, wparam, lparam);
}

// What GetFocus() says while the edit loses the focus.
static HWND focus_at_kill;

static LRESULT CALLBACK KillFocusSubclass(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_KILLFOCUS)
      focus_at_kill = GetFocus();
   return CallWindowProcA(old_edit_proc, hwnd, msg, wparam, lparam);
}

static int Has(int code)
{
   for (int i = 0; i < rcv.code_count; i++)
      if (rcv.codes[i] == code)
         return 1;
   return 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
   HWND parent = MakeParent(400, 300);
   HFONT font = (HFONT) GetStockObject(DEFAULT_GUI_FONT);

   HWND edit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 10, 200,
                               22, parent, (HMENU) 201, instance, NULL);
   CHECK(edit != NULL);
   SendMessageA(edit, WM_SETFONT, (WPARAM) font, FALSE);
   CHECK((HFONT) SendMessageA(edit, WM_GETFONT, 0, 0) == font);

   // Text with Latin-1 characters.
   memset(&rcv, 0, sizeof(rcv));
   SetWindowTextA(edit, "Caf\xE9 M\xFCller Stra\xDF" "e");
   char text[256];
   GetWindowTextA(edit, text, sizeof(text));
   CHECK_STR(text, "Caf\xE9 M\xFCller Stra\xDF" "e");
   CHECK_INT(GetWindowTextLengthA(edit), 18);
   CHECK_INT(Edit_GetTextLength(edit), 18);
   CHECK(Has(EN_CHANGE) || rcv.commands == 0);   // WM_SETTEXT: Windows sends EN_CHANGE for edits

   // Typing at the end; EN_UPDATE then EN_CHANGE.
   SetFocus(edit);
   Edit_SetSel(edit, 18, 18);
   memset(&rcv, 0, sizeof(rcv));
   Type(edit, "!\xF6");
   GetWindowTextA(edit, text, sizeof(text));
   CHECK_STR(text, "Caf\xE9 M\xFCller Stra\xDF" "e!\xF6");
   CHECK(Has(EN_UPDATE));
   CHECK(Has(EN_CHANGE));
   CHECK_INT(rcv.last_id, 201);
   // Backspace and the arrow keys.
   Type(edit, "\b");
   GetWindowTextA(edit, text, sizeof(text));
   CHECK_STR(text, "Caf\xE9 M\xFCller Stra\xDF" "e!");
   Key(edit, VK_HOME);
   DWORD start = 0, end = 0;
   SendMessageA(edit, EM_GETSEL, (WPARAM) &start, (LPARAM) &end);
   CHECK_INT(start, 0);
   CHECK_INT(end, 0);
   Key(edit, VK_RIGHT);
   SendMessageA(edit, EM_GETSEL, (WPARAM) &start, (LPARAM) &end);
   CHECK_INT(start, 1);

   // Selection and replacement.
   Edit_SetSel(edit, 0, 4);
   SendMessageA(edit, EM_GETSEL, (WPARAM) &start, (LPARAM) &end);
   CHECK_INT(start, 0);
   CHECK_INT(end, 4);
   CHECK_INT(LOWORD(SendMessageA(edit, EM_GETSEL, 0, 0)), 0);
   CHECK_INT(HIWORD(SendMessageA(edit, EM_GETSEL, 0, 0)), 4);
   Edit_ReplaceSel(edit, "Tee");
   GetWindowTextA(edit, text, sizeof(text));
   CHECK_STR(text, "Tee M\xFCller Stra\xDF" "e!");

   // The limit.
   Edit_LimitText(edit, 5);
   SetWindowTextA(edit, "abc");
   Edit_SetSel(edit, 3, 3);
   memset(&rcv, 0, sizeof(rcv));
   Type(edit, "defgh");
   GetWindowTextA(edit, text, sizeof(text));
   CHECK_STR(text, "abcde");
   CHECK(Has(EN_MAXTEXT));
   CHECK_INT(SendMessageA(edit, EM_GETLIMITTEXT, 0, 0), 5);
   Edit_LimitText(edit, 0);

   // The caret: shown at the insertion point while focused.
   SetWindowTextA(edit, "caret");
   Edit_SetSel(edit, 0, 0);
   POINT c0, c5;
   GetCaretPos(&c0);
   Edit_SetSel(edit, 5, 5);
   GetCaretPos(&c5);
   CHECK(c5.x > c0.x + 10);
   memset(&rcv, 0, sizeof(rcv));
   SetFocus(parent);
   CHECK(Has(EN_KILLFOCUS));
   memset(&rcv, 0, sizeof(rcv));
   SetFocus(edit);
   CHECK(Has(EN_SETFOCUS));

   // The clipboard: cut and paste.
   SetWindowTextA(edit, "copy \xE9 me");
   Edit_SetSel(edit, 5, 6);
   SendMessageA(edit, WM_COPY, 0, 0);
   CHECK(IsClipboardFormatAvailable(CF_TEXT));
   CHECK(IsClipboardFormatAvailable(CF_UNICODETEXT));
   CHECK(OpenClipboard(parent));
   HANDLE h = GetClipboardData(CF_TEXT);
   CHECK(h != NULL);
   if (h)
   {
      CHECK_STR((const char *) GlobalLock(h), "\xE9");
      GlobalUnlock(h);
   }
   CloseClipboard();
   Edit_SetSel(edit, 0, 0);
   SendMessageA(edit, WM_PASTE, 0, 0);
   GetWindowTextA(edit, text, sizeof(text));
   CHECK_STR(text, "\xE9" "copy \xE9 me");
   Edit_SetSel(edit, 0, 5);
   SendMessageA(edit, WM_CUT, 0, 0);
   GetWindowTextA(edit, text, sizeof(text));
   CHECK_STR(text, " \xE9 me");
   // The clipboard from the program's side: CF_TEXT in, CF_UNICODETEXT out.
   CHECK(OpenClipboard(parent));
   CHECK(EmptyClipboard());
   HGLOBAL g = GlobalAlloc(GMEM_MOVEABLE, 8);
   strcpy((char *) GlobalLock(g), "Gr\xFC\xDF");
   GlobalUnlock(g);
   CHECK(SetClipboardData(CF_TEXT, g) == g);
   CHECK(GetClipboardOwner() == parent);
   CloseClipboard();
   CHECK(OpenClipboard(parent));
   const WCHAR *w = (const WCHAR *) GlobalLock(GetClipboardData(CF_UNICODETEXT));
   CHECK(w && w[2] == 0xFC && w[3] == 0xDF && w[4] == 0);
   CloseClipboard();
   CHECK(!GetClipboardData(CF_TEXT));   // closed

   // Read-only: typing does nothing, EM_SETREADONLY sets ES_READONLY.
   Edit_SetReadOnly(edit, TRUE);
   CHECK(GetWindowLongA(edit, GWL_STYLE) & ES_READONLY);
   SetWindowTextA(edit, "ro");
   Edit_SetSel(edit, 2, 2);
   Type(edit, "x");
   GetWindowTextA(edit, text, sizeof(text));
   CHECK_STR(text, "ro");
   Edit_SetReadOnly(edit, FALSE);

   // An ANSI subclass sees the characters typed (in the code page) and can eat some.
   old_edit_proc = SubclassWindow(edit, EditSubclass);
   SetWindowTextA(edit, "");
   Type(edit, "a#\xE4");
   GetWindowTextA(edit, text, sizeof(text));
   CHECK_STR(text, "a\xE4");
   CHECK_INT(subclass_chars, 3);
   CHECK_INT(subclass_last_char, 0xE4);
   SubclassWindow(edit, old_edit_proc);

   // Password.
   HWND pass = CreateWindowExA(0, "Edit", "secret", WS_CHILD | WS_VISIBLE | ES_PASSWORD, 10, 40, 100, 20, parent,
                               (HMENU) 202, instance, NULL);
   CHECK_INT(SendMessageA(pass, EM_GETPASSWORDCHAR, 0, 0), '*');
   GetWindowTextA(pass, text, sizeof(text));
   CHECK_STR(text, "secret");
   SendMessageA(pass, EM_SETPASSWORDCHAR, '#', 0);
   CHECK_INT(SendMessageA(pass, EM_GETPASSWORDCHAR, 0, 0), '#');

   // Multi-line: lines, scrolling, the formatting rectangle.
   HWND ml = CreateWindowExA(WS_EX_CLIENTEDGE, "Edit", "",
                             WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, 10, 70,
                             200, 60, parent, (HMENU) 203, instance, NULL);
   SendMessageA(ml, WM_SETFONT, (WPARAM) font, FALSE);
   SetWindowTextA(ml, "one\r\ntwo \xFC\r\nthree\r\nfour\r\nfive\r\nsix\r\nseven\r\neight");
   CHECK_INT(Edit_GetLineCount(ml), 8);
   char line[64];
   int n = Edit_GetLine(ml, 1, line, sizeof(line));
   line[n] = 0;
   CHECK_STR(line, "two \xFC");
   CHECK_INT(Edit_LineIndex(ml, 2), 12);
   CHECK_INT(Edit_LineFromChar(ml, 13), 2);
   CHECK_INT(Edit_LineLength(ml, 12), 5);
   CHECK_INT(Edit_GetFirstVisibleLine(ml), 0);
   Edit_Scroll(ml, 3, 0);
   CHECK_INT(Edit_GetFirstVisibleLine(ml), 3);
   SCROLLINFO si = {sizeof(si), SIF_ALL};
   CHECK(GetScrollInfo(ml, SB_VERT, &si));
   CHECK_INT(si.nPos, 3);
   CHECK_INT(si.nMax, 7);
   Edit_Scroll(ml, -10, 0);
   CHECK_INT(Edit_GetFirstVisibleLine(ml), 0);
   // WM_VSCROLL from its scroll bar.
   SendMessageA(ml, WM_VSCROLL, SB_LINEDOWN, 0);
   CHECK_INT(Edit_GetFirstVisibleLine(ml), 1);
   SendMessageA(ml, WM_VSCROLL, SB_TOP, 0);
   RECT fr;
   Edit_GetRect(ml, &fr);
   RECT client;
   GetClientRect(ml, &client);
   CHECK(fr.right <= client.right && fr.right > client.right - 10);
   CHECK(client.right <= 200 - 4 - GetSystemMetrics(SM_CXVSCROLL));
   // Return inserts a line break.
   Edit_SetSel(ml, 3, 3);
   SetFocus(ml);
   SendMessageA(ml, WM_KEYDOWN, VK_RETURN, 1);
   SendMessageA(ml, WM_CHAR, '\r', 1);
   CHECK_INT(Edit_GetLineCount(ml), 9);

   // Rendering: the text dark on white; the selection in the highlight color.
   SetFocus(edit);
   SetWindowTextA(edit, "Hello World");
   SendMessageA(edit, EM_SETSEL, 6, 11);
   struct Image im = Render(edit);
   RECT text_area = {4, 4, 40, 18};
   CHECK(CountDark(&im, text_area) > 15);
   RECT sel_area = {40, 2, 120, 20};
   CHECK(CountColor(&im, sel_area, GetSysColor(COLOR_HIGHLIGHT)) > 50);
   // The client edge: dark gray top-left outer line.
   CHECK(Pixel(&im, 0, 0) == GetSysColor(COLOR_BTNSHADOW));
   SaveBmp(&im, "ctl_edit.bmp");
   FreeImage(&im);
   // WM_CTLCOLOREDIT went to the parent; a read-only edit asks with WM_CTLCOLORSTATIC.
   memset(&rcv, 0, sizeof(rcv));
   im = Render(edit);
   FreeImage(&im);
   CHECK(rcv.ctlcolor[WM_CTLCOLOREDIT - WM_CTLCOLORMSGBOX] >= 1);
   Edit_SetReadOnly(edit, TRUE);
   memset(&rcv, 0, sizeof(rcv));
   im = Render(edit);
   FreeImage(&im);
   CHECK(rcv.ctlcolor[WM_CTLCOLORSTATIC - WM_CTLCOLORMSGBOX] >= 1);
   // The parent's brush colors the background.
   rcv.ctl_brush = CreateSolidBrush(RGB(0, 128, 0));
   im = Render(edit);
   CHECK(Pixel(&im, 150, 10) == RGB(0, 128, 0));
   FreeImage(&im);
   DeleteObject(rcv.ctl_brush);
   rcv.ctl_brush = NULL;

   // Multi-line rendering with its scroll bar.
   im = Render(ml);
   SaveBmp(&im, "ctl_edit_multiline.bmp");
   int sbx = im.width - 2 - GetSystemMetrics(SM_CXVSCROLL) / 2;
   CHECK(Pixel(&im, sbx, 4) == GetSysColor(COLOR_BTNFACE));   // the up arrow's face
   FreeImage(&im);

   // Keys typed ahead of the message loop: each key's characters (posted by
   // TranslateMessage) come before the next key, as on Windows.
   Edit_SetReadOnly(edit, FALSE);
   SetWindowTextA(edit, "");
   SetFocus(edit);
   Pump();
   old_edit_proc = SubclassWindow(edit, ReturnSubclass);
   static const BYTE typed[] = {'H', 'I', VK_RETURN};
   for (int i = 0; i < 3; i++)
   {
      keybd_event(typed[i], 0, 0, 0);
      keybd_event(typed[i], 0, KEYEVENTF_KEYUP, 0);
   }
   Pump();
   CHECK_STR(text_at_return, "hi");
   SubclassWindow(edit, old_edit_proc);

   // Key state is the messages', as Windows': a key tapped before the loop
   // runs reads down while its WM_KEYDOWN is handled, up after its WM_KEYUP.
   keybd_event('Q', 0, 0, 0);
   keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
   CHECK(!(GetKeyState('Q') & 0x8000));
   MSG key;
   CHECK(PeekMessageA(&key, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE));
   CHECK(GetKeyState('Q') & 0x8000);
   BYTE keys[256];
   GetKeyboardState(keys);
   CHECK(keys['Q'] & 0x80);
   Pump();
   CHECK(!(GetKeyState('Q') & 0x8000));

   // Ctrl+A selects all (the chat box), as Windows' edits since Vista; its ^A
   // types nothing.
   SetWindowTextA(edit, "say hello");
   Edit_SetSel(edit, 3, 3);
   keybd_event(VK_CONTROL, 0, 0, 0);
   Pump();
   keybd_event('A', 0, 0, 0);
   keybd_event('A', 0, KEYEVENTF_KEYUP, 0);
   keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
   Pump();
   DWORD sel_start = 0, sel_end = 0;
   SendMessageA(edit, EM_GETSEL, (WPARAM) &sel_start, (LPARAM) &sel_end);
   CHECK_INT(sel_start, 0);
   CHECK_INT(sel_end, 9);
   char after[32];
   GetWindowTextA(edit, after, sizeof(after));
   CHECK_STR(after, "say hello");

   // The focus has moved when WM_KILLFOCUS comes, as on Windows: the client
   // redraws the chat box's highlight by GetFocus() then.
   SetFocus(edit);
   old_edit_proc = SubclassWindow(edit, KillFocusSubclass);
   SetFocus(parent);
   CHECK(focus_at_kill == parent);

   // Hidden or destroyed with the focus, it goes to the parent the same way.
   SetFocus(edit);
   focus_at_kill = NULL;
   ShowWindow(edit, SW_HIDE);
   CHECK(GetFocus() == parent);
   CHECK(focus_at_kill == parent);
   ShowWindow(edit, SW_SHOW);
   SubclassWindow(edit, old_edit_proc);
   HWND doomed = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE, 0, 0, 50, 20, parent, NULL, instance, NULL);
   SetFocus(doomed);
   DestroyWindow(doomed);
   CHECK(GetFocus() == parent);

   DestroyWindow(parent);
   return Report("ctl_edit");
}
