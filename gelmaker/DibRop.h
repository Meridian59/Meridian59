// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// DibRop.h : header file for C, C++ users of fast32.asm
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

#ifndef __DIBROP_H__
#define __DIBROP_H__

/////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

void ApplyVectorTransformDIBBits(LPVOID pDstFirstPixel,
									DWORD dwPixelsCopiedAcross,
									DWORD dwScansCopiedUp,
									DWORD dwBytesPerScanDstDib,
									BYTE* abyVectorTransform);

void TransCopyDIBBits(LPVOID pDstFirstPixel,
									LPVOID pSrcFirstPixel,
									DWORD dwPixelsCopiedAcross,
									DWORD dwScansCopiedUp,
									DWORD dwBytesPerScanDstDib,
									DWORD dwBytesPerScanSrcDib,
									BYTE byTransColorIndex);

void CopyDIBBits(LPVOID pDstFirstPixel,
								LPVOID pSrcFirstPixel,
								DWORD dwPixelsCopiedAcross,
								DWORD dwScansCopiedUp,
								DWORD dwBytesPerScanDstDib,
								DWORD dwBytesPerScanSrcDib);

void FillDIBBits(LPVOID pDstFirstPixel,
								DWORD dwPixelsCopiedAcross,
								DWORD dwScansCopiedUp,
								DWORD dwBytesPerScanDstDib,
								BYTE byFillColorIndex);

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
									BYTE byPatternO);

void GhostCopyDIBBits(LPVOID pDstFirstPixel,
									LPVOID pSrcFirstPixel,
									DWORD dwPixelsCopiedAcross,
									DWORD dwScansCopiedUp,
									DWORD dwBytesPerScanDstDib,
									DWORD dwBytesPerScanSrcDib,
									BYTE byTransColorIndex,
									BYTE byOrgPattern, // ((y&7)<<1)|(x&1)
									BYTE byPatternE,
									BYTE byPatternO);

void ShadowCopyDIBBits(LPVOID pDstFirstPixel,
									LPVOID pSrcFirstPixel,
									DWORD dwPixelsCopiedAcross,
									DWORD dwScansCopiedUp,
									DWORD dwBytesPerScanDstDib,
									DWORD dwBytesPerScanSrcDib,
									BYTE byTransColorIndex,
									BYTE byShadowColorIndex);

#ifdef __cplusplus
} // extern "C"
#endif

/////////////////////////////////////////////////////////////////////////////

#endif // __DIBROP_H__
