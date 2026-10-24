/*
 * cc_gdi.cpp:  GDI and USER functions comctl32's controls draw with: the
 *   viewport origin, region constructors, DIB transfers (GetDIBits, SetDIBits),
 *   alpha blending, icons made from bitmaps and drawn (CreateIconIndirect,
 *   GetIconInfo, DrawIconEx), window regions.  Weak, for GDI's owner to replace.
 */

#include "core/cursor.h"
#include "core/gdi.h"
#include "core/rle.h"
#include "core/window.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>

using namespace w32;

#define CC_WEAK __attribute__((weak))

namespace {

// --- Pixels of any target format, as RGB -------------------------------------------

COLORREF ReadRgb(const RasterTarget &t, int x, int y)
{
   const uint8_t *row = t.Row(y);
   uint32_t index;
   switch (t.bpp)
   {
   case 32:
   case 24:
   {
      const uint8_t *p = row + x * (t.bpp / 8);
      return RGB(p[2], p[1], p[0]);
   }
   case 16:
   {
      WORD v = ((const WORD *) row)[x];
      return RGB(((v >> 10) & 0x1F) * 255 / 31, ((v >> 5) & 0x1F) * 255 / 31, (v & 0x1F) * 255 / 31);
   }
   case 8: index = row[x]; break;
   case 4: index = (row[x >> 1] >> ((x & 1) ? 0 : 4)) & 0xF; break;
   case 1: index = (row[x >> 3] >> (7 - (x & 7))) & 1; break;
   default: return 0;
   }
   if (t.colors && (int) index < t.num_colors)
      return RGB(t.colors[index].rgbRed, t.colors[index].rgbGreen, t.colors[index].rgbBlue);
   return index ? RGB(255, 255, 255) : 0;
}

BYTE ReadAlpha(const RasterTarget &t, int x, int y)
{
   return t.bpp == 32 ? t.Row(y)[x * 4 + 3] : 0xFF;
}

uint32_t Nearest(const RasterTarget &t, COLORREF c)
{
   int best = 0, best_d = 1 << 30;
   for (int i = 0; i < t.num_colors; i++)
   {
      int dr = t.colors[i].rgbRed - GetRValue(c), dg = t.colors[i].rgbGreen - GetGValue(c),
          db = t.colors[i].rgbBlue - GetBValue(c);
      int d = dr * dr + dg * dg + db * db;
      if (d < best_d)
         best_d = d, best = i;
   }
   if (t.num_colors == 0 && t.bpp == 1)
      return (GetRValue(c) + GetGValue(c) + GetBValue(c)) / 3 >= 128 ? 1 : 0;
   return (uint32_t) best;
}

// alpha: the 32-bit pixel's fourth byte.
void WriteRgb(const RasterTarget &t, int x, int y, COLORREF c, BYTE alpha = 0xFF)
{
   uint8_t *row = t.Row(y);
   switch (t.bpp)
   {
   case 32:
   case 24:
   {
      uint8_t *p = row + x * (t.bpp / 8);
      p[0] = GetBValue(c);
      p[1] = GetGValue(c);
      p[2] = GetRValue(c);
      if (t.bpp == 32)
         p[3] = alpha;
      return;
   }
   case 16:
      ((WORD *) row)[x] = (WORD) (((GetRValue(c) >> 3) << 10) | ((GetGValue(c) >> 3) << 5) | (GetBValue(c) >> 3));
      return;
   }
   uint32_t v = Nearest(t, c);
   switch (t.bpp)
   {
   case 8: row[x] = (uint8_t) v; break;
   case 4: row[x >> 1] = (uint8_t) ((x & 1) ? ((row[x >> 1] & 0xF0) | v) : ((row[x >> 1] & 0x0F) | (v << 4))); break;
   case 1:
      if (v)
         row[x >> 3] |= 0x80 >> (x & 7);
      else
         row[x >> 3] &= ~(0x80 >> (x & 7));
      break;
   }
}

RasterTarget BitmapTarget(GdiBitmap *b)
{
   return {b->bits, b->width, b->height, b->stride, b->bpp, b->bottom_up,
           b->colors.empty() ? nullptr : b->colors.data(), (int) b->colors.size()};
}

std::vector<RGBQUAD> DibColors(const BITMAPINFO *bi)
{
   std::vector<RGBQUAD> colors;
   int bpp = bi->bmiHeader.biBitCount;
   if (bpp <= 8)
   {
      int n = bi->bmiHeader.biClrUsed ? (int) bi->bmiHeader.biClrUsed : 1 << bpp;
      const RGBQUAD *table = (const RGBQUAD *) ((const BYTE *) bi + bi->bmiHeader.biSize);
      colors.assign(table, table + n);
   }
   return colors;
}

// The color table Windows gives device bitmaps read as 4 or 8 bits: the 16 VGA
// colors; for 8 bits the 20 static system colors at both ends, a 6x6x6 cube and
// grays between.
std::vector<RGBQUAD> DefaultColors(int bpp)
{
   static const RGBQUAD vga[16] = {
      {0, 0, 0, 0},       {0, 0, 0x80, 0},    {0, 0x80, 0, 0},    {0, 0x80, 0x80, 0},
      {0x80, 0, 0, 0},    {0x80, 0, 0x80, 0}, {0x80, 0x80, 0, 0}, {0xC0, 0xC0, 0xC0, 0},
      {0x80, 0x80, 0x80, 0}, {0, 0, 0xFF, 0}, {0, 0xFF, 0, 0},    {0, 0xFF, 0xFF, 0},
      {0xFF, 0, 0, 0},    {0xFF, 0, 0xFF, 0}, {0xFF, 0xFF, 0, 0}, {0xFF, 0xFF, 0xFF, 0}};
   if (bpp == 4)
      return std::vector<RGBQUAD>(vga, vga + 16);
   std::vector<RGBQUAD> t(256);
   static const RGBQUAD low[10] = {
      {0, 0, 0, 0},    {0, 0, 0x80, 0},    {0, 0x80, 0, 0},    {0, 0x80, 0x80, 0},    {0x80, 0, 0, 0},
      {0x80, 0, 0x80, 0}, {0x80, 0x80, 0, 0}, {0xC0, 0xC0, 0xC0, 0}, {0xC0, 0xDC, 0xC0, 0}, {0xF0, 0xCA, 0xA6, 0}};
   static const RGBQUAD high[10] = {
      {0xF0, 0xFB, 0xFF, 0}, {0xA4, 0xA0, 0xA0, 0}, {0x80, 0x80, 0x80, 0}, {0, 0, 0xFF, 0}, {0, 0xFF, 0, 0},
      {0, 0xFF, 0xFF, 0},    {0xFF, 0, 0, 0},       {0xFF, 0, 0xFF, 0},    {0xFF, 0xFF, 0, 0}, {0xFF, 0xFF, 0xFF, 0}};
   for (int i = 0; i < 10; i++)
   {
      t[i] = low[i];
      t[246 + i] = high[i];
   }
   for (int i = 0; i < 216; i++)
      t[10 + i] = {(BYTE) ((i / 36) * 51), (BYTE) (((i / 6) % 6) * 51), (BYTE) ((i % 6) * 51), 0};
   for (int i = 0; i < 20; i++)
      t[226 + i] = {(BYTE) (8 + i * 12), (BYTE) (8 + i * 12), (BYTE) (8 + i * 12), 0};
   return t;
}

// --- Viewport origins (MM_TEXT: an offset added to logical coordinates) ------------

std::map<HDC, POINT> &Viewports()
{
   static std::map<HDC, POINT> viewports;
   return viewports;
}

// --- Polygon scan conversion -------------------------------------------------------

Region PolygonRegion(const POINT *points, const INT *counts, INT polygons, INT mode)
{
   struct Edge
   {
      double x0, y0, x1, y1;
      int dir;
   };
   std::vector<Edge> edges;
   int top = INT32_MAX, bottom = INT32_MIN;
   for (INT p = 0, base = 0; p < polygons; base += counts[p], p++)
      for (INT i = 0; i < counts[p]; i++)
      {
         POINT a = points[base + i], b = points[base + (i + 1) % counts[p]];
         top = std::min({top, (int) a.y, (int) b.y});
         bottom = std::max({bottom, (int) a.y, (int) b.y});
         if (a.y == b.y)
            continue;
         if (a.y < b.y)
            edges.push_back({(double) a.x, (double) a.y, (double) b.x, (double) b.y, 1});
         else
            edges.push_back({(double) b.x, (double) b.y, (double) a.x, (double) a.y, -1});
      }
   Region r;
   for (int y = top; y < bottom; y++)
   {
      // Pixel centers.
      double cy = y + 0.5;
      std::vector<std::pair<double, int>> xs;
      for (const Edge &e : edges)
         if (cy >= e.y0 && cy < e.y1)
            xs.push_back({e.x0 + (cy - e.y0) * (e.x1 - e.x0) / (e.y1 - e.y0), e.dir});
      std::sort(xs.begin(), xs.end());
      int winding = 0;
      for (size_t i = 0; i + 1 < xs.size(); i++)
      {
         winding += xs[i].second;
         bool inside = mode == WINDING ? winding != 0 : (i % 2) == 0;
         if (!inside)
            continue;
         int x0 = (int) std::ceil(xs[i].first - 0.5), x1 = (int) std::ceil(xs[i + 1].first - 0.5);
         if (x1 > x0)
            r.Add(RECT{x0, y, x1, y + 1});
      }
   }
   return r;
}

std::map<HWND, Region> &WindowRegions()
{
   static std::map<HWND, Region> regions;
   return regions;
}

std::map<HDC, INT> &FillModes()
{
   static std::map<HDC, INT> modes;
   return modes;
}

HRGN NewRegion(const Region &r)
{
   HRGN h = CreateRectRgn(0, 0, 0, 0);
   if (Region *dst = GetRegion(h))
      *dst = r;
   return h;
}

INT RegionType(const Region &r)
{
   return r.Empty() ? NULLREGION : r.Rects().size() == 1 ? SIMPLEREGION : COMPLEXREGION;
}

}  // namespace

