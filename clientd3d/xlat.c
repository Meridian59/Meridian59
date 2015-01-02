// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * xlat.c:  Color Translations
 */

#include "client.h"

extern BYTE light_palettes[NUM_PALETTES][NUM_COLORS];

// Destination indexes for a ramp to old hair types
static BYTE oldhair1_indexes[] = {
   0x23, 0x32, 0x34, 0x36, 0x59, 0x39, 0x3A, 0x3B, 
   0x36, 0x38, 0x5B, 0x47, 0x3C, 0x5C, 0x5E, 0x5E,
};
static BYTE oldhair2_indexes[] = {
   0x50, 0x23, 0x24, 0x33, 0x25, 0x34, 0x26, 0x35, 
   0x27, 0x36, 0x28, 0x37, 0x29, 0x38, 0x2A, 0x39,
};
static BYTE oldhair3_indexes[] = {
   0xC4, 0x52, 0xC6, 0x53, 0xC8, 0x54, 0xCA, 0x55, 
   0x56, 0x57, 0x58, 0x58, 0x59, 0x59, 0x5A, 0x5B,
};
static BYTE platblond_indexes[] = {
   0xB0, 0xB0, 0xB1, 0xB1, 0xB2, 0xB2, 0xD0, 0xD1, 
   0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD9, 0xDA,
};
static BYTE skin1_indexes[] = {
   0x20, 0xF0, 0xF0, 0x21, 0x21, 0x22, 0x22, 0x23, 
   0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,
};
static BYTE skin2_indexes[] = {
   0x20, 0x20, 0xF0, 0x21, 0x22, 0x23, 0x23, 0x24, 
   0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
};
static BYTE green_skin_indexes[] = {
   0xD0, 0xD0, 0xB0, 0xB8, 0x60, 0x60, 0x61, 0x61, 
   0x62, 0x63, 0x64, 0x66, 0x67, 0x68, 0x6A, 0x6C,
};
static BYTE yellow_skin_indexes[] = {
   0xD0, 0xD0, 0xB0, 0xB0, 0xB1, 0xB2, 0xB3, 0xC0, 
   0xC6, 0xC9, 0xCA, 0xCC, 0xCD, 0xCE, 0xCF, 0xCF,
};
static BYTE ramps[] =
{
   // Guild colors and fabric/skin body translation ramps.
   // Must be eleven ramps to match other math.
   0x10, 0x20, 0x30, 0x40, 0x50, 0x70, 0x90, 0xA0, 0xC0, 0xD0, 0xE0,
};
/***************************************************************************/

/* To add a new standard xlat,
 *   1:  define a symbol for it in xlat.h
 *   2:  add an IMPLEMENT_XLAT line in the FindStandardXlat() below
 *   3:  initialize the xlat in InitStandardXlats() below
 * Be sparing in standard xlats, each one takes up static data space.
 */

