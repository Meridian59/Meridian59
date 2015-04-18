//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TCursor, a GDI Cursor object class
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/module.h>

#include <owl/gdiobjec.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGDI);        // General GDI diagnostic group

//
/// Creates a TCursor object and sets the Handle data member to the given borrowed
/// handle. The ShouldDelete data member defaults to false, ensuring that the
/// borrowed handle will not be deleted when the C++ object is destroyed.
//
TCursor::TCursor(HCURSOR handle, TAutoDelete autoDelete)
:
  TGdiBase(handle, autoDelete)
{
  TRACEX(OwlGDI, OWL_CDLEVEL, "TCursor constructed @" << (void*)this <<
    " from handle " << uint(handle));
}

//
/// Creates a copy of the given cursor object.  The 32bit version (for compiling a
/// Win32 application) uses CopyIcon() and does a cast to get to HICON.
//
TCursor::TCursor(HINSTANCE, const TCursor& cursor)
{
  Handle = (HCURSOR)::CopyIcon((HICON)(HCURSOR)cursor);
  CheckValid();
  TRACEX(OwlGDI, OWL_CDLEVEL, "TCursor @" << (void*)this <<
    " copied from TCursor " << (void*)&cursor);
}

//
/// Constructs a cursor object from the specified resource ID.
//
TCursor::TCursor(HINSTANCE instance, TResId resId)
{
  PRECONDITION(resId);
  Handle = ::LoadCursor(instance, resId);
  if(!Handle && instance != GetGlobalModule().GetHandle()){ // default load from OWL DLL
    Handle = GetGlobalModule().LoadCursor(resId);
    instance = GetGlobalModule().GetHandle();
  }
  ShouldDelete = (instance != 0);
  CheckValid();
  TRACEX(OwlGDI, OWL_CDLEVEL, "TCursor @" << (void*)this <<
    " loaded from resource " << resId);
}

//
/// Constructs a TCursor object of the specified size at the specified point.
//
TCursor::TCursor(HINSTANCE instance, const TPoint& hotSpot, const TSize& size,
                 void * andBits, void * xorBits)
{
  PRECONDITION(andBits && xorBits);
  Handle = ::CreateCursor(instance, hotSpot.x, hotSpot.y, size.cx, size.cy,
                          andBits, xorBits);
  CheckValid();
  TRACEX(OwlGDI, OWL_CDLEVEL, "TCursor @" << (void*)this << " created from bits ");
}

//
/// Constructs a TCursor object from the specified resource.
//
TCursor::TCursor(const void* resBits, uint32 resSize)
{
  PRECONDITION(resBits && resSize);
  Handle = ::CreateIconFromResource((PBYTE)resBits, resSize, false, 0x00030000);
  CheckValid();
  TRACEX(OwlGDI, OWL_CDLEVEL, "TCursor @" << (void*)this <<
    " created from bits (32)");
}

//
/// Creates a TCursor object from the specified ICONINFO structure information.
//
TCursor::TCursor(const ICONINFO& iconInfo)
{
  WARN(iconInfo.fIcon, "TCursor constructor called with ICONINFO::fIcon == true"); // Turn this into a precondition?
  ICONINFO i = iconInfo; // Make a clone, since CreateIconIndirect is not const-correct.
  Handle = CreateIconIndirect(&i);
  CheckValid();
  TRACEX(OwlGDI, OWL_CDLEVEL, "TCursor constructed indirectly @" << static_cast<void*>(this));
}

#if defined(OWL5_COMPAT)

//
/// Creates a TCursor object from the specified ICONINFO structure information.
/// This overload is deprecated. Use the overload that takes a reference instead.
//
TCursor::TCursor(const ICONINFO* iconInfo)
{
  PRECONDITION(iconInfo);
  //iconInfo->fIcon = false;
  Handle = ::CreateIconIndirect((PICONINFO)iconInfo);
  CheckValid();
  TRACEX(OwlGDI, OWL_CDLEVEL, "TCursor constructed indirectly @" << (void*)this);
}

#endif

//
/// Destroys a TCursor object.
//
TCursor::~TCursor()
{
  if (ShouldDelete && Handle)
    ::DestroyCursor((HCURSOR)Handle);
  TRACEX(OwlGDI, OWL_CDLEVEL, "TCursor destructed @" << (void*)this);
}

//
/// Retrieves information about this cursor and copies it into the given ICONINFO structure. 
/// Throws TXGdi on failure.
//
ICONINFO TCursor::GetIconInfo() const
{
  ICONINFO iconInfo;
	bool r = ::GetIconInfo(GetHandle(), &iconInfo);
  if (!r) throw TXGdi(IDS_GDIFAILURE, GetHandle());
  return iconInfo;
}


} // OWL namespace
/* ========================================================================== */
