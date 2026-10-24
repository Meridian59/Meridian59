/*
 * gdi_raster.cpp:  Drawing on raster targets: fills, lines, shapes, blits, and
 *   the bitmap functions.  8-bit targets get palette indices (the nearest color
 *   in their table), 32-bit ones BGRA; only the pixels drawn are touched.
 */

#include "core/gdi.h"

#include "core/native.h"
#include "core/rle.h"
#include "core/window.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace w32 {

namespace {

inline uint32_t ReadPixel(const RasterTarget &t, int x, int y)
{
   uint8_t *row = t.Row(y);
   switch (t.bpp)
   {
   case 32: return ((uint32_t *) row)[x];
   case 24: return 0xFF000000u | (row[x * 3 + 2] << 16) | (row[x * 3 + 1] << 8) | row[x * 3];
   case 8: return row[x];
   case 4: return (row[x >> 1] >> ((x & 1) ? 0 : 4)) & 0xF;
   case 1: return (row[x >> 3] >> (7 - (x & 7))) & 1;
   }
   return 0;
}

inline void WritePixel(const RasterTarget &t, int x, int y, uint32_t v)
{
   uint8_t *row = t.Row(y);
   switch (t.bpp)
   {
   case 32: ((uint32_t *) row)[x] = v; break;
   case 24: row[x * 3] = v & 0xFF; row[x * 3 + 1] = (v >> 8) & 0xFF; row[x * 3 + 2] = (v >> 16) & 0xFF; break;
   case 8: row[x] = (uint8_t) v; break;
   case 4: row[x >> 1] = (uint8_t) ((x & 1) ? ((row[x >> 1] & 0xF0) | (v & 0xF)) : ((row[x >> 1] & 0x0F) | ((v & 0xF) << 4))); break;
   case 1:
      if (v & 1)
         row[x >> 3] |= 0x80 >> (x & 7);
      else
         row[x >> 3] &= ~(0x80 >> (x & 7));
      break;
   }
}

// A pixel as RGB, whatever the target's format.
inline COLORREF PixelRgb(const RasterTarget &t, uint32_t v)
{
   if (t.bpp >= 24)
      return RGB((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
   if (t.colors && (int) v < t.num_colors)
      return RGB(t.colors[v].rgbRed, t.colors[v].rgbGreen, t.colors[v].rgbBlue);
   return v ? RGB(255, 255, 255) : 0;
}

// Combines a source pixel with the destination by a binary raster op (pen draws).
inline uint32_t Rop2(int rop, uint32_t src, uint32_t dst, uint32_t mask)
{
   switch (rop)
   {
   case R2_BLACK: return 0 & mask;
   case R2_WHITE: return mask;
   case R2_NOP: return dst;
   case R2_NOT: return ~dst & mask;
   case R2_XORPEN: return (src ^ dst) & mask;
   case R2_NOTXORPEN: return ~(src ^ dst) & mask;
   case R2_MERGEPEN: return (src | dst) & mask;
   case R2_MASKPEN: return (src & dst) & mask;
   case R2_NOTCOPYPEN: return ~src & mask;
   default: return src;
   }
}

// A ternary raster operation (the ROP's index byte) on pattern, source and
// destination bits: bit (P << 2 | S << 1 | D) of the index is the result.
inline uint32_t Rop3(DWORD rop, uint32_t p, uint32_t s, uint32_t d)
{
   uint32_t index = (rop >> 16) & 0xFF, out = 0;
   for (int i = 0; i < 8; i++)
      if (index & (1u << i))
         out |= ((i & 4) ? p : ~p) & ((i & 2) ? s : ~s) & ((i & 1) ? d : ~d);
   return out;
}

inline uint32_t ColorMask(const RasterTarget &t)
{
   return t.bpp >= 24 ? 0x00FFFFFFu : t.bpp == 8 ? 0xFFu : t.bpp == 4 ? 0xFu : 1u;
}

// A target and the region to draw in, for one operation on a DC.
struct Canvas
{
   DC *dc = nullptr;
   RasterTarget t;
   Region clip;
   RECT drawn = {0, 0, 0, 0};       // union of what changed, target coordinates
   bool ok = false;

   explicit Canvas(HDC hdc)
   {
      dc = GetDCObject(hdc);
      if (dc == nullptr || dc->info_only || !GetTarget(dc, &t))
         return;
      clip = EffectiveClip(dc, t);
      ok = !clip.Empty();
   }

   ~Canvas()
   {
      if (ok && !IsRectEmpty(&drawn))
         ShowTargetChanges(dc, drawn);
   }

   // Fills the span [x0, x1) of row y (target coordinates) through the clip.
   template <class F> void Span(int y, int x0, int x1, F pixel)
   {
      if (x0 >= x1)
         return;
      for (const RECT &r : clip.Rects())
      {
         if (y < r.top || y >= r.bottom)
            continue;
         int a = std::max(x0, (int) r.left), b = std::min(x1, (int) r.right);
         for (int x = a; x < b; x++)
            pixel(x, y);
         if (a < b)
         {
            RECT s = {a, y, b, y + 1};
            UnionRect(&drawn, &drawn, &s);
         }
      }
   }

   void Plot(int x, int y, uint32_t v, int rop)
   {
      Span(y, x, x + 1, [&](int px, int py) {
         WritePixel(t, px, py, rop == R2_COPYPEN ? v : Rop2(rop, v, ReadPixel(t, px, py), ColorMask(t)) |
                                                       (t.bpp == 32 ? 0xFF000000u : 0));
      });
   }
};

// The brush's pixel at (x, y) (target coordinates), or false for a hollow brush.
bool BrushPixel(DC *dc, const RasterTarget &t, GdiBrush *b, int x, int y, uint32_t *v)
{
   if (b == nullptr || b->style == BS_NULL)
      return false;
   if (b->style == BS_PATTERN)
   {
      GdiBitmap *p = GetGdiObjectAs<GdiBitmap>(b->pattern, OBJ_BITMAP);
      if (p == nullptr || p->width == 0 || p->height == 0)
         return false;
      RasterTarget pt = {p->bits, p->width, p->height, p->stride, p->bpp, p->bottom_up,
                         p->colors.empty() ? nullptr : p->colors.data(), (int) p->colors.size()};
      int px = ((x - dc->origin.x) % p->width + p->width) % p->width;
      int py = ((y - dc->origin.y) % p->height + p->height) % p->height;
      COLORREF c = (p->bpp == 1) ? (ReadPixel(pt, px, py) ? dc->bk_color : dc->text_color)
                                 : PixelRgb(pt, ReadPixel(pt, px, py));
      *v = PixelValue(t, ResolveColor(dc, c));
      return true;
   }
   if (b->style == BS_HATCHED)
   {
      int lx = x - dc->origin.x, ly = y - dc->origin.y;
      bool on = false;
      switch (b->hatch)
      {
      case HS_HORIZONTAL: on = (ly & 7) == 0; break;
      case HS_VERTICAL: on = (lx & 7) == 0; break;
      case HS_FDIAGONAL: on = ((lx - ly) & 7) == 0; break;
      case HS_BDIAGONAL: on = ((lx + ly) & 7) == 0; break;
      case HS_CROSS: on = (lx & 7) == 0 || (ly & 7) == 0; break;
      case HS_DIAGCROSS: on = ((lx - ly) & 7) == 0 || ((lx + ly) & 7) == 0; break;
      }
      if (!on && dc->bk_mode == TRANSPARENT)
         return false;
      *v = PixelValue(t, ResolveColor(dc, on ? b->color : dc->bk_color));
      return true;
   }
   COLORREF color = b->sys_color >= 0 ? GetSysColor(b->sys_color) : b->color;
   *v = PixelValue(t, ResolveColor(dc, color));
   return true;
}

void FillRectWith(Canvas &c, const RECT &r, GdiBrush *b)
{
   if (b == nullptr || b->style == BS_NULL)
      return;
   bool solid = b->style == BS_SOLID;
   uint32_t v = 0;
   if (solid && !BrushPixel(c.dc, c.t, b, 0, 0, &v))
      return;
   for (int y = r.top; y < r.bottom; y++)
      c.Span(y, r.left, r.right, [&](int x, int py) {
         uint32_t p = v;
         if (solid || BrushPixel(c.dc, c.t, b, x, py, &p))
            WritePixel(c.t, x, py, p);
      });
}

GdiPen *Pen(DC *dc)
{
   return GetGdiObjectAs<GdiPen>(dc->pen, OBJ_PEN);
}

GdiBrush *Brush(DC *dc)
{
   return GetGdiObjectAs<GdiBrush>(dc->brush, OBJ_BRUSH);
}

// A line from a to b (target coordinates), the end point excluded as Windows does.
void DrawLine(Canvas &c, POINT a, POINT b)
{
   GdiPen *pen = Pen(c.dc);
   if (pen == nullptr || pen->style == PS_NULL)
      return;
   uint32_t v = PixelValue(c.t, ResolveColor(c.dc, pen->color));
   int w = std::max(1, pen->width);
   int dx = abs(b.x - a.x), dy = -abs(b.y - a.y);
   int sx = a.x < b.x ? 1 : -1, sy = a.y < b.y ? 1 : -1;
   int err = dx + dy;
   int x = a.x, y = a.y, n = 0;
   static const int dot[] = {1, 1, 1, 1, 0, 0, 0, 0};   // PS_DOT, as Windows' 4-on 4-off
   while (x != b.x || y != b.y)
   {
      bool on = pen->style == PS_SOLID || pen->style == PS_INSIDEFRAME || (pen->style == PS_DOT && dot[n & 7]) ||
                (pen->style == PS_DASH && (n % 24) < 18) || (pen->style == PS_DASHDOT && (n % 24) < 12);
      if (on)
      {
         if (w == 1)
            c.Plot(x, y, v, c.dc->rop2);
         else
            for (int oy = -(w / 2); oy < w - w / 2; oy++)
               for (int ox = -(w / 2); ox < w - w / 2; ox++)
                  c.Plot(x + ox, y + oy, v, c.dc->rop2);
      }
      n++;
      int e2 = 2 * err;
      if (e2 >= dy)
         err += dy, x += sx;
      if (e2 <= dx)
         err += dx, y += sy;
   }
}

// Converts a DC's logical point to target coordinates.
POINT ToTarget(DC *dc, int x, int y)
{
   return {x + dc->origin.x, y + dc->origin.y};
}

// Fills a polygon (target coordinates) with the DC's brush, by scanlines.
void FillPolygon(Canvas &c, const std::vector<POINT> &pts, bool winding)
{
   GdiBrush *b = Brush(c.dc);
   if (b == nullptr || b->style == BS_NULL || pts.size() < 3)
      return;
   int top = pts[0].y, bottom = pts[0].y;
   for (const POINT &p : pts)
      top = std::min(top, (int) p.y), bottom = std::max(bottom, (int) p.y);
   std::vector<std::pair<int, int>> xs;   // crossing x, direction
   for (int y = top; y < bottom; y++)
   {
      xs.clear();
      double sy = y + 0.5;
      for (size_t i = 0; i < pts.size(); i++)
      {
         POINT p = pts[i], q = pts[(i + 1) % pts.size()];
         if ((p.y <= sy && q.y > sy) || (q.y <= sy && p.y > sy))
         {
            double x = p.x + (sy - p.y) * (q.x - p.x) / (double) (q.y - p.y);
            xs.push_back({(int) std::lround(x), q.y > p.y ? 1 : -1});
         }
      }
      std::sort(xs.begin(), xs.end());
      int wind = 0;
      for (size_t i = 0; i + 1 < xs.size(); i++)
      {
         wind += xs[i].second;
         bool inside = winding ? wind != 0 : (i % 2) == 0;
         if (inside)
         {
            RECT r = {xs[i].first, y, xs[i + 1].first, y + 1};
            FillRectWith(c, r, b);
         }
      }
   }
}

void ShowWindowChanges(DC *dc, const RECT &r)
{
   Window *top = GetWindowObject(TopLevelOf(dc->hwnd));
   if (top && top->top)
      native::Present(top->top, r);
}

}  // namespace

void ShowTargetChanges(DC *dc, const RECT &r)
{
   if (dc->hwnd)
      ShowWindowChanges(dc, r);
}

}  // namespace w32

using namespace w32;

/************************************************************************/
// Fills and shapes

INT WINAPI FillRect(HDC hdc, const RECT *rect, HBRUSH brush)
{
   Canvas c(hdc);
   if (!c.ok || rect == nullptr)
      return 0;
   // A brush handle, or COLOR_* + 1 (a system color).
   GdiBrush sys;
   GdiBrush *b = GetGdiObjectAs<GdiBrush>(brush, OBJ_BRUSH);
   if (b == nullptr && (UINT_PTR) brush > 0 && (UINT_PTR) brush <= COLOR_MENUBAR + 1)
   {
      sys.color = GetSysColor((INT) (UINT_PTR) brush - 1);
      b = &sys;
   }
   RECT r = *rect;
   OffsetRect(&r, c.dc->origin.x, c.dc->origin.y);
   FillRectWith(c, r, b);
   return 1;
}

INT WINAPI FrameRect(HDC hdc, const RECT *rect, HBRUSH brush)
{
   if (rect == nullptr || IsRectEmpty(rect))
      return 0;
   RECT r = *rect;
   RECT sides[4] = {{r.left, r.top, r.right, r.top + 1}, {r.left, r.bottom - 1, r.right, r.bottom},
                    {r.left, r.top, r.left + 1, r.bottom}, {r.right - 1, r.top, r.right, r.bottom}};
   for (RECT &s : sides)
      FillRect(hdc, &s, brush);
   return 1;
}

BOOL WINAPI InvertRect(HDC hdc, const RECT *rect)
{
   Canvas c(hdc);
   if (!c.ok || rect == nullptr)
      return FALSE;
   RECT r = *rect;
   OffsetRect(&r, c.dc->origin.x, c.dc->origin.y);
   uint32_t mask = ColorMask(c.t);
   for (int y = r.top; y < r.bottom; y++)
      c.Span(y, r.left, r.right, [&](int x, int py) {
         WritePixel(c.t, x, py, (~ReadPixel(c.t, x, py) & mask) | (c.t.bpp == 32 ? 0xFF000000u : 0));
      });
   return TRUE;
}

BOOL WINAPI DrawFocusRect(HDC hdc, const RECT *rect)
{
   // Dotted XOR rectangle, every other pixel.
   Canvas c(hdc);
   if (!c.ok || rect == nullptr)
      return FALSE;
   RECT r = *rect;
   OffsetRect(&r, c.dc->origin.x, c.dc->origin.y);
   uint32_t mask = ColorMask(c.t);
   auto flip = [&](int x, int y) {
      c.Span(y, x, x + 1, [&](int px, int py) {
         WritePixel(c.t, px, py, (~ReadPixel(c.t, px, py) & mask) | (c.t.bpp == 32 ? 0xFF000000u : 0));
      });
   };
   for (int x = r.left; x < r.right; x += 2)
      flip(x, r.top), flip(x, r.bottom - 1);
   for (int y = r.top + 2; y < r.bottom - 1; y += 2)
      flip(r.left, y), flip(r.right - 1, y);
   return TRUE;
}

BOOL WINAPI PatBlt(HDC hdc, INT x, INT y, INT width, INT height, DWORD rop)
{
   Canvas c(hdc);
   if (!c.ok)
      return FALSE;
   RECT r = {x, y, x + width, y + height};
   if (width < 0)
      std::swap(r.left, r.right);
   if (height < 0)
      std::swap(r.top, r.bottom);
   OffsetRect(&r, c.dc->origin.x, c.dc->origin.y);
   uint32_t mask = ColorMask(c.t);
   uint32_t alpha = c.t.bpp == 32 ? 0xFF000000u : 0;
   for (int py = r.top; py < r.bottom; py++)
      c.Span(py, r.left, r.right, [&](int px, int yy) {
         uint32_t dst = ReadPixel(c.t, px, yy);
         uint32_t pat = 0;
         bool has_pattern = BrushPixel(c.dc, c.t, Brush(c.dc), px, yy, &pat);
         uint32_t out;
         switch (rop)
         {
         case BLACKNESS: out = PixelValue(c.t, 0); break;
         case WHITENESS: out = PixelValue(c.t, RGB(255, 255, 255)); break;
         case DSTINVERT: out = (~dst & mask) | alpha; break;
         case PATINVERT: out = has_pattern ? ((pat ^ dst) & mask) | alpha : dst; break;
         case PATCOPY: out = has_pattern ? pat : dst; break;
         default:
            if (!has_pattern)   // a hollow brush draws nothing
               return;
            out = (Rop3(rop, pat, 0, dst) & mask) | alpha;
            break;
         }
         WritePixel(c.t, px, yy, out);
      });
   return TRUE;
}

BOOL WINAPI Rectangle(HDC hdc, INT left, INT top, INT right, INT bottom)
{
   Canvas c(hdc);
   if (!c.ok)
      return FALSE;
   POINT a = ToTarget(c.dc, left, top), b = ToTarget(c.dc, right, bottom);
   GdiPen *pen = Pen(c.dc);
   bool outline = pen && pen->style != PS_NULL;
   RECT inner = {a.x + (outline ? 1 : 0), a.y + (outline ? 1 : 0), b.x - (outline ? 1 : 0), b.y - (outline ? 1 : 0)};
   FillRectWith(c, inner, Brush(c.dc));
   if (outline)
   {
      DrawLine(c, {a.x, a.y}, {b.x - 1, a.y});
      DrawLine(c, {b.x - 1, a.y}, {b.x - 1, b.y - 1});
      DrawLine(c, {b.x - 1, b.y - 1}, {a.x, b.y - 1});
      DrawLine(c, {a.x, b.y - 1}, {a.x, a.y});
   }
   return TRUE;
}

BOOL WINAPI Ellipse(HDC hdc, INT left, INT top, INT right, INT bottom)
{
   Canvas c(hdc);
   if (!c.ok)
      return FALSE;
   POINT a = ToTarget(c.dc, left, top), b = ToTarget(c.dc, right, bottom);
   double cx = (a.x + b.x - 1) / 2.0, cy = (a.y + b.y - 1) / 2.0;
   double rx = (b.x - a.x - 1) / 2.0, ry = (b.y - a.y - 1) / 2.0;
   if (rx < 0 || ry < 0)
      return TRUE;
   GdiPen *pen = Pen(c.dc);
   bool outline = pen && pen->style != PS_NULL;
   uint32_t pv = outline ? PixelValue(c.t, ResolveColor(c.dc, pen->color)) : 0;
   GdiBrush *br = Brush(c.dc);
   for (int y = a.y; y < b.y; y++)
   {
      double dy = (y - cy) / (ry > 0 ? ry : 1);
      if (fabs(dy) > 1.0)
         continue;
      double half = rx * sqrt(std::max(0.0, 1.0 - dy * dy));
      int x0 = (int) std::lround(cx - half), x1 = (int) std::lround(cx + half);
      RECT span = {x0 + (outline ? 1 : 0), y, x1 + 1 - (outline ? 1 : 0), y + 1};
      FillRectWith(c, span, br);
      if (outline)
      {
         c.Plot(x0, y, pv, c.dc->rop2);
         c.Plot(x1, y, pv, c.dc->rop2);
      }
   }
   if (outline)
      // Close the top and bottom where the outline runs nearly horizontal.
      for (int x = a.x; x < b.x; x++)
      {
         double dx = (x - cx) / (rx > 0 ? rx : 1);
         if (fabs(dx) > 1.0)
            continue;
         double half = ry * sqrt(std::max(0.0, 1.0 - dx * dx));
         c.Plot(x, (int) std::lround(cy - half), pv, c.dc->rop2);
         c.Plot(x, (int) std::lround(cy + half), pv, c.dc->rop2);
      }
   return TRUE;
}

BOOL WINAPI Polygon(HDC hdc, const POINT *points, INT count)
{
   Canvas c(hdc);
   if (!c.ok || points == nullptr || count < 2)
      return FALSE;
   std::vector<POINT> pts;
   for (INT i = 0; i < count; i++)
      pts.push_back(ToTarget(c.dc, points[i].x, points[i].y));
   FillPolygon(c, pts, false);
   for (INT i = 0; i < count; i++)
      DrawLine(c, pts[i], pts[(i + 1) % count]);
   return TRUE;
}

BOOL WINAPI Polyline(HDC hdc, const POINT *points, INT count)
{
   Canvas c(hdc);
   if (!c.ok || points == nullptr)
      return FALSE;
   for (INT i = 0; i + 1 < count; i++)
      DrawLine(c, ToTarget(c.dc, points[i].x, points[i].y), ToTarget(c.dc, points[i + 1].x, points[i + 1].y));
   return TRUE;
}

BOOL WINAPI MoveToEx(HDC hdc, INT x, INT y, POINT *old)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return FALSE;
   if (old)
      *old = dc->pos;
   dc->pos = {x, y};
   return TRUE;
}

