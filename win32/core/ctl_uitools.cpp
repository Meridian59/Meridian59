/*
 * ctl_uitools.cpp:  The classic look's building blocks: DrawEdge,
 *   DrawFrameControl (buttons, check boxes, radio buttons, scroll arrows,
 *   caption and menu glyphs), DrawState and GrayString.  After Wine 10's
 *   user32/uitools.c (LGPL 2.1+, Bertho Stultiens et al.), drawn with the layer's GDI.
 */

#include "core/gdi.h"

#include "core/cursor.h"

#include <algorithm>
#include <cstring>

namespace w32 {

// A pen of a system color (stock: DeleteObject leaves it).
HPEN SysColorPen(int index)
{
   static HPEN pens[COLOR_MENUBAR + 1];
   if (index < 0 || index > COLOR_MENUBAR)
      return (HPEN) GetStockObject(BLACK_PEN);
   if (pens[index] == nullptr)
   {
      pens[index] = CreatePen(PS_SOLID, 1, GetSysColor(index));
      if (GdiObject *o = GetGdiObject(pens[index]))
         o->stock = true;
   }
   return pens[index];
}

// The 50% gray checkerboard (0x55/0xAA rows): text-color and background-color pixels.
HBRUSH Brush55AA()
{
   static HBRUSH brush = [] {
      static const WORD pattern[] = {0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA};
      HBITMAP bitmap = CreateBitmap(8, 8, 1, 1, pattern);
      HBRUSH b = CreatePatternBrush(bitmap);
      if (GdiObject *o = GetGdiObject(b))
         o->stock = true;
      return b;
   }();
   return brush;
}

namespace {

const signed char LTInnerNormal[] = {
   -1, -1, -1, -1,
   -1, COLOR_BTNHIGHLIGHT, COLOR_BTNHIGHLIGHT, -1,
   -1, COLOR_3DDKSHADOW, COLOR_3DDKSHADOW, -1,
   -1, -1, -1, -1};

const signed char LTOuterNormal[] = {
   -1, COLOR_3DLIGHT, COLOR_BTNSHADOW, -1,
   COLOR_BTNHIGHLIGHT, COLOR_3DLIGHT, COLOR_BTNSHADOW, -1,
   COLOR_3DDKSHADOW, COLOR_3DLIGHT, COLOR_BTNSHADOW, -1,
   -1, COLOR_3DLIGHT, COLOR_BTNSHADOW, -1};

const signed char RBInnerNormal[] = {
   -1, -1, -1, -1,
   -1, COLOR_BTNSHADOW, COLOR_BTNSHADOW, -1,
   -1, COLOR_3DLIGHT, COLOR_3DLIGHT, -1,
   -1, -1, -1, -1};

const signed char RBOuterNormal[] = {
   -1, COLOR_3DDKSHADOW, COLOR_BTNHIGHLIGHT, -1,
   COLOR_BTNSHADOW, COLOR_3DDKSHADOW, COLOR_BTNHIGHLIGHT, -1,
   COLOR_3DLIGHT, COLOR_3DDKSHADOW, COLOR_BTNHIGHLIGHT, -1,
   -1, COLOR_3DDKSHADOW, COLOR_BTNHIGHLIGHT, -1};

const signed char LTInnerSoft[] = {
   -1, -1, -1, -1,
   -1, COLOR_3DLIGHT, COLOR_3DLIGHT, -1,
   -1, COLOR_BTNSHADOW, COLOR_BTNSHADOW, -1,
   -1, -1, -1, -1};

const signed char LTOuterSoft[] = {
   -1, COLOR_BTNHIGHLIGHT, COLOR_3DDKSHADOW, -1,
   COLOR_3DLIGHT, COLOR_BTNHIGHLIGHT, COLOR_3DDKSHADOW, -1,
   COLOR_BTNSHADOW, COLOR_BTNHIGHLIGHT, COLOR_3DDKSHADOW, -1,
   -1, COLOR_BTNHIGHLIGHT, COLOR_3DDKSHADOW, -1};

#define RBInnerSoft RBInnerNormal
#define RBOuterSoft RBOuterNormal

const signed char LTRBOuterMono[] = {
   -1, COLOR_WINDOWFRAME, COLOR_WINDOWFRAME, COLOR_WINDOWFRAME,
   COLOR_WINDOW, COLOR_WINDOWFRAME, COLOR_WINDOWFRAME, COLOR_WINDOWFRAME,
   COLOR_WINDOW, COLOR_WINDOWFRAME, COLOR_WINDOWFRAME, COLOR_WINDOWFRAME,
   COLOR_WINDOW, COLOR_WINDOWFRAME, COLOR_WINDOWFRAME, COLOR_WINDOWFRAME};

const signed char LTRBInnerMono[] = {
   -1, -1, -1, -1,
   -1, COLOR_WINDOW, COLOR_WINDOW, COLOR_WINDOW,
   -1, COLOR_WINDOW, COLOR_WINDOW, COLOR_WINDOW,
   -1, COLOR_WINDOW, COLOR_WINDOW, COLOR_WINDOW};

const signed char LTRBOuterFlat[] = {
   -1, COLOR_BTNSHADOW, COLOR_BTNSHADOW, COLOR_BTNSHADOW,
   COLOR_BTNFACE, COLOR_BTNSHADOW, COLOR_BTNSHADOW, COLOR_BTNSHADOW,
   COLOR_BTNFACE, COLOR_BTNSHADOW, COLOR_BTNSHADOW, COLOR_BTNSHADOW,
   COLOR_BTNFACE, COLOR_BTNSHADOW, COLOR_BTNSHADOW, COLOR_BTNSHADOW};

const signed char LTRBInnerFlat[] = {
   -1, -1, -1, -1,
   -1, COLOR_BTNFACE, COLOR_BTNFACE, COLOR_BTNFACE,
   -1, COLOR_BTNFACE, COLOR_BTNFACE, COLOR_BTNFACE,
   -1, COLOR_BTNFACE, COLOR_BTNFACE, COLOR_BTNFACE};

template <class T> T Select(HDC hdc, T object)
{
   return (T) SelectObject(hdc, object);
}

BOOL DrawDiagEdge(HDC hdc, RECT *rc, UINT type, UINT flags)
{
   POINT points[4];
   int width = rc->right - rc->left, height = rc->bottom - rc->top;
   int small_diam = width > height ? height : width;
   BOOL retval = !(((type & BDR_INNER) == BDR_INNER || (type & BDR_OUTER) == BDR_OUTER) && !(flags & (BF_FLAT | BF_MONO)));
   int add = (LTRBInnerMono[type & (BDR_INNER | BDR_OUTER)] != -1 ? 1 : 0) +
             (LTRBOuterMono[type & (BDR_INNER | BDR_OUTER)] != -1 ? 1 : 0);
   HPEN inner_pen, outer_pen;
   outer_pen = inner_pen = (HPEN) GetStockObject(NULL_PEN);
   HPEN save_pen = Select(hdc, inner_pen);
   int spx = 0, spy = 0, epx = 0, epy = 0;
   signed char inner, outer;
   UINT t = type & (BDR_INNER | BDR_OUTER);
   if (flags & BF_MONO)
      inner = LTRBInnerMono[t], outer = LTRBOuterMono[t];
   else if (flags & BF_FLAT)
      inner = LTRBInnerFlat[t], outer = LTRBOuterFlat[t];
   else if (flags & BF_SOFT)
   {
      if (flags & BF_BOTTOM)
         inner = RBInnerSoft[t], outer = RBOuterSoft[t];
      else
         inner = LTInnerSoft[t], outer = LTOuterSoft[t];
   }
   else if (flags & BF_BOTTOM)
      inner = RBInnerNormal[t], outer = RBOuterNormal[t];
   else
      inner = LTInnerNormal[t], outer = LTOuterNormal[t];
   if (inner != -1)
      inner_pen = SysColorPen(inner);
   if (outer != -1)
      outer_pen = SysColorPen(outer);

   POINT save_point;
   MoveToEx(hdc, 0, 0, &save_point);
   switch (flags & BF_RECT)
   {
   case 0:
   case BF_LEFT:
   case BF_BOTTOM:
   case BF_BOTTOMLEFT:
      epx = rc->left - 1;
      spx = epx + small_diam;
      epy = rc->bottom;
      spy = epy - small_diam;
      break;
   case BF_TOPLEFT:
   case BF_BOTTOMRIGHT:
      epx = rc->left - 1;
      spx = epx + small_diam;
      epy = rc->top - 1;
      spy = epy + small_diam;
      break;
   case BF_TOP:
   case BF_RIGHT:
   case BF_TOPRIGHT:
   case BF_RIGHT | BF_LEFT:
   case BF_RIGHT | BF_LEFT | BF_TOP:
   case BF_BOTTOM | BF_TOP:
   case BF_BOTTOM | BF_TOP | BF_LEFT:
   case BF_BOTTOMRIGHT | BF_LEFT:
   case BF_BOTTOMRIGHT | BF_TOP:
   case BF_RECT:
      spx = rc->left;
      epx = spx + small_diam;
      spy = rc->bottom - 1;
      epy = spy - small_diam;
      break;
   }
   MoveToEx(hdc, spx, spy, nullptr);
   SelectObject(hdc, outer_pen);
   LineTo(hdc, epx, epy);
   SelectObject(hdc, inner_pen);
   switch (flags & (BF_RECT | BF_DIAGONAL))
   {
   case BF_DIAGONAL_ENDBOTTOMLEFT:
   case (BF_DIAGONAL | BF_BOTTOM):
   case BF_DIAGONAL:
   case (BF_DIAGONAL | BF_LEFT):
      MoveToEx(hdc, spx - 1, spy, nullptr);
      LineTo(hdc, epx, epy - 1);
      points[0] = {spx - add, spy};
      points[1] = {rc->left, rc->top};
      points[2] = {epx + 1, epy - 1 - add};
      points[3] = points[2];
      break;
   case BF_DIAGONAL_ENDBOTTOMRIGHT:
      MoveToEx(hdc, spx - 1, spy, nullptr);
      LineTo(hdc, epx, epy + 1);
      points[0] = {spx - add, spy};
      points[1] = {rc->left, rc->bottom - 1};
      points[2] = {epx + 1, epy + 1 + add};
      points[3] = points[2];
      break;
   case (BF_DIAGONAL | BF_BOTTOM | BF_RIGHT | BF_TOP):
   case (BF_DIAGONAL | BF_BOTTOM | BF_RIGHT | BF_TOP | BF_LEFT):
   case BF_DIAGONAL_ENDTOPRIGHT:
   case (BF_DIAGONAL | BF_RIGHT | BF_TOP | BF_LEFT):
      MoveToEx(hdc, spx + 1, spy, nullptr);
      LineTo(hdc, epx, epy + 1);
      points[0] = {epx - 1, epy + 1 + add};
      points[1] = {rc->right - 1, rc->top + add};
      points[2] = {rc->right - 1, rc->bottom - 1};
      points[3] = {spx + add, spy};
      break;
   case BF_DIAGONAL_ENDTOPLEFT:
      MoveToEx(hdc, spx, spy - 1, nullptr);
      LineTo(hdc, epx + 1, epy);
      points[0] = {epx + 1 + add, epy + 1};
      points[1] = {rc->right - 1, rc->top};
      points[2] = {rc->right - 1, rc->bottom - 1 - add};
      points[3] = {spx, spy - add};
      break;
   case (BF_DIAGONAL | BF_TOP):
   case (BF_DIAGONAL | BF_BOTTOM | BF_TOP):
   case (BF_DIAGONAL | BF_BOTTOM | BF_TOP | BF_LEFT):
      MoveToEx(hdc, spx + 1, spy - 1, nullptr);
      LineTo(hdc, epx, epy);
      points[0] = {epx - 1, epy + 1};
      points[1] = {rc->right - 1, rc->top};
      points[2] = {rc->right - 1, rc->bottom - 1 - add};
      points[3] = {spx + add, spy - add};
      break;
   case (BF_DIAGONAL | BF_RIGHT):
   case (BF_DIAGONAL | BF_RIGHT | BF_LEFT):
   case (BF_DIAGONAL | BF_RIGHT | BF_LEFT | BF_BOTTOM):
      MoveToEx(hdc, spx, spy, nullptr);
      LineTo(hdc, epx - 1, epy + 1);
      points[0] = {spx, spy};
      points[1] = {rc->left, rc->top + add};
      points[2] = {epx - 1 - add, epy + 1 + add};
      points[3] = points[2];
      break;
   default:
      points[0] = points[1] = points[2] = points[3] = {spx, spy};
      break;
   }
   if ((flags & BF_MIDDLE) && retval)
   {
      HBRUSH save_brush = Select(hdc, GetSysColorBrush(flags & BF_MONO ? COLOR_WINDOW : COLOR_BTNFACE));
      HPEN pen = Select(hdc, SysColorPen(flags & BF_MONO ? COLOR_WINDOW : COLOR_BTNFACE));
      Polygon(hdc, points, 4);
      SelectObject(hdc, save_brush);
      SelectObject(hdc, pen);
   }
   if (flags & BF_ADJUST)
   {
      if (flags & BF_LEFT)
         rc->left += add;
      if (flags & BF_RIGHT)
         rc->right -= add;
      if (flags & BF_TOP)
         rc->top += add;
      if (flags & BF_BOTTOM)
         rc->bottom -= add;
   }
   SelectObject(hdc, save_pen);
   MoveToEx(hdc, save_point.x, save_point.y, nullptr);
   return retval;
}

BOOL DrawRectEdge(HDC hdc, RECT *rc, UINT type, UINT flags, UINT width)
{
   signed char lti, lto, rbi, rbo;
   RECT inner = *rc, fill;
   int lbi_offset = 0, lti_offset = 0, rti_offset = 0, rbi_offset = 0;
   BOOL retval = !(((type & BDR_INNER) == BDR_INNER || (type & BDR_OUTER) == BDR_OUTER) && !(flags & (BF_FLAT | BF_MONO)));
   UINT t = type & (BDR_INNER | BDR_OUTER);
   HBRUSH null_brush = (HBRUSH) GetStockObject(NULL_BRUSH);
   HBRUSH lti_brush = null_brush, lto_brush = null_brush, rbi_brush = null_brush, rbo_brush = null_brush;
   if (flags & BF_MONO)
   {
      lti = rbi = LTRBInnerMono[t];
      lto = rbo = LTRBOuterMono[t];
   }
   else if (flags & BF_FLAT)
   {
      lti = rbi = LTRBInnerFlat[t];
      lto = rbo = LTRBOuterFlat[t];
      if (lti != -1)
         lti = rbi = COLOR_BTNFACE;
   }
   else if (flags & BF_SOFT)
   {
      lti = LTInnerSoft[t];
      lto = LTOuterSoft[t];
      rbi = RBInnerSoft[t];
      rbo = RBOuterSoft[t];
   }
   else
   {
      lti = LTInnerNormal[t];
      lto = LTOuterNormal[t];
      rbi = RBInnerNormal[t];
      rbo = RBOuterNormal[t];
   }
   if ((flags & BF_BOTTOMLEFT) == BF_BOTTOMLEFT)
      lbi_offset = width;
   if ((flags & BF_TOPRIGHT) == BF_TOPRIGHT)
      rti_offset = width;
   if ((flags & BF_BOTTOMRIGHT) == BF_BOTTOMRIGHT)
      rbi_offset = width;
   if ((flags & BF_TOPLEFT) == BF_TOPLEFT)
      lti_offset = width;
   if (lti != -1)
      lti_brush = GetSysColorBrush(lti);
   if (lto != -1)
      lto_brush = GetSysColorBrush(lto);
   if (rbi != -1)
      rbi_brush = GetSysColorBrush(rbi);
   if (rbo != -1)
      rbo_brush = GetSysColorBrush(rbo);

   int w = (int) width;
   if (flags & BF_TOP)
   {
      fill = inner;
      fill.bottom = fill.top + w;
      FillRect(hdc, &fill, lto_brush);
   }
   if (flags & BF_LEFT)
   {
      fill = inner;
      fill.right = fill.left + w;
      FillRect(hdc, &fill, lto_brush);
   }
   if (flags & BF_BOTTOM)
   {
      fill = inner;
      fill.top = fill.bottom - w;
      FillRect(hdc, &fill, rbo_brush);
   }
   if (flags & BF_RIGHT)
   {
      fill = inner;
      fill.left = fill.right - w;
      FillRect(hdc, &fill, rbo_brush);
   }
   if (flags & BF_TOP)
   {
      SetRect(&fill, inner.left + lti_offset, inner.top + w, inner.right - rti_offset, inner.top + 2 * w);
      FillRect(hdc, &fill, lti_brush);
   }
   if (flags & BF_LEFT)
   {
      SetRect(&fill, inner.left + w, inner.top + lti_offset, inner.left + 2 * w, inner.bottom - lbi_offset);
      FillRect(hdc, &fill, lti_brush);
   }
   if (flags & BF_BOTTOM)
   {
      SetRect(&fill, inner.left + lbi_offset, inner.bottom - 2 * w, inner.right - rbi_offset, inner.bottom - w);
      FillRect(hdc, &fill, rbi_brush);
   }
   if (flags & BF_RIGHT)
   {
      SetRect(&fill, inner.right - 2 * w, inner.top + rti_offset, inner.right - w, inner.bottom - rbi_offset);
      FillRect(hdc, &fill, rbi_brush);
   }
   if (((flags & BF_MIDDLE) && retval) || (flags & BF_ADJUST))
   {
      int add = (LTRBInnerMono[t] != -1 ? w : 0) + (LTRBOuterMono[t] != -1 ? w : 0);
      if (flags & BF_LEFT)
         inner.left += add;
      if (flags & BF_RIGHT)
         inner.right -= add;
      if (flags & BF_TOP)
         inner.top += add;
      if (flags & BF_BOTTOM)
         inner.bottom -= add;
      if ((flags & BF_MIDDLE) && retval)
         FillRect(hdc, &inner, GetSysColorBrush(flags & BF_MONO ? COLOR_WINDOW : COLOR_BTNFACE));
      if (flags & BF_ADJUST)
         *rc = inner;
   }
   return retval;
}

int MakeSquareRect(const RECT *src, RECT *dst)
{
   int width = src->right - src->left, height = src->bottom - src->top;
   int small_diam = width > height ? height : width;
   *dst = *src;
   if (width < height)
   {
      dst->top += (height - width) / 2;
      dst->bottom = dst->top + small_diam;
   }
   else if (width > height)
   {
      dst->left += (width - height) / 2;
      dst->right = dst->left + small_diam;
   }
   return small_diam;
}

void DrawCheckedRect(HDC dc, RECT *rect)
{
   if (GetSysColor(COLOR_BTNHIGHLIGHT) == RGB(255, 255, 255))
   {
      FillRect(dc, rect, GetSysColorBrush(COLOR_BTNFACE));
      COLORREF bg = SetBkColor(dc, RGB(255, 255, 255));
      HBRUSH save = Select(dc, Brush55AA());
      PatBlt(dc, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, 0x00FA0089);
      SelectObject(dc, save);
      SetBkColor(dc, bg);
   }
   else
      FillRect(dc, rect, GetSysColorBrush(COLOR_BTNHIGHLIGHT));
}

BOOL ButtonPush(HDC dc, RECT *r, UINT state)
{
   UINT edge = (state & (DFCS_PUSHED | DFCS_CHECKED | DFCS_FLAT)) ? EDGE_SUNKEN : EDGE_RAISED;
   RECT myr = *r;
   if (state & DFCS_CHECKED)
   {
      if (state & DFCS_MONO)
         DrawRectEdge(dc, &myr, edge, BF_MONO | BF_RECT | BF_ADJUST, 1);
      else
         DrawRectEdge(dc, &myr, edge, (state & DFCS_FLAT) | BF_RECT | BF_SOFT | BF_ADJUST, 1);
      if (!(state & DFCS_TRANSPARENT))
         DrawCheckedRect(dc, &myr);
   }
   else if (state & DFCS_MONO)
   {
      DrawRectEdge(dc, &myr, edge, BF_MONO | BF_RECT | BF_ADJUST, 1);
      if (!(state & DFCS_TRANSPARENT))
         FillRect(dc, &myr, GetSysColorBrush(COLOR_BTNFACE));
   }
   else
      DrawRectEdge(dc, r, edge, (state & DFCS_FLAT) | ((state & DFCS_TRANSPARENT) ? 0 : BF_MIDDLE) | BF_RECT | BF_SOFT, 1);
   if (state & DFCS_ADJUSTRECT)
      InflateRect(r, -2, -2);
   return TRUE;
}

BOOL ButtonCheck(HDC dc, RECT *r, UINT state)
{
   RECT myr;
   UINT flags = BF_RECT | BF_ADJUST;
   MakeSquareRect(r, &myr);
   if (state & DFCS_FLAT)
      flags |= BF_FLAT;
   else if (state & DFCS_MONO)
      flags |= BF_MONO;
   DrawRectEdge(dc, &myr, EDGE_SUNKEN, flags, (UINT) std::max<int>((myr.right - myr.left + 8) / 16, 1));
   if (!(state & DFCS_TRANSPARENT))
   {
      if (state & (DFCS_INACTIVE | DFCS_PUSHED))
         FillRect(dc, &myr, GetSysColorBrush(COLOR_BTNFACE));
      else if ((state & DFCS_BUTTON3STATE) && (state & DFCS_CHECKED))
         DrawCheckedRect(dc, &myr);
      else
         FillRect(dc, &myr, GetSysColorBrush(COLOR_WINDOW));
   }
   if (state & DFCS_CHECKED)
   {
      POINT pt[6];
      int color = (state & DFCS_INACTIVE) || (state & 0xff) == DFCS_BUTTON3STATE ? COLOR_BTNSHADOW : COLOR_WINDOWTEXT;
      pt[0] = {myr.right - 1, myr.top};
      pt[1] = {pt[0].x, pt[0].y + (myr.bottom - myr.top) / 3};
      pt[2] = {myr.left + (myr.right - myr.left) / 3, myr.bottom - 1};
      pt[3] = {myr.left + 1, 0};
      pt[3].y = pt[2].y - (pt[2].x - pt[3].x);
      pt[4] = {pt[3].x, pt[3].y - (myr.bottom - myr.top) / 3};
      pt[5] = {pt[2].x, pt[2].y - (myr.bottom - myr.top) / 3};
      HBRUSH old_brush = Select(dc, GetSysColorBrush(color));
      HPEN old_pen = Select(dc, (HPEN) GetStockObject(NULL_PEN));
      Polygon(dc, pt, 6);
      SelectObject(dc, old_brush);
      SelectObject(dc, old_pen);
   }
   return TRUE;
}

BOOL ButtonRadio(HDC dc, RECT *r, UINT state)
{
   RECT myr;
   int small_diam = MakeSquareRect(r, &myr);
   int border = std::max(small_diam / 16, 1);
   if ((state & 0xff) == DFCS_BUTTONRADIOIMAGE)
      FillRect(dc, r, (HBRUSH) GetStockObject(BLACK_BRUSH));
   else if ((state & 0xff) == DFCS_BUTTONRADIOMASK)
      FillRect(dc, r, (HBRUSH) GetStockObject(WHITE_BRUSH));
   int xc = myr.left + small_diam - small_diam / 2;
   int yc = myr.top + small_diam - small_diam / 2;
   int i = 14 * small_diam / 16;
   SetRect(&myr, xc - i + i / 2, yc - i + i / 2, xc + i / 2, yc + i / 2);
   HPEN save_pen;
   HBRUSH save_brush;
   if ((state & 0xff) == DFCS_BUTTONRADIOMASK)
   {
      save_brush = Select(dc, (HBRUSH) GetStockObject(BLACK_BRUSH));
      Ellipse(dc, myr.left, myr.top, myr.right, myr.bottom);
      SelectObject(dc, save_brush);
   }
   else
   {
      if (state & (DFCS_FLAT | DFCS_MONO))
      {
         save_pen = Select(dc, SysColorPen(COLOR_WINDOWFRAME));
         save_brush = Select(dc, GetSysColorBrush(COLOR_WINDOWFRAME));
         Ellipse(dc, myr.left, myr.top, myr.right, myr.bottom);
         SelectObject(dc, save_brush);
         SelectObject(dc, save_pen);
      }
      else
      {
         save_pen = Select(dc, SysColorPen(COLOR_BTNHIGHLIGHT));
         save_brush = Select(dc, GetSysColorBrush(COLOR_BTNHIGHLIGHT));
         Pie(dc, myr.left, myr.top, myr.right + 1, myr.bottom + 1, myr.left - 1, myr.bottom, myr.right + 1, myr.top);
         SelectObject(dc, SysColorPen(COLOR_BTNSHADOW));
         SelectObject(dc, GetSysColorBrush(COLOR_BTNSHADOW));
         Pie(dc, myr.left, myr.top, myr.right + 1, myr.bottom + 1, myr.right + 1, myr.top, myr.left - 1, myr.bottom);
         InflateRect(&myr, -border, -border);
         SelectObject(dc, SysColorPen(COLOR_3DLIGHT));
         SelectObject(dc, GetSysColorBrush(COLOR_3DLIGHT));
         Pie(dc, myr.left, myr.top, myr.right + 1, myr.bottom + 1, myr.left - 1, myr.bottom, myr.right + 1, myr.top);
         SelectObject(dc, SysColorPen(COLOR_3DDKSHADOW));
         SelectObject(dc, GetSysColorBrush(COLOR_3DDKSHADOW));
         Pie(dc, myr.left, myr.top, myr.right + 1, myr.bottom + 1, myr.right + 1, myr.top, myr.left - 1, myr.bottom);
         SelectObject(dc, save_brush);
         SelectObject(dc, save_pen);
      }
      i = 10 * small_diam / 16;
      SetRect(&myr, xc - i + i / 2, yc - i + i / 2, xc + i / 2, yc + i / 2);
      i = !(state & (DFCS_INACTIVE | DFCS_PUSHED)) ? COLOR_WINDOW : COLOR_BTNFACE;
      save_pen = Select(dc, SysColorPen(i));
      save_brush = Select(dc, GetSysColorBrush(i));
      Ellipse(dc, myr.left, myr.top, myr.right, myr.bottom);
      SelectObject(dc, save_brush);
      SelectObject(dc, save_pen);
   }
   if (state & DFCS_CHECKED)
   {
      i = std::max(6 * small_diam / 16, 1);
      SetRect(&myr, xc - i + i / 2, yc - i + i / 2, xc + i / 2, yc + i / 2);
      i = state & DFCS_INACTIVE ? COLOR_BTNSHADOW : COLOR_WINDOWTEXT;
      save_brush = Select(dc, GetSysColorBrush(i));
      save_pen = Select(dc, SysColorPen(i));
      Ellipse(dc, myr.left, myr.top, myr.right, myr.bottom);
      SelectObject(dc, save_pen);
      SelectObject(dc, save_brush);
   }
   return TRUE;
}

BOOL FrameButton(HDC hdc, RECT *rc, UINT state)
{
   switch (state & 0xff)
   {
   case DFCS_BUTTONPUSH: return ButtonPush(hdc, rc, state);
   case DFCS_BUTTONCHECK:
   case DFCS_BUTTON3STATE: return ButtonCheck(hdc, rc, state);
   case DFCS_BUTTONRADIOIMAGE:
   case DFCS_BUTTONRADIOMASK:
   case DFCS_BUTTONRADIO: return ButtonRadio(hdc, rc, state);
   }
   return FALSE;
}

// Caption buttons: Windows draws the Marlett font's glyphs; these are drawn as lines.
BOOL FrameCaption(HDC dc, RECT *r, UINT state)
{
   RECT myr;
   int small_diam = MakeSquareRect(r, &myr) - 2;
   ButtonPush(dc, r, state & 0xff00);
   int color = state & DFCS_INACTIVE ? COLOR_BTNSHADOW : COLOR_BTNTEXT;
   int s = std::max(small_diam / 2, 4) & ~1;   // glyph size
   int x0 = (myr.left + myr.right - s) / 2 + ((state & DFCS_PUSHED) ? 1 : 0);
   int y0 = (myr.top + myr.bottom - s) / 2 + ((state & DFCS_PUSHED) ? 1 : 0);
   auto glyph = [&](int dx, int dy, int c) {
      HPEN pen = CreatePen(PS_SOLID, 1, GetSysColor(c));
      HPEN old = Select(dc, pen);
      int x = x0 + dx, y = y0 + dy;
      switch (state & 0xf)
      {
      case DFCS_CAPTIONCLOSE:
         for (int k = 0; k < 2; k++)
         {
            MoveToEx(dc, x + k, y, nullptr);
            LineTo(dc, x + s + k, y + s);
            MoveToEx(dc, x + s - 1 + k, y, nullptr);
            LineTo(dc, x - 1 + k, y + s);
         }
         break;
      case DFCS_CAPTIONMIN:
         MoveToEx(dc, x, y + s - 2, nullptr);
         LineTo(dc, x + s - 2, y + s - 2);
         MoveToEx(dc, x, y + s - 1, nullptr);
         LineTo(dc, x + s - 2, y + s - 1);
         break;
      case DFCS_CAPTIONMAX:
      case DFCS_CAPTIONRESTORE:
      {
         RECT box = {x, y, x + s, y + s};
         HBRUSH b = (HBRUSH) GetStockObject(NULL_BRUSH);
         HBRUSH ob = Select(dc, b);
         Rectangle(dc, box.left, box.top, box.right, box.bottom);
         MoveToEx(dc, box.left, box.top + 1, nullptr);
         LineTo(dc, box.right, box.top + 1);
         SelectObject(dc, ob);
         break;
      }
      case DFCS_CAPTIONHELP:
         MoveToEx(dc, x + 1, y + 1, nullptr);
         LineTo(dc, x + s / 2, y);
         LineTo(dc, x + s - 1, y + 2);
         LineTo(dc, x + s / 2, y + s / 2 + 1);
         LineTo(dc, x + s / 2, y + s - 2);
         MoveToEx(dc, x + s / 2, y + s, nullptr);
         LineTo(dc, x + s / 2, y + s + 1);
         break;
      }
      SelectObject(dc, old);
      DeleteObject(pen);
   };
   if (state & DFCS_INACTIVE)
      glyph(1, 1, COLOR_BTNHIGHLIGHT);
   glyph(0, 0, color);
   return TRUE;
}

BOOL FrameScroll(HDC dc, RECT *r, UINT state)
{
   POINT line[4];
   RECT myr;
   int small_diam = MakeSquareRect(r, &myr) - 2;
   int tri = std::max(2, 290 * small_diam / 1000 - 1);
   HBRUSH save_brush, hb, hb2;
   HPEN save_pen, hp, hp2;
   int i, d46, d93;
   switch (state & 0xff)
   {
   case DFCS_SCROLLCOMBOBOX:
   case DFCS_SCROLLDOWN:
      line[2].x = myr.left + 470 * small_diam / 1000 + 2;
      line[2].y = myr.top + 687 * small_diam / 1000 + 1;
      line[0].x = line[2].x - tri;
      line[1].x = line[2].x + tri;
      line[0].y = line[1].y = line[2].y - tri;
      break;
   case DFCS_SCROLLUP:
      line[2].x = myr.left + 470 * small_diam / 1000 + 2;
      line[2].y = myr.bottom - (687 * small_diam / 1000 + 1);
      line[0].x = line[2].x - tri;
      line[1].x = line[2].x + tri;
      line[0].y = line[1].y = line[2].y + tri;
      break;
   case DFCS_SCROLLLEFT:
      line[2].x = myr.right - (687 * small_diam / 1000 + 1);
      line[2].y = myr.top + 470 * small_diam / 1000 + 2;
      line[0].y = line[2].y - tri;
      line[1].y = line[2].y + tri;
      line[0].x = line[1].x = line[2].x + tri;
      break;
   case DFCS_SCROLLRIGHT:
      line[2].x = myr.left + 687 * small_diam / 1000 + 1;
      line[2].y = myr.top + 470 * small_diam / 1000 + 2;
      line[0].y = line[2].y - tri;
      line[1].y = line[2].y + tri;
      line[0].x = line[1].x = line[2].x - tri;
      break;
   case DFCS_SCROLLSIZEGRIP:
      DrawRectEdge(dc, r, EDGE_BUMP, BF_MIDDLE | ((state & (DFCS_MONO | DFCS_FLAT)) ? BF_MONO : 0), 1);
      save_pen = Select(dc, (HPEN) GetStockObject(NULL_PEN));
      save_brush = Select(dc, (HBRUSH) GetStockObject(NULL_BRUSH));
      if (state & (DFCS_MONO | DFCS_FLAT))
      {
         hp = hp2 = SysColorPen(COLOR_WINDOWFRAME);
         hb = hb2 = GetSysColorBrush(COLOR_WINDOWFRAME);
      }
      else
      {
         hp = SysColorPen(COLOR_BTNHIGHLIGHT);
         hp2 = SysColorPen(COLOR_BTNSHADOW);
         hb = GetSysColorBrush(COLOR_BTNHIGHLIGHT);
         hb2 = GetSysColorBrush(COLOR_BTNSHADOW);
      }
      line[0].x = line[1].x = r->right - 1;
      line[2].y = line[3].y = r->bottom - 1;
      d46 = 46 * small_diam / 750;
      d93 = 93 * small_diam / 750;
      for (int k : {586, 398, 210})
      {
         i = k * small_diam / 750;
         line[0].y = r->bottom - i - 1;
         line[3].x = r->right - i - 1;
         line[1].y = line[0].y + d46;
         line[2].x = line[3].x + d46;
         SelectObject(dc, hb);
         SelectObject(dc, hp);
         Polygon(dc, line, 4);
         line[1].y++;
         line[2].x++;
         line[0].y = line[1].y + d93;
         line[3].x = line[2].x + d93;
         SelectObject(dc, hb2);
         SelectObject(dc, hp2);
         Polygon(dc, line, 4);
      }
      SelectObject(dc, save_pen);
      SelectObject(dc, save_brush);
      return TRUE;
   default:
      return FALSE;
   }
   if (!(state & (0xff00 & ~DFCS_ADJUSTRECT)))
      DrawRectEdge(dc, r, EDGE_RAISED, (state & DFCS_ADJUSTRECT) | BF_MIDDLE | BF_RECT, 1);
   else
      ButtonPush(dc, r, state & 0xff00);
   if (state & DFCS_INACTIVE)
   {
      save_brush = Select(dc, GetSysColorBrush(COLOR_BTNHIGHLIGHT));
      save_pen = Select(dc, SysColorPen(COLOR_BTNHIGHLIGHT));
      Polygon(dc, line, 3);
      SelectObject(dc, save_pen);
      SelectObject(dc, save_brush);
   }
   if ((state & DFCS_INACTIVE) || !(state & DFCS_PUSHED))
      for (i = 0; i < 3; i++)
      {
         line[i].x--;
         line[i].y--;
      }
   i = state & DFCS_INACTIVE ? COLOR_BTNSHADOW : COLOR_BTNTEXT;
   save_brush = Select(dc, GetSysColorBrush(i));
   save_pen = Select(dc, SysColorPen(i));
   Polygon(dc, line, 3);
   SelectObject(dc, save_pen);
   SelectObject(dc, save_brush);
   return TRUE;
}

BOOL FrameMenu(HDC dc, RECT *r, UINT state)
{
   POINT points[6];
   RECT myr;
   int small_diam = MakeSquareRect(r, &myr);
   BOOL retval = TRUE;
   FillRect(dc, r, (HBRUSH) GetStockObject(WHITE_BRUSH));
   HBRUSH save_brush = Select(dc, (HBRUSH) GetStockObject(BLACK_BRUSH));
   HPEN save_pen = Select(dc, (HPEN) GetStockObject(BLACK_PEN));
   int i, xe, ye, xc, yc;
   switch (state & 0xff)
   {
   case DFCS_MENUARROW:
      i = 187 * small_diam / 750;
      points[2].x = myr.left + 468 * small_diam / 750;
      points[2].y = myr.top + 352 * small_diam / 750 + 1;
      points[0].y = points[2].y - i;
      points[1].y = points[2].y + i;
      points[0].x = points[1].x = points[2].x - i;
      Polygon(dc, points, 3);
      break;
   case DFCS_MENUBULLET:
      xe = myr.left;
      ye = myr.top + small_diam - small_diam / 2;
      xc = myr.left + small_diam - small_diam / 2;
      yc = myr.top + small_diam - small_diam / 2;
      i = std::max(234 * small_diam / 750, 1);
      SetRect(&myr, xc - i + i / 2, yc - i + i / 2, xc + i / 2, yc + i / 2);
      Ellipse(dc, myr.left, myr.top, myr.right, myr.bottom);
      (void) xe;
      (void) ye;
      break;
   case DFCS_MENUCHECK:
      points[0].x = myr.left + 253 * small_diam / 1000;
      points[0].y = myr.top + 445 * small_diam / 1000;
      points[1].x = myr.left + 409 * small_diam / 1000;
      points[1].y = points[0].y + (points[1].x - points[0].x);
      points[2].x = myr.left + 690 * small_diam / 1000;
      points[2].y = points[1].y - (points[2].x - points[1].x);
      points[3].x = points[2].x;
      points[3].y = points[2].y + 3 * small_diam / 16;
      points[4].x = points[1].x;
      points[4].y = points[1].y + 3 * small_diam / 16;
      points[5].x = points[0].x;
      points[5].y = points[0].y + 3 * small_diam / 16;
      Polygon(dc, points, 6);
      break;
   default:
      retval = FALSE;
      break;
   }
   SelectObject(dc, save_pen);
   SelectObject(dc, save_brush);
   return retval;
}

bool IconSize(HICON icon, SIZE *size)
{
   const CursorImage *c = GetCursorImage(icon);
   if (c == nullptr)
      return false;
   size->cx = c->width ? c->width : GetSystemMetrics(SM_CXICON);
   size->cy = c->height ? c->height : GetSystemMetrics(SM_CYICON);
   return true;
}

BOOL DrawStateJam(HDC hdc, UINT opcode, DRAWSTATEPROC func, LPARAM lp, WPARAM wp, RECT *rc, UINT dtflags, bool unicode)
{
   int cx = rc->right - rc->left, cy = rc->bottom - rc->top;
   switch (opcode)
   {
   case DST_TEXT:
   case DST_PREFIXTEXT:
      if (unicode)
         return DrawTextW(hdc, (LPCWSTR) lp, (INT) wp, rc, dtflags);
      return DrawTextA(hdc, (LPCSTR) lp, (INT) wp, rc, dtflags);
   case DST_ICON:
      return DrawIconEx(hdc, rc->left, rc->top, (HICON) lp, 0, 0, 0, nullptr, DI_NORMAL);
   case DST_BITMAP:
   {
      HDC memdc = CreateCompatibleDC(hdc);
      if (memdc == nullptr)
         return FALSE;
      HGDIOBJ save = SelectObject(memdc, (HBITMAP) lp);
      if (save == nullptr)
      {
         DeleteDC(memdc);
         return FALSE;
      }
      BOOL r = BitBlt(hdc, rc->left, rc->top, cx, cy, memdc, 0, 0, SRCCOPY);
      SelectObject(memdc, save);
      DeleteDC(memdc);
      return r;
   }
   case DST_COMPLEX:
      if (func)
      {
         // The procedure draws at (0, 0).
         OffsetViewportOrgEx(hdc, rc->left, rc->top, nullptr);
         BOOL r = func(hdc, lp, wp, cx, cy);
         OffsetViewportOrgEx(hdc, -rc->left, -rc->top, nullptr);
         return r;
      }
      return FALSE;
   }
   return FALSE;
}

BOOL DrawStateCommon(HDC hdc, HBRUSH hbr, DRAWSTATEPROC func, LPARAM lp, WPARAM wp, INT x, INT y, INT cx, INT cy,
                     UINT flags, bool unicode)
{
   UINT dtflags = DT_NOCLIP;
   UINT opcode = flags & 0xf;
   INT len = (INT) wp;
   if ((opcode == DST_TEXT || opcode == DST_PREFIXTEXT) && !len)
   {
      if (!lp)
         return FALSE;
      len = unicode ? lstrlenW((LPCWSTR) lp) : (INT) strlen((LPCSTR) lp);
   }
   if (!cx || !cy)
   {
      SIZE s = {0, 0};
      BITMAP bm;
      switch (opcode)
      {
      case DST_TEXT:
      case DST_PREFIXTEXT:
         if (!(unicode ? GetTextExtentPoint32W(hdc, (LPCWSTR) lp, len, &s)
                       : GetTextExtentPoint32A(hdc, (LPCSTR) lp, len, &s)))
            return FALSE;
         break;
      case DST_ICON:
         if (!IconSize((HICON) lp, &s))
            return FALSE;
         break;
      case DST_BITMAP:
         if (!GetObjectA((HBITMAP) lp, sizeof(bm), &bm))
            return FALSE;
         s.cx = bm.bmWidth;
         s.cy = bm.bmHeight;
         break;
      case DST_COMPLEX:
         return FALSE;
      }
      if (!cx)
         cx = s.cx;
      if (!cy)
         cy = s.cy;
   }
   RECT rc = {x, y, x + cx, y + cy};
   if (flags & DSS_RIGHT)
      dtflags |= DT_RIGHT;
   if (opcode == DST_TEXT)
      dtflags |= DT_NOPREFIX;
   if ((flags & 0x7ff0) == DSS_NORMAL)
      return DrawStateJam(hdc, opcode, func, lp, len, &rc, dtflags, unicode);

   // Other states draw the image in black on white into a monochrome bitmap, then
   // paint its black pixels with a brush.
   COLORREF fg = SetTextColor(hdc, RGB(0, 0, 0)), bg = SetBkColor(hdc, RGB(255, 255, 255));
   BOOL retval = FALSE;
   HBITMAP hbm = CreateBitmap(cx, cy, 1, 1, nullptr);
   HDC memdc = CreateCompatibleDC(hdc);
   HGDIOBJ hbmsave = SelectObject(memdc, hbm), hbsave = nullptr;
   HBRUSH shadow = nullptr;
   HGDIOBJ hfsave;
   BOOL tmp;
   SetRect(&rc, 0, 0, cx, cy);
   FillRect(memdc, &rc, (HBRUSH) GetStockObject(WHITE_BRUSH));
   SetBkColor(memdc, RGB(255, 255, 255));
   SetTextColor(memdc, RGB(0, 0, 0));
   hfsave = SelectObject(memdc, GetCurrentObject(hdc, OBJ_FONT));
   if (!hfsave && opcode <= DST_PREFIXTEXT)
      goto cleanup;
   tmp = DrawStateJam(memdc, opcode, func, lp, len, &rc, dtflags, unicode);
   if (hfsave)
      SelectObject(memdc, hfsave);
   if (!tmp)
      goto cleanup;
   if (flags & DSS_UNION)
   {
      HGDIOBJ old = SelectObject(memdc, Brush55AA());
      PatBlt(memdc, 0, 0, cx, cy, 0x00FA0089);
      SelectObject(memdc, old);
   }
   if (flags & DSS_DISABLED)
      shadow = GetSysColorBrush(COLOR_3DHILIGHT);
   else if (flags & DSS_DEFAULT)
      shadow = GetSysColorBrush(COLOR_3DSHADOW);
   if (flags & (DSS_DISABLED | DSS_DEFAULT))
   {
      hbsave = SelectObject(hdc, shadow);
      BitBlt(hdc, x + 1, y + 1, cx, cy, memdc, 0, 0, 0x00B8074A);
      SelectObject(hdc, hbsave);
   }
   if (flags & DSS_DISABLED)
      hbr = GetSysColorBrush(COLOR_3DSHADOW);
   else if (!hbr)
      hbr = (HBRUSH) GetStockObject(BLACK_BRUSH);
   hbsave = SelectObject(hdc, hbr);
   retval = BitBlt(hdc, x, y, cx, cy, memdc, 0, 0, 0x00B8074A);
   SelectObject(hdc, hbsave);
cleanup:
   SetTextColor(hdc, fg);
   SetBkColor(hdc, bg);
   SelectObject(memdc, hbmsave);
   DeleteObject(hbm);
   DeleteDC(memdc);
   return retval;
}

}  // namespace

}  // namespace w32