extern "C" {

/************************************************************************/
// Coordinates

CC_WEAK BOOL WINAPI SetViewportOrgEx(HDC hdc, INT x, INT y, POINT *old)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return FALSE;
   POINT &vp = Viewports()[hdc];
   if (old)
      *old = vp;
   dc->origin.x += x - vp.x;
   dc->origin.y += y - vp.y;
   vp = {x, y};
   return TRUE;
}

CC_WEAK BOOL WINAPI GetViewportOrgEx(HDC hdc, POINT *out)
{
   if (GetDCObject(hdc) == nullptr || out == nullptr)
      return FALSE;
   auto it = Viewports().find(hdc);
   *out = it == Viewports().end() ? POINT{0, 0} : it->second;
   return TRUE;
}

CC_WEAK BOOL WINAPI OffsetViewportOrgEx(HDC hdc, INT dx, INT dy, POINT *old)
{
   POINT vp;
   if (!GetViewportOrgEx(hdc, &vp))
      return FALSE;
   if (old)
      *old = vp;
   return SetViewportOrgEx(hdc, vp.x + dx, vp.y + dy, nullptr);
}

CC_WEAK BOOL WINAPI RectVisible(HDC hdc, const RECT *rect)
{
   DC *dc = GetDCObject(hdc);
   RasterTarget t;
   if (dc == nullptr || rect == nullptr || !GetTarget(dc, &t))
      return FALSE;
   RECT r = *rect;
   if (r.left > r.right)
      std::swap(r.left, r.right);
   if (r.top > r.bottom)
      std::swap(r.top, r.bottom);
   OffsetRect(&r, dc->origin.x, dc->origin.y);
   return EffectiveClip(dc, t).Intersects(r);
}

