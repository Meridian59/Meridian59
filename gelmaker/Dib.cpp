// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Dib.cpp : implementation file
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

#include "Dib.h"
#include "DibRop.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CDib, CObject)

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define DEBUG(e) (e)
#else
#define DEBUG(e) // nothing
#endif

struct tagDibMemoryStats
{
	DWORD dwDibsCreated;
	DWORD dwBytesHigh;
	DWORD dwBytesAlloced;
tagDibMemoryStats()
	{
		DEBUG(dwDibsCreated = dwBytesHigh = dwBytesAlloced = 0);
	}
~tagDibMemoryStats()
	{
		TRACE("%lu bytes leaked after %lu DIBs\n"
		      "Dib allocation highpoint: %lu bytes\n",
		      dwBytesAlloced, dwDibsCreated, dwBytesHigh);
	}
void addBytes(DWORD dwBytes)
	{
		DEBUG(dwBytesAlloced += dwBytes);
		DEBUG((dwBytesHigh < dwBytesAlloced)? (dwBytesHigh = dwBytesAlloced) : dwBytesHigh);
	}
void removeBytes(DWORD dwBytes)
	{
		DEBUG(dwBytesAlloced -= dwBytes);
	}
void addDib()
	{
		DEBUG(dwDibsCreated++);
	}
};

static tagDibMemoryStats _dibs;

/////////////////////////////////////////////////////////////////////////////

void CDib::_Construct()
{
	m_pInfo = NULL;
	m_pQuad = NULL;
	m_pBits = NULL;
	m_hMemory = NULL;
	m_pOriginalPalette = NULL;
}

CDib::CDib()
{
	_Construct();
}

CDib::CDib(SIZE sz)
{
	_Construct();

	CreateDib(sz);
}

CDib::CDib(UINT uResID, HINSTANCE hInstance /* = NULL */)
{
	_Construct();

	LoadDib(uResID, hInstance);
}

CDib::CDib(CFile* pFile)
{
	_Construct();

	ASSERT_DYNAMIC(pFile, CFile);
	LoadDib(pFile);
}

CDib::~CDib()
{
	if (m_pInfo)
		DeleteDib();
}

#ifdef _DEBUG

void CDib::AssertValid() const
{
    CObject::AssertValid();
    ASSERT(!m_pInfo || m_pInfo->biSize == sizeof(BITMAPINFOHEADER));
}

