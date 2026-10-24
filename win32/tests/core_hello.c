/*
 * core_hello.c:  A window painted with GDI through the message loop: background
 *   brush, text, lines, shapes.  Quits after M59_TEST_SECONDS (default 30).
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_PAINT:
   {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      RECT r;
      GetClientRect(hwnd, &r);
      SetBkMode(hdc, TRANSPARENT);
      SetTextColor(hdc, RGB(0, 0, 128));
      HFONT font = CreateFont(-20, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, "Times New Roman");
      HGDIOBJ old = SelectObject(hdc, font);
      TextOut(hdc, 20, 20, "Meridian 59 on the Win32 layer", 30);
      SelectObject(hdc, old);
      DeleteObject(font);
      SetTextColor(hdc, RGB(0, 0, 0));
      RECT box = {20, 60, r.right - 20, 120};
      DrawText(hdc, "DrawText: centered, single line, with &prefix", -1, &box, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      HPEN pen = CreatePen(PS_SOLID, 3, RGB(200, 0, 0));
      old = SelectObject(hdc, pen);
      MoveToEx(hdc, 20, 140, NULL);
      LineTo(hdc, r.right - 20, 200);
      SelectObject(hdc, old);
      DeleteObject(pen);
      HBRUSH brush = CreateSolidBrush(RGB(0, 160, 0));
      old = SelectObject(hdc, brush);
      Ellipse(hdc, 20, 210, 160, 300);
      Rectangle(hdc, 180, 210, 320, 300);
      SelectObject(hdc, old);
      DeleteObject(brush);
      EndPaint(hwnd, &ps);
      return 0;
   }
   case WM_TIMER:
      PostQuitMessage(0);
      return 0;
   case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
   }
   return DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   WNDCLASS wc = {0};
   wc.lpfnWndProc = WndProc;
   wc.hInstance = instance;
   wc.hbrBackground = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
   wc.lpszClassName = "HelloClass";
   if (!RegisterClass(&wc))
   {
      printf("RegisterClass failed\n");
      return 1;
   }
   HWND hwnd = CreateWindow("HelloClass", "Win32 layer: hello", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                            560, 340, NULL, NULL, instance, NULL);
   if (!hwnd)
   {
      printf("CreateWindow failed\n");
      return 1;
   }
   ShowWindow(hwnd, show);
   UpdateWindow(hwnd);
   const char *seconds = getenv("M59_TEST_SECONDS");
   SetTimer(hwnd, 1, (seconds ? atoi(seconds) : 30) * 1000, NULL);
   MSG msg;
   while (GetMessage(&msg, NULL, 0, 0))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
   printf("quit: %d\n", (int) msg.wParam);
   return 0;
}