BOOL WINAPI LineTo(HDC hdc, INT x, INT y)
{
   Canvas c(hdc);
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return FALSE;
   if (c.ok)
      DrawLine(c, ToTarget(dc, dc->pos.x, dc->pos.y), ToTarget(dc, x, y));
   dc->pos = {x, y};
   return TRUE;
}

COLORREF WINAPI SetPixel(HDC hdc, INT x, INT y, COLORREF color)
{
   Canvas c(hdc);
   if (!c.ok)
      return CLR_INVALID;
   POINT p = ToTarget(c.dc, x, y);
   COLORREF rgb = ResolveColor(c.dc, color);
   c.Plot(p.x, p.y, PixelValue(c.t, rgb), R2_COPYPEN);
   return rgb;
}

COLORREF WINAPI GetPixel(HDC hdc, INT x, INT y)
{
   DC *dc = GetDCObject(hdc);
   RasterTarget t;
   if (dc == nullptr || !GetTarget(dc, &t))
      return CLR_INVALID;
   POINT p = ToTarget(dc, x, y);
   if (p.x < 0 || p.y < 0 || p.x >= t.width || p.y >= t.height)
      return CLR_INVALID;
   return PixelRgb(t, ReadPixel(t, p.x, p.y));
}

BOOL WINAPI FillRgn(HDC hdc, HRGN hrgn, HBRUSH brush)
{
   Region *r = GetRegion(hrgn);
   if (r == nullptr)
      return FALSE;
   for (const RECT &rect : r->Rects())
      FillRect(hdc, &rect, brush);
   return TRUE;
}

