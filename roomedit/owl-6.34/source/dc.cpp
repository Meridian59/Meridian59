//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TDC
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/dc.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGDI);        // General GDI diagnostic group

//
/// Sets OrgBrush, OrgPen, OrgFont, OrgBitmap, and OrgPalette to 0, and sets
/// ShouldDelete to true. This function is for internal use by the TDC constructors.
//
void
TDC::Init()
{
  OrgBrush = 0;
  OrgPen = 0;
  OrgFont = 0;
  OrgPalette = 0;
  OrgBitmap = 0;
  OrgTextBrush = 0;
}

//
/// Creates a DC object "borrowing" the handle of an existing DC. The Handle data
/// member is set to the given handle argument.
//
TDC::TDC(HDC handle)
:
  TGdiBase(handle, NoAutoDelete)
{
  Init();
  TRACEX(OwlGDI, OWL_CDLEVEL, _T("TDC constructed @") << (void*)this <<
    _T(" from handle") << uint(handle));
}

//
// Following two ctors are for use by derived classes only

/// Constructor for use by derived classes only
//
TDC::TDC()
{
  Init();
  TRACEX(OwlGDI, OWL_CDLEVEL, _T("TDC constructed @") << (void*)this);
}

//
/// Constructor for use by derived classes only
//
TDC::TDC(HDC handle, TAutoDelete autoDelete)
:
  TGdiBase(handle, autoDelete)
{
  Init();
  TRACEX(OwlGDI, OWL_CDLEVEL, _T("TDC constructed @") << (void*)this <<
    _T(" from handle") << uint(handle));
}

//
/// Default dtor does not delete Handle
/// Calls RestoreObjects.
//
TDC::~TDC()
{
  RestoreObjects();
  TRACEX(OwlGDI, OWL_CDLEVEL, _T("TDC destructed @") << (void*)this);
}

//
/// Returns the attributes of the DC object.
//
HDC
TDC::GetAttributeHDC() const
{
  return HDC(Handle);
}

//
/// Selects the given GDI pen object into this DC. The previously selected pen is saved in
/// the protected data member OrgPen, and can be restored by a call to RestorePen.
//
void
TDC::SelectObject(const TPen& pen)
{
  TRACEX(OwlGDI, 1, _T("TDC::SelectPen @") << (void*)this <<
    _T(" pen @") << (void*)&pen);
  HPEN oldPen = (HPEN)::SelectObject(GetHDC(), pen);
  if (oldPen) {
    TGdiObject::RefInc(pen);
    if (uint(oldPen) > 1)
      if (!OrgPen)
        OrgPen = oldPen;
      else
        TGdiObject::RefDec(oldPen, false);
  }
}

//
/// Selects the given GDI brush object into this DC. The previously selected brush is saved in
/// the protected data member OrgBrush, and can be restored by a call to RestoreBrush.
//
void
TDC::SelectObject(const TBrush& brush)
{
  TRACEX(OwlGDI, 1, _T("TDC::SelectBrush @") << (void*)this <<
    _T(" brush @") << (void*)&brush);
  HBRUSH oldBrush = (HBRUSH)::SelectObject(GetHDC(), brush);
  if (oldBrush) {
    TGdiObject::RefInc(brush);
    if (uint(oldBrush) > 1)
      if (!OrgBrush)
        OrgBrush = oldBrush;
      else
        TGdiObject::RefDec(oldBrush, false);
  }
}

//
/// Selects the given GDI font object into this DC. The previously selected font is saved in
/// the protected data member OrgFont, and can be restored by a call to RestoreFont.
//
void
TDC::SelectObject(const TFont& font)
{
  TRACEX(OwlGDI, 1, _T("TDC::SelectFont @") << (void*)this <<
    _T(" font @") << (void*)&font);
  HFONT oldFont = (HFONT)::SelectObject(GetHDC(), font);
  if (oldFont) {
    TGdiObject::RefInc(font);
    if (uint(oldFont) > 1)
      if (!OrgFont)
        OrgFont = oldFont;
      else
        TGdiObject::RefDec(oldFont, false);
  }
}

//
/// Selects the given GDI palette object into this DC. The previously selected palette is saved in
/// the protected data member OrgPalette, and can be restored by a call to RestorePalette.
/// 
/// If `forceBackgound` is set to `false` (the default), the selected logical palette is a
/// foreground palette when the window has input focus. If `forceBackground` is `true`, the 
/// selected palette is always a background palette, whether the window has focus or not.
//
void
TDC::SelectObject(const TPalette& palette, bool forceBackground)
{
  TRACEX(OwlGDI, 1, _T("TDC::SelectPalette @") << (void*)this <<
    _T(" palette @") << (void*)&palette);
  HPALETTE oldPalette = ::SelectPalette(GetHDC(), palette, forceBackground);
  if (oldPalette) {
    TGdiObject::RefInc(palette);
    if (uint(oldPalette) > 1)
      if (!OrgPalette)
        OrgPalette = oldPalette;
      else
        TGdiObject::RefDec(oldPalette, false);
  }
}

