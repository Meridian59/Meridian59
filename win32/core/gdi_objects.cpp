/*
 * gdi_objects.cpp:  GDI object handles, stock objects, creating and selecting
 *   pens, brushes, fonts, palettes and bitmaps, and DC state.
 */

#include "core/gdi.h"

#include "core/handles.h"
#include "core/native.h"
#include "core/window.h"

#include <cstring>

namespace w32 {

namespace {

HandleTable<GdiObject, HGDIOBJ> &Objects()
{
   static HandleTable<GdiObject, HGDIOBJ> objects(kGdiHandleBase);
   return objects;
}

HGDIOBJ stock[STOCK_LAST + 1];
HGDIOBJ default_bitmap;   // a new memory DC's 1x1 monochrome bitmap

HGDIOBJ MakeStock(GdiObject *o)
{
   o->stock = true;
   return AddGdiObject(o);
}

HGDIOBJ StockBrush(COLORREF color, UINT style = BS_SOLID)
{
   auto *b = new GdiBrush;
   b->color = color;
   b->style = style;
   return MakeStock(b);
}

HGDIOBJ StockPen(COLORREF color, UINT style = PS_SOLID)
{
   auto *p = new GdiPen;
   p->color = color;
   p->style = style;
   return MakeStock(p);
}

// Windows 11's stock fonts (GetObject's LOGFONTs).
HGDIOBJ StockFont(const char *face, int height, int weight, int charset, int precision, int pitch)
{
   auto *f = new GdiFont;
   f->log.lfHeight = height;
   f->log.lfWeight = weight;
   f->log.lfCharSet = (BYTE) charset;
   f->log.lfOutPrecision = (BYTE) precision;
   f->log.lfPitchAndFamily = (BYTE) pitch;
   lstrcpynA(f->log.lfFaceName, face, LF_FACESIZE);
   return MakeStock(f);
}

void InitStock()
{
   static bool done = false;
   if (done)
      return;
   done = true;
   stock[WHITE_BRUSH] = StockBrush(RGB(255, 255, 255));
   stock[LTGRAY_BRUSH] = StockBrush(RGB(192, 192, 192));
   stock[GRAY_BRUSH] = StockBrush(RGB(128, 128, 128));
   stock[DKGRAY_BRUSH] = StockBrush(RGB(64, 64, 64));
   stock[BLACK_BRUSH] = StockBrush(RGB(0, 0, 0));
   stock[NULL_BRUSH] = StockBrush(0, BS_NULL);
   stock[WHITE_PEN] = StockPen(RGB(255, 255, 255));
   stock[BLACK_PEN] = StockPen(RGB(0, 0, 0));
   stock[NULL_PEN] = StockPen(0, PS_NULL);
   // System's character set (254) isn't its own: Windows realizes it as Arial.
   stock[SYSTEM_FONT] = StockFont("System", 16, FW_BOLD, 254, OUT_STRING_PRECIS, VARIABLE_PITCH | FF_SWISS);
   stock[DEVICE_DEFAULT_FONT] = stock[SYSTEM_FONT];
   stock[DEFAULT_GUI_FONT] = StockFont("MS Shell Dlg", -11, FW_NORMAL, 254, 0, 0);
   stock[ANSI_VAR_FONT] = StockFont("MS Sans Serif", 12, FW_NORMAL, ANSI_CHARSET, 0, VARIABLE_PITCH);
   stock[ANSI_FIXED_FONT] = StockFont("Courier", 12, FW_NORMAL, ANSI_CHARSET, 0, FIXED_PITCH);
   stock[OEM_FIXED_FONT] = StockFont("Terminal", 12, FW_NORMAL, OEM_CHARSET, OUT_STRING_PRECIS, FIXED_PITCH | FF_MODERN);
   GetGdiObjectAs<GdiFont>(stock[OEM_FIXED_FONT], OBJ_FONT)->log.lfWidth = 8;
   stock[SYSTEM_FIXED_FONT] = StockFont("Fixedsys", 15, FW_NORMAL, ANSI_CHARSET, OUT_STRING_PRECIS,
                                        FIXED_PITCH | FF_MODERN);
   stock[DC_BRUSH] = StockBrush(RGB(255, 255, 255));
   stock[DC_PEN] = StockPen(RGB(0, 0, 0));
   // The default palette: the 20 static colors.
   auto *p = new GdiPalette;
   static const BYTE colors[20][3] = {
      {0, 0, 0}, {128, 0, 0}, {0, 128, 0}, {128, 128, 0}, {0, 0, 128}, {128, 0, 128}, {0, 128, 128},
      {192, 192, 192}, {192, 220, 192}, {166, 202, 240}, {255, 251, 240}, {160, 160, 164},
      {128, 128, 128}, {255, 0, 0}, {0, 255, 0}, {255, 255, 0}, {0, 0, 255}, {255, 0, 255},
      {0, 255, 255}, {255, 255, 255}};
   for (auto &c : colors)
      p->entries.push_back({c[0], c[1], c[2], 0});
   stock[DEFAULT_PALETTE] = MakeStock(p);
   // A 1x1 monochrome bitmap: what a new memory DC has selected.
   auto *b = new GdiBitmap;
   b->width = b->height = 1;
   b->bpp = 1;
   b->stride = 4;
   b->bits = new uint8_t[4]();
   b->colors = {{0, 0, 0, 0}, {255, 255, 255, 0}};
   default_bitmap = MakeStock(b);
}

void InitDC(DC *dc)
{
   InitStock();
   dc->pen = stock[BLACK_PEN];
   dc->brush = stock[WHITE_BRUSH];
   dc->font = stock[SYSTEM_FONT];
   dc->palette = (HPALETTE) stock[DEFAULT_PALETTE];
   if (dc->memory)
      dc->bitmap = default_bitmap;
}

}  // namespace

HGDIOBJ AddGdiObject(GdiObject *object)
{
   HGDIOBJ h = Objects().Add(object);
   if (object->type == OBJ_DC || object->type == OBJ_MEMDC)
      InitDC(static_cast<DC *>(object));
   return h;
}

GdiObject *GetGdiObject(HGDIOBJ h)
{
   return h ? Objects().Get(h) : nullptr;
}

DC *GetDCObject(HDC hdc)
{
   GdiObject *o = GetGdiObject(hdc);
   return (o && (o->type == OBJ_DC || o->type == OBJ_MEMDC)) ? static_cast<DC *>(o) : nullptr;
}

Region *GetRegion(HRGN hrgn)
{
   GdiRegion *r = GetGdiObjectAs<GdiRegion>(hrgn, OBJ_REGION);
   return r ? &r->region : nullptr;
}

GdiBitmap::~GdiBitmap()
{
   if (owns_bits)
      delete[] bits;
}

bool GetTarget(DC *dc, RasterTarget *t)
{
   if (dc->hwnd)
   {
      Window *top = GetWindowObject(TopLevelOf(dc->hwnd));
      if (top == nullptr || top->top == nullptr)
         return false;
      Surface s = native::GetSurface(top->top);
      if (s.pixels == nullptr)
         return false;
      *t = {s.pixels, s.width, s.height, s.stride, 32, false, nullptr, 0};
      return true;
   }
   GdiBitmap *b = GetGdiObjectAs<GdiBitmap>(dc->bitmap, OBJ_BITMAP);
   if (b == nullptr || b->bits == nullptr)
      return false;
   *t = {b->bits, b->width, b->height, b->stride, b->bpp, b->bottom_up,
         b->colors.empty() ? nullptr : b->colors.data(), (int) b->colors.size()};
   return true;
}

Region EffectiveClip(DC *dc, const RasterTarget &t)
{
   Region r(RECT{0, 0, t.width, t.height});
   if (dc->hwnd)
      r.Intersect(dc->visible);
   if (dc->has_clip)
      r.Intersect(dc->clip);
   return r;
}

COLORREF ResolveColor(DC *dc, COLORREF color)
{
   // PALETTEINDEX(i): entry i of the DC's palette; PALETTERGB: the RGB itself.
   if ((color >> 24) == 0x01)
   {
      GdiPalette *p = GetGdiObjectAs<GdiPalette>(dc ? dc->palette : nullptr, OBJ_PAL);
      UINT i = color & 0xFFFF;
      if (p && i < p->entries.size())
         return RGB(p->entries[i].peRed, p->entries[i].peGreen, p->entries[i].peBlue);
      return 0;
   }
   return color & 0x00FFFFFF;
}

uint32_t PixelValue(const RasterTarget &t, COLORREF rgb)
{
   BYTE r = GetRValue(rgb), g = GetGValue(rgb), b = GetBValue(rgb);
   if (t.bpp == 32 || t.bpp == 24)
      return 0xFF000000u | (r << 16) | (g << 8) | b;
   // 8 (or 1) bpp: the nearest color in the table, as GDI matches.
   int best = 0;
   int best_d = 1 << 30;
   for (int i = 0; i < t.num_colors; i++)
   {
      int dr = t.colors[i].rgbRed - r, dg = t.colors[i].rgbGreen - g, db = t.colors[i].rgbBlue - b;
      int d = dr * dr + dg * dg + db * db;
      if (d < best_d)
      {
         best_d = d;
         best = i;
         if (d == 0)
            break;
      }
   }
   return (uint32_t) best;
}

struct DC::Saved
{
   DC state;
};

}  // namespace w32