void CDib::Dump(CDumpContext& dc) const
{
    CObject::Dump(dc);
	dc	<< "\nm_pInfo = " << (void*)m_pInfo
		<< "\nm_pQuad = " << (void*)m_pQuad
		<< "\nm_pBits = " << (void*)m_pBits
		<< "\nm_hMemory = " << m_hMemory
        ;
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

void CDib::_InitDib()
{
	ASSERT(m_pInfo);

	m_uColumns = 1;
	m_szTile = CSize((int)m_pInfo->biWidth, (int)m_pInfo->biHeight);
	m_szHotSpot = CSize(0, 0);
}

/////////////////////////////////////////////////////////////////////////////

void CDib::LoadDib(UINT uResID, HINSTANCE hInstance /* = NULL */)
{
	ASSERT(!m_pInfo);
	ASSERT(!m_pQuad);
	ASSERT(!m_pBits);
	m_hMemory = NULL;

	if (!hInstance)
		hInstance = AfxGetResourceHandle();


	// Load and lock the HGLOBAL resource
	//
	HRSRC hR = ::FindResource(hInstance,
	                          MAKEINTRESOURCE(uResID),
	                          RT_BITMAP);
	if (!hR)
	{
		TRACE("CDib::LoadDib(%u): resource not found\n", uResID);
		AfxThrowResourceException();
	}
	//
	HGLOBAL hG = ::LoadResource(hInstance, hR);
	if (!hG)
	{
		AfxThrowMemoryException();
	}
	//
	void* pMemory = ::LockResource(hG);
	if (!pMemory)
		AfxThrowMemoryException();
	//
	BYTE* pWalkMemory = (BYTE*)pMemory;


	// Verify the BITMAPINFOHEADER
	//
	BITMAPINFOHEADER* pSourceInfoHeader = (BITMAPINFOHEADER*)pWalkMemory;
	pWalkMemory += sizeof(BITMAPINFOHEADER);
	if (pSourceInfoHeader->biSize != sizeof(BITMAPINFOHEADER))
	{
		TRACE("CDib::LoadDib: error: resource not a Windows DIB\n");
		AfxThrowResourceException();
	}


	// Verify the RGBQUAD array
	//
	WORD wColors = 0;
	switch (pSourceInfoHeader->biBitCount)
	{
	case 8:
		wColors = 256;
		break;

	case 1:
	case 4:
	default:
		TRACE("CDib currently only supports 8bpp DIBs.");
		AfxThrowNotSupportedException();
		break;
	}
	if (LOWORD(pSourceInfoHeader->biClrUsed) &&
	    !HIWORD(pSourceInfoHeader->biClrUsed))
	{
		wColors = LOWORD(pSourceInfoHeader->biClrUsed);
	}
	m_wColorEntries = wColors;


	// Now that we've verified the size and layout of the original
	// (read-only) resource, we create a copy of it which we can
	// manipulate at will.
	//
	m_dwScanBytes = ((pSourceInfoHeader->biWidth + 3) & ~3); //REVIEW: assumes 8bpp
	m_dwImageBytes = m_dwScanBytes * pSourceInfoHeader->biHeight;
	m_dwTotalBytes = sizeof(BITMAPINFOHEADER) +
	                 m_wColorEntries * sizeof(RGBQUAD) +
	                 m_dwImageBytes;
	m_hMemory = ::GlobalAlloc(GMEM_FIXED, m_dwTotalBytes);
	BYTE* pNewDib = (BYTE*)::GlobalLock(m_hMemory);
	if (!pNewDib)
		AfxThrowMemoryException();
	memcpy(pNewDib, pMemory, m_dwTotalBytes);
	//
	m_pInfo = (BITMAPINFOHEADER*)pNewDib;
	m_pQuad = (RGBQUAD*)(m_pInfo+1);
	m_pBits = (BYTE*)(m_pQuad+m_wColorEntries);
	//
	_dibs.addBytes(m_dwTotalBytes);
	_dibs.addDib();

	ASSERT(!m_pOriginalPalette);
	m_pOriginalPalette = ClonePalette(PC_NOCOLLAPSE);
	SetIdentityPalette();

	_InitDib();

	// Free the read-only resource stuff.
	UnlockResource(hG);
	::FreeResource(hG);

	ASSERT_VALID(this);
}

/////////////////////////////////////////////////////////////////////////////

void CDib::LoadDib(CFile* pFile)
{
	ASSERT_DYNAMIC(pFile, CFile);
	ASSERT(!m_pInfo);
	ASSERT(!m_pQuad);
	ASSERT(!m_pBits);
	m_hMemory = NULL;

	if (!pFile)
	{
		TRACE("CDib::LoadDib: error: null file.\n");
		AfxThrowResourceException();
	}


	// Allocate the memory for the file's contents.
	//
	CFileStatus fs;
	pFile->GetStatus(fs);
	BYTE* pMemory = new BYTE[fs.m_size]; // throw(CMemoryException);
	pFile->Read(pMemory, fs.m_size);
	BYTE* pWalkMemory = pMemory;


	// Verify the BITMAPFILEHEADER
	//
	BITMAPFILEHEADER* pFileHeader = (BITMAPFILEHEADER*)pWalkMemory;
	pWalkMemory += sizeof(BITMAPFILEHEADER);
	if (pFileHeader->bfType != (WORD)('B' | ('M' << 8)))
	{
		TRACE("CDib::LoadDib: error: resource not a Windows DIB\n");
		AfxThrowResourceException();
	}


	// Verify the BITMAPINFOHEADER
	//
	BITMAPINFOHEADER* pSourceInfoHeader = (BITMAPINFOHEADER*)pWalkMemory;
	pWalkMemory += sizeof(BITMAPINFOHEADER);
	if (pSourceInfoHeader->biSize != sizeof(BITMAPINFOHEADER))
	{
		TRACE("CDib::LoadDib: error: resource not a Windows DIB\n");
		AfxThrowResourceException();
	}


	// Verify the RGBQUAD array
	//
	WORD wColors = 0;
	switch (pSourceInfoHeader->biBitCount)
	{
	case 8:
		wColors = 256;
		break;

	case 1:
	case 4:
	default:
		TRACE("CDib currently only supports 8bpp DIBs.");
		AfxThrowNotSupportedException();
		break;
	}
	if (LOWORD(pSourceInfoHeader->biClrUsed) &&
	    !HIWORD(pSourceInfoHeader->biClrUsed))
	{
		wColors = LOWORD(pSourceInfoHeader->biClrUsed);
	}
	m_wColorEntries = wColors;


	// Now that we've verified the size and layout of the original
	// (read-only) resource, we create a copy of it which we can
	// manipulate at will.
	//
	m_dwScanBytes = ((pSourceInfoHeader->biWidth + 3) & ~3); //REVIEW: assumes 8bpp
	m_dwImageBytes = m_dwScanBytes * pSourceInfoHeader->biHeight;
	m_dwTotalBytes = sizeof(BITMAPINFOHEADER) +
	                 m_wColorEntries * sizeof(RGBQUAD) +
	                 m_dwImageBytes;
	m_hMemory = ::GlobalAlloc(GMEM_FIXED, m_dwTotalBytes);
	BYTE* pNewDib = (BYTE*)::GlobalLock(m_hMemory);
	if (!pNewDib)
		AfxThrowMemoryException();
	memcpy(pNewDib, pSourceInfoHeader, m_dwTotalBytes);
	//
	m_pInfo = (BITMAPINFOHEADER*)pNewDib;
	m_pQuad = (RGBQUAD*)(m_pInfo+1);
	m_pBits = (BYTE*)(m_pQuad+m_wColorEntries);
	//
	_dibs.addBytes(m_dwTotalBytes);
	_dibs.addDib();

	ASSERT(!m_pOriginalPalette);
	m_pOriginalPalette = ClonePalette(PC_NOCOLLAPSE);
	SetIdentityPalette();

	_InitDib();

	// Free the read-only source memory stuff.
	delete[] pMemory;

	ASSERT_VALID(this);
}

/////////////////////////////////////////////////////////////////////////////

void CDib::CreateDib(SIZE sz)
{
	//REVIEW: only 8bpp supported
	UINT uPlanes = 1;
	UINT uBitsPerPixel = 8;

	ASSERT(!m_pInfo);
	ASSERT(!m_pQuad);
	ASSERT(!m_pBits);
	m_hMemory = NULL;

	// Calculate what memory we'll need.
	m_wColorEntries = 0;
	if (uBitsPerPixel && uBitsPerPixel <= 8)
		m_wColorEntries = 1 << uBitsPerPixel;
	//
	m_dwScanBytes = ((uBitsPerPixel * uPlanes) / 8) * sz.cx;
	m_dwScanBytes = (m_dwScanBytes + 3) & ~3;
	m_dwImageBytes = m_dwScanBytes * sz.cy;
	//
	m_dwTotalBytes = sizeof(BITMAPINFOHEADER) +
	                 sizeof(RGBQUAD) * m_wColorEntries +
	                 m_dwImageBytes;

	// Allocate it.  Keep pointers to the
	// basic portions of the packed dib.
	//
	m_hMemory = ::GlobalAlloc(GMEM_FIXED, m_dwTotalBytes);
	BYTE* pNewDib = (BYTE*)::GlobalLock(m_hMemory);
	if (!pNewDib)
		AfxThrowMemoryException();
	_dibs.addBytes(m_dwTotalBytes);
	_dibs.addDib();
	//
	m_pInfo = (BITMAPINFOHEADER*)pNewDib;
	m_pQuad = (RGBQUAD*)(m_pInfo+1);
	m_pBits = (BYTE*)(m_pQuad+m_wColorEntries);

	// Set the header.
    m_pInfo->biSize = sizeof(BITMAPINFOHEADER);
    m_pInfo->biWidth = sz.cx;
    m_pInfo->biHeight = sz.cy;
    m_pInfo->biPlanes = uPlanes;
    m_pInfo->biBitCount = uBitsPerPixel;
    m_pInfo->biCompression = BI_RGB;
    m_pInfo->biSizeImage = m_dwImageBytes;
    m_pInfo->biXPelsPerMeter = 0;
    m_pInfo->biYPelsPerMeter = 0;
    m_pInfo->biClrUsed = m_wColorEntries;
    m_pInfo->biClrImportant = 0;

	// Force an identity palette.
	SetIdentityPalette();

	_InitDib();

#ifdef _DEBUGRARE
	TRACE("Dib created for %dx%d pixels.\n", (int)m_pInfo->biWidth, (int)m_pInfo->biHeight);
	void* q;
	q = PixelPointer(CPoint(0, 0)); afxDump << " dib pixel @ 0,0: " << (void*)q << " = " << (int)(*(BYTE*)q) << "\n";
	if (m_pInfo->biWidth > 1 && m_pInfo->biHeight > 1)
	{
		q = PixelPointer(CPoint((int)m_pInfo->biWidth-1, 0)); afxDump << " dib pixel @ w,0: " << (void*)q << " = " << (int)(*(BYTE*)q) << "\n";
		q = PixelPointer(CPoint(0, (int)m_pInfo->biHeight-1)); afxDump << " dib pixel @ 0,h: " << (void*)q << " = " << (int)(*(BYTE*)q) << "\n";
		q = PixelPointer(CPoint((int)m_pInfo->biWidth-1, (int)m_pInfo->biHeight-1)); afxDump << " dib pixel @ w,h: " << (void*)q << " = " << (int)(*(BYTE*)q) << "\n";
	}
#endif // _DEBUGRARE
}

/////////////////////////////////////////////////////////////////////////////

void CDib::WriteDib(CFile* pFile)
{
	ASSERT_DYNAMIC(pFile, CFile);
	ASSERT(m_pInfo);

	if (!pFile)
	{
		TRACE("CDib::WriteDib: error: null file\n");
		return;
	}
	if (!m_pInfo)
	{
		TRACE("CDib::WriteDib: error: dib not created or loaded\n");
		return;
	}

	// Write each component of a DIB file in turn.

	// The BITMAPFILEHEADER.
	//
	BITMAPFILEHEADER bmfh;
	memset(&bmfh, 0, sizeof(BITMAPFILEHEADER));
	bmfh.bfType = ('B' | ('M' << 8));
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) +
	                 sizeof(BITMAPINFOHEADER) +
	                 (m_wColorEntries * sizeof(RGBQUAD));
	bmfh.bfSize = bmfh.bfOffBits + m_dwImageBytes;
	pFile->Write(&bmfh, sizeof(BITMAPFILEHEADER));

	// The BITMAPINFOHEADER.
	//
	pFile->Write(m_pInfo, sizeof(BITMAPINFOHEADER));

	// The RGBQUADs.
	//
	PALETTEENTRY quads[256];
	ASSERT_DYNAMIC(m_pOriginalPalette, CPalette);
	m_pOriginalPalette->GetPaletteEntries(0, 256, quads);
	pFile->Write(quads, 256 * sizeof(RGBQUAD));

	// The pixel bits.
	//
	pFile->Write(m_pBits, m_dwImageBytes);
}

