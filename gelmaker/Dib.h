// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Dib.h : header file
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
// A CDib (DIB: Device Independent Bitmap) is an object similar
// to a CBitmap, except is not as closely supported in Windows 3.1
// systems.  The DIB format more closely resembles the actual file
// data stored for the bitmapped image: it contains a header, a
// color table, and an array of bits.  The CDib class maintains the
// memory object containing these elements, and allows bitmap files
// to be loaded, created or deleted directly.
//
// This class only manages the creation, initialization and
// deletion of the memory object called a DIB.  The Windows APIs
// to employ DIBs in memory DCs are different from those used for
// the normal bitmaps.  For example, use StretchDIBits() instead
// of StretchBlt() or BitBlt() with the Windows DC object.
//

#ifndef __DIB_H__
#define __DIB_H__

/////////////////////////////////////////////////////////////////////////////

// HIWORD of a special DIB->DIB DibBlt Raster Operation.
#define DIB_TRANSCOPY	(0x0A00)
#define DIB_HAZECOPY30	(0x0B30)
#define DIB_HAZECOPY50	(0x0B50)
#define DIB_SHADOWCOPY	(0x0C00)
#define DIB_HALOCOPY	(0x0C01)
#define DIB_GHOSTCOPY30	(0x0D30)
#define DIB_GHOSTCOPY50	(0x0D50)
#define DIB_FILL		(0x0E00)

// Use these DIBROP_* macros to compute the DIB_* above using color index arguments.
//
#define DIBROP_TRANSCOPY(iTransparent)				(DIB_TRANSCOPY | (LOBYTE(iTransparent) << 8))
#define DIBROP_HAZECOPY30(iTransparent, iHaze)		(DIB_HAZECOPY30 | (LOBYTE(iTransparent) << 8) | LOBYTE(iHaze))
#define DIBROP_HAZECOPY50(iTransparent, iHaze)		(DIB_HAZECOPY50 | (LOBYTE(iTransparent) << 8) | LOBYTE(iHaze))
#define DIBROP_SHADOWCOPY(iTransparent, iShadow)	(DIB_SHADOWCOPY | (LOBYTE(iTransparent) << 8) | LOBYTE(iShadow))
#define DIBROP_GHOSTCOPY30(iTransparent)			(DIB_GHOSTCOPY30 | (LOBYTE(iTransparent) << 8))
#define DIBROP_GHOSTCOPY50(iTransparent)			(DIB_GHOSTCOPY50 | (LOBYTE(iTransparent) << 8))
#define DIBROP_FILL(iFill)							(DIB_FILL | (LOBYTE(iFill) << 8))
#define DIBROP_HALOCOPY(iTransparent, iHalo)		(DIB_HALOCOPY | (LOBYTE(iTransparent) << 8) | LOBYTE(iHalo))

class CDib : public CObject
{
	DECLARE_DYNCREATE(CDib)
public:
    CDib();
	CDib(SIZE sz);
		// throw(CResourceException, CMemoryException);
	CDib(UINT uResID, HINSTANCE hInstance = NULL);
		// throw(CResourceException, CMemoryException, CNotSupportedException);
	CDib(CFile* pFile);
		// throw(CResourceException, CMemoryException, CNotSupportedException);

// Attributes
public:
	BITMAPINFOHEADER* m_pInfo; // check against NULL to see if initialized
protected:
	HGLOBAL m_hMemory;
	RGBQUAD* m_pQuad;
	BYTE* m_pBits;
	//
	WORD m_wColorEntries;
	DWORD m_dwScanBytes;
	DWORD m_dwImageBytes;
	DWORD m_dwTotalBytes;
	//
	UINT m_uColumns;
	CSize m_szTile;
	CSize m_szHotSpot;
	CPalette* m_pOriginalPalette;
public:
	CSize GetSize() const
		{ ASSERT(m_pInfo);
		  return CSize((int)m_pInfo->biWidth, (int)m_pInfo->biHeight); }
	//
	void GetRect(LPRECT prc) const { *prc = CRect(CPoint(0, 0), GetSize()); }
	//
	CSize GetTileSize() const { return m_szTile; }
	void SetTileSize(const SIZE& sz)
		{ m_szTile = sz; m_uColumns = (int)m_pInfo->biWidth / sz.cx; }
	//
	CSize GetHotSpot() const { return m_szHotSpot; }
	void SetHotSpot(const SIZE& sz) { m_szHotSpot = sz; }

// Operations
public:
	void LoadDib(UINT uResID, HINSTANCE hInstance = NULL);
		// throw(CResourceException, CMemoryException, CNotSupportedException);
	void LoadDib(CFile* pFile);
		// throw(CResourceException, CMemoryException, CNotSupportedException);
	void CreateDib(SIZE sz);
		// throw(CMemoryException);
	void WriteDib(CFile* pFile);
		// throw(CResourceException, CMemoryException);
	void DeleteDib();
	//
	BYTE GetPixel(POINT pt, UINT uTile = 0);
	CRect GetTileRect(UINT uTile);
	//
	// DIB->DIB
	void DibBlt(CDib* pDestDib, const RECT& rcDest, const POINT& ptSrcTopLeft, DWORD dwRop);
	void DibBlt(CDib* pDestDib, const RECT& rcDest, UINT uTileSrc, DWORD dwRop);
	// DIB->DC
	void DibBlt(CDC* pDestDC, const RECT& rcDest, const POINT& ptSrcTopLeft, DWORD dwRop);
	void DibBlt(CDC* pDestDC, const RECT& rcDest, UINT uTileSrc, DWORD dwRop);
	// DIB<-
	void DibBlt(const RECT& rcDest, DWORD dwRop);
	//
	void SetIdentityPalette();
	CPalette* ClonePalette(BYTE fFlags = 0, BOOL bSysEntries = FALSE); // throw(CMemoryException)
	//
	void CopyBits(CDib* pdibSource);
	void ApplyVectorTransform(BYTE* abyTransform);

// Implementation
public:
    virtual ~CDib();
    BYTE* PixelPointer(const POINT& pt);
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
private:
	void _Construct();
    void _InitDib();
};

/////////////////////////////////////////////////////////////////////////////

#endif // __DIB_H__
