// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * draw.c:  Draw bitmaps.
 *
 * You must call DrawInit before drawing any bitmaps, and DrawClose when you are done.
 */

#include "bbgun.h"

#define TRANSPARENT_INDEX 254   // Index of transparent color
#define BACKGROUND_INDEX 255    // Index of background color

#define OFFSCREEN_BITMAP_SIZE 512

#define FIX_DECIMAL 16

#define HOTSPOT_INNER_RADIUS 1
#define HOTSPOT_OUTER_RADIUS 10

static HPEN hHotspotPen1, hHotspotPen2;

/* Offscreen bitmap & DC for drawing */
static HDC      gOffscreenDC;
static HBITMAP  gOldOffscreenBitmap;
static BYTE    *gBits;   /* Pointer to pixels of offscreen bitmap */

/* local function prototypes */
static HDC CreateMemBitmap(int width, int height, HBITMAP *gOldBitmap, BYTE **gBits);
/************************************************************************/
/*
 * DrawInit:  Setup up offscreen buffer.
 */
void DrawInit(void)
{
   hPal = InitializePalette();
   
   gOffscreenDC = CreateMemBitmap(OFFSCREEN_BITMAP_SIZE, OFFSCREEN_BITMAP_SIZE,
				   &gOldOffscreenBitmap, &gBits);

   hHotspotPen1 = CreatePen(PS_SOLID,2,RGB(150, 150, 0));
   hHotspotPen2 = CreatePen(PS_SOLID,2,RGB(255, 0, 0));
}
/************************************************************************/
/*
 * DrawClose:  Free offscreen buffer.
 */
void DrawClose(void)
{
   HBITMAP gCurrentBitmap;
   
   gCurrentBitmap = (HBITMAP) SelectObject(gOffscreenDC, gOldOffscreenBitmap);
   DeleteObject(gCurrentBitmap);
   DeleteDC(gOffscreenDC);
   
   DeleteObject(hPal);
   DeleteObject(hHotspotPen1);
   DeleteObject(hHotspotPen2);
}
/************************************************************************/
/*
 * CreateMemBitmap:  Create a bitmap, select it into a new memory DC,
 *   and return the DC.  Also set the palette of the bitmap.
 *   gOldBitmap is set to the bitmap originally in the DC.
 *   gBits is set to point to the bits of the DC.
 *   Returns NULL on failure.
 */
HDC CreateMemBitmap(int width, int height, HBITMAP *gOldBitmap, BYTE **gBits)
{
   HDC gDC;
   HBITMAP gBitmap;
   struct {
      BITMAPINFOHEADER header;
      RGBQUAD ColorTable[NUM_COLORS];
   } info;

   gDC = CreateCompatibleDC(NULL);
   if (gDC == 0)
   {
      return NULL;
   }

   /* Always top down; easier and we don't really care about speed here */
   memset(&info, 0, sizeof(BITMAPINFOHEADER));
   info.header.biSize = sizeof(BITMAPINFOHEADER);
   info.header.biPlanes = 1;
   info.header.biBitCount = 8;
   info.header.biCompression = BI_RGB;
   info.header.biHeight = - height; 
   info.header.biWidth = width;
   gBitmap = CreateDIBSection(gDC, (BITMAPINFO *) &info, DIB_RGB_COLORS,
                              (void **) gBits, NULL, 0);
   if (gBitmap == 0)
   {
      DeleteDC(gDC);
      return NULL;
   }
   *gOldBitmap = (HBITMAP) SelectObject(gDC, gBitmap);
   SetDIBPalette(gDC);
   return gDC;
}
/************************************************************************/
/*
 * DrawBitmap: Draw given bitmap at (x, y) on the offscreen buffer.
 *   Shrink bitmap by given factor.
 *   If transparent is True, draw bitmap transparently with background color.
 *   Return TRUE iff successful.
 */
