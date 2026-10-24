/*
 * ctl_gdi.cpp:  GDI for the controls: the Unicode text functions (text is drawn
 *   as the code page has it, as the ANSI functions draw), tabbed text, window and
 *   viewport origins, clipping regions, icons and alpha blending.
 */

#include "core/gdi.h"

#include "core/cursor.h"
#include "core/native.h"
#include "core/winproc.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace w32 {

namespace {

native::Font *FontOf(DC *dc)
{
   GdiFont *f = GetGdiObjectAs<GdiFont>(dc->font, OBJ_FONT);
   if (f == nullptr)
      f = GetGdiObjectAs<GdiFont>(GetStockObject(SYSTEM_FONT), OBJ_FONT);
   if (f->font == nullptr)
      f->font = native::CreateFont(f->log);
   return f->font;
}

// What the ANSI functions draw for s: its code page form, in UTF-8.
std::string Drawn(const WCHAR *s, int n)
{
   std::string a = WideToAnsi(s, n);
   return AnsiToUtf8(a.c_str(), (int) a.size());
}

uint32_t Read(const RasterTarget &t, int x, int y)
{
   const uint8_t *row = t.Row(y);
   switch (t.bpp)
   {
   case 32: return ((const uint32_t *) row)[x];
   case 24: return 0xFF000000u | (row[x * 3 + 2] << 16) | (row[x * 3 + 1] << 8) | row[x * 3];
   case 8: return row[x];
   case 1: return (row[x >> 3] >> (7 - (x & 7))) & 1;
   }
   return 0;
}

void Write(const RasterTarget &t, int x, int y, uint32_t v)
{
   uint8_t *row = t.Row(y);
   switch (t.bpp)
   {
   case 32: ((uint32_t *) row)[x] = v; break;
   case 24: row[x * 3] = v & 0xFF; row[x * 3 + 1] = (v >> 8) & 0xFF; row[x * 3 + 2] = (v >> 16) & 0xFF; break;
   case 8: row[x] = (uint8_t) v; break;
   case 1:
      if (v & 1)
         row[x >> 3] |= (uint8_t) (0x80 >> (x & 7));
      else
         row[x >> 3] &= (uint8_t) ~(0x80 >> (x & 7));
      break;
   }
}

// A pixel's RGB.
COLORREF Rgb(const RasterTarget &t, uint32_t v)
{
   if (t.bpp >= 24)
      return RGB((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
   if (t.colors && (int) v < t.num_colors)
      return RGB(t.colors[v].rgbRed, t.colors[v].rgbGreen, t.colors[v].rgbBlue);
   return t.bpp == 1 ? (v ? RGB(255, 255, 255) : 0) : 0;
}

// Blends straight-alpha color (r, g, b, a) over target pixel (x, y).
void Blend(const RasterTarget &t, int x, int y, int r, int g, int b, int a)
{
   if (a <= 0)
      return;
   if (a >= 255 || t.bpp <= 8)
   {
      if (t.bpp <= 8 && a < 128)
         return;
      Write(t, x, y, PixelValue(t, RGB(r, g, b)));
      return;
   }
   COLORREF d = Rgb(t, Read(t, x, y));
   int nr = (r * a + GetRValue(d) * (255 - a)) / 255, ng = (g * a + GetGValue(d) * (255 - a)) / 255,
       nb = (b * a + GetBValue(d) * (255 - a)) / 255;
   Write(t, x, y, PixelValue(t, RGB(nr, ng, nb)));
}

// Calls f(x, y) for each target pixel of the logical rect r that may be drawn,
// then shows the change.
template <class F> bool ForEachPixel(HDC hdc, const RECT &r, F f)
{
   DC *dc = GetDCObject(hdc);
   RasterTarget t;
   if (dc == nullptr || dc->info_only || !GetTarget(dc, &t))
      return false;
   RECT tr = r;
   OffsetRect(&tr, dc->origin.x, dc->origin.y);
   Region clip = EffectiveClip(dc, t);
   clip.Intersect(tr);
   for (const RECT &c : clip.Rects())
      for (int y = c.top; y < c.bottom; y++)
         for (int x = c.left; x < c.right; x++)
            f(t, x, y, x - tr.left, y - tr.top);
   if (!clip.Empty())
      ShowTargetChanges(dc, clip.Bounds());
   return true;
}

}  // namespace

}  // namespace w32

using namespace w32;

/************************************************************************/
// Text

BOOL WINAPI ExtTextOutW(HDC hdc, INT x, INT y, UINT options, const RECT *rect, LPCWSTR text, UINT count,
                        const INT *dx)
{
   if (GetDCObject(hdc) == nullptr)
      return FALSE;
   std::string a = (text && count) ? WideToAnsi(text, (int) count) : std::string();
   std::vector<INT> advances;
   if (dx && (options & ETO_PDY))
   {
      for (UINT i = 0; i < count; i++)
         advances.push_back(dx[i * 2]);
      dx = advances.data();
   }
   return ExtTextOutA(hdc, x, y, options & ~(ETO_PDY | ETO_GLYPH_INDEX), rect, a.c_str(), (UINT) a.size(),
                      a.size() == count ? dx : nullptr);
}

BOOL WINAPI TextOutW(HDC hdc, INT x, INT y, LPCWSTR text, INT count)
{
   return ExtTextOutW(hdc, x, y, 0, nullptr, text, count < 0 ? 0 : (UINT) count, nullptr);
}

BOOL WINAPI GetTextExtentPoint32W(HDC hdc, LPCWSTR text, INT count, SIZE *size)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || size == nullptr || count < 0)
      return FALSE;
   native::Font *font = FontOf(dc);
   size->cx = native::TextWidth(font, Drawn(text, text ? count : 0));
   size->cy = native::GetFontMetrics(font).height;
   return TRUE;
}

