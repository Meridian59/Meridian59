//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TModuleVersionInfo, version resource parser &
/// accessor class
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/module.h>
#include <stdio.h>
#include <owl/except.rh>

namespace owl {

OWL_DIAGINFO;

static tchar viRootStr[] =             _T("\\");
static tchar viTransStr[] =            _T("\\VarFileInfo\\Translation");
static tchar viSubBlockQueryFmt[] =    _T("\\StringFileInfo\\%08lx\\%s");
static tchar viFileDescStr[] =         _T("FileDescription");
static tchar viFileVersionStr[] =      _T("FileVersion");
static tchar viInternalNameStr[] =     _T("InternalName");
static tchar viLegalCopyrightStr[] =   _T("LegalCopyright");
static tchar viOriginalFilenameStr[] = _T("OriginalFilename");
static tchar viProductNameStr[] =      _T("ProductName");
static tchar viProductVersionStr[] =   _T("ProductVersion");
static tchar viSpecialBuildStr[] =     _T("SpecialBuild");

//
/// Constructs a TModuleVersionInfo given a module or instance handle
//
TModuleVersionInfo::TModuleVersionInfo(TModule::THandle hModule)
:
  Buff(0),
  Lang(0),
  FixedInfo(0)
{
  TTmpBuffer<tchar> modFName(_MAX_PATH);
  ::GetModuleFileName(hModule, modFName, _MAX_PATH);
#if !defined(UNICODE)
  ::OemToAnsi(modFName, modFName);
#endif

  Init(modFName);
}

//
/// Constructs a TModuleVersionInfo given a module filename
/// String version. Throws TXModuleVersionInfo on errors. [VH 2005-04-03]
//
TModuleVersionInfo::TModuleVersionInfo(const tstring& modFName)
:
  Buff(0),
  Lang(0),
  FixedInfo(0)
{
  Init(modFName.c_str ());
}

//
// Internal initialization fills in the data members
// Now returns void and throws exceptions. [VH 2005-04-03]
//
void
TModuleVersionInfo::Init(LPCTSTR modFName)
{
  try {
  uint32 fvHandle;
#if defined WINELIB
  uint32 infoSize = TVersion::GetFileVersionInfoSize((LPTSTR)modFName, (unsigned int *)&fvHandle);
#else
  uint32 infoSize = TVersion::GetFileVersionInfoSize((LPTSTR)modFName, &fvHandle);
#endif
    if (!infoSize) 
      throw 1; // Caught below.

    TAPointer<uint8> buf(new uint8[uint(infoSize)]);
    if (!TVersion::GetFileVersionInfo((LPTSTR)modFName, fvHandle, infoSize, buf))
      throw 2; // Caught below.

  // Copy string to buffer so if the -dc compiler switch (Put constant
  // strings in code segments) is on, VerQueryValue will work under Win16.
  // This works around a problem in Microsoft's ver.dll which writes to the
  // string pointed to by subBlockName.
  //
  uint vInfoLen;                 // not really used, but must be passed
  tchar* subBlockName = viRootStr;
    if (!TVersion::VerQueryValue(buf, subBlockName, (void **)&FixedInfo, &vInfoLen))
      throw 3; // Caught below.

  subBlockName = viTransStr;
  uint32 * trans;
    if (!TVersion::VerQueryValue(buf, subBlockName, (void **)&trans, &vInfoLen))
      throw 4; // Caught below.

    // Assign language and buffer.
    //
    // Swap the words so sprintf will print the lang-charset in the correct format.
  Lang = MkUint32(HiUint16(*trans), LoUint16(*trans)); 
    Buff = buf.Relinquish (); // Take ownership.
  }

  catch (...) {

    // Rethrow dedicated exception.
    //
    LPCTSTR msg = 0;
    TSystemMessage last_error;
    if (last_error.SysError()) 
      msg = last_error.SysMessage ().c_str();
    TXModuleVersionInfo::Raise(msg);
  }
}

//
// Clean up the new'd buffer
//
TModuleVersionInfo::~TModuleVersionInfo()
{
  delete[] Buff;
}

//
/// Queries any given "\\StringFileInfo\\lang-charset\\<str>" version. Info string lang
/// indicates the language translation, may be 0 to signify file default.
//
bool
TModuleVersionInfo::GetInfoString(LPCTSTR str, LPCTSTR& value, uint lang)
{
  PRECONDITION(Buff);
  uint    vSize;
  TTmpBuffer<tchar> subBlockName(MAX_PATH);
  _stprintf(subBlockName, viSubBlockQueryFmt, lang ? uint32(lang) : Lang, str);
  return TVersion::VerQueryValue(Buff, subBlockName, (void**)&value, &vSize);
}

//
// Commonly used, predefined info string queries. Pass requested language thru,
// may be 0 to signify default.
//

//
/// Retrieves the file description information in the requested language id.
//
bool TModuleVersionInfo::GetFileDescription(LPCTSTR& fileDesc, uint lang)
{
  return GetInfoString(viFileDescStr, fileDesc, lang);
}

//
/// Retrieves the file version information in the requested language id.
//
bool
TModuleVersionInfo::GetFileVersion(LPCTSTR& fileVersion, uint lang)
{
  return GetInfoString(viFileVersionStr, fileVersion, lang);
}

//
/// Retrieves the internal name of the module.
//
bool
TModuleVersionInfo::GetInternalName(LPCTSTR& internalName, uint lang)
{
  return GetInfoString(viInternalNameStr, internalName, lang);
}

//
/// Retrieves the copyright message.
//
bool
TModuleVersionInfo::GetLegalCopyright(LPCTSTR& copyright, uint lang)
{
  return GetInfoString(viLegalCopyrightStr, copyright, lang);
}

//
/// Retrieves the original filename.
//
bool
TModuleVersionInfo::GetOriginalFilename(LPCTSTR& originalFilename, uint lang)
{
  return GetInfoString(viOriginalFilenameStr, originalFilename, lang);
}

//
/// Retrieves the product name this module is associated with.
//
bool
TModuleVersionInfo::GetProductName(LPCTSTR& prodName, uint lang)
{
  return GetInfoString(viProductNameStr, prodName, lang);
}

//
/// Retrieves the version of the product.
//
bool
TModuleVersionInfo::GetProductVersion(LPCTSTR& prodVersion, uint lang)
{
  return GetInfoString(viProductVersionStr, prodVersion, lang);
}

//
/// Retrieves the special build number.
//
bool
TModuleVersionInfo::GetSpecialBuild(LPCTSTR& specialBuild, uint lang)
{
  // !CQ could check VS_FF_SPECIALBUILD flag first...
  return GetInfoString(viSpecialBuildStr, specialBuild, lang);
}

//
/// Local error handler for string query methods. [VH 2005-04-03]
//
static struct TEH 
  {
  bool operator=(bool r) 
    {if (!r) TXModuleVersionInfo().Throw (); return r;}
  }
  E;

//
// Query any given "\StringFileInfo\lang-charset\<str>" version info string.
// lang indicates the language translation, may be 0 to signify file default.
// String version. Throws exception on failure. [VH 2005-04-03]
//
tstring TModuleVersionInfo::GetInfoString(const tstring& str, uint lang)
  {LPCTSTR v = _T(""); E = GetInfoString(str.c_str (), v, lang); return v;}

//
// Commonly used, predefined info string queries. Pass requested language thru,
// may be 0 to signify default.

/// Retrieves the file description information in the requested language id.
/// String version. [VH 2005-04-03]
//
tstring TModuleVersionInfo::GetFileDescription(uint lang)
  {LPCTSTR v = _T(""); E = GetFileDescription(v, lang); return v;}

//
/// Retrieves the file version information in the requested language id.
/// String version. [VH 2005-04-03]
//
tstring TModuleVersionInfo::GetFileVersion(uint lang)
  {LPCTSTR v = _T(""); E = GetFileVersion(v, lang); return v;}

//
/// Retrieves the internal name of the module.
/// String version. [VH 2005-04-03]
//
tstring TModuleVersionInfo::GetInternalName(uint lang)
  {LPCTSTR v = _T(""); E = GetInternalName(v, lang); return v;}

//
/// Retrieves the copyright message.
/// String version. [VH 2005-04-03]
//
tstring TModuleVersionInfo::GetLegalCopyright(uint lang)
  {LPCTSTR v = _T(""); E = GetLegalCopyright(v, lang); return v;}

//
/// Retrieves the original filename.
/// String version. [VH 2005-04-03]
//
tstring TModuleVersionInfo::GetOriginalFilename(uint lang)
  {LPCTSTR v = _T(""); E = GetOriginalFilename(v, lang); return v;}

//
/// Retrieves the product name this module is associated with.
/// String version. [VH 2005-04-03]
//
tstring TModuleVersionInfo::GetProductName(uint lang)
  {LPCTSTR v = _T(""); E = GetProductName(v, lang); return v;}

//
/// Retrieves the version of the product.
/// String version. [VH 2005-04-03]
//
tstring TModuleVersionInfo::GetProductVersion(uint lang)
  {LPCTSTR v = _T(""); E = GetProductVersion(v, lang); return v;}

//
/// Retrieves the special build number.
/// String version. [VH 2005-04-03]
//
tstring TModuleVersionInfo::GetSpecialBuild(uint lang)
  {LPCTSTR v = _T(""); E = GetSpecialBuild(v, lang); return v;}

//
/// Gets the language name string associated with a language/charset code
//
tstring
TModuleVersionInfo::GetLanguageName(uint lang)
{
  TTmpBuffer<tchar> langStr(128);
  TVersion::VerLanguageName(uint(lang), langStr, 128);

  return tstring((tchar*)langStr);
}

//
/// Returns values of TFileType if GetFileType() returned DevDriver or Font.
/// If GetFileType() returned VirtDriver, this function returns the virtual
/// device identifier included in the virtual device control block.
//
uint32 TModuleVersionInfo::GetFileSubType() const
{
  PRECONDITION(FixedInfo);
  uint32 fileSubType = FixedInfo->dwFileSubtype;

  switch (GetFileType()) {
    case TypeUnknown:
    case App:
    case DLL:
      break;
    case DevDriver:
      switch (FixedInfo->dwFileSubtype) {
        case VFT2_UNKNOWN:
          fileSubType = UnknownDevDriver;
          break;
        case VFT2_DRV_PRINTER:
          fileSubType = PtrDriver;
          break;
        case VFT2_DRV_KEYBOARD:
          fileSubType = KybdDriver;
          break;
        case VFT2_DRV_LANGUAGE:
          fileSubType = LangDriver;
          break;
        case VFT2_DRV_DISPLAY:
          fileSubType = DisplayDriver;
          break;
        case VFT2_DRV_MOUSE:
          fileSubType = MouseDriver;
          break;
        case VFT2_DRV_NETWORK:
          fileSubType = NtwkDriver;
          break;
        case VFT2_DRV_SYSTEM:
          fileSubType = SysDriver;
          break;
        case VFT2_DRV_INSTALLABLE:
          fileSubType = InstallableDriver;
          break;
        default: //case VFT2_DRV_SOUND:
          fileSubType = SoundDriver;
      }
      break;
    case Font:
      switch (FixedInfo->dwFileSubtype) {
        case VFT2_UNKNOWN:
          fileSubType = UnknownFont;
          break;
        case VFT2_FONT_RASTER:
          fileSubType = RasterFont;
          break;
        case VFT2_FONT_VECTOR:
          fileSubType = VectorFont;
          break;
        default: //case VFT2_FONT_TRUETYPE:
          fileSubType = TrueTypeFont;
      }
      break;
    case VirtDevice:
    case StaticLib:
      break;
  }
  return fileSubType;
}

//
/// Retrieves the file date.
//
FILETIME TModuleVersionInfo::GetFileDate() const
{
  PRECONDITION(FixedInfo);
  FILETIME ft = {FixedInfo->dwFileDateLS, FixedInfo->dwFileDateMS};
  return ft;
}


//----------------------------------------------------------------------------
//
// Exception class [VH 2005-04-03]
//

//
/// Create the exception.
//
TXModuleVersionInfo::TXModuleVersionInfo(const tstring& name)
:
  TXOwl(MakeMessage(IDS_INVALIDMODULE, name))
{
}

//
/// Creates a copy of the exception 
//
TXModuleVersionInfo*
TXModuleVersionInfo::Clone() const
{
  return new TXModuleVersionInfo(*this);
}


//
/// Throws the exception
//
void
TXModuleVersionInfo::Throw()
{
  throw *this;
}

//
/// Throws the exception
//
void
TXModuleVersionInfo::Raise(const tstring& name)
{
  TXModuleVersionInfo(name).Throw();
}


} // OWL namespace
/* ========================================================================== */

