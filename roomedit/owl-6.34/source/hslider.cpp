//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of THSlider, a horizontal slider UI widget
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/slider.h>
#include <owl/dc.h>
#include <owl/commctrl.h>
#include <owl/uihelper.h>

namespace owl {

OWL_DIAGINFO;

//
/// Constructs a slider object with a default bitmap resource ID of IDB_HSLIDERTHUMB
/// for the thumb knob.
//
THSlider::THSlider(TWindow* parent, int id, int x, int y, int w, int h, TResId thumbResId, TModule* module)
:
  TSlider(parent, id, x, y, w, h, thumbResId, module)
{
  if (!h)
    Attr.H = 32;
  Attr.Style |= TBS_HORZ;  // In case it is Native, else we dont care
}

//
/// Constructor for a slider object created from resource
//
THSlider::THSlider(TWindow* parent, int resId, TResId thumbResId, TModule* module)
: 
  TSlider(parent, resId, thumbResId, module)
{}

//
/// Constructs a slider object to encapsulate (alias) an existing control.
//
THSlider::THSlider(THandle hWnd, TModule* module)
:
  TSlider(hWnd, module)
{}



IMPLEMENT_STREAMABLE1(THSlider, TSlider);

#if !defined(BI_NO_OBJ_STREAMING)

//
//
//
void*
THSlider::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TSlider*)GetObject(), is);
  return GetObject();
}

//
//
//
void
THSlider::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TSlider*)GetObject(), os);
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */

