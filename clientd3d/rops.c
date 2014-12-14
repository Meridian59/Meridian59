// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * rops.c:  special bitmap Raster OPeration effects
 */

#include "client.h"

/***************************************************************************/

void SandDib(BYTE* pabyBits, int width, int height, int drops)
	//PERFORMANCE
{
	int allrun, allblow, alllength;

	if (!pabyBits)
		return;

	// compute the droplet length
	alllength = 20;

	// compute the droplet run and blow (less run is more slant)
	allblow = -1;
	if (50 > rand() % 100)
		allblow = 1;
	allrun = 1 + (rand() % 6);

	while (drops--)
	{
		int run = rand() % allrun;
		int length = alllength;
		xlat* pXlat;

		// choose a random start point
		int x = rand() % width;
		int y = rand() % (height-(length/2));

		// choose a random droplet type
		pXlat = FindStandardXlat(XLAT_BLEND25YELLOW);

		// while more length and we're still inside dib,
		while (length && x >= 0 && x < width && y < height)
		{
			// draw the droplet
			BYTE* pbyPixel = pabyBits + x + y*width;
			*pbyPixel = fastXLAT(*pbyPixel, pXlat);

			// move down the length of the droplet
			run--;
			y++;
			if (run <= 0)
			{
				run = allrun;
				x += allblow;
			}
			length--;
		}
	}
}

/***************************************************************************/
void RainDib(BYTE* pabyBits, int width, int height, int drops, int heading, int windheading, int windstrength, int torch)
	//PERFORMANCE
{
	int allrun, allblow, alllength;

	if (!pabyBits)
		return;

	// compute the droplet length
	alllength = 8;

	// compute the droplet run and blow (less run is more slant)
	{
		int delta;
		if (windheading >= (heading+NUMDEGREES/2))
			windheading -= NUMDEGREES;

		if ((windheading > heading) && (windheading < (heading+NUMDEGREES/4)))
		{
			// over my shoulder from left to right
			allblow = 1;
			delta = windheading - heading;
		}
		else if ((windheading < heading) && (windheading > (heading-NUMDEGREES/4)))
		{
			// over my shoulder from right to left
			allblow = -1;
			delta = heading - windheading;
		}
		else if ((windheading > heading+NUMDEGREES/4))
		{
			// in my face from left to right
			allblow = 1;
			delta = heading - (windheading+NUMDEGREES/2);
		}
		else
		{
			// in my face from right to left
			allblow = -1;
			delta = (windheading+NUMDEGREES/2) - heading;
		}
		allrun = 3;
	}

	while (drops--)
	{
		int run = rand() % allrun;
		int length = alllength;
		xlat* pXlat;

		// choose a random start point
		int x = rand() % width;
		int y = rand() % (height-(length/2));

		// choose a random droplet type
		pXlat = (rand() & 0x10)?
			FindStandardXlat(XLAT_BLEND25BLUE) :
			FindStandardXlat(XLAT_BLEND50BLUE);

		// if we have local light source, small chance of doing an XLAT_BLEND75YELLOW
		if (torch && ((rand() % 100) < 10))
			pXlat = FindStandardXlat(XLAT_BLEND25YELLOW);

		// while more length and we're still inside dib,
		while (length && x >= 0 && x < width && y < height)
		{
			// draw the droplet
			BYTE* pbyPixel = pabyBits + x + y*width;
			*pbyPixel = fastXLAT(*pbyPixel, pXlat);

			// move down the length of the droplet
			run--;
			y++;
			if (run <= 0)
			{
				run = allrun;
				x += allblow;
			}
			length--;
		}
	}
}

/***************************************************************************/

