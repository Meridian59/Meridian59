// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

// Variables

extern Color base_palette[];
extern PALETTEENTRY gPalette[256];
extern int d3dRenderTextureThreshold;
extern d3d_driver_profile gD3DDriverProfile;

// Interfaces

void D3DRenderPaletteSet(UINT xlatID0, UINT xlatID1, unsigned int flags);

// Implementations

/**
Set the current pallete in use
*/
void D3DRenderPaletteSet(UINT xlatID0, UINT xlatID1, unsigned int flags)
{
	xlat* pXLat0, * pXLat1;
	Color* pPalette;
	int		i;
	unsigned int	effect;

	pXLat0 = FindStandardXlat(xlatID0);
	pXLat1 = FindStandardXlat(xlatID1);

	pPalette = base_palette;
	effect = GetDrawingEffect(flags);

	switch (effect)
	{
	case OF_DRAW_PLAIN:
	case OF_DOUBLETRANS:
	case OF_BLACK:
		for (i = 0; i < 256; i++)
		{
			gPalette[i].peRed = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].red;
			gPalette[i].peGreen = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].green;
			gPalette[i].peBlue = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].blue;

			if (i == 254)
				gPalette[i].peFlags = 0;
			else
				gPalette[i].peFlags = 255;
		}
		break;

	case OF_TRANSLUCENT25:
	case OF_TRANSLUCENT50:
	case OF_TRANSLUCENT75:
		for (i = 0; i < 256; i++)
		{
			gPalette[i].peRed = pPalette[fastXLAT(i, pXLat0)].red;
			gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat0)].green;
			gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat0)].blue;

			if (i == 254)
				gPalette[i].peFlags = 0;
			else
				gPalette[i].peFlags = 255;
		}
		break;

	case OF_INVISIBLE:
		break;

	case OF_TRANSLATE:
		break;

	case OF_DITHERINVIS:
		for (i = 0; i < 256; i++)
		{
			pXLat0 = FindStandardXlat(xlatID0);
			pXLat1 = FindStandardXlat(xlatID1);
			gPalette[i].peRed = pPalette[fastXLAT(i, pXLat1)].red;
			gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat1)].green;
			gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat1)].blue;

			if (i == 254)
				gPalette[i].peFlags = 0;
			else
				gPalette[i].peFlags = 255;
		}
		break;

	case OF_DITHERTRANS:
		if ((0 == xlatID1) || (xlatID0 == xlatID1))
		{
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(i, pXLat0)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat0)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat0)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		}
		else
		{
			for (i = 0; i < 256; i++)
			{
				gPalette[i].peRed = pPalette[fastXLAT(i, pXLat1)].red;
				gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat1)].green;
				gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat1)].blue;

				if (i == 254)
					gPalette[i].peFlags = 0;
				else
					gPalette[i].peFlags = 255;
			}
		}
		break;

	case OF_SECONDTRANS:
		for (i = 0; i < 256; i++)
		{
			gPalette[i].peRed = pPalette[fastXLAT(i, pXLat1)].red;
			gPalette[i].peGreen = pPalette[fastXLAT(i, pXLat1)].green;
			gPalette[i].peBlue = pPalette[fastXLAT(i, pXLat1)].blue;

			if (i == 254)
				gPalette[i].peFlags = 0;
			else
				gPalette[i].peFlags = 255;
		}
		break;

	default:
		for (i = 0; i < 256; i++)
		{
			gPalette[i].peRed = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].red;
			gPalette[i].peGreen = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].green;
			gPalette[i].peBlue = pPalette[fastXLAT(fastXLAT(i, pXLat0), pXLat1)].blue;

			if (i == 254)
				gPalette[i].peFlags = 0;
			else
				gPalette[i].peFlags = 255;
		}
		break;
	}
}