using namespace w32;

/************************************************************************/
// Objects

HGDIOBJ WINAPI GetStockObject(INT index)
{
   InitStock();
   return (index >= 0 && index <= STOCK_LAST) ? stock[index] : nullptr;
}

BOOL WINAPI DeleteObject(HGDIOBJ h)
{
   GdiObject *o = GetGdiObject(h);
   if (o == nullptr)
      return FALSE;
   if (o->stock)
      return TRUE;
   if (o->type == OBJ_DC || o->type == OBJ_MEMDC)
      return DeleteDC((HDC) h);
   // Objects selected into a DC aren't freed (Windows refuses; the client relies on
   // DeleteObject of a selected bitmap failing harmlessly).
   if (o->selected > 0)
      return FALSE;
   Objects().Remove(h);
   delete o;
   return TRUE;
}

DWORD WINAPI GetObjectType(HGDIOBJ h)
{
   GdiObject *o = GetGdiObject(h);
   return o ? o->type : 0;
}

INT WINAPI GetObjectA(HGDIOBJ h, INT size, LPVOID out)
{
   GdiObject *o = GetGdiObject(h);
   if (o == nullptr)
      return 0;
   switch (o->type)
   {
   case OBJ_PEN:
   {
      auto *p = static_cast<GdiPen *>(o);
      LOGPEN lp = {p->style, {p->width, 0}, p->color};
      if (out == nullptr)
         return sizeof(lp);
      memcpy(out, &lp, std::min<INT>(size, sizeof(lp)));
      return std::min<INT>(size, sizeof(lp));
   }
   case OBJ_BRUSH:
   {
      auto *b = static_cast<GdiBrush *>(o);
      LOGBRUSH lb = {b->style, b->color, (ULONG_PTR) b->hatch};
      if (out == nullptr)
         return sizeof(lb);
      memcpy(out, &lb, std::min<INT>(size, sizeof(lb)));
      return std::min<INT>(size, sizeof(lb));
   }
   case OBJ_FONT:
   {
      auto *f = static_cast<GdiFont *>(o);
      if (out == nullptr)
         return sizeof(LOGFONTA);
      memcpy(out, &f->log, std::min<INT>(size, sizeof(LOGFONTA)));
      return std::min<INT>(size, sizeof(LOGFONTA));
   }
   case OBJ_PAL:
   {
      auto *p = static_cast<GdiPalette *>(o);
      if (out && size >= (INT) sizeof(WORD))
         *(WORD *) out = (WORD) p->entries.size();
      return sizeof(WORD);
   }
   case OBJ_BITMAP:
   {
      auto *b = static_cast<GdiBitmap *>(o);
      DIBSECTION ds = {};
      ds.dsBm = {0, b->width, b->height, b->stride, 1, (WORD) b->bpp, b->bits};
      ds.dsBmih = {sizeof(BITMAPINFOHEADER), b->width, b->bottom_up ? b->height : -b->height, 1,
                   (WORD) b->bpp, BI_RGB, (DWORD) (b->stride * b->height), 0, 0, (DWORD) b->colors.size(), 0};
      INT n = size >= (INT) sizeof(DIBSECTION) ? sizeof(DIBSECTION) : sizeof(BITMAP);
      if (out == nullptr)
         return n;
      memcpy(out, &ds, std::min(size, n));
      return std::min(size, n);
   }
   }
   return 0;
}

