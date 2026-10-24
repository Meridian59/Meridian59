/*
 * cc_shared.cpp:  Functions comctl32's controls need that belong to other areas
 *   (Unicode GDI and text, scroll bars, uxtheme, shlwapi, locale, the window
 *   manager).  All weak: an owner's full version replaces each at link time.
 */

#include "core/gdi.h"
#include "core/module.h"
#include "core/native.h"
#include "core/window.h"
#include "core/winproc.h"

#include <uxtheme.h>
#include <shlwapi.h>

#include <cstdarg>
#include <cstring>
#include <map>

using namespace w32;

#define CC_WEAK __attribute__((weak))

extern "C" int m59_vswprintf(unsigned short *buffer, size_t count, const unsigned short *format, va_list args);

/************************************************************************/
// Unicode text and fonts, through the ANSI functions (text in code page 1252)

extern "C" {

CC_WEAK BOOL WINAPI ExtTextOutW(HDC hdc, INT x, INT y, UINT options, const RECT *rect, LPCWSTR text, UINT count,
                                const INT *dx)
{
   // Characters map one to one to the code page's, so dx stays valid.
   std::string a = text ? WideToAnsi(text, (int) count) : std::string();
   return ExtTextOutA(hdc, x, y, options & ~ETO_GLYPH_INDEX, rect, a.c_str(), (UINT) a.size(), dx);
}

CC_WEAK BOOL WINAPI GetTextExtentPoint32W(HDC hdc, LPCWSTR text, INT count, SIZE *size)
{
   std::string a = text ? WideToAnsi(text, count) : std::string();
   return GetTextExtentPoint32A(hdc, a.c_str(), (INT) a.size(), size);
}

CC_WEAK BOOL WINAPI GetTextExtentPointW(HDC hdc, LPCWSTR text, INT count, SIZE *size)
{
   return GetTextExtentPoint32W(hdc, text, count, size);
}

CC_WEAK HFONT WINAPI CreateFontIndirectW(const LOGFONTW *lf)
{
   if (lf == nullptr)
      return nullptr;
   LOGFONTA a;
   memcpy(&a, lf, offsetof(LOGFONTA, lfFaceName));
   lstrcpynA(a.lfFaceName, WideToAnsi(lf->lfFaceName).c_str(), LF_FACESIZE);
   return CreateFontIndirectA(&a);
}

CC_WEAK INT WINAPI lstrcmpW(LPCWSTR a, LPCWSTR b)
{
   if (a == nullptr || b == nullptr)
      return a == b ? 0 : a ? 1 : -1;
   return CompareStringW(LOCALE_USER_DEFAULT, 0, a, -1, b, -1) - CSTR_EQUAL;
}

CC_WEAK INT WINAPI lstrcmpiW(LPCWSTR a, LPCWSTR b)
{
   if (a == nullptr || b == nullptr)
      return a == b ? 0 : a ? 1 : -1;
   return CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, a, -1, b, -1) - CSTR_EQUAL;
}

CC_WEAK INT WINAPI wvsprintfW(LPWSTR buffer, LPCWSTR format, va_list args)
{
   // wsprintf's buffer is 1024 characters at most.
   return m59_vswprintf((unsigned short *) buffer, 1024, (const unsigned short *) format, args);
}

CC_WEAK INT WINAPIV wsprintfW(LPWSTR buffer, LPCWSTR format, ...)
{
   va_list args;
   va_start(args, format);
   INT r = wvsprintfW(buffer, format, args);
   va_end(args);
   return r;
}

namespace {

void LogFontAtoW(const LOGFONTA &a, LOGFONTW *w)
{
   memcpy(w, &a, offsetof(LOGFONTA, lfFaceName));
   memset(w->lfFaceName, 0, sizeof(w->lfFaceName));
   std::basic_string<WCHAR> face = AnsiToWide(a.lfFaceName);
   memcpy(w->lfFaceName, face.data(), std::min(face.size(), (size_t) LF_FACESIZE - 1) * sizeof(WCHAR));
}

}  // namespace