/////////////////////////////////////////////////////////////////////////////

void CDib::DeleteDib()
{
	if (m_pInfo)
	{
		_dibs.removeBytes(m_dwTotalBytes);

		// This may have been discarded or deleted already!
		::GlobalUnlock(m_hMemory);
		::GlobalFree(m_hMemory);
		m_pInfo = NULL;
		m_hMemory = NULL;
	}
	m_pQuad = NULL;
	m_pBits = NULL;

	if (m_pOriginalPalette)
		delete m_pOriginalPalette;

	m_wColorEntries = 0;
	m_dwImageBytes = 0;
	m_dwTotalBytes = 0;
}

/////////////////////////////////////////////////////////////////////////////

// DibBlt:
// Note that DWORD dwRop is not supported as usual for DIB->DIB blits.
//
// Rop:
//		SRCCOPY					Just like GDI's SRCCOPY
//
//		BLACKNESS/WHITENESS		Fills with 0's or 0xFF's, respectively.
//								Ignores source Dib.
//
//		DIBROP_TRANSCOPY(a)
//								All source pixels of index 'a' are not drawn.
//								Everything else drawn as with SRCCOPY.
//
// The ternary raster operations "SRCCOPY" is officially supported.
// If you send MAKELONG(index << 8, DIB_*), then pixels of the given index
// are considered transparent.  See tech note below.  All other rops cause an
// assertion failure in _DEBUG compiles.
//
// For DIB->DC blits, the dwRop is any standard GDI raster operation, and
// transparency isn't supported.
//