//
/// Selects the given GDI bitmap object into this DC. The previously selected bitmap is saved in
/// the protected data member OrgBitmap, and can be restored by a call to RestoreBitmap.
//
void
TDC::SelectObject(const TBitmap& bitmap)
{
  TRACEX(OwlGDI, 1, _T("TDC::SelectBitmap @") << (void*)this <<
    _T(" bitmap @") << (void*)&bitmap);
  HBITMAP oldBitmap = (HBITMAP)::SelectObject(GetHDC(), bitmap);
  if (oldBitmap) {
    TGdiObject::RefInc(bitmap);
    if (uint(oldBitmap) > 1)
      if (!OrgBitmap)
        OrgBitmap = oldBitmap;
      else
        TGdiObject::RefDec(oldBitmap, false);
  }
}

//
/// Selects into the DC a predefined stock pen, brush, font, or palette.
/// For more information about the available stock objects and their indexes, see MSDN:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/dd144925.aspx
//
void
TDC::SelectStockObject(int index)
{
  PRECONDITION(::GetStockObject(index));
  TRACEX(OwlGDI, 1, _T("TDC::SelectStockObject @") << (void*)this <<
    _T(" index ") << index);
  HANDLE oldObj = ::SelectObject(GetHDC(), ::GetStockObject(index));
  if (uint(oldObj) > 1)
    TGdiObject::RefDec(oldObj, false);
}

//
/// Restores the original GDI pen object to this DC.
//
void
TDC::RestorePen()
{
  TRACEX(OwlGDI, 1, _T("TDC::RestorePen @") << (void*)this);
  if (OrgPen) {
    TGdiObject::RefDec(::SelectObject(GetHDC(), OrgPen), false);
    OrgPen = 0;
  }
}

//
/// Restores the original GDI brush object to this DC.
//
void
TDC::RestoreBrush()
{
  TRACEX(OwlGDI, 1, _T("TDC::RestoreBrush @") << (void*)this);
  if (OrgBrush) {
    TGdiObject::RefDec(::SelectObject(GetHDC(), OrgBrush), false);
    OrgBrush = 0;
  }
}

//
/// Restores the original GDI font object to this DC.
//
void
TDC::RestoreFont()
{
  TRACEX(OwlGDI, 1, _T("TDC::RestoreFont @") << (void*)this);
  if (OrgFont) {
    TGdiObject::RefDec(::SelectObject(GetHDC(), OrgFont), false);
    OrgFont = 0;
  }
}

//
/// Restores the original GDI palette object to this DC.
//
void
TDC::RestorePalette()
{
  TRACEX(OwlGDI, 1, _T("TDC::RestorePalette @") << (void*)this);
  if (OrgPalette) {
    TGdiObject::RefDec(::SelectPalette(GetHDC(), OrgPalette, false), false);
    OrgPalette = 0;
  }
}

//
/// Restores the original GDI bitmap object into this DC.
//
void
TDC::RestoreBitmap()
{
  TRACEX(OwlGDI, 1, _T("TDC::RestoreBitmap @") << (void*)this);
  if (OrgBitmap) {
    TGdiObject::RefDec(::SelectObject(GetHDC(), OrgBitmap), false);
    OrgBitmap = 0;
  }
}

//
/// Restores the original GDI text brush object to this DC.
//
void
TDC::RestoreTextBrush()
{
  TRACEX(OwlGDI, 1, _T("TDC::RestoreTextBrush @") << (void*)this);
  if (OrgTextBrush) {
    TGdiObject::RefDec(::SelectObject(GetHDC(), OrgTextBrush), false);
    OrgTextBrush = 0;
  }
}

//
/// Restores all the original GDI objects to this DC.
//
void
TDC::RestoreObjects()
{
  if (Handle) {
    RestorePen();
    RestoreBrush();
    RestoreFont();
    RestorePalette();
    RestoreBitmap();
    RestoreTextBrush();
  }
}

//
/// Returns a handle to the currently selected object of the given objectType
/// associated with this DC. Returns 0 if the call fails. objectType can be OBJ_PEN,
/// OBJ_BRUSH, OBJ_PAL, OBJ_FONT, or OBJ_BITMAP.
//
/// Subset of Win32 GetCurrentObject , just a straight
/// call for normal win32
//
HANDLE
TDC::GetCurrentObject(uint objectType) const
{
    return ::GetCurrentObject(GetHDC(), objectType);
}

extern "C" bool  PASCAL FastWindowFrame(HDC, LPRECT, int xWidth, int yWidth, long rop);

// !CQ add ROP arg to allow this to be used by docking drag frame painting

