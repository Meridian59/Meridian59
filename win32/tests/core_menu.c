/*
 * core_menu.c:  A menu bar with an owner-drawn item, drop-downs with checked,
 *   grayed and nested items; WM_COMMAND shows in the title and on stdout.
 *   Quits after M59_TEST_SECONDS (default 60).
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

enum { ID_OPEN = 101, ID_SAVE, ID_EXIT, ID_MUSIC, ID_SOUND, ID_DEEP, ID_OWNER };

static HBRUSH bar_brush, sel_brush;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_INITMENUPOPUP:
      printf("initmenupopup %p index %d\n", (void *) wparam, (int) LOWORD(lparam));
      return 0;
   case WM_MEASUREITEM:
   {
      MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *) lparam;
      if (mis->CtlType != ODT_MENU)
         return FALSE;
      mis->itemWidth = 60;
      mis->itemHeight = GetSystemMetrics(SM_CYMENU);
      return TRUE;
   }
   case WM_DRAWITEM:
   {
      DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lparam;
      if (dis->CtlType != ODT_MENU)
         return FALSE;
      FillRect(dis->hDC, &dis->rcItem, (dis->itemState & ODS_SELECTED) ? sel_brush : bar_brush);
      SetBkMode(dis->hDC, TRANSPARENT);
      SetTextColor(dis->hDC, RGB(255, 255, 255));
      DrawText(dis->hDC, (const char *) dis->itemData, -1, &dis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      return TRUE;
   }
   case WM_COMMAND:
   {
      char title[64];
      sprintf(title, "command %d", LOWORD(wparam));
      printf("%s\n", title);
      fflush(stdout);
      SetWindowText(hwnd, title);
      if (LOWORD(wparam) == ID_MUSIC)
      {
         HMENU options = GetSubMenu(GetMenu(hwnd), 1);
         UINT state = GetMenuState(options, ID_MUSIC, MF_BYCOMMAND);
         CheckMenuItem(options, ID_MUSIC, (state & MF_CHECKED) ? MF_UNCHECKED : MF_CHECKED);
      }
      if (LOWORD(wparam) == ID_EXIT)
         DestroyWindow(hwnd);
      return 0;
   }
   case WM_PAINT:
   {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      RECT r;
      GetClientRect(hwnd, &r);
      FrameRect(hdc, &r, (HBRUSH) GetStockObject(BLACK_BRUSH));
      TextOut(hdc, 8, 8, "client area origin", 18);
      EndPaint(hwnd, &ps);
      return 0;
   }
   case WM_TIMER:
   case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
   }
   return DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   bar_brush = CreateSolidBrush(RGB(60, 40, 20));
   sel_brush = CreateSolidBrush(RGB(140, 90, 40));
   WNDCLASS wc = {0};
   wc.lpfnWndProc = WndProc;
   wc.hInstance = instance;
   wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
   wc.lpszClassName = "MenuClass";
   RegisterClass(&wc);

   HMENU bar = CreateMenu();
   HMENU game = CreatePopupMenu();
   AppendMenu(game, MF_STRING, ID_OPEN, "&Open\tCtrl+O");
   AppendMenu(game, MF_STRING | MF_GRAYED, ID_SAVE, "&Save");
   AppendMenu(game, MF_SEPARATOR, 0, NULL);
   AppendMenu(game, MF_STRING, ID_EXIT, "E&xit");
   HMENU options = CreatePopupMenu();
   AppendMenu(options, MF_STRING | MF_CHECKED, ID_MUSIC, "&Music");
   AppendMenu(options, MF_STRING, ID_SOUND, "S&ound && effects");
   HMENU deeper = CreatePopupMenu();
   AppendMenu(deeper, MF_STRING, ID_DEEP, "&Deep item");
   AppendMenu(options, MF_POPUP, (UINT_PTR) deeper, "&More");
   AppendMenu(bar, MF_POPUP, (UINT_PTR) game, "&Game");
   AppendMenu(bar, MF_POPUP, (UINT_PTR) options, "&Options");
   AppendMenu(bar, MF_OWNERDRAW, ID_OWNER, (LPCSTR) "Owner");

   RECT want = {0, 0, 400, 240};
   AdjustWindowRect(&want, WS_OVERLAPPEDWINDOW, TRUE);
   HWND hwnd = CreateWindow("MenuClass", "Win32 layer: menus", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                            want.right - want.left, want.bottom - want.top, NULL, bar, instance, NULL);
   ShowWindow(hwnd, show);
   UpdateWindow(hwnd);
   RECT client;
   GetClientRect(hwnd, &client);
   printf("client %ldx%ld\n", (long) client.right, (long) client.bottom);
   fflush(stdout);
   const char *seconds = getenv("M59_TEST_SECONDS");
   SetTimer(hwnd, 1, (seconds ? atoi(seconds) : 60) * 1000, NULL);
   MSG msg;
   while (GetMessage(&msg, NULL, 0, 0))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
   return 0;
}
