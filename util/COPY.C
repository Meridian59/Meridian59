//**********************************************************************
//
//  copy.c
//
//  Source file for Device-Independent Bitmap (DIB) API.  Provides
//  the following functions:
//
//  CopyWindowToDIB()   - Copies a window to a DIB
//  CopyScreenToDIB()   - Copies entire screen to a DIB
//  CopyWindowToBitmap()- Copies a window to a standard Bitmap
//  CopyScreenToBitmap()- Copies entire screen to a standard Bitmap
//  PaintDIB()          - Displays DIB in the specified DC
//  PaintBitmap()       - Displays bitmap in the specified DC
//
//  The following functions are called from DIBUTIL.C:
//
//  DIBToBitmap()       - Creates a bitmap from a DIB
//  BitmapToDIB()       - Creates a DIB from a bitmap
//  DIBWidth()          - Gets the width of the DIB
//  DIBHeight()         - Gets the height of the DIB
//  CreateDIBPalette()  - Gets the DIB's palette
//  GetSystemPalette()  - Gets the current palette
//
// Development Team: Mark Bader
//                   Patrick Schreiber
//                   Garrett McAuliffe
//                   Eric Flo
//                   Tony Claflin
//
// Written by Microsoft Product Support Services, Developer Support.
// COPYRIGHT:
//
//   (C) Copyright Microsoft Corp. 1993.  All rights reserved.
//
//   You have a royalty-free right to use, modify, reproduce and
//   distribute the Sample Files (and/or any modified version) in
//   any way you find useful, provided that you agree that
//   Microsoft has no warranty obligations or liability for any
//   Sample Application Files which are modified.
//
//**********************************************************************

/* header files */
#include <WINDOWS.H>
#include "DIBUTIL.H"
#include "DIBAPI.H"

/*************************************************************************
 *
 * CopyWindowToDIB()
 *
 * Parameters:
 *
 * HWND hWnd        - specifies the window
 *
 * WORD fPrintArea  - specifies the window area to copy into the device-
 *                    independent bitmap
 *
 * Return Value:
 *
 * HDIB             - identifies the device-independent bitmap
 *
 * Description:
 *
 * This function copies the specified part(s) of the window to a device-
 * independent bitmap.
 *
 * History:   Date      Author              Reason
 *            9/15/91   Patrick Schreiber   Created
 *            9/25/91   Patrick Schreiber   Added header and comments
 *
 ************************************************************************/


HDIB FAR CopyWindowToDIB(HWND hWnd, WORD fPrintArea)
{
   HDIB hDIB = NULL;  // handle to DIB

   /* check for a valid window handle */

   if (!hWnd)
      return NULL;
   switch (fPrintArea)
      {
   case PW_WINDOW: // copy entire window
   {
      RECT rectWnd;

      /* get the window rectangle */

      GetWindowRect(hWnd, &rectWnd);

      /*  get the DIB of the window by calling
       *  CopyScreenToDIB and passing it the window rect
       */
      hDIB = CopyScreenToDIB(&rectWnd);
   }
      break;

   case PW_CLIENT: // copy client area
   {
      RECT rectClient;
      POINT pt1, pt2;

      /* get the client area dimensions */

      GetClientRect(hWnd, &rectClient);

      /* convert client coords to screen coords */
      pt1.x = rectClient.left;
      pt1.y = rectClient.top;
      pt2.x = rectClient.right;
      pt2.y = rectClient.bottom;
      ClientToScreen(hWnd, &pt1);
      ClientToScreen(hWnd, &pt2);
      rectClient.left = pt1.x;
      rectClient.top = pt1.y;
      rectClient.right = pt2.x;
      rectClient.bottom = pt2.y;

      /*  get the DIB of the client area by calling
       *  CopyScreenToDIB and passing it the client rect
       */
      hDIB = CopyScreenToDIB(&rectClient);
   }
      break;

   default:    // invalid print area
      return NULL;
      }

   /* return the handle to the DIB */
   return hDIB;
}


/*************************************************************************
 *
 * CopyScreenToDIB()
 *
 * Parameter:
 *
 * LPRECT lpRect    - specifies the window
 *
 * Return Value:
 *
 * HDIB             - identifies the device-independent bitmap
 *
 * Description:
 *
 * This function copies the specified part of the screen to a device-
 * independent bitmap.
 *
 * History:   Date      Author             Reason
 *            9/15/91   Patrick Schreiber  Created
 *            9/25/91   Patrick Schreiber  Added header and comments
 *            12/10/91  Patrick Schreiber  Released palette
 *
 ************************************************************************/