void CDib::DibBlt(CDib* pDstDib, const RECT& rcDst,
                  const POINT& ptSrcTopLeft, DWORD dwRop)
{
	// The destination area still inside the target dib.
	CRect rcClippedDst(&rcDst);
	rcClippedDst.IntersectRect(rcClippedDst,
	                           CRect(CPoint(0, 0), pDstDib->GetSize()));

	// The destination area still coming from the source dib.
	CRect rcClippedSrc(&rcClippedDst);
	rcClippedSrc.OffsetRect(ptSrcTopLeft.x-rcDst.left, ptSrcTopLeft.y-rcDst.top);

	// The "first pixel" for these Blits is the lower-left pixel.
	//
	int nWidth = rcClippedDst.right - rcClippedDst.left;
	int nHeight = rcClippedDst.bottom - rcClippedDst.top;
	if (!nWidth || !nHeight)
		return;
	LPVOID pDst = pDstDib->PixelPointer(CPoint(rcClippedDst.left, rcClippedDst.bottom-1));
	LPVOID pSrc = PixelPointer(CPoint(rcClippedSrc.left, rcClippedSrc.bottom-1));

	// Dithers will align to the destination's hot-spot.
	CPoint ptOrg(rcClippedDst.left - m_szHotSpot.cx,
	             rcClippedDst.bottom-1 - m_szHotSpot.cy);

	// Tech note:
	// The ROP is one of the supported standard ROPS or a computed ROP with
	// one or two color index arguments packed into the low word.
	//
	switch (dwRop)
	{
	case SRCCOPY:
		CopyDIBBits(pDst, pSrc, (DWORD)nWidth, (DWORD)nHeight,
		            pDstDib->m_dwScanBytes, m_dwScanBytes);
		break;

	case BLACKNESS:
		FillDIBBits(pDst, (DWORD)nWidth, (DWORD)nHeight,
		            pDstDib->m_dwScanBytes, 0x00);
		break;

	case WHITENESS:
		FillDIBBits(pDst, (DWORD)nWidth, (DWORD)nHeight,
		            pDstDib->m_dwScanBytes, 0xFF);
		break;

	default:
		switch (HIWORD(dwRop))
		{
		case DIB_TRANSCOPY:
			TransCopyDIBBits(pDst, pSrc, (DWORD)nWidth, (DWORD)nHeight,
			                 pDstDib->m_dwScanBytes, m_dwScanBytes,
			                 HIBYTE(LOWORD(dwRop)));
			break;

		case DIB_SHADOWCOPY:
			ShadowCopyDIBBits(pDst, pSrc, (DWORD)nWidth, (DWORD)nHeight,
			                 pDstDib->m_dwScanBytes, m_dwScanBytes,
			                 HIBYTE(LOWORD(dwRop)), LOBYTE(LOWORD(dwRop)));
			break;

		case DIB_FILL:
			FillDIBBits(pDst, (DWORD)nWidth, (DWORD)nHeight,
			            pDstDib->m_dwScanBytes, HIBYTE(LOWORD(dwRop)));
			break;

		case DIB_HAZECOPY50:
			HazeCopyDIBBits(pDst, pSrc, (DWORD)nWidth, (DWORD)nHeight,
			                 pDstDib->m_dwScanBytes, m_dwScanBytes,
			                 HIBYTE(LOWORD(dwRop)), LOBYTE(LOWORD(dwRop)),
			                 (BYTE)(((ptOrg.x&7)<<1)|(ptOrg.y&1)),
			                 (BYTE)0x55, (BYTE)0xAA);
			break;

		case DIB_HAZECOPY30:
			HazeCopyDIBBits(pDst, pSrc, (DWORD)nWidth, (DWORD)nHeight,
			                 pDstDib->m_dwScanBytes, m_dwScanBytes,
			                 HIBYTE(LOWORD(dwRop)), LOBYTE(LOWORD(dwRop)),
			                 (BYTE)(((ptOrg.x&7)<<1)|(ptOrg.y&1)),
			                 (BYTE)0x44, (BYTE)0x11);
			break;

		case DIB_GHOSTCOPY50:
			GhostCopyDIBBits(pDst, pSrc, (DWORD)nWidth, (DWORD)nHeight,
			                 pDstDib->m_dwScanBytes, m_dwScanBytes,
			                 HIBYTE(LOWORD(dwRop)),
			                 (BYTE)(((ptOrg.x&7)<<1)|(ptOrg.y&1)),
			                 (BYTE)0x55, (BYTE)0xAA);
			break;

		case DIB_GHOSTCOPY30:
			GhostCopyDIBBits(pDst, pSrc, (DWORD)nWidth, (DWORD)nHeight,
			                 pDstDib->m_dwScanBytes, m_dwScanBytes,
			                 HIBYTE(LOWORD(dwRop)),
			                 (BYTE)(((ptOrg.x&7)<<1)|(ptOrg.y&1)),
			                 (BYTE)0x44, (BYTE)0x11);
			break;

		ASSERT_NODEFAULT();
		}
	}
}

