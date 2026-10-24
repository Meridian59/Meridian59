/*
 * core_unicode.c:  ANSI and Unicode window procedures talking across sets: text,
 *   CREATESTRUCT, characters, list strings, EM_GETLINE, procedure handles from
 *   GetWindowLongPtr, WM_NOTIFYFORMAT.
 */

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); failures++; } } while (0)

static WCHAR w_text[64];
static WCHAR w_create_name[64];
static WPARAM w_char;
static WCHAR w_list[64];

static size_t wlen(const WCHAR *s) { size_t n = 0; while (s[n]) n++; return n; }
static int weq(const WCHAR *a, const char *b)
{
   while (*a && *b && *a == (unsigned char) *b) a++, b++;
   return *a == 0 && *b == 0;
}

// A Unicode procedure keeping its own text.
static LRESULT CALLBACK WideProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_CREATE:
   {
      const CREATESTRUCTW *cs = (const CREATESTRUCTW *) lparam;
      if (cs->lpszName)
         memcpy(w_create_name, cs->lpszName, (wlen(cs->lpszName) + 1) * sizeof(WCHAR));
      return 0;
   }
   case WM_SETTEXT:
      memcpy(w_text, (const WCHAR *) lparam, (wlen((const WCHAR *) lparam) + 1) * sizeof(WCHAR));
      return TRUE;
   case WM_GETTEXT:
   {
      size_t n = wlen(w_text);
      if (n >= wparam) n = wparam - 1;
      memcpy((WCHAR *) lparam, w_text, n * sizeof(WCHAR));
      ((WCHAR *) lparam)[n] = 0;
      return n;
   }
   case WM_GETTEXTLENGTH:
      return wlen(w_text);
   case WM_CHAR:
      w_char = wparam;
      return 0;
   case LB_ADDSTRING:
      memcpy(w_list, (const WCHAR *) lparam, (wlen((const WCHAR *) lparam) + 1) * sizeof(WCHAR));
      return 0;
   case LB_GETTEXTLEN:
      return wlen(w_list);
   case LB_GETTEXT:
      memcpy((WCHAR *) lparam, w_list, (wlen(w_list) + 1) * sizeof(WCHAR));
      return wlen(w_list);
   case EM_GETLINE:
   {
      WORD max = *(WORD *) lparam;
      const WCHAR line[] = {'l', 'i', 'n', 'e', 0xE9};
      int n = max < 5 ? max : 5;
      memcpy((WCHAR *) lparam, line, n * sizeof(WCHAR));
      return n;
   }
   }
   return DefWindowProcW(hwnd, msg, wparam, lparam);
}

static WNDPROC old_proc;
static char a_seen_text[64];
static WPARAM a_seen_char;

// An ANSI subclass of the Unicode window, as the client's SubclassWindow makes.
static LRESULT CALLBACK AnsiSubclass(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_SETTEXT && lparam)
      strcpy(a_seen_text, (const char *) lparam);
   if (msg == WM_CHAR)
      a_seen_char = wparam;
   return CallWindowProcA(old_proc, hwnd, msg, wparam, lparam);
}

static char sub_text[64];
static int sub_wide;

// comctl32 subclasses get messages in Unicode form, as on Windows.
static LRESULT CALLBACK Subclass(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR id, DWORD_PTR data)
{
   if (msg == WM_SETTEXT && lparam)
   {
      const WCHAR *w = (const WCHAR *) lparam;
      sub_wide = w[0] == 'h' && w[1] == 'i';
   }
   return DefSubclassProc(hwnd, msg, wparam, lparam);
}