HDIB FAR CopyScreenToDIB(LPRECT lpRect)
{
   HBITMAP hBitmap;    // handle to device-dependent bitmap
   HPALETTE hPalette;  // handle to palette
   HDIB hDIB = NULL;   // handle to DIB

   /*  get the device-dependent bitmap in lpRect by calling
    *  CopyScreenToBitmap and passing it the rectangle to grab
    */

   hBitmap = CopyScreenToBitmap(lpRect);

   /* check for a valid bitmap handle */
   if (!hBitmap)
      return NULL;

   /* get the current palette */
   hPalette = GetSystemPalette();

   /* convert the bitmap to a DIB */
   hDIB = BitmapToDIB(hBitmap, hPalette);

   /* clean up */
   DeleteObject(hPalette);
   DeleteObject(hBitmap);

   /* return handle to the packed-DIB */
   return hDIB;
}


/*************************************************************************
 *
 * CopyWindowToBitmap()
 *
 * Parameters:
 *
 * HWND hWnd        - specifies the window
 *
 * WORD fPrintArea  - specifies the window area to copy into the device-
 *                    dependent bitmap
 *
 * Return Value:
 *
 * HDIB         - identifies the device-dependent bitmap
 *
 * Description:
 *
 * This function copies the specified part(s) of the window to a device-
 * dependent bitmap.
 *
 * History:   Date      Author              Reason
 *            9/15/91   Patrick Schreiber   Created
 *            9/25/91   Patrick Schreiber   Added header and comments
 *
 ************************************************************************/


HBITMAP FAR CopyWindowToBitmap(HWND hWnd, WORD fPrintArea)
{
   HBITMAP hBitmap = NULL;  // handle to device-dependent bitmap

   /* check for a valid window handle */

   if (!hWnd)
      return NULL;
   switch (fPrintArea)
      {
   case PW_WINDOW: // copy entire window
   {
      RECT rectWnd;

      /* get the window rectangle */

      GetWindowRect(hWnd, &rectWnd);

      /*  get the bitmap of that window by calling
       *  CopyScreenToBitmap and passing it the window rect
       */
      hBitmap = CopyScreenToBitmap(&rectWnd);
   }
   break;

   case PW_CLIENT: // copy client area
   {
      RECT rectClient;
      POINT pt1, pt2;

      /* get client dimensions */

      GetClientRect(hWnd, &rectClient);

      /* convert client coords to screen coords */
      pt1.x = rectClient.left;
      pt1.y = rectClient.top;
      pt2.x = rectClient.right;
      pt2.y = rectClient.bottom;
      ClientToScreen(hWnd, &pt1);
      ClientToScreen(hWnd, &pt2);
      rectClient.left = pt1.x;
      rectClient.top = pt1.y;
      rectClient.right = pt2.x;
      rectClient.bottom = pt2.y;

      /*  get the bitmap of the client area by calling
       *  CopyScreenToBitmap and passing it the client rect
       */
      hBitmap = CopyScreenToBitmap(&rectClient);
   }
   break;

   default:    // invalid print area
      return NULL;
      }

   /* return handle to the bitmap */
   return hBitmap;
}


/*************************************************************************
 *
 * CopyScreenToBitmap()
 *
 * Parameter:
 *
 * LPRECT lpRect    - specifies the window
 *
 * Return Value:
 *
 * HDIB             - identifies the device-dependent bitmap
 *
 * Description:
 *
 * This function copies the specified part of the screen to a device-
 * dependent bitmap.
 *
 * History:   Date      Author             Reason
 *            9/15/91   Patrick Schreiber  Created
 *            9/25/91   Patrick Schreiber  Added header and comments
 *
 ************************************************************************/


HBITMAP FAR CopyScreenToBitmap(LPRECT lpRect)
{
   HDC hScrDC, hMemDC;           // screen DC and memory DC
   HBITMAP hBitmap, hOldBitmap;  // handles to deice-dependent bitmaps
   int nX, nY, nX2, nY2;         // coordinates of rectangle to grab
   int nWidth, nHeight;          // DIB width and height
   int xScrn, yScrn;             // screen resolution

   /* check for an empty rectangle */

   if (IsRectEmpty(lpRect))
      return NULL;

   /*  create a DC for the screen and create
    *  a memory DC compatible to screen DC
    */
   hScrDC = CreateDC("DISPLAY", NULL, NULL, NULL);
   hMemDC = CreateCompatibleDC(hScrDC);

   /* get points of rectangle to grab */
   nX = lpRect->left;
   nY = lpRect->top;
   nX2 = lpRect->right;
   nY2 = lpRect->bottom;

   /* get screen resolution */
   xScrn = GetDeviceCaps(hScrDC, HORZRES);
   yScrn = GetDeviceCaps(hScrDC, VERTRES);

   /* make sure bitmap rectangle is visible */
   if (nX < 0)
      nX = 0;
   if (nY < 0)
      nY = 0;
   if (nX2 > xScrn)
      nX2 = xScrn;
   if (nY2 > yScrn)
      nY2 = yScrn;
   nWidth = nX2 - nX;
   nHeight = nY2 - nY;

   /* create a bitmap compatible with the screen DC */
   hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);

   /* select new bitmap into memory DC */
   hOldBitmap = SelectObject(hMemDC, hBitmap);

   /* bitblt screen DC to memory DC */
   BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, nX, nY, SRCCOPY);

   /*  select old bitmap back into memory DC and get handle to
    *  bitmap of the screen
    */
   hBitmap = SelectObject(hMemDC, hOldBitmap);

   /* clean up */
   DeleteDC(hScrDC);
   DeleteDC(hMemDC);

   /* return handle to the bitmap */
   return hBitmap;
}