//
/// Draws a frame of the specified size and thickness with the given brush. The old
/// brush is restored after completion.
//
void
TDC::OWLFastWindowFrame(TBrush& brush, TRect& r, int xThick, int yThick, uint32 rop)
{
  SelectObject(brush);

    int  width = r.Width() - xThick;
    int  height = r.Height() - yThick;

    PatBlt(r.left,        r.top, xThick, height, rop);  // left
    PatBlt(r.left+xThick, r.top, width, yThick, rop);   // top
    PatBlt(r.left, r.top+height, width, yThick, rop);  // bottom
    PatBlt(r.left+width,  r.top+yThick, xThick, height, rop);  // right

  RestoreBrush();
}

//
/// Saves the current state of this DC on a context stack. The saved state can be
/// restored later with RestoreDC. Returns a value specifying the saved DC or 0 if
/// the call fails.
//
int
TDC::SaveDC() const
{
  return ::SaveDC(GetHDC());
}

//
/// Restores the given savedDC. Returns true if the context is successfully
/// restored; otherwise, returns false.
//
bool
TDC::RestoreDC(int savedIndex)
{
  return ::RestoreDC(GetHDC(), savedIndex);
}

//
/// Used under WIN3.1 or later, GetDeviceCaps returns capability information about
/// this DC. The index argument specifies the type of information required.
/// \todo Document the possible values for the index
//
int
TDC::GetDeviceCaps(int index) const
{
  return ::GetDeviceCaps(GetAttributeHDC(), index);
}

//
/// Updates this DC using data in the given devMode structure. Returns true if the
/// call is successful; otherwise, returns false.
//
bool
TDC::ResetDC(DEVMODE & devMode)
{
  return ::ResetDC(GetHDC(), &devMode) != 0;
}

//
/// Sets the current background color of this DC to the given color value or the
/// nearest available. Returns 0x80000000 if the call fails.
//
TColor
TDC::SetBkColor(const TColor& color)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetBkColor(GetHDC(), color);
  return ::SetBkColor(GetAttributeHDC(), color);
}

//
/// Sets the current text color of this DC to the given color value. The text color
/// determines the color displayed by TDC::TextOut and TDC::ExtTextOut.
//
TColor
TDC::SetTextColor(const TColor& color)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetTextColor(GetHDC(), color);
  return ::SetTextColor(GetAttributeHDC(), color);
}

//
/// Sets the current window mapping mode of this DC to mode. Returns the previous
/// mapping mode value. The mapping mode defines how logical coordinates are mapped
/// to device coordinates. It also controls the orientation of the device's x- and
/// y-axes. See TDC::GetMapMode for a complete list of mapping modes.
//
int
TDC::SetMapMode(int mode)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetMapMode(GetHDC(), mode);
  return ::SetMapMode(GetAttributeHDC(), mode);
}

//
/// Sets this DC's viewport origin to the given origin value, and saves the previous
/// origin in oldOrg. Returns true if the call is successful; otherwise, returns
/// false.
//
bool
TDC::SetViewportOrg(const TPoint& point, TPoint * oldOrg)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetViewportOrgEx(GetHDC(), point.x, point.y, oldOrg);
  return ::SetViewportOrgEx(GetAttributeHDC(), point.x, point.y, oldOrg);
}

//
/// Modifies this DC's viewport origin relative to the current values. The delta x-
/// and y-components are added to the previous origin and the resulting point
/// becomes the new viewport origin. The previous origin is saved in oldOrg. Returns
/// true if the call is successful; otherwise, returns false.
//
bool
TDC::OffsetViewportOrg(const TPoint& delta, TPoint * oldOrg)
{
  if (GetHDC() != GetAttributeHDC())
    ::OffsetViewportOrgEx(GetHDC(), delta.x, delta.y, oldOrg);
  return ::OffsetViewportOrgEx(GetAttributeHDC(), delta.x, delta.y, oldOrg);
}

//
/// Sets this DC's viewport x- and y-extents to the given extent values. The
/// previous extents are saved in oldExtent. Returns true if the call is successful;
/// otherwise, returns false. The extent value determines the amount of stretching
/// or compression needed in the logical coordinate system to fit the device
/// coordinate system. extent also determines the relative orientation of the two
/// coordinate systems.
//
bool
TDC::SetViewportExt(const TSize& extent, TSize * oldExtent)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetViewportExtEx(GetHDC(), extent.cx, extent.cy, oldExtent);
  return ::SetViewportExtEx(GetAttributeHDC(), extent.cx, extent.cy, oldExtent);
}

//
/// Modifies this DC's viewport extents relative to the current values. The new
/// extents are derived as follows:
/// \code
/// xNewVE = (xOldVE * xNum)/ xDenom
/// yNewVE = (I * yNum)/ yDenom
/// \endcode
/// The previous extents are saved in oldExtent. Returns true if the call is
/// successful; otherwise, returns false.
//
bool
TDC::ScaleViewportExt(int xNum, int xDenom, int yNum, int yDenom,
                      TSize * oldExtent)
{
  if (GetHDC() != GetAttributeHDC())
    ::ScaleViewportExtEx(GetHDC(), xNum, xDenom, yNum, yDenom, oldExtent);
  return ::ScaleViewportExtEx(GetAttributeHDC(), xNum, xDenom, yNum, yDenom, oldExtent);
}