/**
/ straight texture loader for objects
*/
LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromBGF(PDIB pDib, BYTE xLat0, BYTE xLat1,
	unsigned int effect)
{
	D3DLOCKED_RECT		lockedRect;
	LPDIRECT3DTEXTURE9	pTexture = NULL;
	LPDIRECT3DTEXTURE9	pTextureFinal = NULL;
	unsigned char* pBits = NULL;
	unsigned int		w, h;
	unsigned short* pPixels16;
	int					si, sj, di, dj;
	int					k, l, newWidth, newHeight, diffWidth, diffHeight;
	int					skipValW, skipValH, pitchHalf;
	Color				lastColor;

	lastColor.red = 128;
	lastColor.green = 128;
	lastColor.blue = 128;

	D3DRenderPaletteSet(xLat0, xLat1, effect);
	skipValW = skipValH = 1;

	// convert to power of 2 texture, rounding down
	w = h = 0x80000000;

	while (!(w & pDib->width))
		w = w >> 1;

	while (!(h & pDib->height))
		h = h >> 1;

	// if either dimension is less than 256 pixels, round it back up
	if (pDib->width < d3dRenderTextureThreshold)
	{
		if (w != pDib->width)
			w <<= 1;

		newWidth = w;
		diffWidth = newWidth - pDib->width;
		skipValW = -1;
	}
	else
	{
		newWidth = w;
		diffWidth = pDib->width - newWidth;
		skipValW = 1;
	}

	if (pDib->height < d3dRenderTextureThreshold)
	{
		if (h != pDib->height)
			h <<= 1;

		newHeight = h;
		diffHeight = newHeight - pDib->height;
		skipValH = -1;
	}
	else
	{
		newHeight = h;
		diffHeight = pDib->height - newHeight;
		skipValH = 1;
	}

	k = -newWidth;
	l = -newHeight;

	pBits = DibPtr(pDib);

	if (gD3DDriverProfile.bManagedTextures)
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newWidth, newHeight, 1, 0,
			D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &pTexture, NULL);
	else
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newWidth, newHeight, 1, 0,
			D3DFMT_A1R5G5B5, D3DPOOL_SYSTEMMEM, &pTexture, NULL);

	if (pTexture == NULL)
		return NULL;

	IDirect3DTexture9_LockRect(pTexture, 0, &lockedRect, NULL, 0);

	pitchHalf = lockedRect.Pitch / 2;

	pPixels16 = (unsigned short*)lockedRect.pBits;

	for (si = 0, di = 0; di < newHeight; si++, di++)
	{
		if (diffHeight)
			if ((l += diffHeight) >= 0)
			{
				si += skipValH;
				l -= newHeight;
			}

		for (dj = 0, sj = 0; dj < newWidth; dj++, sj++)
		{
			if (diffWidth)
				if ((k += diffWidth) >= 0)
				{
					sj += skipValW;
					k -= newWidth;
				}

			// 16bit 1555 textures
			if (gPalette[pBits[si * pDib->width + sj]].peFlags != 0)
			{
				pPixels16[di * pitchHalf + dj] =
					(gPalette[pBits[si * pDib->width + sj]].peBlue >> 3) |
					((gPalette[pBits[si * pDib->width + sj]].peGreen >> 3) << 5) |
					((gPalette[pBits[si * pDib->width + sj]].peRed >> 3) << 10);
				pPixels16[di * pitchHalf + dj] |=
					gPalette[pBits[si * pDib->width + sj]].peFlags ? (1 << 15) : 0;

				lastColor.red = gPalette[pBits[si * pDib->width + sj]].peRed;
				lastColor.green = gPalette[pBits[si * pDib->width + sj]].peGreen;
				lastColor.blue = gPalette[pBits[si * pDib->width + sj]].peBlue;
			}
			else
			{
				pPixels16[di * pitchHalf + dj] =
					(lastColor.blue >> 3) |
					((lastColor.green >> 3) << 5) |
					((lastColor.red >> 3) << 10);
				pPixels16[di * pitchHalf + dj] |=
					gPalette[pBits[si * pDib->width + sj]].peFlags ? (1 << 15) : 0;
			}
		}
	}

	IDirect3DTexture9_UnlockRect(pTexture, 0);

	if (gD3DDriverProfile.bManagedTextures == FALSE)
	{
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newWidth, newHeight, 1, 0,
			D3DFMT_A1R5G5B5, D3DPOOL_DEFAULT, &pTextureFinal, NULL);

		if (pTextureFinal)
		{
			IDirect3DTexture9_AddDirtyRect(pTextureFinal, NULL);
			IDirect3DDevice9_UpdateTexture(
				gpD3DDevice, (IDirect3DBaseTexture9*)pTexture,
				(IDirect3DBaseTexture9*)pTextureFinal);
		}
		IDirect3DTexture9_Release(pTexture);

		return pTextureFinal;
	}
	else
		return pTexture;
}

