//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/uihelper.h>
#include <owl/gdiobjec.h>

namespace owl {

OWL_DIAGINFO;

//
/// Empty constructor.
//
TUIPart::TUIPart()
{
}

//
/// Draw the part onto a DC.
/// The type and state control how the part should be painted.
//
bool
TUIPart::Paint(TDC& dc, TRect& rect, TType type, TState state)
{
  return DrawFrameControl(dc, rect, type, state);
}

//
/// Wrapper for the DrawFrameControl API.
//
#if !defined(BI_COMP_GNUC)
#pragma warn -par
#endif
bool
TUIPart::DrawFrameControl(TDC& dc, TRect& rect, TType type, TState state)
{
  static bool hasDrawFrameControl = true;

  // Try once to see if the API call is available. If not, do ourselves.
  //
  if (hasDrawFrameControl) {
    if (::DrawFrameControl(dc, &rect, type, state))
      return true;
    if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
      hasDrawFrameControl = false;
    else
      return false;
  }

  // !BB Emulate call
  //
  //
  return false;
}
#if !defined(BI_COMP_GNUC)
#pragma warn .par
#endif

} // OWL namespace
/* ========================================================================== */

