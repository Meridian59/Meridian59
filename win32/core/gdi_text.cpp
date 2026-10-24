/*
 * gdi_text.cpp:  Text output and measurement.  The backend rasterizes runs into
 *   coverage masks; they are composited here in the text color, onto 8-bit
 *   targets as the nearest palette color, opaque or not as the background mode.
 */

#include "core/fontmap.h"
#include "core/gdi.h"

#include "core/native.h"

#include <algorithm>
#include <cstring>

namespace w32 {

namespace {

// One pixel of row (target t) to v, for the formats without blending.
void PutPixel(const RasterTarget &t, uint8_t *row, int x, uint32_t v)
{
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

native::Font *FontOf(DC *dc)
{
   GdiFont *f = GetGdiObjectAs<GdiFont>(dc->font, OBJ_FONT);
   if (f == nullptr)
      f = GetGdiObjectAs<GdiFont>(GetStockObject(SYSTEM_FONT), OBJ_FONT);
   if (f->font == nullptr)
      f->font = native::CreateFont(f->log);
   return f->font;
}

// Draws utf8 with its top-left at (x, y) (target coordinates).
void DrawRun(DC *dc, const RasterTarget &t, const Region &clip, int x, int y, const std::string &utf8, RECT *drawn)
{
   int w = 0, h = 0;
   std::vector<uint8_t> mask = native::RenderText(FontOf(dc), utf8, &w, &h);
   if (w <= 0 || h <= 0)
      return;
   COLORREF fg = ResolveColor(dc, dc->text_color);
   uint32_t fg_pixel = PixelValue(t, fg);
   BYTE fr = GetRValue(fg), fgc = GetGValue(fg), fb = GetBValue(fg);
   for (const RECT &r : clip.Rects())
   {
      int x0 = std::max((int) r.left, x), x1 = std::min((int) r.right, x + w);
      int y0 = std::max((int) r.top, y), y1 = std::min((int) r.bottom, y + h);
      for (int py = y0; py < y1; py++)
      {
         uint8_t *row = t.Row(py);
         const uint8_t *m = &mask[(size_t) (py - y) * w];
         for (int px = x0; px < x1; px++)
         {
            int a = m[px - x];
            if (a == 0)
               continue;
            if (t.bpp == 32)
            {
               uint8_t *p = row + px * 4;
               // Blend in the text color by coverage.
               p[0] = (uint8_t) ((p[0] * (255 - a) + fb * a) / 255);
               p[1] = (uint8_t) ((p[1] * (255 - a) + fgc * a) / 255);
               p[2] = (uint8_t) ((p[2] * (255 - a) + fr * a) / 255);
               p[3] = 0xFF;
            }
            else if (t.bpp == 24)
            {
               uint8_t *p = row + px * 3;
               p[0] = (uint8_t) ((p[0] * (255 - a) + fb * a) / 255);
               p[1] = (uint8_t) ((p[1] * (255 - a) + fgc * a) / 255);
               p[2] = (uint8_t) ((p[2] * (255 - a) + fr * a) / 255);
            }
            else if (a >= 128)
            {
               // Palettized targets can't blend: text is solid where mostly covered.
               if (t.bpp == 8)
                  row[px] = (uint8_t) fg_pixel;
               else if (t.bpp == 1)
                  PutPixel(t, row, px, fg_pixel);
            }
         }
      }
      RECT d = {x0, y0, x1, y1};
      if (x0 < x1 && y0 < y1)
         UnionRect(drawn, drawn, &d);
   }
}

int TextHeight(DC *dc)
{
   return native::GetFontMetrics(FontOf(dc)).height;
}

}  // namespace

}  // namespace w32

using namespace w32;

BOOL WINAPI ExtTextOutA(HDC hdc, INT x, INT y, UINT options, const RECT *rect, LPCSTR text, UINT count, const INT *dx)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return FALSE;
   RasterTarget t;
   bool target = !dc->info_only && GetTarget(dc, &t);
   if (dc->text_align & TA_UPDATECP)
      x = dc->pos.x, y = dc->pos.y;

   std::string utf8 = AnsiToUtf8(text ? text : "", text ? (int) count : 0);
   native::Font *font = FontOf(dc);
   native::FontMetrics fm = native::GetFontMetrics(font);
   int width;
   if (dx)
   {
      width = 0;
      for (UINT i = 0; i < count; i++)
         width += dx[i];
   }
   else
      width = native::TextWidth(font, utf8);

