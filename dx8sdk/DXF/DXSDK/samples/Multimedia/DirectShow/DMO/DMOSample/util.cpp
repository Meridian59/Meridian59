//------------------------------------------------------------------------------
// File: Util.cpp
//
// Desc: DirectShow sample code - implementation of utility routines.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#include "dmo.h"
#include "amvideo.h"
#include "util.h"

//  Helper to get the important info out of VIDEOINFOHEADER
void GetVideoInfoParameters(
	const VIDEOINFOHEADER *pvih,
	PBYTE pbData,
	DWORD *pdwWidth,
	DWORD *pdwHeight,
	LONG *plStrideInBytes,     //  Add this on to a pointer to a row to get the new row down
	BYTE **ppbTop               //  Return position of first byte of top row of pixels
	)
{
        LONG lStride;

	//  biWidth is the stride in pixels for 'normal formats'
	//  Expand to bytes and round up to a multiple of 4
        if (pvih->bmiHeader.biBitCount != 0 &&
            0 == (7 & pvih->bmiHeader.biBitCount)) {
            lStride = (pvih->bmiHeader.biWidth * (pvih->bmiHeader.biBitCount / 8) + 3) & ~3;
        } else {
            lStride = pvih->bmiHeader.biWidth;
        }

	//  If rcTarget is empty it means 'the whole image'
	if (IsRectEmpty(&pvih->rcTarget)) {
		*pdwWidth = (DWORD)pvih->bmiHeader.biWidth;
		*pdwHeight = (DWORD)(abs(pvih->bmiHeader.biHeight));
		if (pvih->bmiHeader.biHeight < 0) {
			*plStrideInBytes = lStride;
			*ppbTop           = pbData;
		} else {
            *plStrideInBytes = -lStride;
		    *ppbTop = pbData + lStride * (*pdwHeight - 1);
		}
	} else {
		*pdwWidth = (DWORD)(pvih->rcTarget.right - pvih->rcTarget.left);
		*pdwHeight = (DWORD)(pvih->rcTarget.bottom - pvih->rcTarget.top);
        if (pvih->bmiHeader.biHeight < 0) {
			*plStrideInBytes = lStride;
			*ppbTop = pbData +
				     lStride * pvih->rcTarget.top +
					 (pvih->bmiHeader.biBitCount * pvih->rcTarget.left) / 8;
		} else {
			*plStrideInBytes = -lStride;
			*ppbTop = pbData +
				     lStride * (pvih->bmiHeader.biHeight - pvih->rcTarget.top - 1) +
					 (pvih->bmiHeader.biBitCount * pvih->rcTarget.left) / 8;
		}
	}
}

//  stuff to draw string
PBYTE TextBitmap(LPCTSTR lpsz, SIZE *pSize)
{
	HDC hdc = CreateCompatibleDC(NULL);
	if (NULL == hdc) {
		return NULL;
	}
	if (!GetTextExtentPoint32(hdc, lpsz, lstrlen(lpsz), pSize))
	{
		return NULL;
	}
	
	// Create our bitmap
	struct {
		BITMAPINFOHEADER bmiHeader;
		DWORD rgbEntries[2];
	} bmi =
	{
		{
	        sizeof(BITMAPINFOHEADER),
			pSize->cx,
			pSize->cy,
			1,	
			1,	
			BI_RGB,
			0,
			0,
			0
		},
		{
			0x00000000,
		    0xFFFFFFFF
		}
	};

	HBITMAP hbm = CreateDIBitmap(hdc, &bmi.bmiHeader, 0, NULL, NULL, 0);
	if (NULL == hbm) {
		DeleteDC(hdc);
		return NULL;
	}

	HGDIOBJ hobj = SelectObject(hdc, hbm);
	if (NULL == hobj) {
		DeleteObject(hbm);
		DeleteDC(hdc);
		return NULL;
	}
	PBYTE pbReturn = NULL;
	BOOL bResult = ExtTextOut(hdc, 0, 0, ETO_OPAQUE | ETO_CLIPPED, NULL, lpsz,
		                      lstrlen(lpsz), NULL);
	SelectObject(hdc, hobj);
	LONG lLines;
	if (bResult) {
		LONG lWidthInBytes = ((pSize->cx + 31) >> 3) & ~3;
        pbReturn = new BYTE[lWidthInBytes * pSize->cy];
		if (pbReturn) {
			ZeroMemory(pbReturn, lWidthInBytes * pSize->cy);
			lLines = GetDIBits(hdc, hbm, 0, pSize->cy, (PVOID)pbReturn, (BITMAPINFO *)&bmi, DIB_RGB_COLORS);
		}
	}
	DeleteObject(hbm);
	DeleteDC(hdc);
	return pbReturn;
}

