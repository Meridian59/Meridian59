/*
 * text.cpp:  Fonts and text on Pango.  LOGFONT heights are Windows': negative
 *   for the characters' height (the em), positive for the cell's (ascent plus
 *   descent).  Windows' faces map to metric-compatible ones (Wine's MS Sans
 *   Serif, Tahoma, System ...; Liberation's Arial, Times New Roman, Courier
 *   New), and metrics come from the font's tables as GDI computes them.
 */

#include "gtk/gtk_backend.h"

#include "core/fontmap.h"
#include "core/rasterfont.h"

#include <fontconfig/fontconfig.h>
#include <pango/pangocairo.h>
#include <pango/pangofc-fontmap.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <string>
#include <strings.h>

namespace w32::native {

class Font
{
public:
   PangoFontDescription *desc = nullptr;
   PangoLayout *layout = nullptr;      // reused for measuring and drawing
   FontMetrics metrics;
   const RasterFont *raster = nullptr;   // Windows' raster faces: their own bitmaps
   bool underline = false, strike_out = false;
   std::map<std::pair<uint32_t, uint32_t>, int> kerning;
};

namespace {

// Wine's fonts (fonts-wine), made to Windows' metrics, aren't in fontconfig's
// directories: the application's fonts.
void AddWineFonts(PangoFontMap *map)
{
   static const char *const dirs[] = {"/usr/share/wine/fonts", "/usr/local/share/wine/fonts", "/opt/wine/share/wine/fonts"};
   bool added = false;
   for (const char *d : dirs)
      if (FcConfigAppFontAddDir(FcConfigGetCurrent(), (const FcChar8 *) d))
         added = true;
   if (added && PANGO_IS_FC_FONT_MAP(map))
      pango_fc_font_map_config_changed(PANGO_FC_FONT_MAP(map));
}

PangoContext *Context()
{
   static PangoContext *context = [] {
      PangoFontMap *map = pango_cairo_font_map_get_default();
      AddWineFonts(map);
      PangoContext *c = pango_font_map_create_context(map);
      // Windows' fonts render at 96 dpi; Pango then measures in device pixels.
      pango_cairo_context_set_resolution(c, 96);
      cairo_font_options_t *options = cairo_font_options_create();
      cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_GRAY);
      cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_SLIGHT);
      cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_ON);
      pango_cairo_context_set_font_options(c, options);
      cairo_font_options_destroy(options);
      return c;
   }();
   return context;
}

// Families to try for a Windows face name, best first: the metric-compatible
// ones (Wine's own, Liberation's), then near ones; null for other faces.
const char *KnownFamily(const char *face)
{
   struct Map
   {
      const char *windows, *family;
   };
   static const Map maps[] = {
      // MS Shell Dlg and Microsoft Sans Serif are Windows' substitutes for, and
      // have the metrics of, MS Sans Serif.
      {"MS Sans Serif", "MS Sans Serif,Microsoft Sans Serif,Liberation Sans,Sans"},
      {"MS Shell Dlg", "MS Sans Serif,Microsoft Sans Serif,Liberation Sans,Sans"},
      {"Microsoft Sans Serif", "Microsoft Sans Serif,MS Sans Serif,Liberation Sans,Sans"},
      {"MS Shell Dlg 2", "Tahoma,Liberation Sans,Sans"},
      {"Tahoma", "Tahoma,Liberation Sans,Sans"},
      {"Arial", "Arial,Liberation Sans,Sans"},
      {"Helvetica", "Arial,Liberation Sans,Sans"},
      {"Times New Roman", "Times New Roman,Liberation Serif,Serif"},
      {"Courier New", "Courier New,Liberation Mono,Monospace"},
      {"Courier", "Courier,Courier New,Liberation Mono,Monospace"},
      {"System", "System,MS Sans Serif,Liberation Sans,Sans"},
      {"Small Fonts", "Small Fonts,MS Sans Serif,Liberation Sans,Sans"},
      {"Fixedsys", "Fixedsys,Liberation Mono,Monospace"},
      {"Terminal", "Fixedsys,Liberation Mono,Monospace"},
      {"Verdana", "Verdana,DejaVu Sans,Sans"},
      {"Segoe UI", "Segoe UI,Noto Sans,DejaVu Sans,Liberation Sans,Sans"},
   };
   for (const Map &m : maps)
      if (strcasecmp(face, m.windows) == 0)
         return m.family;
   return nullptr;
}