//
/// Sets the origin of the window associated with this DC to the given origin value,
/// and saves the previous origin in oldOrg. Returns true if the call is successful;
/// otherwise, returns false.
//
bool
TDC::SetWindowOrg(const TPoint& point, TPoint * oldOrg)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetWindowOrgEx(GetHDC(), point.x, point.y, oldOrg);
  return ::SetWindowOrgEx(GetAttributeHDC(), point.x, point.y, oldOrg);
}

//
/// Modifies this DC's window origin relative to the current values. The delta x-
/// and y-components are added to the previous origin and the resulting point
/// becomes the new window origin. The previous origin is saved in oldOrg. Returns
/// true if the call is successful; otherwise, returns false.
//
bool
TDC::OffsetWindowOrg(const TPoint& delta, TPoint * oldOrg)
{
  if (GetHDC() != GetAttributeHDC())
    ::OffsetWindowOrgEx(GetHDC(), delta.x, delta.y, oldOrg);
  return ::OffsetWindowOrgEx(GetAttributeHDC(), delta.x, delta.y, oldOrg);
}

//
/// Sets this DC's window x- and y-extents to the given extent values. The previous
/// extents are saved in oldExtent. Returns true if the call is successful;
/// otherwise, returns false. The extent value determines the amount of stretching
/// or compression needed in the logical coordinate system to fit the device
/// coordinate system. extent also determines the relative orientation of the two
/// coordinate systems.
//
bool
TDC::SetWindowExt(const TSize& extent, TSize * oldExtent)
{
  if (GetHDC() != GetAttributeHDC())
    ::SetWindowExtEx(GetHDC(), extent.cx, extent.cy, oldExtent);
  return ::SetWindowExtEx(GetAttributeHDC(), extent.cx, extent.cy, oldExtent);
}

//
/// Modifies this DC's window extents relative to the current values. The new
/// extents are derived as follows:
/// \code
/// xNewWE = (xOldWE * xNum)/ xDenom
/// yNewWE = (yOldWE * yNum)/ yDenom
/// \endcode
/// The previous extents are saved in oldExtent. Returns true if the call is
/// successful; otherwise, returns false.
//
bool
TDC::ScaleWindowExt(int xNum, int xDenom, int yNum, int yDenom, TSize * oldExtent)
{
  if (GetHDC() != GetAttributeHDC())
    ::ScaleWindowExtEx(GetHDC(), xNum, xDenom, yNum, yDenom, oldExtent);
  return ::ScaleWindowExtEx(GetAttributeHDC(), xNum, xDenom, yNum, yDenom, oldExtent);
}

//
/// Draws up to count characters of the given null-terminated string in the current
/// font on this DC. If count is -1 (the default), the entire string is written.
/// The (x, y) or p arguments specify the logical coordinates of the reference point
/// that is used to align the first character, depending on the current
/// text-alignment mode. This mode can be inspected with TDC::GetTextAlign and
/// changed with TDC::SetTextAlign. By default, the current position is neither used
/// nor altered by TextOut. However, the align mode can be set to TA_UPDATECP, which
/// makes Windows use and update the current position. In this mode, TextOut ignores
/// the reference point argument(s).
/// TextOut returns true if the call is successful; otherwise, it returns false.
//
bool
TDC::TextOut(int x, int y, const tstring& str, int count)
{
  return ::TextOut(GetHDC(), x, y, str.c_str(), count>=0 ? count : static_cast<int>(str.length()));
}

//
/// Draws up to count characters of the given null-terminated string in the current
/// font on this DC. If count is -1, the whole string is written.
/// An optional rectangle r can be specified for clipping, opaquing, or both, as
/// determined by the options value. If options is set to ETO_CLIPPED, the rectangle
/// is used for clipping the drawn text. If options is set to ETO_OPAQUE, the
/// current background color is used to fill the rectangle. Both options can be used
/// if ETO_CLIPPED is OR'd with ETO_OPAQUE.
/// The (x, y) orp arguments specify the logical coordinates of the reference point
/// that is used to align the first character. The current text-alignment mode can
/// be inspected with TDC::GetTextAlign and changed with TDC::SetTextAlign. By
/// default, the current position is neither used nor altered by ExtTextOut.
/// However, if the align mode is set to TA_UPDATECP, ExtTextOut ignores the
/// reference point argument(s) and uses or updates the current position as the
/// reference point.
/// The dx argument is an optional array of values used to set the distances between
/// the origins (upper left corners) of adjacent character cells. For example, dx[i]
/// represents the number of logical units separating the origins of character cells
/// i and i+1. If dx is 0, ExtTextOut uses the default inter-character spacings.
/// ExtTextOut returns true if the call is successful; otherwise, it returns false.
//
bool
TDC::ExtTextOut(int x, int y, uint16 options, const TRect* rect,
  const tstring& str, int count, const int * dx)
{
  PRECONDITION(count == 0 || str.length() > 0);
  return ::ExtTextOut(GetHDC(), x, y, options, rect, str.c_str(),
    count >= 0 ? count : static_cast<int>(str.length()), dx);
}

