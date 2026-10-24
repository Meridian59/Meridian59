/*
 * rasterfont.h:  Windows' raster fonts (MS Sans Serif, System, Small Fonts,
 *   Courier, Fixedsys) from Wine's .fon files, as GDI uses them: fixed sizes
 *   with their own metrics and character widths, drawn from their bitmaps.  The
 *   backends ask here before their own fonts.
 */

#ifndef W32_CORE_RASTERFONT_H
#define W32_CORE_RASTERFONT_H

#include "core/win32.h"

#include <cstdint>
#include <string>
#include <vector>

namespace w32 {

class RasterFont;

// The bundled raster font size GDI realizes for lf, or nullptr if it realizes
// none of them (core/fontmap.h).  Lives for the process.
const RasterFont *OpenRasterFont(const LOGFONTA &lf);

struct RasterMetrics
{
   int height = 0, ascent = 0, descent = 0, internal_leading = 0, external_leading = 0;
   int average_width = 0, max_width = 0, weight = FW_NORMAL, overhang = 0;
   bool italic = false;
   std::string face;
};

RasterMetrics RasterFontMetrics(const RasterFont *font);
// Text is ANSI (the fonts' code page, 1252).
int RasterTextWidth(const RasterFont *font, const std::string &ansi);
void RasterCharWidths(const RasterFont *font, unsigned char ch, int *a, int *b, int *c);
// Coverage (0 or 255), width x height, the text's top-left at (0, 0).
std::vector<uint8_t> RasterRender(const RasterFont *font, const std::string &ansi, int *width, int *height);

}  // namespace w32

#endif
