//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TDibDC encapsulation class
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/dc.h>
#include <owl/gdiobjec.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGDI);        // General GDI diagnostic group

//
// Construct a DC that can be used with DI Bitmaps. Under Win32 bitmaps
// selected must be DIBSections
//
TDibDC::TDibDC()
:
  TMemoryDC()
{
  CheckValid();
}

//
/// Constructs a DC that can be used with DI Bitmaps, then auto select the given
/// bitmap in. Under Win32 bitmaps selected must be DIBSections
//
TDibDC::TDibDC(const TBitmap& bitmap)
:
  TMemoryDC()
{
  CheckValid();
  SelectObject(bitmap);
}


//
// Get the color table of the currently selected bitmap.
//
uint
TDibDC::GetDIBColorTable(uint start, uint count, RGBQUAD * colors)
{
  return ::GetDIBColorTable(GetHDC(), start, count, colors);
}

//
// Set the color table of the currently selected bitmap.
//
uint
TDibDC::SetDIBColorTable(uint start, uint count, const RGBQUAD * colors)
{
# if defined(__GNUC__)
  return ::SetDIBColorTable(GetHDC(), start, count, (RGBQUAD*)colors);
# else
  return ::SetDIBColorTable(GetHDC(), start, count, colors);
# endif //WINELIB
}


//
// Dib Screen Update BitBlt's. A screen DC must be the destination.
// BitBlts from this DIB onto the destination DC.
//
bool
TDibDC::BitBltToScreen(TDC& dstDC, const TRect& dst, const TPoint& src) const
{
  return dstDC.BitBlt(dst, *this, src);
}

//
// BitBlts from this DIB onto the destination DC.
//
bool
TDibDC::BitBltToScreen(TDC& dstDC, int dstX, int dstY, int dstW, int dstH,
                       int srcX, int srcY) const
{
  return dstDC.BitBlt(dstX, dstY, dstW, dstH, *this, srcX, srcY);
}

//
// Stretches the DIB onto the destination DC.
//
bool
TDibDC::StretchBltToScreen(TDC& dstDC, const TRect& dst, const TRect& src) const
{
  return dstDC.StretchBlt(dst, *this, src);
}

//
// Stretches the DIB onto the destination DC.
//
bool
TDibDC::StretchBltToScreen(TDC& dstDC, int dstX, int dstY, int dstW, int dstH,
                           int srcX, int srcY, int srcW, int srcH) const
{
  return dstDC.StretchBlt(dstX, dstY, dstW, dstH, *this, srcX, srcY, srcW, srcH);
}


} // OWL namespace

