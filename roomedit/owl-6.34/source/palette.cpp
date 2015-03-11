//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TPalette, an encapsulation of the GDI Palette object
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/gdiobjec.h>
#include <owl/clipboar.h>
#include <owl/file.h>
#include <owl/filename.h>
#include <memory.h>


#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGDI);        // General GDI diagnostic group
DIAG_DECLARE_GROUP(OwlGDIOrphan);  // Orphan control tracing group

//
// Constructors
//

//
/// Alias an existing palette handle. Assume ownership if autoDelete says so
//
/// Creates a TPalette object and sets the Handle data member to the given borrowed
/// handle. The ShouldDelete data member defaults to false, ensuring that the
/// borrowed handle will not be deleted when the C++ object is destroyed.
//
TPalette::TPalette(HPALETTE handle, TAutoDelete autoDelete)
:
  TGdiObject(handle, autoDelete)
{
  if (ShouldDelete)
    RefAdd(Handle, Palette);
}

//
/// Creates a TPalette object with values taken from the given clipboard.
//
TPalette::TPalette(const TClipboard& clipboard)
:
  TGdiObject(clipboard.GetClipboardData(CF_PALETTE))
{
  WARNX(OwlGDI, !Handle, 0, "Cannot create TPalette from the clipboard");
  CheckValid();
  RefAdd(Handle, Palette);
  RefInc(Handle);
}

//
/// This public copy constructor creates a complete copy of the given palette object
/// as in TPalette myPalette = yourPalette;
//
/// Always performs full, deep object copy
//
TPalette::TPalette(const TPalette& src)
{
  uint16  nColors;
  src.GetObject(nColors);
  if (nColors) {
    LOGPALETTE* logPal = (LOGPALETTE*) new
      uint8[sizeof(LOGPALETTE)+(nColors-1)*sizeof(PALETTEENTRY)];

    logPal->palVersion = 0x300;  // !CQ check to see if we should use 0x400
    logPal->palNumEntries = nColors;
    src.GetPaletteEntries(0, nColors, logPal->palPalEntry);
    Handle = ::CreatePalette(logPal);
    delete[] logPal;
  }
  else
    Handle = 0;  // Force a failure

  WARNX(OwlGDI, !Handle, 0, "Cannot create palette from palette " <<
        uint(HPALETTE(src)));
  CheckValid();
  RefAdd(Handle, Palette);
}

//
/// Creates a TPalette object from the given LOGPALETTE.
//
TPalette::TPalette(const LOGPALETTE& logPalette)
{
  Handle = ::CreatePalette(&logPalette);
  WARNX(OwlGDI, !Handle, 0, "Cannot create palette from LOGPALETTE @" << static_cast<const void*>(&logPalette));
  CheckValid();
  RefAdd(Handle, Palette);
}

#if defined(OWL5_COMPAT)

//
/// Creates a TPalette object from the given LOGPALETTE.
/// This overload is deprecated. Use the overload that takes a reference instead.
//
TPalette::TPalette(const LOGPALETTE * logPalette)
{
  PRECONDITION(logPalette);
  Handle = ::CreatePalette(logPalette);
  WARNX(OwlGDI, !Handle, 0, "Cannot create palette from LOGPALETTE @" <<
        hex << uint32(LPVOID(logPalette)));
  CheckValid();
  RefAdd(Handle, Palette);
}

#endif

//
/// Creates a TPalette object with count entries from the given entries array.
//
TPalette::TPalette(const PALETTEENTRY* entries, int count)
{
  Init(entries, count);
}

//
// Initializes a TPalette object with count entries from the given entries array.
//
void TPalette::Init(const PALETTEENTRY* entries, int count)
{
  LOGPALETTE* logPal = (LOGPALETTE*)new uint8[
     sizeof(LOGPALETTE)+(count-1)*sizeof(PALETTEENTRY) ];

  logPal->palVersion  = 0x300;
  logPal->palNumEntries = (uint16)count;
  memcpy(logPal->palPalEntry, entries, count*sizeof(PALETTEENTRY));
  Handle = ::CreatePalette(logPal);
  delete[] logPal;

  WARNX(OwlGDI, !Handle, 0, "Cannot create palette from " << count <<
        "palette entries @" << hex << uint32(LPVOID(entries)));
  CheckValid();
  RefAdd(Handle, Palette);
}

