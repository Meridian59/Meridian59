//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TMetaFileDC, a Metafile DC encapsulation
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/dc.h>

namespace owl {

OWL_DIAGINFO;

//
/// Creates a device context for a Windows-format metafile. If the provided filename is NULL,
/// the Windows-format metafile is memory based and its contents are lost when it is destroyed.
/// \note This constructor is provided only for compatibility with Windows-format metafiles.  
/// Enhanced-format metafiles provide superior functionality and are recommended for new applications. 
/// To create an enhanced metafile, use one of the constructors that accept a TDC& as the first parameter.
//
TMetaFileDC::TMetaFileDC(LPCTSTR filename)
:
  TDC(),
  Enhanced(false)
{
  Handle = ::CreateMetaFile(filename);
  CheckValid();
}

//
/// Creates a device context for a Windows-format metafile. If the provided filename is empty,
/// the Windows-format metafile is memory based and its contents are lost when it is destroyed.
/// \note This constructor is provided only for compatibility with Windows-format metafiles.  
/// Enhanced-format metafiles provide superior functionality and are recommended for new applications. 
/// To create an enhanced metafile, use one of the constructors that accept a TDC& as the first parameter.
//
TMetaFileDC::TMetaFileDC(const tstring& filename)
:
  TDC(),
  Enhanced(false)
{
  Handle = ::CreateMetaFile(filename.empty() ? nullptr : filename.c_str());
  CheckValid();
}

//
/// Destroys the metafile device context. If the metafile is memory-based, its contents will be lost.
//
TMetaFileDC::~TMetaFileDC()
{
  if (Handle)
  {
    if (ShouldDelete)
    {
      // Do not restore objects, Metafiles cleanup after themselves
      //
      if (IsEnhanced())
        ::DeleteEnhMetaFile(CloseEnh());
      else
        ::DeleteMetaFile(Close());
    }
    Handle = nullptr;
  }
}

//
/// Creates a device context for an enhanced-format metafile.
/// \note The given description string pointer, if not null, should point to two consecutive
/// null-terminated strings.
//
TMetaFileDC::TMetaFileDC(const TDC& dc, LPCTSTR filename, TRect* rect,
                         LPCTSTR description)
:
  TDC(),
  Enhanced(true)
{
  Handle = ::CreateEnhMetaFile(dc, filename, rect, description);
  CheckValid();
}

//
/// String-aware overload
//
TMetaFileDC::TMetaFileDC(const TDC& dc, const tstring& filename, const TRect& r, const tstring& appName, const tstring& picName)
  : TDC(),
  Enhanced(true)
{
  tstring description = appName + _T('\0') + picName + _T('\0');
  Handle = ::CreateEnhMetaFile(
    dc,
    filename.empty() ? nullptr : filename.c_str(),
    &r,
    description.c_str());
  CheckValid();
}

} // OWL namespace
/* ========================================================================== */