BOOL WINAPI GetTextExtentPointW(HDC hdc, LPCWSTR text, INT count, SIZE *size)
{
   return GetTextExtentPoint32W(hdc, text, count, size);
}

BOOL WINAPI GetTextExtentExPointW(HDC hdc, LPCWSTR text, INT count, INT max_extent, INT *fit, INT *dx, SIZE *size)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || count < 0 || (count && text == nullptr))
      return FALSE;
   native::Font *font = FontOf(dc);
   std::vector<int> edges = native::CharEdges(font, Drawn(text, count));
   edges.resize(count, edges.empty() ? 0 : edges.back());
   if (dx)
      for (int i = 0; i < count; i++)
         dx[i] = edges[i];
   if (fit)
   {
      int n = 0;
      while (n < count && edges[n] <= max_extent)
         n++;
      *fit = n;
   }
   if (size)
   {
      size->cx = count ? edges.back() : 0;
      size->cy = native::GetFontMetrics(font).height;
   }
   return TRUE;
}

BOOL WINAPI GetTextExtentExPointA(HDC hdc, LPCSTR text, INT count, INT max_extent, INT *fit, INT *dx, SIZE *size)
{
   if (count < 0 || (count && text == nullptr))
      return FALSE;
   std::basic_string<WCHAR> w = AnsiToWide(text, count);
   return GetTextExtentExPointW(hdc, w.c_str(), (INT) w.size(), max_extent, fit, dx, size);
}

BOOL WINAPI GetCharWidth32W(HDC hdc, UINT first, UINT last, INT *widths)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || widths == nullptr || last < first)
      return FALSE;
   native::Font *font = FontOf(dc);
   for (UINT ch = first; ch <= last; ch++)
   {
      WCHAR c = (WCHAR) ch;
      widths[ch - first] = native::TextWidth(font, Drawn(&c, 1));
   }
   return TRUE;
}

BOOL WINAPI GetCharWidthW(HDC hdc, UINT first, UINT last, INT *widths)
{
   return GetCharWidth32W(hdc, first, last, widths);
}

BOOL WINAPI GetCharWidthA(HDC hdc, UINT first, UINT last, INT *widths)
{
   return GetCharWidth32A(hdc, first, last, widths);
}

BOOL WINAPI GetCharABCWidthsW(HDC hdc, UINT first, UINT last, ABC *abc)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || abc == nullptr || last < first)
      return FALSE;
   native::Font *font = FontOf(dc);
   for (UINT ch = first; ch <= last; ch++)
   {
      int a, b, c;
      native::CharWidths(font, ch, &a, &b, &c);
      abc[ch - first] = {a, (UINT) std::max(b, 0), c};
   }
   return TRUE;
}

HFONT WINAPI CreateFontIndirectW(const LOGFONTW *lf)
{
   if (lf == nullptr)
      return nullptr;
   LOGFONTA a;
   memcpy(&a, lf, offsetof(LOGFONTA, lfFaceName));
   std::string face = WideToAnsi(lf->lfFaceName);
   lstrcpynA(a.lfFaceName, face.c_str(), LF_FACESIZE);
   return CreateFontIndirectA(&a);
}