CC_WEAK BOOL WINAPI PtVisible(HDC hdc, INT x, INT y)
{
   RECT r = {x, y, x + 1, y + 1};
   return RectVisible(hdc, &r);
}

CC_WEAK COLORREF WINAPI GetNearestColor(HDC hdc, COLORREF color)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return CLR_INVALID;
   COLORREF rgb = ResolveColor(dc, color);
   RasterTarget t;
   if (GetTarget(dc, &t) && t.bpp <= 8 && t.colors)
   {
      const RGBQUAD &q = t.colors[Nearest(t, rgb)];
      return RGB(q.rgbRed, q.rgbGreen, q.rgbBlue);
   }
   return rgb;
}

CC_WEAK INT WINAPI SetPolyFillMode(HDC hdc, INT mode)
{
   if (GetDCObject(hdc) == nullptr || (mode != ALTERNATE && mode != WINDING))
      return 0;
   auto it = FillModes().find(hdc);
   INT old = it == FillModes().end() ? ALTERNATE : it->second;
   FillModes()[hdc] = mode;
   return old;
}

CC_WEAK INT WINAPI GetPolyFillMode(HDC hdc)
{
   if (GetDCObject(hdc) == nullptr)
      return 0;
   auto it = FillModes().find(hdc);
   return it == FillModes().end() ? ALTERNATE : it->second;
}

/************************************************************************/
// Regions

CC_WEAK HRGN WINAPI ExtCreateRegion(const XFORM *xform, DWORD size, const RGNDATA *data)
{
   if (data == nullptr || size < sizeof(RGNDATAHEADER) || data->rdh.iType != RDH_RECTANGLES)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return nullptr;
   }
   Region r;
   const RECT *rects = (const RECT *) data->Buffer;
   for (DWORD i = 0; i < data->rdh.nCount; i++)
   {
      RECT rc = rects[i];
      if (xform)
      {
         // Translation and scale; rotations would make a non-rectangular region.
         rc = {(LONG) (rc.left * xform->eM11 + xform->eDx), (LONG) (rc.top * xform->eM22 + xform->eDy),
               (LONG) (rc.right * xform->eM11 + xform->eDx), (LONG) (rc.bottom * xform->eM22 + xform->eDy)};
      }
      if (rc.left < rc.right && rc.top < rc.bottom)
         r.Add(rc);
   }
   return NewRegion(r);
}