BOOL DrawBitmap(PDIB pdib, int x, int y, float shrink, BOOL transparent)
{
   int width, height, row, xinc, yinc, xpos, ypos, xoffset, yoffset;
   int endx, endy;
   BYTE index;
   BYTE *bits, *ptr, *gPtr;
   
   width  = min(DibWidth(pdib), OFFSCREEN_BITMAP_SIZE);
   height = min(DibHeight(pdib), OFFSCREEN_BITMAP_SIZE);

   if (shrink == 0 || width == 0 || height == 0)
      return FALSE;

   xinc = (int) ((1 << FIX_DECIMAL) * shrink);
   yinc = (int) ((1 << FIX_DECIMAL) * shrink);

   endx = width << FIX_DECIMAL;
   endy = height << FIX_DECIMAL;

   // If bitmap too large, clip to sides
   if ((int) (x + width / shrink) >= OFFSCREEN_BITMAP_SIZE - 1)
      endx = ((int) ((OFFSCREEN_BITMAP_SIZE - x - 1) * shrink)) << FIX_DECIMAL;
   if ((int) (y + height / shrink) >= OFFSCREEN_BITMAP_SIZE - 1)
      endy = ((int) ((OFFSCREEN_BITMAP_SIZE - y - 1) * shrink)) << FIX_DECIMAL;

   // If displaying outside offscreen area, move drawing origin
   xoffset = 0;
   if (x < 0)
   {
      xoffset = - (x * xinc);
      x = 0;
   }
   yoffset = 0;
   if (y < 0)
   {
      yoffset = - (y * yinc);
      y = 0;
   }

   bits = (BYTE *) DibPtr(pdib);
   ypos = yoffset;
   row = 0;
   while (ypos < endy)
   {
      ptr = bits + (ypos >> FIX_DECIMAL) * WIDTHBYTES(width);  // DWORD aligned
      gPtr = gBits + (y + row) * OFFSCREEN_BITMAP_SIZE + x;
      
      xpos = xoffset;
      if (transparent)
      {
	 while (xpos < endx)
	 {
	    index = *(ptr + (xpos >> FIX_DECIMAL));
	    if (index != TRANSPARENT_INDEX)
	       *gPtr = index;
	    xpos += xinc;
	    gPtr++;
	 }
      }
      else
      {
	 while (xpos < endx)
	 {
	    *gPtr = *(ptr + (xpos >> FIX_DECIMAL));
	    xpos += xinc;
	    gPtr++;
	 }
      }
      ypos += yinc;
      row++;
   }
   
   return TRUE;
}
/************************************************************************/
/*
 * ClearBitmap:  Fill the offscreen bitmap with the background color.
 */
void ClearBitmap(void)
{
   int i;
   BYTE *gPtr;

   for (i=0; i < OFFSCREEN_BITMAP_SIZE; i++)
   {
      gPtr = gBits + i * OFFSCREEN_BITMAP_SIZE;
      memset(gPtr, BACKGROUND_INDEX, OFFSCREEN_BITMAP_SIZE);
   }
}
/************************************************************************/
/*
 * DisplayBitmaps:  Copy (width, height) of the offscreen bitmap to 
 *   (x, y) on hdc.
 */
void DisplayBitmaps(HDC hdc, int x, int y, int width, int height)
{
   SelectPalette(hdc, hPal, FALSE);
   RealizePalette(hdc);

   BitBlt(hdc, x, y, width, height, gOffscreenDC, 0, 0, SRCCOPY);   
   GdiFlush();
}
/************************************************************************/
/*
 * DrawHotspot: Draw hotspot centered at (x, y) on hdc.
 *   If highlight is TRUE, the hotspot is drawn in a different color.
 */
void DrawHotspot(HDC hdc, int x, int y, BOOL highlight)
{
   HPEN hPen;

   if (highlight)
      hPen = hHotspotPen2;
   else hPen = hHotspotPen1;
   
   SelectObject(hdc, hPen);
   SelectObject(hdc, (HBRUSH) GetStockObject(NULL_BRUSH));

   Ellipse(hdc, 
	   x - HOTSPOT_INNER_RADIUS, y - HOTSPOT_INNER_RADIUS,
	   x + HOTSPOT_INNER_RADIUS, y + HOTSPOT_INNER_RADIUS);

   Ellipse(hdc, 
	   x - HOTSPOT_OUTER_RADIUS, y - HOTSPOT_OUTER_RADIUS,
	   x + HOTSPOT_OUTER_RADIUS, y + HOTSPOT_OUTER_RADIUS);
}