xlat* FindStandardXlat(UINT uXlatID)
{

/* IMPLEMENT_XLAT() combines the lookup from ID number to pointer
 *                  and the actual static storage for the xlat.
 */
#define IMPLEMENT_XLAT(symbol) case symbol: { static xlat _std_##symbol; return &_std_##symbol; }

	switch (uXlatID)
	{

	IMPLEMENT_XLAT(XLAT_IDENTITY);

	IMPLEMENT_XLAT(XLAT_BLEND10RED);
	IMPLEMENT_XLAT(XLAT_BLEND20RED);
	IMPLEMENT_XLAT(XLAT_BLEND30RED);
	IMPLEMENT_XLAT(XLAT_BLEND40RED);
	IMPLEMENT_XLAT(XLAT_BLEND50RED);
	IMPLEMENT_XLAT(XLAT_BLEND60RED);
	IMPLEMENT_XLAT(XLAT_BLEND70RED);
	IMPLEMENT_XLAT(XLAT_BLEND80RED);
	IMPLEMENT_XLAT(XLAT_BLEND90RED);
	IMPLEMENT_XLAT(XLAT_BLEND100RED);

	IMPLEMENT_XLAT(XLAT_BLEND10WHITE);
	IMPLEMENT_XLAT(XLAT_BLEND20WHITE);
	IMPLEMENT_XLAT(XLAT_BLEND30WHITE);
	IMPLEMENT_XLAT(XLAT_BLEND40WHITE);
	IMPLEMENT_XLAT(XLAT_BLEND50WHITE);
	IMPLEMENT_XLAT(XLAT_BLEND60WHITE);
	IMPLEMENT_XLAT(XLAT_BLEND70WHITE);
	IMPLEMENT_XLAT(XLAT_BLEND80WHITE);
	IMPLEMENT_XLAT(XLAT_BLEND90WHITE);
	IMPLEMENT_XLAT(XLAT_BLEND100WHITE);

	IMPLEMENT_XLAT(XLAT_BLEND25YELLOW);

	IMPLEMENT_XLAT(XLAT_BLEND25RED);
	//IMPLEMENT_XLAT(XLAT_BLEND50RED);
	IMPLEMENT_XLAT(XLAT_BLEND75RED);
	IMPLEMENT_XLAT(XLAT_BLEND25GREEN);
	IMPLEMENT_XLAT(XLAT_BLEND50GREEN);
	IMPLEMENT_XLAT(XLAT_BLEND75GREEN);
	IMPLEMENT_XLAT(XLAT_BLEND25BLUE);
	IMPLEMENT_XLAT(XLAT_BLEND50BLUE);
	IMPLEMENT_XLAT(XLAT_BLEND75BLUE);

	IMPLEMENT_XLAT(XLAT_FILTERRED);
	IMPLEMENT_XLAT(XLAT_FILTERGREEN);
	IMPLEMENT_XLAT(XLAT_FILTERBLUE);

	IMPLEMENT_XLAT(XLAT_GRAYTORED);
	IMPLEMENT_XLAT(XLAT_GRAYTOORANGE);
	IMPLEMENT_XLAT(XLAT_GRAYTODGREEN);
	IMPLEMENT_XLAT(XLAT_GRAYTOBGREEN);
	IMPLEMENT_XLAT(XLAT_GRAYTOSKY);
	IMPLEMENT_XLAT(XLAT_GRAYTODBLUE);
	IMPLEMENT_XLAT(XLAT_GRAYTOPURPLE);
	IMPLEMENT_XLAT(XLAT_GRAYTOGOLD);
	IMPLEMENT_XLAT(XLAT_GRAYTOBBLUE);

	IMPLEMENT_XLAT(XLAT_GRAYTOLRED);
	IMPLEMENT_XLAT(XLAT_GRAYTOLORANGE);
	IMPLEMENT_XLAT(XLAT_GRAYTOLGREEN);
	IMPLEMENT_XLAT(XLAT_GRAYTOLBGREEN);
	IMPLEMENT_XLAT(XLAT_GRAYTOLSKY);
	IMPLEMENT_XLAT(XLAT_GRAYTOLBLUE);
	IMPLEMENT_XLAT(XLAT_GRAYTOLPURPLE);
	IMPLEMENT_XLAT(XLAT_GRAYTOLGOLD);
	IMPLEMENT_XLAT(XLAT_GRAYTOLBBLUE);

	IMPLEMENT_XLAT(XLAT_GRAYTOKRED);
	IMPLEMENT_XLAT(XLAT_GRAYTOKORANGE);
	IMPLEMENT_XLAT(XLAT_GRAYTOKGREEN);
	IMPLEMENT_XLAT(XLAT_GRAYTOKBGREEN);
	IMPLEMENT_XLAT(XLAT_GRAYTOKSKY);
	IMPLEMENT_XLAT(XLAT_GRAYTOKBLUE);
	IMPLEMENT_XLAT(XLAT_GRAYTOKPURPLE);
	IMPLEMENT_XLAT(XLAT_GRAYTOKGOLD);
	IMPLEMENT_XLAT(XLAT_GRAYTOKBBLUE);
	IMPLEMENT_XLAT(XLAT_GRAYTOKGRAY);

	IMPLEMENT_XLAT(XLAT_GRAYTOBLACK);

	IMPLEMENT_XLAT(XLAT_GRAYTOOLDHAIR1);
	IMPLEMENT_XLAT(XLAT_GRAYTOOLDHAIR2);
	IMPLEMENT_XLAT(XLAT_GRAYTOOLDHAIR3);

	IMPLEMENT_XLAT(XLAT_GRAYTOPLATBLOND);

	IMPLEMENT_XLAT(XLAT_GRAYTOSKIN1);
	IMPLEMENT_XLAT(XLAT_GRAYTOSKIN2);
	IMPLEMENT_XLAT(XLAT_GRAYTOSKIN3);
	IMPLEMENT_XLAT(XLAT_GRAYTOSKIN4);
	IMPLEMENT_XLAT(XLAT_GRAYTOSKIN5);

	IMPLEMENT_XLAT(XLAT_DBLUETOSKIN1);
	IMPLEMENT_XLAT(XLAT_DBLUETOSKIN2);
	IMPLEMENT_XLAT(XLAT_DBLUETOSKIN3);
	IMPLEMENT_XLAT(XLAT_DBLUETOSKIN4);

	IMPLEMENT_XLAT(XLAT_DBLUETOSICKGREEN);
	IMPLEMENT_XLAT(XLAT_DBLUETOSICKYELLOW);

	IMPLEMENT_XLAT(XLAT_DBLUETOGRAY);
	IMPLEMENT_XLAT(XLAT_DBLUETOLBLUE);

	IMPLEMENT_XLAT(XLAT_DBLUETOASHEN);

	IMPLEMENT_XLAT(XLAT_REDTOBLACK);
	IMPLEMENT_XLAT(XLAT_BLUETOBLACK);
	IMPLEMENT_XLAT(XLAT_PURPLETOBLACK);

	IMPLEMENT_XLAT(XLAT_REDTOBLACK1);
	IMPLEMENT_XLAT(XLAT_REDTOBLACK2);
	IMPLEMENT_XLAT(XLAT_REDTOBLACK3);
	IMPLEMENT_XLAT(XLAT_REDTODKBLACK1);
	IMPLEMENT_XLAT(XLAT_REDTODKBLACK2);
	IMPLEMENT_XLAT(XLAT_REDTODKBLACK3);
	IMPLEMENT_XLAT(XLAT_REDBLK_BLWHT);
	IMPLEMENT_XLAT(XLAT_BLBLK_REDWHT);
	IMPLEMENT_XLAT(XLAT_REDTODGREEN1);
	IMPLEMENT_XLAT(XLAT_REDTODGREEN2);
	IMPLEMENT_XLAT(XLAT_REDTODGREEN3);

	IMPLEMENT_XLAT(XLAT_PURPLETOLBLUE);
	IMPLEMENT_XLAT(XLAT_PURPLETOBRED);
	IMPLEMENT_XLAT(XLAT_PURPLETOGREEN);
	IMPLEMENT_XLAT(XLAT_PURPLETOYELLOW);

	IMPLEMENT_XLAT(XLAT_RAMPUP1);
	IMPLEMENT_XLAT(XLAT_RAMPUP2);
	IMPLEMENT_XLAT(XLAT_RAMPDOWN2);
	IMPLEMENT_XLAT(XLAT_RAMPDOWN1);

	IMPLEMENT_XLAT(XLAT_FILTERWHITE90);
	IMPLEMENT_XLAT(XLAT_FILTERWHITE80);
	IMPLEMENT_XLAT(XLAT_FILTERWHITE70);

	IMPLEMENT_XLAT(XLAT_FILTERBRIGHT1);
	IMPLEMENT_XLAT(XLAT_FILTERBRIGHT2);
	IMPLEMENT_XLAT(XLAT_FILTERBRIGHT3);

	}

	if (uXlatID >= XLAT_GUILDCOLOR_BASE && uXlatID <= XLAT_GUILDCOLOR_END)
	{
		static xlat _std_guildcolors[XLAT_GUILDCOLOR_END-XLAT_GUILDCOLOR_BASE+1];
		return &_std_guildcolors[uXlatID-XLAT_GUILDCOLOR_BASE];
	}
 
	return NULL;
}

