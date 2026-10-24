/*
 * fontmap.cpp:  The font GDI realizes for a LOGFONT, as Windows 11 does, from
 *   what win32/tools/fontprobe.c measured there (core/fonttables.inc).
 */

#include "core/fontmap.h"

#include "core/native.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <map>
#include <mutex>
#include <strings.h>

namespace w32 {

namespace {

struct RasterSize
{
   int height, ascent, internal, external, average, max, weight;
};

// Heights from..to realize sizes[size] scale times as high, xscale as wide.
struct RasterRun
{
   int from, to, size, scale, xscale;
};

struct RasterFace
{
   const char *face;
   int charset;
   const RasterSize *sizes;
   size_t sizes_count;
   const RasterRun *runs;
   size_t runs_count;
};

// Per weight (normal, bold): for each height -100..-1, 1..100, a letter, the
// difference of GDI's value from the unhinted one ('M' none).
struct TrueTypeFace
{
   const char *face;
   int upem, win_ascent, win_descent, line_gap;
   int average[2];
   const char *ascent[2], *descent[2], *internal[2], *average_delta[2];
};

#include "core/fonttables.inc"

// Names GDI realizes as another face (FontSubstitutes), keeping their own.
const struct
{
   const char *alias, *face;
} aliases[] = {{"MS Shell Dlg 2", "Tahoma"}, {"Helvetica", "Arial"}, {"Times", "Times New Roman"}};

// What draws a raster face's sizes the bundled fonts lack.
const struct
{
   const char *raster, *outline;
} stand_ins[] = {{"MS Sans Serif", "Microsoft Sans Serif"}, {"MS Serif", "Times New Roman"}, {"Small Fonts", "Arial"},
                 {"Courier", "Courier New"}, {"Fixedsys", "Courier New"}, {"Terminal", "Courier New"},
                 {"System", "Arial"}};

const RasterFace *FindRaster(const char *face)
{
   for (const RasterFace &f : raster_faces)
      if (strcasecmp(face, f.face) == 0)
         return &f;
   return nullptr;
}

const TrueTypeFace *FindTrueType(const char *face)
{
   for (const auto &a : aliases)
      if (strcasecmp(face, a.alias) == 0)
         face = a.face;
   for (const TrueTypeFace &f : truetype_faces)
      if (strcasecmp(face, f.face) == 0)
         return &f;
   return nullptr;
}

// GDI picks the bold font past 550 (a raster face is emboldened).
bool Bold(int weight)
{
   return weight > 550;
}

void FromRaster(RealizedFont &r, const RasterFace &f, const RasterSize &s, int scale, int xscale, int weight)
{
   r.raster = f.face;
   r.draw = f.face;
   for (const auto &stand_in : stand_ins)
      if (r.raster == stand_in.raster)
         r.draw = stand_in.outline;
   r.raster_height = s.height;
   r.raster_internal = s.internal;
   r.scale = scale;
   r.known = true;
   r.height = s.height * scale;
   r.ascent = s.ascent * scale;
   r.descent = r.height - r.ascent;
   r.internal_leading = s.internal * scale;
   r.external_leading = s.external * scale;
   r.average_width = s.average * xscale;
   r.max_width = s.max * xscale;
   r.weight = s.weight;
   if (Bold(weight) && s.weight < FW_SEMIBOLD)
   {
      r.weight = FW_BOLD;
      r.average_width += xscale;
      r.max_width += xscale;
      r.overhang = 1;
   }
   r.em = r.height - r.internal_leading;
}

// The size and scales Windows realizes for height (beyond +-100, as at 100).
const RasterSize &RasterAt(const RasterFace &f, int height, int *scale, int *xscale)
{
   height = std::clamp(height, -100, 100);
   for (size_t i = 0; i < f.runs_count; i++)
      if (height >= f.runs[i].from && height <= f.runs[i].to)
      {
         *scale = f.runs[i].scale;
         *xscale = f.runs[i].xscale;
         return f.sizes[f.runs[i].size];
      }
   *scale = *xscale = 1;
   return f.sizes[0];
}

void FromTrueType(RealizedFont &r, const TrueTypeFace &f, int height, int weight)
{
   // Height 0 is the default size, 12 points.
   if (height == 0)
      height = -16;
   int b = Bold(weight);
   double em = height < 0 ? -height : height * (double) f.upem / (f.win_ascent + f.win_descent);
   double scale = em / f.upem;
   int index = height < 0 ? height + 100 : height + 99;
   auto delta = [&](const char *table) { return height >= -100 && height <= 100 ? table[index] - 'M' : 0; };
   r.draw = f.face;
   r.known = true;
   r.ascent = (int) std::lround(f.win_ascent * scale) + delta(f.ascent[b]);
   r.descent = (int) std::lround(f.win_descent * scale) + delta(f.descent[b]);
   r.height = r.ascent + r.descent;
   r.internal_leading = r.height - (int) std::lround(em) + delta(f.internal[b]);
   r.average_width = std::max(1, (int) std::lround(f.average[b] * scale)) + delta(f.average_delta[b]);
   r.em = r.height - r.internal_leading;
   r.external_leading = std::max(0, (int) std::lround(f.line_gap * r.em / (double) f.upem));
   r.weight = b ? FW_BOLD : FW_NORMAL;
}

// GDI's font mapper for a face that isn't installed (measured on Windows 11): a
// weight no TrueType font has (not 400 or 700) takes a raster face's size that
// is the height asked for, unscaled; otherwise, or with none, the TrueType face.
// The family (fixed pitch, roman, other) picks the faces.
void Missing(RealizedFont &r, const LOGFONTA &lf)
{
   static const char *const fixed[] = {"Courier", "Fixedsys"};
   static const char *const roman[] = {"MS Serif"};
   static const char *const swiss[] = {"MS Sans Serif", "Small Fonts"};
   const char *const *rasters = swiss;
   size_t count = std::size(swiss);
   const char *truetype = "Arial";
   if ((lf.lfPitchAndFamily & 0x3) == FIXED_PITCH)
      rasters = fixed, count = std::size(fixed), truetype = "Courier New";
   else if ((lf.lfPitchAndFamily & 0xF0) == FF_ROMAN)
      rasters = roman, count = std::size(roman), truetype = "Times New Roman";
   int height = lf.lfHeight;
   if (lf.lfOutPrecision != OUT_TT_ONLY_PRECIS && lf.lfWeight != FW_NORMAL && lf.lfWeight != FW_BOLD)
      for (size_t i = 0; i < count; i++)
      {
         const RasterFace *f = FindRaster(rasters[i]);
         if (height == 0)
         {
            int scale, xscale;
            const RasterSize &s = RasterAt(*f, 0, &scale, &xscale);
            FromRaster(r, *f, s, scale, xscale, lf.lfWeight);
            r.face = f->face;
            return;
         }
         for (size_t j = 0; j < f->sizes_count; j++)
         {
            const RasterSize &s = f->sizes[j];
            if (height > 0 ? s.height == height : s.height - s.internal == -height)
            {
               FromRaster(r, *f, s, 1, 1, lf.lfWeight);
               r.face = f->face;
               return;
            }
         }
      }
   FromTrueType(r, *FindTrueType(truetype), height, lf.lfWeight);
   r.face = truetype;
}

// native::FaceInstalled, asked once per face.
bool Installed(const char *face)
{
   static std::mutex mutex;
   static std::map<std::string, bool> known;
   std::string key = face;
   for (char &c : key)
      c = (char) tolower((unsigned char) c);
   std::lock_guard<std::mutex> lock(mutex);
   auto it = known.find(key);
   if (it == known.end())
      it = known.emplace(key, native::FaceInstalled(face)).first;
   return it->second;
}

}  // namespace

RealizedFont RealizeFont(const LOGFONTA &lf)
{
   RealizedFont r;
   const char *face = lf.lfFaceName;
   r.face = r.draw = face;
   if (face[0] == '\0')
      return r;
   if (const RasterFace *f = FindRaster(face))
   {
      // Raster faces have their own character set (Terminal OEM, the others
      // ANSI); asked for in another, a face that has it stands in.
      if (lf.lfCharSet == f->charset || lf.lfCharSet == DEFAULT_CHARSET)
      {
         int scale, xscale;
         const RasterSize *s = &RasterAt(*f, lf.lfHeight, &scale, &xscale);
         // A width asked for picks among a height's sizes (Terminal's).
         if (lf.lfWidth > 0 && scale == 1)
            for (size_t i = 0; i < f->sizes_count; i++)
               if (f->sizes[i].height == s->height && f->sizes[i].ascent == s->ascent &&
                   f->sizes[i].average == lf.lfWidth)
                  s = &f->sizes[i], xscale = 1;
         FromRaster(r, *f, *s, scale, xscale, lf.lfWeight);
         r.face = f->face;
      }
      else
         Missing(r, lf);
      return r;
   }
   if (const TrueTypeFace *f = FindTrueType(face))
   {
      FromTrueType(r, *f, lf.lfHeight, lf.lfWeight);
      r.face = face;
      return r;
   }
   if (!Installed(face))
      Missing(r, lf);
   return r;
}

}  // namespace w32
