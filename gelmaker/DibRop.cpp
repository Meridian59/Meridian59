// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// DibRop.cpp
//
//////////
//
// Copyright (C) Ed Halley.
//
// This unpublished source code contains trade secrets and
// copyrighted materials which are the property of Ed Halley.
//
// Unauthorized use, copying or distribution is a violation of
// U.S. and international laws and is strictly prohibited.
//
//////////
//

#include "StdAfx.h"

#include "DibRop.h"

void ApplyVectorTransformDIBBits(LPVOID pDstFirstPixel,
									DWORD dwPixelsCopiedAcross,
									DWORD dwScansCopiedUp,
									DWORD dwBytesPerScanDstDib,
									BYTE* abyVectorTransform)
{
    DWORD v, h;
    LPBYTE ppD;

    ppD = (LPBYTE)pDstFirstPixel;
    for (v = 0; v < dwScansCopiedUp; v++)
    {
        ppD = (LPBYTE)pDstFirstPixel + (v * dwBytesPerScanDstDib); //REVIEW
        for (h = 0; h < dwPixelsCopiedAcross; h++)
        {
            *ppD = abyVectorTransform[*ppD];
            ppD++;
        }
    }
}

void TransCopyDIBBits(LPVOID pDstFirstPixel,
									LPVOID pSrcFirstPixel,
									DWORD dwPixelsCopiedAcross,
									DWORD dwScansCopiedUp,
									DWORD dwBytesPerScanDstDib,
									DWORD dwBytesPerScanSrcDib,
									BYTE byTransColorIndex)
{
    DWORD v, h;
    LPBYTE ppD;
    LPBYTE ppS;

    ppD = (LPBYTE)pDstFirstPixel;
    ppS = (LPBYTE)pSrcFirstPixel;
    for (v = 0; v < dwScansCopiedUp; v++)
    {
        ppD = (LPBYTE)pDstFirstPixel + (v * dwBytesPerScanDstDib); //REVIEW
        ppS = (LPBYTE)pSrcFirstPixel + (v * dwBytesPerScanSrcDib); //REVIEW
        for (h = 0; h < dwPixelsCopiedAcross; h++)
        {
            if (*ppS != byTransColorIndex)
            	*ppD = *ppS;
            ppD++;
            ppS++;
        }
    }
}

void CopyDIBBits(LPVOID pDstFirstPixel,
								LPVOID pSrcFirstPixel,
								DWORD dwPixelsCopiedAcross,
								DWORD dwScansCopiedUp,
								DWORD dwBytesPerScanDstDib,
								DWORD dwBytesPerScanSrcDib)
{
    DWORD v, h;
    LPBYTE ppD;
    LPBYTE ppS;

    ppD = (LPBYTE)pDstFirstPixel;
    ppS = (LPBYTE)pSrcFirstPixel;
    for (v = 0; v < dwScansCopiedUp; v++)
    {
        ppD = (LPBYTE)pDstFirstPixel + (v * dwBytesPerScanDstDib); //REVIEW
        ppS = (LPBYTE)pSrcFirstPixel + (v * dwBytesPerScanSrcDib); //REVIEW
        for (h = 0; h < dwPixelsCopiedAcross; h++)
        {
           	*ppD++ = *ppS++;
        }
    }
}


void FillDIBBits(LPVOID pDstFirstPixel,
								DWORD dwPixelsCopiedAcross,
								DWORD dwScansCopiedUp,
								DWORD dwBytesPerScanDstDib,
								BYTE byFillColorIndex)
{
    DWORD v, h;
    LPBYTE ppD;

    ppD = (LPBYTE)pDstFirstPixel;
    for (v = 0; v < dwScansCopiedUp; v++)
    {
        ppD = (LPBYTE)pDstFirstPixel + (v * dwBytesPerScanDstDib); //REVIEW
        for (h = 0; h < dwPixelsCopiedAcross; h++)
        {
           	*ppD++ = byFillColorIndex;
        }
    }
}


void HazeCopyDIBBits(LPVOID pDstFirstPixel,
									LPVOID pSrcFirstPixel,
									DWORD dwPixelsCopiedAcross,
									DWORD dwScansCopiedUp,
									DWORD dwBytesPerScanDstDib,
									DWORD dwBytesPerScanSrcDib,
									BYTE byTransColorIndex,
									BYTE byHazeColorIndex,
									BYTE byOrgPattern, // ((y&7)<<1)|(x&1)
									BYTE byPatternE,
									BYTE byPatternO)
{
    DWORD v, h;
    LPBYTE ppD;
    LPBYTE ppS;

    ppD = (LPBYTE)pDstFirstPixel;
    ppS = (LPBYTE)pSrcFirstPixel;
    for (v = 0; v < dwScansCopiedUp; v++)
    {
        ppD = (LPBYTE)pDstFirstPixel + (v * dwBytesPerScanDstDib); //REVIEW
        ppS = (LPBYTE)pSrcFirstPixel + (v * dwBytesPerScanSrcDib); //REVIEW
        for (h = 0; h < dwPixelsCopiedAcross; h++)
        {
			//TODO: support patterns
            if (*ppS != byTransColorIndex)
            	*ppD = *ppS;
            ppD++;
            ppS++;
        }
    }
}


void GhostCopyDIBBits(LPVOID pDstFirstPixel,
									LPVOID pSrcFirstPixel,
									DWORD dwPixelsCopiedAcross,
									DWORD dwScansCopiedUp,
									DWORD dwBytesPerScanDstDib,
									DWORD dwBytesPerScanSrcDib,
									BYTE byTransColorIndex,
									BYTE byOrgPattern, // ((y&7)<<1)|(x&1)
									BYTE byPatternE,
									BYTE byPatternO)
{
    DWORD v, h;
    LPBYTE ppD;
    LPBYTE ppS;

    ppD = (LPBYTE)pDstFirstPixel;
    ppS = (LPBYTE)pSrcFirstPixel;
    for (v = 0; v < dwScansCopiedUp; v++)
    {
        ppD = (LPBYTE)pDstFirstPixel + (v * dwBytesPerScanDstDib); //REVIEW
        ppS = (LPBYTE)pSrcFirstPixel + (v * dwBytesPerScanSrcDib); //REVIEW
        for (h = 0; h < dwPixelsCopiedAcross; h++)
        {
			//TODO: support patterns
            if (*ppS != byTransColorIndex)
            	*ppD = *ppS;
            ppD++;
            ppS++;
        }
    }
}


void ShadowCopyDIBBits(LPVOID pDstFirstPixel,
									LPVOID pSrcFirstPixel,
									DWORD dwPixelsCopiedAcross,
									DWORD dwScansCopiedUp,
									DWORD dwBytesPerScanDstDib,
									DWORD dwBytesPerScanSrcDib,
									BYTE byTransColorIndex,
									BYTE byShadowColorIndex)
{
    DWORD v, h;
    LPBYTE ppD;
    LPBYTE ppS;

    ppD = (LPBYTE)pDstFirstPixel;
    ppS = (LPBYTE)pSrcFirstPixel;
    for (v = 0; v < dwScansCopiedUp; v++)
    {
        ppD = (LPBYTE)pDstFirstPixel + (v * dwBytesPerScanDstDib); //REVIEW
        ppS = (LPBYTE)pSrcFirstPixel + (v * dwBytesPerScanSrcDib); //REVIEW
        for (h = 0; h < dwPixelsCopiedAcross; h++)
        {
            if (*ppS != byTransColorIndex)
            	*ppD = byShadowColorIndex;
            ppD++;
            ppS++;
        }
    }
}