const char *Family(const char *face, BYTE pitch)
{
   if (const char *known = KnownFamily(face))
      return known;
   // An installed face (RealizeFont replaces the others), then near ones.
   static std::string family;
   const char *fallback = (pitch & 0x3) == FIXED_PITCH ? "Courier New,Liberation Mono,Monospace"
                                                       : "Arial,Liberation Sans,Sans";
   if (*face)
   {
      family = std::string(face) + "," + fallback;
      return family.c_str();
   }
   return fallback;
}

// What GDI computes metrics from (OS/2 and hhea), in font units.
struct Tables
{
   bool ok = false;
   double upem = 2048;
   int win_ascent = 0, win_descent = 0, avg_width = 0;
   int hhea_ascender = 0, hhea_descender = 0, line_gap = 0, max_advance = 0;
};

int Be16(const char *p, unsigned offset, unsigned length, bool is_signed)
{
   if (offset + 2 > length)
      return 0;
   unsigned v = ((unsigned char) p[offset] << 8) | (unsigned char) p[offset + 1];
   return is_signed ? (int) (int16_t) v : (int) v;
}

Tables ReadTables(PangoFontDescription *desc)
{
   Tables t;
   PangoFont *font = pango_font_map_load_font(pango_context_get_font_map(Context()), Context(), desc);
   if (font == nullptr)
      return t;
   hb_face_t *face = hb_font_get_face(pango_font_get_hb_font(font));
   t.upem = hb_face_get_upem(face);
   unsigned length;
   hb_blob_t *os2 = hb_face_reference_table(face, HB_TAG('O', 'S', '/', '2'));
   const char *p = hb_blob_get_data(os2, &length);
   if (p && length >= 78)
   {
      t.avg_width = Be16(p, 2, length, true);
      t.win_ascent = Be16(p, 74, length, false);
      t.win_descent = Be16(p, 76, length, false);
      t.ok = t.win_ascent + t.win_descent > 0;
   }
   hb_blob_destroy(os2);
   hb_blob_t *hhea = hb_face_reference_table(face, HB_TAG('h', 'h', 'e', 'a'));
   p = hb_blob_get_data(hhea, &length);
   if (p && length >= 12)
   {
      t.hhea_ascender = Be16(p, 4, length, true);
      t.hhea_descender = Be16(p, 6, length, true);
      t.line_gap = Be16(p, 8, length, true);
      t.max_advance = Be16(p, 10, length, false);
   }
   hb_blob_destroy(hhea);
   // GDI's tmAveCharWidth is xAvgCharWidth, which Windows' fonts carry as OS/2
   // version 1's weighted average of the lowercase letters and space; newer fonts
   // (Liberation's, Wine's) average all glyphs instead.  The old formula, on the
   // same advances, gives Windows' value.
   static const int weights[27] = {64, 14, 27, 35, 100, 20, 14, 42, 63, 3, 6, 35, 20, 56,
                                   56, 17, 4,  49, 56, 71,  31, 10, 18, 3,  18, 2, 166};
   hb_font_t *units = hb_font_create(face);   // scale: font units
   long total = 0;
   bool all = true;
   for (int i = 0; i < 27 && all; i++)
   {
      hb_codepoint_t glyph;
      if (!hb_font_get_nominal_glyph(units, i < 26 ? 'a' + i : ' ', &glyph))
         all = false;
      else
         total += (long) hb_font_get_glyph_h_advance(units, glyph) * weights[i];
   }
   hb_font_destroy(units);
   if (all)
      t.avg_width = (int) (total / 1000);
   g_object_unref(font);
   return t;
}

// GDI's text metrics for the em size em (pixels), as Wine computes them
// (dlls/win32u/freetype.c, tested against Windows).
void Measure(Font *f, const Tables &t, double em)
{
   if (!t.ok)
   {
      PangoFontMetrics *m = pango_context_get_metrics(Context(), f->desc, nullptr);
      f->metrics.ascent = (int) std::lround(pango_font_metrics_get_ascent(m) / (double) PANGO_SCALE);
      f->metrics.descent = (int) std::lround(pango_font_metrics_get_descent(m) / (double) PANGO_SCALE);
      f->metrics.average_width = std::max(1, (int) std::lround(pango_font_metrics_get_approximate_char_width(m) /
                                                               (double) PANGO_SCALE));
      pango_font_metrics_unref(m);
      f->metrics.height = f->metrics.ascent + f->metrics.descent;
      f->metrics.internal_leading = std::max(0, f->metrics.height - (int) std::lround(em));
      f->metrics.max_width = f->metrics.average_width * 2;
      return;
   }
   double scale = em / t.upem;
   f->metrics.ascent = (int) std::lround(t.win_ascent * scale);
   f->metrics.descent = (int) std::lround(t.win_descent * scale);
   f->metrics.height = f->metrics.ascent + f->metrics.descent;
   f->metrics.internal_leading = std::max(0, f->metrics.height - (int) std::lround(em));
   int external = t.line_gap - ((t.win_ascent + t.win_descent) - (t.hhea_ascender - t.hhea_descender));
   f->metrics.external_leading = std::max(0, (int) std::lround(external * scale));
   f->metrics.average_width = std::max(1, (int) std::lround(t.avg_width * scale));
   if (t.avg_width <= 0)
   {
      PangoFontMetrics *m = pango_context_get_metrics(Context(), f->desc, nullptr);
      f->metrics.average_width = std::max(1, (int) std::lround(pango_font_metrics_get_approximate_char_width(m) /
                                                               (double) PANGO_SCALE));
      pango_font_metrics_unref(m);
   }
   f->metrics.max_width = std::max(f->metrics.average_width, (int) std::lround(t.max_advance * scale));
}