//
/// Creates a TPalette object from the color table following the given BITMAPINFO
/// structure. This constructor works only for 2-color, 16-color, and 256-color
/// bitmaps. A handle with value 0 (zero) is returned for other bitmaps, including
/// 24-bit DIBs.
//
TPalette::TPalette(const BITMAPINFO& info, uint flags)
{
  Create(&info, flags);
}

#if defined(OWL5_COMPAT)

//
/// Creates a TPalette object from the color table following the given BITMAPINFO
/// structure. This constructor works only for 2-color, 16-color, and 256-color
/// bitmaps. A handle with value 0 (zero) is returned for other bitmaps, including
/// 24-bit DIBs.
/// This overload is deprecated. Use the overload that takes a reference instead.
//
TPalette::TPalette(const BITMAPINFO * info, uint flags)
{
  Create(info, flags);
}

#endif

//
/// Creates a TPalette object from the given DIB object. The flags argument
/// represents the values of the LOGPALETTE data structure used to create the
/// palette.
//
TPalette::TPalette(const TDib& dib, uint flags)
{
  Create(dib.GetInfo(), flags);
}

//
/// Read from file: *.dib,*.bmp,*.pal,*.aco,*.act
//
TPalette::TPalette(const tchar * fileName)
{
  PRECONDITION(fileName);
  Handle = 0;

  Read(fileName);

  WARNX(OwlGDI, !Handle, 0, "Cannot create palette from file: " << fileName);

  CheckValid(IDS_GDIFILEREADFAIL);
  ShouldDelete = true;
  RefAdd(Handle, Palette);
}

//
/// Accept a pointer to a BITMAPINFO structure and create a GDI logical
/// palette from the color table which follows it, for 2, 16 and 256 color
/// bitmaps. Fail for all others, including 24-bit DIB's
///
/// !CQ Don't fail for other formats!?
//
void
TPalette::Create(const BITMAPINFO * info, uint flags)
{
  PRECONDITION(info);
  const RGBQUAD * rgb = info->bmiColors;

  // if the ClrUsed field of the header is non-zero,
  // it means that we could have have a short color table.
  //
  uint16 nColors = uint16(info->bmiHeader.biClrUsed ?
           info->bmiHeader.biClrUsed :
           NColors(info->bmiHeader.biBitCount));

  if (nColors) {
    LOGPALETTE* logPal = (LOGPALETTE*)
       new uint8[sizeof(LOGPALETTE) + (nColors-1)*sizeof(PALETTEENTRY)];

    logPal->palVersion  = 0x300;      // Windows 3.0 version
    logPal->palNumEntries = nColors;
    for (uint16 n = 0; n < nColors; n++) {
      logPal->palPalEntry[n].peRed   = rgb[n].rgbRed;
      logPal->palPalEntry[n].peGreen = rgb[n].rgbGreen;
      logPal->palPalEntry[n].peBlue  = rgb[n].rgbBlue;
      logPal->palPalEntry[n].peFlags = (uint8)flags;
    }
    Handle = ::CreatePalette(logPal);
    delete[] logPal;
  }
  else
    Handle = 0;

  WARNX(OwlGDI, !Handle, 0, "Cannot create palette from bitmapinfo @" <<
        hex << uint32(LPVOID(info)));
  CheckValid();
  RefAdd(Handle, Palette);
}

//
/// Moves this palette to the target Clipboard argument. If a copy is to be put on
/// the Clipboard, use TPalette(myPalette).ToClipboard; to make a copy first. The
/// handle in the temporary copy of the object is moved to the clipboard.
/// ToClipboard sets ShouldDelete to false so that the object on the clipboard is
/// not deleted. The handle will still be available for examination.
//
void
TPalette::ToClipboard(TClipboard& clipboard)
{
  if (Handle) {
    clipboard.SetClipboardData(CF_PALETTE, Handle);
    ShouldDelete = false; // GDI object now owned by Clipboard
    RefRemove(Handle);
  }
}

//
/// Read palette from file
//
bool
TPalette::Read(const tchar * filename)
{
  TRiffFile file(filename);
  if (!file.IsOpen())
    return false;

  const tchar* lp = _tcsrchr(filename, _T('.'));
  if(lp){
    if(_tcsicmp(lp, _T(".pal"))==0)  // check extension
      return Read_PAL(file);
    else if(_tcsicmp(lp,_T(".aco"))==0)
      return Read_ACO(file);
    else if(_tcsicmp(lp,_T(".act"))==0)
      return Read_ACT(file);
    else if(_tcsicmp(lp,_T(".bmp"))==0)
      return Read_BMP(file);
    else if(_tcsicmp(lp,_T(".dib"))==0)
      return Read_BMP(file);
  }
  return false;
}

