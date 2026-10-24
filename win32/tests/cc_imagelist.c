/*
 * cc_imagelist.c:  Image lists (Wine's imagelist.c) as the client's owner-drawn
 *   lists use them: ImageList_Create(16, 16, ILC_COLOR | ILC_MASK),
 *   ImageList_AddMasked with a teal key color, ImageList_Draw(ILD_TRANSPARENT)
 *   into a 32-bit DIB, ImageList_Destroy.  Checks the key color drew nothing and
 *   the rest drew the image, with a synthetic strip and the client's own
 *   (clientd3d/bitmap/imglist.bmp, when found); writes cc_imagelist.bmp.
 */

#include "cc_util.h"

#include <commoncontrols.h>

static const COLORREF kKey = RGB(0, 128, 128);
static const COLORREF kBack = RGB(255, 0, 255);

// An 8-bit DIB section of n 16x16 images: image i a filled square of one color
// with a key-colored 3-pixel border.
static HBITMAP MakeStrip(int n, const COLORREF *colors)
{
   struct
   {
      BITMAPINFOHEADER h;
      RGBQUAD colors[256];
   } bi = {};
   bi.h.biSize = sizeof(bi.h);
   bi.h.biWidth = 16 * n;
   bi.h.biHeight = 16;
   bi.h.biPlanes = 1;
   bi.h.biBitCount = 8;
   // Black first, as in the system palette the client's bitmaps have: image
   // lists clear masked pixels to index 0.
   bi.h.biClrUsed = (DWORD) n + 2;
   bi.colors[0] = {0, 0, 0, 0};
   bi.colors[1] = {GetBValue(kKey), GetGValue(kKey), GetRValue(kKey), 0};
   for (int i = 0; i < n; i++)
      bi.colors[i + 2] = {GetBValue(colors[i]), GetGValue(colors[i]), GetRValue(colors[i]), 0};
   void *bits;
   HBITMAP bmp = CreateDIBSection(NULL, (BITMAPINFO *) &bi, DIB_RGB_COLORS, &bits, NULL, 0);
   int stride = (16 * n + 3) & ~3;
   for (int y = 0; y < 16; y++)
      for (int x = 0; x < 16 * n; x++)
      {
         int lx = x % 16;
         bool border = lx < 3 || lx >= 13 || y < 3 || y >= 13;
         ((BYTE *) bits)[y * stride + x] = border ? 1 : (BYTE) (x / 16 + 2);
      }
   return bmp;
}