/*************************************************************************
 *
 * PaintDIB()
 *
 * Parameters:
 *
 * HDC hDC          - DC to do output to
 *
 * LPRECT lpDCRect  - rectangle on DC to do output to
 *
 * HDIB hDIB        - handle to global memory with a DIB spec
 *                    in it followed by the DIB bits
 *
 * LPRECT lpDIBRect - rectangle of DIB to output into lpDCRect
 *
 * Return Value:
 *
 * BOOL             - TRUE if DIB was drawn, FALSE otherwise
 *
 * Description:
 *   Painting routine for a DIB.  Calls StretchDIBits() or
 *   SetDIBitsToDevice() to paint the DIB.  The DIB is
 *   output to the specified DC, at the coordinates given
 *   in lpDCRect.  The area of the DIB to be output is
 *   given by lpDIBRect.
 *
 * NOTE: This function always selects the palette as background. Before
 * calling this function, be sure your palette is selected to desired
 * priority (foreground or background).
 *
 * History:   
 *            
 *    Date      Author               Reason        
 *    6/1/91    Garrett McAuliffe    Created        
 *    12/12/91  Patrick Schreiber    Removed palette param, added get        
 *                                   palette stuff, return value, header
 *                                   and some comments
 *    6/8/92    Patrick Schreiber    Put palette param back in, select as
 *                                   background palette always. Added NOTE
 *                                   above.
 *
 ************************************************************************/

BOOL FAR PaintDIB(HDC      hDC,
                  LPRECT   lpDCRect,
                  HDIB     hDIB,
                  LPRECT   lpDIBRect,
                  HPALETTE hPal)
{
   LPSTR    lpDIBHdr;            // Pointer to BITMAPINFOHEADER
   LPSTR    lpDIBBits;           // Pointer to DIB bits
   BOOL     bSuccess=FALSE;      // Success/fail flag
   HPALETTE hOldPal=NULL;        // Previous palette

   /* Check for valid DIB handle */
   if (!hDIB)
      return FALSE;

   /* Lock down the DIB, and get a pointer to the beginning of the bit
    *  buffer
    */
   lpDIBHdr  = GlobalLock(hDIB);
   lpDIBBits = FindDIBBits(lpDIBHdr);

   /* Select and realize our palette as background */
   if (hPal)
   {
      hOldPal = SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
   }

   /* Make sure to use the stretching mode best for color pictures */
   SetStretchBltMode(hDC, COLORONCOLOR);

   /* Determine whether to call StretchDIBits() or SetDIBitsToDevice() */
   if ((RECTWIDTH(lpDCRect)  == RECTWIDTH(lpDIBRect)) &&
       (RECTHEIGHT(lpDCRect) == RECTHEIGHT(lpDIBRect)))
      bSuccess = SetDIBitsToDevice(hDC,                        // hDC
                                   lpDCRect->left,             // DestX
                                   lpDCRect->top,              // DestY
                                   RECTWIDTH(lpDCRect),        // nDestWidth
                                   RECTHEIGHT(lpDCRect),       // nDestHeight
                                   lpDIBRect->left,            // SrcX
                                   (int)DIBHeight(lpDIBHdr) -
                                      lpDIBRect->top - 
                                      RECTHEIGHT(lpDIBRect),   // SrcY
                                   0,                          // nStartScan
                                   (WORD)DIBHeight(lpDIBHdr),  // nNumScans
                                   lpDIBBits,                  // lpBits
                                   (LPBITMAPINFO)lpDIBHdr,     // lpBitsInfo
                                   DIB_RGB_COLORS);            // wUsage
   else 
      bSuccess = StretchDIBits(hDC,                            // hDC
                               lpDCRect->left,                 // DestX
                               lpDCRect->top,                  // DestY
                               RECTWIDTH(lpDCRect),            // nDestWidth
                               RECTHEIGHT(lpDCRect),           // nDestHeight
                               lpDIBRect->left,                // SrcX
                               lpDIBRect->top,                 // SrcY
                               RECTWIDTH(lpDIBRect),           // wSrcWidth
                               RECTHEIGHT(lpDIBRect),          // wSrcHeight
                               lpDIBBits,                      // lpBits
                               (LPBITMAPINFO)lpDIBHdr,         // lpBitsInfo
                               DIB_RGB_COLORS,                 // wUsage
                               SRCCOPY);                       // dwROP

   /* Unlock the memory block */
   GlobalUnlock(hDIB);

   /* Reselect old palette */
   if (hOldPal)
     SelectPalette(hDC, hOldPal, FALSE);

   /* Return with success/fail flag */
   return bSuccess;
}


