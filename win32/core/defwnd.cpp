/*
 * defwnd.cpp:  DefWindowProc, and the non-client drawing of child windows (their
 *   borders and 3D edges; top-level frames are the toolkit's).
 */

#include "core/gdi.h"
#include "core/menu.h"
#include "core/message.h"
#include "core/native.h"
#include "core/scroll.h"
#include "core/window.h"
#include "core/winproc.h"

#include <cstring>
#include <vector>

namespace w32 {

HBRUSH Brush55AA();

namespace {

void FillSolid(HDC hdc, const RECT &r, COLORREF color)
{
   HBRUSH b = CreateSolidBrush(color);
   FillRect(hdc, &r, b);
   DeleteObject(b);
}

}  // namespace

// Draws a 3D edge (DrawEdge's EDGE_* and BF_RECT) inside r, shrinking it.
void Draw3DEdge(HDC hdc, RECT *r, UINT edge)
{
   COLORREF light = GetSysColor(COLOR_3DLIGHT), hilight = GetSysColor(COLOR_BTNHIGHLIGHT),
            shadow = GetSysColor(COLOR_BTNSHADOW), dark = GetSysColor(COLOR_3DDKSHADOW);
   auto ring = [&](COLORREF tl, COLORREF br) {
      FillSolid(hdc, {r->left, r->top, r->right, r->top + 1}, tl);
      FillSolid(hdc, {r->left, r->top, r->left + 1, r->bottom}, tl);
      FillSolid(hdc, {r->left, r->bottom - 1, r->right, r->bottom}, br);
      FillSolid(hdc, {r->right - 1, r->top, r->right, r->bottom}, br);
      InflateRect(r, -1, -1);
   };
   if (edge & BDR_RAISEDOUTER)
      ring(light, dark);
   else if (edge & BDR_SUNKENOUTER)
      ring(shadow, hilight);
   if (edge & BDR_RAISEDINNER)
      ring(hilight, shadow);
   else if (edge & BDR_SUNKENINNER)
      ring(dark, light);
}

}  // namespace w32

using namespace w32;

// DrawEdge: core/ctl_uitools.cpp.

// The non-client area of ours (child windows', frameless popups') on hdc, in
// window coordinates: borders, 3D edges, scroll bars.
static void DrawNonClient(HWND hwnd, HDC hdc)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return;
   RECT r = {0, 0, w->rect.right - w->rect.left, w->rect.bottom - w->rect.top};
   if (w->exstyle & WS_EX_DLGMODALFRAME || (w->style & WS_CAPTION) == WS_DLGFRAME)
      Draw3DEdge(hdc, &r, EDGE_RAISED), Draw3DEdge(hdc, &r, BDR_RAISEDINNER);
   else if (w->style & WS_BORDER)
   {
      HBRUSH b = CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME));
      FrameRect(hdc, &r, b);
      DeleteObject(b);
      InflateRect(&r, -1, -1);
   }
   if (w->exstyle & WS_EX_CLIENTEDGE)
      Draw3DEdge(hdc, &r, EDGE_SUNKEN);
   if (w->exstyle & WS_EX_STATICEDGE)
      Draw3DEdge(hdc, &r, BDR_SUNKENOUTER);
   DrawStandardScrollBars(hwnd, hdc);
}

static void PaintNonClient(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return;
   if (!IsChildWindow(w) && w->client.left == 0 && w->client.right == w->rect.right - w->rect.left)
   {
      PaintMenuBar(w);   // a toolkit-framed window: only the menu bar is ours
      return;
   }
   HDC hdc = GetWindowDC(hwnd);
   DrawNonClient(hwnd, hdc);
   ReleaseDC(hwnd, hdc);
}

// Moves hdc's device origin by (dx, dy) and limits it to the rectangle of the
// given size there: the coordinates a window's own DC would have.
static void EnterRect(HDC hdc, int dx, int dy, int width, int height)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return;
   dc->origin.x += dx;
   dc->origin.y += dy;
   POINT device = DeviceOrigin(dc);
   RECT r = {device.x, device.y, device.x + width, device.y + height};
   if (dc->has_clip)
      dc->clip.Intersect(r);
   else
   {
      dc->clip = Region(r);
      dc->has_clip = true;
   }
}

