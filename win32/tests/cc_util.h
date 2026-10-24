/* cc_util.h:  What the cc_* test programs (comctl32's controls) share: CHECK,
 * a 32-bit DIB to render controls into (WM_PRINTCLIENT), pixel counts and a
 * BMP dump of it. */
#ifndef CC_UTIL_H
#define CC_UTIL_H

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kernel_check.h"

// A memory DC with a top-down 32-bit DIB section selected.
struct Canvas32
{
   HDC dc;
   HBITMAP bmp, old;
   DWORD *bits;
   int width, height;
};

static Canvas32 MakeCanvas(int width, int height, COLORREF fill)
{
   Canvas32 c = {};
   BITMAPINFO bi = {};
   bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bi.bmiHeader.biWidth = width;
   bi.bmiHeader.biHeight = -height;
   bi.bmiHeader.biPlanes = 1;
   bi.bmiHeader.biBitCount = 32;
   void *bits = NULL;
   c.bmp = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
   c.bits = (DWORD *) bits;
   c.width = width;
   c.height = height;
   c.dc = CreateCompatibleDC(NULL);
   c.old = (HBITMAP) SelectObject(c.dc, c.bmp);
   RECT r = {0, 0, width, height};
   HBRUSH b = CreateSolidBrush(fill);
   FillRect(c.dc, &r, b);
   DeleteObject(b);
   return c;
}

static void FreeCanvas(Canvas32 *c)
{
   SelectObject(c->dc, c->old);
   DeleteDC(c->dc);
   DeleteObject(c->bmp);
}

static COLORREF CanvasPixel(const Canvas32 *c, int x, int y)
{
   DWORD p = c->bits[(size_t) y * c->width + x];
   return RGB((p >> 16) & 0xFF, (p >> 8) & 0xFF, p & 0xFF);
}

static int Near(COLORREF a, COLORREF b, int tolerance)
{
   return abs(GetRValue(a) - GetRValue(b)) <= tolerance && abs(GetGValue(a) - GetGValue(b)) <= tolerance &&
          abs(GetBValue(a) - GetBValue(b)) <= tolerance;
}

// Pixels of rect (clipped to the canvas) within tolerance of color.
static int CountColor(const Canvas32 *c, RECT r, COLORREF color, int tolerance)
{
   int n = 0;
   for (int y = r.top < 0 ? 0 : r.top; y < r.bottom && y < c->height; y++)
      for (int x = r.left < 0 ? 0 : r.left; x < r.right && x < c->width; x++)
         if (Near(CanvasPixel(c, x, y), color, tolerance))
            n++;
   return n;
}

// Pixels of rect darker than level (each channel).
static int CountDark(const Canvas32 *c, RECT r, int level)
{
   int n = 0;
   for (int y = r.top < 0 ? 0 : r.top; y < r.bottom && y < c->height; y++)
      for (int x = r.left < 0 ? 0 : r.left; x < r.right && x < c->width; x++)
      {
         COLORREF p = CanvasPixel(c, x, y);
         if (GetRValue(p) < level && GetGValue(p) < level && GetBValue(p) < level)
            n++;
      }
   return n;
}

static void DumpBmp(const Canvas32 *c, const char *path)
{
   FILE *f = fopen(path, "wb");
   if (f == NULL)
      return;
   DWORD size = (DWORD) (c->width * c->height * 4);
   BITMAPFILEHEADER fh = {0x4D42, (DWORD) (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + size), 0, 0,
                          (DWORD) (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))};
   BITMAPINFOHEADER ih = {};
   ih.biSize = sizeof(ih);
   ih.biWidth = c->width;
   ih.biHeight = -c->height;
   ih.biPlanes = 1;
   ih.biBitCount = 32;
   ih.biSizeImage = size;
   fwrite(&fh, sizeof(fh), 1, f);
   fwrite(&ih, sizeof(ih), 1, f);
   fwrite(c->bits, size, 1, f);
   fclose(f);
   printf("wrote %s\n", path);
}

// Runs posted messages and timers for ms milliseconds.
static void Pump(DWORD ms)
{
   DWORD end = GetTickCount() + ms;
   do
   {
      MSG msg;
      while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
      {
         TranslateMessage(&msg);
         DispatchMessageA(&msg);
      }
      Sleep(5);
   } while ((LONG) (end - GetTickCount()) > 0);
}

// Runs them until *flag is set, for ms milliseconds at most.
static void PumpUntil(const int *flag, DWORD ms)
{
   DWORD end = GetTickCount() + ms;
   while (!*flag && (LONG) (end - GetTickCount()) > 0)
      Pump(5);
}

// An ANSI top-level window (hidden popup) for controls to live in.
static HWND MakeParent(WNDPROC proc, int width, int height)
{
   WNDCLASSA wc = {};
   wc.lpfnWndProc = proc;
   wc.hInstance = GetModuleHandleA(NULL);
   wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
   wc.lpszClassName = "CcParent";
   RegisterClassA(&wc);
   return CreateWindowExA(0, "CcParent", "cc", WS_POPUP, 0, 0, width, height, NULL, NULL, wc.hInstance, NULL);
}

#endif