HFONT WINAPI CreateFontW(INT height, INT width, INT escapement, INT orientation, INT weight, DWORD italic,
                         DWORD underline, DWORD strikeout, DWORD charset, DWORD out_precision, DWORD clip_precision,
                         DWORD quality, DWORD pitch, LPCWSTR face)
{
   LOGFONTW lf = {height, width, escapement, orientation, weight, (BYTE) italic, (BYTE) underline, (BYTE) strikeout,
                  (BYTE) charset, (BYTE) out_precision, (BYTE) clip_precision, (BYTE) quality, (BYTE) pitch, {}};
   if (face)
      lstrcpynW(lf.lfFaceName, face, LF_FACESIZE);
   return CreateFontIndirectW(&lf);
}

INT WINAPI GetTextFaceW(HDC hdc, INT size, LPWSTR name)
{
   char a[LF_FACESIZE];
   if (!GetTextFaceA(hdc, sizeof(a), a))
      return 0;
   std::basic_string<WCHAR> w = AnsiToWide(a);
   if (name == nullptr)
      return (INT) w.size() + 1;
   lstrcpynW(name, w.c_str(), size);
   return lstrlenW(name);
}

// After Wine's user32/text.c TEXT_TabbedTextOut.
static LONG TabbedText(HDC hdc, INT x, INT y, LPCWSTR s, INT count, INT tab_count, const INT *tabs, INT origin,
                       bool display)
{
   if (s == nullptr || count == 0)
      return 0;
   if (tabs == nullptr)
      tab_count = 0;
   TEXTMETRICW tm;
   GetTextMetricsW(hdc, &tm);
   INT def_width;
   if (tab_count == 1)
   {
      def_width = *tabs;
      tab_count = 0;
   }
   else
      def_width = 8 * tm.tmAveCharWidth;
   int start = x;
   SIZE extent = {0, 0};
   while (count > 0)
   {
      int i, j;
      INT x0 = x;
      RECT r;
      r.left = x0;
      for (i = 0; i < count; i++)
         if (s[i] != '\t')
            break;
      for (j = i; j < count; j++)
         if (s[j] == '\t')
            break;
      GetTextExtentPointW(hdc, s + i, j - i, &extent);
      if (i)
      {
         for (; tab_count >= i; tabs++, tab_count--)
         {
            if (origin + abs(*tabs) > x)
            {
               if (tabs[i - 1] >= 0)
               {
                  x0 = origin + tabs[i - 1];
                  x = x0 + extent.cx;
                  break;
               }
               else if (origin - tabs[i - 1] - extent.cx > x)
               {
                  x = origin - tabs[i - 1];
                  x0 = x - extent.cx;
                  break;
               }
            }
         }
         if (tab_count < i && def_width > 0)
         {
            x0 = origin + ((x - origin) / def_width + i) * def_width;
            x = x0 + extent.cx;
         }
         else if (tab_count < i && def_width < 0)
         {
            x = origin + ((x - origin + extent.cx) / -def_width + i) * -def_width;
            x0 = x - extent.cx;
         }
      }
      else
         x += extent.cx;
      if (!extent.cy)
         extent.cy = tm.tmHeight;
      if (display)
      {
         r.top = y;
         r.right = x;
         r.bottom = y + extent.cy;
         ExtTextOutW(hdc, x0, y, GetBkMode(hdc) == OPAQUE ? ETO_OPAQUE : 0, &r, s + i, j - i, nullptr);
      }
      count -= j;
      s += j;
   }
   return MAKELONG(x - start, extent.cy);
}

LONG WINAPI TabbedTextOutW(HDC hdc, INT x, INT y, LPCWSTR s, INT count, INT tab_count, const INT *tabs, INT origin)
{
   if (count < 0 && s)
      count = lstrlenW(s);
   return TabbedText(hdc, x, y, s, count, tab_count, tabs, origin, true);
}

LONG WINAPI TabbedTextOutA(HDC hdc, INT x, INT y, LPCSTR s, INT count, INT tab_count, const INT *tabs, INT origin)
{
   std::basic_string<WCHAR> w = AnsiToWide(s, count);
   return TabbedText(hdc, x, y, s ? w.c_str() : nullptr, (INT) w.size(), tab_count, tabs, origin, true);
}

