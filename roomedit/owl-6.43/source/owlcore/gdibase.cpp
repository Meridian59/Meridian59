//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TGdiBase, base abstract class for all GDI objects that
/// have Handles.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/gdibase.h>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;

//
// TGdiBase constructors
//
TGdiBase::TGdiBase()
:
  Handle(0),
  ShouldDelete(true)
{
  // Handle must be set by derived class
}

//
//
//
TGdiBase::TGdiBase(HANDLE handle, TAutoDelete autoDelete)
:
  Handle(handle),
  ShouldDelete(autoDelete==AutoDelete)
{
  PRECONDITION(handle);
}

#if !defined(BI_COMP_GNUC)
#pragma warn -par   // resId is never used in small model
#endif
//
//
//
void
TGdiBase::CheckValid(uint resId)
{
  _CheckValid(Handle, resId);
}

//
//
//
void
TGdiBase::_CheckValid(HANDLE handle, uint resId)
{
  if (!handle)
    TXGdi::Raise(resId);
}
#if !defined(BI_COMP_GNUC)
#pragma warn .par
#endif

//----------------------------------------------------------------------------

//
/// Converts the resource ID to a string and returns the string message.
/// \note For backward compatibility
//
tstring
TXGdi::Msg(uint resId, HANDLE handle)
{
  return MakeMessage(resId, static_cast<uint>(reinterpret_cast<UINT_PTR>(handle)));
}

//
/// Constructs a TXGdi object with a default IDS_GDIFAILURE message.
//
TXGdi::TXGdi(uint resId, HANDLE handle)
:
  TXOwl(Msg(resId, handle), resId)
{
}

//
// Clone the exception object for safe-throwing across Windows.
//
TXGdi*
TXGdi::Clone()
{
  return new TXGdi(*this);
}


//
/// Throws the exception object. Throw must be implemented in any class derived from
/// TXOwl.
//
void
TXGdi::Throw()
{
  throw *this;
}

//
/// Throws the exception.
//
void
TXGdi::Raise(uint resId, HANDLE handle)
{
  TXGdi(resId, handle).Throw();
}

} // OWL namespace
/* ========================================================================== */

