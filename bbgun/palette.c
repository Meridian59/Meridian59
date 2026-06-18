// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * palette.c:  Set up drawing palettes.
 *
 * Most of this deals with creating an "identity palette" for use with WinG.  In
 * other words, the system palette must be set up to match the BlakSton palette
 * exactly.
 */

#include "bbgun.h"

static RGBQUAD colors[NUM_COLORS];   /* Colors of our standard palette */
HPALETTE hPal;                       /* our standard drawing palette */
extern Color base_palette[];         

static int GetClosestPaletteIndex(COLORREF c);

/******************************************************************************/
/*
 *  ClearSystemPalette:  Clear the system palette so that we can ensure an 
 *  identity palette mapping for fast performance.
 */
void ClearSystemPalette(void)
{
   //*** A dummy palette setup
   struct
   {
      WORD Version;
      WORD NumberOfEntries;
      PALETTEENTRY aEntries[256];
   } Palette =
   {
      0x300,
      256
      };
   
   HPALETTE ScreenPalette = 0;
   HDC ScreenDC;
   int Counter;
   UINT nMapped = 0;
   BOOL bOK = FALSE;
   int  nOK = 0;
   
   //*** Reset everything in the system palette to black
   for(Counter = 0; Counter < 256; Counter++)
   {
      Palette.aEntries[Counter].peRed = 0;
      Palette.aEntries[Counter].peGreen = 0;
      Palette.aEntries[Counter].peBlue = 0;
      Palette.aEntries[Counter].peFlags = PC_NOCOLLAPSE;
   }
   
   //*** Create, select, realize, deselect, and delete the palette
   ScreenDC = GetDC(NULL);
   ScreenPalette = CreatePalette((LOGPALETTE *)&Palette);
   
   if (ScreenPalette)
   {
      ScreenPalette = SelectPalette(ScreenDC,ScreenPalette,FALSE);
      nMapped = RealizePalette(ScreenDC);
      ScreenPalette = SelectPalette(ScreenDC,ScreenPalette,FALSE);
      bOK = DeleteObject(ScreenPalette);
   }
   
   nOK = ReleaseDC(NULL, ScreenDC);
}
/************************************************************************/
/* 
 * Set up palettes.  Return new palette for drawing 3D view.
 */
HPALETTE InitializePalette(void)
{
   int has_palette;
   int i, reserved_colors;
   HPALETTE hTempPal;
   HDC hdc;
   struct
   {
      WORD Version;
      WORD NumberOfEntries;
      PALETTEENTRY aEntries[NUM_COLORS];
   } Palette =
   {
      0x300,
      NUM_COLORS
   };
   
   /* See if display is palette-based */
   hdc = GetDC(NULL);
   has_palette = GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE;

   if (has_palette)
   {
      ClearSystemPalette();
      
      /* Copy system colors */   
      reserved_colors = GetDeviceCaps(hdc, NUMRESERVED);
      
      GetSystemPaletteEntries(hdc, 0, 256, Palette.aEntries);
      
      for (i=0; i < NUM_COLORS; i++)
      {
	 colors[i].rgbRed =   Palette.aEntries[i].peRed;
	 colors[i].rgbBlue =  Palette.aEntries[i].peBlue;
	 colors[i].rgbGreen = Palette.aEntries[i].peGreen;
      }
   }

   if (has_palette)
   {
      /* Build up a new palette from system colors and our colors */
      for (i=0; i < reserved_colors / 2; i++)
      {
	 Palette.aEntries[i].peFlags = 0;
	 Palette.aEntries[i].peRed = colors[i].rgbRed;
	 Palette.aEntries[i].peGreen = colors[i].rgbGreen;
	 Palette.aEntries[i].peBlue = colors[i].rgbBlue;
      }
      
      for (i = reserved_colors / 2; i < NUM_COLORS - reserved_colors / 2; i++)
      {
	 Palette.aEntries[i].peFlags = PC_RESERVED;
	 Palette.aEntries[i].peRed   = colors[i].rgbRed   = base_palette[i].red;
	 Palette.aEntries[i].peBlue  = colors[i].rgbBlue  = base_palette[i].blue;
	 Palette.aEntries[i].peGreen = colors[i].rgbGreen = base_palette[i].green;
      }
      
      for (i = NUM_COLORS - reserved_colors / 2; i < NUM_COLORS; i++)
      {
	 Palette.aEntries[i].peFlags = 0;
	 Palette.aEntries[i].peRed = colors[i].rgbRed;
	 Palette.aEntries[i].peGreen = colors[i].rgbGreen;
	 Palette.aEntries[i].peBlue = colors[i].rgbBlue;
      }
   }
   else 
   {
      /* Just use our whole palette */
      for (i = 0; i < NUM_COLORS; i++)
      {
	 Palette.aEntries[i].peFlags = PC_RESERVED;
	 Palette.aEntries[i].peRed   = colors[i].rgbRed   = base_palette[i].red;
	 Palette.aEntries[i].peBlue  = colors[i].rgbBlue  = base_palette[i].blue;
	 Palette.aEntries[i].peGreen = colors[i].rgbGreen = base_palette[i].green;
      }
   }

   hTempPal = CreatePalette((LOGPALETTE *) &Palette);
   if (hTempPal == NULL)
      return NULL;
//      dprintf("CreatePalette failed!\n");

   SelectPalette(hdc, hTempPal, FALSE);
   RealizePalette(hdc);
   ReleaseDC(NULL, hdc);
   return hTempPal;
}
/************************************************************************/
void SetDIBPalette(HDC gdc)
{
   /* Set a WinG's bitmap (in given hdc) color table to our standard palette */

   SetDIBColorTable(gdc, 0, NUM_COLORS, colors);
}
/************************************************************************/
/*
 * GetNearestPaletteColor:  Return the color nearest to the given color
 *   in our standard palette.
 *   Requires that InitializePalette has been called.
 */