   // Alignment: the reference point is the text's left/center/right and
   // top/baseline/bottom.
   int left = x, top = y;
   switch (dc->text_align & (TA_LEFT | TA_RIGHT | TA_CENTER))
   {
   case TA_RIGHT: left = x - width; break;
   case TA_CENTER: left = x - width / 2; break;
   }
   switch (dc->text_align & (TA_TOP | TA_BOTTOM | TA_BASELINE))
   {
   case TA_BOTTOM: top = y - fm.height; break;
   case TA_BASELINE: top = y - fm.ascent; break;
   }
   if (dc->text_align & TA_UPDATECP)
      dc->pos.x = (dc->text_align & TA_RIGHT) ? left : left + width;

   if (!target)
      return TRUE;
   Region clip = EffectiveClip(dc, t);
   RECT drawn = {0, 0, 0, 0};
   int ox = dc->origin.x, oy = dc->origin.y;

   if (rect && (options & ETO_CLIPPED))
   {
      RECT r = *rect;
      OffsetRect(&r, ox, oy);
      clip.Intersect(r);
   }
   // The background: the rectangle (ETO_OPAQUE), else the text's cell when opaque.
   RECT bg = {0, 0, 0, 0};
   if (rect && (options & ETO_OPAQUE))
      bg = *rect;
   else if (dc->bk_mode == OPAQUE)
      bg = {left, top, left + width, top + fm.height};
   if (!IsRectEmpty(&bg))
   {
      OffsetRect(&bg, ox, oy);
      Region fill = clip;
      fill.Intersect(bg);
      uint32_t v = PixelValue(t, ResolveColor(dc, dc->bk_color));
      for (const RECT &r : fill.Rects())
         for (int py = r.top; py < r.bottom; py++)
            for (int px = r.left; px < r.right; px++)
               PutPixel(t, t.Row(py), px, v);
      UnionRect(&drawn, &drawn, &bg);
   }

   if (dx)
   {
      // Each character at its own advance.
      int px = left;
      for (UINT i = 0; i < count; i++)
      {
         DrawRun(dc, t, clip, px + ox, top + oy, AnsiToUtf8(text + i, 1), &drawn);
         px += dx[i];
      }
   }
   else
      DrawRun(dc, t, clip, left + ox, top + oy, utf8, &drawn);

   if (fm.underline || fm.strike_out)
   {
      uint32_t v = PixelValue(t, ResolveColor(dc, dc->text_color));
      int ly = top + oy + (fm.underline ? fm.ascent + 1 : fm.ascent * 2 / 3);
      RECT line = {left + ox, ly, left + ox + width, ly + 1};
      Region lr = clip;
      lr.Intersect(line);
      for (const RECT &r : lr.Rects())
         for (int px = r.left; px < r.right; px++)
            PutPixel(t, t.Row(r.top), px, v);
   }
   if (!IsRectEmpty(&drawn))
      ShowTargetChanges(dc, drawn);
   return TRUE;
}

BOOL WINAPI TextOutA(HDC hdc, INT x, INT y, LPCSTR text, INT count)
{
   return ExtTextOutA(hdc, x, y, 0, nullptr, text, count < 0 ? 0 : count, nullptr);
}

BOOL WINAPI GetTextExtentPoint32A(HDC hdc, LPCSTR text, INT count, SIZE *size)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || size == nullptr)
      return FALSE;
   native::Font *font = FontOf(dc);
   size->cx = native::TextWidth(font, AnsiToUtf8(text ? text : "", text ? count : 0));
   size->cy = native::GetFontMetrics(font).height;
   return TRUE;
}

BOOL WINAPI GetTextExtentPointA(HDC hdc, LPCSTR text, INT count, SIZE *size)
{
   return GetTextExtentPoint32A(hdc, text, count, size);
}