bixlat* FindStandardBiXlat(UINT uBiXlatID)
{
	extern bixlat _blend25;
	extern bixlat _blend50;
	extern bixlat _blend75;

	switch (uBiXlatID)
	{
	case BIXLAT_BLEND25:
		return &_blend25;
	case BIXLAT_BLEND50:
		return &_blend50;
	case BIXLAT_BLEND75:
		return &_blend75;
	}

	return NULL;
}

/***************************************************************************/

BYTE GetRGBLightness(PALETTEENTRY* pe)
{
#if 0
	int minv = min(min(pe->peRed, pe->peGreen), pe->peBlue);
	int maxv = max(max(pe->peRed, pe->peGreen), pe->peBlue);
	return LOBYTE((maxv + minv) / 2);
#endif
#if 0
	return LOBYTE((pe->peRed + pe->peGreen + pe->peBlue)/3);
#endif
#if 1
	return LOBYTE(max(max(pe->peRed, pe->peGreen), pe->peBlue));
#endif
}

/* CalcFilterXlat:
 * Given a palette, writes all indices  of the xlat.
 * Each index is the best fit for the given color at the original palette
 * color's lightness.  For a golden filter, you'd give gold as the color.
 */
void CalcFilterXlat(HPALETTE hPalette, xlat* pXlat, COLORREF crMix)
{
	int i;
	PALETTEENTRY pe; /* i want to be able to see it always in this function */

	if (!hPalette || !pXlat)
		return;

	for (i = 0; i < NUM_COLORS; i++)
	{
		BYTE byLightness;
		LONG l;
		GetPaletteEntries(hPalette, i, 1, &pe);

		byLightness = GetRGBLightness(&pe);

		l = ((LONG)GetRValue(crMix))*((LONG)byLightness)/256;

		pe.peRed = (BYTE)(l & 0xFF);

		l = ((LONG)GetGValue(crMix))*((LONG)byLightness)/256;
		
		pe.peGreen = (BYTE)(l & 0xFF);

		l = ((LONG)GetBValue(crMix))*((LONG)byLightness)/256;
		
		pe.peBlue = (BYTE)(l & 0xFF);

		pXlat->entry[i] = GetNearestPaletteIndex(hPalette, RGB(pe.peRed, pe.peGreen, pe.peBlue));
	}
}