CC_WEAK DWORD WINAPI GetRegionData(HRGN hrgn, DWORD size, RGNDATA *data)
{
   Region *r = GetRegion(hrgn);
   if (r == nullptr)
      return 0;
   DWORD need = sizeof(RGNDATAHEADER) + (DWORD) (r->Rects().size() * sizeof(RECT));
   if (data == nullptr)
      return need;
   if (size < need)
      return 0;
   data->rdh.dwSize = sizeof(RGNDATAHEADER);
   data->rdh.iType = RDH_RECTANGLES;
   data->rdh.nCount = (DWORD) r->Rects().size();
   data->rdh.nRgnSize = (DWORD) (r->Rects().size() * sizeof(RECT));
   data->rdh.rcBound = r->Bounds();
   memcpy(data->Buffer, r->Rects().data(), r->Rects().size() * sizeof(RECT));
   return need;
}

CC_WEAK HRGN WINAPI CreatePolygonRgn(const POINT *points, INT count, INT mode)
{
   if (points == nullptr || count < 2)
      return CreateRectRgn(0, 0, 0, 0);
   return NewRegion(PolygonRegion(points, &count, 1, mode));
}

CC_WEAK HRGN WINAPI CreatePolyPolygonRgn(const POINT *points, const INT *counts, INT polygons, INT mode)
{
   if (points == nullptr || counts == nullptr || polygons <= 0)
      return CreateRectRgn(0, 0, 0, 0);
   return NewRegion(PolygonRegion(points, counts, polygons, mode));
}

CC_WEAK HRGN WINAPI CreateRoundRectRgn(INT left, INT top, INT right, INT bottom, INT ew, INT eh)
{
   if (left > right)
      std::swap(left, right);
   if (top > bottom)
      std::swap(top, bottom);
   ew = std::min(std::abs(ew), right - left);
   eh = std::min(std::abs(eh), bottom - top);
   Region r;
   if (ew < 2 || eh < 2)
   {
      r.Add(RECT{left, top, right, bottom});
      return NewRegion(r);
   }
   // Each row's span: the corner ellipses' inset where the row is in a corner.
   double rx = ew / 2.0, ry = eh / 2.0;
   for (int y = top; y < bottom; y++)
   {
      double cy = y + 0.5;
      double dy = 0;
      if (cy < top + ry)
         dy = top + ry - cy;
      else if (cy > bottom - ry)
         dy = cy - (bottom - ry);
      double inset = dy > 0 ? rx - rx * std::sqrt(std::max(0.0, 1.0 - (dy / ry) * (dy / ry))) : 0;
      int x0 = left + (int) std::lround(inset), x1 = right - (int) std::lround(inset);
      if (x1 > x0)
         r.Add(RECT{x0, y, x1, y + 1});
   }
   return NewRegion(r);
}

CC_WEAK HRGN WINAPI CreateEllipticRgn(INT left, INT top, INT right, INT bottom)
{
   return CreateRoundRectRgn(left, top, right, bottom, right - left, bottom - top);
}

CC_WEAK BOOL WINAPI FrameRgn(HDC hdc, HRGN hrgn, HBRUSH brush, INT width, INT height)
{
   Region *r = GetRegion(hrgn);
   if (r == nullptr || width <= 0 || height <= 0)
      return FALSE;
   // The frame: the region less itself shrunk by width and height.
   Region inner = *r;
   for (int dx : {-width, width})
   {
      Region s = *r;
      s.Offset(dx, 0);
      inner.Intersect(s);
   }
   for (int dy : {-height, height})
   {
      Region s = *r;
      s.Offset(0, dy);
      inner.Intersect(s);
   }
   Region frame = *r;
   frame.Subtract(inner);
   for (const RECT &rc : frame.Rects())
      FillRect(hdc, &rc, brush);
   return TRUE;
}

CC_WEAK BOOL WINAPI InvertRgn(HDC hdc, HRGN hrgn)
{
   Region *r = GetRegion(hrgn);
   if (r == nullptr)
      return FALSE;
   for (const RECT &rc : r->Rects())
      InvertRect(hdc, &rc);
   return TRUE;
}

// A window's region: kept and returned; windows are still painted and hit as
// rectangles.
CC_WEAK INT WINAPI SetWindowRgn(HWND hwnd, HRGN hrgn, BOOL redraw)
{
   if (GetWindowObject(hwnd) == nullptr)
      return 0;
   if (hrgn == nullptr)
      WindowRegions().erase(hwnd);
   else if (Region *r = GetRegion(hrgn))
   {
      WindowRegions()[hwnd] = *r;
      DeleteObject(hrgn);   // the system owns it now
   }
   if (redraw)
      RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME);
   return 1;
}

