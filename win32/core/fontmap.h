/*
 * fontmap.h:  The font GDI realizes for a LOGFONT, as Windows 11 does: the face
 *   (an installed one, or its font mapper's choice for a face that isn't), and
 *   for Windows' own faces their metrics at each size, measured on Windows
 *   (core/fonttables.inc).  The backends draw it with their stand-ins and these
 *   metrics.
 */

#ifndef W32_CORE_FONTMAP_H
#define W32_CORE_FONTMAP_H

#include "core/win32.h"

#include <string>

namespace w32 {

struct RealizedFont
{
   std::string face;                 // as GetTextFace gives it
   std::string draw;                 // the outline face that draws it (a raster size's stand-in)
   std::string raster;               // a raster face: the size of it, scale times its own, Windows uses
   int raster_height = 0, raster_internal = 0, scale = 1;
   bool known = false;               // the metrics below are Windows'
   int height = 0, ascent = 0, descent = 0, internal_leading = 0, external_leading = 0, average_width = 0;
   int max_width = 0;                // 0: the backend's
   int weight = FW_NORMAL, overhang = 0;
   int em = 0;                       // the outline em that draws it: height - internal leading
};

RealizedFont RealizeFont(const LOGFONTA &lf);

}  // namespace w32

#endif
