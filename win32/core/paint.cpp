/*
 * paint.cpp:  Update regions, WM_PAINT, and window device contexts.  Windows
 *   other than native controls draw on their top-level window's surface; a window
 *   DC is that surface offset to the window's client area and clipped to it.
 */

#include "core/caret.h"
#include "core/gdi.h"
#include "core/message.h"
#include "core/native.h"
#include "core/window.h"

namespace w32 {

namespace {

Region &UpdateRegion(Window *w)
{
   if (w->update == nullptr)
      w->update = CreateRectRgn(0, 0, 0, 0);
   return *GetRegion(w->update);
}

RECT ClientBounds(Window *w)
{
   return {0, 0, w->client.right - w->client.left, w->client.bottom - w->client.top};
}

void DropEmptyUpdate(Window *w)
{
   if (w->update && GetRegion(w->update)->Empty())
   {
      DeleteObject(w->update);
      w->update = nullptr;
   }
}

// A child painting before its parents takes its area out of theirs, as Wine's
// validate_parents: the parents don't then paint over it (a list view updates
// its header first, then erases its whole client area).
void ValidateParents(Window *w)
{
   if (!IsChildWindow(w))
      return;
   RECT r = w->rect;   // in the parent's client coordinates
   for (Window *p = GetWindowObject(w->parent); p; p = GetWindowObject(p->parent))
   {
      if (p->update)
      {
         GetRegion(p->update)->Subtract(r);
         DropEmptyUpdate(p);
      }
      if (!IsChildWindow(p))
         break;
      OffsetRect(&r, p->rect.left + p->client.left, p->rect.top + p->client.top);
   }
}

// Adds r (client coordinates; nullptr: all) to hwnd's update region, and to its
// children's with all_children.
void Invalidate(HWND hwnd, const Region *r, bool erase, bool all_children)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || !w->visible)
      return;
   Region add = r ? *r : Region(ClientBounds(w));
   // The part over the frame (in client coordinates, outside the client area):
   // it repaints by BeginPaint's WM_NCPAINT, even with no client area at all
   // (an etched line), as a parent's invalidation covers its children's frames.
   Region frame = add;
   RECT whole = {-w->client.left, -w->client.top, w->rect.right - w->rect.left - w->client.left,
                 w->rect.bottom - w->rect.top - w->client.top};
   if (r == nullptr)
      frame = Region(whole);
   frame.Intersect(whole);
   frame.Subtract(ClientBounds(w));
   add.Intersect(ClientBounds(w));
   if (!add.Empty())
   {
      UpdateRegion(w).Add(add);
      if (erase)
         w->erase_pending = true;
      PaintPending();
   }
   if (erase && !frame.Empty())
   {
      w->erase_pending = true;
      w->internal_paint = true;
      PaintPending();
   }
   // A parent without WS_CLIPCHILDREN paints over its children, so they repaint
   // too, as RedrawWindow does without RDW_NOCHILDREN.
   if (!all_children && (w->style & WS_CLIPCHILDREN))
      return;
   for (HWND c : w->children)
   {
      Window *cw = GetWindowObject(c);
      if (cw == nullptr || !cw->visible)
         continue;
      // The part of r over the child, in the child's client coordinates.
      Region part = r ? *r : Region(ClientBounds(w));
      part.Intersect(cw->rect);
      if (part.Empty())
         continue;
      part.Offset(-cw->rect.left - cw->client.left, -cw->rect.top - cw->client.top);
      Invalidate(c, &part, erase, true);
   }
}

// What of the top-level surface hwnd's client area may draw on: its client rect
// in top coordinates, less (WS_CLIPCHILDREN) its visible children and
// (WS_CLIPSIBLINGS, it or an ancestor) the visible siblings above, and clipped by
// each ancestor's client area.
Region VisibleRegion(HWND hwnd)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return Region();
   RECT c = ClientRectInTop(hwnd);
   Region visible(c);
   if (w->style & WS_CLIPCHILDREN)
      for (HWND child : w->children)
         if (Window *cw = GetWindowObject(child); cw && cw->visible)
         {
            RECT r = cw->rect;
            OffsetRect(&r, c.left, c.top);
            visible.Subtract(r);
         }
   for (Window *a = w; IsChildWindow(a) && a->parent;)
   {
      Window *p = GetWindowObject(a->parent);
      if (p == nullptr)
         break;
      // WS_CLIPSIBLINGS: not where visible siblings above it in the Z order are.
      if (a->style & WS_CLIPSIBLINGS)
      {
         RECT pc = ClientRectInTop(p->hwnd);
         for (HWND sibling : p->children)
         {
            if (sibling == a->hwnd)
               break;
            if (Window *sw = GetWindowObject(sibling); sw && sw->visible)
            {
               RECT r = sw->rect;
               OffsetRect(&r, pc.left, pc.top);
               visible.Subtract(r);
            }
         }
      }
      a = p;
      visible.Intersect(ClientRectInTop(a->hwnd));
   }
   return visible;
}

