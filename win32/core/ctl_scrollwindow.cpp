/*
 * ctl_scrollwindow.cpp:  ScrollWindowEx, ScrollWindow and ScrollDC: the pixels of
 *   a rectangle move on the surface; what they uncover (and what couldn't move:
 *   covered or outside the window's visible part) is returned and, for windows,
 *   invalidated, and a pending update region moves with the contents.
 */

#include "core/caret.h"
#include "core/gdi.h"
#include "core/window.h"

#include <cstring>
#include <vector>

namespace w32 {

namespace {

int RegionType(const Region &r)
{
   return r.Empty() ? NULLREGION : r.Rects().size() == 1 ? SIMPLEREGION : COMPLEXREGION;
}

// Moves the pixels of dc's target that land in region dst (target coordinates)
// from (x - dx, y - dy); dst must come from pixels inside the target.
void MovePixels(DC *dc, const RasterTarget &t, const Region &dst, int dx, int dy)
{
   if (dst.Empty() || t.bpp < 8)
      return;
   RECT b = dst.Bounds();
   int bytes = t.bpp / 8;
   int w = b.right - b.left, h = b.bottom - b.top;
   std::vector<uint8_t> copy((size_t) w * h * bytes);
   // The source rows first (they overlap the destination).
   for (int y = 0; y < h; y++)
      memcpy(&copy[(size_t) y * w * bytes], t.Row(b.top + y - dy) + (size_t) (b.left - dx) * bytes, (size_t) w * bytes);
   for (const RECT &r : dst.Rects())
      for (int y = r.top; y < r.bottom; y++)
         memcpy(t.Row(y) + (size_t) r.left * bytes, &copy[((size_t) (y - b.top) * w + (r.left - b.left)) * bytes],
                (size_t) (r.right - r.left) * bytes);
   ShowTargetChanges(dc, b);
}

// Scrolls on hdc: src (logical) by (dx, dy), only into clip (logical, or all); the
// uncovered part (logical) into *exposed.
void ScrollOnDC(HDC hdc, int dx, int dy, const RECT &src, const RECT *clip, Region *exposed)
{
   exposed->Clear();
   DC *dc = GetDCObject(hdc);
   RasterTarget t;
   RECT area = src;
   if (clip)
      IntersectRect(&area, &area, clip);
   Region want(area);
   if (dc == nullptr || !GetTarget(dc, &t))
   {
      *exposed = want;
      return;
   }
   int ox = dc->origin.x, oy = dc->origin.y;
   // What may be drawn, and what pixels are there to take, in target coordinates.
   Region visible = EffectiveClip(dc, t);
   RECT s = src;
   OffsetRect(&s, ox, oy);
   Region from = visible;
   from.Intersect(s);
   from.Offset(dx, dy);      // where the visible source pixels go
   Region dst = from;
   RECT a = area;
   OffsetRect(&a, ox, oy);
   dst.Intersect(a);
   dst.Intersect(visible);
   MovePixels(dc, t, dst, dx, dy);
   dst.Offset(-ox, -oy);
   *exposed = want;
   exposed->Subtract(dst);
}

}  // namespace

}  // namespace w32

using namespace w32;

BOOL WINAPI ScrollDC(HDC hdc, INT dx, INT dy, const RECT *scroll, const RECT *clip, HRGN update, RECT *update_rect)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return FALSE;
   RECT src;
   if (scroll)
      src = *scroll;
   else
   {
      RasterTarget t;
      if (!GetTarget(dc, &t))
         return FALSE;
      src = {-dc->origin.x, -dc->origin.y, t.width - dc->origin.x, t.height - dc->origin.y};
   }
   Region exposed;
   ScrollOnDC(hdc, dx, dy, src, clip, &exposed);
   if (Region *r = GetRegion(update))
      *r = exposed;
   if (update_rect)
      *update_rect = exposed.Bounds();
   return TRUE;
}

INT WINAPI ScrollWindowEx(HWND hwnd, INT dx, INT dy, const RECT *scroll, const RECT *clip, HRGN update,
                          RECT *update_rect, UINT flags)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
   {
      SetLastError(ERROR_INVALID_WINDOW_HANDLE);
      return ERROR;
   }
   RECT client;
   GetClientRect(hwnd, &client);
   RECT src = client;
   if (scroll)
      IntersectRect(&src, scroll, &client);
   RECT cl = client;
   if (clip)
      IntersectRect(&cl, clip, &client);

   Region exposed;
   if (IsWindowVisible(hwnd) && (dx || dy))
   {
      bool caret = CaretHideIn(hwnd, src);
      HDC hdc = GetDCEx(hwnd, nullptr, DCX_CACHE | DCX_USESTYLE);
      ScrollOnDC(hdc, dx, dy, src, &cl, &exposed);
      ReleaseDC(hwnd, hdc);
      // The pending update region moves with the contents.
      if (w->update)
         if (Region *u = GetRegion(w->update))
         {
            Region moved = *u;
            moved.Intersect(src);
            u->Subtract(src);
            moved.Offset(dx, dy);
            moved.Intersect(cl);
            u->Add(moved);
            exposed.Add(moved);
         }
      if (caret)
         CaretRestore();
   }
   else
   {
      RECT a;
      IntersectRect(&a, &src, &cl);
      exposed = Region(a);
   }

   if (flags & SW_SCROLLCHILDREN)
   {
      std::vector<HWND> children = w->children;
      for (HWND c : children)
      {
         Window *cw = GetWindowObject(c);
         RECT i;
         if (cw == nullptr || (scroll && !IntersectRect(&i, &cw->rect, &src)))
            continue;
         SetWindowPos(c, nullptr, cw->rect.left + dx, cw->rect.top + dy, 0, 0,
                      SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);
         RedrawWindow(c, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
      }
   }

   if (flags & (SW_INVALIDATE | SW_ERASE))
   {
      HRGN r = CreateRectRgn(0, 0, 0, 0);
      *GetRegion(r) = exposed;
      RedrawWindow(hwnd, nullptr, r,
                   RDW_INVALIDATE | ((flags & SW_ERASE) ? RDW_ERASE : 0) | ((flags & SW_SCROLLCHILDREN) ? RDW_ALLCHILDREN : 0));
      DeleteObject(r);
   }
   if (Region *r = GetRegion(update))
      *r = exposed;
   if (update_rect)
      *update_rect = exposed.Bounds();
   return RegionType(exposed);
}

BOOL WINAPI ScrollWindow(HWND hwnd, INT dx, INT dy, const RECT *rect, const RECT *clip)
{
   return ScrollWindowEx(hwnd, dx, dy, rect, clip, nullptr, nullptr,
                         SW_INVALIDATE | SW_ERASE | (rect ? 0 : SW_SCROLLCHILDREN)) != ERROR;
}
