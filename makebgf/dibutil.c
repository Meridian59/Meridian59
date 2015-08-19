// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * dibutil.c: Utility functions to operate on bitmaps.
 */

#include "makebgf.h"
#include <io.h>

#define BFT_ICON   0x4349   /* 'IC' */
#define BFT_BITMAP 0x4d42   /* 'BM' */
#define BFT_CURSOR 0x5450   /* 'PT' */

/*
 *   Open a DIB file and return a MEMORY DIB, a memory handle containing..
 *
 *   BITMAP INFO    bi
 *   palette data
 *   bits....
 *
 *   Returns NULL if not successful.
 */

PDIB DibOpenFile(LPSTR szFile)
{
    HFILE               fh;
    DWORD               dwLen;
    DWORD               dwBits;
    PDIB                pdib;
    LPVOID              p;
    long                i, width, height;
    BYTE                *row1, *row2, *temp, *bits;

    fh = open(szFile, O_BINARY | O_RDONLY);

    if (fh == -1)
        return NULL;

    pdib = DibReadBitmapInfo(fh);

    if (!pdib)
       return NULL;
    
    /* How much memory do we need to hold the DIB */

    dwBits = DibWidthBytes(pdib) * pdib->biHeight;
    dwLen  = pdib->biSize + DibPaletteSize(pdib) + dwBits;

    /* Can we get more memory? */

    p = realloc(pdib,dwLen);

    if (!p)
    {
       free(pdib);
       pdib = NULL;
    }
    else
    {
        pdib = (PDIB)p;
    }

    if (pdib)
    {
       /* read in the bits */
       bits = (LPBYTE) pdib + (UINT)pdib->biSize + DibPaletteSize(pdib);
       read(fh, bits, dwBits);
       
       /* Flip the bits to make bitmap top-down */
       width = DibWidthBytes(pdib);
       height = DibHeight(pdib);
       temp = (BYTE *) malloc(width);
       for (i=0; i < height / 2; i++)
       {
          row1 = bits + (height - i - 1) * width; 
          row2 = bits + i * width; 
          memcpy(temp, row1, (size_t) width);
          memcpy(row1, row2, (size_t) width);
          memcpy(row2, temp, (size_t) width);
       }
       free(temp);
    }

    close(fh);

    return pdib;
}


/*
 *  ReadDibBitmapInfo()
 *
 *  Will read a file in DIB format and return a global HANDLE to its
 *  BITMAPINFO.  This function will work with both "old" and "new"
 *  bitmap formats, but will always return a "new" BITMAPINFO.
 */

PDIB DibReadBitmapInfo(int fh)
{
    DWORD     off;
    HANDLE    hbi = NULL;
    int       size;
    int       i;
    int       nNumColors;

    RGBQUAD FAR       *pRgb;
    BITMAPINFOHEADER   bi;
    BITMAPCOREHEADER   bc;
    BITMAPFILEHEADER   bf;
    PDIB               pdib;

    if (fh == -1)
        return NULL;

    off = lseek(fh,0L,SEEK_CUR);

    if (sizeof(bf) != read(fh,(LPSTR)&bf,sizeof(bf)))
        return FALSE;

    /*
     *  do we have a RC HEADER?
     */
    if (bf.bfType != BFT_BITMAP)
    {
        bf.bfOffBits = 0L;
        lseek(fh,off,SEEK_SET);
    }

    if (sizeof(bi) != read(fh,(LPSTR)&bi,sizeof(bi)))
        return FALSE;

    /*
     *  what type of bitmap info is this?
     */
    switch (size = (int)bi.biSize)
    {
    default:
    case sizeof(BITMAPINFOHEADER):
       break;
       
    case sizeof(BITMAPCOREHEADER):
       bc = *(BITMAPCOREHEADER*)&bi;
       bi.biSize               = sizeof(BITMAPINFOHEADER);
       bi.biWidth              = (DWORD)bc.bcWidth;
       bi.biHeight             = (DWORD)bc.bcHeight;
       bi.biPlanes             =  (UINT)bc.bcPlanes;
       bi.biBitCount           =  (UINT)bc.bcBitCount;
       bi.biCompression        = BI_RGB;
       bi.biSizeImage          = 0;
       bi.biXPelsPerMeter      = 0;
       bi.biYPelsPerMeter      = 0;
       bi.biClrUsed            = 0;
       bi.biClrImportant       = 0;
       
       lseek(fh,(LONG)sizeof(BITMAPCOREHEADER)-sizeof(BITMAPINFOHEADER),SEEK_CUR);
       
       break;
    }
    
    nNumColors = DibNumColors(&bi);
    
    if (bi.biSizeImage == 0)
        bi.biSizeImage = DibSizeImage(&bi);

    if (bi.biClrUsed == 0)
        bi.biClrUsed = DibNumColors(&bi);

    pdib = (PDIB) malloc((LONG)bi.biSize + nNumColors * sizeof(RGBQUAD));

    if (!pdib)
        return NULL;

    *pdib = bi;

    pRgb = DibColors(pdib);

    if (nNumColors == 0)
    {
       printf("Bitmap has no palette (24 bit)\n");
       return NULL;
    }

    if (nNumColors != NUM_COLORS)
    {
       printf("Expecting %d color bitmap; found %d colors\n", NUM_COLORS, nNumColors);
       return NULL;
    }
       
    if (size == sizeof(BITMAPCOREHEADER))
    {
       /*
        * convert a old color table (3 byte entries) to a new
        * color table (4 byte entries)
        */
       read(fh,(LPVOID)pRgb,nNumColors * sizeof(RGBTRIPLE));
       
       for (i=nNumColors-1; i>=0; i--)
       {
          RGBQUAD rgb;
          
          rgb.rgbRed      = ((RGBTRIPLE FAR *)pRgb)[i].rgbtRed;
          rgb.rgbBlue     = ((RGBTRIPLE FAR *)pRgb)[i].rgbtBlue;
          rgb.rgbGreen    = ((RGBTRIPLE FAR *)pRgb)[i].rgbtGreen;
          rgb.rgbReserved = (BYTE)0;
          
          pRgb[i] = rgb;
       }
    }
    else
    {
       read(fh,(LPVOID)pRgb,nNumColors * sizeof(RGBQUAD));
    }

    if (bf.bfOffBits != 0L)
        lseek(fh,off + bf.bfOffBits,SEEK_SET);

    return pdib;
}
