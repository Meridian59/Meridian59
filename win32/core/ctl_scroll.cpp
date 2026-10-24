/*
 * ctl_scroll.cpp:  Scroll bars: the "ScrollBar" control (SB_CTL) and the
 *   standard bars of windows with WS_VSCROLL/WS_HSCROLL in their non-client
 *   area, their state (SetScrollInfo ...), classic drawing, and mouse tracking
 *   with auto-repeat sending WM_VSCROLL/WM_HSCROLL.  After Wine 10's
 *   win32u/scroll.c and user32/scroll.c (LGPL 2.1+, Martin Ayotte, Alexandre
 *   Julliard).
 */

#include "core/scroll.h"

#include "core/window.h"

#include <algorithm>
#include <map>

namespace w32 {

HPEN SysColorPen(int index);
HBRUSH Brush55AA();

namespace {

constexpr int kMinRect = 4;          // between the arrows
constexpr int kMinThumb = 17;
constexpr UINT kFirstDelay = 200;    // before the first repeat
constexpr UINT kRepeatDelay = 50;
constexpr UINT_PTR kTimer = 0;       // the system timer's id (Windows')

enum HitTest
{
   SCROLL_NOWHERE,
   SCROLL_TOP_ARROW,
   SCROLL_TOP_RECT,
   SCROLL_THUMB,
   SCROLL_BOTTOM_RECT,
   SCROLL_BOTTOM_ARROW
};

struct Info
{
   INT cur = 0, min = 0, max = 100;
   INT page = 0;
   UINT flags = ESB_ENABLE_BOTH;
   bool painted = false;
};

struct Tracking
{
   HWND win = nullptr;
   int bar = 0;
   int thumb_pos = 0;
   int thumb_val = 0;
   bool vertical = false;
   HitTest hit_test = SCROLL_NOWHERE;
};

Tracking tracking;
bool moving_thumb = false;
bool timer_running = false;

struct WindowBars
{
   Info horz, vert;
};

std::map<HWND, WindowBars> &Standard()
{
   static std::map<HWND, WindowBars> bars;
   return bars;
}

std::map<HWND, Info> &Controls()
{
   static std::map<HWND, Info> controls;
   return controls;
}


Info *GetInfo(HWND hwnd, int bar, bool alloc)
{
   if (!IsWindow(hwnd))
      return nullptr;
   switch (bar)
   {
   case SB_HORZ:
   case SB_VERT:
   {
      auto &bars = Standard();
      auto it = bars.find(hwnd);
      if (it == bars.end())
      {
         if (!alloc)
            return nullptr;
         it = bars.emplace(hwnd, WindowBars()).first;
      }
      return bar == SB_HORZ ? &it->second.horz : &it->second.vert;
   }
   case SB_CTL:
   {
      auto it = Controls().find(hwnd);
      return it == Controls().end() ? nullptr : &it->second;
   }
   }
   return nullptr;
}

DWORD Style(HWND hwnd)
{
   return (DWORD) GetWindowLongW(hwnd, GWL_STYLE);
}

bool ShowBar(HWND hwnd, int bar, bool show_horz, bool show_vert)
{
   DWORD set = 0, clear = 0;
   switch (bar)
   {
   case SB_CTL:
      ShowWindow(hwnd, show_horz ? SW_SHOW : SW_HIDE);
      return true;
   case SB_BOTH:
   case SB_HORZ:
      (show_horz ? set : clear) |= WS_HSCROLL;
      if (bar == SB_HORZ)
         break;
      [[fallthrough]];
   case SB_VERT:
      (show_vert ? set : clear) |= WS_VSCROLL;
      break;
   default:
      return false;
   }
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return false;
   DWORD old = w->style;
   // The style bits change without WM_STYLECHANGING, as Windows' set_window_style.
   w->style = (old | set) & ~clear;
   if ((old & clear) || (old & set) != set)
   {
      SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
      return true;
   }
   return false;
}

// The bar's rectangle in drawing coordinates (client for SB_CTL, window for the
// standard bars), the arrow size, thumb size and position; true if vertical.
bool BarRect(HWND hwnd, int bar, RECT *rect, int *arrow, int *thumb_size, int *thumb_pos)
{
   Window *w = GetWindowObject(hwnd);
   *arrow = *thumb_size = *thumb_pos = 0;
   SetRectEmpty(rect);
   if (w == nullptr)
      return false;
   bool vertical;
   switch (bar)
   {
   case SB_HORZ:
      *rect = w->client;
      rect->top = rect->bottom;
      rect->bottom += GetSystemMetrics(SM_CYHSCROLL);
      if (w->style & WS_VSCROLL)
         rect->right++;
      vertical = false;
      break;
   case SB_VERT:
      *rect = w->client;
      if (w->exstyle & WS_EX_LEFTSCROLLBAR)
      {
         rect->right = rect->left;
         rect->left -= GetSystemMetrics(SM_CXVSCROLL);
      }
      else
      {
         rect->left = rect->right;
         rect->right += GetSystemMetrics(SM_CXVSCROLL);
      }
      if (w->style & WS_HSCROLL)
         rect->bottom++;
      vertical = true;
      break;
   case SB_CTL:
      GetClientRect(hwnd, rect);
      vertical = (w->style & SBS_VERT) != 0;
      break;
   default:
      return false;
   }
   int pixels = vertical ? rect->bottom - rect->top : rect->right - rect->left;
   int cxv = GetSystemMetrics(SM_CXVSCROLL);
   if (pixels <= 2 * cxv + kMinRect)
   {
      *arrow = pixels > kMinRect ? (pixels - kMinRect) / 2 : 0;
      return vertical;
   }
   Info *info = GetInfo(hwnd, bar, true);
   if (info == nullptr)
      return vertical;
   *arrow = cxv;
   pixels -= 2 * cxv;
   if (info->page)
      *thumb_size = std::max(MulDiv(pixels, info->page, info->max - info->min + 1), kMinThumb);
   else
      *thumb_size = cxv;
   if ((pixels -= *thumb_size) < 0 || (info->flags & ESB_DISABLE_BOTH) == ESB_DISABLE_BOTH)
      *thumb_pos = *thumb_size = 0;
   else
   {
      int max = info->max - std::max(info->page - 1, 0);
      if (info->min >= max)
         *thumb_pos = *arrow;
      else
         *thumb_pos = *arrow + MulDiv(pixels, info->cur - info->min, max - info->min);
   }
   return vertical;
}

void DrawArrows(HDC hdc, UINT flags, const RECT &rect, int arrow, bool vertical, bool top_pressed,
                bool bottom_pressed)
{
   RECT r = rect;
   if (vertical)
      r.bottom = r.top + arrow;
   else
      r.right = r.left + arrow;
   DrawFrameControl(hdc, &r, DFC_SCROLL,
                    (vertical ? DFCS_SCROLLUP : DFCS_SCROLLLEFT) | (top_pressed ? (DFCS_PUSHED | DFCS_FLAT) : 0) |
                       ((flags & ESB_DISABLE_LTUP) ? DFCS_INACTIVE : 0));
   r = rect;
   if (vertical)
      r.top = r.bottom - arrow;
   else
      r.left = r.right - arrow;
   DrawFrameControl(hdc, &r, DFC_SCROLL,
                    (vertical ? DFCS_SCROLLDOWN : DFCS_SCROLLRIGHT) | (bottom_pressed ? (DFCS_PUSHED | DFCS_FLAT) : 0) |
                       ((flags & ESB_DISABLE_RTDN) ? DFCS_INACTIVE : 0));
}

void DrawInterior(HWND hwnd, HDC hdc, int bar, const RECT &rect, int arrow, int thumb_size, int thumb_pos,
                  bool vertical, bool top_selected, bool bottom_selected)
{
   HBRUSH brush;
   if (bar == SB_CTL)
      brush = (HBRUSH) SendMessageW(GetParent(hwnd), WM_CTLCOLORSCROLLBAR, (WPARAM) hdc, (LPARAM) hwnd);
   else
   {
      COLORREF bk = GetSysColor(COLOR_3DHILIGHT);
      SetTextColor(hdc, GetSysColor(COLOR_3DFACE));
      SetBkColor(hdc, bk);
      // A dithered background when it would match the window's.
      brush = bk == GetSysColor(COLOR_WINDOW) ? Brush55AA() : GetSysColorBrush(COLOR_SCROLLBAR);
   }
   HGDIOBJ save_pen = SelectObject(hdc, SysColorPen(COLOR_WINDOWFRAME));
   HGDIOBJ save_brush = SelectObject(hdc, brush);
   RECT r = rect;
   if (vertical)
      r.top += arrow, r.bottom -= arrow;
   else
      r.left += arrow, r.right -= arrow;
   if (!thumb_pos)
   {
      PatBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, PATCOPY);
      SelectObject(hdc, save_pen);
      SelectObject(hdc, save_brush);
      return;
   }
   if (vertical)
   {
      PatBlt(hdc, r.left, r.top, r.right - r.left, thumb_pos - arrow, top_selected ? 0x0f0000 : PATCOPY);
      r.top += thumb_pos - arrow;
      PatBlt(hdc, r.left, r.top + thumb_size, r.right - r.left, r.bottom - r.top - thumb_size,
             bottom_selected ? 0x0f0000 : PATCOPY);
      r.bottom = r.top + thumb_size;
   }
   else
   {
      PatBlt(hdc, r.left, r.top, thumb_pos - arrow, r.bottom - r.top, top_selected ? 0x0f0000 : PATCOPY);
      r.left += thumb_pos - arrow;
      PatBlt(hdc, r.left + thumb_size, r.top, r.right - r.left - thumb_size, r.bottom - r.top,
             bottom_selected ? 0x0f0000 : PATCOPY);
      r.right = r.left + thumb_size;
   }
   SelectObject(hdc, GetSysColorBrush(COLOR_BTNFACE));
   Rectangle(hdc, r.left + 1, r.top + 1, r.right - 1, r.bottom - 1);
   DrawEdge(hdc, &r, EDGE_RAISED, BF_RECT);
   SelectObject(hdc, save_pen);
   SelectObject(hdc, save_brush);
}

void DrawBar(HWND hwnd, HDC hdc, int bar, HitTest hit_test, bool arrows, bool interior)
{
   DWORD style = Style(hwnd);
   if ((bar == SB_VERT && !(style & WS_VSCROLL)) || (bar == SB_HORZ && !(style & WS_HSCROLL)))
      return;
   Info *info = GetInfo(hwnd, bar, true);
   if (info == nullptr)
      return;
   UINT enable = info->flags;
   RECT rect;
   int arrow, thumb_size, thumb_pos;
   bool vertical;
   if (bar == SB_CTL && (style & (SBS_SIZEGRIP | SBS_SIZEBOX)))
   {
      GetClientRect(hwnd, &rect);
      FillRect(hdc, &rect, GetSysColorBrush(COLOR_BTNFACE));
      if (style & SBS_SIZEGRIP)
      {
         RECT rc = rect;
         rc.left = std::max(rc.left, rc.right - GetSystemMetrics(SM_CXVSCROLL) - 1);
         rc.top = std::max(rc.top, rc.bottom - GetSystemMetrics(SM_CYHSCROLL) - 1);
         DrawFrameControl(hdc, &rc, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
      }
      return;
   }
   vertical = BarRect(hwnd, bar, &rect, &arrow, &thumb_size, &thumb_pos);
   if (moving_thumb && tracking.win == hwnd && tracking.bar == bar)
   {
      int max_size = (vertical ? rect.bottom - rect.top : rect.right - rect.left) - (arrow + thumb_size);
      thumb_pos = std::clamp(tracking.thumb_pos, arrow, std::max(arrow, max_size));
   }
   if (IsRectEmpty(&rect))
      return;
   bool pressed = vertical == tracking.vertical && GetCapture() == hwnd;
   if (arrows && arrow)
      DrawArrows(hdc, enable, rect, arrow, vertical,
                 pressed && hit_test == tracking.hit_test && hit_test == SCROLL_TOP_ARROW,
                 pressed && hit_test == tracking.hit_test && hit_test == SCROLL_BOTTOM_ARROW);
   if (interior)
      DrawInterior(hwnd, hdc, bar, rect, arrow, thumb_size, thumb_pos, vertical,
                   pressed && hit_test == tracking.hit_test && hit_test == SCROLL_TOP_RECT,
                   pressed && hit_test == tracking.hit_test && hit_test == SCROLL_BOTTOM_RECT);
   if (bar == SB_HORZ || bar == SB_VERT)
      info->painted = true;
   // A focused scroll bar control's caret follows the thumb.
   if (bar == SB_CTL && hwnd == GetFocus())
   {
      if (!vertical)
         SetCaretPos(thumb_pos + 1, rect.top + 1);
      else
         SetCaretPos(rect.top + 1, thumb_pos + 1);
   }
}

void Refresh(HWND hwnd, int bar, bool arrows, bool interior)
{
   if (!IsWindowVisible(hwnd))
      return;
   HDC hdc = GetDCEx(hwnd, nullptr, DCX_CACHE | (bar == SB_CTL ? 0 : DCX_WINDOW));
   if (hdc == nullptr)
      return;
   DrawBar(hwnd, hdc, bar, tracking.hit_test, arrows, interior);
   ReleaseDC(hwnd, hdc);
}

bool PointInScrollRect(const RECT &r, POINT pt, bool vertical)
{
   // Dragging may stray some way off the bar and still count.
   RECT rect = r;
   int width = vertical ? r.right - r.left : r.bottom - r.top;
   if (vertical)
      InflateRect(&rect, width * 8, width * 2);
   else
      InflateRect(&rect, width * 2, width * 8);
   return PtInRect(&rect, pt);
}

HitTest Hit(HWND hwnd, int bar, POINT pt, bool dragging)
{
   RECT rect;
   int arrow, thumb_size, thumb_pos;
   bool vertical = BarRect(hwnd, bar, &rect, &arrow, &thumb_size, &thumb_pos);
   if ((dragging && !PointInScrollRect(rect, pt, vertical)) || (!dragging && !PtInRect(&rect, pt)))
      return SCROLL_NOWHERE;
   if (vertical)
   {
      if (pt.y < rect.top + arrow)
         return SCROLL_TOP_ARROW;
      if (pt.y >= rect.bottom - arrow)
         return SCROLL_BOTTOM_ARROW;
      if (!thumb_pos)
         return SCROLL_TOP_RECT;
      pt.y -= rect.top;
      if (pt.y < thumb_pos)
         return SCROLL_TOP_RECT;
      if (pt.y >= thumb_pos + thumb_size)
         return SCROLL_BOTTOM_RECT;
   }
   else
   {
      if (pt.x < rect.left + arrow)
         return SCROLL_TOP_ARROW;
      if (pt.x >= rect.right - arrow)
         return SCROLL_BOTTOM_ARROW;
      if (!thumb_pos)
         return SCROLL_TOP_RECT;
      pt.x -= rect.left;
      if (pt.x < thumb_pos)
         return SCROLL_TOP_RECT;
      if (pt.x >= thumb_pos + thumb_size)
         return SCROLL_BOTTOM_RECT;
   }
   return SCROLL_THUMB;
}

// The position for a thumb at pos pixels from the bar's start.
int ThumbValue(HWND hwnd, int bar, const RECT &rect, bool vertical, int pos)
{
   Info *info = GetInfo(hwnd, bar, false);
   if (info == nullptr)
      return 0;
   int cxv = GetSystemMetrics(SM_CXVSCROLL);
   int pixels = (vertical ? rect.bottom - rect.top : rect.right - rect.left) - 2 * cxv;
   int value = info->min;
   if (pixels > 0)
   {
      int thumb_size = info->page ? std::max(MulDiv(pixels, info->page, info->max - info->min + 1), kMinThumb) : cxv;
      if ((pixels -= thumb_size) > 0)
      {
         pos = std::min(std::max(0, pos - cxv), pixels);
         int range = info->page ? info->max - info->min - info->page + 1 : info->max - info->min;
         value = info->min + MulDiv(pos, range, pixels);
      }
   }
   return value;
}

POINT ClipToRect(const RECT &rect, POINT pt)
{
   pt.x = std::clamp(pt.x, rect.left, rect.right);
   pt.y = std::clamp(pt.y, rect.top, rect.bottom);
   return pt;
}

void UpdateTimer(HWND hwnd, HWND owner, HWND ctl, HitTest hit, UINT msg, UINT code, bool vertical)
{
   if (hit == tracking.hit_test)
   {
      if (!timer_running || msg == WM_LBUTTONDOWN || msg == WM_SYSTIMER)
      {
         SendMessageW(owner, vertical ? WM_VSCROLL : WM_HSCROLL, code, (LPARAM) ctl);
         timer_running = true;
         SetSystemTimer(hwnd, kTimer, msg == WM_LBUTTONDOWN ? kFirstDelay : kRepeatDelay, nullptr);
      }
   }
   else
   {
      timer_running = false;
      KillSystemTimer(hwnd, kTimer);
   }
}

// A mouse or timer event of a scroll bar being tracked; pt in client (SB_CTL) or
// window coordinates.
void HandleEvent(HWND hwnd, int bar, UINT msg, POINT pt)
{
   static POINT prev_pt;
   static int track_thumb_pos, last_click_pos, last_mouse_pos;

   if (GetInfo(hwnd, bar, false) == nullptr)
      return;
   if (tracking.hit_test == SCROLL_NOWHERE && msg != WM_LBUTTONDOWN && msg != WM_MOUSEMOVE &&
       msg != WM_MOUSELEAVE && msg != WM_NCMOUSEMOVE && msg != WM_NCMOUSELEAVE)
      return;
   if (bar == SB_CTL && (Style(hwnd) & (SBS_SIZEGRIP | SBS_SIZEBOX)))
   {
      switch (msg)
      {
      case WM_LBUTTONDOWN:
         HideCaret(hwnd);
         SetCapture(hwnd);
         prev_pt = pt;
         tracking.hit_test = SCROLL_THUMB;
         break;
      case WM_MOUSEMOVE:
         prev_pt = pt;
         break;
      case WM_LBUTTONUP:
         ReleaseCapture();
         tracking.hit_test = SCROLL_NOWHERE;
         if (hwnd == GetFocus())
            ShowCaret(hwnd);
         break;
      }
      return;
   }

   HDC hdc = GetDCEx(hwnd, nullptr, DCX_CACHE | (bar == SB_CTL ? 0 : DCX_WINDOW));
   RECT rect;
   int arrow, thumb_size, thumb_pos;
   bool vertical = BarRect(hwnd, bar, &rect, &arrow, &thumb_size, &thumb_pos);
   HWND owner = bar == SB_CTL ? GetParent(hwnd) : hwnd;
   HWND ctl = bar == SB_CTL ? hwnd : nullptr;
   HitTest hit = SCROLL_NOWHERE;

   switch (msg)
   {
   case WM_LBUTTONDOWN:
      HideCaret(hwnd);
      tracking.vertical = vertical;
      tracking.hit_test = hit = Hit(hwnd, bar, pt, false);
      last_click_pos = vertical ? pt.y - rect.top : pt.x - rect.left;
      last_mouse_pos = last_click_pos;
      track_thumb_pos = thumb_pos;
      prev_pt = pt;
      if (bar == SB_CTL && (Style(hwnd) & WS_TABSTOP))
         SetFocus(hwnd);
      SetCapture(hwnd);
      break;
   case WM_MOUSEMOVE:
   case WM_NCMOUSEMOVE:
      hit = Hit(hwnd, bar, pt, vertical == tracking.vertical && GetCapture() == hwnd);
      prev_pt = pt;
      break;
   case WM_NCMOUSELEAVE:
      if (bar == SB_CTL)
      {
         ReleaseDC(hwnd, hdc);
         return;
      }
      hit = SCROLL_NOWHERE;
      break;
   case WM_MOUSELEAVE:
      if (bar != SB_CTL)
      {
         ReleaseDC(hwnd, hdc);
         return;
      }
      hit = SCROLL_NOWHERE;
      break;
   case WM_LBUTTONUP:
      hit = SCROLL_NOWHERE;
      ReleaseCapture();
      if (hwnd == GetFocus())
         ShowCaret(hwnd);
      break;
   case WM_SYSTIMER:
      pt = prev_pt;
      hit = Hit(hwnd, bar, pt, false);
      break;
   default:
      ReleaseDC(hwnd, hdc);
      return;
   }

   switch (tracking.hit_test)
   {
   case SCROLL_NOWHERE:
      break;
   case SCROLL_TOP_ARROW:
      DrawBar(hwnd, hdc, bar, hit, true, false);
      UpdateTimer(hwnd, owner, ctl, hit, msg, SB_LINEUP, vertical);
      break;
   case SCROLL_TOP_RECT:
      DrawBar(hwnd, hdc, bar, hit, false, true);
      UpdateTimer(hwnd, owner, ctl, hit, msg, SB_PAGEUP, vertical);
      break;
   case SCROLL_THUMB:
      if (msg == WM_LBUTTONDOWN)
      {
         tracking.win = hwnd;
         tracking.bar = bar;
         tracking.thumb_pos = track_thumb_pos + last_mouse_pos - last_click_pos;
         tracking.thumb_val = ThumbValue(hwnd, bar, rect, vertical, tracking.thumb_pos);
         if (!moving_thumb)
         {
            moving_thumb = true;
            DrawBar(hwnd, hdc, bar, hit, false, true);
         }
      }
      else if (msg == WM_LBUTTONUP)
         DrawBar(hwnd, hdc, bar, SCROLL_NOWHERE, false, true);
      else if (msg == WM_MOUSEMOVE)
      {
         int pos;
         if (!PointInScrollRect(rect, pt, vertical))
            pos = last_click_pos;
         else
         {
            pt = ClipToRect(rect, pt);
            pos = vertical ? pt.y - rect.top : pt.x - rect.left;
         }
         if (pos != last_mouse_pos || !moving_thumb)
         {
            last_mouse_pos = pos;
            tracking.thumb_pos = track_thumb_pos + pos - last_click_pos;
            tracking.thumb_val = ThumbValue(hwnd, bar, rect, vertical, tracking.thumb_pos);
            SendMessageW(owner, vertical ? WM_VSCROLL : WM_HSCROLL, MAKEWPARAM(SB_THUMBTRACK, tracking.thumb_val),
                         (LPARAM) ctl);
            moving_thumb = true;
            DrawBar(hwnd, hdc, bar, hit, false, true);
         }
      }
      break;
   case SCROLL_BOTTOM_RECT:
      DrawBar(hwnd, hdc, bar, hit, false, true);
      UpdateTimer(hwnd, owner, ctl, hit, msg, SB_PAGEDOWN, vertical);
      break;
   case SCROLL_BOTTOM_ARROW:
      DrawBar(hwnd, hdc, bar, hit, true, false);
      UpdateTimer(hwnd, owner, ctl, hit, msg, SB_LINEDOWN, vertical);
      break;
   }

   if (msg == WM_LBUTTONDOWN && hit == SCROLL_THUMB)
   {
      int val = ThumbValue(hwnd, bar, rect, vertical, track_thumb_pos + last_mouse_pos - last_click_pos);
      SendMessageW(owner, vertical ? WM_VSCROLL : WM_HSCROLL, MAKEWPARAM(SB_THUMBTRACK, val), (LPARAM) ctl);
   }
   if (msg == WM_LBUTTONUP)
   {
      HitTest was = tracking.hit_test;
      tracking.hit_test = SCROLL_NOWHERE;
      if (was == SCROLL_THUMB)
      {
         int val = ThumbValue(hwnd, bar, rect, vertical, track_thumb_pos + last_mouse_pos - last_click_pos);
         SendMessageW(owner, vertical ? WM_VSCROLL : WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, val), (LPARAM) ctl);
      }
      SendMessageW(owner, vertical ? WM_VSCROLL : WM_HSCROLL, SB_ENDSCROLL, (LPARAM) ctl);
      tracking.win = nullptr;
      moving_thumb = false;
      timer_running = false;
      KillSystemTimer(hwnd, kTimer);
      if (IsWindow(hwnd))
         DrawBar(hwnd, hdc, bar, SCROLL_NOWHERE, true, true);
   }
   ReleaseDC(hwnd, hdc);
}

int SetInfo(HWND hwnd, int bar, const SCROLLINFO *si, bool redraw)
{
   if (si == nullptr || (si->fMask & ~(SIF_ALL | SIF_DISABLENOSCROLL | SIF_RETURNPREV)) ||
       (si->cbSize != sizeof(*si) && si->cbSize != sizeof(*si) - sizeof(si->nTrackPos)))
      return 0;
   Info *info = GetInfo(hwnd, bar, true);
   if (info == nullptr)
      return 0;
   enum
   {
      HIDE = 1,
      SHOW = 2,
      REFRESH = 4,
      REPAINT_ARROWS = 8
   };
   int action = 0, ret = 0;
   if (si->fMask & SIF_RETURNPREV)
      ret = info->cur;
   if ((si->fMask & SIF_PAGE) && info->page != (INT) si->nPage)
   {
      info->page = (INT) si->nPage;
      action |= REFRESH;
   }
   if ((si->fMask & SIF_POS) && info->cur != si->nPos)
   {
      info->cur = si->nPos;
      action |= REFRESH;
   }
   if (si->fMask & SIF_RANGE)
   {
      if (si->nMin > si->nMax || (UINT) (si->nMax - si->nMin) >= 0x80000000)
      {
         action |= REFRESH;
         info->min = info->max = 0;
      }
      else if (info->min != si->nMin || info->max != si->nMax)
      {
         action |= REFRESH;
         info->min = si->nMin;
         info->max = si->nMax;
      }
   }
   if (info->page < 0)
      info->page = 0;
   else if (info->page > info->max - info->min + 1)
      info->page = info->max - info->min + 1;
   if (info->cur < info->min)
      info->cur = info->min;
   else if (info->cur > info->max - std::max(info->page - 1, 0))
      info->cur = info->max - std::max(info->page - 1, 0);

   if (si->fMask & SIF_ALL)
   {
      if (si->fMask & (SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL))
      {
         UINT new_flags = info->flags;
         if (info->min >= info->max - std::max(info->page - 1, 0))
         {
            if (si->fMask & SIF_DISABLENOSCROLL)
            {
               new_flags = ESB_DISABLE_BOTH;
               action |= REFRESH;
            }
            else if (bar != SB_CTL && (action & REFRESH))
               action = HIDE;
         }
         else if (si->fMask != SIF_PAGE)
         {
            new_flags = ESB_ENABLE_BOTH;
            if (bar != SB_CTL && (action & REFRESH))
               action |= SHOW;
         }
         if (bar == SB_CTL && redraw && IsWindowVisible(hwnd) &&
             (new_flags == ESB_ENABLE_BOTH || new_flags == ESB_DISABLE_BOTH))
         {
            EnableWindow(hwnd, new_flags == ESB_ENABLE_BOTH);
            if ((info = GetInfo(hwnd, bar, false)) == nullptr)
               return 0;
         }
         if (info->flags != new_flags)
         {
            info->flags = new_flags;
            action |= REPAINT_ARROWS;
         }
      }
   }
   if (!(si->fMask & SIF_RETURNPREV))
      ret = info->cur;
   if (action & HIDE)
      ShowBar(hwnd, bar, false, false);
   else
   {
      if ((action & SHOW) && ShowBar(hwnd, bar, true, true))
         return ret;   // the frame change repaints
      if (redraw)
         Refresh(hwnd, bar, true, true);
      else if (action & REPAINT_ARROWS)
         Refresh(hwnd, bar, true, false);
   }
   return ret;
}

BOOL GetInfoOf(HWND hwnd, int bar, SCROLLINFO *si)
{
   if (si == nullptr || (si->fMask & ~(SIF_ALL | SIF_DISABLENOSCROLL | SIF_RETURNPREV)) ||
       (si->cbSize != sizeof(*si) && si->cbSize != sizeof(*si) - sizeof(si->nTrackPos)))
      return FALSE;
   Info *info = GetInfo(hwnd, bar, false);
   if (info == nullptr)
      return FALSE;
   if (si->fMask & SIF_PAGE)
      si->nPage = (UINT) info->page;
   if (si->fMask & SIF_POS)
      si->nPos = info->cur;
   if ((si->fMask & SIF_TRACKPOS) && si->cbSize == sizeof(*si))
      si->nTrackPos = tracking.win == hwnd ? tracking.thumb_val : info->cur;
   if (si->fMask & SIF_RANGE)
   {
      si->nMin = info->min;
      si->nMax = info->max;
   }
   return (si->fMask & SIF_ALL) != 0;
}

BOOL BarInfo(HWND hwnd, LONG id, SCROLLBARINFO *sbi)
{
   int bar;
   switch (id)
   {
   case OBJID_CLIENT: bar = SB_CTL; break;
   case OBJID_HSCROLL: bar = SB_HORZ; break;
   case OBJID_VSCROLL: bar = SB_VERT; break;
   default: return FALSE;
   }
   if (sbi == nullptr || sbi->cbSize != sizeof(*sbi))
      return FALSE;
   int arrow;
   BarRect(hwnd, bar, &sbi->rcScrollBar, &arrow, &sbi->dxyLineButton, &sbi->xyThumbTop);
   RECT wr;
   GetWindowRect(hwnd, &wr);
   if (bar == SB_CTL)
      MapWindowPoints(hwnd, nullptr, (POINT *) &sbi->rcScrollBar, 2);
   else
      OffsetRect(&sbi->rcScrollBar, wr.left, wr.top);
   sbi->xyThumbBottom = sbi->xyThumbTop + sbi->dxyLineButton;
   Info *info = GetInfo(hwnd, bar, true);
   if (info == nullptr)
      return FALSE;
   DWORD style = Style(hwnd);
   memset(sbi->rgstate, 0, sizeof(sbi->rgstate));
   if ((bar == SB_HORZ && !(style & WS_HSCROLL)) || (bar == SB_VERT && !(style & WS_VSCROLL)))
      sbi->rgstate[0] |= STATE_SYSTEM_INVISIBLE;
   if (info->min >= info->max - std::max(info->page - 1, 0))
      sbi->rgstate[0] |= (sbi->rgstate[0] & STATE_SYSTEM_INVISIBLE) ? STATE_SYSTEM_OFFSCREEN : STATE_SYSTEM_UNAVAILABLE;
   if (bar == SB_CTL && !IsWindowEnabled(hwnd))
      sbi->rgstate[0] |= STATE_SYSTEM_UNAVAILABLE;
   bool pressed = (bar == SB_VERT) == tracking.vertical && GetCapture() == hwnd;
   if (pressed && tracking.hit_test == SCROLL_TOP_ARROW)
      sbi->rgstate[1] |= STATE_SYSTEM_PRESSED;
   if (info->flags & ESB_DISABLE_LTUP)
      sbi->rgstate[1] |= STATE_SYSTEM_UNAVAILABLE;
   if (info->cur == info->min)
      sbi->rgstate[2] |= STATE_SYSTEM_INVISIBLE;
   if (pressed && tracking.hit_test == SCROLL_TOP_RECT)
      sbi->rgstate[2] |= STATE_SYSTEM_PRESSED;
   if (pressed && tracking.hit_test == SCROLL_THUMB)
      sbi->rgstate[3] |= STATE_SYSTEM_PRESSED;
   if (info->cur >= info->max - 1)
      sbi->rgstate[4] |= STATE_SYSTEM_INVISIBLE;
   if (pressed && tracking.hit_test == SCROLL_BOTTOM_RECT)
      sbi->rgstate[4] |= STATE_SYSTEM_PRESSED;
   if (pressed && tracking.hit_test == SCROLL_BOTTOM_ARROW)
      sbi->rgstate[5] |= STATE_SYSTEM_PRESSED;
   if (info->flags & ESB_DISABLE_RTDN)
      sbi->rgstate[5] |= STATE_SYSTEM_UNAVAILABLE;
   return TRUE;
}

void CreateControl(HWND hwnd, const CREATESTRUCTW *cs)
{
   Info &info = Controls()[hwnd];
   if (cs->style & WS_DISABLED)
      info.flags = ESB_DISABLE_BOTH;
   int cxv = GetSystemMetrics(SM_CXVSCROLL), cyh = GetSystemMetrics(SM_CYHSCROLL);
   if (cs->style & (SBS_SIZEGRIP | SBS_SIZEBOX))
   {
      if (cs->style & SBS_SIZEBOXTOPLEFTALIGN)
         MoveWindow(hwnd, cs->x, cs->y, cxv + 1, cyh + 1, FALSE);
      else if (cs->style & SBS_SIZEBOXBOTTOMRIGHTALIGN)
         MoveWindow(hwnd, cs->x + cs->cx - cxv - 1, cs->y + cs->cy - cyh - 1, cxv + 1, cyh + 1, FALSE);
   }
   else if (cs->style & SBS_VERT)
   {
      if (cs->style & SBS_LEFTALIGN)
         MoveWindow(hwnd, cs->x, cs->y, cxv + 1, cs->cy, FALSE);
      else if (cs->style & SBS_RIGHTALIGN)
         MoveWindow(hwnd, cs->x + cs->cx - cxv - 1, cs->y, cxv + 1, cs->cy, FALSE);
   }
   else
   {
      if (cs->style & SBS_TOPALIGN)
         MoveWindow(hwnd, cs->x, cs->y, cs->cx, cyh + 1, FALSE);
      else if (cs->style & SBS_BOTTOMALIGN)
         MoveWindow(hwnd, cs->x, cs->y + cs->cy - cyh - 1, cs->cx, cyh + 1, FALSE);
   }
}

void KeyDown(HWND hwnd, WPARAM key)
{
   WPARAM code;
   switch (key)
   {
   case VK_PRIOR: code = SB_PAGEUP; break;
   case VK_NEXT: code = SB_PAGEDOWN; break;
   case VK_HOME: code = SB_TOP; break;
   case VK_END: code = SB_BOTTOM; break;
   case VK_UP: code = SB_LINEUP; break;
   case VK_DOWN: code = SB_LINEDOWN; break;
   case VK_LEFT: code = SB_LINEUP; break;
   case VK_RIGHT: code = SB_LINEDOWN; break;
   default: return;
   }
   SendMessageW(GetParent(hwnd), (Style(hwnd) & SBS_VERT) ? WM_VSCROLL : WM_HSCROLL, code, (LPARAM) hwnd);
}

LRESULT CALLBACK ScrollBarProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (!IsWindow(hwnd))
      return 0;
   switch (msg)
   {
   case WM_CREATE:
      CreateControl(hwnd, (const CREATESTRUCTW *) lparam);
      return 0;
   case WM_NCDESTROY:
      Controls().erase(hwnd);
      break;
   case WM_KEYDOWN:
      HideCaret(hwnd);
      KeyDown(hwnd, wparam);
      return 0;
   case WM_KEYUP:
      ShowCaret(hwnd);
      return 0;
   case WM_LBUTTONDBLCLK:
   case WM_LBUTTONDOWN:
      if (Style(hwnd) & SBS_SIZEGRIP)
         SendMessageW(GetParent(hwnd), WM_SYSCOMMAND, SC_SIZE + WMSZ_BOTTOMRIGHT, lparam);
      else
         TrackScrollBar(hwnd, SB_CTL, {(SHORT) LOWORD(lparam), (SHORT) HIWORD(lparam)});
      return 0;
   case WM_LBUTTONUP:
   case WM_NCMOUSEMOVE:
   case WM_NCMOUSELEAVE:
   case WM_MOUSEMOVE:
   case WM_MOUSELEAVE:
   case WM_SYSTIMER:
      HandleEvent(hwnd, SB_CTL, msg, {(SHORT) LOWORD(lparam), (SHORT) HIWORD(lparam)});
      return 0;
   case WM_ENABLE:
      if (Info *info = GetInfo(hwnd, SB_CTL, false))
      {
         info->flags = wparam ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH;
         Refresh(hwnd, SB_CTL, true, true);
      }
      return 0;
   case WM_SETFOCUS:
   {
      // A caret over the thumb shows the focus.
      RECT rect;
      int arrow, thumb_size, thumb_pos;
      bool vertical = BarRect(hwnd, SB_CTL, &rect, &arrow, &thumb_size, &thumb_pos);
      if (!vertical)
      {
         CreateCaret(hwnd, (HBITMAP) 1, thumb_size - 2, rect.bottom - rect.top - 2);
         SetCaretPos(thumb_pos + 1, rect.top + 1);
      }
      else
      {
         CreateCaret(hwnd, (HBITMAP) 1, rect.right - rect.left - 2, thumb_size - 2);
         SetCaretPos(rect.top + 1, thumb_pos + 1);
      }
      ShowCaret(hwnd);
      return 0;
   }
   case WM_KILLFOCUS:
   {
      RECT rect;
      int arrow, thumb_size, thumb_pos;
      bool vertical = BarRect(hwnd, SB_CTL, &rect, &arrow, &thumb_size, &thumb_pos);
      if (!vertical)
         rect.left = thumb_pos + 1, rect.right = rect.left + thumb_size;
      else
         rect.top = thumb_pos + 1, rect.bottom = rect.top + thumb_size;
      HideCaret(hwnd);
      InvalidateRect(hwnd, &rect, FALSE);
      DestroyCaret();
      return 0;
   }
   case WM_ERASEBKGND:
      return 1;
   case WM_GETDLGCODE:
      return DLGC_WANTARROWS;
   case WM_PAINT:
   case WM_PRINTCLIENT:
   {
      PAINTSTRUCT ps;
      HDC hdc = wparam ? (HDC) wparam : BeginPaint(hwnd, &ps);
      DrawBar(hwnd, hdc, SB_CTL, tracking.hit_test, true, true);
      if (!wparam)
         EndPaint(hwnd, &ps);
      return 0;
   }
   case SBM_SETRANGEREDRAW:
   case SBM_SETRANGE:
   {
      int old_pos = GetScrollPos(hwnd, SB_CTL);
      if (Info *info = GetInfo(hwnd, SB_CTL, false))
      {
         info->min = (INT) wparam;
         info->max = (INT) lparam;
      }
      if (msg == SBM_SETRANGEREDRAW)
         Refresh(hwnd, SB_CTL, true, true);
      if (old_pos != GetScrollPos(hwnd, SB_CTL))
         return old_pos;
      return 0;
   }
   case SBM_GETSCROLLINFO:
      return GetInfoOf(hwnd, SB_CTL, (SCROLLINFO *) lparam);
   case SBM_GETSCROLLBARINFO:
      return BarInfo(hwnd, OBJID_CLIENT, (SCROLLBARINFO *) lparam);
   case SBM_SETSCROLLINFO:
      return SetInfo(hwnd, SB_CTL, (const SCROLLINFO *) lparam, wparam != 0);
   case WM_SETCURSOR:
      if (Style(hwnd) & SBS_SIZEGRIP)
      {
         SetCursor(LoadCursorA(nullptr, (LPCSTR) IDC_SIZENWSE));
         return TRUE;
      }
      break;
   case SBM_SETPOS:
   {
      SCROLLINFO si = {sizeof(si), SIF_POS | SIF_RETURNPREV};
      si.nPos = (INT) wparam;
      return SetInfo(hwnd, SB_CTL, &si, lparam != 0);
   }
   case SBM_GETPOS:
   {
      Info *info = GetInfo(hwnd, SB_CTL, false);
      return info ? info->cur : 0;
   }
   case SBM_GETRANGE:
   {
      Info *info = GetInfo(hwnd, SB_CTL, false);
      if (info == nullptr)
         return FALSE;
      if (wparam)
         *(INT *) wparam = info->min;
      if (lparam)
         *(INT *) lparam = info->max;
      return TRUE;
   }
   case SBM_ENABLE_ARROWS:
      return EnableScrollBar(hwnd, SB_CTL, (UINT) wparam);
   }
   return DefWindowProcW(hwnd, msg, wparam, lparam);
}

}  // namespace