//
/// Write this palette into file: *.dib,*.bmp,*.pal,*.aco,*.act
//
bool
TPalette::Write(const tchar * filename)
{
  TRiffFile file(filename, TRiffFile::WriteOnly|TRiffFile::PermExclusive|TRiffFile::CreateAlways);
  if (!file.IsOpen())
    return false;

  bool bOk = false;
  const tchar* lp = _tcsrchr(filename, _T('.'));
  if(lp){
    if(_tcsicmp(lp, _T(".pal"))==0)  // check extension
      bOk = Write_PAL(file);
    else if(_tcsicmp(lp, _T(".aco"))==0)
      bOk = Write_ACO(file);
    else if(_tcsicmp(lp, _T(".act"))==0)
      bOk = Write_ACT(file);
    else if(_tcsicmp(lp, _T(".bmp"))==0)
      bOk = Write_BMP(file);
    else if(_tcsicmp(lp, _T(".dib"))==0)
      bOk = Write_BMP(file);
  }
  if(!bOk)
    TFileName(filename).Remove();

  return bOk;
}
//------------------------------------------------------------------------------
static TBinField PALHDR_Fields[] = {
  {varSHORT,2, 1},   /* uint16  Version;  */
  {varSHORT,2, 1},   /* uint16   Count;    */
  {varEnd,  0, 0},
};

//
/// read Microsoft *.pal format, return true if success
//
bool
TPalette::Read_PAL(TRiffFile& file)
{
  // Check whether it's a RIFF PAL file.
  TCkInfo  ckFile;
  ckFile.Type = owlFCC('P','A','L',' ');
  if(!file.Descent(ckFile, 0, TRiffFile::ffFindRiff)) {
    WARNX(OwlGDI, 1, 0, "TPalette::Read_PAL() Not a RIFF or PAL file");
    TXBadFormat::Raise();
    return false;
  }
  // Find the 'data' chunk.
  TCkInfo  ckChunk;
  ckChunk.CkId = owlFCC('d','a','t','a');
  if (!file.Descent(ckChunk, &ckFile, TRiffFile::ffFindChunk)) {
    WARNX(OwlGDI, 1, 0, "TPalette::Read_PAL() No data chunk in file");
    TXBadFormat::Raise();
    return false;
  }
  // Allocate some memory for the data chunk.
  int iSize = (int)ckChunk.Size;
  TAPointer<char> pdata(new char[iSize]);
  LOGPALETTE* pLogPal = (LOGPALETTE*)(char*)pdata;
  // Read the data chunk.
  if(!file.ReadStruct((char*)pLogPal, PALHDR_Fields, boLittle_Endian)){
    WARNX(OwlGDI, 1, 0, "TPalette::Read_PAL() Failed to PAL Colors, line" << __LINE__);
    return false;
  }
  uint count = (iSize - sizeof(uint16)*2);
  if (file.Read(pLogPal->palPalEntry, count) == TFILE_ERROR) {
    WARNX(OwlGDI, 1, 0, "TPalette::Read_PAL() Failed to PAL Colors, line" << __LINE__);
    return false;
  }
  // The data chunk should be a LOGPALETTE structure
  // that we can create a palette from.
  if (pLogPal->palVersion != 0x300) {
    WARNX(OwlGDI, 1, 0, "TPalette::Read_PAL(): Invalid version number");
    TXBadFormat::Raise();
    return false;
  }
  // Get the number of entries.
  if (pLogPal->palNumEntries <= 0){
    WARNX(OwlGDI, 1, 0, "TPalette::Read_PAL(): No colors in palette");
    TXBadFormat::Raise();
    return false;
  }
  Handle = ::CreatePalette(pLogPal);
  return true;
}

