/*
 * cc_trackbar.c:  The trackbar (Wine's, msctls_trackbar32) as the client's
 *   preferences use it (TBS_BOTH | TBS_NOTICKS, TBS_AUTOTICKS): TBM_SETRANGE,
 *   TBM_SETPOS, TBM_GETPOS, TBM_SETTICFREQ, the keyboard (arrows, Page, Home,
 *   End) with WM_HSCROLL to the parent, dragging the thumb, a click on the
 *   channel; renders by WM_PRINTCLIENT, checks the thumb moves and writes
 *   cc_trackbar.bmp.
 */

#include "cc_util.h"

static HWND bar;
static int scrolls;
static WORD last_code;

static LRESULT CALLBACK ParentProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_HSCROLL && (HWND) lparam == bar)
   {
      scrolls++;
      last_code = LOWORD(wparam);
      return 0;
   }
   return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static int Pos(void)
{
   return (int) SendMessageA(bar, TBM_GETPOS, 0, 0);
}

static void Key(int vk)
{
   SendMessageA(bar, WM_KEYDOWN, vk, 0);
   SendMessageA(bar, WM_KEYUP, vk, 0);
}

// The thumb's center, by its face color in a rendering (x of the median face pixel).
static int ThumbX(const char *dump)
{
   RECT cr;
   GetClientRect(bar, &cr);
   Canvas32 c = MakeCanvas(cr.right, cr.bottom, RGB(255, 0, 255));
   SendMessageA(bar, WM_PRINTCLIENT, (WPARAM) c.dc, PRF_CLIENT);
   RECT thumb;
   SendMessageA(bar, TBM_GETTHUMBRECT, 0, (LPARAM) &thumb);
   // The thumb's highlight edge (white) on its left side.
   int white_left = -1;
   for (int x = 0; x < c.width && white_left < 0; x++)
      if (CanvasPixel(&c, x, (thumb.top + thumb.bottom) / 2) == GetSysColor(COLOR_BTNHIGHLIGHT) &&
          CanvasPixel(&c, x, thumb.top + 3) == GetSysColor(COLOR_BTNHIGHLIGHT))
         white_left = x;
   // The channel: a sunken edge (shadow color) across.
   RECT channel;
   SendMessageA(bar, TBM_GETCHANNELRECT, 0, (LPARAM) &channel);
   CHECK(channel.right - channel.left > 100);
   CHECK(CountColor(&c, RECT{channel.left, channel.top, channel.right, channel.top + 1},
                    GetSysColor(COLOR_BTNSHADOW), 0) > 50);
   // The background: the parent's WM_CTLCOLORSTATIC brush (button face).
   CHECK(CountColor(&c, RECT{0, 0, c.width, 2}, GetSysColor(COLOR_BTNFACE), 0) > c.width);
   if (dump)
      DumpBmp(&c, dump);
   FreeCanvas(&c);
   CHECK_INT(white_left, thumb.left);   // BF_SOFT: the highlight is the outer edge
   return (thumb.left + thumb.right) / 2;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   (void) prev, (void) cmd, (void) show;
   InitCommonControls();
   HWND parent = MakeParent(ParentProc, 300, 100);
   bar = CreateWindowExA(0, TRACKBAR_CLASSA, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_BOTH | TBS_AUTOTICKS,
                         10, 10, 220, 30, parent, (HMENU) 7, instance, NULL);
   CHECK(bar != NULL);
   if (bar == NULL)
      return CheckReport("cc_trackbar");

   // As the preferences dialog: range 0..100, ticks every 10.
   SendMessageA(bar, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
   CHECK_INT(SendMessageA(bar, TBM_GETRANGEMIN, 0, 0), 0);
   CHECK_INT(SendMessageA(bar, TBM_GETRANGEMAX, 0, 0), 100);
   SendMessageA(bar, TBM_SETTICFREQ, 10, 0);
   CHECK_INT(SendMessageA(bar, TBM_GETNUMTICS, 0, 0), 11);
   SendMessageA(bar, TBM_SETPOS, TRUE, 30);
   CHECK_INT(Pos(), 30);
   SendMessageA(bar, TBM_SETPOS, TRUE, 500);   // clamped
   CHECK_INT(Pos(), 100);
   SendMessageA(bar, TBM_SETPOS, TRUE, 0);
   int x0 = ThumbX(NULL);

   // Keyboard: line 1, page (range / 5) 20, ends.
   SetFocus(bar);
   scrolls = 0;
   Key(VK_RIGHT);
   CHECK_INT(Pos(), 1);
   CHECK(scrolls >= 1);
   CHECK(last_code == TB_LINEDOWN || last_code == TB_ENDTRACK);
   Key(VK_NEXT);
   CHECK_INT(Pos(), 21);
   Key(VK_LEFT);
   CHECK_INT(Pos(), 20);
   Key(VK_PRIOR);
   CHECK_INT(Pos(), 0);
   Key(VK_END);
   CHECK_INT(Pos(), 100);
   Key(VK_HOME);
   CHECK_INT(Pos(), 0);
   Key(VK_DOWN);   // down/right increase too
   CHECK_INT(Pos(), 1);

   // The thumb follows the position.
   SendMessageA(bar, TBM_SETPOS, TRUE, 50);
   int x50 = ThumbX(NULL);
   SendMessageA(bar, TBM_SETPOS, TRUE, 100);
   int x100 = ThumbX(NULL);
   CHECK(x0 < x50 && x50 < x100);
   CHECK(abs((x50 - x0) - (x100 - x50)) <= 1);

   // Dragging the thumb from 50 to the far left end.
   SendMessageA(bar, TBM_SETPOS, TRUE, 50);
   RECT thumb;
   SendMessageA(bar, TBM_GETTHUMBRECT, 0, (LPARAM) &thumb);
   int cy = (thumb.top + thumb.bottom) / 2;
   scrolls = 0;
   SendMessageA(bar, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM((thumb.left + thumb.right) / 2, cy));
   SendMessageA(bar, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM((thumb.left + thumb.right) / 2 - (x50 - x0) / 2, cy));
   int mid = Pos();
   CHECK(mid > 15 && mid < 35);
   SendMessageA(bar, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(0, cy));
   SendMessageA(bar, WM_LBUTTONUP, 0, MAKELPARAM(0, cy));
   CHECK_INT(Pos(), 0);
   CHECK(scrolls >= 2);
   CHECK_INT(last_code, TB_ENDTRACK);

   // A click on the channel right of the thumb pages toward it.
   SendMessageA(bar, TBM_SETPOS, TRUE, 0);
   RECT channel;
   SendMessageA(bar, TBM_GETCHANNELRECT, 0, (LPARAM) &channel);
   SendMessageA(bar, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(channel.right - 5, cy));
   SendMessageA(bar, WM_LBUTTONUP, 0, MAKELPARAM(channel.right - 5, cy));
   CHECK_INT(Pos(), 20);

   SendMessageA(bar, TBM_SETPOS, TRUE, 60);
   ThumbX("cc_trackbar.bmp");
   DestroyWindow(parent);
   return CheckReport("cc_trackbar");
}