CC_WEAK INT WINAPI GetWindowRgn(HWND hwnd, HRGN hrgn)
{
   auto it = WindowRegions().find(hwnd);
   Region *dst = GetRegion(hrgn);
   if (it == WindowRegions().end() || dst == nullptr || GetWindowObject(hwnd) == nullptr)
      return ERROR;
   *dst = it->second;
   return RegionType(*dst);
}

/************************************************************************/
// DIBs

CC_WEAK INT WINAPI SetDIBits(HDC hdc, HBITMAP hbm, UINT start, UINT lines, const void *bits, const BITMAPINFO *bi,
                             UINT usage)
{
   (void) hdc;
   GdiBitmap *b = GetGdiObjectAs<GdiBitmap>(hbm, OBJ_BITMAP);
   if (b == nullptr || bits == nullptr || bi == nullptr)
      return 0;
   const BITMAPINFOHEADER &h = bi->bmiHeader;
   if (h.biCompression == BI_RLE8 || h.biCompression == BI_RLE4)
   {
      std::vector<BYTE> info, pixels;
      if (!DecodeRleDib(bi, bits, &info, &pixels))
         return 0;
      return SetDIBits(hdc, hbm, start, lines, pixels.data(), (const BITMAPINFO *) info.data(), usage);
   }
   if (h.biCompression != BI_RGB && h.biCompression != BI_BITFIELDS)
      return 0;
   int bpp = h.biBitCount;
   if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32)
      return 0;
   std::vector<RGBQUAD> colors = usage == DIB_RGB_COLORS ? DibColors(bi) : std::vector<RGBQUAD>();
   if (usage != DIB_RGB_COLORS && bpp <= 8)
      colors.assign(1 << bpp, RGBQUAD{0, 0, 0, 0});
   int height = std::abs(h.biHeight);
   bool bottom_up = h.biHeight > 0;
   int stride = ((h.biWidth * bpp + 31) / 32) * 4;
   // The DIB's scan lines start .. start + lines (from the bottom when bottom-up).
   RasterTarget src = {(uint8_t *) bits, (int) h.biWidth, (int) lines, stride, bpp, bottom_up,
                       colors.empty() ? nullptr : colors.data(), (int) colors.size()};
   RasterTarget dst = BitmapTarget(b);
   bool raw = bpp == b->bpp && (bpp > 8 || b->colors.size() == colors.size()) &&
              (bpp > 8 || memcmp(b->colors.data(), colors.data(), colors.size() * sizeof(RGBQUAD)) == 0);
   int w = std::min<int>(h.biWidth, b->width);
   UINT done = 0;
   for (UINT i = 0; i < lines; i++)
   {
      UINT line = start + i;
      if ((int) line >= height)
         break;
      int y = bottom_up ? height - 1 - (int) line : (int) line;   // image row, top 0
      if (y < 0 || y >= b->height)
         continue;
      int sy = bottom_up ? (int) lines - 1 - (int) i : (int) i;   // row within src (top 0)
      if (raw)
         memcpy(dst.Row(y), src.Row(sy), (size_t) ((w * bpp + 7) / 8));
      else
         for (int x = 0; x < w; x++)
            WriteRgb(dst, x, y, ReadRgb(src, x, sy), ReadAlpha(src, x, sy));
      done++;
   }
   return (INT) done;
}