void RegisterScrollBarClass()
{
   WNDCLASSW wc = {};
   wc.style = CS_GLOBALCLASS | CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW | CS_PARENTDC;
   wc.lpfnWndProc = ScrollBarProc;
   wc.hCursor = LoadCursorA(nullptr, (LPCSTR) IDC_ARROW);
   wc.lpszClassName = u"ScrollBar";
   RegisterClassW(&wc);
}

void DrawStandardScrollBars(HWND hwnd, HDC hdc)
{
   DWORD style = Style(hwnd);
   if (style & WS_HSCROLL)
      DrawBar(hwnd, hdc, SB_HORZ, tracking.hit_test, true, true);
   if (style & WS_VSCROLL)
      DrawBar(hwnd, hdc, SB_VERT, tracking.hit_test, true, true);
   // The corner between both bars.
   Window *w = GetWindowObject(hwnd);
   if (w && (style & WS_HSCROLL) && (style & WS_VSCROLL))
   {
      RECT r = {w->client.right, w->client.bottom, w->client.right + GetSystemMetrics(SM_CXVSCROLL),
                w->client.bottom + GetSystemMetrics(SM_CYHSCROLL)};
      FillRect(hdc, &r, GetSysColorBrush(COLOR_BTNFACE));
   }
}

LRESULT ScrollBarHitTest(HWND hwnd, POINT window_pt)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return HTNOWHERE;
   DWORD style = w->style;
   RECT r;
   int arrow, size, pos;
   if (style & WS_VSCROLL)
   {
      BarRect(hwnd, SB_VERT, &r, &arrow, &size, &pos);
      if (style & WS_HSCROLL)
         r.bottom--;
      if (PtInRect(&r, window_pt))
         return HTVSCROLL;
   }
   if (style & WS_HSCROLL)
   {
      BarRect(hwnd, SB_HORZ, &r, &arrow, &size, &pos);
      if (style & WS_VSCROLL)
         r.right--;
      if (PtInRect(&r, window_pt))
         return HTHSCROLL;
   }
   if ((style & WS_VSCROLL) && (style & WS_HSCROLL))
   {
      RECT corner = {w->client.right, w->client.bottom, w->client.right + GetSystemMetrics(SM_CXVSCROLL),
                     w->client.bottom + GetSystemMetrics(SM_CYHSCROLL)};
      if (PtInRect(&corner, window_pt))
         return (style & WS_THICKFRAME) ? HTSIZE : HTNOWHERE;
   }
   return HTNOWHERE;
}