// What a DC has selected when nothing was (Windows' defaults): the old object
// SelectObject returns and GetCurrentObject reports; selecting it back selects
// nothing again.  Memory DCs' default bitmap is a 1x1 monochrome one.
static HGDIOBJ DefaultObject(DWORD type)
{
   static HGDIOBJ bitmap = [] {
      HGDIOBJ b = CreateBitmap(1, 1, 1, 1, nullptr);
      GetGdiObject(b)->stock = true;
      return b;
   }();
   switch (type)
   {
   case OBJ_PEN: return GetStockObject(BLACK_PEN);
   case OBJ_BRUSH: return GetStockObject(WHITE_BRUSH);
   case OBJ_FONT: return GetStockObject(SYSTEM_FONT);
   case OBJ_BITMAP: return bitmap;
   case OBJ_PAL: return GetStockObject(DEFAULT_PALETTE);
   }
   return nullptr;
}

HGDIOBJ WINAPI SelectObject(HDC hdc, HGDIOBJ h)
{
   DC *dc = GetDCObject(hdc);
   GdiObject *o = GetGdiObject(h);
   if (dc == nullptr || o == nullptr)
      return nullptr;
   HGDIOBJ *slot = nullptr;
   switch (o->type)
   {
   case OBJ_PEN: slot = &dc->pen; break;
   case OBJ_BRUSH: slot = &dc->brush; break;
   case OBJ_FONT: slot = &dc->font; break;
   case OBJ_BITMAP:
      if (!dc->memory)
         return nullptr;
      // A bitmap is in at most one DC at a time.
      if (o->selected > 0 && dc->bitmap != h && !o->stock)
         return nullptr;
      slot = &dc->bitmap;
      break;
   case OBJ_REGION:
   {
      SelectClipRgn(hdc, (HRGN) h);
      return (HGDIOBJ) (UINT_PTR) SIMPLEREGION;
   }
   default:
      return nullptr;
   }
   HGDIOBJ old = *slot;
   if (GdiObject *oo = GetGdiObject(old))
      oo->selected--;
   if (o->type == OBJ_BITMAP && h == DefaultObject(OBJ_BITMAP))
      h = nullptr;
   else
      o->selected++;
   *slot = h;
   return old ? old : DefaultObject(o->type);
}