/* CalcBlendXlat:
 * Given a palette, writes all indices  of the xlat.
 * Each index is the best fit for the blend of a fixed color and the palette
 * color.  For a 20% golden blend, you'd give gold as the color to mix,
 * and 80 to 20 as the ratio partsFirst to partsSecond.
 */
void CalcBlendXlat(HPALETTE hPalette, xlat* pXlat, COLORREF crMix, int partsFirst, int partsSecond)
{
	int i;

	if (!hPalette || !pXlat || partsFirst <= 0 || partsSecond <= 0)
		return;

	for (i = 0; i < NUM_COLORS; i++)
	{
		PALETTEENTRY pe;
		LONG l;
		GetPaletteEntries(hPalette, i, 1, &pe);

		l = (((LONG)pe.peRed)*partsFirst + (LONG)GetRValue(crMix)*partsSecond) /
				(partsFirst+partsSecond);
		pe.peRed = (BYTE)(l & 0xFF);

		l = (((LONG)pe.peGreen)*partsFirst + (LONG)GetGValue(crMix)*partsSecond) /
				(partsFirst+partsSecond);
		pe.peGreen = (BYTE)(l & 0xFF);

		l = (((LONG)pe.peBlue)*partsFirst + (LONG)GetBValue(crMix)*partsSecond) /
				(partsFirst+partsSecond);
		pe.peBlue = (BYTE)(l & 0xFF);

		pXlat->entry[i] = GetNearestPaletteIndex(hPalette, RGB(pe.peRed, pe.peGreen, pe.peBlue));
	}
}

#if 0
// Done by MAKEPAL which generates the pal.c sourcefile.
void CalcBlendBiXlat(HPALETTE hPalette, bixlat* pBiXlat, int partsFirst, int partsSecond)
{
	int i, j;
	PALETTEENTRY pe;

	if (!hPalette || !pBiXlat || partsFirst <= 0 || partsSecond <= 0)
		return;

	for (i = 0; i < NUM_COLORS; i++)
	{
		PALETTEENTRY pe1;
		GetPaletteEntries(hPalette, i, 1, &pe1);

		for (j = 0; j < NUM_COLORS; j++)
		{
			PALETTEENTRY pe2, pe;
			LONG l;
			GetPaletteEntries(hPalette, j, 1, &pe2);

			l = (((LONG)pe1.peRed)*partsFirst + ((LONG)pe2.peRed)*partsSecond) /
					(partsFirst+partsSecond);
			pe.peRed = (BYTE)(l & 0xFF);

			l = (((LONG)pe1.peGreen)*partsFirst + ((LONG)pe2.peGreen)*partsSecond) /
					(partsFirst+partsSecond);
			pe.peGreen = (BYTE)(l & 0xFF);

			l = (((LONG)pe1.peBlue)*partsFirst + ((LONG)pe2.peBlue)*partsSecond) /
					(partsFirst+partsSecond);
			pe.peBlue = (BYTE)(l & 0xFF);

			pBiXlat->entry[(i<<8)|(j)] = GetNearestPaletteIndex(hPalette, RGB(pe.peRed, pe.peGreen, pe.peBlue));
		}
	}
}
#endif
/***************************************************************************/
/*
 * CalcRampXlat:  Make pXlat a palette translation where the the byMask
 *   part of the index is changed from byFromRamp to byToRamp.  Other colors
 *   not matching byFromRamp are unaffected.
 */
void CalcRampXlat(HPALETTE hPalette, xlat* pXlat, BYTE byMask, BYTE byFromRamp, BYTE byToRamp, BOOL bIdentity)
{
   int i;

   if (bIdentity)
      *pXlat = *FindStandardXlat(XLAT_IDENTITY);

   if (byFromRamp == byToRamp)
      return;

   for (i = 0; i < NUM_COLORS; i++)
   {
      if ((i & byMask) == byFromRamp)
	 pXlat->entry[i] = ((i & ~byMask) | byToRamp);
   }
}
/***************************************************************************/
/*
 * CalcRampOffsetXlat:  Make pXlat a palette translation where the the byMask
 *   part of the index is changed to another ramp at a given ramp offset.
 *   Known ramps are defined in the static global ramps[].
 *   Unlike other ramp routines in here, this assumes the ramps are sixteen
 *   colors aligned to sixteens, such as 0x30-0x3F, or 0xA0-0xAF.
 */