HDC MakeWindowDC(HWND hwnd, bool client)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return nullptr;
   auto *dc = new DC;
   dc->hwnd = hwnd;
   RECT c = ClientRectInTop(hwnd);
   if (client)
   {
      dc->origin = {c.left, c.top};
      dc->visible = VisibleRegion(hwnd);
   }
   else
   {
      // The whole window, non-client area included (child windows only have one).
      RECT r = c;
      r.left -= w->client.left;
      r.top -= w->client.top;
      r.right = r.left + (w->rect.right - w->rect.left);
      r.bottom = r.top + (w->rect.bottom - w->rect.top);
      dc->origin = {r.left, r.top};
      dc->visible = Region(r);
   }
   return (HDC) AddGdiObject(dc);
}

}  // namespace

void EraseNow(HWND hwnd)
{
   // Parents first, as their backgrounds go under their children's; siblings
   // from the top of the Z order down, as FindPaintWindow.
   std::vector<HWND> order = {hwnd};
   for (size_t i = 0; i < order.size(); i++)
      if (Window *o = GetWindowObject(order[i]))
         order.insert(order.end(), o->children.begin(), o->children.end());
   for (HWND h : order)
   {
      Window *w = GetWindowObject(h);
      if (w == nullptr || !w->visible || !w->erase_pending)
         continue;
      if (w->client.left > 0 || w->client.top > 0 || w->client.right < w->rect.right - w->rect.left ||
          w->client.bottom < w->rect.bottom - w->rect.top)
         SendMessageA(h, WM_NCPAINT, 1, 0);
      if ((w = GetWindowObject(h)) == nullptr)
         continue;
      HDC hdc = w->update ? GetDCEx(h, w->update, DCX_INTERSECTRGN) : GetDC(h);
      w->erase_pending = !SendMessageA(h, WM_ERASEBKGND, (WPARAM) hdc, 0);
      ReleaseDC(h, hdc);
      if ((w = GetWindowObject(h)) != nullptr)
         w->erase_pending = false;   // done, or the window chose to paint it itself
   }
}

}  // namespace w32

using namespace w32;

BOOL WINAPI InvalidateRect(HWND hwnd, const RECT *rect, BOOL erase)
{
   if (hwnd == nullptr)
   {
      // All windows.
      ForEachTopLevel([](HWND h, void *e) { InvalidateWhole(h, *(BOOL *) e); }, &erase);
      return TRUE;
   }
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return FALSE;
   if (rect)
   {
      Region r(*rect);
      Invalidate(hwnd, &r, erase, false);
   }
   else
      Invalidate(hwnd, nullptr, erase, false);
   return TRUE;
}

BOOL WINAPI InvalidateRgn(HWND hwnd, HRGN hrgn, BOOL erase)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return FALSE;
   Region *r = GetRegion(hrgn);
   Invalidate(hwnd, r, erase, false);
   return TRUE;
}

BOOL WINAPI ValidateRect(HWND hwnd, const RECT *rect)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return FALSE;
   if (rect == nullptr)
   {
      if (w->update)
         GetRegion(w->update)->Clear();
      w->erase_pending = false;
      w->internal_paint = false;
   }
   else if (w->update)
      GetRegion(w->update)->Subtract(*rect);
   DropEmptyUpdate(w);
   return TRUE;
}

BOOL WINAPI ValidateRgn(HWND hwnd, HRGN hrgn)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return FALSE;
   if (Region *r = GetRegion(hrgn); r && w->update)
      GetRegion(w->update)->Subtract(*r);
   else if (hrgn == nullptr)
      return ValidateRect(hwnd, nullptr);
   DropEmptyUpdate(w);
   return TRUE;
}

BOOL WINAPI GetUpdateRect(HWND hwnd, RECT *rect, BOOL erase)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return FALSE;
   RECT b = w->update ? GetRegion(w->update)->Bounds() : RECT{0, 0, 0, 0};
   if (rect)
      *rect = b;
   if (erase && w->erase_pending)
   {
      HDC hdc = GetDC(hwnd);
      w->erase_pending = !SendMessageA(hwnd, WM_ERASEBKGND, (WPARAM) hdc, 0);
      ReleaseDC(hwnd, hdc);
   }
   return !IsRectEmpty(&b);
}

BOOL WINAPI UpdateWindow(HWND hwnd)
{
   return RedrawWindow(hwnd, nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN);
}