/// write Microsoft *.pal format
///
/// return true if success
bool
TPalette::Write_PAL(TRiffFile& file)
{
  // Create a RIFF chunk for a PAL file.
  TCkInfo  ckFile;
  ckFile.Size = 0; // Corrected later
  ckFile.Type = owlFCC('P','A','L',' ');
  if (!file.CreateChunk(ckFile, TRiffFile::cfCreateRiff)) {
    WARNX(OwlGDI, 1, 0, "TPalette::Write_PAL() Failed to create RIFF-PAL chunk");
    return false;
  }
  // Create the LOGPALETTE data which will become
  // the data chunk.
  uint16  nColors;
  GetObject(nColors);
  if (!nColors){
    WARNX(OwlGDI, 1, 0, "TPalette::Write_PAL() no colors in palette");
    return false;
  }
  int iSize = sizeof(LOGPALETTE)+(nColors-1)*sizeof(PALETTEENTRY);
  TAPointer<char> pdata(new char[iSize]);
  LOGPALETTE* logPal = (LOGPALETTE*)(char*)pdata;
  logPal->palVersion = 0x300;
  logPal->palNumEntries = nColors;
  GetPaletteEntries(0, nColors, logPal->palPalEntry);

  // create the data chunk.
  TCkInfo  ckData;
  ckData.Size = iSize;
  ckData.CkId = owlFCC('d','a','t','a');
  if (!file.CreateChunk(ckData)) {
    WARNX(OwlGDI, 1, 0, "TPalette::Write_PAL() Failed to create data chunk");
    return false;
  }
  // Write the data chunk.
  if(!file.Write((char*)pdata, iSize)) {
    WARNX(OwlGDI, 1, 0, "TPalette::Write_PAL() Failed to write data chunk");
    return false;
  }
  // Ascend from the data chunk which will correct the length.
  file.Ascent(ckData);
  // Ascend from the RIFF-PAL chunk.
  file.Ascent(ckFile);

  return true;
}
/* -------------------------------------------------------------------------- */
/// convert HSL to RGB
///
/// given h,s,l on [0..1],return r,g,b on [0..1]
void HSL_to_RGB(double& h, double& sl, double& l, double* r, double* g, double* b)
{
  double v;
  v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
  if (v <= 0)
    *r = *g = *b = 0.0;
  else {
    double m;
    double sv;
    int sextant;
    double fract, vsf, mid1, mid2;

    m = l + l - v;
    sv = (v - m ) / v;
    h *= 6.0;
    sextant = static_cast<int>(h); //JJH added static cast
    fract = h - sextant;
    vsf = v * sv * fract;
    mid1 = m + vsf;
    mid2 = v - vsf;
    switch (sextant) {
      case 0: *r = v; *g = mid1; *b = m; break;
      case 1: *r = mid2; *g = v; *b = m; break;
      case 2: *r = m; *g = v; *b = mid1; break;
      case 3: *r = m; *g = mid2; *b = v; break;
      case 4: *r = mid1; *g = m; *b = v; break;
      case 5: *r = v; *g = m; *b = mid2; break;
    }
  }
}
#include <pshpack1.h>
/// ACO header
struct ACO_Hdr{
  uint16 Version; ///< Version== 1
  uint16 Count;   ///< number of color
};
static TBinField ACO_HdrFields[] = {
  {varSHORT,2, 1},   /* uint16  Version;  */
  {varSHORT,2, 1},   /* uint16   Count;    */
  {varEnd,  0, 0},
};
/// ACO color structure
struct ACO_Color{
  uint16 clrSpace;  ///<  color space
  uint16 Color1;     ///<  color value1
  uint16 Color2;    ///<  color value2
  uint16 Color3;    ///<  color value3
  uint16 Color4;    ///<  color value4
};
static TBinField ACO_ColorFields[] = {
  {varSHORT,2, 1},   /* uint16 clrSpace; */
  {varSHORT,2, 1},   /* uint16 Color1;   */
  {varSHORT,2, 1},   /* uint16 Color2;   */
  {varSHORT,2, 1},   /* uint16 Color3;   */
  {varSHORT,2, 1},   /* uint16 Color4;   */
  {varEnd,  0, 0},
};
struct MLOGPALETTE { // lgpl
    WORD         palVersion;
    WORD         palNumEntries;
    PALETTEENTRY palPalEntry[256];
};
#include <poppack.h>

#if !defined(CMYK)
#define CMYK(c,m,y,k)       ((COLORREF)((((BYTE)(c)|((WORD)((BYTE)(m))<<8))|(((DWORD)(BYTE)(y))<<16))|(((DWORD)(BYTE)(k))<<24)))
/* Macros to retrieve CMYK values from a COLORREF */
#define GetCValue(cmyk)      ((BYTE)(cmyk))
#define GetMValue(cmyk)      ((BYTE)((cmyk)>> 8))
#define GetYValue(cmyk)      ((BYTE)((cmyk)>>16))
#define GetKValue(cmyk)      ((BYTE)((cmyk)>>24))
#endif