void CalcRampOffsetXlat(HPALETTE hPalette, xlat* pXlat, int iOffset)
{
	int i, j, r;
	*pXlat = *FindStandardXlat(XLAT_IDENTITY);

	// Number of defined ramps.
	j = sizeof(ramps)/sizeof(ramps[0]);

	// If offset is negative, add enough multiples of J to make it positive.
	if (iOffset < 0)
		iOffset += (1-iOffset/j)*j;

	// For each ramp, for each element in the ramp,
	// change the high nibble to the offset ramp's high nibble.
	//
	for (r = 0; r < j; r++)
	{
		for (i = ramps[r]; i < (ramps[r]+0x10); i++)
		{
			pXlat->entry[i] = ((i & 0x0F) | ramps[(r+iOffset+j)%j]);
		}
	}
}
/***************************************************************************/
/*
 * CalcHalfRampXlat:  Make pXlat a palette translation where the the byMask
 *   part of the index is changed from byFromRamp to byToRamp.  Other colors
 *   not matching byFromRamp are unaffected.
 *   The index is halved to translate to a half ramp; byToRampOffset is added
 *   to the result to move the half-ramp inside its enclosing ramp.
 */
void CalcHalfRampXlat(HPALETTE hPalette, xlat* pXlat, BYTE byMask, BYTE byFromRamp, BYTE byToRamp, BYTE byToRampOffset, BOOL bIdentity)
{
   int i;

   if (bIdentity)
      *pXlat = *FindStandardXlat(XLAT_IDENTITY);
   
   for (i = 0; i < NUM_COLORS; i++)
   {
      if ((i & byMask) == byFromRamp)
	 pXlat->entry[i] = (((i & ~byMask) / 2) | byToRamp) + byToRampOffset;
   }
}
/***************************************************************************/
/*
 * CalcLightXlat:  Change pXlat to be at the given light level.
 */
void CalcLightXlat(HPALETTE hPalette, xlat* pXlat, BYTE byMask, BYTE byFromRamp, BYTE light)
{
   int i;
   BYTE *palette;

   palette = light_palettes[light];
   for (i = 0; i < NUM_COLORS; i++)
   {
      if ((i & byMask) == byFromRamp)
	 pXlat->entry[i] = palette[pXlat->entry[i]];
   }
   
}
/***************************************************************************/
/*
 * CalcRampMoveXlat:  Make pXlat a translation that moves the given ramp
 *   (whose byMask matches byFromRamp) to the given indexes.
 */