BOOL WINAPI GetTextMetricsA(HDC hdc, TEXTMETRICA *tm)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || tm == nullptr)
      return FALSE;
   native::FontMetrics fm = native::GetFontMetrics(FontOf(dc));
   memset(tm, 0, sizeof(*tm));
   tm->tmHeight = fm.height;
   tm->tmAscent = fm.ascent;
   tm->tmDescent = fm.descent;
   tm->tmInternalLeading = fm.internal_leading;
   tm->tmExternalLeading = fm.external_leading;
   tm->tmAveCharWidth = fm.average_width;
   tm->tmMaxCharWidth = fm.max_width;
   tm->tmWeight = fm.weight;
   tm->tmItalic = fm.italic;
   tm->tmUnderlined = fm.underline;
   tm->tmStruckOut = fm.strike_out;
   tm->tmFirstChar = 32;
   tm->tmLastChar = 255;
   tm->tmDefaultChar = '?';
   tm->tmBreakChar = ' ';
   tm->tmPitchAndFamily = TMPF_VECTOR | TMPF_TRUETYPE | TMPF_FIXED_PITCH;   // FIXED_PITCH set means variable
   tm->tmCharSet = ANSI_CHARSET;
   // A raster face's size (core/fontmap.h): no vector bits, its own pitch and
   // character set, and the emboldened's overhang.
   GdiFont *f = GetGdiObjectAs<GdiFont>(dc->font, OBJ_FONT);
   RealizedFont w = f ? RealizeFont(f->log) : RealizedFont{};
   if (!w.raster.empty())
   {
      bool fixed = w.raster == "Courier" || w.raster == "Fixedsys" || w.raster == "Terminal";
      tm->tmPitchAndFamily = fixed ? 0 : TMPF_FIXED_PITCH;
      if (w.raster == "Terminal")
         tm->tmCharSet = OEM_CHARSET;
   }
   tm->tmOverhang = w.overhang;
   return TRUE;
}

INT WINAPI GetTextFaceA(HDC hdc, INT size, LPSTR name)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return 0;
   GdiFont *f = GetGdiObjectAs<GdiFont>(dc->font, OBJ_FONT);
   // The face realized, as Windows' (a missing one's substitute).
   std::string realized = f ? RealizeFont(f->log).face : "System";
   const char *face = realized.c_str();
   if (name == nullptr)
      return (INT) strlen(face) + 1;
   lstrcpynA(name, face, size);
   return (INT) strlen(name);
}

BOOL WINAPI GetCharABCWidthsA(HDC hdc, UINT first, UINT last, ABC *abc)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || abc == nullptr || last < first)
      return FALSE;
   native::Font *font = FontOf(dc);
   for (UINT ch = first; ch <= last; ch++)
   {
      char c = (char) ch;
      std::string u = AnsiToUtf8(&c, 1);
      // The code point of the Windows-1252 character.
      uint32_t cp = 0;
      unsigned char b0 = u.empty() ? 0 : (unsigned char) u[0];
      if (b0 < 0x80)
         cp = b0;
      else if ((b0 & 0xE0) == 0xC0 && u.size() >= 2)
         cp = ((b0 & 0x1F) << 6) | (u[1] & 0x3F);
      else if ((b0 & 0xF0) == 0xE0 && u.size() >= 3)
         cp = ((b0 & 0x0F) << 12) | ((u[1] & 0x3F) << 6) | (u[2] & 0x3F);
      int a, b, cc;
      native::CharWidths(font, cp, &a, &b, &cc);
      abc[ch - first] = {a, (UINT) std::max(b, 0), cc};
   }
   return TRUE;
}

BOOL WINAPI GetCharWidth32A(HDC hdc, UINT first, UINT last, INT *widths)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || widths == nullptr || last < first)
      return FALSE;
   native::Font *font = FontOf(dc);
   for (UINT ch = first; ch <= last; ch++)
   {
      char c = (char) ch;
      widths[ch - first] = native::TextWidth(font, AnsiToUtf8(&c, 1));
   }
   return TRUE;
}

DWORD WINAPI GetKerningPairsA(HDC hdc, DWORD count, KERNINGPAIR *pairs)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return 0;
   // The printable Windows-1252 characters' pairs that kern.  Computed once per
   // font: the client asks twice (count, then the pairs).
   native::Font *font = FontOf(dc);
   static native::Font *cached_font = nullptr;
   static std::vector<KERNINGPAIR> cached;
   if (cached_font != font)
   {
      cached.clear();
      cached_font = font;
      for (int a = 32; a < 127; a++)
         for (int b = 32; b < 127; b++)
            if (int k = native::Kerning(font, (uint32_t) a, (uint32_t) b))
               cached.push_back({(WORD) a, (WORD) b, k});
   }
   if (pairs == nullptr)
      return (DWORD) cached.size();
   DWORD n = std::min<DWORD>(count, (DWORD) cached.size());
   std::copy(cached.begin(), cached.begin() + n, pairs);
   return n;
}

/************************************************************************/
// DrawText