HPEN WINAPI CreatePen(INT style, INT width, COLORREF color)
{
   auto *p = new GdiPen;
   p->style = style & PS_STYLE_MASK;
   p->width = width < 1 ? 1 : width;
   p->color = color;
   return (HPEN) AddGdiObject(p);
}

HPEN WINAPI CreatePenIndirect(const LOGPEN *lp)
{
   return lp ? CreatePen(lp->lopnStyle, lp->lopnWidth.x, lp->lopnColor) : nullptr;
}

HBRUSH WINAPI CreateSolidBrush(COLORREF color)
{
   auto *b = new GdiBrush;
   b->color = color;
   return (HBRUSH) AddGdiObject(b);
}

HBRUSH WINAPI CreateBrushIndirect(const LOGBRUSH *lb)
{
   if (lb == nullptr)
      return nullptr;
   auto *b = new GdiBrush;
   b->style = lb->lbStyle == BS_HOLLOW ? BS_NULL : lb->lbStyle;
   b->color = lb->lbColor;
   b->hatch = (int) lb->lbHatch;
   if (b->style == BS_PATTERN)
      b->pattern = (HBITMAP) lb->lbHatch;
   return (HBRUSH) AddGdiObject(b);
}

HBRUSH WINAPI CreatePatternBrush(HBITMAP bitmap)
{
   auto *b = new GdiBrush;
   b->style = BS_PATTERN;
   b->pattern = bitmap;
   return (HBRUSH) AddGdiObject(b);
}

HBRUSH WINAPI CreateHatchBrush(INT hatch, COLORREF color)
{
   auto *b = new GdiBrush;
   b->style = BS_HATCHED;
   b->hatch = hatch;
   b->color = color;
   return (HBRUSH) AddGdiObject(b);
}

HFONT WINAPI CreateFontIndirectA(const LOGFONTA *lf)
{
   if (lf == nullptr)
      return nullptr;
   auto *f = new GdiFont;
   f->log = *lf;
   return (HFONT) AddGdiObject(f);
}

HFONT WINAPI CreateFontA(INT height, INT width, INT escapement, INT orientation, INT weight, DWORD italic,
                         DWORD underline, DWORD strikeout, DWORD charset, DWORD out_precision,
                         DWORD clip_precision, DWORD quality, DWORD pitch, LPCSTR face)
{
   LOGFONTA lf = {height, width, escapement, orientation, weight, (BYTE) italic, (BYTE) underline,
                  (BYTE) strikeout, (BYTE) charset, (BYTE) out_precision, (BYTE) clip_precision,
                  (BYTE) quality, (BYTE) pitch, ""};
   if (face)
      lstrcpynA(lf.lfFaceName, face, LF_FACESIZE);
   return CreateFontIndirectA(&lf);
}

