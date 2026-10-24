/*
 * cursor.cpp:  LoadCursor, LoadIcon, LoadImage: standard shapes, and cursors and
 *   icons from RT_GROUP_CURSOR / RT_GROUP_ICON resources (the image closest to
 *   the size wanted, decoded from its DIB with the AND mask as transparency).
 */

#include "core/cursor.h"

#include "core/gdi.h"
#include "core/handles.h"
#include "core/module.h"

#include <cstring>
#include <map>

namespace w32 {

namespace {

HandleTable<CursorImage, HANDLE> &Cursors()
{
   static HandleTable<CursorImage, HANDLE> cursors(kMiscHandleBase);
   return cursors;
}

const char *StandardName(UINT_PTR id)
{
   static const struct
   {
      LPCSTR id;
      const char *name;
   } names[] = {
      {IDC_ARROW, "default"}, {IDC_IBEAM, "text"}, {IDC_WAIT, "wait"}, {IDC_CROSS, "crosshair"},
      {IDC_UPARROW, "default"}, {IDC_SIZENWSE, "nwse-resize"}, {IDC_SIZENESW, "nesw-resize"},
      {IDC_SIZEWE, "ew-resize"}, {IDC_SIZENS, "ns-resize"}, {IDC_SIZEALL, "move"}, {IDC_NO, "not-allowed"},
      {IDC_HAND, "pointer"}, {IDC_APPSTARTING, "progress"}, {IDC_HELP, "help"},
   };
   for (const auto &n : names)
      if ((UINT_PTR) n.id == id)
         return n.name;
   return nullptr;
}

HANDLE Standard(const char *name)
{
   static std::map<std::string, HANDLE> made;
   auto it = made.find(name);
   if (it != made.end())
      return it->second;
   auto *c = new CursorImage;
   c->standard = name;
   HANDLE h = Cursors().Add(c);
   made[name] = h;
   return h;
}

#pragma pack(push, 2)
struct GroupHeader
{
   WORD reserved, type, count;
};
struct GroupEntry
{
   // Icons: width, height, colors, reserved, planes, bit count.  Cursors: width,
   // height (twice: XOR and AND masks), planes, bit count.
   BYTE width, height, colors, reserved;
   WORD planes, bit_count;
   DWORD bytes;
   WORD id;
};
#pragma pack(pop)

// Decodes a cursor/icon image resource (a DIB with XOR and AND masks; cursors
// first have the hot spot as two WORDs).  Returns false for PNG images.
bool Decode(const BYTE *data, DWORD size, bool cursor, CursorImage *out)
{
   if (cursor)
   {
      if (size < 4)
         return false;
      out->hot_x = ((const WORD *) data)[0];
      out->hot_y = ((const WORD *) data)[1];
      data += 4;
      size -= 4;
   }
   if (size < sizeof(BITMAPINFOHEADER) || memcmp(data, "\x89PNG", 4) == 0)
      return false;
   const BITMAPINFOHEADER *h = (const BITMAPINFOHEADER *) data;
   int w = h->biWidth, ht = abs(h->biHeight) / 2;   // the height counts both masks
   int bpp = h->biBitCount;
   int colors = bpp <= 8 ? (h->biClrUsed ? h->biClrUsed : 1 << bpp) : 0;
   const RGBQUAD *table = (const RGBQUAD *) (data + h->biSize);
   const BYTE *xor_bits = (const BYTE *) (table + colors);
   int xor_stride = ((w * bpp + 31) / 32) * 4;
   const BYTE *and_bits = xor_bits + (size_t) xor_stride * ht;
   int and_stride = ((w + 31) / 32) * 4;
   if (w <= 0 || ht <= 0 || and_bits + (size_t) and_stride * ht > data + size)
      return false;
   out->width = w;
   out->height = ht;
   out->pixels.assign((size_t) w * ht, 0);
   bool alpha = bpp == 32;
   for (int y = 0; y < ht; y++)
   {
      // DIB rows are bottom-up.
      const BYTE *xr = xor_bits + (size_t) (ht - 1 - y) * xor_stride;
      const BYTE *ar = and_bits + (size_t) (ht - 1 - y) * and_stride;
      for (int x = 0; x < w; x++)
      {
         uint32_t rgb;
         uint32_t a = 0xFF;
         switch (bpp)
         {
         case 32: rgb = (xr[x * 4 + 2] << 16) | (xr[x * 4 + 1] << 8) | xr[x * 4]; a = xr[x * 4 + 3]; break;
         case 24: rgb = (xr[x * 3 + 2] << 16) | (xr[x * 3 + 1] << 8) | xr[x * 3]; break;
         default:
         {
            int index;
            if (bpp == 8)
               index = xr[x];
            else if (bpp == 4)
               index = (xr[x / 2] >> ((x & 1) ? 0 : 4)) & 0xF;
            else
               index = (xr[x / 8] >> (7 - (x & 7))) & 1;
            const RGBQUAD &q = table[index < colors ? index : 0];
            rgb = (q.rgbRed << 16) | (q.rgbGreen << 8) | q.rgbBlue;
         }
         }
         bool transparent = (ar[x / 8] >> (7 - (x & 7))) & 1;
         if (!alpha && transparent)
            a = 0;
         out->pixels[(size_t) y * w + x] = (a << 24) | rgb;
      }
   }
   return true;
}

HANDLE LoadFromResource(HINSTANCE instance, LPCSTR name, bool cursor, int want)
{
   const BYTE *group;
   DWORD size;
   if (!FindResourceData(instance, name, cursor ? (LPCSTR) RT_GROUP_CURSOR : (LPCSTR) RT_GROUP_ICON, &group, &size) ||
       size < sizeof(GroupHeader))
      return nullptr;
   const GroupHeader *gh = (const GroupHeader *) group;
   const GroupEntry *entries = (const GroupEntry *) (gh + 1);
   // The image whose width is closest to the size wanted, the deepest of those.
   const GroupEntry *best = nullptr;
   for (int i = 0; i < gh->count; i++)
   {
      const GroupEntry &e = entries[i];
      int w = cursor ? *(const WORD *) &e.width : (e.width ? e.width : 256);
      if (best == nullptr)
      {
         best = &e;
         continue;
      }
      int bw = cursor ? *(const WORD *) &best->width : (best->width ? best->width : 256);
      if (abs(w - want) < abs(bw - want) || (w == bw && e.bit_count > best->bit_count))
         best = &e;
   }
   if (best == nullptr)
      return nullptr;
   const BYTE *image;
   DWORD image_size;
   if (!FindResourceData(instance, MAKEINTRESOURCEA(best->id), cursor ? (LPCSTR) RT_CURSOR : (LPCSTR) RT_ICON,
                         &image, &image_size))
      return nullptr;
   auto *c = new CursorImage;
   if (!Decode(image, image_size, cursor, c))
   {
      delete c;
      return Standard("default");
   }
   if (!cursor)
      for (int i = 0; i < gh->count; i++)
      {
         const BYTE *other;
         DWORD other_size;
         CursorImage img;
         if (FindResourceData(instance, MAKEINTRESOURCEA(entries[i].id), (LPCSTR) RT_ICON, &other, &other_size) &&
             Decode(other, other_size, false, &img))
            c->sizes.push_back(std::move(img));
      }
   return Cursors().Add(c);
}

}  // namespace

const CursorImage *GetCursorImage(HANDLE h)
{
   return Cursors().Get(h);
}

HANDLE AddCursorImage(CursorImage *image)
{
   return Cursors().Add(image);
}

}  // namespace w32