void SnowDib(BYTE* pabyBits, int width, int height, int drops, int heading, int windheading, int windstrength, int torch)
	//PERFORMANCE
{
	int allrun, allblow, alllength, alldropwidth;

	if (!pabyBits)
		return;

	// compute the droplet length and width
	alllength = 3;
	alldropwidth = 2;

	// compute the droplet run and blow (less run is more slant)
	{
		int delta;
		if (windheading >= (heading+NUMDEGREES/2))
			windheading -= NUMDEGREES;

		if ((windheading > heading) && (windheading < (heading+NUMDEGREES/4)))
		{
			// over my shoulder from left to right
			allblow = 1;
			delta = windheading - heading;
		}
		else if ((windheading < heading) && (windheading > (heading-NUMDEGREES/4)))
		{
			// over my shoulder from right to left
			allblow = -1;
			delta = heading - windheading;
		}
		else if ((windheading > heading+NUMDEGREES/4))
		{
			// in my face from left to right
			allblow = 1;
			delta = heading - (windheading+NUMDEGREES/2);
		}
		else
		{
			// in my face from right to left
			allblow = -1;
			delta = (windheading+NUMDEGREES/2) - heading;
		}

		allrun = 5;
	}

	while (drops--)
	{
		int run = rand() % allrun;
		int length = alllength;
		int dropwidth = alldropwidth;
		xlat* pXlat;

		// choose a random start point
		int x = rand() % (width - (dropwidth/2));
		int y = rand() % (height-(length/2));

		// white snow
		pXlat = FindStandardXlat(XLAT_BLEND100WHITE);

		// while more length and we're still inside dib,
		while (length && x >= 0 && x < width && y < height)
		{
			dropwidth = alldropwidth;
			// draw the droplet
			BYTE* pbyPixel = pabyBits + x + y*width;
			*pbyPixel = fastXLAT(*pbyPixel, pXlat);
			while (dropwidth)
			{
				BYTE* pbyPixel = pabyBits + (x+dropwidth) + y*width;
				*pbyPixel = fastXLAT(*pbyPixel, pXlat);
				dropwidth--;
				}

			// move down the length of the droplet
			run--;
			y++;
			if (run <= 0)
			{
				run = allrun;
				x += allblow;
			}
			length--;
		}
	}
}

/***************************************************************************/

static int _blur_offset[4] = { 1, -1, MAXX, -MAXX };

void BlurDib(BYTE* pabyBits, int width, int height, int amount)
	//PERFORMANCE
{
	int i, j;
	register BYTE* pbyPixel;
	BYTE* pbyEnd;
	BYTE byNew;

	if (!pabyBits)
		return;

	if (amount < 1)
		return;
	if (amount > 5)
		amount = 5;
	j = 2 + 3*amount;

	for (i = 1; i < height-1; i++)
	{
		int row = i;
		//BUGBUG: if flipped dib, row = height - 1 - i;
		pbyPixel = pabyBits + row*width;
		pbyEnd = pbyPixel + width;
		pbyPixel += j;
		while (pbyPixel < pbyEnd-j)
		{
			byNew = *(pbyPixel+((((DWORD)(BYTE*)pbyPixel+i) & 1)? -j : j));
			//REVIEW: could do: pBiXlat = FindStandardBiXlat(BIXLAT_BLEND50);
			//REVIEW: could do: *pbyPixel = fastBIXLAT(*pbyPixel, byNew, pBiXlat); // blend the trashed pixels softly
			*pbyPixel = byNew;
			pbyPixel += 3;
		}
	}
}

static int _shiftbits[] = {  0, 0, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 0, 0, -1, -1, -1, -2, -2, -2, -2, -2, -1, -1, -1, };
static int _table = (sizeof(_shiftbits)/sizeof(_shiftbits[0]));

void WaverDib(BYTE* pabyBits, int width, int height, int offset)
	//PERFORMANCE
{
	int i, j;
	register BYTE* pbyPixel;

	if (!pabyBits)
		return;

	if (offset < 0)
		offset = 0;
	offset = offset % _table;

	for (i = 1; i < height-1; i++)
	{
		int row = i;
		//BUGBUG: if flipped dib, row = height - 1 - i;
		pbyPixel = pabyBits + row*width;

		j = _shiftbits[offset++];
		offset = offset % _table;
		if (!j)
			continue;
		else if (j < 0)
			memmove(pbyPixel, pbyPixel - j, width + j);
		else
			memmove(pbyPixel + j, pbyPixel, width - j);
	}
}

/***************************************************************************/

