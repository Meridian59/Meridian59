/*
 * cc_misc.c:  comctl32 as a module and its helper exports: GetModuleHandle /
 *   LoadLibrary("comctl32.dll") give the module carrying its resources (strings,
 *   bitmaps, cursors, the property sheet templates); InitCommonControlsEx,
 *   _TrackMouseEvent, GetEffectiveClientRect, DrawStatusTextA,
 *   CreateMappedBitmap, CreateUpDownControl; the classes registered at startup.
 */

#include "cc_util.h"

static int leaves;

static LRESULT CALLBACK ParentProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_MOUSELEAVE)
      leaves++;
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   (void) prev, (void) cmd, (void) show;

   // Registered before WinMain, as comctl32 5.82+ does when it loads.
   const char *classes[] = {WC_LISTVIEWA, WC_HEADERA, WC_TABCONTROLA, TRACKBAR_CLASSA, TOOLTIPS_CLASSA,
                            ANIMATE_CLASSA, UPDOWN_CLASSA, PROGRESS_CLASSA};
   for (const char *name : classes)
   {
      WNDCLASSA wc;
      BOOL found = GetClassInfoA(NULL, name, &wc);
      if (!found)
         printf("class %s not registered\n", name);
      CHECK(found);
   }

   INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS};
   CHECK(InitCommonControlsEx(&icc));

   // The module.
   HMODULE comctl = GetModuleHandleA("comctl32");
   CHECK(comctl != NULL);
   CHECK(GetModuleHandleA("COMCTL32.DLL") == comctl);
   CHECK(LoadLibraryA("comctl32.dll") == comctl);
   CHECK(FreeLibrary(comctl));
   char path[MAX_PATH];
   CHECK(GetModuleFileNameA(comctl, path, sizeof(path)) > 0);
   CHECK(FindResourceA(comctl, MAKEINTRESOURCEA(1006), (LPCSTR) RT_DIALOG) != NULL);   // IDD_PROPSHEET
   CHECK(FindResourceA(comctl, MAKEINTRESOURCEA(1020), (LPCSTR) RT_DIALOG) != NULL);   // IDD_WIZARD
   HBITMAP std_small = LoadBitmapA(comctl, MAKEINTRESOURCEA(120));                      // IDB_STD_SMALL
   CHECK(std_small != NULL);
   BITMAP bm = {};
   GetObjectA(std_small, sizeof(bm), &bm);
   CHECK(bm.bmWidth >= 16 * 15 && bm.bmHeight == 16);
   CHECK(LoadCursorA(comctl, MAKEINTRESOURCEA(106)) != NULL);   // IDC_DIVIDER (the header's)
   // The executable's resources are not comctl32's.
   CHECK(FindResourceA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(1006), (LPCSTR) RT_DIALOG) == NULL);

   // CreateMappedBitmap: comctl32's check marks (IDT_CHECK, 4-bit RLE) with white mapped.
   COLORMAP map = {RGB(255, 255, 255), RGB(255, 0, 255)};
   HBITMAP mapped = CreateMappedBitmap(comctl, 401, 0, &map, 1);
   CHECK(mapped != NULL);
   if (mapped)
   {
      HDC dc = CreateCompatibleDC(NULL);
      HGDIOBJ old = SelectObject(dc, mapped);
      int magenta = 0;
      int white = 0;
      for (int x = 0; x < 31; x++)
         for (int y = 0; y < 16; y++)
         {
            COLORREF p = GetPixel(dc, x, y);
            magenta += p == RGB(255, 0, 255);
            white += p == RGB(255, 255, 255);
         }
      CHECK(magenta > 100);   // white became magenta
      CHECK_INT(white, 0);
      SelectObject(dc, old);
      DeleteDC(dc);
      DeleteObject(mapped);
   }

   // DrawStatusTextA: a sunken panel with text.
   Canvas32 c = MakeCanvas(200, 20, RGB(255, 0, 255));
   RECT r = {0, 0, 200, 20};
   DrawStatusTextA(c.dc, &r, "Pronto \xE9", 0);
   CHECK_INT(CountColor(&c, r, RGB(255, 0, 255), 0), 0);
   CHECK(CountDark(&c, r, 80) > 20);
   FreeCanvas(&c);

   HWND parent = MakeParent(ParentProc, 200, 100);
   // GetEffectiveClientRect: the client area less the visible controls listed.
   HWND bar = CreateWindowExA(0, "CcParent", "", WS_CHILD | WS_VISIBLE, 0, 80, 200, 20, parent, (HMENU) 5, instance, NULL);
   INT info[] = {0, 0, 1, 5, 0, 0};
   GetEffectiveClientRect(parent, &r, info);
   CHECK_INT(r.bottom, 80);
   DestroyWindow(bar);

   // CreateUpDownControl.
   HWND ud = CreateUpDownControl(WS_CHILD | WS_VISIBLE, 0, 0, 16, 24, parent, 9, instance, NULL, 20, 0, 5);
   CHECK(ud != NULL);
   CHECK_INT(LOWORD(SendMessageA(ud, UDM_GETPOS, 0, 0)), 5);

   // _TrackMouseEvent: WM_MOUSELEAVE when the pointer leaves.
   TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, parent, 0};
   CHECK(_TrackMouseEvent(&tme));
   tme.dwFlags = TME_QUERY;
   CHECK(_TrackMouseEvent(&tme));

   DestroyWindow(parent);
   return CheckReport("cc_misc");
}
