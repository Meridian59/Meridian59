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
    ::ReleaseDC(Wnd, (HDC)Handle);
  Handle = 0;
}

//
//
//
TScreenDC::TScreenDC()
:
  TWindowDC(0)
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

} // OWL namespace
/* ========================================================================== */