CC_WEAK BOOL WINAPI SystemParametersInfoW(UINT action, UINT param, PVOID out, UINT flags)
{
   switch (action)
   {
   case SPI_GETNONCLIENTMETRICS:
   {
      auto *w = (NONCLIENTMETRICSW *) out;
      if (w == nullptr)
         return FALSE;
      NONCLIENTMETRICSA a = {};
      a.cbSize = sizeof(a);
      if (!SystemParametersInfoA(action, sizeof(a), &a, flags))
         return FALSE;
      UINT size = w->cbSize;
      memset(w, 0, size >= sizeof(*w) ? sizeof(*w) : size);
      w->cbSize = size;
      w->iBorderWidth = a.iBorderWidth;
      w->iScrollWidth = a.iScrollWidth;
      w->iScrollHeight = a.iScrollHeight;
      w->iCaptionWidth = a.iCaptionWidth;
      w->iCaptionHeight = a.iCaptionHeight;
      w->iSmCaptionWidth = a.iSmCaptionWidth;
      w->iSmCaptionHeight = a.iSmCaptionHeight;
      w->iMenuWidth = a.iMenuWidth;
      w->iMenuHeight = a.iMenuHeight;
      LogFontAtoW(a.lfCaptionFont, &w->lfCaptionFont);
      LogFontAtoW(a.lfSmCaptionFont, &w->lfSmCaptionFont);
      LogFontAtoW(a.lfMenuFont, &w->lfMenuFont);
      LogFontAtoW(a.lfStatusFont, &w->lfStatusFont);
      LogFontAtoW(a.lfMessageFont, &w->lfMessageFont);
      return TRUE;
   }
   case SPI_GETICONTITLELOGFONT:
   {
      if (out == nullptr)
         return FALSE;
      NONCLIENTMETRICSA a = {};
      a.cbSize = sizeof(a);
      SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(a), &a, 0);
      LogFontAtoW(a.lfMessageFont, (LOGFONTW *) out);
      return TRUE;
   }
   case SPI_GETFLATMENU:
   case SPI_GETHOTTRACKING:
   case SPI_GETKEYBOARDCUES:
   case SPI_GETTOOLTIPANIMATION:
   case SPI_GETTOOLTIPFADE:
   case SPI_GETLISTBOXSMOOTHSCROLLING:
   case SPI_GETGRADIENTCAPTIONS:
      if (out)
         *(BOOL *) out = FALSE;
      return TRUE;
   case SPI_GETFONTSMOOTHING:
      if (out)
         *(BOOL *) out = TRUE;
      return TRUE;
   case SPI_GETMOUSEHOVERWIDTH:
   case SPI_GETMOUSEHOVERHEIGHT:
      if (out)
         *(UINT *) out = 4;
      return TRUE;
   case SPI_GETMOUSEHOVERTIME:
      if (out)
         *(UINT *) out = 400;
      return TRUE;
   }
   return SystemParametersInfoA(action, param, out, flags);
}

CC_WEAK HCURSOR WINAPI LoadCursorW(HINSTANCE instance, LPCWSTR name)
{
   return LoadCursorA(instance, AnsiArg(name));
}

CC_WEAK HICON WINAPI LoadIconW(HINSTANCE instance, LPCWSTR name)
{
   return LoadIconA(instance, AnsiArg(name));
}

CC_WEAK HBITMAP WINAPI LoadBitmapW(HINSTANCE instance, LPCWSTR name)
{
   return LoadBitmapA(instance, AnsiArg(name));
}

CC_WEAK HMODULE WINAPI LoadLibraryW(LPCWSTR name)
{
   return name ? LoadLibraryA(WideToAnsi(name).c_str()) : nullptr;
}

CC_WEAK HMODULE WINAPI LoadLibraryExW(LPCWSTR name, HANDLE file, DWORD flags)
{
   return name ? LoadLibraryExA(WideToAnsi(name).c_str(), file, flags) : nullptr;
}

CC_WEAK HMODULE WINAPI GetModuleHandleW(LPCWSTR name)
{
   return GetModuleHandleA(name ? WideToAnsi(name).c_str() : nullptr);
}

CC_WEAK BOOL WINAPI GetMonitorInfoW(HMONITOR monitor, MONITORINFO *info)
{
   if (info == nullptr)
      return FALSE;
   MONITORINFOEXA a = {};
   a.cbSize = sizeof(a);
   if (!GetMonitorInfoA(monitor, (MONITORINFO *) &a))
      return FALSE;
   info->rcMonitor = a.rcMonitor;
   info->rcWork = a.rcWork;
   info->dwFlags = a.dwFlags;
   if (info->cbSize >= sizeof(MONITORINFOEXW))
   {
      std::basic_string<WCHAR> device = AnsiToWide(a.szDevice);
      memcpy(((MONITORINFOEXW *) info)->szDevice, device.c_str(), (device.size() + 1) * sizeof(WCHAR));
   }
   return TRUE;
}

CC_WEAK UINT WINAPI GetDpiForWindow(HWND hwnd)
{
   return GetWindowObject(hwnd) ? (UINT) native::ScreenDpi() : 0;
}