using namespace w32;

HCURSOR WINAPI LoadCursorA(HINSTANCE instance, LPCSTR name)
{
   if (instance == nullptr)
   {
      const char *standard = IS_INTRESOURCE(name) ? StandardName((UINT_PTR) name) : nullptr;
      return standard ? (HCURSOR) Standard(standard) : nullptr;
   }
   return (HCURSOR) LoadFromResource(instance, name, true, 32);
}

HICON WINAPI LoadIconA(HINSTANCE instance, LPCSTR name)
{
   // IDI_HAND and the others: user32's icons (Wine's, win32/res/user32.rc.in).
   if (instance == nullptr)
      return IS_INTRESOURCE(name) ? (HICON) LoadFromResource(BuiltinDll("user32.dll"), name, false, 32) : nullptr;
   return (HICON) LoadFromResource(instance, name, false, 32);
}

HANDLE WINAPI LoadImageA(HINSTANCE instance, LPCSTR name, UINT type, INT cx, INT cy, UINT flags)
{
   (void) cy;
   if (flags & LR_LOADFROMFILE)
      return nullptr;   // the client loads images from its resources only
   switch (type)
   {
   case IMAGE_BITMAP: return LoadBitmapA(instance, name);
   case IMAGE_CURSOR: return instance ? LoadFromResource(instance, name, true, cx ? cx : 32) : LoadCursorA(nullptr, name);
   case IMAGE_ICON: return instance ? LoadFromResource(instance, name, false, cx ? cx : 32) : LoadIconA(nullptr, name);
   }
   return nullptr;
}

BOOL WINAPI DestroyCursor(HCURSOR)
{
   // Shared, as LoadCursor's are.
   return TRUE;
}

BOOL WINAPI DestroyIcon(HICON)
{
   return TRUE;
}