using namespace w32;

BOOL WINAPI DrawEdge(HDC hdc, RECT *rc, UINT edge, UINT flags)
{
   if (rc == nullptr)
      return FALSE;
   if (flags & BF_DIAGONAL)
      return DrawDiagEdge(hdc, rc, edge, flags);
   return DrawRectEdge(hdc, rc, edge, flags, 1);
}

BOOL WINAPI DrawFrameControl(HDC hdc, RECT *rc, UINT type, UINT state)
{
   if (rc == nullptr)
      return FALSE;
   switch (type)
   {
   case DFC_BUTTON: return FrameButton(hdc, rc, state);
   case DFC_CAPTION: return FrameCaption(hdc, rc, state);
   case DFC_MENU: return FrameMenu(hdc, rc, state);
   case DFC_SCROLL: return FrameScroll(hdc, rc, state);
   }
   return FALSE;
}

BOOL WINAPI DrawStateA(HDC hdc, HBRUSH hbr, DRAWSTATEPROC func, LPARAM ldata, WPARAM wdata, INT x, INT y, INT cx,
                       INT cy, UINT flags)
{
   return DrawStateCommon(hdc, hbr, func, ldata, wdata, x, y, cx, cy, flags, false);
}

BOOL WINAPI DrawStateW(HDC hdc, HBRUSH hbr, DRAWSTATEPROC func, LPARAM ldata, WPARAM wdata, INT x, INT y, INT cx,
                       INT cy, UINT flags)
{
   return DrawStateCommon(hdc, hbr, func, ldata, wdata, x, y, cx, cy, flags, true);
}