/************************************************************************/
// Messages

CC_WEAK DWORD WINAPI GetMessagePos(void)
{
   // Where the pointer was for the message being handled: input messages come
   // as the pointer moves, so its last known position.
   POINT p = native::GetCursorPos();
   return MAKELONG((SHORT) p.x, (SHORT) p.y);
}

CC_WEAK LONG WINAPI GetMessageTime(void)
{
   return (LONG) GetTickCount();
}

CC_WEAK void WINAPI NotifyWinEvent(DWORD, HWND, LONG, LONG)
{
   // No accessibility clients.
}

/************************************************************************/
// Window manager

CC_WEAK HWND WINAPI GetTopWindow(HWND hwnd)
{
   if (hwnd == nullptr)
      return GetWindow(GetDesktopWindow(), GW_CHILD);
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return nullptr;
   return w->children.empty() ? nullptr : w->children.front();
}

/************************************************************************/
// Scroll bars of a window (WS_VSCROLL / WS_HSCROLL): their state.  Drawing and
// mouse input on them belong to the scroll bar code.

namespace {

struct ScrollState
{
   SCROLLINFO bar[2] = {};
};

std::map<HWND, ScrollState> &Scrolls()
{
   static std::map<HWND, ScrollState> scrolls;
   return scrolls;
}

SCROLLINFO *BarInfo(HWND hwnd, INT bar)
{
   if ((bar != SB_HORZ && bar != SB_VERT) || GetWindowObject(hwnd) == nullptr)
      return nullptr;
   SCROLLINFO *si = &Scrolls()[hwnd].bar[bar == SB_VERT];
   si->cbSize = sizeof(*si);
   return si;
}

void ShowBar(HWND hwnd, INT bar, bool show)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return;
   DWORD flags = 0;
   if (bar == SB_HORZ || bar == SB_BOTH)
      flags |= WS_HSCROLL;
   if (bar == SB_VERT || bar == SB_BOTH)
      flags |= WS_VSCROLL;
   DWORD style = show ? (w->style | flags) : (w->style & ~flags);
   if (style == w->style)
      return;
   w->style = style;
   SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

}  // namespace

CC_WEAK INT WINAPI SetScrollInfo(HWND hwnd, INT bar, const SCROLLINFO *info, BOOL redraw)
{
   (void) redraw;
   if (info == nullptr)
      return 0;
   if (bar == SB_CTL)
      return (INT) SendMessageW(hwnd, SBM_SETSCROLLINFO, redraw, (LPARAM) info);
   SCROLLINFO *si = BarInfo(hwnd, bar);
   if (si == nullptr)
      return 0;
   if (info->fMask & SIF_RANGE)
   {
      si->nMin = info->nMin;
      si->nMax = info->nMax;
   }
   if (info->fMask & SIF_PAGE)
      si->nPage = info->nPage;
   if (info->fMask & SIF_POS)
      si->nPos = info->nPos;
   // The page fits in the range, the position within the range less the page.
   if (si->nMax < si->nMin)
      si->nMax = si->nMin;
   if (si->nPage > (UINT) (si->nMax - si->nMin + 1))
      si->nPage = si->nMax - si->nMin + 1;
   int top = si->nMax - std::max<int>((int) si->nPage - 1, 0);
   si->nPos = std::max(si->nMin, std::min(si->nPos, top));
   si->nTrackPos = si->nPos;
   if (info->fMask & (SIF_RANGE | SIF_PAGE))
   {
      // Shown while there is something to scroll, unless asked to stay.
      bool scrollable = si->nMax - si->nMin >= (int) std::max<UINT>(si->nPage, 1) && si->nPage < (UINT) (si->nMax - si->nMin + 1);
      if (scrollable || (info->fMask & SIF_DISABLENOSCROLL))
         ShowBar(hwnd, bar, true);
      else
         ShowBar(hwnd, bar, false);
   }
   return si->nPos;
}

CC_WEAK BOOL WINAPI GetScrollInfo(HWND hwnd, INT bar, SCROLLINFO *info)
{
   if (info == nullptr)
      return FALSE;
   if (bar == SB_CTL)
      return (BOOL) SendMessageW(hwnd, SBM_GETSCROLLINFO, 0, (LPARAM) info);
   SCROLLINFO *si = BarInfo(hwnd, bar);
   if (si == nullptr)
      return FALSE;
   if (info->fMask & SIF_RANGE)
   {
      info->nMin = si->nMin;
      info->nMax = si->nMax;
   }
   if (info->fMask & SIF_PAGE)
      info->nPage = si->nPage;
   if (info->fMask & SIF_POS)
      info->nPos = si->nPos;
   if (info->fMask & SIF_TRACKPOS)
      info->nTrackPos = si->nTrackPos;
   return (info->fMask & SIF_ALL) != 0;
}

