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
/// Creates a TMetaFileDC object with the data written to the named file if one is
/// provided.
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
/// String-aware overload.
//
TMetaFileDC::TMetaFileDC(const tstring& filename)
:
  TDC(),
  Enhanced(false)
{
  Handle = ::CreateMetaFile(filename.empty() ? 0 : filename.c_str());
  CheckValid();
}

//
// Destroy the metafile.
//
TMetaFileDC::~TMetaFileDC()
{
  if (Handle) {
    if (ShouldDelete)
      // Do not restore objects, Metafiles cleanup after themselves
      //
      if (IsEnhanced())
        ::DeleteEnhMetaFile(CloseEnh());
      else
        ::DeleteMetaFile(Close());
    Handle = 0;
  }
}

//
/// Provides access to a DC with a metafile selected into it for drawing into.
/// The given description string pointer, if not null, should point to two consequitive 
/// null-terminated strings
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
    filename.empty() ? 0 : filename.c_str(), 
    &r, 
    description.c_str());
  CheckValid();
}

} // OWL namespace
/* ========================================================================== */

