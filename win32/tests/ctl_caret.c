/*
 * ctl_caret.c:  The caret: drawn inverted on the window's surface, shown and
 *   hidden by count, moved, blinking, off while the window paints, gone with
 *   DestroyCaret; read back with GetPixel on the window's DC.
 */

#include "tests/ctl_common.h"

static HWND target;
static COLORREF during_paint = CLR_INVALID;

static LRESULT CALLBACK PaintProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_PAINT)
   {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      during_paint = GetPixel(hdc, 5, 5);   // the caret is off while painting
      RECT r;
      GetClientRect(hwnd, &r);
      FillRect(hdc, &r, (HBRUSH) GetStockObject(WHITE_BRUSH));
      EndPaint(hwnd, &ps);
      return 0;
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static COLORREF At(int x, int y)
{
   HDC hdc = GetDC(target);
   COLORREF c = GetPixel(hdc, x, y);
   ReleaseDC(target, hdc);
   return c;
}

static void Wait(DWORD ms)
{
   DWORD end = GetTickCount() + ms;
   while ((LONG) (GetTickCount() - end) < 0)
   {
      Pump();
      Sleep(5);
   }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
   HWND parent = MakeParent(200, 200);
   WNDCLASSA wc = {0};
   wc.lpfnWndProc = PaintProc;
   wc.hInstance = instance;
   wc.lpszClassName = "CaretTarget";
   RegisterClassA(&wc);
   target = CreateWindowExA(0, "CaretTarget", "", WS_CHILD | WS_VISIBLE, 10, 10, 100, 50, parent, NULL, instance, NULL);
   // The parent doesn't paint over its child (which would take the caret's
   // pixels with it, as on Windows); everything painted before the caret shows.
   SetWindowLongA(parent, GWL_STYLE, GetWindowLongA(parent, GWL_STYLE) | WS_CLIPCHILDREN);
   UpdateWindow(parent);
   Pump();
   CHECK(At(5, 5) == RGB(255, 255, 255));

   CHECK(CreateCaret(target, NULL, 2, 12));
   CHECK(SetCaretPos(4, 4));
   POINT p;
   CHECK(GetCaretPos(&p));
   CHECK(p.x == 4 && p.y == 4);
   CHECK(At(5, 5) == RGB(255, 255, 255));   // hidden until shown
   CHECK(ShowCaret(target));
   CHECK(At(5, 5) == RGB(0, 0, 0));
   CHECK(At(6, 5) == RGB(255, 255, 255));   // 2 wide
   // Counted: two hides need two shows.
   HideCaret(target);
   HideCaret(target);
   CHECK(At(5, 5) == RGB(255, 255, 255));
   ShowCaret(target);
   CHECK(At(5, 5) == RGB(255, 255, 255));
   ShowCaret(target);
   CHECK(At(5, 5) == RGB(0, 0, 0));
   // Moving it moves the inverted pixels.
   SetCaretPos(30, 4);
   CHECK(At(5, 5) == RGB(255, 255, 255));
   CHECK(At(31, 5) == RGB(0, 0, 0));
   // It blinks.
   CHECK_INT(GetCaretBlinkTime(), 530);
   SetCaretBlinkTime(100);
   int on = 0, off = 0;
   for (int i = 0; i < 20; i++)
   {
      Wait(25);
      if (At(31, 5) == RGB(0, 0, 0))
         on++;
      else
         off++;
   }
   CHECK(on > 2 && off > 2);
   SetCaretBlinkTime(530);
   // Off while painting, back after.
   SetCaretPos(4, 4);
   InvalidateRect(target, NULL, TRUE);
   UpdateWindow(target);
   CHECK(during_paint == RGB(255, 255, 255));
   CHECK(At(5, 5) == RGB(0, 0, 0));
   // Only its owner shows or hides it.
   CHECK(!HideCaret(parent));
   CHECK(DestroyCaret());
   CHECK(At(5, 5) == RGB(255, 255, 255));
   CHECK(!GetCaretPos(&p));
   CHECK(!ShowCaret(target));

   // A gray caret (bitmap 1) and one going with its window.
   CHECK(CreateCaret(target, (HBITMAP) 1, 4, 4));
   SetCaretPos(10, 10);
   ShowCaret(target);
   int changed = 0;
   for (int y = 10; y < 14; y++)
      for (int x = 10; x < 14; x++)
         changed += At(x, y) != RGB(255, 255, 255);
   CHECK(changed >= 4 && changed <= 12);
   DestroyWindow(target);
   CHECK(!SetCaretPos(1, 1));

   DestroyWindow(parent);
   return Report("ctl_caret");
}