PangoLayout *Layout(Font *f, const std::string &utf8)
{
   pango_layout_set_text(f->layout, utf8.c_str(), (int) utf8.size());
   return f->layout;
}

}  // namespace

bool FaceInstalled(const char *face)
{
   if (KnownFamily(face))
      return true;
   PangoFontFamily **families = nullptr;
   int n = 0;
   pango_context_list_families(Context(), &families, &n);
   bool found = false;
   for (int i = 0; i < n && !found; i++)
      found = strcasecmp(pango_font_family_get_name(families[i]), face) == 0;
   g_free(families);
   return found;
}

Font *CreateFont(const LOGFONTA &lf)
{
   auto *f = new Font;
   if ((f->raster = OpenRasterFont(lf)) != nullptr)
   {
      RasterMetrics m = RasterFontMetrics(f->raster);
      f->metrics.height = m.height;
      f->metrics.ascent = m.ascent;
      f->metrics.descent = m.descent;
      f->metrics.internal_leading = m.internal_leading;
      f->metrics.external_leading = m.external_leading;
      f->metrics.average_width = m.average_width;
      f->metrics.max_width = m.max_width;
      f->metrics.weight = m.weight;
      f->metrics.italic = lf.lfItalic != 0;
      f->metrics.underline = f->underline = lf.lfUnderline != 0;
      f->metrics.strike_out = f->strike_out = lf.lfStrikeOut != 0;
      f->metrics.face = m.face;
      return f;
   }
   // Windows' face and, for its own faces, its metrics (core/fontmap.h): drawn by
   // the face's stand-in at the em they make.
   RealizedFont w = RealizeFont(lf);
   LOGFONTA drawn = lf;
   if (w.known)
   {
      lstrcpynA(drawn.lfFaceName, w.draw.c_str(), LF_FACESIZE);
      drawn.lfHeight = -w.em;
   }
   f->desc = pango_font_description_new();
   pango_font_description_set_family(f->desc, Family(drawn.lfFaceName, lf.lfPitchAndFamily));
   int weight = lf.lfWeight ? lf.lfWeight : FW_NORMAL;
   pango_font_description_set_weight(f->desc, (PangoWeight) std::clamp(weight, 100, 1000));
   pango_font_description_set_style(f->desc, lf.lfItalic ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL);
   // The em in pixels: |height| for character heights; for cell heights, the em
   // whose win ascent plus descent is the height (GDI's ppem).
   int height = drawn.lfHeight == 0 ? -13 : drawn.lfHeight;
   pango_font_description_set_absolute_size(f->desc, std::abs(height) * PANGO_SCALE);
   Tables t = ReadTables(f->desc);
   double em = std::abs(height);
   if (height > 0 && t.ok)
      em = height * t.upem / (t.win_ascent + t.win_descent);
   else if (height > 0)
   {
      Measure(f, t, em);
      double cell = f->metrics.height > 0 ? f->metrics.height : height;
      em = height * (double) height / cell;
   }
   pango_font_description_set_absolute_size(f->desc, em * PANGO_SCALE);
   Measure(f, t, em);
   f->layout = pango_layout_new(Context());
   pango_layout_set_font_description(f->layout, f->desc);
   f->metrics.weight = weight;
   if (w.known)
   {
      f->metrics.height = w.height;
      f->metrics.ascent = w.ascent;
      f->metrics.descent = w.descent;
      f->metrics.internal_leading = w.internal_leading;
      f->metrics.external_leading = w.external_leading;
      f->metrics.average_width = w.average_width;
      if (w.max_width)
         f->metrics.max_width = w.max_width;
      f->metrics.weight = w.weight;
   }
   f->metrics.italic = lf.lfItalic != 0;
   f->metrics.underline = f->underline = lf.lfUnderline != 0;
   f->metrics.strike_out = f->strike_out = lf.lfStrikeOut != 0;
   f->metrics.face = w.face;
   return f;
}