// GrayString: the text (or what proc draws) dithered gray, as DrawState with
// DSS_DISABLED would look in the classic scheme's GrayString.
static BOOL GrayStringCommon(HDC hdc, HBRUSH hbr, GRAYSTRINGPROC proc, LPARAM data, INT count, INT x, INT y, INT cx,
                             INT cy, bool unicode)
{
   if (proc == nullptr)
   {
      if (count == 0)
         count = unicode ? lstrlenW((LPCWSTR) data) : (INT) strlen((LPCSTR) data);
      if (cx == 0 || cy == 0)
      {
         SIZE s;
         if (!(unicode ? GetTextExtentPoint32W(hdc, (LPCWSTR) data, count, &s)
                       : GetTextExtentPoint32A(hdc, (LPCSTR) data, count, &s)))
            return FALSE;
         cx = cx ? cx : s.cx;
         cy = cy ? cy : s.cy;
      }
      UINT flags = DST_TEXT | DSS_UNION;
      return unicode ? DrawStateW(hdc, hbr, nullptr, data, count, x, y, cx, cy, flags)
                     : DrawStateA(hdc, hbr, nullptr, data, count, x, y, cx, cy, flags);
   }
   return DrawStateW(hdc, hbr, (DRAWSTATEPROC) proc, data, count, x, y, cx, cy, DST_COMPLEX | DSS_UNION);
}

BOOL WINAPI GrayStringA(HDC hdc, HBRUSH hbr, GRAYSTRINGPROC proc, LPARAM data, INT count, INT x, INT y, INT cx, INT cy)
{
   return GrayStringCommon(hdc, hbr, proc, data, count, x, y, cx, cy, false);
}

BOOL WINAPI GrayStringW(HDC hdc, HBRUSH hbr, GRAYSTRINGPROC proc, LPARAM data, INT count, INT x, INT y, INT cx, INT cy)
{
   return GrayStringCommon(hdc, hbr, proc, data, count, x, y, cx, cy, true);
}