void TrackScrollBar(HWND hwnd, int bar, POINT pt)
{
   // pt: client coordinates for SB_CTL, screen coordinates for the standard bars;
   // tracking works in the bar's drawing coordinates.
   POINT offset = {0, 0};
   if (bar != SB_CTL)
   {
      Window *w = GetWindowObject(hwnd);
      if (w == nullptr)
         return;
      ScreenToClient(hwnd, &pt);
      offset = {-w->client.left, -w->client.top};   // the window's top-left in client coordinates
      pt.x -= offset.x;
      pt.y -= offset.y;
   }
   HandleEvent(hwnd, bar, WM_LBUTTONDOWN, pt);
   MSG msg;
   do
   {
      if (!GetMessageW(&msg, nullptr, 0, 0))
      {
         PostQuitMessage((INT) msg.wParam);
         break;
      }
      if (msg.hwnd == hwnd && (msg.message == WM_LBUTTONUP || msg.message == WM_MOUSEMOVE ||
                               msg.message == WM_MOUSELEAVE || msg.message == WM_NCMOUSEMOVE ||
                               msg.message == WM_NCMOUSELEAVE || (msg.message == WM_SYSTIMER && msg.wParam == kTimer)))
      {
         POINT p = {(SHORT) LOWORD(msg.lParam) - offset.x, (SHORT) HIWORD(msg.lParam) - offset.y};
         if (msg.message == WM_NCMOUSEMOVE)
         {
            // Non-client messages have screen coordinates.
            p = {(SHORT) LOWORD(msg.lParam), (SHORT) HIWORD(msg.lParam)};
            ScreenToClient(hwnd, &p);
            p.x -= offset.x;
            p.y -= offset.y;
         }
         HandleEvent(hwnd, bar, msg.message, p);
      }
      else
      {
         TranslateMessage(&msg);
         DispatchMessageW(&msg);
      }
      if (!IsWindow(hwnd))
      {
         ReleaseCapture();
         break;
      }
   } while (msg.message != WM_LBUTTONUP && GetCapture() == hwnd);
   // Tracking ended otherwise (capture taken away): stop as a button release would.
   if (IsWindow(hwnd) && tracking.hit_test != SCROLL_NOWHERE)
   {
      tracking.hit_test = SCROLL_NOWHERE;
      tracking.win = nullptr;
      moving_thumb = false;
      timer_running = false;
      KillSystemTimer(hwnd, kTimer);
      Refresh(hwnd, bar, true, true);
   }
}

}  // namespace w32

