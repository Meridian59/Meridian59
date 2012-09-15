// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * dibutil.h:  Header file for dibutil.c
 *             Taken mostly from WinG sample code.
 */

#ifndef _DIBUTIL_H
#define _DIBUTIL_H

typedef     LPBITMAPINFOHEADER PDIB;
typedef     HANDLE             HDIB;

/***************************************************************************
   External function declarations
 **************************************************************************/

PDIB        DibOpenFile(LPSTR szFile);
PDIB        DibCreate(int bits, int dx, int dy);

/****************************************************************************
   Internal function declarations
 ***************************************************************************/

PDIB        DibReadBitmapInfo(int fh);

/****************************************************************************
   DIB macros.
 ***************************************************************************/

#ifdef  WIN32
    #define HandleFromDib(lpbi) GlobalHandle(lpbi)
    #define _huge
#else
    #define HandleFromDib(lpbi) (HANDLE)GlobalHandle(SELECTOROF(lpbi))
#endif

#define DibFromHandle(h)        (PDIB)GlobalLock(h)

#define DibFree(pdib)           GlobalFreePtr(pdib)

#define WIDTHBYTES(i)           ((unsigned)((i+31)&(~31))/8)  /* ULONG aligned ! */

#define DibWidth(lpbi)          (UINT)(((LPBITMAPINFOHEADER)(lpbi))->biWidth)
#define DibHeight(lpbi)         (UINT)(((LPBITMAPINFOHEADER)(lpbi))->biHeight)
#define DibBitCount(lpbi)       (UINT)(((LPBITMAPINFOHEADER)(lpbi))->biBitCount)
#define DibCompression(lpbi)    (DWORD)(((LPBITMAPINFOHEADER)(lpbi))->biCompression)

#define DibWidthBytesN(lpbi, n) (UINT)WIDTHBYTES((UINT)(lpbi)->biWidth * (UINT)(n))
#define DibWidthBytes(lpbi)     DibWidthBytesN(lpbi, (lpbi)->biBitCount)

#define DibSizeImage(lpbi)      ((lpbi)->biSizeImage == 0 \
                                    ? ((DWORD)(UINT)DibWidthBytes(lpbi) * (DWORD)(UINT)(lpbi)->biHeight) \
                                    : (lpbi)->biSizeImage)

#define DibSize(lpbi)           ((lpbi)->biSize + (lpbi)->biSizeImage + (int)(lpbi)->biClrUsed * sizeof(RGBQUAD))
#define DibPaletteSize(lpbi)    (DibNumColors(lpbi) * sizeof(RGBQUAD))

#define DibFlipY(lpbi, y)       ((int)(lpbi)->biHeight-1-(y))

//HACK for NT BI_BITFIELDS DIBs
#ifdef WIN32
    #define DibPtr(lpbi)            ((lpbi)->biCompression == BI_BITFIELDS \
                                       ? (LPVOID)(DibColors(lpbi) + 3) \
                                       : (LPVOID)(DibColors(lpbi) + (UINT)(lpbi)->biClrUsed))
#else
    #define DibPtr(lpbi)            (LPVOID)(DibColors(lpbi) + (UINT)(lpbi)->biClrUsed)
#endif

#define DibColors(lpbi)         ((RGBQUAD FAR *)((LPBYTE)(lpbi) + (int)(lpbi)->biSize))

#define DibNumColors(lpbi)      ((lpbi)->biClrUsed == 0 && (lpbi)->biBitCount <= 8 \
                                    ? (int)(1 << (int)(lpbi)->biBitCount)          \
                                    : (int)(lpbi)->biClrUsed)

#define DibXYN(lpbi,pb,x,y,n)   (LPVOID)(                                     \
                                (BYTE _huge *)(pb) +                          \
                                (UINT)((UINT)(x) * (UINT)(n) / 8u) +          \
                                ((DWORD)DibWidthBytesN(lpbi,n) * (DWORD)(UINT)(y)))

#define DibXY(lpbi,x,y)         DibXYN(lpbi,DibPtr(lpbi),x,y,(lpbi)->biBitCount)

#define FixBitmapInfo(lpbi)     if ((lpbi)->biSizeImage == 0)                 \
                                    (lpbi)->biSizeImage = DibSizeImage(lpbi); \
                                if ((lpbi)->biClrUsed == 0)                   \
                                    (lpbi)->biClrUsed = DibNumColors(lpbi);   \
                                if ((lpbi)->biCompression == BI_BITFIELDS && (lpbi)->biClrUsed == 0) \
                                    ; // (lpbi)->biClrUsed = 3;                    

#define DibInfo(pDIB)     ((BITMAPINFO FAR *)(pDIB))

/***************************************************************************/

#ifndef BI_BITFIELDS
  #define BI_BITFIELDS 3
#endif  

#ifndef HALFTONE
  #define HALFTONE COLORONCOLOR
#endif



#endif /* #ifndef _DIBUTIL_H */