HPALETTE WINAPI CreatePalette(const LOGPALETTE *lp)
{
   if (lp == nullptr)
      return nullptr;
   auto *p = new GdiPalette;
   p->entries.assign(lp->palPalEntry, lp->palPalEntry + lp->palNumEntries);
   return (HPALETTE) AddGdiObject(p);
}

UINT WINAPI GetPaletteEntries(HPALETTE h, UINT start, UINT count, PALETTEENTRY *entries)
{
   GdiPalette *p = GetGdiObjectAs<GdiPalette>(h, OBJ_PAL);
   if (p == nullptr)
      return 0;
   if (entries == nullptr)
      return (UINT) p->entries.size();
   UINT n = 0;
   for (UINT i = start; i < start + count && i < p->entries.size(); i++, n++)
      entries[n] = p->entries[i];
   return n;
}

UINT WINAPI SetPaletteEntries(HPALETTE h, UINT start, UINT count, const PALETTEENTRY *entries)
{
   GdiPalette *p = GetGdiObjectAs<GdiPalette>(h, OBJ_PAL);
   if (p == nullptr || entries == nullptr)
      return 0;
   UINT n = 0;
   for (UINT i = start; i < start + count && i < p->entries.size(); i++, n++)
      p->entries[i] = entries[n];
   return n;
}

UINT WINAPI GetNearestPaletteIndex(HPALETTE h, COLORREF color)
{
   GdiPalette *p = GetGdiObjectAs<GdiPalette>(h, OBJ_PAL);
   if (p == nullptr || p->entries.empty())
      return CLR_INVALID;
   UINT best = 0;
   int best_d = 1 << 30;
   for (UINT i = 0; i < p->entries.size(); i++)
   {
      int dr = p->entries[i].peRed - GetRValue(color), dg = p->entries[i].peGreen - GetGValue(color),
          db = p->entries[i].peBlue - GetBValue(color);
      int d = dr * dr + dg * dg + db * db;
      if (d < best_d)
         best_d = d, best = i;
   }
   return best;
}

UINT WINAPI GetSystemPaletteEntries(HDC, UINT start, UINT count, PALETTEENTRY *entries)
{
   // A true-color screen has no system palette; Windows returns 0 entries then.
   (void) start;
   (void) count;
   (void) entries;
   return 0;
}

HPALETTE WINAPI SelectPalette(HDC hdc, HPALETTE h, BOOL)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || GetGdiObjectAs<GdiPalette>(h, OBJ_PAL) == nullptr)
      return nullptr;
   HPALETTE old = dc->palette;
   dc->palette = h;
   return old;
}

UINT WINAPI RealizePalette(HDC hdc)
{
   // True color: nothing to map; the count of entries changed is 0.
   return GetDCObject(hdc) ? 0 : GDI_ERROR;
}

/************************************************************************/
// Device contexts

HDC WINAPI CreateCompatibleDC(HDC)
{
   auto *dc = new DC;
   dc->memory = true;
   dc->type = OBJ_MEMDC;
   return (HDC) AddGdiObject(dc);
}

HDC WINAPI CreateICA(LPCSTR, LPCSTR, LPCSTR, const DEVMODEA *)
{
   HDC hdc = CreateCompatibleDC(nullptr);
   if (DC *dc = GetDCObject(hdc))
      dc->info_only = true;
   return hdc;
}

HDC WINAPI CreateDCA(LPCSTR driver, LPCSTR device, LPCSTR output, const DEVMODEA *mode)
{
   return CreateICA(driver, device, output, mode);
}

BOOL WINAPI DeleteDC(HDC hdc)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return FALSE;
   for (HGDIOBJ h : {dc->pen, dc->brush, dc->font, dc->bitmap})
      if (GdiObject *o = GetGdiObject(h))
         o->selected--;
   Objects().Remove(hdc);
   delete dc;
   return TRUE;
}