/*************************************************************************
 *
 * PaintBitmap()
 *
 * Parameters:
 *
 * HDC hDC          - DC to do output to
 *
 * LPRECT lpDCRect  - rectangle on DC to do output to
 *
 * HBITMAP hDDB     - handle to device-dependent bitmap (DDB)
 *
 * LPRECT lpDDBRect - rectangle of DDB to output into lpDCRect
 *
 * HPALETTE hPalette - handle to the palette to use with hDDB
 *
 * Return Value:
 *
 * BOOL             - TRUE if bitmap was drawn, FLASE otherwise
 *
 * Description:
 *
 * Painting routine for a DDB.  Calls BitBlt() or
 * StretchBlt() to paint the DDB.  The DDB is
 * output to the specified DC, at the coordinates given
 * in lpDCRect.  The area of the DDB to be output is
 * given by lpDDBRect.  The specified palette is used.
 *
 * NOTE: This function always selects the palette as background. Before
 * calling this function, be sure your palette is selected to desired
 * priority (foreground or background).
 *
 * History:   
 *            
 *   Date      Author               Reason         
 *   6/1/91    Garrett McAuliffe    Created         
 *   12/12/91  Patrick Schreiber    Added return value, realizepalette,
 *                                       header and some comments
 *   6/8/92    Patrick Schreiber    Select palette as background always, added
 *                                  NOTE above.
 *
 ************************************************************************/

BOOL FAR PaintBitmap(HDC      hDC,
                     LPRECT   lpDCRect, 
                     HBITMAP  hDDB, 
                     LPRECT   lpDDBRect, 
                     HPALETTE hPal)
{
   HDC      hMemDC;            // Handle to memory DC
   HBITMAP  hOldBitmap;        // Handle to previous bitmap
   HPALETTE hOldPal1 = NULL;   // Handle to previous palette
   HPALETTE hOldPal2 = NULL;   // Handle to previous palette
   BOOL     bSuccess = FALSE;  // Success/fail flag

   /* Create a memory DC */
   hMemDC = CreateCompatibleDC (hDC);

   /* If this failed, return FALSE */
   if (!hMemDC)
      return FALSE;

   /* If we have a palette, select and realize it */
   if (hPal)
   {
      hOldPal1 = SelectPalette(hMemDC, hPal, TRUE);
      hOldPal2 = SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
   }

   /* Select bitmap into the memory DC */
   hOldBitmap = SelectObject (hMemDC, hDDB);

   /* Make sure to use the stretching mode best for color pictures */   
   SetStretchBltMode (hDC, COLORONCOLOR);

   /* Determine whether to call StretchBlt() or BitBlt() */ 
   if ((RECTWIDTH(lpDCRect)  == RECTWIDTH(lpDDBRect)) &&
       (RECTHEIGHT(lpDCRect) == RECTHEIGHT(lpDDBRect)))
      bSuccess = BitBlt(hDC,
                        lpDCRect->left,
                        lpDCRect->top,
                        lpDCRect->right - lpDCRect->left,
                        lpDCRect->bottom - lpDCRect->top,
                        hMemDC,
                        lpDDBRect->left,
                        lpDDBRect->top,
                        SRCCOPY);
   else
      bSuccess = StretchBlt(hDC,
                            lpDCRect->left, 
                            lpDCRect->top, 
                            lpDCRect->right - lpDCRect->left,
                            lpDCRect->bottom - lpDCRect->top,
                            hMemDC,
                            lpDDBRect->left, 
                            lpDDBRect->top, 
                            lpDDBRect->right - lpDDBRect->left,
                            lpDDBRect->bottom - lpDDBRect->top,
                            SRCCOPY);

   /* Clean up */
   SelectObject(hMemDC, hOldBitmap);

   if (hOldPal1)
      SelectPalette (hMemDC, hOldPal1, FALSE);

   if (hOldPal2)
      SelectPalette (hDC, hOldPal2, FALSE);

   DeleteDC (hMemDC);

   /* Return with success/fail flag */
   return bSuccess;
}