BOOL WINAPI RedrawWindow(HWND hwnd, const RECT *rect, HRGN hrgn, UINT flags)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr)
      return FALSE;
   bool all_children = (flags & RDW_ALLCHILDREN) != 0;
   if (flags & RDW_INVALIDATE)
   {
      Region r;
      if (Region *given = GetRegion(hrgn))
         r = *given;
      else if (rect)
         r = Region(*rect);
      Invalidate(hwnd, (hrgn || rect) ? &r : nullptr, (flags & RDW_ERASE) != 0, all_children);
      if (flags & RDW_INTERNALPAINT)
         w->internal_paint = true, PaintPending();
   }
   if (flags & RDW_VALIDATE)
      ValidateRect(hwnd, rect);
   if (flags & (RDW_UPDATENOW | RDW_ERASENOW))
   {
      // Paint now: this window, then its children, parents first and siblings
      // from the top of the Z order down (Wine's update_now), so lower ones
      // paint last where they overlap.
      std::vector<HWND> order = {hwnd};
      for (size_t i = 0; i < order.size(); i++)
         if (Window *o = GetWindowObject(order[i]); o && (all_children || order[i] == hwnd || (flags & RDW_UPDATENOW)))
            order.insert(order.end(), o->children.begin(), o->children.end());
      for (HWND h : order)
      {
         Window *o = GetWindowObject(h);
         if (o == nullptr || !o->visible || (!o->update && !o->internal_paint))
            continue;
         if (flags & RDW_UPDATENOW)
         {
            MSG m = {h, WM_PAINT, 0, 0, GetTickCount(), {0, 0}};
            DispatchMessageA(&m);
         }
         else if (o->erase_pending)
         {
            HDC hdc = GetDC(h);
            o->erase_pending = !SendMessageA(h, WM_ERASEBKGND, (WPARAM) hdc, 0);
            ReleaseDC(h, hdc);
         }
      }
   }
   return TRUE;
}

HDC WINAPI BeginPaint(HWND hwnd, PAINTSTRUCT *ps)
{
   Window *w = GetWindowObject(hwnd);
   if (w == nullptr || ps == nullptr)
      return nullptr;
   HDC hdc = MakeWindowDC(hwnd, true);
   DC *dc = GetDCObject(hdc);

   Region update = w->update ? *GetRegion(w->update) : Region();
   if (update.Empty() && !w->internal_paint)
      update = Region();   // nothing to paint: an empty clip
   ps->hdc = hdc;
   ps->rcPaint = update.Bounds();
   ps->fRestore = FALSE;
   ps->fIncUpdate = FALSE;

   // Clip to the update region (in surface coordinates).
   Region clip = update;
   clip.Offset(dc->origin.x, dc->origin.y);
   dc->clip = clip;
   dc->has_clip = true;

   bool erase = w->erase_pending;
   // Windows with a non-client area of ours draw it first: child windows' borders,
   // top-level windows' menu bars.
   if (erase && (w->client.left > 0 || w->client.top > 0 || w->client.right < w->rect.right - w->rect.left ||
                 w->client.bottom < w->rect.bottom - w->rect.top))
      SendMessageA(hwnd, WM_NCPAINT, 1, 0);
   if (w->update)
      GetRegion(w->update)->Clear();
   DropEmptyUpdate(w);
   ValidateParents(w);
   w->erase_pending = false;
   w->internal_paint = false;
   CaretBeginPaint(hwnd);
   ps->fErase = erase ? !SendMessageA(hwnd, WM_ERASEBKGND, (WPARAM) hdc, 0) : FALSE;
   return hdc;
}

BOOL WINAPI EndPaint(HWND hwnd, const PAINTSTRUCT *ps)
{
   if (ps == nullptr)
      return FALSE;
   BOOL r = ReleaseDC(hwnd, ps->hdc);
   CaretEndPaint(hwnd);
   return r;
}

HDC WINAPI GetDC(HWND hwnd)
{
   if (hwnd == nullptr)
   {
      // The screen: a display DC to measure text and make (32-bit) compatible
      // bitmaps with; drawing on it shows nothing.
      HDC hdc = CreateCompatibleDC(nullptr);
      if (DC *dc = GetDCObject(hdc))
      {
         dc->memory = false;
         dc->type = OBJ_DC;
         dc->bitmap = nullptr;
      }
      return hdc;
   }
   return MakeWindowDC(hwnd, true);
}

HDC WINAPI GetDCEx(HWND hwnd, HRGN clip, DWORD flags)
{
   HDC hdc = (flags & DCX_WINDOW) ? MakeWindowDC(hwnd, false) : GetDC(hwnd);
   if (DC *dc = GetDCObject(hdc); dc && clip && (flags & (DCX_INTERSECTRGN | DCX_EXCLUDERGN)))
      if (Region *r = GetRegion(clip))
      {
         Region c = *r;
         c.Offset(dc->origin.x, dc->origin.y);
         if (flags & DCX_INTERSECTRGN)
            dc->visible.Intersect(c);
         else
            dc->visible.Subtract(c);
      }
   return hdc;
}

HDC WINAPI GetWindowDC(HWND hwnd)
{
   return MakeWindowDC(hwnd, false);
}

INT WINAPI ReleaseDC(HWND, HDC hdc)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || dc->memory)
      return 0;
   // Window DCs are single-use: they go back, showing what was drawn.
   return DeleteDC(hdc);
}
