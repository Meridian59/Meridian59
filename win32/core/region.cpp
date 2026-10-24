/*
 * region.cpp:  Region arithmetic, and the HRGN API on it.
 */

#include "core/region.h"

#include "core/gdi.h"

#include <algorithm>

namespace w32 {

namespace {

bool Valid(const RECT &r)
{
   return r.right > r.left && r.bottom > r.top;
}

// Appends the parts of a not covered by b.
void Difference(const RECT &a, const RECT &b, std::vector<RECT> &out)
{
   RECT i;
   if (!IntersectRect(&i, &a, &b))
   {
      out.push_back(a);
      return;
   }
   if (a.top < i.top)
      out.push_back({a.left, a.top, a.right, i.top});
   if (i.bottom < a.bottom)
      out.push_back({a.left, i.bottom, a.right, a.bottom});
   if (a.left < i.left)
      out.push_back({a.left, i.top, i.left, i.bottom});
   if (i.right < a.right)
      out.push_back({i.right, i.top, a.right, i.bottom});
}

}  // namespace

RECT Region::Bounds() const
{
   if (rects_.empty())
      return {0, 0, 0, 0};
   RECT b = rects_[0];
   for (const RECT &r : rects_)
      UnionRect(&b, &b, &r);
   return b;
}

bool Region::Contains(int x, int y) const
{
   POINT p = {x, y};
   for (const RECT &r : rects_)
      if (PtInRect(&r, p))
         return true;
   return false;
}

bool Region::Intersects(const RECT &rect) const
{
   RECT i;
   for (const RECT &r : rects_)
      if (IntersectRect(&i, &r, &rect))
         return true;
   return false;
}

void Region::Add(const RECT &r)
{
   if (!Valid(r))
      return;
   // Keep rectangles disjoint: add the parts of r not already covered.
   std::vector<RECT> pieces = {r};
   for (const RECT &have : rects_)
   {
      std::vector<RECT> next;
      for (const RECT &p : pieces)
         Difference(p, have, next);
      pieces.swap(next);
      if (pieces.empty())
         return;
   }
   rects_.insert(rects_.end(), pieces.begin(), pieces.end());
   // Many small rectangles slow every later operation; collapse to the bounds
   // past a limit (repaints get a little larger, never smaller).
   if (rects_.size() > 64)
   {
      RECT b = Bounds();
      rects_.assign(1, b);
   }
}

void Region::Add(const Region &r)
{
   for (const RECT &rect : r.rects_)
      Add(rect);
}

void Region::Subtract(const RECT &r)
{
   std::vector<RECT> out;
   for (const RECT &have : rects_)
      Difference(have, r, out);
   rects_.swap(out);
}

void Region::Subtract(const Region &r)
{
   for (const RECT &rect : r.rects_)
      Subtract(rect);
}

void Region::Intersect(const RECT &r)
{
   std::vector<RECT> out;
   RECT i;
   for (const RECT &have : rects_)
      if (IntersectRect(&i, &have, &r))
         out.push_back(i);
   rects_.swap(out);
}

void Region::Intersect(const Region &r)
{
   std::vector<RECT> out;
   RECT i;
   for (const RECT &a : rects_)
      for (const RECT &b : r.rects_)
         if (IntersectRect(&i, &a, &b))
            out.push_back(i);
   rects_.swap(out);
}

void Region::Offset(int dx, int dy)
{
   for (RECT &r : rects_)
      OffsetRect(&r, dx, dy);
}

}  // namespace w32

using namespace w32;

static INT Complexity(const Region &r)
{
   return r.Empty() ? NULLREGION : r.Rects().size() == 1 ? SIMPLEREGION : COMPLEXREGION;
}

HRGN WINAPI CreateRectRgn(INT left, INT top, INT right, INT bottom)
{
   RECT r = {left, top, right, bottom};
   return (HRGN) AddGdiObject(new GdiRegion(Region(r)));
}

HRGN WINAPI CreateRectRgnIndirect(const RECT *r)
{
   return CreateRectRgn(r->left, r->top, r->right, r->bottom);
}

INT WINAPI CombineRgn(HRGN dest, HRGN a, HRGN b, INT mode)
{
   Region *d = GetRegion(dest), *ra = GetRegion(a), *rb = GetRegion(b);
   if (d == nullptr || ra == nullptr || (mode != RGN_COPY && rb == nullptr))
      return ERROR;
   Region result = *ra;
   switch (mode)
   {
   case RGN_AND: result.Intersect(*rb); break;
   case RGN_OR: result.Add(*rb); break;
   case RGN_DIFF: result.Subtract(*rb); break;
   case RGN_XOR:
   {
      Region both = *ra;
      both.Intersect(*rb);
      result.Add(*rb);
      result.Subtract(both);
      break;
   }
   case RGN_COPY: break;
   default: return ERROR;
   }
   *d = result;
   return Complexity(*d);
}

INT WINAPI OffsetRgn(HRGN hrgn, INT dx, INT dy)
{
   Region *r = GetRegion(hrgn);
   if (r == nullptr)
      return ERROR;
   r->Offset(dx, dy);
   return Complexity(*r);
}

INT WINAPI GetRgnBox(HRGN hrgn, RECT *rect)
{
   Region *r = GetRegion(hrgn);
   if (r == nullptr || rect == nullptr)
      return ERROR;
   *rect = r->Bounds();
   return Complexity(*r);
}

BOOL WINAPI PtInRegion(HRGN hrgn, INT x, INT y)
{
   Region *r = GetRegion(hrgn);
   return r && r->Contains(x, y);
}

BOOL WINAPI SetRectRgn(HRGN hrgn, INT left, INT top, INT right, INT bottom)
{
   Region *r = GetRegion(hrgn);
   if (r == nullptr)
      return FALSE;
   r->Clear();
   r->Add(RECT{left, top, right, bottom});
   return TRUE;
}
