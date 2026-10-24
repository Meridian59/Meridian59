/*
 * text.mm:  Fonts and text on Core Text.  LOGFONT heights are Windows': negative
 *   for the characters' height (the em), positive for the cell's (ascent plus
 *   descent); sizes are pixels (Core Text's points at 72 per inch are our pixels).
 */

#include "appkit/appkit_backend.h"

#import <CoreText/CoreText.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <strings.h>

namespace w32::native {

class Font
{
public:
   CTFontRef font = nullptr;
   mac::FontMetrics metrics;
   std::map<std::pair<uint32_t, uint32_t>, int> kerning;
};

}  // namespace w32::native

namespace w32::mac {

namespace {

// The installed face standing in for a Windows face name, or nil for a face
// macOS has as it is (most of Windows' own).
NSString *KnownFamily(const std::string &face)
{
   struct Map
   {
      const char *windows, *family;
   };
   static const Map maps[] = {
      // The raster faces are drawn from their own fonts (core/rasterfont.cpp);
      // these are for sizes those fonts lack.
      {"MS Sans Serif", "Microsoft Sans Serif"}, {"MS Shell Dlg", "Microsoft Sans Serif"},
      {"MS Shell Dlg 2", "Tahoma"},              {"System", "Microsoft Sans Serif"},
      {"Courier", "Courier New"},                {"Terminal", "Courier New"},
      {"Fixedsys", "Courier New"},
      {"Segoe UI", "Helvetica Neue"},
      {"Helvetica", "Arial"},
   };
   for (const Map &m : maps)
      if (strcasecmp(face.c_str(), m.windows) == 0)
         return [NSString stringWithUTF8String:m.family];
   return nil;
}

// The installed face for a Windows face name (RealizeFont replaces the others).
NSString *Family(const std::string &face, bool fixed_pitch)
{
   if (NSString *known = KnownFamily(face))
      return known;
   if (!face.empty())
      return [NSString stringWithUTF8String:face.c_str()] ?: @"Helvetica";
   return fixed_pitch ? @"Courier New" : @"Arial";
}

CTFontRef StyledFont(NSString *family, CGFloat size, int weight, bool italic)
{
   CTFontSymbolicTraits traits = (weight >= 600 ? kCTFontBoldTrait : 0) | (italic ? kCTFontItalicTrait : 0);
   CTFontRef base = CTFontCreateWithName((CFStringRef) family, size, nullptr);
   if (traits == 0)
      return base;
   CTFontRef styled = CTFontCreateCopyWithSymbolicTraits(base, size, nullptr, traits, traits);
   if (styled == nullptr)
      return base;
   CFRelease(base);
   return styled;
}

// What GDI computes metrics from (OS/2 and hhea), in font units.
struct Tables
{
   bool ok = false;
   double upem = 2048;
   int win_ascent = 0, win_descent = 0, avg_width = 0;
   int hhea_ascender = 0, hhea_descender = 0, line_gap = 0, max_advance = 0;
};

int Be16(const uint8_t *p, CFIndex offset, CFIndex length, bool is_signed)
{
   if (offset + 2 > length)
      return 0;
   unsigned v = (p[offset] << 8) | p[offset + 1];
   return is_signed ? (int) (int16_t) v : (int) v;
}

Tables ReadTables(CTFontRef font)
{
   Tables t;
   t.upem = CTFontGetUnitsPerEm(font);
   if (CFDataRef os2 = CTFontCopyTable(font, kCTFontTableOS2, kCTFontTableOptionNoOptions))
   {
      CFIndex n = CFDataGetLength(os2);
      const uint8_t *p = CFDataGetBytePtr(os2);
      if (n >= 78)
      {
         t.avg_width = Be16(p, 2, n, true);
         t.win_ascent = Be16(p, 74, n, false);
         t.win_descent = Be16(p, 76, n, false);
         t.ok = t.win_ascent + t.win_descent > 0;
      }
      CFRelease(os2);
   }
   if (CFDataRef hhea = CTFontCopyTable(font, kCTFontTableHhea, kCTFontTableOptionNoOptions))
   {
      CFIndex n = CFDataGetLength(hhea);
      const uint8_t *p = CFDataGetBytePtr(hhea);
      t.hhea_ascender = Be16(p, 4, n, true);
      t.hhea_descender = Be16(p, 6, n, true);
      t.line_gap = Be16(p, 8, n, true);
      t.max_advance = Be16(p, 10, n, false);
      CFRelease(hhea);
   }
   // GDI's tmAveCharWidth is xAvgCharWidth, which Windows' fonts carry as OS/2
   // version 1's weighted average of the lowercase letters and space.
   static const int weights[27] = {64, 14, 27, 35, 100, 20, 14, 42, 63, 3, 6, 35, 20, 56,
                                   56, 17, 4,  49, 56, 71,  31, 10, 18, 3,  18, 2, 166};
   UniChar chars[27];
   CGGlyph glyphs[27];
   for (int i = 0; i < 27; i++)
      chars[i] = i < 26 ? 'a' + i : ' ';
   if (CTFontGetGlyphsForCharacters(font, chars, glyphs, 27))
   {
      CGSize advances[27];
      CTFontGetAdvancesForGlyphs(font, kCTFontOrientationHorizontal, glyphs, advances, 27);
      double size = CTFontGetSize(font);
      long total = 0;
      for (int i = 0; i < 27; i++)
         total += std::lround(advances[i].width * t.upem / size) * weights[i];
      t.avg_width = (int) (total / 1000);
   }
   return t;
}

// GDI's text metrics for the em size em (pixels), as Wine computes them.
void Measure(Font *f, const Tables &t, double em)
{
   if (!t.ok)
   {
      f->metrics.ascent = (int) std::lround(CTFontGetAscent(f->font));
      f->metrics.descent = (int) std::lround(CTFontGetDescent(f->font));
   }
   else
   {
      f->metrics.ascent = (int) std::lround(t.win_ascent * em / t.upem);
      f->metrics.descent = (int) std::lround(t.win_descent * em / t.upem);
   }
   double scale = em / t.upem;
   f->metrics.height = f->metrics.ascent + f->metrics.descent;
   f->metrics.internal_leading = std::max(0, f->metrics.height - (int) std::lround(em));
   int external = t.line_gap - ((t.win_ascent + t.win_descent) - (t.hhea_ascender - t.hhea_descender));
   f->metrics.external_leading = t.ok ? std::max(0, (int) std::lround(external * scale)) : 0;
   f->metrics.average_width = std::max(1, (int) std::lround(t.avg_width * scale));
   f->metrics.max_width = std::max(f->metrics.average_width, (int) std::lround(t.max_advance * scale));
}

std::string Utf8(uint32_t cp)
{
   std::string out;
   if (cp < 0x80)
      out += (char) cp;
   else if (cp < 0x800)
      out += (char) (0xC0 | (cp >> 6)), out += (char) (0x80 | (cp & 0x3F));
   else if (cp < 0x10000)
      out += (char) (0xE0 | (cp >> 12)), out += (char) (0x80 | ((cp >> 6) & 0x3F)), out += (char) (0x80 | (cp & 0x3F));
   else
      out += (char) (0xF0 | (cp >> 18)), out += (char) (0x80 | ((cp >> 12) & 0x3F)),
         out += (char) (0x80 | ((cp >> 6) & 0x3F)), out += (char) (0x80 | (cp & 0x3F));
   return out;
}

CTLineRef Line(Font *f, const std::string &utf8)
{
   NSString *s = [[NSString alloc] initWithBytes:utf8.data() length:utf8.size() encoding:NSUTF8StringEncoding];
   if (s == nil)
      s = [[NSString alloc] initWithString:@""];
   NSDictionary *attrs = @{(id) kCTFontAttributeName : (id) f->font};
   NSAttributedString *a = [[NSAttributedString alloc] initWithString:s attributes:attrs];
   CTLineRef line = CTLineCreateWithAttributedString((CFAttributedStringRef) a);
   [a release];
   [s release];
   return line;
}

}  // namespace

bool FaceInstalled(const std::string &face)
{
   if (KnownFamily(face))
      return true;
   NSString *name = [NSString stringWithUTF8String:face.c_str()];
   if (name == nil)
      return false;
   CTFontDescriptorRef want =
      CTFontDescriptorCreateWithAttributes((CFDictionaryRef) @{(id) kCTFontFamilyNameAttribute: name});
   CTFontDescriptorRef found = CTFontDescriptorCreateMatchingFontDescriptor(want, nullptr);
   CFRelease(want);
   if (found)
      CFRelease(found);
   return found != nullptr;
}

Font *MakeFont(const FontSpec &spec)
{
   @autoreleasepool
   {
      auto *f = new Font;
      NSString *family = Family(spec.face, spec.fixed_pitch);
      int height = spec.height == 0 ? -13 : spec.height;
      f->font = StyledFont(family, std::abs(height), spec.weight, spec.italic);
      Tables t = ReadTables(f->font);
      double em = std::abs(height);
      if (height > 0 && t.ok)
      {
         // A cell height: the em whose win ascent plus descent is it (GDI's ppem).
         em = height * t.upem / (t.win_ascent + t.win_descent);
         CFRelease(f->font);
         f->font = StyledFont(family, em, spec.weight, spec.italic);
      }
      Measure(f, t, em);
      f->metrics.weight = spec.weight;
      if (spec.windows_metrics)
      {
         int max_width = f->metrics.max_width;
         f->metrics = spec.metrics;
         if (f->metrics.max_width == 0)
            f->metrics.max_width = max_width;
      }
      f->metrics.italic = spec.italic;
      f->metrics.underline = spec.underline;
      f->metrics.strike_out = spec.strike_out;
      f->metrics.face = spec.windows_metrics ? spec.metrics.face : spec.face;
      return f;
   }
}

void DestroyFont(Font *f)
{
   if (f == nullptr)
      return;
   CFRelease(f->font);
   delete f;
}

FontMetrics GetFontMetrics(Font *f)
{
   return f->metrics;
}

int TextWidth(Font *f, const std::string &utf8)
{
   if (utf8.empty())
      return 0;
   @autoreleasepool
   {
      CTLineRef line = Line(f, utf8);
      double width = CTLineGetTypographicBounds(line, nullptr, nullptr, nullptr);
      CFRelease(line);
      return (int) std::lround(width);
   }
}

std::vector<int> CharEdges(Font *f, const std::string &utf8)
{
   std::vector<int> edges;
   if (utf8.empty())
      return edges;
   @autoreleasepool
   {
      CTLineRef line = Line(f, utf8);
      // UTF-16 index after each code point.
      CFIndex index = 0;
      for (size_t i = 0; i < utf8.size();)
      {
         unsigned char c = (unsigned char) utf8[i];
         size_t n = c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
         index += n == 4 ? 2 : 1;
         edges.push_back((int) std::lround(CTLineGetOffsetForStringIndex(line, index, nullptr)));
         i += n;
      }
      CFRelease(line);
   }
   return edges;
}

void CharWidths(Font *f, uint32_t cp, int *a, int *b, int *c)
{
   UniChar chars[2];
   CFIndex n = 1;
   if (cp > 0xFFFF)
   {
      cp -= 0x10000;
      chars[0] = (UniChar) (0xD800 + (cp >> 10));
      chars[1] = (UniChar) (0xDC00 + (cp & 0x3FF));
      n = 2;
   }
   else
      chars[0] = (UniChar) cp;
   CGGlyph glyphs[2] = {0, 0};
   CTFontGetGlyphsForCharacters(f->font, chars, glyphs, n);
   CGSize advance;
   CTFontGetAdvancesForGlyphs(f->font, kCTFontOrientationHorizontal, glyphs, &advance, 1);
   CGRect box = CTFontGetBoundingRectsForGlyphs(f->font, kCTFontOrientationHorizontal, glyphs, nullptr, 1);
   int width = (int) std::lround(advance.width);
   *a = box.size.width > 0 ? (int) std::floor(box.origin.x) : 0;
   *b = box.size.width > 0 ? (int) std::ceil(box.size.width) : 0;
   *c = width - *a - *b;
}

int Kerning(Font *f, uint32_t first, uint32_t second)
{
   auto key = std::make_pair(first, second);
   if (auto it = f->kerning.find(key); it != f->kerning.end())
      return it->second;
   std::string a = Utf8(first), b = Utf8(second);
   int k = TextWidth(f, a + b) - TextWidth(f, a) - TextWidth(f, b);
   f->kerning[key] = k;
   return k;
}

std::vector<uint8_t> RenderText(Font *f, const std::string &utf8, int *width, int *height)
{
   @autoreleasepool
   {
      CTLineRef line = Line(f, utf8);
      double advance = CTLineGetTypographicBounds(line, nullptr, nullptr, nullptr);
      CGRect ink = CTLineGetImageBounds(line, nullptr);
      // Room for what hangs past the advance (italics, bearings).
      int w = (int) std::max(std::ceil(advance), std::ceil(ink.origin.x + ink.size.width)) + 1;
      // Down to what hangs below the cell too (a stand-in's descenders), as GDI
      // draws it: clipped by the DC, not by the cell.
      CGRect glyphs = CTLineGetBoundsWithOptions(line, kCTLineBoundsUseGlyphPathBounds);
      int h = std::max(f->metrics.height, f->metrics.ascent + (int) std::ceil(-glyphs.origin.y) + 1);
      *width = w;
      *height = h;
      std::vector<uint8_t> mask((size_t) std::max(w, 0) * std::max(h, 0), 0);
      if (w <= 0 || h <= 0 || utf8.empty())
      {
         CFRelease(line);
         return mask;
      }
      // Alpha only; row 0 of the memory is the image's top.
      CGContextRef cg = CGBitmapContextCreate(mask.data(), w, h, 8, w, nullptr, (CGBitmapInfo) kCGImageAlphaOnly);
      CGContextSetShouldAntialias(cg, true);
      CGContextSetShouldSmoothFonts(cg, false);
      CGContextSetTextPosition(cg, 0, h - f->metrics.ascent);
      CTLineDraw(line, cg);
      CGContextRelease(cg);
      CFRelease(line);
      return mask;
   }
}

}  // namespace w32::mac
