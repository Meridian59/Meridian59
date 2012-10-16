//------------------------------------------------------------------------------
// File: Util.h
//
// Desc: DirectShow sample code - prototypes of utility routines.
//
// Copyright (c) 1999-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


void GetVideoInfoParameters(
	const VIDEOINFOHEADER *pvih, 
	PBYTE pbData,
	DWORD *pdwWidth, 
	DWORD *pdwHeight, 
	LONG *plStrideInBytes,
	BYTE **ppbTop             
);

void OurFillRect(const VIDEOINFOHEADER *pvih, PBYTE pbData, WORD wVal);

void DrawOurText(const VIDEOINFOHEADER * pvih, PBYTE pbData, LPCTSTR szBuffer);

PBYTE TextBitmap(LPCTSTR lpsz, SIZE *pSize);

//  Helper
bool TypesMatch(const DMO_MEDIA_TYPE *pmt1, const DMO_MEDIA_TYPE *pmt2);