// WM_PRINT: the window (and with PRF_CHILDREN its children) drawn into hdc, its
// top-left at hdc's (0, 0), as DefWindowProc does on Windows.
static void Print(HWND hwnd, HDC hdc, LPARAM flags)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || ((flags & PRF_CHECKVISIBLE) && !IsWindowVisible(hwnd)))
      return;
   int saved = SaveDC(hdc);
   if (flags & PRF_NONCLIENT)
      DrawNonClient(hwnd, hdc);
   EnterRect(hdc, w->client.left, w->client.top, w->client.right - w->client.left, w->client.bottom - w->client.top);
   if (flags & PRF_ERASEBKGND)
      SendMessageA(hwnd, WM_ERASEBKGND, (WPARAM) hdc, 0);
   if (flags & PRF_CLIENT)
      SendMessageA(hwnd, WM_PRINTCLIENT, (WPARAM) hdc, flags);
   if (flags & PRF_CHILDREN)
   {
      // In painting's order, so the picture is what painting shows: from the
      // top of the Z order down, lower siblings over higher ones where they
      // overlap without WS_CLIPSIBLINGS.
      std::vector<HWND> children(w->children.begin(), w->children.end());
      for (HWND c : children)
      {
         Window *cw = GetWindowObject(c);
         if (cw == nullptr || !(cw->style & WS_VISIBLE))
            continue;
         int inner = SaveDC(hdc);
         EnterRect(hdc, cw->rect.left, cw->rect.top, cw->rect.right - cw->rect.left, cw->rect.bottom - cw->rect.top);
         SendMessageA(c, WM_PRINT, (WPARAM) hdc, flags | PRF_NONCLIENT | PRF_CLIENT);
         RestoreDC(hdc, inner);
      }
   }
   RestoreDC(hdc, saved);
}