COLORREF GetNearestPaletteColor(COLORREF c)
{
   int index = GetClosestPaletteIndex(c);
   return RGB(colors[index].rgbRed, colors[index].rgbGreen, colors[index].rgbBlue);
}
/************************************************************************/
/*
 * GetClosestPaletteIndex:  Return the palette index of the color in our 
 *   standard palette nearest to the given color.
 *   Requires that InitializePalette has been called.
 */
int GetClosestPaletteIndex(COLORREF c)
{
   long i, rdist, bdist, gdist;
   long min_dist, distance, min_index;

   min_dist = 1L << 30;  /* Larger than distances to real colors */

   for (i=0; i < NUM_COLORS; i++)
   {
      rdist = colors[i].rgbRed - GetRValue(c);
      gdist = colors[i].rgbGreen - GetGValue(c);
      bdist = colors[i].rgbBlue - GetBValue(c);
      distance = rdist * rdist + bdist * bdist + gdist * gdist;
      if (distance < min_dist)
      {
	 min_dist = distance;
	 min_index = i;
      }
   }
   return min_index;
}
/************************************************************************/
/*
 * DibRemapToBasePalette:  Remap an 8-bit DIB's pixels from its own color
 *   table to the indices of our standard (base) palette, so that the raw
 *   indices line up with the offscreen buffer's color table when drawn.
 *   The transparent (254) and background (255) indices are preserved, since
 *   the drawing code relies on those specific index values.
 *   Requires that InitializePalette has been called.
 */
void DibRemapToBasePalette(PDIB pdib)
{
   RGBQUAD *src;
   BYTE map[NUM_COLORS];
   BYTE *bits, *row;

   if (pdib == NULL || DibBitCount(pdib) != 8)
      return;

   src = DibColors(pdib);
   int numColors = DibNumColors(pdib);
   if (numColors > NUM_COLORS)
      numColors = NUM_COLORS;

   for (int i = 0; i < numColors; i++)
      map[i] = (BYTE) GetClosestPaletteIndex(
		  RGB(src[i].rgbRed, src[i].rgbGreen, src[i].rgbBlue));

   // Preserve special indices used by the drawing code
   if (numColors > TRANSPARENT_INDEX)
      map[TRANSPARENT_INDEX] = TRANSPARENT_INDEX;
   if (numColors > BACKGROUND_INDEX)
      map[BACKGROUND_INDEX] = BACKGROUND_INDEX;

   bits = (BYTE *) DibPtr(pdib);
   int width = DibWidth(pdib);
   int height = DibHeight(pdib);
   int rowbytes = WIDTHBYTES(width);

   for (int y = 0; y < height; y++)
   {
      row = bits + y * rowbytes;
      for (int x = 0; x < width; x++)
	   if (row[x] < numColors)
	      row[x] = map[row[x]];
   }
}