void CalcRampMoveXlat(HPALETTE hPalette, xlat* pXlat, BYTE byMask, BYTE byFromRamp, BYTE *indexes, BOOL bIdentity)
{
   int i;
   int count;
   
   if (bIdentity)
      *pXlat = *FindStandardXlat(XLAT_IDENTITY);
   
   count = 0;
   for (i = 0; i < NUM_COLORS; i++)
   {
      if ((i & byMask) == byFromRamp)
      {
	 pXlat->entry[i] = indexes[count];
	 count++;
      }
   }
}
/***************************************************************************/
void InitStandardXlats(HPALETTE hPalette)
{
	int i, j, nRamps;
	xlat* pIdentity = FindStandardXlat(XLAT_IDENTITY);

	xlat* pXlat = pIdentity;
	for (i = 0; i < NUM_COLORS; i++)
		pXlat->entry[i] = i;

	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND10RED),   RGB(255, 0, 0), 90, 10);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND20RED),   RGB(255, 0, 0), 80, 20);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND30RED),   RGB(255, 0, 0), 70, 30);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND40RED),   RGB(255, 0, 0), 60, 40);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND50RED),   RGB(255, 0, 0), 50, 50);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND60RED),   RGB(255, 0, 0), 40, 60);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND70RED),   RGB(255, 0, 0), 30, 70);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND80RED),   RGB(255, 0, 0), 20, 80);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND90RED),   RGB(255, 0, 0), 10, 90);
	//CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND100RED),  RGB(255, 0, 0), 0, 100);
	pXlat = FindStandardXlat(XLAT_BLEND100RED);
	for (i = 0; i < NUM_COLORS; i++)
		pXlat->entry[i] = 0x10;

	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND10WHITE),   RGB(255, 255, 255), 90, 10);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND20WHITE),   RGB(255, 255, 255), 80, 20);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND30WHITE),   RGB(255, 255, 255), 70, 30);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND40WHITE),   RGB(255, 255, 255), 60, 40);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND50WHITE),   RGB(255, 255, 255), 50, 50);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND60WHITE),   RGB(255, 255, 255), 40, 60);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND70WHITE),   RGB(255, 255, 255), 30, 70);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND80WHITE),   RGB(255, 255, 255), 20, 80);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND90WHITE),   RGB(255, 255, 255), 10, 90);
	//CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND100WHITE),   RGB(255, 255, 255), 0, 100);
	pXlat = FindStandardXlat(XLAT_BLEND100WHITE);
	for (i = 0; i < NUM_COLORS; i++)
		pXlat->entry[i] = 255;

	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND25YELLOW),  RGB(255, 255, 0), 75, 25);

	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND25RED),   RGB(255, 0, 0), 75, 25);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND25GREEN), RGB(0, 255, 0), 75, 25);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND25BLUE),  RGB(0, 0, 255), 75, 25);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND50RED),   RGB(255, 0, 0), 50, 50);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND50GREEN), RGB(0, 255, 0), 50, 50);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND50BLUE),  RGB(0, 0, 255), 50, 50);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND75RED),   RGB(255, 0, 0), 25, 75);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND75GREEN), RGB(0, 255, 0), 25, 75);
	CalcBlendXlat(hPalette, FindStandardXlat(XLAT_BLEND75BLUE),  RGB(0, 0, 255), 25, 75);

	CalcFilterXlat(hPalette, FindStandardXlat(XLAT_FILTERRED),   RGB(255, 0, 0));
	CalcFilterXlat(hPalette, FindStandardXlat(XLAT_FILTERGREEN), RGB(0, 255, 0));
	CalcFilterXlat(hPalette, FindStandardXlat(XLAT_FILTERBLUE),  RGB(0, 0, 255));

	CalcFilterXlat(hPalette, FindStandardXlat(XLAT_FILTERWHITE90), RGB(240, 240, 240));
	CalcFilterXlat(hPalette, FindStandardXlat(XLAT_FILTERWHITE80), RGB(220, 220, 220));
	CalcFilterXlat(hPalette, FindStandardXlat(XLAT_FILTERWHITE70), RGB(200, 200, 200));

	CalcFilterXlat(hPalette, FindStandardXlat(XLAT_FILTERBRIGHT1), RGB(253, 253, 253));
	CalcFilterXlat(hPalette, FindStandardXlat(XLAT_FILTERBRIGHT2), RGB(250, 250, 250));
	CalcFilterXlat(hPalette, FindStandardXlat(XLAT_FILTERBRIGHT3), RGB(245, 245, 245));

	CalcRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTORED), 0xF0, 0xD0, 0x10, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOORANGE), 0xF0, 0xD0, 0x50, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTODGREEN), 0xF0, 0xD0, 0x60, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOBGREEN), 0xF0, 0xD0, 0x70, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOSKY), 0xF0, 0xD0, 0x80, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTODBLUE), 0xF0, 0xD0, 0x90, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOPURPLE), 0xF0, 0xD0, 0xA0, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOGOLD), 0xF0, 0xD0, 0xC0, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOBBLUE), 0xF0, 0xD0, 0xE0, TRUE);

	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOLRED), 0xF0, 0xD0, 0x10, 0, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOLORANGE), 0xF0, 0xD0, 0x50, 0, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOLGREEN), 0xF0, 0xD0, 0x60, 0, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOLBGREEN), 0xF0, 0xD0, 0x70, 0, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOLSKY), 0xF0, 0xD0, 0x80, 0, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOLBBLUE), 0xF0, 0xD0, 0x90, 0, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOLPURPLE), 0xF0, 0xD0, 0xA0, 0, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOLGOLD), 0xF0, 0xD0, 0xC0, 0, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOLBLUE), 0xF0, 0xD0, 0xE0, 0, TRUE);

	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOKRED), 0xF0, 0xD0, 0x10, 8, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOKORANGE), 0xF0, 0xD0, 0x50, 8, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOKGREEN), 0xF0, 0xD0, 0x60, 8, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOKBGREEN), 0xF0, 0xD0, 0x70, 8, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOKSKY), 0xF0, 0xD0, 0x80, 8, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOKBBLUE), 0xF0, 0xD0, 0x90, 8, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOKPURPLE), 0xF0, 0xD0, 0xA0, 8, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOKGOLD), 0xF0, 0xD0, 0xC0, 8, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOKBLUE), 0xF0, 0xD0, 0xE0, 8, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOKGRAY), 0xF0, 0xD0, 0xD0, 8, TRUE);

	// Red-to-Black has zero light level.
	pXlat = FindStandardXlat(XLAT_REDTOBLACK);
	for (i = 0; i < NUM_COLORS; i++)
		pXlat->entry[i] = i;
	CalcLightXlat(hPalette, pXlat, 0xF0, 0x10, 0);

	// Blue-to-Black has zero light level.
	pXlat = FindStandardXlat(XLAT_BLUETOBLACK);
	for (i = 0; i < NUM_COLORS; i++)
		pXlat->entry[i] = i;
	CalcLightXlat(hPalette, pXlat, 0xF0, 0x90, 0);

	// Purple-to-Black has zero light level.
	pXlat = FindStandardXlat(XLAT_PURPLETOBLACK);
	for (i = 0; i < NUM_COLORS; i++)
		pXlat->entry[i] = i;
	CalcLightXlat(hPalette, pXlat, 0xF0, 0xA0, 0);

	// Gray-to-Black has lowered light level.
	pXlat = FindStandardXlat(XLAT_GRAYTOBLACK);
	for (i = 0; i < NUM_COLORS; i++)
		pXlat->entry[i] = i;
	CalcLightXlat(hPalette, pXlat, 0xF0, 0xD0, LIGHT_LEVELS / 6);

	CalcRampMoveXlat(hPalette, FindStandardXlat(XLAT_GRAYTOOLDHAIR1), 0xF0, 0xD0, oldhair1_indexes, TRUE);
	CalcRampMoveXlat(hPalette, FindStandardXlat(XLAT_GRAYTOOLDHAIR2), 0xF0, 0xD0, oldhair2_indexes, TRUE);
	CalcRampMoveXlat(hPalette, FindStandardXlat(XLAT_GRAYTOOLDHAIR3), 0xF0, 0xD0, oldhair3_indexes, TRUE);
	CalcRampMoveXlat(hPalette, FindStandardXlat(XLAT_GRAYTOPLATBLOND), 0xF0, 0xD0, platblond_indexes, TRUE);

    // Mostly obsolete.
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOSKIN1), 0xF0, 0xD0, 0x20, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOSKIN2), 0xF0, 0xD0, 0x30, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOSKIN3), 0xF0, 0xD0, 0x30, 5, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOSKIN4), 0xF0, 0xD0, 0x40, TRUE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_GRAYTOSKIN5), 0xF0, 0xD0, 0x40, 8, TRUE);

    // Skin to healthy shades.
	CalcRampMoveXlat(hPalette, FindStandardXlat(XLAT_DBLUETOSKIN1), 0xF0, 0x90, skin1_indexes, TRUE);
	CalcRampMoveXlat(hPalette, FindStandardXlat(XLAT_DBLUETOSKIN2), 0xF0, 0x90, skin2_indexes, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_DBLUETOSKIN3), 0xF0, 0x90, 0x30, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_DBLUETOSKIN4), 0xF0, 0x90, 0x40, TRUE);

    // Skin to sickly effects.
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_DBLUETOGRAY), 0xF0, 0x90, 0xD0, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_DBLUETOLBLUE), 0xF0, 0x90, 0x80, TRUE);
	CalcRampMoveXlat(hPalette, FindStandardXlat(XLAT_DBLUETOSICKGREEN), 0xF0, 0x90, green_skin_indexes, TRUE);
	CalcRampMoveXlat(hPalette, FindStandardXlat(XLAT_DBLUETOSICKYELLOW), 0xF0, 0x90, yellow_skin_indexes, TRUE);

    // Custom forked color:  grey->red while blue->grey.
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_DBLUETOASHEN), 0xF0, 0x90, 0xD0, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_DBLUETOASHEN), 0xF0, 0xD0, 0x10, FALSE);

    // Custom forked color:  red->dgreen while blue->skin1.5,2,3.
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_REDTODGREEN1), 0xF0, 0x10, 0x70, 8, TRUE);
	CalcRampXlat(hPalette,     FindStandardXlat(XLAT_REDTODGREEN1), 0xF0, 0x90, 0x20, FALSE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_REDTODGREEN2), 0xF0, 0x10, 0x70, 8, TRUE);
	CalcRampXlat(hPalette,     FindStandardXlat(XLAT_REDTODGREEN2), 0xF0, 0x90, 0x30, FALSE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_REDTODGREEN3), 0xF0, 0x10, 0x70, 8, TRUE);
	CalcRampXlat(hPalette,     FindStandardXlat(XLAT_REDTODGREEN3), 0xF0, 0x90, 0x40, FALSE);

    // Custom forked color:  red->black while blue->skin1.5,3,4.
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_REDTOBLACK1), 0xF0, 0x10, 0xD0, 8, TRUE);
	CalcRampXlat(hPalette,     FindStandardXlat(XLAT_REDTOBLACK1), 0xF0, 0x90, 0x20, FALSE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_REDTOBLACK2), 0xF0, 0x10, 0xD0, 8, TRUE);
	CalcRampXlat(hPalette,     FindStandardXlat(XLAT_REDTOBLACK2), 0xF0, 0x90, 0x30, FALSE);
	CalcHalfRampXlat(hPalette, FindStandardXlat(XLAT_REDTOBLACK3), 0xF0, 0x10, 0xD0, 8, TRUE);
	CalcRampXlat(hPalette,     FindStandardXlat(XLAT_REDTOBLACK3), 0xF0, 0x90, 0x40, FALSE);

   // Custom colors: red->pitch black while blue->skin1.5,3,4
	pXlat = FindStandardXlat(XLAT_REDTODKBLACK1);
	for (i = 0; i < NUM_COLORS; i++)
		pXlat->entry[i] = i;
	CalcLightXlat(hPalette, pXlat, 0xF0, 0x10, 0);
	CalcRampXlat(hPalette,  pXlat, 0xF0, 0x90, 0x20, FALSE);
	pXlat = FindStandardXlat(XLAT_REDTODKBLACK2);
	for (i = 0; i < NUM_COLORS; i++)
		pXlat->entry[i] = i;
	CalcLightXlat(hPalette, pXlat, 0xF0, 0x10, 0);
	CalcRampXlat(hPalette,  pXlat, 0xF0, 0x90, 0x30, FALSE);
	pXlat = FindStandardXlat(XLAT_REDTODKBLACK3);
	for (i = 0; i < NUM_COLORS; i++)
		pXlat->entry[i] = i;
	CalcLightXlat(hPalette, pXlat, 0xF0, 0x10, 0);
	CalcRampXlat(hPalette,  pXlat, 0xF0, 0x90, 0x40, FALSE);

   // Red->black while blue->white.
	pXlat = FindStandardXlat(XLAT_REDBLK_BLWHT);
	for (i = 0; i < NUM_COLORS; i++)
		pXlat->entry[i] = i;
	CalcLightXlat(hPalette,pXlat, 0xF0, 0x10, 0);
	CalcRampXlat(hPalette, pXlat, 0xF0, 0x90, 0xD0, FALSE);

   // Blue->black while red->white.
	pXlat = FindStandardXlat(XLAT_BLBLK_REDWHT);
	for (i = 0; i < NUM_COLORS; i++)
		pXlat->entry[i] = i;
	CalcLightXlat(hPalette,pXlat, 0xF0, 0x90, 0);
	CalcRampXlat(hPalette, pXlat, 0xF0, 0x10, 0xD0, FALSE);

	CalcRampXlat(hPalette, FindStandardXlat(XLAT_PURPLETOLBLUE), 0xF0, 0xA0, 0x80, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_PURPLETOBRED), 0xF0, 0xA0, 0x10, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_PURPLETOGREEN), 0xF0, 0xA0, 0x70, TRUE);
	CalcRampXlat(hPalette, FindStandardXlat(XLAT_PURPLETOYELLOW), 0xF0, 0xA0, 0x50, TRUE);

