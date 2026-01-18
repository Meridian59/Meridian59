//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of classes TWindowDC, TScreenDC, TDesktopDC & TClientDC
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/dc.h>

namespace owl {

OWL_DIAGINFO;

//
/// Used for derived classes only.
//
TWindowDC::TWindowDC()
:
  TDC()
{
}

//
/// Creates a TWindow object with the given owned window. The data member Wnd is set
/// to wnd.
//
TWindowDC::TWindowDC(HWND hWnd)
:
  TDC(),
  Wnd(hWnd)
{
  Handle = ::GetWindowDC(Wnd);
  CheckValid();
}

//
//
//
TWindowDC::~TWindowDC()
{
  RestoreObjects();
  if (ShouldDelete)
    ::ReleaseDC(Wnd, static_cast<HDC>(Handle));
  Handle = nullptr;
}

//
//
//
TScreenDC::TScreenDC()
:
  TWindowDC(nullptr)
{
}

//
/// Default constructor for TDesktopDC objects.
//
TDesktopDC::TDesktopDC()
:
  TWindowDC(::GetDesktopWindow())
{
}

//
/// Creates a TClientDC object with the given owned window. The data member Wnd is
/// set to wnd.
//
TClientDC::TClientDC(HWND wnd)
:
  TWindowDC()
{
  Wnd = wnd;
  Handle = ::GetDC(Wnd);
  CheckValid();
}


//
/// Creates a TNCPaintDC object with the given owned window. The data member Wnd is
/// set to wnd. hPaintRgn is the WParam of WM_NCPAINT.
//
TNCPaintDC::TNCPaintDC(HWND wnd, HRGN hPaintRgn)
  :
  TWindowDC()
{
  Wnd = wnd;

  // hPaintRgn can be 0, 1 or handle to region. 0 means something noone says what it is, 1 means
  // entire area (undocumented), else, according to the documentation, we own hPaintRgn, and
  // according to the documentation of GetDCEx, if DCX_INTERSECTRGN is set, the OS owns the region
  // and will destroy it.
  //
  const auto dcxUseStyle = 0x10000u; // Undocumented style.
  const auto regionCode = reinterpret_cast<INT_PTR>(hPaintRgn);
  if (regionCode == 0 || regionCode == 1)
  {
    Handle = ::GetDCEx(wnd, nullptr, DCX_WINDOW | DCX_EXCLUDERGN | dcxUseStyle);
  }
  else
  {
    Handle = ::GetDCEx(wnd, hPaintRgn, DCX_WINDOW | DCX_INTERSECTRGN | dcxUseStyle);
  }
  CheckValid();
}


} // OWL namespace
/* ========================================================================== */