CC_WEAK INT WINAPI GetDIBits(HDC hdc, HBITMAP hbm, UINT start, UINT lines, void *bits, BITMAPINFO *bi, UINT usage)
{
   (void) hdc;
   GdiBitmap *b = GetGdiObjectAs<GdiBitmap>(hbm, OBJ_BITMAP);
   if (b == nullptr || bi == nullptr)
      return 0;
   BITMAPINFOHEADER &h = bi->bmiHeader;
   if (h.biSize < sizeof(BITMAPCOREHEADER))
      return 0;
   if (h.biBitCount == 0)
   {
      // The bitmap's format.
      h.biWidth = b->width;
      h.biHeight = b->height;
      h.biPlanes = 1;
      h.biBitCount = (WORD) b->bpp;
      h.biCompression = BI_RGB;
      h.biSizeImage = (DWORD) (b->stride * b->height);
      h.biClrUsed = 0;
      h.biClrImportant = 0;
      if (bits == nullptr)
         return 1;
   }
   int bpp = h.biBitCount;
   if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32)
      return 0;
   if (h.biWidth <= 0)
      h.biWidth = b->width;
   // The color table: the bitmap's, black and white for monochrome, else a
   // uniform palette.
   RGBQUAD *table = (RGBQUAD *) ((BYTE *) bi + h.biSize);
   int ncolors = bpp <= 8 ? 1 << bpp : 0;
   std::vector<RGBQUAD> colors;
   if (ncolors)
   {
      if (b->bpp == bpp && !b->colors.empty())
         colors = b->colors;
      else if (bpp == 1)
         colors = {{0, 0, 0, 0}, {255, 255, 255, 0}};
      else
         colors = DefaultColors(bpp);
      colors.resize(ncolors);
      if (usage == DIB_RGB_COLORS)
         memcpy(table, colors.data(), ncolors * sizeof(RGBQUAD));
   }
   if (h.biCompression == BI_BITFIELDS && bpp == 32)
   {
      DWORD *masks = (DWORD *) table;
      masks[0] = 0xFF0000;
      masks[1] = 0xFF00;
      masks[2] = 0xFF;
   }
   int stride = ((h.biWidth * bpp + 31) / 32) * 4;
   h.biSizeImage = (DWORD) (stride * std::abs(h.biHeight));
   if (bits == nullptr)
      return (INT) std::min<UINT>(lines, std::abs(h.biHeight));
   int height = std::abs(h.biHeight);
   bool bottom_up = h.biHeight > 0;
   RasterTarget src = BitmapTarget(b);
   RasterTarget dst = {(uint8_t *) bits, (int) h.biWidth, (int) lines, stride, bpp, bottom_up,
                       colors.empty() ? nullptr : colors.data(), (int) colors.size()};
   bool raw = bpp == b->bpp && (bpp > 8 || (colors.size() == b->colors.size() &&
                                           memcmp(colors.data(), b->colors.data(), colors.size() * sizeof(RGBQUAD)) == 0));
   int w = std::min<int>(h.biWidth, b->width);
   UINT done = 0;
   for (UINT i = 0; i < lines; i++)
   {
      UINT line = start + i;
      if ((int) line >= height)
         break;
      int y = bottom_up ? height - 1 - (int) line : (int) line;
      int dy = bottom_up ? (int) lines - 1 - (int) i : (int) i;
      if (y >= b->height)
      {
         memset(dst.Row(dy), 0, stride);
         continue;
      }
      if (raw)
         memcpy(dst.Row(dy), src.Row(y), (size_t) ((w * bpp + 7) / 8));
      else
         for (int x = 0; x < w; x++)
            WriteRgb(dst, x, dy, ReadRgb(src, x, y), ReadAlpha(src, x, y));
      done++;
   }
   return (INT) done;
}

CC_WEAK BOOL WINAPI GdiAlphaBlend(HDC dest, INT dx, INT dy, INT dw, INT dh, HDC source, INT sx, INT sy, INT sw,
                                  INT sh, BLENDFUNCTION blend)
{
   DC *ddc = GetDCObject(dest), *sdc = GetDCObject(source);
   RasterTarget dt, st;
   if (ddc == nullptr || sdc == nullptr || !GetTarget(ddc, &dt) || !GetTarget(sdc, &st))
      return FALSE;
   if (dw <= 0 || dh <= 0 || sw <= 0 || sh <= 0)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
   }
   bool per_pixel = (blend.AlphaFormat & AC_SRC_ALPHA) && st.bpp == 32;
   int constant = blend.SourceConstantAlpha;
   Region clip = EffectiveClip(ddc, dt);
   int ox = dx + ddc->origin.x, oy = dy + ddc->origin.y;
   RECT area = {ox, oy, ox + dw, oy + dh};
   clip.Intersect(area);
   for (const RECT &r : clip.Rects())
      for (int y = r.top; y < r.bottom; y++)
      {
         int syy = sy + sdc->origin.y + (int) ((int64_t) (y - oy) * sh / dh);
         if (syy < 0 || syy >= st.height)
            continue;
         for (int x = r.left; x < r.right; x++)
         {
            int sxx = sx + sdc->origin.x + (int) ((int64_t) (x - ox) * sw / dw);
            if (sxx < 0 || sxx >= st.width)
               continue;
            COLORREF s = ReadRgb(st, sxx, syy), d = ReadRgb(dt, x, y);
            // Source premultiplied by its alpha, as AlphaBlend takes it.
            int sa = per_pixel ? ReadAlpha(st, sxx, syy) : 255;
            int a = sa * constant / 255;
            auto mix = [&](int sc, int dc) { return std::min(255, (sc * constant / 255) + dc * (255 - a) / 255); };
            COLORREF out = per_pixel ? RGB(mix(GetRValue(s), GetRValue(d)), mix(GetGValue(s), GetGValue(d)),
                                           mix(GetBValue(s), GetBValue(d)))
                                     : RGB((GetRValue(s) * a + GetRValue(d) * (255 - a)) / 255,
                                           (GetGValue(s) * a + GetGValue(d) * (255 - a)) / 255,
                                           (GetBValue(s) * a + GetBValue(d) * (255 - a)) / 255);
            WriteRgb(dt, x, y, out, 0xFF);
         }
      }
   ShowTargetChanges(ddc, area);
   return TRUE;
}