DWORD WINAPI GetTabbedTextExtentW(HDC hdc, LPCWSTR s, INT count, INT tab_count, const INT *tabs)
{
   if (count < 0 && s)
      count = lstrlenW(s);
   return (DWORD) TabbedText(hdc, 0, 0, s, count, tab_count, tabs, 0, false);
}

DWORD WINAPI GetTabbedTextExtentA(HDC hdc, LPCSTR s, INT count, INT tab_count, const INT *tabs)
{
   std::basic_string<WCHAR> w = AnsiToWide(s, count);
   return (DWORD) TabbedText(hdc, 0, 0, s ? w.c_str() : nullptr, (INT) w.size(), tab_count, tabs, 0, false);
}

LONG WINAPI GdiGetCharDimensions(HDC hdc, TEXTMETRICW *tm, LONG *height)
{
   // As Windows: the average width of the alphabet, rounded.
   static const WCHAR alphabet[] = u"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
   SIZE size;
   if (tm && !GetTextMetricsW(hdc, tm))
      return 0;
   if (!GetTextExtentPointW(hdc, alphabet, 52, &size))
      return 0;
   if (height)
      *height = size.cy;
   return (size.cx / 26 + 1) / 2;
}

UINT WINAPI GetTextCharsetInfo(HDC hdc, FONTSIGNATURE *fs, DWORD)
{
   if (fs)
   {
      memset(fs, 0, sizeof(*fs));
      fs->fsCsb[0] = FS_LATIN1;
   }
   return GetDCObject(hdc) ? ANSI_CHARSET : DEFAULT_CHARSET;
}

UINT WINAPI GetTextCharset(HDC hdc)
{
   return GetTextCharsetInfo(hdc, nullptr, 0);
}

DWORD WINAPI GdiGetCodePage(HDC)
{
   return 1252;
}

// gdi32's undocumented export (Wine's edit.c asks for CJK fonts only).
extern "C" W32_EXPORT BOOL WINAPI GetCharWidthInfo(HDC, void *)
{
   return FALSE;
}

/************************************************************************/
// Coordinates

BOOL WINAPI SetWindowOrgEx(HDC hdc, INT x, INT y, POINT *old)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return FALSE;
   if (old)
      *old = dc->window_org;
   dc->origin.x += dc->window_org.x - x;
   dc->origin.y += dc->window_org.y - y;
   dc->window_org = {x, y};
   return TRUE;
}

BOOL WINAPI GetWindowOrgEx(HDC hdc, POINT *p)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || p == nullptr)
      return FALSE;
   *p = dc->window_org;
   return TRUE;
}

BOOL WINAPI OffsetWindowOrgEx(HDC hdc, INT dx, INT dy, POINT *old)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return FALSE;
   return SetWindowOrgEx(hdc, dc->window_org.x + dx, dc->window_org.y + dy, old);
}

BOOL WINAPI SetViewportOrgEx(HDC hdc, INT x, INT y, POINT *old)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return FALSE;
   if (old)
      *old = dc->viewport_org;
   dc->origin.x += x - dc->viewport_org.x;
   dc->origin.y += y - dc->viewport_org.y;
   dc->viewport_org = {x, y};
   return TRUE;
}

BOOL WINAPI GetViewportOrgEx(HDC hdc, POINT *p)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || p == nullptr)
      return FALSE;
   *p = dc->viewport_org;
   return TRUE;
}

BOOL WINAPI OffsetViewportOrgEx(HDC hdc, INT dx, INT dy, POINT *old)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return FALSE;
   return SetViewportOrgEx(hdc, dc->viewport_org.x + dx, dc->viewport_org.y + dy, old);
}

BOOL WINAPI DPtoLP(HDC hdc, POINT *points, INT count)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || (points == nullptr && count))
      return FALSE;
   // MM_TEXT: logical = device - viewport origin + window origin.
   for (INT i = 0; i < count; i++)
   {
      points[i].x += dc->window_org.x - dc->viewport_org.x;
      points[i].y += dc->window_org.y - dc->viewport_org.y;
   }
   return TRUE;
}

BOOL WINAPI LPtoDP(HDC hdc, POINT *points, INT count)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || (points == nullptr && count))
      return FALSE;
   for (INT i = 0; i < count; i++)
   {
      points[i].x -= dc->window_org.x - dc->viewport_org.x;
      points[i].y -= dc->window_org.y - dc->viewport_org.y;
   }
   return TRUE;
}