void CDib::DibBlt(CDib* pDstDib, const RECT& rcDst,
                  UINT uTileSrc, DWORD dwRop)
{
	CPoint pt((int)((uTileSrc % m_uColumns) * m_szTile.cx),
	          (int)((uTileSrc / m_uColumns) * m_szTile.cy));
	DibBlt(pDstDib, rcDst, pt, dwRop);
}

void CDib::DibBlt(CDC* pDstDC, const RECT& rcDst,
                  const POINT& ptSrcTopLeft, DWORD dwRop)
{
	int nWidth = rcDst.right - rcDst.left;
	int nHeight = rcDst.bottom - rcDst.top;
	::StretchDIBits(pDstDC->m_hDC,
	                rcDst.left, rcDst.top, nWidth, nHeight,
	                ptSrcTopLeft.x, (int)m_pInfo->biHeight - (ptSrcTopLeft.y + nHeight),
	                nWidth, nHeight,
	                (LPVOID)m_pBits, (LPBITMAPINFO)m_pInfo,
	                DIB_PAL_COLORS, dwRop);
}

void CDib::DibBlt(CDC* pDstDC, const RECT& rcDst,
                  UINT uTileSrc, DWORD dwRop)
{
	CPoint pt((int)((uTileSrc % m_uColumns) * m_szTile.cx),
	          (int)((uTileSrc / m_uColumns) * m_szTile.cy));

	DibBlt(pDstDC, rcDst, pt, dwRop);
}