CC_WEAK BOOL WINAPI AlphaBlend(HDC dest, INT dx, INT dy, INT dw, INT dh, HDC source, INT sx, INT sy, INT sw, INT sh,
                               BLENDFUNCTION blend)
{
   return GdiAlphaBlend(dest, dx, dy, dw, dh, source, sx, sy, sw, sh, blend);
}

/************************************************************************/
// Icons from bitmaps, and drawing them

CC_WEAK HICON WINAPI CreateIconIndirect(ICONINFO *info)
{
   if (info == nullptr || info->hbmMask == nullptr)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return nullptr;
   }
   GdiBitmap *mask = GetGdiObjectAs<GdiBitmap>(info->hbmMask, OBJ_BITMAP);
   GdiBitmap *color = info->hbmColor ? GetGdiObjectAs<GdiBitmap>(info->hbmColor, OBJ_BITMAP) : nullptr;
   if (mask == nullptr)
      return nullptr;
   // Without a color bitmap the mask is twice as high: AND mask over XOR image.
   int w = color ? color->width : mask->width;
   int h = color ? color->height : mask->height / 2;
   RasterTarget mt = BitmapTarget(mask);
   RasterTarget ct = color ? BitmapTarget(color) : mt;
   int coff = color ? 0 : h;
   bool has_alpha = false;
   if (color && color->bpp == 32)
      for (int y = 0; y < h && !has_alpha; y++)
         for (int x = 0; x < w; x++)
            if (ReadAlpha(ct, x, y))
            {
               has_alpha = true;
               break;
            }
   auto *img = new CursorImage;
   img->width = w;
   img->height = h;
   img->hot_x = info->fIcon ? w / 2 : (int) info->xHotspot;
   img->hot_y = info->fIcon ? h / 2 : (int) info->yHotspot;
   img->pixels.resize((size_t) w * h);
   for (int y = 0; y < h; y++)
      for (int x = 0; x < w; x++)
      {
         COLORREF c = ReadRgb(ct, x, y + coff);
         uint32_t a;
         if (has_alpha)
            a = ReadAlpha(ct, x, y);
         else
            a = (y < mask->height && x < mask->width && ReadRgb(mt, x, y) != 0) ? 0 : 0xFF;
         img->pixels[(size_t) y * w + x] = (a << 24) | (GetRValue(c) << 16) | (GetGValue(c) << 8) | GetBValue(c);
      }
   return (HICON) AddCursorImage(img);
}

CC_WEAK BOOL WINAPI GetIconInfo(HICON icon, ICONINFO *info)
{
   const CursorImage *c = GetCursorImage(icon);
   if (c == nullptr || info == nullptr)
      return FALSE;
   int w = c->width ? c->width : 32, h = c->height ? c->height : 32;
   info->fIcon = c->hot_x == w / 2 && c->hot_y == h / 2;
   info->xHotspot = c->hot_x;
   info->yHotspot = c->hot_y;
   BITMAPINFO bi = {};
   bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bi.bmiHeader.biWidth = w;
   bi.bmiHeader.biHeight = -h;
   bi.bmiHeader.biPlanes = 1;
   bi.bmiHeader.biBitCount = 32;
   void *bits = nullptr;
   info->hbmColor = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
   info->hbmMask = CreateBitmap(w, h, 1, 1, nullptr);
   GdiBitmap *mask = GetGdiObjectAs<GdiBitmap>(info->hbmMask, OBJ_BITMAP);
   if (info->hbmColor == nullptr || mask == nullptr)
      return FALSE;
   RasterTarget mt = BitmapTarget(mask);
   // Color premultiplied, as Windows keeps 32-bit icons' color bitmaps.
   for (int y = 0; y < h; y++)
      for (int x = 0; x < w; x++)
      {
         uint32_t p = (c->pixels.size() == (size_t) w * h) ? c->pixels[(size_t) y * w + x] : 0;
         uint32_t a = p >> 24;
         BYTE *q = (BYTE *) bits + ((size_t) y * w + x) * 4;
         q[0] = (BYTE) ((p & 0xFF) * a / 255);
         q[1] = (BYTE) (((p >> 8) & 0xFF) * a / 255);
         q[2] = (BYTE) (((p >> 16) & 0xFF) * a / 255);
         q[3] = (BYTE) a;
         WriteRgb(mt, x, y, a < 128 ? RGB(255, 255, 255) : 0);
      }
   return TRUE;
}

CC_WEAK HICON WINAPI CopyIcon(HICON icon)
{
   const CursorImage *c = GetCursorImage(icon);
   if (c == nullptr)
      return nullptr;
   return (HICON) AddCursorImage(new CursorImage(*c));
}