//  Helper to fill memory
void OurFillRect(const VIDEOINFOHEADER *pvih, PBYTE pbData, WORD wVal)
{
	DWORD dwWidth, dwHeight;
	LONG  lStrideInBytes;
	PBYTE pbTop;
	GetVideoInfoParameters(pvih, pbData, &dwWidth, &dwHeight, &lStrideInBytes, &pbTop);

    PBYTE pbPixels = pbTop;

	//  For just filling we don't care which way up the bitmap is - we just start at pbData
    for (DWORD dwCount = 0; dwCount < dwHeight; dwCount++) {
		WORD *pWord = (WORD *)pbPixels;
		for (DWORD dwPixel = 0; dwPixel < dwWidth; dwPixel++) {
		    pWord[dwPixel] = wVal;	
		}
		//  biWidth is the stride
        pbPixels += lStrideInBytes;
	}
}

//  Helper to get some text into memory (note we're ignoring errors!)
void DrawOurText(const VIDEOINFOHEADER * pvih, PBYTE pbData, LPCTSTR szBuffer)
{

	//  Copy the data into our real buffer (top lhs)
	DWORD dwWidthTarget, dwHeightTarget;
	LONG lStrideInBytesTarget;
	PBYTE pbTarget;

	SIZE size;

	//  Get a bit map representing our bits
	PBYTE pbBits = TextBitmap(szBuffer, &size);
	if (NULL == pbBits) {
		return;
	}
    GetVideoInfoParameters(pvih, pbData, &dwWidthTarget, &dwHeightTarget, &lStrideInBytesTarget, &pbTarget);

	//  Now copy the data from the DIB section (which is bottom up)
	//  but first check if it's too big
	if (dwWidthTarget >= (DWORD)size.cx && dwHeightTarget > (DWORD)size.cy && size.cx > 0 && size.cy > 0) {
		// PBYTE pbSource = (PBYTE)pbBits;
		DWORD dwSourceStride = ((size.cx + 31) >> 3) & ~3;

		for (DWORD dwY = 0; dwY < (DWORD)size.cy; dwY++) {
			WORD *pwTarget = (WORD *)pbTarget;
			PBYTE pbSource = pbBits + dwSourceStride * ((DWORD)size.cy - dwY - 1);
			for (DWORD dwX = 0; dwX < (DWORD)size.cx; dwX++) {
				if ( !((0x80 >> (dwX & 7)) & pbSource[dwX >> 3]) ) {
					pwTarget[dwX] = 0x0000; // Black
				}
			}
			pbTarget += lStrideInBytesTarget;
		}
	}
	delete [] pbBits;
}

//  Helper - compares media types - ignoring the advisory fields
bool TypesMatch(const DMO_MEDIA_TYPE *pmt1, const DMO_MEDIA_TYPE *pmt2)
{
	if (pmt1->majortype   == pmt2->majortype &&
		pmt1->subtype     == pmt2->subtype &&
		pmt1->lSampleSize == pmt2->lSampleSize &&
		pmt1->formattype  == pmt2->formattype &&
		pmt1->cbFormat    == pmt2->cbFormat &&
		0 == memcmp(pmt1->pbFormat, pmt2->pbFormat, pmt1->cbFormat)) {
		return true;
	} else {
		return false;
	}
}
