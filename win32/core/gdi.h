/*
 * gdi.h:  GDI objects and device contexts.  Drawing happens on raster targets
 *   the layer owns: a window's top-level surface (32-bit BGRA) or a bitmap (8-bit
 *   indexed with a color table, 24- or 32-bit), never by converting whole
 *   bitmaps (the client's offscreen DIBs are large, its minimap 9000x9000).
 */

#ifndef W32_CORE_GDI_H
#define W32_CORE_GDI_H

#include "core/region.h"
#include "core/win32.h"

#include <memory>
#include <string>
#include <vector>

namespace w32 {

namespace native {
class Font;
}

struct GdiObject
{
   explicit GdiObject(DWORD type) : type(type) {}
   virtual ~GdiObject() = default;
   DWORD type;                     // OBJ_PEN, OBJ_BRUSH ...
   bool stock = false;             // GetStockObject's: DeleteObject leaves them
   int selected = 0;               // DCs it is selected into
};

HGDIOBJ AddGdiObject(GdiObject *object);
GdiObject *GetGdiObject(HGDIOBJ h);
template <class T> T *GetGdiObjectAs(HGDIOBJ h, DWORD type)
{
   GdiObject *o = GetGdiObject(h);
   return (o && o->type == type) ? static_cast<T *>(o) : nullptr;
}

struct GdiRegion : GdiObject
{
   explicit GdiRegion(Region r) : GdiObject(OBJ_REGION), region(std::move(r)) {}
   Region region;
};

struct GdiPen : GdiObject
{
   GdiPen() : GdiObject(OBJ_PEN) {}
   UINT style = PS_SOLID;
   int width = 1;
   COLORREF color = 0;
};

struct GdiBrush : GdiObject
{
   GdiBrush() : GdiObject(OBJ_BRUSH) {}
   UINT style = BS_SOLID;          // BS_SOLID, BS_NULL, BS_HATCHED, BS_PATTERN, BS_DIBPATTERN
   COLORREF color = 0;
   int hatch = 0;
   HBITMAP pattern = nullptr;
   int sys_color = -1;             // GetSysColorBrush's: the color follows the system color
};

struct GdiFont : GdiObject
{
   GdiFont() : GdiObject(OBJ_FONT) {}
   LOGFONTA log = {};
   native::Font *font = nullptr;   // made on first use
};

struct GdiPalette : GdiObject
{
   GdiPalette() : GdiObject(OBJ_PAL) {}
   std::vector<PALETTEENTRY> entries;
};

// A bitmap: compatible (32-bit), DIB section or device-independent resource.
struct GdiBitmap : GdiObject
{
   GdiBitmap() : GdiObject(OBJ_BITMAP) {}
   ~GdiBitmap() override;
   int width = 0, height = 0;
   int bpp = 32;                   // 1, 8, 24 or 32
   int stride = 0;                 // bytes per row, DWORD-aligned
   bool bottom_up = false;         // DIB rows stored bottom row first (positive biHeight)
   uint8_t *bits = nullptr;
   bool owns_bits = true;
   std::vector<RGBQUAD> colors;    // 1 and 8 bpp: the color table
   // Row y (0 = top) in memory.
   uint8_t *Row(int y) const { return bits + (size_t) (bottom_up ? height - 1 - y : y) * stride; }
};

// What a DC draws on.
struct RasterTarget
{
   uint8_t *bits = nullptr;
   int width = 0, height = 0, stride = 0;
   int bpp = 32;
   bool bottom_up = false;
   const RGBQUAD *colors = nullptr;   // 8 bpp
   int num_colors = 0;
   uint8_t *Row(int y) const { return bits + (size_t) (bottom_up ? height - 1 - y : y) * stride; }
};

struct DC : GdiObject
{
   DC() : GdiObject(OBJ_DC) {}
   HWND hwnd = nullptr;            // window DCs (GetDC, BeginPaint)
   bool memory = false;            // CreateCompatibleDC
   bool info_only = false;         // CreateIC
   bool printer = false;

   HGDIOBJ pen = nullptr, brush = nullptr, font = nullptr, bitmap = nullptr;
   HPALETTE palette = nullptr;
   COLORREF text_color = RGB(0, 0, 0), bk_color = RGB(255, 255, 255);
   int bk_mode = OPAQUE;
   int rop2 = R2_COPYPEN;
   int stretch_mode = BLACKONWHITE;
   int map_mode = MM_TEXT;
   UINT text_align = TA_TOP | TA_LEFT;
   POINT pos = {0, 0};             // MoveToEx
   POINT origin = {0, 0};          // logical (0, 0) in the target: the device origin + viewport_org - window_org
   POINT window_org = {0, 0};      // SetWindowOrgEx
   POINT viewport_org = {0, 0};    // SetViewportOrgEx
   DWORD layout = 0;               // SetLayout
   Region visible;                 // window DCs: what of the target belongs to the window (target coordinates)
   Region clip;                    // SelectClipRgn, BeginPaint's update region; empty = none (target coordinates)
   bool has_clip = false;

   struct Saved;
   std::vector<std::shared_ptr<Saved>> saved;
};

DC *GetDCObject(HDC hdc);
// Device (0, 0) in the target (window DCs: the client area's offset).
inline POINT DeviceOrigin(const DC *dc)
{
   return {dc->origin.x - dc->viewport_org.x + dc->window_org.x, dc->origin.y - dc->viewport_org.y + dc->window_org.y};
}
// The target hdc draws on now (the selected bitmap, the window surface).
bool GetTarget(DC *dc, RasterTarget *target);
// The region drawing is limited to, in target coordinates.
Region EffectiveClip(DC *dc, const RasterTarget &target);

// COLORREF (with PALETTEINDEX and PALETTERGB forms) to an RGB COLORREF.
COLORREF ResolveColor(DC *dc, COLORREF color);
// Pixel value for target: BGRA for 32 bpp, the nearest palette index for 8 bpp.
uint32_t PixelValue(const RasterTarget &t, COLORREF rgb);

// Text: Windows-1252 to UTF-8, and back.
std::string AnsiToUtf8(const char *text, int length = -1);
std::string Utf8ToAnsi(const std::string &utf8);

void ShowTargetChanges(DC *dc, const RECT &target_rect);

}  // namespace w32

#endif