//
/// Draws up to count characters of the given null-terminated string in the current
/// font on this DC. If count is -1, the whole string is written.
/// Tabs are expanded according to the given arguments. The positions array
/// specifies numPositions tab stops given in device units. The tab stops must have
/// strictly increasing values in the array. If numPositions and positions are both
/// 0, tabs are expanded to eight times the average character width. If numPositions
/// is 1, all tab stops are taken to be positions[0] apart. tabOrigin specifies the
/// x-coordinate in logical units from which tab expansion will start.
/// The p argument specifies the logical coordinates of the reference point that is
/// used to align the first character depending on the current text-alignment mode.
/// This mode can be inspected with TDC::GetTextAlign and changed with
/// TDC::SetTextAlign. By default, the current position is neither used nor altered
/// by TabbedTextOut. However, if the align mode is set to TA_UPDATECP,
/// TabbedTextOut ignores the reference point argument(s) and uses/updates the
/// current position as the reference point.
/// The size argument in the second version of TabbedTextOut reports the dimensions
/// (size.y = height and size.x = width) of the string in logical units.
/// TabbedTextOut returns true if the call is successful; otherwise, it returns
/// false.
//
bool
TDC::TabbedTextOut(const TPoint& p, const tstring& str, int count,
  int numPositions, const int* positions, int tabOrigin, TSize& size) 
{
  LONG r = ::TabbedTextOut(GetHDC(), p.x, p.y, str.c_str(),
    count >= 0 ? count : static_cast<int>(str.length()), 
    numPositions, 
    const_cast<int*>(positions), // Cast for legacy compatibility.
    tabOrigin); 
  size = TSize(static_cast<DWORD>(r));
  return r != 0;
}

//
/// Overload for const TRect&
/// For obvious reasons, this overload does not support the DT_CALCRECT format flag.
/// If the given format contains the DT_CALCRECT flag, the function returns 0.
/// Otherwise, see the documentation for the overload for non-const TRect.
//
int
TDC::DrawText(const tstring& str, int count, const TRect& rect, uint16 format)
{
  if ((format & DT_CALCRECT) != 0) return 0;
  TRect r = rect;
  return DrawText(str, count, r, format); // Forward to virtual overload for non-const TRect.
}

//
/// Formats and draws in the given rectangle, r, up to count characters of the
/// null-terminated string using the current font for this DC. If count is -1, the
/// whole string is written. The rectangle must be specified in logical units.
/// Formatting is controlled with the format argument, which can be various
/// combinations of the following values:
/// - \c \b  DT_BOTTOM	Specifies bottom-justified text. This value must be combined (bitwise
/// OR'd) with DT_SINGLELINE.
/// - \c \b  DT_CALCRECT	Determines the width and height of the rectangle. If there are
/// multiple lines of text, DrawText uses the width of r (the rectangle argument)
/// and extends the base of the rectangle to bound the last line of text. If there
/// is only one line of text, DrawText uses a modified value for the right side of r
/// so that it bounds the last character in the line. In both cases, DrawText
/// returns the height of the formatted text but does not draw the text.
/// - \c \b  DT_CENTER	Centers text horizontally.
/// - \c \b  DT_EXPANDTABS	Expands tab characters. The default number of characters per tab
/// is eight.
/// - \c \b  DT_EXTERNALLEADING	Includes the font external leading in line height. Normally,
/// external leading is not included in the height of a line of text.
/// - \c \b  DT_LEFT	Aligns text flush-left.
/// - \c \b  DT_NOCLIP	Draws without clipping. DrawText is somewhat faster when DT_NOCLIP is
/// used.
/// - \c \b  DT_NOPREFIX	Turns off processing of prefix characters. Normally, DrawText
/// interprets the prefix character & as a directive to underscore the character
/// that follows, and the prefix characters && as a directive to print a single &.
/// By specifying DT_NOPREFIX, this processing is turned off.
/// - \c \b  DT_RIGHT	Aligns text flush-right.
/// - \c \b  DT_SINGLELINE	Specifies single line only. Carriage returns and linefeeds do not
/// break the line.
/// - \c \b  DT_TABSTOP	Sets tab stops. Bits 15-8 (the high-order byte of the low-order
/// word) of the format argument are the number of characters for each tab. The
/// default number of characters per tab is eight.
/// - \c \b  DT_TOP	Specifies top-justified text (single line only).
/// - \c \b  DT_VCENTER	Specifies vertically centered text (single line only).
/// - \c \b  DT_WORDBREAK	Specifies word breaking. Lines are automatically broken between
/// words if a word would extend past the edge of the rectangle specified by r. A
/// carriage return/line sequence will also break the line.
///
/// \note Note that the DT_CALCRECT, DT_EXTERNALLEADING, DT_INTERNAL, DT_NOCLIP, and
/// DT_NOPREFIX values cannot be used with the DT_TABSTOP value.
/// DrawText uses this DC's currently selected font, text color, and background
/// color to draw the text. Unless the DT_NOCLIP format is used, DrawText clips the
/// text so that it does not appear outside the given rectangle. All formatting is
/// assumed to have multiple lines unless the DT_SINGLELINE format is given.
/// If the selected font is too large for the specified rectangle, DrawText does not
/// attempt to substitute a smaller font.
/// If the function succeeds, the return value is the height of the text. If the
/// function fails, the return value is zero.
//
int
TDC::DrawText(const tstring& str, int count, TRect& rect, uint16 format)
{
  return ::DrawText(GetHDC(), str.c_str(), count, &rect, format);
}