static LRESULT CALLBACK AnsiProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   const WCHAR wide_name[] = {'W', 'i', 'd', 'e', 0};
   WNDCLASSW wc = {0};
   wc.lpfnWndProc = WideProc;
   wc.hInstance = instance;
   wc.lpszClassName = wide_name;
   CHECK(RegisterClassW(&wc));
   WNDCLASSA ac = {0};
   ac.lpfnWndProc = AnsiProc;
   ac.hInstance = instance;
   ac.lpszClassName = "Ansi";
   CHECK(RegisterClassA(&ac));

   // Created through the A function: the W procedure gets W text.
   HWND wide = CreateWindowExA(0, "Wide", "caf\xe9", WS_POPUP, 0, 0, 10, 10, NULL, NULL, instance, NULL);
   CHECK(wide != NULL);
   CHECK(IsWindowUnicode(wide));
   CHECK(w_create_name[3] == 0xE9 && w_create_name[4] == 0);

   SetWindowTextA(wide, "na\xefve");
   CHECK(weq(w_text, "na\xefve"));
   char buffer[64];
   CHECK(GetWindowTextA(wide, buffer, sizeof(buffer)) == 5 && strcmp(buffer, "na\xefve") == 0);
   CHECK(GetWindowTextA(wide, buffer, 3) == 2 && strcmp(buffer, "na") == 0);
   CHECK(GetWindowTextLengthA(wide) == 5);

   SendMessageA(wide, WM_CHAR, 0xE4, 0);
   CHECK(w_char == 0xE4);
   SendMessageA(wide, WM_CHAR, 0x80, 0);   // euro sign in code page 1252
   CHECK(w_char == 0x20AC);

   SendMessageA(wide, LB_ADDSTRING, 0, (LPARAM) "item");
   CHECK(weq(w_list, "item"));
   CHECK(SendMessageA(wide, LB_GETTEXT, 0, (LPARAM) buffer) == 4 && strcmp(buffer, "item") == 0);

   char line[8];
   *(WORD *) line = 8;
   CHECK(SendMessageA(wide, EM_GETLINE, 0, (LPARAM) line) == 5 && memcmp(line, "line\xe9", 5) == 0);

   // GetWindowLongPtrA of a W procedure: a handle CallWindowProcA translates through.
   old_proc = (WNDPROC) SetWindowLongPtrA(wide, GWLP_WNDPROC, (LONG_PTR) AnsiSubclass);
   CHECK(old_proc != WideProc);
   CHECK(!IsWindowUnicode(wide));
   CHECK((WNDPROC) GetWindowLongPtrA(wide, GWLP_WNDPROC) == AnsiSubclass);
   SetWindowTextA(wide, "sub\xe9");
   CHECK(strcmp(a_seen_text, "sub\xe9") == 0);
   CHECK(weq(w_text, "sub\xe9"));
   const WCHAR wide_text[] = {'w', 0xFC, 0};
   SetWindowTextW(wide, wide_text);   // W caller -> A subclass -> W procedure
   CHECK(strcmp(a_seen_text, "w\xfc") == 0);
   CHECK(weq(w_text, "w\xfc"));
   SendMessageW(wide, WM_CHAR, 0x20AC, 0);
   CHECK(a_seen_char == 0x80 && w_char == 0x20AC);
   WCHAR wbuffer[16];
   CHECK(GetWindowTextW(wide, wbuffer, 16) == 2 && wbuffer[1] == 0xFC);

   // And back: restoring the handle makes the window Unicode again.
   SetWindowLongPtrA(wide, GWLP_WNDPROC, (LONG_PTR) old_proc);
   CHECK(IsWindowUnicode(wide));
   CHECK((WNDPROC) GetWindowLongPtrW(wide, GWLP_WNDPROC) == WideProc);

   // An ANSI window seen from W.
   HWND ansi = CreateWindowExW(0, u"Ansi", NULL, WS_POPUP, 0, 0, 10, 10, NULL, NULL, instance, NULL);
   CHECK(ansi != NULL && !IsWindowUnicode(ansi));
   SetWindowTextW(ansi, wide_text);
   CHECK(GetWindowTextA(ansi, buffer, sizeof(buffer)) == 2 && strcmp(buffer, "w\xfc") == 0);
   CHECK(GetWindowTextW(ansi, wbuffer, 16) == 2 && wbuffer[0] == 'w' && wbuffer[1] == 0xFC && wbuffer[2] == 0);
   WNDPROC handle = (WNDPROC) GetWindowLongPtrW(ansi, GWLP_WNDPROC);
   CHECK(handle != AnsiProc);
   CHECK(CallWindowProcW(handle, ansi, WM_GETTEXTLENGTH, 0, 0) == 2);

   CHECK(SetWindowSubclass(ansi, Subclass, 1, 0));
   SetWindowTextA(ansi, "hi");
   CHECK(sub_wide);
   CHECK(GetWindowTextA(ansi, buffer, sizeof(buffer)) == 2 && strcmp(buffer, "hi") == 0);
   CHECK(RemoveWindowSubclass(ansi, Subclass, 1));
   CHECK(!IsWindowUnicode(ansi));
   CHECK((WNDPROC) GetWindowLongPtrA(ansi, GWLP_WNDPROC) == AnsiProc);

   CHECK(SendMessageA(ansi, WM_NOTIFYFORMAT, (WPARAM) wide, NF_QUERY) == NFR_ANSI);
   CHECK(SendMessageA(wide, WM_NOTIFYFORMAT, (WPARAM) ansi, NF_QUERY) == NFR_UNICODE);

   DestroyWindow(ansi);
   DestroyWindow(wide);
   printf(failures ? "core_unicode: %d failed\n" : "core_unicode: all passed\n", failures);
   return failures != 0;
}
