//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TPaintDC, an encapsulation of client DCs used when
/// responding to paint messages
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/dc.h>

namespace owl {

OWL_DIAGINFO;

//
/// Creates a TPaintDC object with the given owned window. The data member Wnd is
/// set to wnd.
//
TPaintDC::TPaintDC(HWND hWnd)
:
  TDC()
{
  Wnd = hWnd;
  Handle = ::BeginPaint(hWnd, &Ps);
  CheckValid();
}

//
//
//
TPaintDC::~TPaintDC()
{
  RestoreObjects();
  if (ShouldDelete)
    ::EndPaint(Wnd, &Ps);
}

} // OWL namespace
/* ========================================================================== */