CC_WEAK INT WINAPI GetScrollPos(HWND hwnd, INT bar)
{
   SCROLLINFO si = {sizeof(si), SIF_POS};
   return GetScrollInfo(hwnd, bar, &si) ? si.nPos : 0;
}

CC_WEAK INT WINAPI SetScrollPos(HWND hwnd, INT bar, INT pos, BOOL redraw)
{
   INT old = GetScrollPos(hwnd, bar);
   SCROLLINFO si = {sizeof(si), SIF_POS};
   si.nPos = pos;
   SetScrollInfo(hwnd, bar, &si, redraw);
   return old;
}

CC_WEAK BOOL WINAPI GetScrollRange(HWND hwnd, INT bar, INT *min, INT *max)
{
   SCROLLINFO si = {sizeof(si), SIF_RANGE};
   BOOL ok = GetScrollInfo(hwnd, bar, &si);
   if (min)
      *min = ok ? si.nMin : 0;
   if (max)
      *max = ok ? si.nMax : 0;
   return ok;
}

CC_WEAK BOOL WINAPI SetScrollRange(HWND hwnd, INT bar, INT min, INT max, BOOL redraw)
{
   SCROLLINFO si = {sizeof(si), SIF_RANGE};
   si.nMin = min;
   si.nMax = max;
   SetScrollInfo(hwnd, bar, &si, redraw);
   return TRUE;
}

CC_WEAK BOOL WINAPI ShowScrollBar(HWND hwnd, INT bar, BOOL show)
{
   if (GetWindowObject(hwnd) == nullptr)
      return FALSE;
   if (bar == SB_CTL)
   {
      ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE);
      return TRUE;
   }
   ShowBar(hwnd, bar, show != FALSE);
   return TRUE;
}

CC_WEAK BOOL WINAPI EnableScrollBar(HWND hwnd, UINT, UINT)
{
   return GetWindowObject(hwnd) != nullptr;
}