using namespace w32;

INT WINAPI SetScrollInfo(HWND hwnd, INT bar, const SCROLLINFO *si, BOOL redraw)
{
   if (bar == SB_CTL)
      return (INT) SendMessageW(hwnd, SBM_SETSCROLLINFO, redraw, (LPARAM) si);
   return SetInfo(hwnd, bar, si, redraw != FALSE);
}

BOOL WINAPI GetScrollInfo(HWND hwnd, INT bar, SCROLLINFO *si)
{
   if (bar == SB_CTL)
   {
      SendMessageW(hwnd, SBM_GETSCROLLINFO, 0, (LPARAM) si);
      return TRUE;
   }
   return GetInfoOf(hwnd, bar, si);
}

INT WINAPI SetScrollPos(HWND hwnd, INT bar, INT pos, BOOL redraw)
{
   SCROLLINFO si = {sizeof(si), SIF_POS | SIF_RETURNPREV};
   si.nPos = pos;
   return SetScrollInfo(hwnd, bar, &si, redraw);
}

INT WINAPI GetScrollPos(HWND hwnd, INT bar)
{
   if (bar == SB_CTL)
      return (INT) SendMessageW(hwnd, SBM_GETPOS, 0, 0);
   SCROLLINFO si = {sizeof(si), SIF_POS};
   return GetScrollInfo(hwnd, bar, &si) ? si.nPos : 0;
}

