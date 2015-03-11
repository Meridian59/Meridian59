//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TDiBitmap class, a combination of a GDI TBitmap object and
/// a Device Independent Bitmap TDib object.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/gdiobjec.h>
#include <owl/dibitmap.h>
#include <owl/pointer.h>
#include <owl/wsyscls.h>

using namespace std;

namespace owl {

OWL_DIAGINFO;

//-------------------------------------------------------------------------

//
/// Initializes our bitmap side from the DIB side. TDib::Bits gets setup to point to
/// the new DIBSection-managed DIB buffer.
//
void TDiBitmap::InitBitmap()
{
//  PRECONDITION(Bits == 0);

  // !CQ hacky!!! need to get a better dc...
  TScreenDC dc;
  TPointer<TPalette> p;
  if (NumColors() != 0)
    p = new TPalette(*(TDib*)this);
  else
    p = new TPalette((HPALETTE)::GetStockObject(DEFAULT_PALETTE));

  dc.SelectObject(*(TPalette*)p);
  dc.RealizePalette();
  void *bits;
  TBitmap::Handle = dc.CreateDIBSection(*GetInfo(), Usage(), &bits);
  memcpy(bits, Bits, SizeImage());
  Bits = bits;
//JJH - maybe this is bad, but this->CheckValid wasn't defined!
  TDib::CheckValid();
  TBitmap::RefAdd(TBitmap::Handle, Bitmap);
}


//    TWinGBitmap(TWinGIdentity&, TDibDC& dc, const TSize& size);
//    TWinGBitmap(TWinGIdentity&, TDibDC& dc, TDib& dib);
//    TWinGBitmap(TWinGIdentity&, TDibDC& dc, int width, int height);


//
/// Construct from existing bitmap handle.
//
TDiBitmap::TDiBitmap(HGLOBAL handle, TAutoDelete autoDelete)
:
  TDib(handle, autoDelete)
{
  InitBitmap();
}


//
/// Construct from bitmap stored in clipboard.
//
TDiBitmap::TDiBitmap(const TClipboard& clipboard)
:
  TDib(clipboard)
{
  InitBitmap();
}


//
/// Construct a copy from existing TDib.
//
TDiBitmap::TDiBitmap(const TDib& src)
:
  TDib(src)
{
  InitBitmap();
}


//
/// Construct new bitmap with the passed-in parameters.
//
TDiBitmap::TDiBitmap(int width, int height, int nColors, uint16 mode)
:
  TDib(width, height, nColors, mode)
{
  InitBitmap();
}


//
/// Construct a bitmap stored as resource.
//
TDiBitmap::TDiBitmap(HINSTANCE module, TResId resid)
:
  TDib(module, resid)
{
  InitBitmap();
}


//
/// Construct from DIB stored as a file.
//
TDiBitmap::TDiBitmap(LPCTSTR filename)
:
  TDib(filename)
{
  InitBitmap();
}

//
/// String-aware overload
//
TDiBitmap::TDiBitmap(const tstring& filename)
:
  TDib(filename)
{
  InitBitmap();
}


//
/// Construct from a TFile.
//
TDiBitmap::TDiBitmap(TFile& file, bool readFileHeader)
:
  TDib(file, readFileHeader)
{
  InitBitmap();
}

//
/// Construct from input stream.
//
TDiBitmap::TDiBitmap( istream& is, bool readFileHeader)
:
  TDib(is, readFileHeader)
{
  InitBitmap();
}


//
/// Construct from a bitmap and palette information.
//
TDiBitmap::TDiBitmap(const TBitmap& bitmap, const TPalette* pal)
:
  TDib(bitmap, pal)
{
  InitBitmap();
}


//
/// Restore Bits data member to 0.
//
TDiBitmap::~TDiBitmap()
{
  Bits = 0;  // Make sure that TDib doesn't mess with our unusual bits
}


/// Implementation of Identity Palette object, TIdentityPalette.
/// Use of an identity palette allows objects to be blitted without having
/// the color mapper come into play.  The overall effect is faster graphics.
//
/// Return an identity palette to be used by bitmaps.  This ensures
/// speed and color accuracy.
//
TIdentityPalette::TIdentityPalette(const TPalette& palette)
:
  TPalette(0, NoAutoDelete)
{
  TScreenDC sdc;

  int nColors = palette.GetNumEntries();
  int nSysColors = sdc.GetDeviceCaps(SIZEPALETTE);

  LOGPALETTE* logPal = (LOGPALETTE*)new uint8[
     sizeof(LOGPALETTE)+(nColors-1)*sizeof(PALETTEENTRY) ];
  logPal->palVersion  = 0x300;
  logPal->palNumEntries = (uint16)nColors;

  if (sdc.GetSystemPaletteUse() == SYSPAL_NOSTATIC) {
    int colorIndex;

    // Set the entries to PC_NOCOLLAPSE
    // !CQ First colors will get stomped. Need to offset & then fixup dib...
    //
    palette.GetPaletteEntries(0, uint16(nColors), logPal->palPalEntry);
    for (colorIndex=0; colorIndex < nColors; colorIndex++)
      logPal->palPalEntry[colorIndex].peFlags = PC_NOCOLLAPSE;

    // Make the first entry black
    //
    logPal->palPalEntry[0].peRed   = 0;
    logPal->palPalEntry[0].peGreen = 0;
    logPal->palPalEntry[0].peBlue  = 0;
    logPal->palPalEntry[0].peFlags = 0;

    // Make the last entry white
    //
    logPal->palPalEntry[255].peRed   = 255;
    logPal->palPalEntry[255].peGreen = 255;
    logPal->palPalEntry[255].peBlue  = 255;
    logPal->palPalEntry[255].peFlags = 0;
  }
  else {
    // SystemPaletteUse == SYSPAL_STATIC
    //
    int staticColors = sdc.GetDeviceCaps(NUMCOLORS) / 2;
    int usableColors = nColors - staticColors;

    ///TH The passed color table should be indexed from the start not from
    //    the offset of the systems static palette use.
    // !CQ is it really an identity palette then? More work needs to be done.
    //
    sdc.GetSystemPaletteEntries(0, staticColors, logPal->palPalEntry);
    palette.GetPaletteEntries(0, uint16(usableColors), logPal->palPalEntry+staticColors);
    sdc.GetSystemPaletteEntries(nSysColors - staticColors, staticColors,
                                logPal->palPalEntry + nSysColors - staticColors);

    int colorIndex = 0;
    for (; colorIndex < staticColors; colorIndex++)
      logPal->palPalEntry[colorIndex].peFlags = 0;
    for (; colorIndex < nColors; colorIndex++)
      logPal->palPalEntry[colorIndex].peFlags = PC_NOCOLLAPSE;
    for (; colorIndex < nSysColors; colorIndex++)
      logPal->palPalEntry[colorIndex].peFlags = 0;
  }
  Handle = ::CreatePalette(logPal);
  delete[] logPal;

  ShouldDelete = true;
  CheckValid();
  RefAdd(Handle, Palette);
}


//
/// Default constructor. Retrieves the system's halftone palette.
//
THalftonePalette::THalftonePalette()
:
  TPalette(::CreateHalftonePalette(TScreenDC()), NoAutoDelete)
{
  ShouldDelete = true;
  CheckValid();
  RefAdd(Handle, Palette);
}


} // OWL namespace

/* ========================================================================== */