CC_WEAK INT WINAPI ScrollWindowEx(HWND hwnd, INT dx, INT dy, const RECT *scroll, const RECT *clip, HRGN update,
                                  RECT *update_rect, UINT flags)
{
   // The scrolled area is repainted instead of moved: the same pixels result.
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return ERROR;
   RECT client;
   GetClientRect(hwnd, &client);
   RECT area = scroll ? *scroll : client;
   IntersectRect(&area, &area, &client);
   if (clip)
      IntersectRect(&area, &area, clip);
   if (update)
      SetRectRgn(update, area.left, area.top, area.right, area.bottom);
   if (update_rect)
      *update_rect = area;
   if (flags & SW_SCROLLCHILDREN)
   {
      std::vector<HWND> children = w->children;
      for (HWND c : children)
      {
         Window *cw = GetWindowObject(c);
         if (cw == nullptr)
            continue;
         RECT r = cw->rect;
         if (scroll == nullptr || IntersectRect(&r, &r, scroll))
            SetWindowPos(c, nullptr, cw->rect.left + dx, cw->rect.top + dy, 0, 0,
                         SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
      }
   }
   if (flags & (SW_INVALIDATE | SW_ERASE))
      RedrawWindow(hwnd, &area, nullptr, RDW_INVALIDATE | ((flags & SW_ERASE) ? RDW_ERASE : 0) |
                                             ((flags & SW_SCROLLCHILDREN) ? RDW_ALLCHILDREN : 0));
   return IsRectEmpty(&area) ? NULLREGION : SIMPLEREGION;
}

CC_WEAK BOOL WINAPI ScrollWindow(HWND hwnd, INT dx, INT dy, const RECT *scroll, const RECT *clip)
{
   return ScrollWindowEx(hwnd, dx, dy, scroll, clip, nullptr, nullptr,
                         SW_INVALIDATE | SW_ERASE | (scroll ? 0 : SW_SCROLLCHILDREN)) != ERROR;
}

/************************************************************************/
// DrawFrameControl: the classic look's buttons and scroll arrows

namespace {

void FillSys(HDC hdc, const RECT &r, int color)
{
   FillRect(hdc, &r, GetSysColorBrush(color));
}

// A filled triangle pointing dir (0 up, 1 down, 2 left, 3 right), centered in r.
void Arrow(HDC hdc, const RECT &r, int dir, COLORREF color, int shift)
{
   int w = r.right - r.left, h = r.bottom - r.top;
   int size = std::max(1, (std::min(w, h) + 1) / 4);   // half the base
   int cx = r.left + w / 2 + shift, cy = r.top + h / 2 + shift;
   POINT p[3];
   switch (dir)
   {
   case 0: p[0] = {cx - size, cy + size / 2}; p[1] = {cx + size, cy + size / 2}; p[2] = {cx, cy + size / 2 - size}; break;
   case 1: p[0] = {cx - size, cy - size / 2}; p[1] = {cx + size, cy - size / 2}; p[2] = {cx, cy - size / 2 + size}; break;
   case 2: p[0] = {cx + size / 2, cy - size}; p[1] = {cx + size / 2, cy + size}; p[2] = {cx + size / 2 - size, cy}; break;
   default: p[0] = {cx - size / 2, cy - size}; p[1] = {cx - size / 2, cy + size}; p[2] = {cx - size / 2 + size, cy}; break;
   }
   HBRUSH brush = CreateSolidBrush(color);
   HPEN pen = CreatePen(PS_SOLID, 1, color);
   HGDIOBJ ob = SelectObject(hdc, brush), op = SelectObject(hdc, pen);
   Polygon(hdc, p, 3);
   SelectObject(hdc, ob);
   SelectObject(hdc, op);
   DeleteObject(brush);
   DeleteObject(pen);
}

void ButtonFrame(HDC hdc, RECT *r, UINT state)
{
   if (state & DFCS_FLAT)
   {
      FrameRect(hdc, r, GetSysColorBrush(COLOR_WINDOWFRAME));
      InflateRect(r, -1, -1);
   }
   else if (state & DFCS_PUSHED)
      DrawEdge(hdc, r, (state & DFCS_MONO) ? BDR_SUNKENOUTER : EDGE_SUNKEN, BF_RECT | BF_ADJUST);
   else
      DrawEdge(hdc, r, (state & DFCS_MONO) ? BDR_RAISEDOUTER : EDGE_RAISED, BF_RECT | BF_ADJUST);
   if (!(state & DFCS_TRANSPARENT))
      FillSys(hdc, *r, COLOR_BTNFACE);
}

}  // namespace

CC_WEAK BOOL WINAPI DrawFrameControl(HDC hdc, RECT *rect, UINT type, UINT state)
{
   if (hdc == nullptr || rect == nullptr)
      return FALSE;
   RECT r = *rect;
   switch (type)
   {
   case DFC_SCROLL:
   {
      int dir;
      switch (state & 0xFF)
      {
      case DFCS_SCROLLUP: dir = 0; break;
      case DFCS_SCROLLDOWN:
      case DFCS_SCROLLCOMBOBOX: dir = 1; break;
      case DFCS_SCROLLLEFT: dir = 2; break;
      case DFCS_SCROLLRIGHT: dir = 3; break;
      default: return FALSE;
      }
      ButtonFrame(hdc, &r, state);
      int shift = (state & DFCS_PUSHED) && !(state & DFCS_FLAT) ? 1 : 0;
      if (state & DFCS_INACTIVE)
      {
         Arrow(hdc, r, dir, GetSysColor(COLOR_BTNHIGHLIGHT), 1);
         Arrow(hdc, r, dir, GetSysColor(COLOR_BTNSHADOW), 0);
      }
      else
         Arrow(hdc, r, dir, GetSysColor(COLOR_BTNTEXT), shift);
      return TRUE;
   }
   case DFC_BUTTON:
   {
      UINT kind = state & 0x1F;
      if (kind == DFCS_BUTTONPUSH)
      {
         ButtonFrame(hdc, &r, state);
         return TRUE;
      }
      bool radio = kind == DFCS_BUTTONRADIO || kind == DFCS_BUTTONRADIOIMAGE || kind == DFCS_BUTTONRADIOMASK;
      // A square box (radio buttons: a circle) of the rect's smaller side.
      int size = std::min(r.right - r.left, r.bottom - r.top);
      RECT box = {r.left, r.top + (r.bottom - r.top - size) / 2, r.left + size, r.top + (r.bottom - r.top - size) / 2 + size};
      int bg = (state & (DFCS_PUSHED | DFCS_INACTIVE)) ? COLOR_BTNFACE : COLOR_WINDOW;
      if (radio)
      {
         HBRUSH b = GetSysColorBrush(bg);
         HPEN p = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
         HGDIOBJ ob = SelectObject(hdc, b), op = SelectObject(hdc, p);
         Ellipse(hdc, box.left, box.top, box.right, box.bottom);
         SelectObject(hdc, op);
         DeleteObject(p);
         if (state & DFCS_CHECKED)
         {
            HBRUSH dot = CreateSolidBrush(GetSysColor((state & DFCS_INACTIVE) ? COLOR_BTNSHADOW : COLOR_WINDOWTEXT));
            HPEN np = (HPEN) GetStockObject(NULL_PEN);
            SelectObject(hdc, dot);
            op = SelectObject(hdc, np);
            int d = size / 4;
            Ellipse(hdc, box.left + d, box.top + d, box.right - d, box.bottom - d);
            SelectObject(hdc, op);
            DeleteObject(dot);
         }
         SelectObject(hdc, ob);
         return TRUE;
      }
      if (state & DFCS_FLAT)
      {
         FrameRect(hdc, &box, GetSysColorBrush(COLOR_BTNSHADOW));
         InflateRect(&box, -1, -1);
      }
      else
         DrawEdge(hdc, &box, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
      FillSys(hdc, box, bg);
      if (state & DFCS_CHECKED)
      {
         // The classic check mark: a 7x7 tick, three pixels thick.
         COLORREF c = GetSysColor((state & DFCS_INACTIVE) ? COLOR_BTNSHADOW : COLOR_WINDOWTEXT);
         int w = box.right - box.left, h = box.bottom - box.top;
         int ox = box.left + (w - 7) / 2, oy = box.top + (h - 7) / 2;
         static const int ys[7] = {2, 3, 4, 3, 2, 1, 0};
         for (int x = 0; x < 7; x++)
            for (int t = 0; t < 3; t++)
               SetPixel(hdc, ox + x, oy + ys[x] + t, c);
      }
      return TRUE;
   }
   case DFC_CAPTION:
   case DFC_MENU:
   case DFC_POPUPMENU:
      ButtonFrame(hdc, &r, state);
      return TRUE;
   }
   return FALSE;
}

/************************************************************************/
// uxtheme: no theme, so controls draw the classic way.

CC_WEAK HTHEME WINAPI OpenThemeData(HWND, LPCWSTR)
{
   return nullptr;
}

CC_WEAK HTHEME WINAPI OpenThemeDataEx(HWND, LPCWSTR, DWORD)
{
   return nullptr;
}

CC_WEAK HTHEME WINAPI OpenThemeDataForDpi(HWND, LPCWSTR, UINT)
{
   return nullptr;
}

CC_WEAK HTHEME WINAPI GetWindowTheme(HWND)
{
   return nullptr;
}

CC_WEAK HRESULT WINAPI CloseThemeData(HTHEME)
{
   return S_OK;
}

CC_WEAK BOOL WINAPI IsThemeActive(void)
{
   return FALSE;
}

CC_WEAK BOOL WINAPI IsAppThemed(void)
{
   return FALSE;
}

CC_WEAK HRESULT WINAPI SetWindowTheme(HWND, LPCWSTR, LPCWSTR)
{
   return S_OK;
}

CC_WEAK HRESULT WINAPI EnableThemeDialogTexture(HWND, DWORD)
{
   return S_OK;
}

CC_WEAK BOOL WINAPI IsThemeDialogTextureEnabled(HWND)
{
   return FALSE;
}

CC_WEAK HRESULT WINAPI DrawThemeBackground(HTHEME, HDC, int, int, const RECT *, const RECT *)
{
   return E_HANDLE;
}

CC_WEAK HRESULT WINAPI DrawThemeBackgroundEx(HTHEME, HDC, int, int, const RECT *, const DTBGOPTS *)
{
   return E_HANDLE;
}

CC_WEAK HRESULT WINAPI DrawThemeParentBackground(HWND hwnd, HDC hdc, RECT *rect)
{
   // Without themes: the parent paints the background, as uxtheme does.
   HWND parent = GetParent(hwnd);
   if (parent == nullptr)
      return E_FAIL;
   RECT r;
   GetClientRect(hwnd, &r);
   if (rect)
      IntersectRect(&r, &r, rect);
   POINT org = {0, 0};
   MapWindowPoints(hwnd, parent, &org, 1);
   SendMessageW(parent, WM_ERASEBKGND, (WPARAM) hdc, 0);
   return S_OK;
}

CC_WEAK HRESULT WINAPI DrawThemeText(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT *)
{
   return E_HANDLE;
}

CC_WEAK HRESULT WINAPI DrawThemeEdge(HTHEME, HDC, int, int, const RECT *, UINT, UINT, RECT *)
{
   return E_HANDLE;
}

CC_WEAK HRESULT WINAPI GetThemeTextExtent(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, const RECT *, RECT *)
{
   return E_HANDLE;
}

CC_WEAK HRESULT WINAPI GetThemePartSize(HTHEME, HDC, int, int, RECT *, THEMESIZE, SIZE *)
{
   return E_HANDLE;
}

CC_WEAK HRESULT WINAPI GetThemeInt(HTHEME, int, int, int, int *)
{
   return E_HANDLE;
}

CC_WEAK HRESULT WINAPI GetThemeColor(HTHEME, int, int, int, COLORREF *)
{
   return E_HANDLE;
}

CC_WEAK HRESULT WINAPI GetThemeBackgroundContentRect(HTHEME, HDC, int, int, const RECT *, RECT *)
{
   return E_HANDLE;
}

CC_WEAK HRESULT WINAPI GetThemeBackgroundExtent(HTHEME, HDC, int, int, const RECT *, RECT *)
{
   return E_HANDLE;
}

CC_WEAK HRESULT WINAPI GetThemeMargins(HTHEME, HDC, int, int, int, RECT *, MARGINS *)
{
   return E_HANDLE;
}

CC_WEAK HRESULT WINAPI GetThemeFont(HTHEME, HDC, int, int, int, LOGFONTW *)
{
   return E_HANDLE;
}

CC_WEAK BOOL WINAPI IsThemeBackgroundPartiallyTransparent(HTHEME, int, int)
{
   return FALSE;
}

CC_WEAK BOOL WINAPI IsThemePartDefined(HTHEME, int, int)
{
   return FALSE;
}

CC_WEAK COLORREF WINAPI GetThemeSysColor(HTHEME, int color)
{
   return GetSysColor(color);
}

CC_WEAK HBRUSH WINAPI GetThemeSysColorBrush(HTHEME, int color)
{
   return GetSysColorBrush(color);
}

/************************************************************************/
// shlwapi

CC_WEAK int WINAPI StrCmpIW(LPCWSTR a, LPCWSTR b)
{
   return CompareStringW(GetThreadLocale(), NORM_IGNORECASE, a, -1, b, -1) - CSTR_EQUAL;
}

CC_WEAK int WINAPI StrCmpNIW(LPCWSTR a, LPCWSTR b, int n)
{
   return CompareStringW(GetThreadLocale(), NORM_IGNORECASE, a, n, b, n) - CSTR_EQUAL;
}

CC_WEAK int WINAPI StrCmpNW(LPCWSTR a, LPCWSTR b, int n)
{
   return CompareStringW(GetThreadLocale(), 0, a, n, b, n) - CSTR_EQUAL;
}

/************************************************************************/
// Locale

CC_WEAK INT WINAPI GetLocaleInfoW(LCID lcid, LCTYPE type, LPWSTR buffer, INT size)
{
   (void) lcid;
   // The user's locale (resources' language): English, German or Portuguese.
   WORD lang = PRIMARYLANGID(UserLanguage());
   bool comma_decimal = lang == LANG_GERMAN || lang == LANG_PORTUGUESE;
   const char *value = nullptr;
   char number[16];
   switch (type & ~(LOCALE_NOUSEROVERRIDE | LOCALE_USE_CP_ACP | LOCALE_RETURN_NUMBER))
   {
   case LOCALE_SDECIMAL: value = comma_decimal ? "," : "."; break;
   case LOCALE_STHOUSAND: value = comma_decimal ? "." : ","; break;
   case LOCALE_SGROUPING: value = "3;0"; break;
   case LOCALE_IDIGITS: value = "2"; break;
   case LOCALE_ILZERO: value = "1"; break;
   case LOCALE_INEGNUMBER: value = "1"; break;
   case LOCALE_SNEGATIVESIGN: value = "-"; break;
   case LOCALE_SPOSITIVESIGN: value = ""; break;
   case LOCALE_SLIST: value = comma_decimal ? ";" : ","; break;
   case LOCALE_IMEASURE: value = lang == LANG_ENGLISH ? "1" : "0"; break;
   case LOCALE_SDATE: value = lang == LANG_GERMAN ? "." : "/"; break;
   case LOCALE_STIME: value = ":"; break;
   case LOCALE_SSHORTDATE: value = lang == LANG_GERMAN ? "dd.MM.yyyy" : lang == LANG_PORTUGUESE ? "dd/MM/yyyy" : "M/d/yyyy"; break;
   case LOCALE_SLONGDATE: value = lang == LANG_ENGLISH ? "dddd, MMMM d, yyyy" : "dddd, d. MMMM yyyy"; break;
   case LOCALE_STIMEFORMAT: value = lang == LANG_ENGLISH ? "h:mm:ss tt" : "HH:mm:ss"; break;
   case LOCALE_S1159: value = lang == LANG_ENGLISH ? "AM" : ""; break;
   case LOCALE_S2359: value = lang == LANG_ENGLISH ? "PM" : ""; break;
   case LOCALE_IFIRSTDAYOFWEEK: value = lang == LANG_ENGLISH ? "6" : "0"; break;
   case LOCALE_ILANGUAGE:
      snprintf(number, sizeof(number), "%04x", UserLanguage());
      value = number;
      break;
   default:
      SetLastError(ERROR_INVALID_FLAGS);
      return 0;
   }
   std::basic_string<WCHAR> w = AnsiToWide(value);
   INT need = (INT) w.size() + 1;
   if (size == 0)
      return need;
   if (buffer == nullptr || size < need)
   {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return 0;
   }
   memcpy(buffer, w.c_str(), need * sizeof(WCHAR));
   return need;
}

CC_WEAK INT WINAPI GetLocaleInfoA(LCID lcid, LCTYPE type, LPSTR buffer, INT size)
{
   WCHAR w[128];
   INT n = GetLocaleInfoW(lcid, type, w, 128);
   if (n == 0)
      return 0;
   std::string a = WideToAnsi(w);
   if (size == 0)
      return (INT) a.size() + 1;
   if (buffer == nullptr || size < (INT) a.size() + 1)
   {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return 0;
   }
   memcpy(buffer, a.c_str(), a.size() + 1);
   return (INT) a.size() + 1;
}

}  // extern "C"

/************************************************************************/
// Dialogs: the Unicode entry points over the dialog manager's ANSI ones

extern "C" {

CC_WEAK LRESULT WINAPI DefDlgProcW(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   return CallProcRef({DefDlgProcA, false}, hwnd, msg, wparam, lparam, true);
}

CC_WEAK BOOL WINAPI IsDialogMessageW(HWND dialog, MSG *msg)
{
   if (msg == nullptr)
      return FALSE;
   MSG a = *msg;
   if (a.message == WM_CHAR || a.message == WM_SYSCHAR || a.message == WM_DEADCHAR || a.message == WM_SYSDEADCHAR)
      a.wParam = CharWtoA(a.wParam);
   return IsDialogMessageA(dialog, &a);
}

}  // extern "C"

/************************************************************************/
// Window classes

namespace {

template <class WC> BOOL FillClassInfo(HINSTANCE instance, const char *name, WC *out, bool unicode)
{
   WindowClass *cls = FindClass(name, instance);
   if (cls == nullptr || out == nullptr)
   {
      SetLastError(ERROR_CLASS_DOES_NOT_EXIST);
      return FALSE;
   }
   out->style = cls->style;
   out->lpfnWndProc = ProcValue({cls->proc, cls->unicode}, unicode);
   out->cbClsExtra = cls->class_extra;
   out->cbWndExtra = cls->window_extra;
   out->hInstance = cls->system ? nullptr : cls->instance;
   out->hIcon = cls->icon;
   out->hCursor = cls->cursor;
   out->hbrBackground = cls->background;
   out->lpszMenuName = nullptr;
   return TRUE;
}

}  // namespace

extern "C" {

CC_WEAK BOOL WINAPI GetClassInfoExA(HINSTANCE instance, LPCSTR name, WNDCLASSEXA *wc)
{
   if (!FillClassInfo(instance, name, wc, false))
      return FALSE;
   wc->lpszClassName = name;
   wc->hIconSm = FindClass(name, instance)->icon_small;
   return TRUE;
}

CC_WEAK BOOL WINAPI GetClassInfoA(HINSTANCE instance, LPCSTR name, WNDCLASSA *wc)
{
   if (!FillClassInfo(instance, name, wc, false))
      return FALSE;
   wc->lpszClassName = name;
   return TRUE;
}

CC_WEAK BOOL WINAPI GetClassInfoW(HINSTANCE instance, LPCWSTR name, WNDCLASSW *wc)
{
   AnsiArg a(name);
   if (!FillClassInfo(instance, a, wc, true))
      return FALSE;
   wc->lpszClassName = name;
   return TRUE;
}

CC_WEAK BOOL WINAPI GetClassInfoExW(HINSTANCE instance, LPCWSTR name, WNDCLASSEXW *wc)
{
   AnsiArg a(name);
   if (!FillClassInfo(instance, a, wc, true))
      return FALSE;
   wc->lpszClassName = name;
   wc->hIconSm = FindClass(a, instance)->icon_small;
   return TRUE;
}

}  // extern "C"