/**
* texture loader that rotates for walls and such
*/
LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromBGFSwizzled(PDIB pDib, BYTE xLat0, BYTE xLat1,
	unsigned int effect)
{
	D3DLOCKED_RECT		lockedRect;
	LPDIRECT3DTEXTURE9	pTexture = NULL;
	LPDIRECT3DTEXTURE9	pTextureFinal = NULL;
	unsigned char* pBits = NULL;
	unsigned int		w, h;
	unsigned short* pPixels16;
	int					si, sj, di, dj;
	int					k, l, newWidth, newHeight, diffWidth, diffHeight;
	int					skipValW, skipValH, pitchHalf;
	Color				lastColor;

	lastColor.red = 128;
	lastColor.green = 128;
	lastColor.blue = 128;

	D3DRenderPaletteSet(xLat0, xLat1, effect);

	skipValW = skipValH = 1;

	// convert to power of 2 texture, rounding down
	w = h = 0x80000000;

	while (!(w & pDib->width))
		w = w >> 1;

	while (!(h & pDib->height))
		h = h >> 1;

	// if either dimension is less than 256 pixels, round it back up
	if (pDib->width < d3dRenderTextureThreshold)
	{
		if (w != pDib->width)
			w <<= 1;

		newWidth = w;
		diffWidth = newWidth - pDib->width;
		skipValW = -1;
	}
	else
	{
		newWidth = w;
		diffWidth = pDib->width - newWidth;
		skipValW = 1;
	}

	if (pDib->height < d3dRenderTextureThreshold)
	{
		if (h != pDib->height)
			h <<= 1;

		newHeight = h;
		diffHeight = newHeight - pDib->height;
		skipValH = -1;
	}
	else
	{
		newHeight = h;
		diffHeight = pDib->height - newHeight;
		skipValH = 1;
	}

	k = -newWidth;
	l = -newHeight;

	pBits = DibPtr(pDib);

	if (gD3DDriverProfile.bManagedTextures)
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newHeight, newWidth, 1, 0,
			D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &pTexture, NULL);
	else
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newHeight, newWidth, 1, 0,
			D3DFMT_A1R5G5B5, D3DPOOL_SYSTEMMEM, &pTexture, NULL);

	if (NULL == pTexture)
		return NULL;

	IDirect3DTexture9_LockRect(pTexture, 0, &lockedRect, NULL, 0);

	pitchHalf = lockedRect.Pitch / 2;

	pPixels16 = (unsigned short*)lockedRect.pBits;

	for (si = 0, di = 0; di < newWidth; si++, di++)
	{
		if (diffWidth)
			if ((k += diffWidth) >= 0)
			{
				si += skipValW;
				k -= newWidth;
			}

		for (dj = 0, sj = 0; dj < newHeight; dj++, sj++)
		{
			if (diffHeight)
				if ((l += diffHeight) >= 0)
				{
					sj += skipValH;
					l -= newHeight;
				}

			// 16bit 1555 textures
			if (gPalette[pBits[(sj * pDib->width) + si]].peFlags != 0)
			{
				pPixels16[di * pitchHalf + dj] =
					(gPalette[pBits[(sj * pDib->width) + si]].peBlue >> 3) |
					((gPalette[pBits[(sj * pDib->width) + si]].peGreen >> 3) << 5) |
					((gPalette[pBits[(sj * pDib->width) + si]].peRed >> 3) << 10);
				pPixels16[di * pitchHalf + dj] |=
					gPalette[pBits[(sj * pDib->width) + si]].peFlags ? (1 << 15) : 0;

				lastColor.red = gPalette[pBits[(sj * pDib->width) + si]].peRed;
				lastColor.green = gPalette[pBits[(sj * pDib->width) + si]].peGreen;
				lastColor.blue = gPalette[pBits[(sj * pDib->width) + si]].peBlue;
			}
			else
			{
				pPixels16[di * pitchHalf + dj] =
					(lastColor.blue >> 3) |
					((lastColor.green >> 3) << 5) |
					((lastColor.red >> 3) << 10);
				pPixels16[di * pitchHalf + dj] |=
					gPalette[pBits[(sj * pDib->width) + si]].peFlags ? (1 << 15) : 0;
			}
		}
	}

	IDirect3DTexture9_UnlockRect(pTexture, 0);

	if (gD3DDriverProfile.bManagedTextures == FALSE)
	{
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newHeight, newWidth, 1, 0,
			D3DFMT_A1R5G5B5, D3DPOOL_DEFAULT, &pTextureFinal, NULL);

		if (pTextureFinal)
		{
			IDirect3DTexture9_AddDirtyRect(pTextureFinal, NULL);
			IDirect3DDevice9_UpdateTexture(
				gpD3DDevice, (IDirect3DBaseTexture9*)pTexture,
				(IDirect3DBaseTexture9*)pTextureFinal);
		}
		IDirect3DTexture9_Release(pTexture);

		return pTextureFinal;
	}
	else
		return pTexture;
}

