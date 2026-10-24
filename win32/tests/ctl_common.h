/*
 * ctl_common.h:  What the ctl_* tests share: a hidden ANSI parent window that
 *   records the notifications its controls send, synthesized input, pumping
 *   posted messages, and rendering a window (WM_PRINT) into a 32-bit DIB for
 *   pixel checks and BMP dumps.
 */

#ifndef CTL_COMMON_H
#define CTL_COMMON_H

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

static int failures, checks;
#define CHECK(c) do { checks++; if (!(c)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); failures++; } } while (0)
#define CHECK_INT(a, b) do { long long va_ = (long long) (a), vb_ = (long long) (b); checks++; \
   if (va_ != vb_) { failures++; printf("FAIL %s:%d: %s == %lld, expected %lld\n", __FILE__, __LINE__, #a, va_, vb_); } } while (0)
#define CHECK_STR(a, b) do { checks++; if (strcmp((a), (b)) != 0) { failures++; \
   printf("FAIL %s:%d: \"%s\" != \"%s\"\n", __FILE__, __LINE__, (a), (b)); } } while (0)

// What the parent received.
struct Received
{
   int commands;
   WORD last_id, last_code;
   HWND last_control;
   int codes[64];          // WM_COMMAND notification codes, in order
   int code_count;
   int ctlcolor[8];        // WM_CTLCOLORMSGBOX ... WM_CTLCOLORSTATIC, counts
   int scrolls;
   WPARAM last_scroll;
   HWND last_scroll_control;
   UINT last_scroll_msg;
   int measures, draws;
   MEASUREITEMSTRUCT last_measure;
   DRAWITEMSTRUCT last_draw;
   int compares;
   HBRUSH ctl_brush;       // what WM_CTLCOLOR* answers, or 0 for DefWindowProc's
};
static struct Received rcv;

static LRESULT CALLBACK ParentProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_COMMAND:
      rcv.commands++;
      rcv.last_id = LOWORD(wparam);
      rcv.last_code = HIWORD(wparam);
      rcv.last_control = (HWND) lparam;
      if (rcv.code_count < 64)
         rcv.codes[rcv.code_count++] = HIWORD(wparam);
      return 0;
   case WM_CTLCOLORMSGBOX:
   case WM_CTLCOLOREDIT:
   case WM_CTLCOLORLISTBOX:
   case WM_CTLCOLORBTN:
   case WM_CTLCOLORDLG:
   case WM_CTLCOLORSCROLLBAR:
   case WM_CTLCOLORSTATIC:
      rcv.ctlcolor[msg - WM_CTLCOLORMSGBOX]++;
      if (rcv.ctl_brush)
         return (LRESULT) rcv.ctl_brush;
      break;
   case WM_VSCROLL:
   case WM_HSCROLL:
      rcv.scrolls++;
      rcv.last_scroll = wparam;
      rcv.last_scroll_control = (HWND) lparam;
      rcv.last_scroll_msg = msg;
      return 0;
   case WM_MEASUREITEM:
   {
      MEASUREITEMSTRUCT *mi = (MEASUREITEMSTRUCT *) lparam;
      rcv.measures++;
      rcv.last_measure = *mi;
      mi->itemHeight = 20;
      return TRUE;
   }
   case WM_DRAWITEM:
   {
      DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *) lparam;
      rcv.draws++;
      rcv.last_draw = *di;
      // Owner-drawn items: red when selected, else blue.
      HBRUSH b = CreateSolidBrush((di->itemState & ODS_SELECTED) ? RGB(255, 0, 0) : RGB(0, 0, 255));
      FillRect(di->hDC, &di->rcItem, b);
      DeleteObject(b);
      return TRUE;
   }
   case WM_COMPAREITEM:
   {
      COMPAREITEMSTRUCT *ci = (COMPAREITEMSTRUCT *) lparam;
      rcv.compares++;
      return ci->itemData1 < ci->itemData2 ? -1 : ci->itemData1 > ci->itemData2 ? 1 : 0;
   }
   case WM_ERASEBKGND:
   {
      RECT r;
      GetClientRect(hwnd, &r);
      FillRect((HDC) wparam, &r, GetSysColorBrush(COLOR_BTNFACE));
      return 1;
   }
   case WM_PRINTCLIENT:
      return 0;
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

// A top-level window for the controls, never shown on screen.
static HWND MakeParent(int width, int height)
{
   static ATOM atom;
   if (!atom)
   {
      WNDCLASSA wc = {0};
      wc.lpfnWndProc = ParentProc;
      wc.hInstance = GetModuleHandleA(NULL);
      wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
      wc.lpszClassName = "CtlTestParent";
      atom = RegisterClassA(&wc);
   }
   HWND hwnd = CreateWindowExA(0, "CtlTestParent", "parent", WS_POPUP, 0, 0, width, height, NULL,
                               NULL, GetModuleHandleA(NULL), NULL);
   // Visible as far as Windows is concerned (WS_VISIBLE set directly shows no
   // native window), so controls paint as in a shown window.
   SetWindowLongA(hwnd, GWL_STYLE, GetWindowLongA(hwnd, GWL_STYLE) | WS_VISIBLE);
   return hwnd;
}

