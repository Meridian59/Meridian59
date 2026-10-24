/*
 * rasterfont.cpp:  Windows' raster fonts from .fon files (NE executables of
 *   FNT resources, version 2 or 3): Wine's, installed next to the layer in fonts/
 *   (win32/wine-port/fonts), or fonts-wine's.  Bold is synthesized as GDI does: each
 *   glyph overstruck one pixel right, one pixel wider (the overhang).
 */

#include "core/rasterfont.h"

#include "core/fontmap.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <map>
#include <memory>
#include <mutex>
#include <strings.h>

namespace w32 {

namespace {

struct Strike
{
   std::vector<uint8_t> data;          // the FNT resource
   std::string face;
   int points = 0, height = 0, ascent = 0, internal_leading = 0, external_leading = 0;
   int average_width = 0, max_width = 0, weight = FW_NORMAL, first = 0, last = 0, default_char = 0;
   bool italic = false;
   std::vector<int> widths, offsets;   // per character, first..last
};

}  // namespace

class RasterFont
{
public:
   const Strike *strike;
   bool bold;
};

namespace {

unsigned U16(const uint8_t *p)
{
   return p[0] | (p[1] << 8);
}

unsigned U32(const uint8_t *p)
{
   return U16(p) | (U16(p + 2) << 16);
}

bool ParseFnt(std::vector<uint8_t> fnt, Strike *s)
{
   if (fnt.size() < 118)
      return false;
   const uint8_t *f = fnt.data();
   unsigned version = U16(f);
   if (version != 0x200 && version != 0x300)
      return false;
   if (U16(f + 70) != 96)
      return false;   // strikes for other resolutions
   s->points = U16(f + 68);
   s->ascent = U16(f + 74);
   s->internal_leading = U16(f + 76);
   s->external_leading = U16(f + 78);
   s->italic = f[80] != 0;
   s->weight = U16(f + 83);
   s->height = U16(f + 88);
   s->average_width = U16(f + 91);
   s->max_width = U16(f + 93);
   s->first = f[95];
   s->last = f[96];
   s->default_char = s->first + f[97];
   unsigned face = U32(f + 105);
   for (unsigned i = face; i < fnt.size() && fnt[i]; i++)
      s->face += (char) fnt[i];
   unsigned table = version == 0x200 ? 118 : 148, step = version == 0x200 ? 4 : 6;
   for (int c = s->first; c <= s->last; c++)
   {
      unsigned e = table + (c - s->first) * step;
      if (e + step > fnt.size())
         return false;
      s->widths.push_back(U16(f + e));
      s->offsets.push_back(version == 0x200 ? U16(f + e + 2) : U32(f + e + 2));
   }
   s->data = std::move(fnt);
   return s->height > 0;
}

// The FNT resources of an NE .fon file.
std::vector<Strike> ParseFon(const std::vector<uint8_t> &d)
{
   std::vector<Strike> out;
   if (d.size() < 0x40 || d[0] != 'M' || d[1] != 'Z')
      return out;
   unsigned ne = U16(&d[0x3C]);
   if (ne + 0x40 > d.size() || d[ne] != 'N' || d[ne + 1] != 'E')
      return out;
   unsigned p = ne + U16(&d[ne + 0x24]);
   if (p + 2 > d.size())
      return out;
   unsigned shift = U16(&d[p]);
   p += 2;
   while (p + 8 <= d.size())
   {
      unsigned type = U16(&d[p]), count = U16(&d[p + 2]);
      if (type == 0)
         break;
      p += 8;
      for (unsigned i = 0; i < count && p + 12 <= d.size(); i++, p += 12)
      {
         size_t offset = (size_t) U16(&d[p]) << shift, length = (size_t) U16(&d[p + 2]) << shift;
         if (type != 0x8008 || offset + length > d.size())
            continue;
         Strike s;
         if (ParseFnt(std::vector<uint8_t>(d.begin() + offset, d.begin() + offset + length), &s))
            out.push_back(std::move(s));
      }
   }
   return out;
}

std::vector<uint8_t> ReadAll(const std::string &path)
{
   std::vector<uint8_t> out;
   FILE *f = fopen(path.c_str(), "rb");
   if (f == nullptr)
      return out;
   uint8_t buf[65536];
   size_t n;
   while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
      out.insert(out.end(), buf, buf + n);
   fclose(f);
   return out;
}

// fonts/ next to the layer's library, then fonts-wine's.
std::vector<std::string> Directories()
{
   std::vector<std::string> dirs;
   Dl_info info;
   if (dladdr((void *) &OpenRasterFont, &info) && info.dli_fname)
   {
      std::string lib = info.dli_fname;
      size_t slash = lib.find_last_of('/');
      dirs.push_back((slash == std::string::npos ? std::string(".") : lib.substr(0, slash)) + "/fonts");
   }
   dirs.push_back("/usr/share/wine/fonts");
   dirs.push_back("/usr/local/share/wine/fonts");
   return dirs;
}

std::vector<Strike> &Strikes()
{
   static std::vector<Strike> strikes = [] {
      std::vector<Strike> all;
      static const char *const files[] = {"sserife.fon", "vgasys.fon", "smalle.fon", "coure.fon", "vgafix.fon"};
      for (const char *file : files)
         for (const std::string &dir : Directories())
         {
            std::vector<Strike> s = ParseFon(ReadAll(dir + "/" + file));
            if (s.empty())
               continue;
            for (Strike &x : s)
               all.push_back(std::move(x));
            break;
         }
      return all;
   }();
   return strikes;
}

int CharWidth(const RasterFont *font, unsigned char ch)
{
   const Strike *s = font->strike;
   int c = (ch < s->first || ch > s->last) ? s->default_char : ch;
   return s->widths[c - s->first] + (font->bold ? 1 : 0);
}

}  // namespace

const RasterFont *OpenRasterFont(const LOGFONTA &lf)
{
   // The size Windows realizes (core/fontmap.cpp), when it is one of the bundled
   // fonts', unscaled; the outline stand-in draws the others.  MS Shell Dlg's
   // small sizes are Microsoft Sans Serif's embedded bitmaps, MS Sans Serif's.
   RealizedFont r = RealizeFont(lf);
   std::string face = r.raster;
   int height = r.raster_height, internal = r.raster_internal, ascent = -1, average = -1;
   if (face.empty() && strcasecmp(r.face.c_str(), "MS Shell Dlg") == 0)
   {
      face = "MS Sans Serif", height = r.height, internal = r.internal_leading, ascent = r.ascent;
      average = r.average_width - (r.weight >= FW_SEMIBOLD ? 1 : 0);
   }
   else if (face.empty() || r.scale != 1)
      return nullptr;
   const Strike *best = nullptr;
   for (const Strike &x : Strikes())
      if (strcasecmp(x.face.c_str(), face.c_str()) == 0 && x.height == height && x.internal_leading == internal &&
          (ascent < 0 || x.ascent == ascent) && (average < 0 || x.average_width == average))
         best = &x;
   if (best == nullptr)
      return nullptr;
   bool bold = lf.lfWeight > 550 && best->weight < FW_SEMIBOLD;
   static std::mutex mutex;
   static std::map<std::pair<const Strike *, bool>, std::unique_ptr<RasterFont>> fonts;
   std::lock_guard<std::mutex> lock(mutex);
   auto &f = fonts[{best, bold}];
   if (f == nullptr)
      f.reset(new RasterFont{best, bold});
   return f.get();
}

RasterMetrics RasterFontMetrics(const RasterFont *font)
{
   const Strike *s = font->strike;
   RasterMetrics m;
   m.height = s->height;
   m.ascent = s->ascent;
   m.descent = s->height - s->ascent;
   m.internal_leading = s->internal_leading;
   m.external_leading = s->external_leading;
   m.average_width = s->average_width + (font->bold ? 1 : 0);
   m.max_width = s->max_width + (font->bold ? 1 : 0);
   m.weight = font->bold ? FW_BOLD : s->weight;
   m.overhang = font->bold ? 1 : 0;
   m.italic = s->italic;
   m.face = s->face;
   return m;
}

int RasterTextWidth(const RasterFont *font, const std::string &ansi)
{
   int w = 0;
   for (unsigned char ch : ansi)
      w += CharWidth(font, ch);
   return w;
}

void RasterCharWidths(const RasterFont *font, unsigned char ch, int *a, int *b, int *c)
{
   *a = 0;
   *b = CharWidth(font, ch);
   *c = 0;
}

std::vector<uint8_t> RasterRender(const RasterFont *font, const std::string &ansi, int *width, int *height)
{
   const Strike *s = font->strike;
   int w = RasterTextWidth(font, ansi), h = s->height;
   *width = std::max(w, 1);
   *height = h;
   std::vector<uint8_t> mask((size_t) *width * h, 0);
   int x = 0;
   for (unsigned char ch : ansi)
   {
      int c = (ch < s->first || ch > s->last) ? s->default_char : ch;
      int cw = s->widths[c - s->first];
      size_t offset = s->offsets[c - s->first];
      // Columns of 8 pixels, each its height of bytes, the high bit leftmost.
      for (int col = 0; col < (cw + 7) / 8; col++)
         for (int y = 0; y < h; y++)
         {
            size_t at = offset + (size_t) col * h + y;
            if (at >= s->data.size())
               continue;
            uint8_t bits = s->data[at];
            for (int b = 0; b < 8 && col * 8 + b < cw; b++)
               if (bits & (0x80 >> b))
               {
                  int px = x + col * 8 + b;
                  mask[(size_t) y * *width + px] = 255;
                  if (font->bold && px + 1 < *width)
                     mask[(size_t) y * *width + px + 1] = 255;
               }
         }
      x += cw + (font->bold ? 1 : 0);
   }
   return mask;
}

}  // namespace w32