/**
* straight texture loader from resource pointer
*/
LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromResource(BYTE* ptr, int width, int height)
{
	D3DLOCKED_RECT		lockedRect;
	LPDIRECT3DTEXTURE9	pTexture = NULL;
	LPDIRECT3DTEXTURE9	pTextureFinal = NULL;
	unsigned char* pBits = NULL;
	unsigned int		w, h;
	unsigned short* pPixels16;
	int					si, sj, di, dj;
	int					k, l, newWidth, newHeight, diffWidth, diffHeight;
	int					skipValW, skipValH, pitchHalf;

	D3DRenderPaletteSet(0, 0, 0);

	skipValW = skipValH = 1;

	// convert to power of 2 texture, rounding down
	w = h = 0x80000000;

	while (!(w & width))
		w = w >> 1;

	while (!(h & height))
		h = h >> 1;

	// if either dimension is less than 256 pixels, round it back up
	if (width < d3dRenderTextureThreshold)
	{
		if (w != width)
			w <<= 1;

		newWidth = w;
		diffWidth = newWidth - width;
		skipValW = -1;
	}
	else
	{
		newWidth = w;
		diffWidth = width - newWidth;
		skipValW = 1;
	}

	if (height < d3dRenderTextureThreshold)
	{
		if (h != height)
			h <<= 1;

		newHeight = h;
		diffHeight = newHeight - height;
		skipValH = -1;
	}
	else
	{
		newHeight = h;
		diffHeight = height - newHeight;
		skipValH = 1;
	}

	k = -newWidth;
	l = -newHeight;

	pBits = ptr;

	if (gD3DDriverProfile.bManagedTextures)
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newHeight, newWidth, 1, 0,
			D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, &pTexture, NULL);
	else
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newHeight, newWidth, 1, 0,
			D3DFMT_A1R5G5B5, D3DPOOL_SYSTEMMEM, &pTexture, NULL);

	if (NULL == pTexture)
		return NULL;

	IDirect3DTexture9_LockRect(pTexture, 0, &lockedRect, NULL, 0);

	pitchHalf = lockedRect.Pitch / 2;

	pPixels16 = (unsigned short*)lockedRect.pBits;

	//	for (dj = 0, sj = 0; dj < newHeight; dj++, sj++)
	for (dj = newHeight - 1, sj = 0; dj >= 0; dj--, sj++)
	{
		if (diffHeight)
			if ((l += diffHeight) >= 0)
			{
				sj += skipValH;
				l -= newHeight;
			}

		for (si = 0, di = 0; di < newWidth; si++, di++)
		{
			if (diffWidth)
				if ((k += diffWidth) >= 0)
				{
					si += skipValW;
					k -= newWidth;
				}

			// 16bit 1555 textures
			pPixels16[dj * pitchHalf + di] =
				(gPalette[pBits[(sj * width) + si]].peBlue >> 3) |
				((gPalette[pBits[(sj * width) + si]].peGreen >> 3) << 5) |
				((gPalette[pBits[(sj * width) + si]].peRed >> 3) << 10);
			pPixels16[dj * pitchHalf + di] |=
				gPalette[pBits[(sj * width) + si]].peFlags ? (1 << 15) : 0;
		}
	}

	IDirect3DTexture9_UnlockRect(pTexture, 0);

	if (gD3DDriverProfile.bManagedTextures == FALSE)
	{
		IDirect3DDevice9_CreateTexture(gpD3DDevice, newHeight, newWidth, 1, 0,
			D3DFMT_A1R5G5B5, D3DPOOL_DEFAULT, &pTextureFinal, NULL);

		if (pTextureFinal)
		{
			IDirect3DTexture9_AddDirtyRect(pTextureFinal, NULL);
			IDirect3DDevice9_UpdateTexture(
				gpD3DDevice, (IDirect3DBaseTexture9*)pTexture,
				(IDirect3DBaseTexture9*)pTextureFinal);
		}
		IDirect3DTexture9_Release(pTexture);

		return pTextureFinal;
	}
	else
		return pTexture;
}