/************************************************************************/
// Blits

namespace {

// Copies src (sx, sy, sw, sh) onto dst (dx, dy, dw, dh), target coordinates of both,
// scaling by nearest pixel, combining by rop.
void Blit(Canvas &dst, const RasterTarget &src, DC *src_dc, int dx, int dy, int dw, int dh, int sx, int sy,
          int sw, int sh, DWORD rop)
{
   if (dw == 0 || dh == 0 || sw == 0 || sh == 0)
      return;
   bool flip_x = (dw < 0) != (sw < 0), flip_y = (dh < 0) != (sh < 0);
   if (dw < 0)
      dx += dw, dw = -dw;
   if (dh < 0)
      dy += dh, dh = -dh;
   if (sw < 0)
      sx += sw, sw = -sw;
   if (sh < 0)
      sy += sh, sh = -sh;

   // Source pixel -> destination pixel: same format copies raw; 8-bit to 8-bit
   // with the same color table copies indices; otherwise through RGB.
   bool same = src.bpp == dst.t.bpp &&
               ((src.bpp != 8 && src.bpp != 4) || (src.colors == dst.t.colors) ||
                (src.num_colors == dst.t.num_colors && src.colors && dst.t.colors &&
                 memcmp(src.colors, dst.t.colors, src.num_colors * sizeof(RGBQUAD)) == 0));
   // 8-bit sources into 8-bit targets with different tables: map each source
   // index once.
   uint32_t map[256];
   bool use_map = !same && (src.bpp == 8 || src.bpp == 4) && (dst.t.bpp == 8 || dst.t.bpp == 4) && src.colors;
   if (use_map)
      for (int i = 0; i < 256; i++)
         map[i] = i < src.num_colors ? PixelValue(dst.t, RGB(src.colors[i].rgbRed, src.colors[i].rgbGreen,
                                                             src.colors[i].rgbBlue))
                                     : 0;
   // Monochrome sources take the destination DC's text/background colors.
   uint32_t mono_fg = PixelValue(dst.t, ResolveColor(dst.dc, dst.dc->text_color));
   uint32_t mono_bg = PixelValue(dst.t, ResolveColor(dst.dc, dst.dc->bk_color));
   uint32_t mask = ColorMask(dst.t);
   uint32_t alpha = dst.t.bpp == 32 ? 0xFF000000u : 0;
   GdiBrush *pattern_brush = Brush(dst.dc);
   (void) src_dc;

   auto convert = [&](uint32_t s) -> uint32_t {
      if (same)
         return s;
      if (use_map)
         return map[s & 0xFF];
      if (src.bpp == 1)
         return s ? mono_bg : mono_fg;
      if (dst.t.bpp == 1)
      {
         // To monochrome: pixels matching the source DC's background become 1.
         COLORREF bk = src_dc ? ResolveColor(src_dc, src_dc->bk_color) : RGB(255, 255, 255);
         return PixelRgb(src, s) == bk ? 1 : 0;
      }
      return PixelValue(dst.t, PixelRgb(src, s));
   };

   for (int y = 0; y < dh; y++)
   {
      int syy = sy + (int) ((int64_t) (flip_y ? dh - 1 - y : y) * sh / dh);
      if (syy < 0 || syy >= src.height)
         continue;
      int ty = dy + y;
      dst.Span(ty, dx, dx + dw, [&](int tx, int yy) {
         int x = tx - dx;
         int sxx = sx + (int) ((int64_t) (flip_x ? dw - 1 - x : x) * sw / dw);
         if (sxx < 0 || sxx >= src.width)
            return;
         uint32_t s = convert(ReadPixel(src, sxx, syy));
         uint32_t out;
         switch (rop)
         {
         case SRCCOPY: out = s; break;
         case SRCPAINT: out = ((s | ReadPixel(dst.t, tx, yy)) & mask) | alpha; break;
         case SRCAND: out = ((s & ReadPixel(dst.t, tx, yy)) & mask) | alpha; break;
         case SRCINVERT: out = ((s ^ ReadPixel(dst.t, tx, yy)) & mask) | alpha; break;
         case NOTSRCCOPY: out = (~s & mask) | alpha; break;
         case SRCERASE: out = ((s & ~ReadPixel(dst.t, tx, yy)) & mask) | alpha; break;
         case NOTSRCERASE: out = (~(s | ReadPixel(dst.t, tx, yy)) & mask) | alpha; break;
         case MERGECOPY:
         {
            uint32_t p = 0;
            BrushPixel(dst.dc, dst.t, pattern_brush, tx, yy, &p);
            out = ((s & p) & mask) | alpha;
            break;
         }
         case MERGEPAINT: out = ((~s | ReadPixel(dst.t, tx, yy)) & mask) | alpha; break;
         case DSTINVERT: out = (~ReadPixel(dst.t, tx, yy) & mask) | alpha; break;
         default:
         {
            uint32_t p = 0;
            BrushPixel(dst.dc, dst.t, pattern_brush, tx, yy, &p);
            out = (Rop3(rop, p, s, ReadPixel(dst.t, tx, yy)) & mask) | alpha;
            break;
         }
         }
         if (dst.t.bpp == 32)
            out |= 0xFF000000u;
         WritePixel(dst.t, tx, yy, out);
      });
   }
}

}  // namespace