DWORD WINAPI GetLayout(HDC hdc)
{
   DC *dc = GetDCObject(hdc);
   return dc ? dc->layout : GDI_ERROR;
}

DWORD WINAPI SetLayout(HDC hdc, DWORD layout)
{
   // Right-to-left layouts aren't drawn mirrored; the value is kept.
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return GDI_ERROR;
   DWORD old = dc->layout;
   dc->layout = layout;
   return old;
}

BOOL WINAPI SetBrushOrgEx(HDC hdc, INT, INT, POINT *old)
{
   // Patterns are aligned to the DC's origin, as with the default brush origin.
   if (old)
      *old = {0, 0};
   return GetDCObject(hdc) != nullptr;
}

BOOL WINAPI GetBrushOrgEx(HDC hdc, POINT *p)
{
   if (p)
      *p = {0, 0};
   return GetDCObject(hdc) != nullptr;
}

BOOL WINAPI UnrealizeObject(HGDIOBJ h)
{
   return GetGdiObject(h) != nullptr;
}

/************************************************************************/
// Clipping regions (device coordinates)

INT WINAPI GetClipRgn(HDC hdc, HRGN hrgn)
{
   DC *dc = GetDCObject(hdc);
   Region *r = GetRegion(hrgn);
   if (dc == nullptr || r == nullptr)
      return -1;
   if (!dc->has_clip)
      return 0;
   *r = dc->clip;
   POINT device = DeviceOrigin(dc);
   r->Offset(-device.x, -device.y);
   return 1;
}

INT WINAPI ExtSelectClipRgn(HDC hdc, HRGN hrgn, INT mode)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return ERROR;
   if (mode == RGN_COPY)
      return SelectClipRgn(hdc, hrgn);
   Region *r = GetRegion(hrgn);
   if (r == nullptr)
      return ERROR;
   POINT device = DeviceOrigin(dc);
   Region add = *r;
   add.Offset(device.x, device.y);
   RasterTarget t;
   Region current = dc->has_clip ? dc->clip
                                 : (GetTarget(dc, &t) ? Region(RECT{0, 0, t.width, t.height}) : Region(RECT{0, 0, 32767, 32767}));
   switch (mode)
   {
   case RGN_AND: current.Intersect(add); break;
   case RGN_OR: current.Add(add); break;
   case RGN_DIFF: current.Subtract(add); break;
   case RGN_XOR:
   {
      Region both = current;
      both.Intersect(add);
      current.Add(add);
      current.Subtract(both);
      break;
   }
   default: return ERROR;
   }
   dc->clip = current;
   dc->has_clip = true;
   return current.Empty() ? NULLREGION : (current.Rects().size() == 1 ? SIMPLEREGION : COMPLEXREGION);
}

/************************************************************************/
// Bitmaps and blending

INT WINAPI GetDIBits(HDC hdc, HBITMAP hbitmap, UINT start, UINT lines, LPVOID bits, BITMAPINFO *bi, UINT usage)
{
   GdiBitmap *b = GetGdiObjectAs<GdiBitmap>(hbitmap, OBJ_BITMAP);
   if (b == nullptr || bi == nullptr)
      return 0;
   BITMAPINFOHEADER &h = bi->bmiHeader;
   if (bits == nullptr && h.biBitCount == 0)
   {
      // The bitmap's format.
      h.biWidth = b->width;
      h.biHeight = b->bottom_up ? b->height : -b->height;
      h.biPlanes = 1;
      h.biBitCount = (WORD) b->bpp;
      h.biCompression = BI_RGB;
      h.biSizeImage = (DWORD) (b->stride * b->height);
      return 1;
   }
   if (bits == nullptr)
      return (INT) lines;
   // Converts to the requested 24- or 32-bit layout (or copies same-format rows).
   int bpp = h.biBitCount;
   bool bottom_up = h.biHeight > 0;
   int stride = ((b->width * bpp + 31) / 32) * 4;
   RasterTarget src = {b->bits, b->width, b->height, b->stride, b->bpp, b->bottom_up,
                       b->colors.empty() ? nullptr : b->colors.data(), (int) b->colors.size()};
   if (bpp != 24 && bpp != 32 && bpp != b->bpp)
      return 0;
   if (bpp <= 8 && b->bpp == bpp && usage == DIB_RGB_COLORS)
      for (size_t i = 0; i < b->colors.size(); i++)
         bi->bmiColors[i] = b->colors[i];
   UINT done = 0;
   for (UINT n = 0; n < lines; n++)
   {
      // Scan line start + n, counted from the bottom for bottom-up requests.
      int line = (int) (start + n);
      if (line >= b->height)
         break;
      int y = bottom_up ? b->height - 1 - line : line;
      uint8_t *out = (uint8_t *) bits + (size_t) n * stride;
      if (bpp == b->bpp)
         memcpy(out, src.Row(y), std::min(stride, b->stride));
      else
         for (int x = 0; x < b->width; x++)
         {
            uint32_t v = Read(src, x, y);
            COLORREF c = Rgb(src, v);
            uint8_t a = b->bpp == 32 ? (uint8_t) (v >> 24) : 0;
            uint8_t *p = out + x * (bpp / 8);
            p[0] = GetBValue(c), p[1] = GetGValue(c), p[2] = GetRValue(c);
            if (bpp == 32)
               p[3] = a;
         }
      done++;
   }
   (void) hdc;
   return (INT) done;
}