/**
* straight texture loader from Png file
*/
LPDIRECT3DTEXTURE9 D3DRenderTextureCreateFromPNG(char* pFilename)
{
	FILE* pFile;
	char* filename = NULL;
	png_structp	pPng = NULL;
	png_infop	pInfo = NULL;
	png_infop	pInfoEnd = NULL;
	png_bytepp   rows;

	D3DLOCKED_RECT		lockedRect;
	LPDIRECT3DTEXTURE9	pTexture = NULL;
	PALETTEENTRY* pTemp = NULL;
	unsigned char* pBits = NULL;
	unsigned int		w, h, b;
	int					pitchHalf, bytePP, stride;

	pFile = fopen(pFilename, "rb");
	if (pFile == NULL)
		return NULL;

	pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (NULL == pPng)
	{
		fclose(pFile);
		return NULL;
	}

	pInfo = png_create_info_struct(pPng);
	if (NULL == pInfo)
	{
		png_destroy_read_struct(&pPng, NULL, NULL);
		fclose(pFile);
		return NULL;
	}

	pInfoEnd = png_create_info_struct(pPng);
	if (NULL == pInfoEnd)
	{
		png_destroy_read_struct(&pPng, &pInfo, NULL);
		fclose(pFile);
		return NULL;
	}

	if (setjmp(png_jmpbuf(pPng)))
	{
		png_destroy_read_struct(&pPng, &pInfo, &pInfoEnd);
		fclose(pFile);
		return NULL;
	}

	png_init_io(pPng, pFile);
	png_read_png(pPng, pInfo, PNG_TRANSFORM_IDENTITY, NULL);
	rows = png_get_rows(pPng, pInfo);

	unsigned int image_width = png_get_image_width(pPng, pInfo);
	unsigned int image_height = png_get_image_height(pPng, pInfo);
	bytePP = png_get_bit_depth(pPng, pInfo) / 8;
	stride = image_width * bytePP - bytePP;

	{
		int	i;
		png_bytep	curRow;

		for (i = 0; i < 6; i++)
		{
			IDirect3DDevice9_CreateTexture(gpD3DDevice, image_width, image_height, 1, 0,
				D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture, NULL);

			IDirect3DTexture9_LockRect(pTexture, 0, &lockedRect, NULL, 0);

			pitchHalf = lockedRect.Pitch / 2;

			pBits = (unsigned char*)lockedRect.pBits;

			for (h = 0; h < image_height; h++)
			{
				curRow = rows[h];

				for (w = 0; w < image_width; w++)
				{
					for (b = 0; b < 4; b++)
					{
						if (b == 3)
							pBits[h * lockedRect.Pitch + w * 4 + b] = 255;
						else
							pBits[h * lockedRect.Pitch + w * 4 + (3 - b)] =
							curRow[(w * bytePP) + b];
					}
				}
			}

			IDirect3DTexture9_UnlockRect(pTexture, 0);
		}
	}

	png_destroy_read_struct(&pPng, &pInfo, &pInfoEnd);
	fclose(pFile);

	return pTexture;
}
