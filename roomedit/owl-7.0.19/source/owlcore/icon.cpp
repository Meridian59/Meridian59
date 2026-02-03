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
  ShouldDelete = (instance != nullptr);
  CheckValid();
}

//
/// Creates an icon object from the given resource file.
//
TIcon::TIcon(HINSTANCE instance, const tstring& fileName, int index)
{
  Handle = TShell::ExtractIcon(instance, fileName.c_str(), index);

  if (reinterpret_cast<INT_PTR>(Handle) == 1)
    Handle = nullptr;
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
                        static_cast<const uint8 *>(andBits), static_cast<const uint8 *>(xorBits));
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
    ::DestroyIcon(static_cast<HICON>(Handle));
}

//
/// Retrieves information about this icon.
/// Throws TXGdi on failure. 
//
auto TIcon::GetInfo() const -> TInfo
{
  auto i = GetIconInfoEx();
  CHECK(i.fIcon != FALSE);
  i.szModName[MAX_PATH - 1] = _T('\0'); // Ensure termination.
  i.szResName[MAX_PATH - 1] = _T('\0'); // Ensure termination.
  return TInfo
  {
    TPoint{static_cast<int>(i.xHotspot), static_cast<int>(i.yHotspot)}, 
    TBitmap{i.hbmMask, AutoDelete}, // Takes ownership.
    TBitmap{i.hbmColor, AutoDelete}, // Takes ownership.
    i.wResID,
    tstring{&i.szModName[0]},
    tstring{&i.szResName[0]}
  };
}

//
/// Retrieves information about this icon.
/// Wrapper for the Windows API function of the same name. Throws TXGdi on failure. 
///
/// \note GetIconInfo creates bitmaps for the hbmMask and hbmColor members of ICONINFO. The calling
/// application must manage these bitmaps and delete them when they are no longer necessary (using
/// the Windows API function ::DeleteObject explicitly, or by passing ownership of the handles to
/// TBitmap with the parameter `autoDelete` set to TAutoDelete::AutoDelete).
///
/// \sa http://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-geticoninfo
//
auto TIcon::GetIconInfo() const -> ICONINFO
{
  auto i = ICONINFO{};
  const auto r = ::GetIconInfo(GetHandle(), &i);
  if (!r || i.fIcon == FALSE) throw TXGdi(IDS_GDIFAILURE, GetHandle());
  return i;
}

//
/// Retrieves information about this icon.
/// Wrapper for the Windows API function of the same name. Throws TXGdi on failure. 
///
/// \note GetIconInfoEx creates bitmaps for the hbmMask and hbmColor members of ICONINFOEX. The calling
/// application must manage these bitmaps and delete them when they are no longer necessary (using
/// the Windows API function ::DeleteObject explicitly, or by passing ownership of the handles to
/// TBitmap with the parameter `autoDelete` set to TAutoDelete::AutoDelete).
///
/// \sa http://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-geticoninfoexa
//
auto TIcon::GetIconInfoEx() const -> ICONINFOEX
{
  auto i = ICONINFOEX{sizeof(ICONINFOEX)};
  const auto r = ::GetIconInfoEx(GetHandle(), &i);
  if (!r || i.fIcon == FALSE) throw TXGdi(IDS_GDIFAILURE, GetHandle());
  return i;
}

} // OWL namespace
/* ========================================================================== */