BOOL WINAPI StretchBlt(HDC dest, INT dx, INT dy, INT dw, INT dh, HDC source, INT sx, INT sy, INT sw, INT sh, DWORD rop)
{
   Canvas c(dest);
   if (!c.ok)
      return GetDCObject(dest) != nullptr;
   // Raster ops without a source.
   if (rop == PATCOPY || rop == PATINVERT || rop == BLACKNESS || rop == WHITENESS || rop == DSTINVERT)
      return PatBlt(dest, dx, dy, dw, dh, rop);
   DC *sdc = GetDCObject(source);
   RasterTarget s;
   if (sdc == nullptr || !GetTarget(sdc, &s))
      return FALSE;
   Blit(c, s, sdc, dx + c.dc->origin.x, dy + c.dc->origin.y, dw, dh, sx + sdc->origin.x, sy + sdc->origin.y, sw, sh, rop);
   return TRUE;
}

BOOL WINAPI BitBlt(HDC dest, INT dx, INT dy, INT width, INT height, HDC source, INT sx, INT sy, DWORD rop)
{
   return StretchBlt(dest, dx, dy, width, height, source, sx, sy, width, height, rop);
}

/************************************************************************/
// Bitmaps

namespace {

GdiBitmap *NewBitmap(int width, int height, int bpp, bool bottom_up)
{
   auto *b = new GdiBitmap;
   b->width = width;
   b->height = height;
   b->bpp = bpp;
   b->stride = ((width * bpp + 31) / 32) * 4;
   b->bottom_up = bottom_up;
   b->bits = new uint8_t[(size_t) b->stride * std::max(height, 1)]();
   return b;
}

void SetColorTable(GdiBitmap *b, const BITMAPINFO *bi, UINT usage, HDC hdc)
{
   if (b->bpp > 8)
      return;
   int n = bi->bmiHeader.biClrUsed ? bi->bmiHeader.biClrUsed : (1 << b->bpp);
   n = std::min(n, 1 << b->bpp);
   // The table has biClrUsed entries, as GetDIBColorTable reports them.
   b->colors.resize(n);
   if (usage == DIB_PAL_COLORS)
   {
      // Indices into the DC's palette.
      const WORD *indices = (const WORD *) bi->bmiColors;
      DC *dc = GetDCObject(hdc);
      GdiPalette *pal = dc ? GetGdiObjectAs<GdiPalette>(dc->palette, OBJ_PAL) : nullptr;
      for (int i = 0; i < n; i++)
         if (pal && indices[i] < pal->entries.size())
         {
            const PALETTEENTRY &e = pal->entries[indices[i]];
            b->colors[i] = {e.peBlue, e.peGreen, e.peRed, 0};
         }
      return;
   }
   for (int i = 0; i < n; i++)
      b->colors[i] = bi->bmiColors[i];
}

}  // namespace