/* -------------------------------------------------------------------------- */
/// Read Adobe Photoshop *.ACO color table
///
/// return true if success
//
bool TPalette::Read_ACO(TRiffFile& file)
{
  ACO_Hdr hdr;
  // Read the Header.
  if(!file.ReadStruct(&hdr, ACO_HdrFields, boBig_Endian)){
    WARNX(OwlGDI, 1, 0, "TPalette::Read_ACO() Failed to read ACO header");
    return false;
  }
  TRACEX(OwlGDI, 1, "TPalette::Read_ACO() Data for header: hdr.Version = " << dec << hdr.Version <<", hdr.Count   = " << hdr.Count);
  if(hdr.Version != 1){ // equial to 1    ///////////////////////////////
    WARNX(OwlGDI, 1, 0, "TPalette::Read_ACO() Invalid version number");
    TXBadFormat::Raise();
    return false;
  }

  TAPointer<ACO_Color> clr(new ACO_Color[hdr.Count]);

  // Read the Colors.
  for(int i = 0; i < hdr.Count; i++){
    if(!file.ReadStruct((char*)&clr[i], ACO_ColorFields, boBig_Endian)){
      WARNX(OwlGDI, 1, 0, "TPalette::Read_ACO() Failed to ACO Colors, line" << __LINE__);
      return false;
    }
  }
#if defined(__TRACE)
  TRACEX(OwlGDI, 1, "TPalette::Read_ACO() Data for Colors:");
  for (int j= 0; j < hdr.Count; j++){
    TRACEX(OwlGDI, 1, "TPalette::Read_ACO() index("<< dec << j << "), clrSpace("
           << clr[j].clrSpace << "), Color1("<< clr[j].Color1<< "), Color2(" <<
           clr[j].Color2<< "), Color3("<< clr[j].Color3<<"), Color4("<<
           clr[j].Color4<< ")");
  }
#endif
  MLOGPALETTE LogPal;
  LogPal.palVersion = 0x300;
  LogPal.palNumEntries = 256;
  memset(&LogPal.palPalEntry, 0, sizeof(LogPal.palPalEntry));

  //////////////////////////////////////////////////////////////////////////////
  // copy color values from ACO_Clr to logical palette
  uint colors = 0;

  // only 256 colors use !!!!!!!!!!!!!!!!!!
  if(hdr.Count > 256)
    hdr.Count = 256;

  for (int k= 0; k < hdr.Count; k++){
    switch(clr[k].clrSpace){
      default:
        break;

      case 0:    // RGB
        LogPal.palPalEntry[k].peRed   = uint8(clr[k].Color1*255ul/65535u);
        LogPal.palPalEntry[k].peGreen = uint8(clr[k].Color2*255ul/65535u);
        LogPal.palPalEntry[k].peBlue   = uint8(clr[k].Color3*255ul/65535u);
        LogPal.palPalEntry[k].peFlags = 0;
        break;

      // not checked
      case 1: {   // HSB (HSL)          ?????????????????????????????????????
          double h,s,l,r,g,b;
          h = clr[k].Color1/65535.0;
          s = clr[k].Color2/65535.0;
          l = clr[k].Color3/65535.0;
          HSL_to_RGB(h, s, l, &r, &g, &b);

          LogPal.palPalEntry[k].peRed     = uint8(r*255u);
          LogPal.palPalEntry[k].peGreen   = uint8(g*255u);
          LogPal.palPalEntry[k].peBlue     = uint8(b*255u);
          LogPal.palPalEntry[k].peFlags   = 0;
        }
        break;

      // not checked
      case 2: {   // CMYK
          COLORREF clrRef = CMYK( uint8(clr[k].Color1*255ul/65535u),
                                  uint8(clr[k].Color2*255ul/65535u),
                                  uint8(clr[k].Color3*255ul/65535u),
                                  uint8(clr[k].Color4*255ul/65535u)
                                );
          LogPal.palPalEntry[k].peRed     = GetRValue(clrRef);
          LogPal.palPalEntry[k].peGreen   = GetGValue(clrRef);
          LogPal.palPalEntry[k].peBlue     = GetBValue(clrRef);
          LogPal.palPalEntry[k].peFlags   = 0;
        break;
      }

      // if I must do it also ??????????????????
      //case 7:   // LAB
      //  LogPal.palPalEntry[k].peRed     = uint8(clr[k].Color1*255ul/65535u);
      //  LogPal.palPalEntry[k].peGreen   = uint8(clr[k].Color2*255ul/65535u);
      //  LogPal.palPalEntry[k].peBlue     = uint8(clr[k].Color3*255ul/65535u);
      //  LogPal.palPalEntry[k].peFlags   = 0;
      //  break;

      // not checked
      case 8:    // GrayScale
        LogPal.palPalEntry[k].peRed     = uint8(clr[k].Color1*255ul/10000u);
        LogPal.palPalEntry[k].peGreen   = uint8(clr[k].Color1*255ul/10000u);
        LogPal.palPalEntry[k].peBlue     = uint8(clr[k].Color1*255ul/10000u);
        LogPal.palPalEntry[k].peFlags   = 0;
        break;
    }
    colors++;
  }
  if(!colors){
    WARNX(OwlGDI, 1, 0, "TPalette::Read_ACO() Error no colors found.");
    //TXGdi::Raise(IDS_BADFORMAT);
    return false;
  }
  Handle = ::CreatePalette((LOGPALETTE*)&LogPal);
  return true;
}
/* -------------------------------------------------------------------------- */
/// Write Adobe Photoshop *.ACO color table
///
/// return true if success
bool TPalette::Write_ACO(TRiffFile& file)
{
  // Create the LOGPALETTE data which will become
  // the data chunk.
  uint16  nColors;
  GetObject(nColors);
  if (!nColors){
    WARNX(OwlGDI, 1, 0, "TPalette::Write_ACO() Error no colors in palette");
    TXBadFormat::Raise();
    return false;
  }
  TAPointer<PALETTEENTRY> pColors(new PALETTEENTRY[nColors]);
  GetPaletteEntries(0, nColors, (PALETTEENTRY*)pColors);

  ACO_Hdr hdr;
  hdr.Version = 0x1;
  hdr.Count   = nColors;

  // Write the Header.
  if(!file.WriteStruct(&hdr, ACO_HdrFields, boBig_Endian)){
    WARNX(OwlGDI, 1, 0, "TPalette::Write_ACO() Failed to write ACO header");
    return false;
  }

  ACO_Color clr[256];
  for (int i= 0; i < nColors; i++){
    clr[i].clrSpace = 0; // RGB
    clr[i].Color1  = uint16(pColors[i].peRed*65535ul/255);
    clr[i].Color2 = uint16(pColors[i].peGreen * 65535ul/255);
    clr[i].Color3 = uint16(pColors[i].peBlue * 65535ul/255);
    clr[i].Color4 = 0;
  }
  // Read the Colors.
  for(int j = 0; j < nColors; j++){
    if(!file.WriteStruct((char*)&clr[j], ACO_ColorFields, boBig_Endian)){
      WARNX(OwlGDI, 1, 0, "TPalette::Write_ACO() Failed to write ACO Colors, line" << __LINE__);
      return false;
    }
  }
  return true;
}
// -----------------------------------------------------------------------------
/// ACT color structure
struct ACT_Color{
  uint8 Red;    ///< Red compponent
  uint8 Green;  ///< Green compponent
  uint8 Blue;   ///< Blue compponent
};
// -----------------------------------------------------------------------------
/// Read Adobe Photoshop *.ACT color table
///
/// return true if success
bool TPalette::Read_ACT(TRiffFile& file)
{
  MLOGPALETTE LogPal;
  LogPal.palVersion = 0x300;
  LogPal.palNumEntries = 256;
  memset(&LogPal.palPalEntry, 0, sizeof(LogPal.palPalEntry));

  ACT_Color clr[256];

  // Read the Color table.
  if(file.Read((char*)&clr, sizeof(clr)) == TFILE_ERROR){
    WARNX(OwlGDI, 1, 0, "TPalette::Read_ACT() Failed to read ACT colors");
    return false;
  }
  for (int i= 0; i < 256; i++){
    LogPal.palPalEntry[i].peRed     = clr[i].Red;
    LogPal.palPalEntry[i].peGreen   = clr[i].Green;
    LogPal.palPalEntry[i].peBlue     = clr[i].Blue;
    LogPal.palPalEntry[i].peFlags   = 0;
  }
  Handle = ::CreatePalette((LOGPALETTE*)&LogPal);
  return true;
}
/* -------------------------------------------------------------------------- */
/// Read Adobe Photoshop *.ACT color table
///
/// return true if success
bool TPalette::Write_ACT(TRiffFile& file)
{
  // Create the LOGPALETTE data which will become
  uint16  nColors;
  GetObject(nColors);
  if (!nColors){
    WARNX(OwlGDI, 1, 0, "TPalette::Write_ACT() Error no colors in palette");
    return false;
  }
  TAPointer<PALETTEENTRY> pColors(new PALETTEENTRY[nColors]);
  GetPaletteEntries(0, nColors, (PALETTEENTRY*)pColors);

  ACT_Color clr[256];
  int i = 0;
  for (; i < nColors; i++){
    clr[i].Red     = pColors[i].peRed;
    clr[i].Green   = pColors[i].peGreen;
    clr[i].Blue   = pColors[i].peBlue;
  }
  for (; i < 256; i++){
    clr[i].Red     = 0;
    clr[i].Green   = 0;
    clr[i].Blue   = 0;
  }

  // Write the data
  if(!file.Write((char*)&clr, sizeof(clr))) {
    WARNX(OwlGDI, 1, 0, "TPalette::Write_ACT() Failed to write ACT data");
    return false;
  }
  return true;
}
/* -------------------------------------------------------------------------- */
/// Read color table from *.DIB
///
/// return true if success
bool TPalette::Read_BMP(TRiffFile& file)
{
  // Read the Header.
  BITMAPFILEHEADER bmf;
  // Read the Header.
  if(file.Read(&bmf, sizeof bmf) == TFILE_ERROR){
    WARNX(OwlGDI, 1, 0, "TPalette::Read_BMP() Failed to read BMP header");
    return false;
  }
  uint16 bmp_id = 0x4D42; // 'BM'
  if(bmf.bfType != bmp_id) {
    WARNX(OwlGDI, 1, 0, "TPalette::Read_BMP() Not a Windows 3.x or PM 1.x bitmap file");
    TXBadFormat::Raise();
    return false;
  }
  // Read bitmap header size & check it. It must be one of the two known header
  // sizes.
  // Will add BITMAPV4HEADER support when available
  uint32 headerSize;
  if (file.Read((char*)&headerSize, sizeof headerSize)==TFILE_ERROR
      || headerSize != sizeof(BITMAPCOREHEADER)
      && headerSize != sizeof(BITMAPINFOHEADER)) {
    WARNX(OwlGDI, 1, 0, "TPalette::Read_BMP() Not a Windows 3.x or PM 1.x bitmap file");
    TXBadFormat::Raise();
    return false;
  }
  uint16 BitCount, biPlanes;
  int ClrUsed;

  // If this is a PM 1.x DIB, read the core header & copy over to the Info header
  bool   isCore = headerSize == sizeof(BITMAPCOREHEADER);
  if (isCore) {
    // Read in the rest of the core header, aborting if it is truncated
    BITMAPCOREHEADER coreHeader;
    if (file.Read((char*)&coreHeader.bcWidth,
        (int)headerSize-sizeof(uint32))==TFILE_ERROR) {
      WARNX(OwlGDI, 1, 0, "TPalette::Read_BMP() Invalid PM 1.x DIB Header");
      TXBadFormat::Raise();
      return false;
    }
    BitCount = coreHeader.bcBitCount;
    biPlanes = coreHeader.bcPlanes;
    ClrUsed = 0;
  }
  else {
    BITMAPINFOHEADER infoHeader;
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    // Read in the rest of the info header, aborting if it is truncated
    if (file.Read((char*)&infoHeader.biWidth,
         (int)headerSize-sizeof(uint32))==TFILE_ERROR){
      WARNX(OwlGDI, 1, 0, "TPalette::Read_BMP() Invalid Win 3.x DIB Header");
      TXBadFormat::Raise();
      return false;
    }
    BitCount   = infoHeader.biBitCount;
    biPlanes   = infoHeader.biPlanes;
    ClrUsed    = infoHeader.biClrUsed;
  }
  // Check number of planes. Windows supports only 1 plane DIBs
  if (biPlanes != 1) {
    WARNX(OwlGDI, 1, 0, "TPalette::Read_BMP() Invalid number of planes in DIB");
    TXBadFormat::Raise();
    return false;
  }

  // Fill in the default value for biClrsUsed, if not supplied, using the
  // bit count. Will remain 0 for 16bpp or greater.
  if (!ClrUsed)
    ClrUsed = NColors(BitCount);

  // Read color table directly into allocated memory
  // Walk backwards & expand to RGBQUADs if it is a PM Core DIB
  int colorRead = isCore ?          // Color table size on disk
        (int)ClrUsed * sizeof(RGBTRIPLE) :
        (int)ClrUsed * sizeof(RGBQUAD);

  RGBQUAD Colors[256];
  if(ClrUsed){
    if (file.Read((char*)&Colors, colorRead)==TFILE_ERROR) {
      WARNX(OwlGDI, 1, 0, "TPalette::Read_BMP() Could not read DIB color table");
      return false;
    }
    if(isCore) {
      for (int i = int(ClrUsed-1); i >= 0; i--) {
        Colors[i].rgbRed   = ((RGBTRIPLE*)Colors)[i].rgbtRed;
        Colors[i].rgbGreen = ((RGBTRIPLE*)Colors)[i].rgbtGreen;
        Colors[i].rgbBlue  = ((RGBTRIPLE*)Colors)[i].rgbtBlue;
      }
    }
  }
  MLOGPALETTE LogPal;
  LogPal.palVersion = 0x300;
  LogPal.palNumEntries = 256;
  memset(&LogPal.palPalEntry, 0, sizeof(LogPal.palPalEntry));
  int i = 0;
  for(; i < ClrUsed; i++){
    LogPal.palPalEntry[i].peRed     = Colors[i].rgbRed;
    LogPal.palPalEntry[i].peGreen   = Colors[i].rgbGreen;
    LogPal.palPalEntry[i].peBlue     = Colors[i].rgbBlue;
    LogPal.palPalEntry[i].peFlags   = 0;
  }
  for(; i < 256; i++){
    LogPal.palPalEntry[i].peRed     = 0;
    LogPal.palPalEntry[i].peGreen   = 0;
    LogPal.palPalEntry[i].peBlue     = 0;
    LogPal.palPalEntry[i].peFlags   = 0;
  }
  Handle = ::CreatePalette((LOGPALETTE*)&LogPal);
  return true;
}
/* -------------------------------------------------------------------------- */
/// Write Color table to *.DIB with dimensions 1x1 pixel
///
/// return true if success
bool TPalette::Write_BMP(TRiffFile& file)
{
  BITMAPFILEHEADER bmf;
  bmf.bfType = 0x4D42; // 'BM'
  bmf.bfSize = sizeof(bmf) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256 + sizeof(uint8);
  bmf.bfReserved1 = 0;
  bmf.bfReserved2 = 0;
  bmf.bfOffBits = sizeof bmf + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256;
  if(!file.Write((char*)&bmf, sizeof bmf)){
    WARNX(OwlGDI, 1, 0, "TPalette::Write_BMP() Error write header");
    return false;
  }

  BITMAPINFOHEADER infoHeader;
  infoHeader.biSize = sizeof(BITMAPINFOHEADER);
  infoHeader.biWidth = 1;
  infoHeader.biHeight = 1;
  infoHeader.biPlanes = 1;
  infoHeader.biBitCount = 8;
  infoHeader.biCompression = BI_RGB;  // No compression
  infoHeader.biSizeImage = 1;         // Calculate this below
  infoHeader.biXPelsPerMeter = 0;     // Zero is OK
  infoHeader.biYPelsPerMeter = 0;     // Zero is OK
  infoHeader.biClrUsed = 256;           // Calculate this below
  infoHeader.biClrImportant = 256;      // Zero is OK

  // Write the data
  if(!file.Write((char*)&infoHeader, sizeof(infoHeader))){
    WARNX(OwlGDI, 1, 0, "TPalette::Write_BMP() Failed to write BMP Info header");
    return false;
  }

  uint16  nColors;
  GetObject(nColors);
  if (!nColors){
    WARNX(OwlGDI, 1, 0, "TPalette::Write_BMP() no colors in palette");
    return false;
  }
  TAPointer<PALETTEENTRY> pColors(new PALETTEENTRY[nColors]);
  GetPaletteEntries(0, nColors, (PALETTEENTRY*)pColors);

  RGBQUAD clr[256];
  int i = 0;
  for (; i < nColors; i++){
    clr[i].rgbRed       = pColors[i].peRed;
    clr[i].rgbGreen     = pColors[i].peGreen;
    clr[i].rgbBlue       = pColors[i].peBlue;
    clr[i].rgbReserved   = 0;
  }
  for (; i < 256; i++){
    clr[i].rgbRed       = 0;
    clr[i].rgbGreen     = 0;
    clr[i].rgbBlue       = 0;
    clr[i].rgbReserved   = 0;
  }

  // Write the Colors
  if(!file.Write((char*)&clr, sizeof(clr))) {
    WARNX(OwlGDI, 1, 0, "TPalette::Write_BMP() Failed to write BMP Colors");
    return false;
  }
  // Write the 1x1 bitmap
  uint32 pixel = 0;
  if(!file.Write((char*)&pixel, sizeof(pixel))) {
    WARNX(OwlGDI, 1, 0, "TPalette::Write_BMP() Failed to write BMP 1x1 bitmap");
    return false;
  }
  return true;
}

} // OWL namespace