CC_WEAK BOOL WINAPI DrawIconEx(HDC hdc, INT x, INT y, HICON icon, INT cx, INT cy, UINT step, HBRUSH flicker,
                               UINT flags)
{
   (void) step;
   const CursorImage *c = GetCursorImage(icon);
   DC *dc = GetDCObject(hdc);
   RasterTarget t;
   if (c == nullptr || dc == nullptr || !GetTarget(dc, &t))
      return FALSE;
   if (c->pixels.empty())
      return TRUE;   // a standard cursor shape: no image of ours
   if (cx == 0)
      cx = (flags & DI_DEFAULTSIZE) ? GetSystemMetrics(SM_CXICON) : c->width;
   if (cy == 0)
      cy = (flags & DI_DEFAULTSIZE) ? GetSystemMetrics(SM_CYICON) : c->height;
   if (flags == 0)
      flags = DI_NORMAL;
   if (flicker)
   {
      RECT r = {x, y, x + cx, y + cy};
      FillRect(hdc, &r, flicker);
   }
   int ox = x + dc->origin.x, oy = y + dc->origin.y;
   RECT area = {ox, oy, ox + cx, oy + cy};
   Region clip = EffectiveClip(dc, t);
   clip.Intersect(area);
   bool use_mask = (flags & DI_MASK) != 0, use_image = (flags & DI_IMAGE) != 0;
   for (const RECT &r : clip.Rects())
      for (int py = r.top; py < r.bottom; py++)
      {
         int sy = (py - oy) * c->height / cy;
         for (int px = r.left; px < r.right; px++)
         {
            int sx = (px - ox) * c->width / cx;
            uint32_t p = c->pixels[(size_t) sy * c->width + sx];
            uint32_t a = p >> 24;
            COLORREF s = RGB((p >> 16) & 0xFF, (p >> 8) & 0xFF, p & 0xFF);
            if (use_image && !use_mask)
            {
               WriteRgb(t, px, py, s);
               continue;
            }
            if (use_mask && !use_image)
            {
               // The AND mask: black where the icon is opaque.
               if (a >= 128)
                  WriteRgb(t, px, py, 0);
               continue;
            }
            if (a == 0)
               continue;
            COLORREF d = a == 255 ? 0 : ReadRgb(t, px, py);
            COLORREF out = RGB((GetRValue(s) * a + GetRValue(d) * (255 - a)) / 255,
                               (GetGValue(s) * a + GetGValue(d) * (255 - a)) / 255,
                               (GetBValue(s) * a + GetBValue(d) * (255 - a)) / 255);
            WriteRgb(t, px, py, out);
         }
      }
   ShowTargetChanges(dc, area);
   return TRUE;
}

CC_WEAK BOOL WINAPI DrawIcon(HDC hdc, INT x, INT y, HICON icon)
{
   return DrawIconEx(hdc, x, y, icon, 0, 0, 0, nullptr, DI_NORMAL | DI_DEFAULTSIZE);
}

CC_WEAK HANDLE WINAPI CopyImage(HANDLE image, UINT type, INT cx, INT cy, UINT flags)
{
   switch (type)
   {
   case IMAGE_ICON:
   case IMAGE_CURSOR:
      return CopyIcon((HICON) image);
   case IMAGE_BITMAP:
   {
      GdiBitmap *b = GetGdiObjectAs<GdiBitmap>(image, OBJ_BITMAP);
      if (b == nullptr)
         return nullptr;
      int w = cx ? cx : b->width, h = cy ? cy : b->height;
      std::vector<BYTE> info(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
      auto *bi = (BITMAPINFO *) info.data();
      bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bi->bmiHeader.biWidth = w;
      bi->bmiHeader.biHeight = b->bottom_up ? h : -h;
      bi->bmiHeader.biPlanes = 1;
      bi->bmiHeader.biBitCount = (WORD) b->bpp;
      bi->bmiHeader.biClrUsed = (DWORD) b->colors.size();
      if (!b->colors.empty())
         memcpy(bi->bmiColors, b->colors.data(), b->colors.size() * sizeof(RGBQUAD));
      void *bits = nullptr;
      HBITMAP copy = CreateDIBSection(nullptr, bi, DIB_RGB_COLORS, &bits, nullptr, 0);
      GdiBitmap *nb = GetGdiObjectAs<GdiBitmap>(copy, OBJ_BITMAP);
      if (nb == nullptr)
         return nullptr;
      if (w == b->width && h == b->height)
         memcpy(nb->bits, b->bits, (size_t) b->stride * b->height);
      else
      {
         RasterTarget s = BitmapTarget(b), d = BitmapTarget(nb);
         for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
               WriteRgb(d, x, y, ReadRgb(s, x * b->width / w, y * b->height / h),
                        ReadAlpha(s, x * b->width / w, y * b->height / h));
      }
      if (flags & LR_COPYDELETEORG)
         DeleteObject(image);
      return copy;
   }
   }
   return nullptr;
}

}  // extern "C"