void CDib::DibBlt(const RECT& rcDst, DWORD dwRop)
{
	// The destination area still inside the target dib.
	CRect rcClippedDst(&rcDst);
	rcClippedDst.IntersectRect(rcClippedDst,
	                           CRect(CPoint(0, 0), GetSize()));

	// The "first pixel" for these Blits is the lower-left pixel.
	//
	int nWidth = rcClippedDst.right - rcClippedDst.left;
	int nHeight = rcClippedDst.bottom - rcClippedDst.top;
	LPVOID pDst = PixelPointer(CPoint(rcClippedDst.left, rcClippedDst.bottom-1));

	FillDIBBits(pDst, (DWORD)nWidth, (DWORD)nHeight,
	            m_dwScanBytes, HIBYTE(LOWORD(dwRop)));
}

/////////////////////////////////////////////////////////////////////////////

CRect CDib::GetTileRect(UINT uTile)
{
	CRect rc;

	rc.left = (int)((uTile % m_uColumns) * m_szTile.cx);
	rc.top = (int)((uTile / m_uColumns) * m_szTile.cy);
	rc.right = rc.left + m_szTile.cx;
	rc.bottom = rc.top + m_szTile.cy;

	return rc;
}

/////////////////////////////////////////////////////////////////////////////

void CDib::SetIdentityPalette()
{
	ASSERT(m_pInfo);

	if (!m_wColorEntries || !m_pQuad)
		return;

	// Note that this writes the first m_wColorEntries WORDs.
	// The remaining half of the existing RGBQUADs are then ignored.
	// Once identity palette is used, only use DIB_PAL_COLORS with
	// ::StretchDIBits (like DibBlt(CDC*, ...) does).
	//
	WORD* pw;
	WORD w;
	for (w = 0, pw = (WORD*)m_pQuad; w < m_wColorEntries; w++, pw++)
		*pw = w;
}