BOOL WINAPI GdiAlphaBlend(HDC dest, int dx, int dy, int dw, int dh, HDC source, int sx, int sy, int sw, int sh,
                          BLENDFUNCTION blend)
{
   DC *sdc = GetDCObject(source);
   RasterTarget s;
   if (sdc == nullptr || !GetTarget(sdc, &s) || dw <= 0 || dh <= 0 || sw <= 0 || sh <= 0)
      return FALSE;
   sx += sdc->origin.x;
   sy += sdc->origin.y;
   bool per_pixel = (blend.AlphaFormat & AC_SRC_ALPHA) && s.bpp == 32;
   int constant = blend.SourceConstantAlpha;
   return ForEachPixel(dest, RECT{dx, dy, dx + dw, dy + dh}, [&](const RasterTarget &t, int x, int y, int lx, int ly) {
      int px = sx + lx * sw / dw, py = sy + ly * sh / dh;
      if (px < 0 || py < 0 || px >= s.width || py >= s.height)
         return;
      uint32_t v = Read(s, px, py);
      COLORREF c = Rgb(s, v);
      int r = GetRValue(c), g = GetGValue(c), b = GetBValue(c);
      if (per_pixel)
      {
         // Premultiplied source: out = src * k + dst * (1 - srcA * k).
         int a = (int) (v >> 24) * constant / 255;
         if (a == 0 && r == 0 && g == 0 && b == 0)
            return;
         COLORREF d = Rgb(t, Read(t, x, y));
         int nr = std::min(255, r * constant / 255 + GetRValue(d) * (255 - a) / 255);
         int ng = std::min(255, g * constant / 255 + GetGValue(d) * (255 - a) / 255);
         int nb = std::min(255, b * constant / 255 + GetBValue(d) * (255 - a) / 255);
         Write(t, x, y, PixelValue(t, RGB(nr, ng, nb)));
      }
      else
         Blend(t, x, y, r, g, b, constant);
   });
}

BOOL WINAPI AlphaBlend(HDC dest, int dx, int dy, int dw, int dh, HDC source, int sx, int sy, int sw, int sh,
                       BLENDFUNCTION blend)
{
   return GdiAlphaBlend(dest, dx, dy, dw, dh, source, sx, sy, sw, sh, blend);
}

BOOL WINAPI PlayEnhMetaFile(HDC, HENHMETAFILE, const RECT *)
{
   // There are no metafiles (SS_ENHMETAFILE statics draw nothing).
   return FALSE;
}

/************************************************************************/
// Icons

BOOL WINAPI DrawIconEx(HDC hdc, INT x, INT y, HICON icon, INT cx, INT cy, UINT step, HBRUSH background, UINT flags)
{
   (void) step;
   const CursorImage *c = GetCursorImage(icon);
   if (c == nullptr || GetDCObject(hdc) == nullptr)
      return FALSE;
   if (cx == 0)
      cx = (flags & DI_DEFAULTSIZE) ? GetSystemMetrics(SM_CXICON) : c->width;
   if (cy == 0)
      cy = (flags & DI_DEFAULTSIZE) ? GetSystemMetrics(SM_CYICON) : c->height;
   if (background)
   {
      RECT r = {x, y, x + cx, y + cy};
      FillRect(hdc, &r, background);
   }
   if (c->pixels.empty() || c->width <= 0 || c->height <= 0)
      return TRUE;   // a standard shape: nothing to draw
   bool mask_only = (flags & DI_NORMAL) == DI_MASK, image_only = (flags & DI_NORMAL) == DI_IMAGE;
   return ForEachPixel(hdc, RECT{x, y, x + cx, y + cy}, [&](const RasterTarget &t, int px, int py, int lx, int ly) {
      uint32_t p = c->pixels[(size_t) (ly * c->height / cy) * c->width + lx * c->width / cx];
      int a = (int) (p >> 24);
      if (mask_only)
      {
         if (a < 128)
            Write(t, px, py, PixelValue(t, RGB(0, 0, 0)));
         return;
      }
      if (image_only)
         a = 255;
      Blend(t, px, py, (p >> 16) & 0xFF, (p >> 8) & 0xFF, p & 0xFF, a);
   });
}