LRESULT WINAPI DefWindowProcA(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return 0;
   switch (msg)
   {
   case WM_NCCREATE:
   {
      auto *cs = (CREATESTRUCTA *) lparam;
      if (cs && cs->lpszName && !IS_INTRESOURCE(cs->lpszName))
         w->text = cs->lpszName;
      return TRUE;
   }

   case WM_NCCALCSIZE:
      return 0;

   case WM_SETREDRAW:
      // As Windows: the WS_VISIBLE bit goes, without hiding the window, so it
      // isn't painted or invalidated until redrawing is back on.
      if (wparam)
         w->style |= WS_VISIBLE;
      else
         w->style &= ~WS_VISIBLE;
      w->visible = wparam != 0;
      return 0;

   case WM_SETICON:
   {
      // The window's icon (ICON_BIG) is the toolkit's window or application icon.
      HICON old = (HICON) GetPropA(hwnd, wparam == ICON_SMALL ? "m59.icon.small" : "m59.icon");
      SetPropA(hwnd, wparam == ICON_SMALL ? "m59.icon.small" : "m59.icon", (HANDLE) lparam);
      if (wparam != ICON_SMALL && w->top && lparam)
         native::SetIcon(w->top, (HICON) lparam);
      return (LRESULT) old;
   }
   case WM_GETICON:
      return (LRESULT) GetPropA(hwnd, wparam == ICON_SMALL ? "m59.icon.small" : "m59.icon");

   case WM_NOTIFYFORMAT:
      // Controls ask their parent which notifications it takes.
      if (lparam == NF_QUERY)
         return w->unicode ? NFR_UNICODE : NFR_ANSI;
      return 0;

   case WM_SETTEXT:
      w->text = lparam ? (const char *) lparam : "";
      if (w->top)
         native::SetTitle(w->top, w->text);
      return TRUE;

   case WM_GETTEXT:
   {
      if (wparam == 0 || lparam == 0)
         return 0;
      lstrcpynA((LPSTR) lparam, w->text.c_str(), (INT) wparam);
      return (LRESULT) strlen((LPSTR) lparam);
   }

   case WM_GETTEXTLENGTH:
      return (LRESULT) w->text.size();

   case WM_NCPAINT:
      PaintNonClient(hwnd);
      return 0;

   case WM_PAINT:
   {
      PAINTSTRUCT ps;
      BeginPaint(hwnd, &ps);
      EndPaint(hwnd, &ps);
      return 0;
   }

   case WM_ERASEBKGND:
   {
      HBRUSH b = w->cls->background;
      if (b == nullptr)
         return 0;
      RECT r;
      GetClientRect(hwnd, &r);
      FillRect((HDC) wparam, &r, b);
      return 1;
   }

   case WM_CTLCOLORSCROLLBAR:
   {
      // As Windows: the scroll bar's shaft in 3D face on highlight, dithered when
      // the highlight is the window color.
      HDC hdc = (HDC) wparam;
      COLORREF bk = GetSysColor(COLOR_3DHILIGHT);
      SetTextColor(hdc, GetSysColor(COLOR_3DFACE));
      SetBkColor(hdc, bk);
      if (bk == GetSysColor(COLOR_WINDOW))
         return (LRESULT) Brush55AA();
      return (LRESULT) GetSysColorBrush(COLOR_SCROLLBAR);
   }

   case WM_CTLCOLORMSGBOX:
   case WM_CTLCOLOREDIT:
   case WM_CTLCOLORLISTBOX:
   case WM_CTLCOLORBTN:
   case WM_CTLCOLORDLG:
   case WM_CTLCOLORSTATIC:
   {
      HDC hdc = (HDC) wparam;
      bool window_like = msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORLISTBOX;
      SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
      SetBkColor(hdc, GetSysColor(window_like ? COLOR_WINDOW : COLOR_BTNFACE));
      return (LRESULT) GetSysColorBrush(window_like ? COLOR_WINDOW : COLOR_BTNFACE);
   }

   case WM_SETCURSOR:
   {
      // The parent may set it first (Windows asks parents, outermost first ... here
      // just the parent chain, as DefWindowProc forwards).
      if (IsChildWindow(w) && w->parent && SendMessageA(w->parent, WM_SETCURSOR, wparam, lparam))
         return TRUE;
      if (LOWORD(lparam) == HTCLIENT && w->cls->cursor)
      {
         SetCursor(w->cls->cursor);
         return TRUE;
      }
      if ((SHORT) LOWORD(lparam) > HTCLIENT)
      {
         // The non-client area's parts (scroll bars, borders) show the arrow.
         SetCursor(LoadCursorA(nullptr, (LPCSTR) IDC_ARROW));
         return TRUE;
      }
      return FALSE;
   }

   case WM_NCHITTEST:
   {
      POINT p = {(SHORT) LOWORD(lparam), (SHORT) HIWORD(lparam)};
      RECT wr;
      GetWindowRect(hwnd, &wr);
      if (!PtInRect(&wr, p))
         return HTNOWHERE;
      p.x -= wr.left;
      p.y -= wr.top;
      if (PtInRect(&w->client, p))
         return HTCLIENT;
      if (LRESULT hit = ScrollBarHitTest(hwnd, p); hit != HTNOWHERE)
         return hit;
      if (!IsChildWindow(w) && p.y < w->client.top)
         return HTMENU;
      return (w->style & (WS_BORDER | WS_DLGFRAME | WS_THICKFRAME)) || (w->exstyle & (WS_EX_CLIENTEDGE | WS_EX_STATICEDGE))
                ? HTBORDER : HTNOWHERE;
   }

   case WM_NCLBUTTONDOWN:
   case WM_NCLBUTTONDBLCLK:
      if (wparam == HTVSCROLL)
         SendMessageA(hwnd, WM_SYSCOMMAND, SC_VSCROLL + HTVSCROLL, lparam);
      else if (wparam == HTHSCROLL)
         SendMessageA(hwnd, WM_SYSCOMMAND, SC_HSCROLL + HTHSCROLL, lparam);
      return 0;

   case WM_MOUSEACTIVATE:
      if (IsChildWindow(w) && w->parent)
         if (LRESULT r = SendMessageA(w->parent, WM_MOUSEACTIVATE, wparam, lparam))
            return r;
      return MA_ACTIVATE;

   case WM_MOUSEWHEEL:
      // Unhandled wheel turns go to the parent.
      if (IsChildWindow(w) && w->parent)
         return SendMessageA(w->parent, msg, wparam, lparam);
      return 0;

   case WM_WINDOWPOSCHANGED:
   {
      auto *pos = (WINDOWPOS *) lparam;
      if (!(pos->flags & SWP_NOMOVE))
         SendMessageA(hwnd, WM_MOVE, 0, MAKELPARAM(w->rect.left + w->client.left, w->rect.top + w->client.top));
      if (!(pos->flags & SWP_NOSIZE))
         SendMessageA(hwnd, WM_SIZE, w->maximized ? SIZE_MAXIMIZED : w->minimized ? SIZE_MINIMIZED : SIZE_RESTORED,
                      MAKELPARAM(w->client.right - w->client.left, w->client.bottom - w->client.top));
      return 0;
   }

   case WM_CLOSE:
      DestroyWindow(hwnd);
      return 0;

   case WM_SYSCOMMAND:
      switch (wparam & 0xFFF0)
      {
      case SC_CLOSE: SendMessageA(hwnd, WM_CLOSE, 0, 0); break;
      case SC_MINIMIZE: ShowWindow(hwnd, SW_MINIMIZE); break;
      case SC_MAXIMIZE: ShowWindow(hwnd, SW_MAXIMIZE); break;
      case SC_RESTORE: ShowWindow(hwnd, SW_RESTORE); break;
      case SC_VSCROLL:
      case SC_HSCROLL:
         TrackScrollBar(hwnd, (wparam & 0xFFF0) == SC_VSCROLL ? SB_VERT : SB_HORZ,
                        {(SHORT) LOWORD(lparam), (SHORT) HIWORD(lparam)});
         break;
      }
      return 0;

   case WM_SYSKEYDOWN:
      // Alt+F4 closes the top-level window.
      if (wparam == VK_F4 && (lparam & (1 << 29)))
         SendMessageA(TopLevelOf(hwnd), WM_SYSCOMMAND, SC_CLOSE, 0);
      return 0;

   case WM_KEYDOWN:
   case WM_KEYUP:
   case WM_SYSKEYUP:
   case WM_CHAR:
   case WM_SYSCHAR:
      return 0;

   case WM_QUERYENDSESSION:
   case WM_QUERYOPEN:
      return TRUE;

   case WM_SHOWWINDOW:
      return 0;

   case WM_GETFONT:
      return 0;

   case WM_GETDLGCODE:
      return 0;

   case WM_PRINTCLIENT:
      return 0;

   case WM_PRINT:
      Print(hwnd, (HDC) wparam, lparam);
      return 0;
   }
   return 0;
}

LRESULT WINAPI DefWindowProcW(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   return CallProcRef({DefWindowProcA, false}, hwnd, msg, wparam, lparam, true);
}
