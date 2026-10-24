/*
 * rect.cpp:  The rectangle and point helpers of user32.
 */

#include "core/win32.h"

#include <algorithm>

static bool Empty(const RECT *r)
{
   return r->left >= r->right || r->top >= r->bottom;
}

BOOL WINAPI SetRect(RECT *r, INT left, INT top, INT right, INT bottom)
{
   if (r == nullptr)
      return FALSE;
   *r = {left, top, right, bottom};
   return TRUE;
}

BOOL WINAPI SetRectEmpty(RECT *r)
{
   return SetRect(r, 0, 0, 0, 0);
}

BOOL WINAPI CopyRect(RECT *dest, const RECT *src)
{
   if (dest == nullptr || src == nullptr)
      return FALSE;
   *dest = *src;
   return TRUE;
}

BOOL WINAPI IsRectEmpty(const RECT *r)
{
   return r == nullptr || Empty(r);
}

BOOL WINAPI EqualRect(const RECT *a, const RECT *b)
{
   return a && b && a->left == b->left && a->top == b->top && a->right == b->right && a->bottom == b->bottom;
}

BOOL WINAPI OffsetRect(RECT *r, INT dx, INT dy)
{
   if (r == nullptr)
      return FALSE;
   r->left += dx;
   r->right += dx;
   r->top += dy;
   r->bottom += dy;
   return TRUE;
}

BOOL WINAPI InflateRect(RECT *r, INT dx, INT dy)
{
   if (r == nullptr)
      return FALSE;
   r->left -= dx;
   r->right += dx;
   r->top -= dy;
   r->bottom += dy;
   return TRUE;
}

BOOL WINAPI PtInRect(const RECT *r, POINT p)
{
   return r && p.x >= r->left && p.x < r->right && p.y >= r->top && p.y < r->bottom;
}

BOOL WINAPI IntersectRect(RECT *dest, const RECT *a, const RECT *b)
{
   if (dest == nullptr || a == nullptr || b == nullptr || Empty(a) || Empty(b) ||
       a->left >= b->right || b->left >= a->right || a->top >= b->bottom || b->top >= a->bottom)
   {
      if (dest)
         SetRectEmpty(dest);
      return FALSE;
   }
   *dest = {std::max(a->left, b->left), std::max(a->top, b->top), std::min(a->right, b->right),
            std::min(a->bottom, b->bottom)};
   return TRUE;
}

BOOL WINAPI UnionRect(RECT *dest, const RECT *a, const RECT *b)
{
   if (dest == nullptr || a == nullptr || b == nullptr)
      return FALSE;
   if (Empty(a))
   {
      if (Empty(b))
      {
         SetRectEmpty(dest);
         return FALSE;
      }
      *dest = *b;
      return TRUE;
   }
   if (Empty(b))
   {
      *dest = *a;
      return TRUE;
   }
   *dest = {std::min(a->left, b->left), std::min(a->top, b->top), std::max(a->right, b->right),
            std::max(a->bottom, b->bottom)};
   return TRUE;
}

BOOL WINAPI SubtractRect(RECT *dest, const RECT *a, const RECT *b)
{
   // Windows' rule: the result is a only shrunk when b covers a full side of it.
   if (dest == nullptr || a == nullptr || b == nullptr)
      return FALSE;
   RECT r = *a, i;
   if (IntersectRect(&i, a, b))
   {
      if (i.left == a->left && i.right == a->right)
      {
         if (i.top == a->top)
            r.top = i.bottom;
         else if (i.bottom == a->bottom)
            r.bottom = i.top;
      }
      else if (i.top == a->top && i.bottom == a->bottom)
      {
         if (i.left == a->left)
            r.left = i.right;
         else if (i.right == a->right)
            r.right = i.left;
      }
   }
   *dest = r;
   return !Empty(dest);
}