//
/// Formats and draws in the given rectangle, r, up to count characters of the
/// null-terminated string using the current font for this DC. If count is -1, the
/// whole string is written. The rectangle must be specified in logical units.
/// Formatting is controlled with the format argument, which can be various
/// combinations of the following values:
/// - \c \b  DT_BOTTOM	Specifies bottom-justified text. This value must be combined (bitwise
/// OR'd) with DT_SINGLELINE.
/// - \c \b  DT_CALCRECT	Determines the width and height of the rectangle. If there are
/// multiple lines of text, DrawText uses the width of r (the rectangle argument)
/// and extends the base of the rectangle to bound the last line of text. If there
/// is only one line of text, DrawText uses a modified value for the right side of r
/// so that it bounds the last character in the line. In both cases, DrawText
/// returns the height of the formatted text but does not draw the text.
/// - \c \b  DT_CENTER	Centers text horizontally.
/// - \c \b  DT_EDITCONTROL	Duplicates the text-displaying characteristics of a multiline
/// edit control. Specifically, the average character width is calculated in the
/// same manner as for an edit control, and the function does not display a
/// partially visible last line.
/// - \c \b  DT_END_ELLIPSIS or DT_PATH_ELLIPSIS	Replaces part of the given string with
/// ellipses, if necessary, so that the result fits in the specified rectangle. The
/// given string is not modified unless the DT_MODIFYSTRING flag is specified.You
/// can specify DT_END_ELLIPSIS to replace characters at the end of the string, or
/// - \c \b  DT_PATH_ELLIPSIS to replace characters in the middle of the string. If the
/// string contains backslash (\) characters, DT_PATH_ELLIPSIS preserves as much as
/// possible of the text after the last backslash.
/// - \c \b  DT_EXPANDTABS	Expands tab characters. The default number of characters per tab
/// is eight.
/// - \c \b  DT_EXTERNALLEADING	Includes the font external leading in line height. Normally,
/// external leading is not included in the height of a line of text.
/// - \c \b  DT_INTERNAL	Uses the system font to calculate text metrics.
/// - \c \b  DT_LEFT	Aligns text flush-left.
/// - \c \b  DT_MODIFYSTRING	Modifies the given string to match the displayed text. This flag
/// has no effect unless the DT_END_ELLIPSIS or DT_PATH_ELLIPSIS flag is specified.
/// - \c \b  DT_NOCLIP	Draws without clipping. DrawText is somewhat faster when DT_NOCLIP is
/// used.
/// - \c \b  DT_NOPREFIX	Turns off processing of prefix characters. Normally, DrawText
/// interprets the prefix character & as a directive to underscore the character
/// that follows, and the prefix characters && as a directive to print a single &.
/// By specifying DT_NOPREFIX, this processing is turned off.
/// - \c \b  DT_RIGHT	Aligns text flush-right.
/// - \c \b  DT_RTLREADING	Layout in right to left reading order for bi-directional text when
/// the font selected into the hdc is a Hebrew or Arabic font. The default reading
/// order for all text is left to right.
/// - \c \b  DT_SINGLELINE	Specifies single line only. Carriage returns and linefeeds do not
/// break the line.
/// - \c \b  DT_TABSTOP	Sets tab stops. Bits 15-8 (the high-order byte of the low-order
/// word) of the format argument are the number of characters for each tab. The
/// default number of characters per tab is eight.
/// - \c \b  DT_TOP	Specifies top-justified text (single line only).
/// - \c \b  DT_VCENTER	Specifies vertically centered text (single line only).
/// - \c \b  DT_WORDBREAK	Specifies word breaking. Lines are automatically broken between
/// words if a word would extend past the edge of the rectangle specified by r. A
/// carriage return/line sequence will also break the line.
/// - \c \b  DT_WORD_ELLIPSIS		Truncates text that does not fit in the rectangle and adds
/// ellipses.
///
/// \note Note that the DT_CALCRECT, DT_EXTERNALLEADING, DT_INTERNAL, DT_NOCLIP, and
/// DT_NOPREFIX values cannot be used with the DT_TABSTOP value.
/// DrawText uses this DC's currently selected font, text color, and background
/// color to draw the text. Unless the DT_NOCLIP format is used, DrawText clips the
/// text so that it does not appear outside the given rectangle. All formatting is
/// assumed to have multiple lines unless the DT_SINGLELINE format is given.
/// If the selected font is too large for the specified rectangle, DrawText does not
/// attempt to substitute a smaller font.
/// If the function succeeds, the return value is the height of the text. If the
/// function fails, the return value is zero.
/// Windows NT: To get extended error information, call GetLastError.
//
int
TDC::DrawTextEx(LPTSTR str, int count, TRect* rect, uint format,
  LPDRAWTEXTPARAMS params)
{
  if (params)
    params->cbSize = sizeof(DRAWTEXTPARAMS);
  return ::DrawTextEx(GetHDC(), str, count, rect, format, params);
}

