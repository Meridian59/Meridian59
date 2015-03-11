//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TIcon, a GDI Icon object encapsulation
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/gdiobjec.h>
#include <owl/shellitm.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGDI);        // General GDI diagnostic group

//
/// Creates a TIcon object and sets the Handle data member to the given borrowed
/// handle. The ShouldDelete data member defaults to false, ensuring that the
/// borrowed handle will not be deleted when the C++ object is destroyed.
//
TIcon::TIcon(HICON handle, TAutoDelete autoDelete)
:
  TGdiBase(handle, autoDelete)
{
}

//
/// Creates a copy of the given icon object.
//
TIcon::TIcon(HINSTANCE, const TIcon& icon)
{
  Handle = ::CopyIcon(icon);
  CheckValid();
}

//
/// Creates an icon object from the given resource.
//
TIcon::TIcon(HINSTANCE instance, TResId resId)
{
  Handle = ::LoadIcon(instance, resId);
  ShouldDelete = (instance != 0);
  CheckValid();
}

//
/// Creates an icon object from the given resource file.
//
TIcon::TIcon(HINSTANCE instance, const tstring& fileName, int index)
{
  Handle = TShell::ExtractIcon(instance, fileName.c_str(), index);

  if ((int)Handle == 1)
    Handle = 0;
  CheckValid();
}

//
/// Creates an icon object with the given values.
//
TIcon::TIcon(HINSTANCE instance, const TSize& size, int planes, int bitsPixel,
             const void * andBits, const void * xorBits)
{
  Handle = ::CreateIcon(instance, size.cx, size.cy,
                        uint8(planes), uint8(bitsPixel),
                        (const uint8 *)andBits, (const uint8 *)xorBits);
  CheckValid();
}

//
/// Creates an icon object of the given size from the bits found in the resBits
/// buffer.
//
TIcon::TIcon(const void* resBits, uint32 resSize)
{
  Handle = CreateIconFromResource((PBYTE)resBits, resSize, true, 0x00030000);
  CheckValid();
}

//
/// Creates an icon object with the given ICONINFO information.
//
TIcon::TIcon(const ICONINFO& iconInfo)
{
  WARN(!iconInfo.fIcon, "TIcon constructor called with ICONINFO::fIcon == false"); // Turn this into a precondition?
  ICONINFO i = iconInfo; // Make a clone, since CreateIconIndirect is not const-correct.
  Handle = CreateIconIndirect(&i);
  CheckValid();
}

#if defined(OWL5_COMPAT)

//
/// Creates an icon object with the given ICONINFO information.
/// This overload is deprecated. Use the overload that takes a reference instead.
//
TIcon::TIcon(const ICONINFO* iconInfo)
{
  //IconInfo->fIcon = true;  // assume the user setup the struct OK
  Handle = CreateIconIndirect((PICONINFO)iconInfo);
  CheckValid();
}

#endif

//
/// Destroys the icon and frees any memory that the icon occupied.
//
TIcon::~TIcon()
{
  if (ShouldDelete && Handle)
    ::DestroyIcon((HICON)Handle);
}

//
/// Retrieves information about this icon and copies it into the given ICONINFO structure. 
/// Throws TXGdi on failure.
//
ICONINFO TIcon::GetIconInfo() const
{
  ICONINFO iconInfo;
	bool r = ::GetIconInfo(GetHandle(), &iconInfo);
  if (!r) throw TXGdi(IDS_GDIFAILURE, GetHandle());
  return iconInfo;
}

} // OWL namespace
/* ========================================================================== */