// The client's strip, from its packed DIB file.
static HBITMAP LoadClientStrip(void)
{
   FILE *f = fopen("../../../clientd3d/bitmap/imglist.bmp", "rb");
   if (f == NULL)
      return NULL;
   static BYTE data[65536];
   size_t n = fread(data, 1, sizeof(data), f);
   fclose(f);
   if (n < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
      return NULL;
   BITMAPFILEHEADER *fh = (BITMAPFILEHEADER *) data;
   BITMAPINFO *bi = (BITMAPINFO *) (data + sizeof(BITMAPFILEHEADER));
   void *bits;
   HBITMAP bmp = CreateDIBSection(NULL, bi, DIB_RGB_COLORS, &bits, NULL, 0);
   int stride = ((bi->bmiHeader.biWidth * bi->bmiHeader.biBitCount + 31) / 32) * 4;
   memcpy(bits, data + fh->bfOffBits, (size_t) stride * abs(bi->bmiHeader.biHeight));
   return bmp;
}

static void CheckDrawn(const Canvas32 *c, int x0, int y0, COLORREF color, int tolerance)
{
   int key = 0, back = 0, drawn = 0;
   for (int y = 0; y < 16; y++)
      for (int x = 0; x < 16; x++)
      {
         COLORREF p = CanvasPixel(c, x0 + x, y0 + y);
         bool border = x < 3 || x >= 13 || y < 3 || y >= 13;
         if (p == kKey)
            key++;
         if (border && p == kBack)
            back++;
         if (!border && Near(p, color, tolerance))
            drawn++;
      }
   CHECK_INT(key, 0);
   CHECK_INT(back, 256 - 100);
   CHECK_INT(drawn, 100);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int show)
{
   (void) instance, (void) prev, (void) cmd, (void) show;
   InitCommonControls();
   Canvas32 c = MakeCanvas(200, 60, kBack);

   // The client's flags: ILC_COLOR (4-bit, the 16 VGA colors) with a mask.
   const COLORREF colors[3] = {RGB(255, 0, 0), RGB(0, 0, 255), RGB(0, 0, 0)};
   HIMAGELIST himl = ImageList_Create(16, 16, ILC_COLOR | ILC_MASK, 0, 1);
   CHECK(himl != NULL);
   CHECK_INT(ImageList_GetImageCount(himl), 0);
   HBITMAP strip = MakeStrip(3, colors);
   CHECK_INT(ImageList_AddMasked(himl, strip, kKey), 0);
   DeleteObject(strip);
   CHECK_INT(ImageList_GetImageCount(himl), 3);
   int cx = 0, cy = 0;
   CHECK(ImageList_GetIconSize(himl, &cx, &cy));
   CHECK_INT(cx, 16);
   CHECK_INT(cy, 16);
   for (int i = 0; i < 3; i++)
   {
      CHECK(ImageList_Draw(himl, i, c.dc, 2 + 20 * i, 2, ILD_TRANSPARENT));
      CheckDrawn(&c, 2 + 20 * i, 2, colors[i], 0);
   }
   CHECK(ImageList_Destroy(himl));

   // 32-bit color: any color stays exact.
   const COLORREF odd[2] = {RGB(10, 200, 30), RGB(250, 180, 90)};
   himl = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
   strip = MakeStrip(2, odd);
   CHECK_INT(ImageList_AddMasked(himl, strip, kKey), 0);
   DeleteObject(strip);
   for (int i = 0; i < 2; i++)
   {
      CHECK(ImageList_Draw(himl, i, c.dc, 70 + 20 * i, 2, ILD_TRANSPARENT));
      CheckDrawn(&c, 70 + 20 * i, 2, odd[i], 0);
   }
   // ILD_NORMAL with a background color fills the masked part with it.
   ImageList_SetBkColor(himl, RGB(0, 255, 0));
   CHECK(ImageList_Draw(himl, 0, c.dc, 110, 2, ILD_NORMAL));
   CHECK_INT(CanvasPixel(&c, 110, 2), RGB(0, 255, 0));
   CHECK_INT(CanvasPixel(&c, 118, 10), odd[0]);
   // Through the COM interface too.
   IImageList *iface = NULL;
   CHECK(SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_IImageList, (void **) &iface)));
   if (iface)
   {
      int count = 0;
      CHECK(SUCCEEDED(iface->GetImageCount(&count)));
      CHECK_INT(count, 2);
      iface->Release();
   }
   CHECK(ImageList_Destroy(himl));

   // The client's own strip, drawn as ownerdrw.c does.
   HBITMAP client = LoadClientStrip();
   if (client)
   {
      himl = ImageList_Create(16, 16, ILC_COLOR | ILC_MASK, 0, 1);
      CHECK_INT(ImageList_AddMasked(himl, client, kKey), 0);
      DeleteObject(client);
      int n = ImageList_GetImageCount(himl);
      CHECK_INT(n, 8);
      for (int i = 0; i < n; i++)
         ImageList_Draw(himl, i, c.dc, 2 + 20 * i, 30, ILD_TRANSPARENT);
      RECT row = {2, 30, 2 + 20 * 8, 46};
      CHECK_INT(CountColor(&c, row, kKey, 0), 0);
      CHECK(CountColor(&c, row, kBack, 0) > 100);         // key pixels left the background
      CHECK(CountColor(&c, row, kBack, 0) < 160 * 16 - 600); // the icons drew
      ImageList_Destroy(himl);
   }
   else
      printf("clientd3d/bitmap/imglist.bmp not found: skipped\n");
   DumpBmp(&c, "cc_imagelist.bmp");
   FreeCanvas(&c);
   return CheckReport("cc_imagelist");
}
