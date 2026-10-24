/*
 * ctl_scroll.c:  Scroll bars: the ScrollBar control (SB_CTL) as the client's
 *   inventory and stats lists use it, a window's standard bar (WS_VSCROLL), the
 *   scroll API, WM_VSCROLL/WM_HSCROLL from clicks, auto-repeat, thumb drags and
 *   keys, and the classic rendering.
 */

#include "tests/ctl_common.h"

static int line_downs, ends, thumb_tracks, thumb_positions;

static HWND release_target;
static int release_after;      // timer ticks before the button goes up
static int release_at_lines;   // or, if set, once this many SB_LINEDOWNs came (10 s at most)

static void CALLBACK Release(HWND, UINT, UINT_PTR id, DWORD)
{
   if (release_at_lines ? line_downs < release_at_lines && --release_after > 0 : --release_after > 0)
      return;
   KillTimer(NULL, id);
   release_at_lines = 0;
   PostMessageA(release_target, WM_LBUTTONUP, 0, MAKELPARAM(8, 8));
}

static WPARAM last_track;

static LRESULT CALLBACK OwnerProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (msg == WM_VSCROLL || msg == WM_HSCROLL)
   {
      rcv.scrolls++;
      rcv.last_scroll = wparam;
      rcv.last_scroll_control = (HWND) lparam;
      rcv.last_scroll_msg = msg;
      switch (LOWORD(wparam))
      {
      case SB_LINEDOWN: line_downs++; break;
      case SB_ENDSCROLL: ends++; break;
      case SB_THUMBTRACK: thumb_tracks++, last_track = wparam; break;
      case SB_THUMBPOSITION: thumb_positions++; break;
      }
      return 0;
   }
   if (msg == WM_SIZE)
      rcv.measures++;   // counts WM_SIZE here
   return ParentProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
   WNDCLASSA wc = {0};
   wc.lpfnWndProc = OwnerProc;
   wc.hInstance = instance;
   wc.lpszClassName = "ScrollOwner";
   RegisterClassA(&wc);
   HWND parent = CreateWindowExA(0, "ScrollOwner", "", WS_POPUP, 0, 0, 300, 300, NULL, NULL, instance, NULL);
   SetWindowLongA(parent, GWL_STYLE, GetWindowLongA(parent, GWL_STYLE) | WS_VISIBLE);

   // The control.
   HWND bar = CreateWindowExA(0, "SCROLLBAR", "", WS_CHILD | WS_VISIBLE | SBS_VERT, 10, 10, 16, 150, parent,
                              (HMENU) 601, instance, NULL);
   CHECK(bar != NULL);
   SCROLLINFO si = {sizeof(si), SIF_RANGE | SIF_PAGE | SIF_POS};
   si.nMin = 0;
   si.nMax = 99;
   si.nPage = 10;
   si.nPos = 5;
   CHECK_INT(SetScrollInfo(bar, SB_CTL, &si, TRUE), 5);
   SCROLLINFO got = {sizeof(got), SIF_ALL};
   CHECK(GetScrollInfo(bar, SB_CTL, &got));
   CHECK_INT(got.nMin, 0);
   CHECK_INT(got.nMax, 99);
   CHECK_INT(got.nPage, 10);
   CHECK_INT(got.nPos, 5);
   // Positions clamp to max - page + 1.
   CHECK_INT(SetScrollPos(bar, SB_CTL, 200, TRUE), 5);   // the previous position
   CHECK_INT(GetScrollPos(bar, SB_CTL), 90);
   int mn = -1, mx = -1;
   SetScrollRange(bar, SB_CTL, 0, 20, TRUE);
   GetScrollRange(bar, SB_CTL, &mn, &mx);
   CHECK_INT(mn, 0);
   CHECK_INT(mx, 20);
   CHECK_INT(SendMessageA(bar, SBM_GETPOS, 0, 0), 11);   // 20 - 10 + 1
   SetScrollPos(bar, SB_CTL, 0, TRUE);
   CHECK_INT(SendMessageA(bar, SBM_SETPOS, 3, TRUE), 0);
   CHECK_INT(SendMessageA(bar, SBM_GETPOS, 0, 0), 3);
   si.fMask = SIF_ALL;
   si.nMax = 99;
   si.nPos = 0;
   SetScrollInfo(bar, SB_CTL, &si, TRUE);

   // A click on the down arrow: SB_LINEDOWN, then SB_ENDSCROLL, lParam the bar.
   memset(&rcv, 0, sizeof(rcv));
   line_downs = ends = 0;
   PostMessageA(bar, WM_LBUTTONUP, 0, MAKELPARAM(8, 145));
   SendMessageA(bar, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(8, 145));
   CHECK_INT(line_downs, 1);
   CHECK_INT(ends, 1);
   CHECK(rcv.last_scroll_control == bar);
   CHECK_INT(rcv.last_scroll_msg, WM_VSCROLL);
   CHECK(GetCapture() == NULL);
   // Held down: it repeats (after 200 ms, every 50 ms).
   line_downs = ends = 0;
   release_target = bar;
   release_at_lines = 4;
   release_after = 1000;
   SetTimer(NULL, 0, 10, Release);
   SendMessageA(bar, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(8, 145));
   CHECK(line_downs >= 4);
   CHECK_INT(ends, 1);
   // Page down below the thumb.
   memset(&rcv, 0, sizeof(rcv));
   PostMessageA(bar, WM_LBUTTONUP, 0, MAKELPARAM(8, 120));
   SendMessageA(bar, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(8, 120));
   CHECK_INT(rcv.scrolls, 2);
   // The thumb at the top (pos 0): drag it down 40 pixels.
   thumb_tracks = thumb_positions = ends = 0;
   PostMessageA(bar, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(8, 60));
   PostMessageA(bar, WM_LBUTTONUP, 0, MAKELPARAM(8, 60));
   SendMessageA(bar, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(8, 20));
   CHECK(thumb_tracks >= 1);
   CHECK_INT(thumb_positions, 1);
   CHECK_INT(ends, 1);
   // 40 of the 118 - 12 = 106 track pixels the thumb can move: about 40 * 90 / 106.
   CHECK(HIWORD(last_track) >= 30 && HIWORD(last_track) <= 38);
   // Keys.
   memset(&rcv, 0, sizeof(rcv));
   Key(bar, VK_DOWN);
   CHECK_INT(LOWORD(rcv.last_scroll), SB_LINEDOWN);
   Key(bar, VK_NEXT);
   CHECK_INT(LOWORD(rcv.last_scroll), SB_PAGEDOWN);
   Key(bar, VK_HOME);
   CHECK_INT(LOWORD(rcv.last_scroll), SB_TOP);
   CHECK_INT(SendMessageA(bar, WM_GETDLGCODE, 0, 0), DLGC_WANTARROWS);

   // Enabling.
   CHECK(EnableScrollBar(bar, SB_CTL, ESB_DISABLE_BOTH));
   CHECK(!IsWindowEnabled(bar));
   CHECK(EnableScrollBar(bar, SB_CTL, ESB_ENABLE_BOTH));
   CHECK(IsWindowEnabled(bar));
   SCROLLBARINFO sbi = {sizeof(sbi)};
   CHECK(GetScrollBarInfo(bar, OBJID_CLIENT, &sbi));
   CHECK_INT(sbi.dxyLineButton > 0, 1);

   // Rendering: arrows (dark glyphs), a raised thumb, the dithered track.
   SetScrollPos(bar, SB_CTL, 0, TRUE);
   struct Image im = Render(bar);
   SaveBmp(&im, "ctl_scrollbar.bmp");
   RECT up = {3, 3, 13, 13};
   CHECK(CountDark(&im, up) >= 4);
   CHECK(Pixel(&im, 8, 20) == GetSysColor(COLOR_BTNFACE));   // the thumb's face
   RECT track = {2, 60, 14, 120};
   CHECK(CountColor(&im, track, RGB(255, 255, 255)) > 50);
   CHECK(CountColor(&im, track, GetSysColor(COLOR_3DFACE)) > 50);
   FreeImage(&im);

   // A window's standard bar: shown by a range that scrolls, hidden by one that
   // doesn't; the client area narrows and WM_SIZE comes.
   HWND win = CreateWindowExA(0, "ScrollOwner", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 50, 10, 120, 100, parent,
                              (HMENU) 602, instance, NULL);
   RECT cr;
   GetClientRect(win, &cr);
   CHECK_INT(cr.right, 118);
   rcv.measures = 0;
   SCROLLINFO ws = {sizeof(ws), SIF_RANGE | SIF_PAGE};
   ws.nMax = 50;
   ws.nPage = 10;
   SetScrollInfo(win, SB_VERT, &ws, TRUE);
   CHECK(GetWindowLongA(win, GWL_STYLE) & WS_VSCROLL);
   GetClientRect(win, &cr);
   CHECK_INT(cr.right, 118 - GetSystemMetrics(SM_CXVSCROLL));
   CHECK(rcv.measures >= 1);
   ws.nMax = 5;
   SetScrollInfo(win, SB_VERT, &ws, TRUE);
   CHECK(!(GetWindowLongA(win, GWL_STYLE) & WS_VSCROLL));
   GetClientRect(win, &cr);
   CHECK_INT(cr.right, 118);
   ShowScrollBar(win, SB_VERT, TRUE);
   CHECK(GetWindowLongA(win, GWL_STYLE) & WS_VSCROLL);
   ws.nMax = 50;
   ws.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
   ws.nPos = 0;
   SetScrollInfo(win, SB_VERT, &ws, TRUE);
   // Its non-client hit test and a click on its down arrow: WM_VSCROLL to the
   // window itself, lParam NULL.
   RECT wr;
   GetWindowRect(win, &wr);
   int bx = wr.right - 1 - GetSystemMetrics(SM_CXVSCROLL) / 2;
   CHECK_INT(SendMessageA(win, WM_NCHITTEST, 0, MAKELPARAM(bx, wr.top + 50)), HTVSCROLL);
   CHECK_INT(SendMessageA(win, WM_NCHITTEST, 0, MAKELPARAM(wr.left, wr.top + 50)), HTBORDER);
   memset(&rcv, 0, sizeof(rcv));
   line_downs = ends = 0;
   PostMessageA(win, WM_LBUTTONUP, 0, 0);
   SendMessageA(win, WM_NCLBUTTONDOWN, HTVSCROLL, MAKELPARAM(bx, wr.bottom - 5));
   CHECK_INT(line_downs, 1);
   CHECK_INT(ends, 1);
   CHECK(rcv.last_scroll_control == NULL);
   SetScrollPos(win, SB_VERT, 20, TRUE);
   CHECK_INT(GetScrollPos(win, SB_VERT), 20);
   // Rendering: the window with its bar.
   im = Render(win);
   SaveBmp(&im, "ctl_scroll_window.bmp");
   CHECK(Pixel(&im, 0, 0) == GetSysColor(COLOR_WINDOWFRAME));   // WS_BORDER
   CHECK(Pixel(&im, im.width - 1 - 8, 1 + 8) != RGB(255, 0, 255));   // the bar was drawn
   FreeImage(&im);

   DestroyWindow(parent);
   return Report("ctl_scroll");
}