namespace {

struct Line
{
   std::string text;      // Windows-1252, prefix ampersands removed
   int underline = -1;    // index of the prefixed character, or -1
};

std::vector<Line> SplitLines(DC *dc, const std::string &text, UINT flags, int width)
{
   bool prefix = !(flags & DT_NOPREFIX);
   std::vector<Line> lines;
   Line cur;
   auto measure = [&](const std::string &s) { return native::TextWidth(FontOf(dc), AnsiToUtf8(s.c_str(), (int) s.size())); };
   auto push = [&] {
      lines.push_back(cur);
      cur = Line();
   };
   for (size_t i = 0; i < text.size(); i++)
   {
      char c = text[i];
      if (prefix && c == '&')
      {
         if (i + 1 < text.size() && text[i + 1] == '&')
         {
            cur.text += '&';
            i++;
         }
         else if (!(flags & DT_HIDEPREFIX))
            cur.underline = (int) cur.text.size();
         continue;
      }
      if (!(flags & DT_SINGLELINE) && (c == '\n' || c == '\r'))
      {
         if (c == '\r' && i + 1 < text.size() && text[i + 1] == '\n')
            i++;
         push();
         continue;
      }
      if (c == '\t' && (flags & DT_EXPANDTABS))
      {
         cur.text += "        ";
         continue;
      }
      cur.text += c;
      if ((flags & DT_WORDBREAK) && !(flags & DT_SINGLELINE) && width > 0 && measure(cur.text) > width)
      {
         // Break at the last space; a word longer than the line stays whole.
         size_t space = cur.text.find_last_of(' ');
         if (space != std::string::npos && space > 0)
         {
            std::string rest = cur.text.substr(space + 1);
            cur.text.resize(space);
            push();
            cur.text = rest;
         }
      }
   }
   push();
   return lines;
}

}  // namespace

INT WINAPI DrawTextA(HDC hdc, LPCSTR text, INT count, RECT *rect, UINT flags)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || text == nullptr || rect == nullptr)
      return 0;
   std::string s = count < 0 ? std::string(text) : std::string(text, count);
   native::Font *font = FontOf(dc);
   int line_height = TextHeight(dc);
   int width = rect->right - rect->left;
   std::vector<Line> lines = SplitLines(dc, s, flags, width);

   int text_width = 0;
   for (const Line &l : lines)
      text_width = std::max(text_width, native::TextWidth(font, AnsiToUtf8(l.text.c_str(), (int) l.text.size())));
   int total = line_height * (int) lines.size();

   if (flags & DT_CALCRECT)
   {
      rect->right = rect->left + ((flags & DT_WORDBREAK) && !(flags & DT_SINGLELINE) && width > 0
                                      ? std::min(width, text_width) : text_width);
      rect->bottom = rect->top + total;
      return total;
   }

   int y = rect->top;
   if (flags & DT_SINGLELINE)
   {
      if (flags & DT_VCENTER)
         y = rect->top + (rect->bottom - rect->top - line_height) / 2;
      else if (flags & DT_BOTTOM)
         y = rect->bottom - line_height;
   }
   UINT saved_align = dc->text_align;
   dc->text_align = TA_TOP | TA_LEFT;
   for (const Line &l : lines)
   {
      std::string line = l.text;
      int w = native::TextWidth(font, AnsiToUtf8(line.c_str(), (int) line.size()));
      if ((flags & DT_END_ELLIPSIS) && w > width)
      {
         while (!line.empty() && native::TextWidth(font, AnsiToUtf8((line + "...").c_str())) > width)
            line.pop_back();
         line += "...";
         w = native::TextWidth(font, AnsiToUtf8(line.c_str()));
      }
      int x = rect->left;
      if (flags & DT_CENTER)
         x = rect->left + (width - w) / 2;
      else if (flags & DT_RIGHT)
         x = rect->right - w;
      ExtTextOutA(hdc, x, y, (flags & DT_NOCLIP) ? 0 : ETO_CLIPPED, rect, line.c_str(), (UINT) line.size(), nullptr);
      if (l.underline >= 0 && l.underline < (int) line.size())
      {
         int ux = x + native::TextWidth(font, AnsiToUtf8(line.c_str(), l.underline));
         int uw = native::TextWidth(font, AnsiToUtf8(line.c_str() + l.underline, 1));
         HPEN pen = CreatePen(PS_SOLID, 1, dc->text_color);
         HGDIOBJ old = SelectObject(hdc, pen);
         int uy = y + native::GetFontMetrics(font).ascent + 1;
         MoveToEx(hdc, ux, uy, nullptr);
         LineTo(hdc, ux + uw, uy);
         SelectObject(hdc, old);
         DeleteObject(pen);
      }
      y += line_height;
   }
   dc->text_align = saved_align;
   return total;
}