HBITMAP WINAPI CreateDIBSection(HDC hdc, const BITMAPINFO *bi, UINT usage, void **bits, HANDLE section, DWORD offset)
{
   if (bi == nullptr || bi->bmiHeader.biWidth <= 0 || bi->bmiHeader.biHeight == 0 || section != nullptr)
      return nullptr;
   int bpp = bi->bmiHeader.biBitCount;
   if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 24 && bpp != 32)
      return nullptr;
   bool bottom_up = bi->bmiHeader.biHeight > 0;
   GdiBitmap *b = NewBitmap(bi->bmiHeader.biWidth, abs(bi->bmiHeader.biHeight), bpp, bottom_up);
   SetColorTable(b, bi, usage, hdc);
   if (bits)
      *bits = b->bits;
   (void) offset;
   return (HBITMAP) AddGdiObject(b);
}

HBITMAP WINAPI CreateCompatibleBitmap(HDC hdc, INT width, INT height)
{
   DC *dc = GetDCObject(hdc);
   // A memory DC's compatible bitmap has its selected bitmap's format (1 bpp for a
   // new DC, as Windows'); window and screen DCs give 32 bpp.
   int bpp = 32;
   GdiBitmap *sel = dc && dc->memory ? GetGdiObjectAs<GdiBitmap>(dc->bitmap, OBJ_BITMAP) : nullptr;
   if (sel)
      bpp = sel->bpp;
   GdiBitmap *b = NewBitmap(std::max(width, 1), std::max(height, 1), bpp, false);
   if (sel)
      b->colors = sel->colors;
   return (HBITMAP) AddGdiObject(b);
}