//
//
//
bool  //JBC
TDC::DrawFrameControl(TRect lpRect, UINT nType, UINT nState)
{ 
  RECT rect;
  rect.left = lpRect.Left();
  rect.top = lpRect.Top();
  rect.right = lpRect.Right();
  rect.bottom = lpRect.Bottom();
  return ::DrawFrameControl((HDC)Handle, &rect, nType, nState); 
}

//
/// Draws in the given rectangle (r) up to count characters of gray text from string
/// using the given brush, brush, and the current font for this DC. If count is -1
/// and string is null-terminated, the whole string is written. The rectangle must
/// be specified in logical units. If brush is 0, the text is grayed with the same
/// brush used to draw window text on this DC. Gray text is primarily used to
/// indicate disabled commands and menu items.
/// GrayString writes the selected text to a memory bitmap, grays the bitmap, then
/// displays the result. The graying is performed regardless of the current brush
/// and background color.
/// The outputFunc pointer to a function can specify the procedure instance of an
/// application-supplied drawing function and is defined as
/// \code
/// 	typedef BOOL (CALLBACK* GRAYSTRINGPROC)(HDC, LPARAM, int);
/// \endcode
/// If outputFunc is 0, GrayString uses TextOut and string is assumed to be a
/// normal, null-terminated character string. If string cannot be handled by TextOut
/// (for example, the string is stored as a bitmap), you must provide a suitable
/// drawing function via outputFunc.
/// If the device supports a solid gray color, it is possible to draw gray strings
/// directly without using GraySring. Call GetSysColor to find the color value; for
/// example, G of COLOR_GRAYTEXT. If G is nonzero (non-black), you can set the text
/// color with SetTextColor(G) and then use any convenient text-drawing function.
/// GrayString returns true if the call is successful; otherwise, it returns false.
/// Failure can result if TextOut or outputFunc return false, or if there is
/// insufficient memory to create the bitmap.
//
bool
TDC::GrayString(const TBrush& brush, GRAYSTRINGPROC outputFunc,
  const tstring& str, int count, const TRect& rect)
{
  return ::GrayString(GetHDC(), brush, outputFunc, reinterpret_cast<LPARAM>(str.c_str()),
    count >= 0 ? count : 0, rect.left, rect.top, rect.Width(), rect.Height());
}

//
// For use with CopyText.
//
struct TGetTextFace
{
  const TDC& dc;
  TGetTextFace(const TDC& d) : dc(d) {}

  int operator() (LPTSTR buf, int buf_size)
  {return dc.GetTextFace(buf_size - 1, buf);}
};

tstring 
TDC::GetTextFace() const
{
  return CopyText(GetTextFaceLength(), TGetTextFace(*this));
}