INT WINAPI SaveDC(HDC hdc)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return 0;
   auto s = std::make_shared<DC::Saved>();
   s->state.pen = dc->pen;
   s->state.brush = dc->brush;
   s->state.font = dc->font;
   s->state.bitmap = dc->bitmap;
   s->state.palette = dc->palette;
   s->state.text_color = dc->text_color;
   s->state.bk_color = dc->bk_color;
   s->state.bk_mode = dc->bk_mode;
   s->state.rop2 = dc->rop2;
   s->state.stretch_mode = dc->stretch_mode;
   s->state.map_mode = dc->map_mode;
   s->state.text_align = dc->text_align;
   s->state.pos = dc->pos;
   s->state.origin = dc->origin;
   s->state.window_org = dc->window_org;
   s->state.viewport_org = dc->viewport_org;
   s->state.layout = dc->layout;
   s->state.clip = dc->clip;
   s->state.has_clip = dc->has_clip;
   dc->saved.push_back(s);
   return (INT) dc->saved.size();
}

BOOL WINAPI RestoreDC(HDC hdc, INT level)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || dc->saved.empty())
      return FALSE;
   size_t index = level < 0 ? dc->saved.size() + level : (size_t) level - 1;
   if (index >= dc->saved.size())
      return FALSE;
   const DC &s = dc->saved[index]->state;
   SelectObject(hdc, s.pen ? s.pen : DefaultObject(OBJ_PEN));
   SelectObject(hdc, s.brush ? s.brush : DefaultObject(OBJ_BRUSH));
   SelectObject(hdc, s.font ? s.font : DefaultObject(OBJ_FONT));
   if (dc->memory)
      SelectObject(hdc, s.bitmap ? s.bitmap : DefaultObject(OBJ_BITMAP));
   dc->palette = s.palette;
   dc->text_color = s.text_color;
   dc->bk_color = s.bk_color;
   dc->bk_mode = s.bk_mode;
   dc->rop2 = s.rop2;
   dc->stretch_mode = s.stretch_mode;
   dc->map_mode = s.map_mode;
   dc->text_align = s.text_align;
   dc->pos = s.pos;
   dc->origin = s.origin;
   dc->window_org = s.window_org;
   dc->viewport_org = s.viewport_org;
   dc->layout = s.layout;
   dc->clip = s.clip;
   dc->has_clip = s.has_clip;
   dc->saved.resize(index);
   return TRUE;
}

COLORREF WINAPI SetTextColor(HDC hdc, COLORREF color)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return CLR_INVALID;
   COLORREF old = dc->text_color;
   dc->text_color = color;
   return old;
}

COLORREF WINAPI GetTextColor(HDC hdc)
{
   DC *dc = GetDCObject(hdc);
   return dc ? dc->text_color : CLR_INVALID;
}

COLORREF WINAPI SetBkColor(HDC hdc, COLORREF color)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return CLR_INVALID;
   COLORREF old = dc->bk_color;
   dc->bk_color = color;
   return old;
}

COLORREF WINAPI GetBkColor(HDC hdc)
{
   DC *dc = GetDCObject(hdc);
   return dc ? dc->bk_color : CLR_INVALID;
}

INT WINAPI SetBkMode(HDC hdc, INT mode)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return 0;
   INT old = dc->bk_mode;
   dc->bk_mode = mode;
   return old;
}

INT WINAPI GetBkMode(HDC hdc)
{
   DC *dc = GetDCObject(hdc);
   return dc ? dc->bk_mode : 0;
}

INT WINAPI SetROP2(HDC hdc, INT rop)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return 0;
   INT old = dc->rop2;
   dc->rop2 = rop;
   return old;
}

INT WINAPI SetStretchBltMode(HDC hdc, INT mode)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return 0;
   INT old = dc->stretch_mode;
   dc->stretch_mode = mode;
   return old;
}

INT WINAPI GetStretchBltMode(HDC hdc)
{
   DC *dc = GetDCObject(hdc);
   return dc ? dc->stretch_mode : 0;
}

INT WINAPI SetMapMode(HDC hdc, INT mode)
{
   // Only MM_TEXT (device units) is drawn; the client sets nothing else for its
   // own drawing.
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return 0;
   INT old = dc->map_mode;
   dc->map_mode = mode;
   return old;
}

UINT WINAPI SetTextAlign(HDC hdc, UINT align)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return GDI_ERROR;
   UINT old = dc->text_align;
   dc->text_align = align;
   return old;
}