//  Too slow. Done by makepal.exe which writes pal.c now.
//	CalcBlendBiXlat(hPalette, FindStandardBiXlat(BIXLAT_BLEND50), 50, 50);

	// Each of these scrolls the ramps up or down.
	CalcRampOffsetXlat(hPalette, FindStandardXlat(XLAT_RAMPUP1), +1);
	CalcRampOffsetXlat(hPalette, FindStandardXlat(XLAT_RAMPUP2), +2);
	CalcRampOffsetXlat(hPalette, FindStandardXlat(XLAT_RAMPDOWN2), -2);
	CalcRampOffsetXlat(hPalette, FindStandardXlat(XLAT_RAMPDOWN1), -1);

	// Do the guild shield colors.
	nRamps = (sizeof(ramps)/sizeof(ramps[0]));
	for (i = 0; i < nRamps; i++)
	{
		for (j = 0; j < nRamps; j++)
		{
			pXlat = FindStandardXlat(XLAT_GUILDCOLOR_BASE + i*nRamps + j);
			*pXlat = *pIdentity;

			// swap red to ramp i
			CalcRampXlat(hPalette, pXlat, 0xF0, 0x10/*red*/, ramps[i], FALSE);

			// swap blue to ramp j
			CalcRampXlat(hPalette, pXlat, 0xF0, 0x90/*blu*/, ramps[j], FALSE);
		}
	}
}

/***************************************************************************/

void XlatDib(BYTE* pabyBits, int width, int height, xlat* pXlat)
	//PERFORMANCE
{
	int i;

	if (!pabyBits || !pXlat)
		return;

	for (i = 0; i < height; i++)
	{
		int row = i;
		//BUGBUG: if flipped dib, row = height - 1 - i;
		register BYTE* pbyPixel = pabyBits + row*width;
		BYTE* pbyEnd = pbyPixel + width;
		while (pbyPixel < pbyEnd)
		{
			*pbyPixel = fastXLAT(*pbyPixel, pXlat);
			pbyPixel++;
		}
	}
}