CPalette* CDib::ClonePalette(BYTE fFlags /* = 0 */, BOOL bSysEntries /* = FALSE */)
{
	ASSERT(m_pInfo);

	// Color table?
	WORD wColors = m_wColorEntries;
	if (!wColors)
	{
		TRACE("CDib::ClonePalette: warning: no color table\n");
		return NULL;
	}

	// Allocate the logpalette.
	// It is a LOGPALETTE immediately followed by a PALETTEENTRY array.
	// Even more confusing, the LOGPALETTE's last member is the first
	// PALETTEENTRY of the array.  Thus, the 255 instead of a full 256.
	//
	BYTE* p = new BYTE[sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * (255)];
	if (!p)
		AfxThrowMemoryException();

	// Fill in the logpalette.
	//
	LOGPALETTE* pLP = (LOGPALETTE*)p;
	pLP->palVersion = 0x0300;      // Windows 3.0 and higher
	if (m_pOriginalPalette)
	{
		// Copy the original palette's entries into the logpalette.
		m_pOriginalPalette->GetPaletteEntries(0, 256, &pLP->palPalEntry[0]);
		pLP->palNumEntries = 256;
	}
	else
	{
		// Init the logpalette with our color table.
		//
		pLP->palNumEntries = wColors;
		RGBQUAD* pQuad = m_pQuad;
		PALETTEENTRY* pLPE = &pLP->palPalEntry[0];
		for ( ; wColors; wColors--)
		{
			pLPE->peRed = pQuad->rgbRed;
			pLPE->peGreen = pQuad->rgbGreen;
			pLPE->peBlue = pQuad->rgbBlue;
			pLPE->peFlags = fFlags;

			pLPE++;
			pQuad++;
		}
	}

	if (bSysEntries)
	{
		CWindowDC dc(NULL);
		if (dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE)
		{
			ASSERT(m_wColorEntries == 256);
	    	::GetSystemPaletteEntries(dc.m_hDC, 0, 10, &pLP->palPalEntry[0]);
	    	::GetSystemPaletteEntries(dc.m_hDC, 256-10, 10, &pLP->palPalEntry[256-10]);
		}
	}

#ifdef _DEBUGRARE
	for (wColors = 0; wColors < pLP->palNumEntries; wColors++)
	{
		TRACE("Dib color %u: %d R, %d G, %d B.\n",
		      wColors,
		      pLP->palPalEntry[wColors].peRed,
		      pLP->palPalEntry[wColors].peGreen,
		      pLP->palPalEntry[wColors].peBlue);
	}
#endif // _DEBUGRARE

	// Create the palette object from the logical palette.
	// Once created (or failed), we can delete the logpalette.
	//
	CPalette* pPal = new CPalette();
	if (pPal)
		pPal->CreatePalette(pLP);
	delete pLP;

    return pPal;
}

/////////////////////////////////////////////////////////////////////////////

void CDib::CopyBits(CDib* pdibSource)
{
	ASSERT(pdibSource);
	ASSERT(pdibSource->GetSize() == GetSize());
	if (!pdibSource || pdibSource->GetSize() != GetSize())
		return;

	pdibSource->DibBlt(this, CRect(CPoint(0, 0), GetSize()), CPoint(0, 0), SRCCOPY);
}

void CDib::ApplyVectorTransform(BYTE* abyTransform)
{
	// The "first pixel" for these Blits is the lower-left pixel.
	//
	int nWidth = GetSize().cx;
	int nHeight = GetSize().cy;
	LPVOID pDst = PixelPointer(CPoint(0, nHeight-1));

	ApplyVectorTransformDIBBits(pDst, (DWORD)nWidth, (DWORD)nHeight,
	            m_dwScanBytes, abyTransform);
}

/////////////////////////////////////////////////////////////////////////////

BYTE CDib::GetPixel(POINT pt, UINT uTile /* = 0 */)
{
	ASSERT(m_pInfo);

	CPoint point = pt;
	point += CSize((int)((uTile % m_uColumns) * m_szTile.cx),
	               (int)((uTile / m_uColumns) * m_szTile.cy));

	BYTE* p = PixelPointer(point);
	return *p;
}

/////////////////////////////////////////////////////////////////////////////

BYTE* CDib::PixelPointer(const POINT& pt)
{
	ASSERT(m_pInfo);

#ifdef WIN32
	BYTE* p;
#else
	BYTE __huge* p;
#endif // WIN32

	ASSERT(pt.x >= 0 && pt.x < m_pInfo->biWidth);
	ASSERT(pt.y >= 0 && pt.y < m_pInfo->biHeight);

	//
	p = m_pBits;
	DWORD s = (m_pInfo->biHeight - 1 - pt.y);
	s *= m_dwScanBytes;
	p += s;
	p += pt.x;
	return p;
}