UINT WINAPI GetTextAlign(HDC hdc)
{
   DC *dc = GetDCObject(hdc);
   return dc ? dc->text_align : GDI_ERROR;
}

HGDIOBJ WINAPI GetCurrentObject(HDC hdc, UINT type)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return nullptr;
   switch (type)
   {
   case OBJ_PEN: return dc->pen ? dc->pen : DefaultObject(OBJ_PEN);
   case OBJ_BRUSH: return dc->brush ? dc->brush : DefaultObject(OBJ_BRUSH);
   case OBJ_FONT: return dc->font ? dc->font : DefaultObject(OBJ_FONT);
   case OBJ_BITMAP: return dc->bitmap ? dc->bitmap : DefaultObject(OBJ_BITMAP);
   case OBJ_PAL: return dc->palette ? dc->palette : DefaultObject(OBJ_PAL);
   }
   return nullptr;
}

INT WINAPI SelectClipRgn(HDC hdc, HRGN hrgn)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return ERROR;
   if (Region *r = GetRegion(hrgn))
   {
      dc->clip = *r;
      POINT device = DeviceOrigin(dc);   // the region is in device coordinates
      dc->clip.Offset(device.x, device.y);
      dc->has_clip = true;
      return dc->clip.Empty() ? NULLREGION : SIMPLEREGION;
   }
   dc->clip.Clear();
   dc->has_clip = false;
   return SIMPLEREGION;
}

INT WINAPI IntersectClipRect(HDC hdc, INT left, INT top, INT right, INT bottom)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return ERROR;
   RECT r = {left + dc->origin.x, top + dc->origin.y, right + dc->origin.x, bottom + dc->origin.y};
   if (!dc->has_clip)
   {
      dc->clip = Region(r);
      dc->has_clip = true;
   }
   else
      dc->clip.Intersect(r);
   return dc->clip.Empty() ? NULLREGION : SIMPLEREGION;
}

INT WINAPI ExcludeClipRect(HDC hdc, INT left, INT top, INT right, INT bottom)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr)
      return ERROR;
   RasterTarget t;
   if (!dc->has_clip)
   {
      dc->clip = GetTarget(dc, &t) ? Region(RECT{0, 0, t.width, t.height}) : Region(RECT{0, 0, 32767, 32767});
      dc->has_clip = true;
   }
   dc->clip.Subtract(RECT{left + dc->origin.x, top + dc->origin.y, right + dc->origin.x, bottom + dc->origin.y});
   return dc->clip.Empty() ? NULLREGION : COMPLEXREGION;
}

INT WINAPI GetClipBox(HDC hdc, RECT *rect)
{
   DC *dc = GetDCObject(hdc);
   if (dc == nullptr || rect == nullptr)
      return ERROR;
   RasterTarget t;
   if (!GetTarget(dc, &t))
   {
      *rect = {0, 0, 0, 0};
      return NULLREGION;
   }
   Region clip = EffectiveClip(dc, t);
   *rect = clip.Bounds();
   OffsetRect(rect, -dc->origin.x, -dc->origin.y);
   return clip.Empty() ? NULLREGION : SIMPLEREGION;
}

INT WINAPI GetDeviceCaps(HDC hdc, INT cap)
{
   DC *dc = GetDCObject(hdc);
   RasterTarget t;
   bool target = dc && GetTarget(dc, &t);
   RECT screen = native::ScreenRect();
   switch (cap)
   {
   case HORZRES: return screen.right - screen.left;
   case VERTRES: return screen.bottom - screen.top;
   case BITSPIXEL: return (target && dc->memory) ? t.bpp : 32;
   case PLANES: return 1;
   case NUMCOLORS: return -1;
   case LOGPIXELSX:
   case LOGPIXELSY: return 96;   // the client's layout assumes 96 dpi, as Windows' default
   case RASTERCAPS: return RC_BITBLT | RC_STRETCHBLT | RC_DI_BITMAP | RC_DIBTODEV | RC_STRETCHDIB;
   case SIZEPALETTE: return 0;
   case NUMRESERVED: return 0;
   case COLORRES: return 24;
   case TECHNOLOGY: return DT_RASDISPLAY;
   case HORZSIZE: return (screen.right - screen.left) * 254 / 960;
   case VERTSIZE: return (screen.bottom - screen.top) * 254 / 960;
   }
   return 0;
}

BOOL WINAPI GdiFlush(void)
{
   return TRUE;
}