BOOL WINAPI SetScrollRange(HWND hwnd, INT bar, INT min, INT max, BOOL redraw)
{
   SCROLLINFO si = {sizeof(si), SIF_RANGE};
   si.nMin = min;
   si.nMax = max;
   SetScrollInfo(hwnd, bar, &si, redraw);
   return TRUE;
}

BOOL WINAPI GetScrollRange(HWND hwnd, INT bar, INT *min, INT *max)
{
   if (bar == SB_CTL)
   {
      SendMessageW(hwnd, SBM_GETRANGE, (WPARAM) min, (LPARAM) max);
      return TRUE;
   }
   SCROLLINFO si = {sizeof(si), SIF_RANGE};
   GetScrollInfo(hwnd, bar, &si);
   if (min)
      *min = si.nMin;
   if (max)
      *max = si.nMax;
   return TRUE;
}

BOOL WINAPI ShowScrollBar(HWND hwnd, INT bar, BOOL show)
{
   if (hwnd == nullptr)
      return FALSE;
   ShowBar(hwnd, bar, bar == SB_VERT ? false : show != FALSE, bar == SB_HORZ ? false : show != FALSE);
   return TRUE;
}

BOOL WINAPI EnableScrollBar(HWND hwnd, UINT bar, UINT flags)
{
   flags &= ESB_DISABLE_BOTH;
   bool check;
   if (bar == SB_BOTH)
   {
      Info *v = GetInfo(hwnd, SB_VERT, true);
      if (v == nullptr)
         return FALSE;
      check = v->flags == flags;
      v->flags = flags;
      if (!check)
         Refresh(hwnd, SB_VERT, true, true);
      bar = SB_HORZ;
   }
   else
      check = bar != SB_CTL;
   Info *info = GetInfo(hwnd, (int) bar, true);
   if (info == nullptr)
      return FALSE;
   if (check)
      check = info->flags == flags;
   info->flags = flags;
   if (check)
      return FALSE;
   if (bar == SB_CTL && (flags == ESB_DISABLE_BOTH || flags == ESB_ENABLE_BOTH))
      EnableWindow(hwnd, flags == ESB_ENABLE_BOTH);
   Refresh(hwnd, (int) bar, true, true);
   return TRUE;
}

BOOL WINAPI GetScrollBarInfo(HWND hwnd, LONG id, SCROLLBARINFO *sbi)
{
   if (id == OBJID_CLIENT)
      return (BOOL) SendMessageW(hwnd, SBM_GETSCROLLBARINFO, 0, (LPARAM) sbi);
   return BarInfo(hwnd, id, sbi);
}