void DestroyFont(Font *f)
{
   if (f == nullptr)
      return;
   if (f->layout)
      g_object_unref(f->layout);
   if (f->desc)
      pango_font_description_free(f->desc);
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
   if (f->raster)
      return RasterTextWidth(f->raster, Utf8ToAnsi(utf8));
   PangoRectangle logical;
   pango_layout_get_pixel_extents(Layout(f, utf8), nullptr, &logical);
   return logical.width;
}

std::vector<int> CharEdges(Font *f, const std::string &utf8)
{
   std::vector<int> edges;
   if (utf8.empty())
      return edges;
   if (f->raster)
   {
      int x = 0;
      for (const char *p = utf8.c_str(); *p;)
      {
         const char *next = g_utf8_next_char(p);
         std::string ansi = Utf8ToAnsi(std::string(p, next - p));
         x += RasterTextWidth(f->raster, ansi.empty() ? "?" : ansi);
         edges.push_back(x);
         p = next;
      }
      return edges;
   }
   PangoLayout *layout = Layout(f, utf8);
   PangoLayoutLine *line = pango_layout_get_line_readonly(layout, 0);
   for (const char *p = utf8.c_str(); *p; p = g_utf8_next_char(p))
   {
      int x = 0;
      pango_layout_line_index_to_x(line, (int) (p - utf8.c_str()), TRUE, &x);
      edges.push_back(PANGO_PIXELS(x));
   }
   return edges;
}

void CharWidths(Font *f, uint32_t cp, int *a, int *b, int *c)
{
   char buf[8];
   std::string s(buf, g_unichar_to_utf8(cp, buf));
   if (f->raster)
   {
      std::string ansi = Utf8ToAnsi(s);
      RasterCharWidths(f->raster, ansi.empty() ? '?' : (unsigned char) ansi[0], a, b, c);
      return;
   }
   PangoRectangle ink, logical;
   pango_layout_get_pixel_extents(Layout(f, s), &ink, &logical);
   *a = ink.width > 0 ? ink.x - logical.x : 0;
   *b = ink.width > 0 ? ink.width : 0;
   *c = logical.width - *a - *b;
}

int Kerning(Font *f, uint32_t first, uint32_t second)
{
   if (f->raster)
      return 0;
   auto key = std::make_pair(first, second);
   if (auto it = f->kerning.find(key); it != f->kerning.end())
      return it->second;
   char buf[16];
   int n = g_unichar_to_utf8(first, buf);
   int m = g_unichar_to_utf8(second, buf + n);
   std::string pair(buf, n + m), a(buf, n), b(buf + n, m);
   int k = TextWidth(f, pair) - TextWidth(f, a) - TextWidth(f, b);
   f->kerning[key] = k;
   return k;
}

std::vector<uint8_t> RenderText(Font *f, const std::string &utf8, int *width, int *height)
{
   if (f->raster)
      return RasterRender(f->raster, Utf8ToAnsi(utf8), width, height);
   PangoLayout *layout = Layout(f, utf8);
   PangoRectangle ink, logical;
   pango_layout_get_pixel_extents(layout, &ink, &logical);
   // Room for what hangs past the advance (italics, bearings).
   int w = std::max(logical.width, ink.x + ink.width) + 1;
   // Down to what hangs below the cell too (a stand-in's descenders), as GDI
   // draws it: clipped by the DC, not by the cell.
   int baseline = pango_layout_get_baseline(layout) / PANGO_SCALE;
   int h = std::max(f->metrics.height, f->metrics.ascent - baseline + ink.y + ink.height + 1);
   *width = w;
   *height = h;
   std::vector<uint8_t> mask((size_t) w * h, 0);
   if (w <= 0 || h <= 0 || utf8.empty())
      return mask;
   int stride = cairo_format_stride_for_width(CAIRO_FORMAT_A8, w);
   std::vector<uint8_t> buf((size_t) stride * h, 0);
   cairo_surface_t *s = cairo_image_surface_create_for_data(buf.data(), CAIRO_FORMAT_A8, w, h, stride);
   cairo_t *cr = cairo_create(s);
   cairo_set_source_rgba(cr, 0, 0, 0, 1);
   // The layout's baseline at the font's ascent.
   cairo_move_to(cr, 0, f->metrics.ascent - baseline);
   pango_cairo_show_layout(cr, layout);
   cairo_destroy(cr);
   cairo_surface_flush(s);
   cairo_surface_destroy(s);
   for (int y = 0; y < h; y++)
      memcpy(&mask[(size_t) y * w], &buf[(size_t) y * stride], w);
   return mask;
}

}  // namespace w32::native