BOOL WINAPI DrawIcon(HDC hdc, INT x, INT y, HICON icon)
{
   return DrawIconEx(hdc, x, y, icon, 0, 0, 0, nullptr, DI_NORMAL | DI_DEFAULTSIZE);
}

BOOL WINAPI GetIconInfo(HICON icon, ICONINFO *info)
{
   const CursorImage *c = GetCursorImage(icon);
   if (c == nullptr || info == nullptr)
      return FALSE;
   int w = c->width ? c->width : GetSystemMetrics(SM_CXICON), h = c->height ? c->height : GetSystemMetrics(SM_CYICON);
   info->fIcon = c->hot_x == 0 && c->hot_y == 0;
   info->xHotspot = c->hot_x;
   info->yHotspot = c->hot_y;
   // The image (32 bpp with alpha) and the AND mask (1 where transparent).
   BITMAPINFO bi = {};
   bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
   bi.bmiHeader.biWidth = w;
   bi.bmiHeader.biHeight = -h;
   bi.bmiHeader.biPlanes = 1;
   bi.bmiHeader.biBitCount = 32;
   void *bits = nullptr;
   info->hbmColor = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
   info->hbmMask = CreateBitmap(w, h, 1, 1, nullptr);
   GdiBitmap *mask = GetGdiObjectAs<GdiBitmap>(info->hbmMask, OBJ_BITMAP);
   for (int y = 0; y < h; y++)
      for (int x = 0; x < w; x++)
      {
         uint32_t p = (int) c->pixels.size() == c->width * c->height && x < c->width && y < c->height
                         ? c->pixels[(size_t) y * c->width + x] : 0;
         ((uint32_t *) bits)[(size_t) y * w + x] = p;
         if (mask && (p >> 24) < 128)
            mask->Row(y)[x >> 3] |= (uint8_t) (0x80 >> (x & 7));
      }
   return TRUE;
}

HANDLE WINAPI CopyImage(HANDLE image, UINT type, INT cx, INT cy, UINT flags)
{
   if (type == IMAGE_ICON || type == IMAGE_CURSOR)
   {
      // Icons and cursors are shared images (DestroyIcon leaves them): the copy is
      // the same image.
      if (flags & LR_COPYDELETEORG)
         return image;
      return GetCursorImage(image) ? image : nullptr;
   }
   if (type != IMAGE_BITMAP)
      return nullptr;
   GdiBitmap *b = GetGdiObjectAs<GdiBitmap>(image, OBJ_BITMAP);
   if (b == nullptr)
      return nullptr;
   int w = cx ? cx : b->width, h = cy ? cy : b->height;
   HBITMAP copy;
   if (w == b->width && h == b->height)
   {
      copy = CreateBitmap(w, h, 1, b->bpp, nullptr);
      if (GdiBitmap *c = GetGdiObjectAs<GdiBitmap>(copy, OBJ_BITMAP))
      {
         c->colors = b->colors;
         for (int y = 0; y < h; y++)
            memcpy(c->Row(y), b->Row(y), std::min(c->stride, b->stride));
      }
   }
   else
   {
      // Scaled: through two memory DCs.
      HDC src = CreateCompatibleDC(nullptr), dst = CreateCompatibleDC(nullptr);
      copy = CreateBitmap(w, h, 1, b->bpp == 1 ? 1 : 32, nullptr);
      HGDIOBJ old_src = SelectObject(src, image), old_dst = SelectObject(dst, copy);
      StretchBlt(dst, 0, 0, w, h, src, 0, 0, b->width, b->height, SRCCOPY);
      SelectObject(src, old_src);
      SelectObject(dst, old_dst);
      DeleteDC(src);
      DeleteDC(dst);
   }
   if (flags & LR_COPYDELETEORG)
      DeleteObject(image);
   return copy;
}