// DLN (stripped from TUIHandle)
// Draws an edge using system ::DrawEdge if available, otherwise
// does it the hard way
bool
TDC::DrawEdge(const TRect& frame, uint edge, uint flags)
{
  static int hasDrawEdge = true;

  // Try once to see if the API call is available. If not, do ourselves.
  //
  if (hasDrawEdge) {
    if (::DrawEdge(*this, (LPRECT)&frame, edge, flags))
      return true;
    if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
      hasDrawEdge = false;
    else
      return false;
  }

  // ::DrawEdge is not available, do the drawing ourselves
  //
  TRect f(frame);  // working frame rectangle

  // If mono is set, draw a thin, flat, black (windowFrame) frame
  //
  if (flags & Mono) 
    {
    if (edge & EdgeOuter) 
      {
// KSM      PaintFrame(*this, f, flags, TColor::SysWindowFrame, TColor::SysWindowFrame);
      PaintFrame(f, flags, TColor::SysWindowFrame, TColor::SysWindowFrame);

      f.Inflate(-1,-1);
      }
    if (flags & Fill) 
      { // !CQ repeated code--nest else?
      TBrush brsh(TColor::SysWindow);
      SelectObject(brsh);
      PatBlt(f);
      RestoreBrush();
      }
    return true;
    }

  // If flat is set, draw a thin, flat, shadow frame
  //
  if (flags & Flat) 
    {
    if (edge & EdgeOuter) 
      {
      PaintFrame(f, flags, TColor::Sys3dShadow, TColor::Sys3dShadow);
      f.Inflate(-1,-1);
      }
    if (flags & Fill) 
      { // !CQ repeated code--nest else?
      TBrush brsh(TColor::Sys3dFace);
      SelectObject(brsh);
      PatBlt(f);
      RestoreBrush();
      }
    return true;
    }

  // Draw outer edge if indicated, adjusting rect afterwards
  //
  if (edge & EdgeOuter) {
    static TColor tlColors[] = {
      TColor::Sys3dLight,       // EdgeRaised
      TColor::Sys3dHilight,     // EdgeRaised + Soft
      TColor::Sys3dShadow,      // EdgeSunken
      TColor::Sys3dDkShadow,    // EdgeSunken + Soft
    };
    static TColor brColors[] = {
      TColor::Sys3dDkShadow,    // EdgeRaised
      TColor::Sys3dDkShadow,    // EdgeRaised + Soft
      TColor::Sys3dHilight,     // EdgeSunken
      TColor::Sys3dHilight,     // EdgeSunken + Soft
    };
    int ci = ((edge & SunkenOuter) ? 2 : 0) | ((flags & Soft) ? 1 : 0);
//KSM    PaintFrame(*this, f, flags, tlColors[ci], brColors[ci]);
    PaintFrame(f, flags, tlColors[ci], brColors[ci]);
    f.Inflate(-1,-1);
  }

  // Draw inner edge if indicated, adjusting rect afterwards
  //
  if (edge & EdgeInner) {
    static TColor tlColors[] = {
      TColor::Sys3dHilight,     // EdgeRaised
      TColor::Sys3dLight,       // EdgeRaised + Soft
      TColor::Sys3dDkShadow,    // EdgeSunken
      TColor::Sys3dShadow,      // EdgeSunken + Soft
    };
    static TColor brColors[] = {
      TColor::Sys3dShadow,      // EdgeRaised
      TColor::Sys3dShadow,      // EdgeRaised + Soft
      TColor::Sys3dLight,       // EdgeSunken
      TColor::Sys3dLight,       // EdgeSunken + Soft
    };
    int ci = ((edge & SunkenOuter) ? 2 : 0) | ((flags & Soft) ? 1 : 0);
//KSM    PaintFrame(*this, f, flags, tlColors[ci], brColors[ci]);
    PaintFrame(f, flags, tlColors[ci], brColors[ci]);
    f.Inflate(-1,-1);
  }

  // Fill interior if indicated
  //
  if (flags & Fill) {
    TBrush brsh(TColor::Sys3dFace);
    SelectObject(brsh);
    PatBlt(f);
    RestoreBrush();
  }

// !CQ not really usefull since frame is not returned
//  if (flags & Adjust)
//    frame = f;

  return true;
}

// DLN ripped from TUIHandle
// Paint a 2-color single pixel thick frame, bevel corners get their own color
//
/*void
TDC::PaintFrame(const TRect& fr, uint flags, const TColor& tlColor, const TColor& brColor)
{
  if (flags & (Top | Left)) {
    TBrush brsh(tlColor);
    dc.SelectObject(brsh);
    if (flags & Top)
      dc.PatBlt(fr.left, fr.top, fr.Width()-1, 1);
    if (flags & Left)
      dc.PatBlt(fr.left, fr.top+1, 1, fr.Height()-2);
    dc.RestoreBrush();
  }

  if (flags & (Bottom | Right)) {
    TBrush brsh(brColor);
    dc.SelectObject(brsh);
    if (flags & Bottom)
      dc.PatBlt(fr.left, fr.bottom-1, fr.Width(), 1);
    if (flags & Right)
      dc.PatBlt(fr.right-1, fr.top, 1, fr.Height()-1);
    dc.RestoreBrush();
  }
}
*/
void
TDC::PaintFrame(const TRect& fr, uint flags, const TColor& tlColor, const TColor& brColor)
{
  if (flags & (Top | Left)) {
    TBrush brsh(tlColor);
    SelectObject(brsh);
    if (flags & Top)
      PatBlt(fr.left, fr.top, fr.Width()-1, 1);
    if (flags & Left)
      PatBlt(fr.left, fr.top+1, 1, fr.Height()-2);
    RestoreBrush();
  }

  if (flags & (Bottom | Right)) {
    TBrush brsh(brColor);
    SelectObject(brsh);
    if (flags & Bottom)
      PatBlt(fr.left, fr.bottom-1, fr.Width(), 1);
    if (flags & Right)
      PatBlt(fr.right-1, fr.top, 1, fr.Height()-1);
    RestoreBrush();
  }
}
} // OWL namespace
/* ========================================================================== */