static void Pump(void)
{
   MSG m;
   while (PeekMessageA(&m, NULL, 0, 0, PM_REMOVE))
   {
      TranslateMessage(&m);
      DispatchMessageA(&m);
   }
}

static void Click(HWND hwnd, int x, int y)
{
   SendMessageA(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
   SendMessageA(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
}

static void Key(HWND hwnd, WPARAM vk)
{
   SendMessageA(hwnd, WM_KEYDOWN, vk, 1);
   SendMessageA(hwnd, WM_KEYUP, vk, (LPARAM) 0xC0000001);
}

static void Type(HWND hwnd, const char *text)
{
   for (; *text; text++)
      SendMessageA(hwnd, WM_CHAR, (unsigned char) *text, 1);
}

// A window drawn into a 32-bit top-down DIB of its size.
struct Image
{
   HDC dc;
   HBITMAP bitmap;
   DWORD *bits;
   int width, height;
};

static struct Image Render(HWND hwnd)
{
   struct Image im;
   RECT r;
   GetWindowRect(hwnd, &r);
   im.width = r.right - r.left;
   im.height = r.bottom - r.top;
   BITMAPINFO bi = {0};
   bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
   bi.bmiHeader.biWidth = im.width;
   bi.bmiHeader.biHeight = -im.height;
   bi.bmiHeader.biPlanes = 1;
   bi.bmiHeader.biBitCount = 32;
   im.dc = CreateCompatibleDC(NULL);
   im.bitmap = CreateDIBSection(im.dc, &bi, DIB_RGB_COLORS, (void **) &im.bits, NULL, 0);
   SelectObject(im.dc, im.bitmap);
   // A color nothing draws, to see what was drawn.
   for (int i = 0; i < im.width * im.height; i++)
      im.bits[i] = 0xFF00FF;
   SendMessageA(hwnd, WM_PRINT, (WPARAM) im.dc, PRF_NONCLIENT | PRF_CLIENT | PRF_ERASEBKGND | PRF_CHILDREN);
   return im;
}

static void FreeImage(struct Image *im)
{
   DeleteDC(im->dc);
   DeleteObject(im->bitmap);
}

static COLORREF Pixel(const struct Image *im, int x, int y)
{
   if (x < 0 || y < 0 || x >= im->width || y >= im->height)
      return CLR_INVALID;
   DWORD v = im->bits[y * im->width + x];
   return RGB((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
}

static int IsDark(COLORREF c)
{
   return GetRValue(c) + GetGValue(c) + GetBValue(c) < 3 * 96;
}

// Pixels of color c in the rectangle.
static int CountColor(const struct Image *im, RECT r, COLORREF c)
{
   int n = 0;
   for (int y = r.top; y < r.bottom; y++)
      for (int x = r.left; x < r.right; x++)
         if (Pixel(im, x, y) == c)
            n++;
   return n;
}

static int CountLight(const struct Image *im, RECT r)
{
   int n = 0;
   for (int y = r.top; y < r.bottom; y++)
      for (int x = r.left; x < r.right; x++)
      {
         COLORREF c = Pixel(im, x, y);
         if (GetRValue(c) + GetGValue(c) + GetBValue(c) > 3 * 160)
            n++;
      }
   return n;
}

static int CountDark(const struct Image *im, RECT r)
{
   int n = 0;
   for (int y = r.top; y < r.bottom; y++)
      for (int x = r.left; x < r.right; x++)
         if (IsDark(Pixel(im, x, y)))
            n++;
   return n;
}

static void SaveBmp(const struct Image *im, const char *path)
{
   FILE *f = fopen(path, "wb");
   if (f == NULL)
      return;
   BITMAPFILEHEADER fh = {0};
   BITMAPINFOHEADER ih = {0};
   int stride = im->width * 3 + (4 - im->width * 3 % 4) % 4;
   fh.bfType = 0x4D42;
   fh.bfOffBits = sizeof(fh) + sizeof(ih);
   fh.bfSize = fh.bfOffBits + stride * im->height;
   ih.biSize = sizeof(ih);
   ih.biWidth = im->width;
   ih.biHeight = im->height;
   ih.biPlanes = 1;
   ih.biBitCount = 24;
   fwrite(&fh, sizeof(fh), 1, f);
   fwrite(&ih, sizeof(ih), 1, f);
   unsigned char row[8192 * 3 + 4];
   for (int y = im->height - 1; y >= 0; y--)
   {
      memset(row, 0, stride);
      for (int x = 0; x < im->width; x++)
      {
         DWORD v = im->bits[y * im->width + x];
         row[x * 3] = v & 0xFF;
         row[x * 3 + 1] = (v >> 8) & 0xFF;
         row[x * 3 + 2] = (v >> 16) & 0xFF;
      }
      fwrite(row, stride, 1, f);
   }
   fclose(f);
}

static int Report(const char *name)
{
   printf("%s: %d checks, %d failed\n", name, checks, failures);
   return failures != 0;
}

#endif