/************************************************************************/
// Pie: the ellipse's sector from the start radial counterclockwise to the end one.

BOOL WINAPI Pie(HDC hdc, INT left, INT top, INT right, INT bottom, INT xstart, INT ystart, INT xend, INT yend)
{
   if (GetDCObject(hdc) == nullptr)
      return FALSE;
   double cx = (left + right) / 2.0, cy = (top + bottom) / 2.0;
   double rx = (right - left) / 2.0, ry = (bottom - top) / 2.0;
   if (rx <= 0 || ry <= 0)
      return TRUE;
   // Angles in the y-up sense GDI's counterclockwise uses.
   double a0 = atan2(-(ystart - cy), xstart - cx), a1 = atan2(-(yend - cy), xend - cx);
   if (a1 <= a0)
      a1 += 2 * M_PI;
   std::vector<POINT> pts;
   pts.push_back({(LONG) lround(cx), (LONG) lround(cy)});
   int steps = std::max(8, (int) ((a1 - a0) * std::max(rx, ry)));
   for (int i = 0; i <= steps; i++)
   {
      double a = a0 + (a1 - a0) * i / steps;
      pts.push_back({(LONG) lround(cx + rx * cos(a)), (LONG) lround(cy - ry * sin(a))});
   }
   return Polygon(hdc, pts.data(), (INT) pts.size());
}

/************************************************************************/
// Icons from bitmaps

HICON WINAPI CreateIconIndirect(ICONINFO *info)
{
   if (info == nullptr || (info->hbmColor == nullptr && info->hbmMask == nullptr))
      return nullptr;
   GdiBitmap *color = GetGdiObjectAs<GdiBitmap>(info->hbmColor, OBJ_BITMAP);
   GdiBitmap *mask = GetGdiObjectAs<GdiBitmap>(info->hbmMask, OBJ_BITMAP);
   GdiBitmap *size = color ? color : mask;
   if (size == nullptr)
      return nullptr;
   auto *c = new CursorImage;
   c->width = size->width;
   // A monochrome icon without color: the mask's top half is AND, its bottom XOR.
   c->height = color ? size->height : size->height / 2;
   c->hot_x = info->fIcon ? c->width / 2 : (int) info->xHotspot;
   c->hot_y = info->fIcon ? c->height / 2 : (int) info->yHotspot;
   c->pixels.resize((size_t) c->width * c->height);
   RasterTarget ct = {}, mt = {};
   if (color)
      ct = {color->bits, color->width, color->height, color->stride, color->bpp, color->bottom_up,
            color->colors.empty() ? nullptr : color->colors.data(), (int) color->colors.size()};
   if (mask)
      mt = {mask->bits, mask->width, mask->height, mask->stride, mask->bpp, mask->bottom_up,
            mask->colors.empty() ? nullptr : mask->colors.data(), (int) mask->colors.size()};
   // 32-bit color with any alpha: per-pixel (premultiplied) alpha; else the mask's.
   bool alpha = false;
   if (color && color->bpp == 32)
      for (int y = 0; y < c->height && !alpha; y++)
         for (int x = 0; x < c->width && !alpha; x++)
            alpha = (Read(ct, x, y) >> 24) != 0;
   for (int y = 0; y < c->height; y++)
      for (int x = 0; x < c->width; x++)
      {
         bool transparent = mask && y < mask->height && x < mask->width && Read(mt, x, y) != 0;
         uint32_t argb;
         if (color)
         {
            uint32_t v = Read(ct, x, y);
            COLORREF rgb = Rgb(ct, v);
            int a = alpha ? (int) (v >> 24) : (transparent ? 0 : 255);
            int r = GetRValue(rgb), g = GetGValue(rgb), b = GetBValue(rgb);
            if (alpha && a)
               r = std::min(255, r * 255 / a), g = std::min(255, g * 255 / a), b = std::min(255, b * 255 / a);
            argb = ((uint32_t) a << 24) | (r << 16) | (g << 8) | b;
         }
         else
            argb = transparent ? 0 : (Read(mt, x, y + c->height) ? 0xFFFFFFFFu : 0xFF000000u);
         c->pixels[(size_t) y * c->width + x] = argb;
      }
   return (HICON) AddCursorImage(c);
}

HICON WINAPI CopyIcon(HICON icon)
{
   return GetCursorImage(icon) ? icon : nullptr;
}