HBITMAP WINAPI CreateBitmap(INT width, INT height, UINT planes, UINT bpp, const void *bits)
{
   if (planes != 1 || (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 24 && bpp != 32))
      return nullptr;
   GdiBitmap *b = NewBitmap(std::max(width, 1), std::max(height, 1), bpp, false);
   if (bpp == 1)
      b->colors = {{0, 0, 0, 0}, {255, 255, 255, 0}};
   if (bits)
   {
      // CreateBitmap rows are WORD-aligned.
      int src_stride = ((width * bpp + 15) / 16) * 2;
      for (int y = 0; y < height; y++)
         memcpy(b->bits + (size_t) y * b->stride, (const uint8_t *) bits + (size_t) y * src_stride,
                std::min(src_stride, b->stride));
   }
   return (HBITMAP) AddGdiObject(b);
}

UINT WINAPI SetDIBColorTable(HDC hdc, UINT start, UINT count, const RGBQUAD *colors)
{
   DC *dc = GetDCObject(hdc);
   GdiBitmap *b = dc ? GetGdiObjectAs<GdiBitmap>(dc->bitmap, OBJ_BITMAP) : nullptr;
   if (b == nullptr || b->bpp > 8 || colors == nullptr)
      return 0;
   UINT n = 0;
   for (UINT i = start; i < start + count && i < b->colors.size(); i++, n++)
      b->colors[i] = colors[n];
   return n;
}

