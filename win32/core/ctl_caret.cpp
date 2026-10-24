/*
 * ctl_caret.cpp:  The caret: one per program, owned by a window, drawn by
 *   inverting its rectangle on the window's surface and blinking on a timer.
 *   BeginPaint hides it for the window being painted and EndPaint shows it
 *   again, as Windows does.
 */

#include "core/caret.h"

#include "core/eventloop.h"
#include "core/gdi.h"
#include "core/window.h"

namespace w32 {

HBRUSH Brush55AA();

namespace {

struct Caret
{
   HWND hwnd = nullptr;
   HBITMAP bitmap = nullptr;        // nullptr: solid; (HBITMAP) 1: gray; else the image
   int width = 0, height = 0;
   POINT pos = {0, 0};
   int hidden = 1;                  // ShowCaret/HideCaret count: shown at 0
   bool drawn = false;              // the rectangle is inverted on the surface now
   bool painting = false;           // hidden for BeginPaint
   int timer = 0;
};

Caret caret;
UINT blink_time = 530;

void Invert()
{
   HDC hdc = GetDC(caret.hwnd);
   if (hdc == nullptr)
      return;
   if (caret.bitmap && caret.bitmap != (HBITMAP) 1)
   {
      HDC mem = CreateCompatibleDC(hdc);
      HGDIOBJ old = SelectObject(mem, caret.bitmap);
      BitBlt(hdc, caret.pos.x, caret.pos.y, caret.width, caret.height, mem, 0, 0, SRCINVERT);
      SelectObject(mem, old);
      DeleteDC(mem);
   }
   else if (caret.bitmap == (HBITMAP) 1)
   {
      HGDIOBJ old = SelectObject(hdc, Brush55AA());
      SetTextColor(hdc, RGB(255, 255, 255));
      SetBkColor(hdc, RGB(0, 0, 0));
      PatBlt(hdc, caret.pos.x, caret.pos.y, caret.width, caret.height, PATINVERT);
      SelectObject(hdc, old);
   }
   else
      PatBlt(hdc, caret.pos.x, caret.pos.y, caret.width, caret.height, DSTINVERT);
   ReleaseDC(caret.hwnd, hdc);
   caret.drawn = !caret.drawn;
}

bool ShouldShow()
{
   return caret.hwnd && caret.hidden <= 0 && !caret.painting && IsWindowVisible(caret.hwnd);
}

void Erase()
{
   if (caret.drawn && IsWindow(caret.hwnd))
      Invert();
   caret.drawn = false;
}

void StopBlink()
{
   if (caret.timer)
      loop::RemoveTimeout(caret.timer);
   caret.timer = 0;
}

// Draws the caret now (if it shows) and restarts the blinking, as moving or
// showing it does on Windows.
void Restart()
{
   StopBlink();
   if (!ShouldShow())
      return;
   if (!caret.drawn)
      Invert();
   caret.timer = loop::AddTimeout(blink_time, [] {
      if (!IsWindow(caret.hwnd))
      {
         StopBlink();
         caret = Caret();
         return;
      }
      if (caret.drawn || ShouldShow())
         Invert();
   });
}

void Destroy()
{
   StopBlink();
   Erase();
   caret = Caret();
}

}  // namespace

void CaretBeginPaint(HWND hwnd)
{
   if (caret.hwnd == hwnd && hwnd)
   {
      Erase();
      StopBlink();
      caret.painting = true;
   }
}

void CaretEndPaint(HWND hwnd)
{
   if (caret.hwnd == hwnd && hwnd && caret.painting)
   {
      caret.painting = false;
      Restart();
   }
}

void CaretWindowDestroyed(HWND hwnd)
{
   if (caret.hwnd == hwnd)
   {
      StopBlink();
      caret = Caret();   // its pixels go with the window
   }
}

bool CaretHideIn(HWND hwnd, const RECT &client_rect)
{
   if (caret.hwnd != hwnd || !caret.drawn)
      return false;
   RECT c = {caret.pos.x, caret.pos.y, caret.pos.x + caret.width, caret.pos.y + caret.height}, i;
   if (!IntersectRect(&i, &c, &client_rect))
      return false;
   Erase();
   StopBlink();
   return true;
}

void CaretRestore()
{
   Restart();
}

}  // namespace w32

using namespace w32;

BOOL WINAPI CreateCaret(HWND hwnd, HBITMAP bitmap, INT width, INT height)
{
   if (!IsWindow(hwnd))
   {
      SetLastError(ERROR_INVALID_WINDOW_HANDLE);
      return FALSE;
   }
   Destroy();
   caret.hwnd = hwnd;
   caret.bitmap = bitmap;
   if (bitmap && bitmap != (HBITMAP) 1)
   {
      BITMAP bm;
      if (!GetObjectA(bitmap, sizeof(bm), &bm))
      {
         caret = Caret();
         return FALSE;
      }
      width = bm.bmWidth;
      height = bm.bmHeight;
   }
   caret.width = width ? width : GetSystemMetrics(SM_CXBORDER);
   caret.height = height ? height : GetSystemMetrics(SM_CYBORDER);
   return TRUE;
}

BOOL WINAPI DestroyCaret(void)
{
   if (caret.hwnd == nullptr)
      return FALSE;
   Destroy();
   return TRUE;
}

BOOL WINAPI SetCaretPos(INT x, INT y)
{
   if (caret.hwnd == nullptr)
      return FALSE;
   if (caret.pos.x == x && caret.pos.y == y)
      return TRUE;
   Erase();
   caret.pos = {x, y};
   Restart();
   return TRUE;
}

BOOL WINAPI GetCaretPos(POINT *pos)
{
   if (caret.hwnd == nullptr || pos == nullptr)
      return FALSE;
   *pos = caret.pos;
   return TRUE;
}

BOOL WINAPI ShowCaret(HWND hwnd)
{
   if (caret.hwnd == nullptr || (hwnd && hwnd != caret.hwnd))
      return FALSE;
   if (caret.hidden > 0)
      caret.hidden--;
   if (caret.hidden == 0)
      Restart();
   return TRUE;
}

BOOL WINAPI HideCaret(HWND hwnd)
{
   if (caret.hwnd == nullptr || (hwnd && hwnd != caret.hwnd))
      return FALSE;
   caret.hidden++;
   StopBlink();
   Erase();
   return TRUE;
}

UINT WINAPI GetCaretBlinkTime(void)
{
   return blink_time;
}

BOOL WINAPI SetCaretBlinkTime(UINT ms)
{
   blink_time = ms ? ms : 530;
   if (caret.timer)
      Restart();
   return TRUE;
}
