//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TFont, an encapsulation of the GDI Font object
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/gdiobjec.h>
#include <owl/uimetric.h>

#if !defined(DEFAULT_GUI_FONT)      // May not be defined by 16-bit headers
# define  DEFAULT_GUI_FONT  17
#endif

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGDI);          // General GDI diagnostic group
DIAG_DECLARE_GROUP(OwlGDIOrphan);    // Oprhan control tracing group

//
// Constructors
//

//
/// Creates a TFont object and sets the Handle data member to the given borrowed
/// handle. The ShouldDelete data member defaults to false, ensuring that the
/// borrowed handle will not be deleted when the C++ object is destroyed.
//
TFont::TFont(HFONT handle, TAutoDelete autoDelete)
:
  TGdiObject(handle, autoDelete)
{
  if (ShouldDelete)
    RefAdd(Handle, Font);
}

//
/// Creates a TFont object with the given values.
//
TFont::TFont(const tstring& facename,
             int height, int width, int escapement, int orientation,
             int weight,
             uint8 pitchAndFamily,
             uint8 italic, uint8 underline, uint8 strikeout, uint8 charSet,
             uint8 outputPrecision, uint8 clipPrecision, uint8 quality)
{
  Handle = ::CreateFont(height, width, escapement, orientation, weight,
                        italic, underline, strikeout, charSet,
                        outputPrecision, clipPrecision, quality,
                        pitchAndFamily, facename.length() > 0 ? facename.c_str() : 0);
  WARNX(OwlGDI, !Handle, 0, _T("Cannot create TFont (") << facename <<
        _T(" ") << height << _T("pt)"));
  CheckValid();
  RefAdd(Handle, Font);
}

//
/// Creates a TFont object with the given values. The constructor parameter list and
/// default values match the Windows API CreateFont call.
//
TFont::TFont(int height, int width, int escapement, int orientation,
             int weight,
             uint8 italic, uint8 underline, uint8 strikeout,
             uint8 charSet,
             uint8 outputPrecision,
             uint8 clipPrecision,
             uint8 quality,
             uint8 pitchAndFamily,
             const tstring& facename)
{
  Handle = ::CreateFont(height, width, escapement, orientation, weight,
                        italic, underline, strikeout, charSet,
                        outputPrecision, clipPrecision, quality,
                        pitchAndFamily, facename.length() > 0 ? facename.c_str() : 0);
  WARNX(OwlGDI, !Handle, 0, _T("Cannot create TFont (") << facename <<
        _T(" ") << height << _T("pt)"));
  CheckValid();
  RefAdd(Handle, Font);
}

//
/// Creates a TFont object from the given LOGFONT.
//
TFont::TFont(const LOGFONT& logFont)
{
  Handle = ::CreateFontIndirect(&logFont);
  WARNX(OwlGDI, !Handle, 0, "Cannot create TFont from logfont @" << static_cast<const void*>(&logFont));
  CheckValid();
  RefAdd(Handle, Font);
}

#if defined(OWL5_COMPAT)

//
/// Creates a TFont object from the given LOGFONT.
/// This overload is deprecated; use the overload for a LOGFONT reference instead.
//
TFont::TFont(const LOGFONT * logFont)
{
  PRECONDITION(logFont);
  Handle = ::CreateFontIndirect((LPLOGFONT)logFont);  // API typecast
  WARNX(OwlGDI, !Handle, 0, _T("Cannot create TFont from logfont @") <<
        hex << uint32(LPVOID(logFont)));
  CheckValid();
  RefAdd(Handle, Font);
}

#endif

//
/// The TFont copy constructor.
//
TFont::TFont(const TFont& src)
{
#if !defined(NO_GDI_SHARE_HANDLES)
  Handle = src.Handle;
  RefAdd(Handle, Font);
#else
  LOGFONT logFont;
  src.GetObject(logFont);
  Handle = ::CreateFontIndirect(&logFont);
  WARNX(OwlGDI, !Handle, 0, _T("Cannot create TFont from TFont @") <<
        hex << uint32(LPVOID(&src)));
  CheckValid();
  RefAdd(Handle, Font);
#endif
}

//
/// Retrieves information about this font when selected in the specified dc.
//
void
TFont::GetTextMetrics(TEXTMETRIC& tm, TDC& dc) const
{
  dc.SelectObject(*this);
  dc.GetTextMetrics(tm);
  dc.RestoreFont();
}

//
/// Retrieves information about this font when selected in the specified dc.
//
TEXTMETRIC
TFont::GetTextMetrics(TDC& dc) const
{
  TEXTMETRIC tm;
  GetTextMetrics(tm, dc);
  return tm;
}

//
/// Retrieves information about this font when selected in a screen DC.
//
void
TFont::GetTextMetrics(TEXTMETRIC& tm) const
{
  TScreenDC dc;
  GetTextMetrics(tm, dc);
}

//
/// Retrieves information about this font when selected in a screen DC.
//
TEXTMETRIC
TFont::GetTextMetrics() const
{
  TEXTMETRIC  tm;
  TScreenDC   dc;
  GetTextMetrics(tm, dc);
  return tm;
}

//
/// Returns the extent of a given string using this particular font.
//
TSize
TFont::GetTextExtent(const tstring& text) const
{
  TScreenDC dc;
  return GetTextExtent(dc, text);
}

//
/// Returns the extent of a given string using this particular font in a
/// particular DC.
//
TSize
TFont::GetTextExtent(TDC& dc, const tstring& text) const
{
  dc.SelectObject(*this);
  TSize size = dc.GetTextExtent(text, static_cast<int>(text.length())); // TODO: Widen parameter type to avoid this narrowing cast.
  dc.RestoreFont();
  return size;
}

//
/// Returns the font's name.
/// Throws TXGdi on failure.
//
tstring
TFont::GetFaceName() const
{
  return GetObject().lfFaceName;
}

//
/// Returns the pitch and family flags for this logical font.
/// Throws TXGdi on failure.
//
uint8
TFont::GetPitchAndFamily() const
{
  return GetObject().lfPitchAndFamily;
}

//
/// Returns information about this font object.
/// Throws TXGdi on failure.
//
LOGFONT 
TFont::GetObject() const
{
  LOGFONT lf;
	bool r = TGdiObject::GetObject(sizeof(lf), &lf) != 0;
  if (!r) throw TXGdi(IDS_GDIFAILURE, Handle);
  return lf;
}

//
// Create a wrapper for the default GUI font.
//
TDefaultGUIFont::TDefaultGUIFont()
:
  TFont(HFONT(::GetStockObject(DEFAULT_GUI_FONT)))
{
}

} // OWL namespace
/* ========================================================================== */

