/*
 * gdi_mask.c:  The client's transparent sprite blit (drawbmp.c,
 *   DrawStretchedObjectGroupTransparent): an 8-bit DIB with a chroma-key index,
 *   copied to a 32-bit bitmap, a monochrome mask from the key, SRCAND and
 *   SRCPAINT onto a 32-bit target.  The sprite must survive, the key must not.
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); failures++; } } while (0)

static COLORREF Pixel(HDC dc, int x, int y)
{
   return GetPixel(dc, x, y) & 0xFFFFFF;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   const int size = 16, key = 254, sprite = 10;
   const COLORREF key_rgb = RGB(0, 255, 255), sprite_rgb = RGB(200, 30, 40), face = RGB(212, 208, 200);

   // The offscreen: 8-bit top-down DIB section, color table set afterwards.
   struct { BITMAPINFOHEADER h; RGBQUAD colors[256]; } info;
   memset(&info, 0, sizeof(info));
   info.h.biSize = sizeof(BITMAPINFOHEADER);
   info.h.biPlanes = 1;
   info.h.biBitCount = 8;
   info.h.biCompression = BI_RGB;
   info.h.biWidth = size;
   info.h.biHeight = -size;
   BYTE *bits;
   HDC off = CreateCompatibleDC(NULL);
   HBITMAP dib = CreateDIBSection(off, (BITMAPINFO *) &info, DIB_RGB_COLORS, (void **) &bits, NULL, 0);
   SelectObject(off, dib);
   RGBQUAD table[256];
   memset(table, 0, sizeof(table));
   table[key].rgbRed = GetRValue(key_rgb), table[key].rgbGreen = GetGValue(key_rgb), table[key].rgbBlue = GetBValue(key_rgb);
   table[sprite].rgbRed = GetRValue(sprite_rgb), table[sprite].rgbGreen = GetGValue(sprite_rgb), table[sprite].rgbBlue = GetBValue(sprite_rgb);
   SetDIBColorTable(off, 0, 256, table);
   memset(bits, key, size * size);
   for (int y = 4; y < 12; y++)
      memset(bits + y * size + 4, sprite, 8);

   // The destination: a 32-bit bitmap standing in for the dialog's window DC.
   HDC screen = GetDC(NULL);
   HDC dest = CreateCompatibleDC(screen);
   HBITMAP dest_bmp = CreateCompatibleBitmap(screen, size, size);
   SelectObject(dest, dest_bmp);
   RECT all = {0, 0, size, size};
   HBRUSH face_brush = CreateSolidBrush(face);
   FillRect(dest, &all, face_brush);

   // drawbmp.c's sequence.
   HDC mem = CreateCompatibleDC(dest);
   HBITMAP mem_bmp = CreateCompatibleBitmap(dest, size, size);
   SelectObject(mem, mem_bmp);
   BitBlt(mem, 0, 0, size, size, off, 0, 0, SRCCOPY);
   CHECK(Pixel(mem, 0, 0) == key_rgb);
   CHECK(Pixel(mem, 6, 6) == sprite_rgb);

   HDC mask = CreateCompatibleDC(dest);
   HBITMAP mask_bmp = CreateBitmap(size, size, 1, 1, NULL);
   SelectObject(mask, mask_bmp);
   SetBkColor(mem, key_rgb);
   BitBlt(mask, 0, 0, size, size, mem, 0, 0, SRCCOPY);
   CHECK(Pixel(mask, 0, 0) == RGB(255, 255, 255));   // key: 1
   CHECK(Pixel(mask, 6, 6) == RGB(0, 0, 0));         // sprite: 0

   SetBkColor(mem, RGB(0, 0, 0));
   SetTextColor(mem, RGB(255, 255, 255));
   BitBlt(mem, 0, 0, size, size, mask, 0, 0, SRCAND);
   CHECK(Pixel(mem, 0, 0) == RGB(0, 0, 0));
   CHECK(Pixel(mem, 6, 6) == sprite_rgb);

   SetBkColor(dest, RGB(255, 255, 255));
   SetTextColor(dest, RGB(0, 0, 0));
   BitBlt(dest, 0, 0, size, size, mask, 0, 0, SRCAND);
   CHECK(Pixel(dest, 0, 0) == face);
   CHECK(Pixel(dest, 6, 6) == RGB(0, 0, 0));
   BitBlt(dest, 0, 0, size, size, mem, 0, 0, SRCPAINT);
   CHECK(Pixel(dest, 0, 0) == face);
   CHECK(Pixel(dest, 6, 6) == sprite_rgb);

   printf(failures ? "gdi_mask: %d failed\n" : "gdi_mask: all passed\n", failures);
   return failures != 0;
}