UINT WINAPI GetDIBColorTable(HDC hdc, UINT start, UINT count, RGBQUAD *colors)
{
   DC *dc = GetDCObject(hdc);
   GdiBitmap *b = dc ? GetGdiObjectAs<GdiBitmap>(dc->bitmap, OBJ_BITMAP) : nullptr;
   if (b == nullptr || b->bpp > 8 || colors == nullptr)
      return 0;
   UINT n = 0;
   for (UINT i = start; i < start + count && i < b->colors.size(); i++, n++)
      colors[n] = b->colors[i];
   return n;
}

INT WINAPI StretchDIBits(HDC hdc, INT dx, INT dy, INT dw, INT dh, INT sx, INT sy, INT sw, INT sh, const void *bits,
                         const BITMAPINFO *bi, UINT usage, DWORD rop)
{
   if (bits == nullptr || bi == nullptr)
      return 0;
   if (bi->bmiHeader.biCompression == BI_RLE8 || bi->bmiHeader.biCompression == BI_RLE4)
   {
      std::vector<BYTE> info, pixels;
      if (!DecodeRleDib(bi, bits, &info, &pixels))
         return 0;
      return StretchDIBits(hdc, dx, dy, dw, dh, sx, sy, sw, sh, pixels.data(), (const BITMAPINFO *) info.data(), usage,
                           rop);
   }
   Canvas c(hdc);
   if (!c.ok)
      return 0;
   int bpp = bi->bmiHeader.biBitCount;
   int width = bi->bmiHeader.biWidth, height = abs(bi->bmiHeader.biHeight);
   std::vector<RGBQUAD> colors;
   if (bpp <= 8)
   {
      int n = bi->bmiHeader.biClrUsed ? bi->bmiHeader.biClrUsed : (1 << bpp);
      if (usage == DIB_PAL_COLORS)
      {
         GdiPalette *pal = GetGdiObjectAs<GdiPalette>(c.dc->palette, OBJ_PAL);
         for (int i = 0; i < n; i++)
         {
            WORD index = ((const WORD *) bi->bmiColors)[i];
            PALETTEENTRY e = (pal && index < pal->entries.size()) ? pal->entries[index] : PALETTEENTRY{};
            colors.push_back({e.peBlue, e.peGreen, e.peRed, 0});
         }
      }
      else
         colors.assign(bi->bmiColors, bi->bmiColors + n);
   }
   RasterTarget src = {(uint8_t *) bits, width, height, ((width * bpp + 31) / 32) * 4, bpp,
                       bi->bmiHeader.biHeight > 0, colors.empty() ? nullptr : colors.data(), (int) colors.size()};
   Blit(c, src, nullptr, dx + c.dc->origin.x, dy + c.dc->origin.y, dw, dh, sx, sy, sw, sh, rop);
   return dh;
}

INT WINAPI SetDIBitsToDevice(HDC hdc, INT dx, INT dy, DWORD width, DWORD height, INT sx, INT sy, UINT start,
                             UINT lines, const void *bits, const BITMAPINFO *bi, UINT usage)
{
   // Bottom-up DIBs number source rows from the bottom.
   INT dib_height = abs(bi->bmiHeader.biHeight);
   INT top = bi->bmiHeader.biHeight > 0 ? dib_height - sy - (INT) height : sy;
   (void) start;
   (void) lines;
   return StretchDIBits(hdc, dx, dy, width, height, sx, top, width, height, bits, bi, usage, SRCCOPY);
}
