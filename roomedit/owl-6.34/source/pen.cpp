//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TPen, an encapsulation of the GDI Pen object
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/gdiobjec.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGDI);        // General GDI diagnostic group
DIAG_DECLARE_GROUP(OwlGDIOrphan);  // Orphan control tracing group

//
// Constructors
//

//
/// Alias an existing pen handle. Assume ownership if autoDelete says so
//
/// Creates a TPen object and sets the Handle data member to the given borrowed
/// handle. The ShouldDelete data member defaults to false, ensuring that the
/// borrowed handle will not be deleted when the C++ object is destroyed.
//
TPen::TPen(HPEN handle, TAutoDelete autoDelete)
:
  TGdiObject(handle, autoDelete)
{
    if (ShouldDelete)
      RefAdd(Handle, Pen);
}

//
/// Basic pen constructor. Detect constructions of stock pens & get stock objects instead
// 
/// Creates a TPen object with the given values. The width argument is in device
/// units, but if set to 0, a 1-pixel width is assumed. Sets Handle with the given
/// default values. If color is black or white, width is one, style is solid, a
/// stock pen handle is returned. The values for style are listed in the following
/// table.
/// - \c \b  PS_SOLID 	Creates a solid pen.
/// - \c \b  PS_DASH 	Creates a dashed pen. Valid only when the pen width is one or less in
/// device units.
/// - \c \b  PS_DOT 	Creates a dotted pen. Valid only when the pen width is one or less in
/// device units.
/// - \c \b  PS_DASHDOT	Creates a pen with alternating dashes dots. Valid only when the pen
/// width is one or less in device units.
/// - \c \b  PS_DASHDOTDOT	Creates a pen with alternating dashes double-dots. Valid only when
/// the pen width is one or less in device units.
/// - \c \b  PS_NULL 	Creates a null pen.
/// - \c \b  PS_INSIDEFRAME	Creates a solid pen. When this pen is used in any GDI drawing
/// function that takes a bounding rectangle, the dimensions of the figure will be
/// shrunk so that it fits entirely in the bounding rectangle, taking into account
/// the width of the pen.
//
TPen::TPen(const TColor& color, int width, int style)
{
  if (width == 1 && style == PS_SOLID &&
     (color == TColor::Black || color == TColor::White)) {
    if (color == TColor::Black)
      Handle = ::GetStockObject(BLACK_PEN);
    else
      Handle = ::GetStockObject(WHITE_PEN);
    ShouldDelete = false;
    return;
  }
  Handle = ::CreatePen(style, width, color);
  WARNX(OwlGDI, !Handle, 0, "Cannot create TPen (" << color << " " << width <<
        " " << style << ")");
  CheckValid();
  RefAdd(Handle, Pen);
}

//
/// Creates a TPen object from the given logPen values.
//
TPen::TPen(const LOGPEN& logPen)
{
  Handle = ::CreatePenIndirect(&logPen);
  WARNX(OwlGDI, !Handle, 0, "Cannot create TPen from logPen @" << static_cast<const void*>(&logPen));
  CheckValid();
  RefAdd(Handle, Pen);
}

#if defined(OWL5_COMPAT)

//
/// Creates a TPen object from the given logPen values.
/// This overload is deprecated. Use the overload that takes a reference instead.
//
TPen::TPen(const LOGPEN * logPen)
{
  PRECONDITION(logPen);
  Handle = ::CreatePenIndirect((LPLOGPEN)logPen);
  WARNX(OwlGDI, !Handle, 0, "Cannot create TPen from logPen @" <<
        hex << uint32(LPVOID(logPen)));
  CheckValid();
  RefAdd(Handle, Pen);
}

#endif

//
/// Construct a copy of an existing pen. Contructed pen will share the handle
/// unless NO_GDI_SHARE_HANDLES is defined, in which case a new handle is
/// created
//
TPen::TPen(const TPen& src)
{
#if !defined(NO_GDI_SHARE_HANDLES)
  Handle = src.Handle;
  RefAdd(Handle, Pen);
#else
  LOGPEN logPen;

  src.GetObject(logPen);
  Handle = ::CreatePenIndirect(&logPen);
  WARNX(OwlGDI, !Handle, 0, "Cannot create TPen from TPen @" <<
        hex << uint32(LPVOID(&src)));
  CheckValid();
  RefAdd(Handle, Pen);
#endif
}

//
/// Creates a TPen object with the given values.
//
TPen::TPen(uint32 penStyle, uint32 width, const TBrush& brush,
           uint32 styleCount, const uint32* style)
{
  LOGBRUSH logBrush = brush.GetObject();
  Handle = ::ExtCreatePen(penStyle, width, &logBrush, styleCount, (const DWORD*)style);
  WARNX(OwlGDI, !Handle, 0, "Cannot create TPen from brush " << hex <<
        uint(HBRUSH(brush)));
  CheckValid();
  RefAdd(Handle, Pen);
}

//
/// Creates a TPen object with the given values.
//
TPen::TPen(uint32 penStyle, uint32 width, const LOGBRUSH& logBrush,
           uint32 styleCount, const uint32* style)
{
  Handle = ::ExtCreatePen(penStyle, width, (LPLOGBRUSH)&logBrush, styleCount, (const DWORD*)style);
  WARNX(OwlGDI, !Handle, 0, "Cannot create TPen from logBrush @" <<
        hex << uint32(LPVOID(&logBrush)));
  CheckValid();
  RefAdd(Handle, Pen);
}

//
/// Retrieves information about this pen object and places it in the given LOGPEN structure. 
/// Throws TXGdi on failure.
//
LOGPEN TPen::GetObject() const
{
  LOGPEN logPen;
	bool r = TGdiObject::GetObject(sizeof(logPen), &logPen) != 0;
  if (!r) throw TXGdi(IDS_GDIFAILURE, Handle);
  return logPen;
}

} // OWL namespace
/* ========================================================================== */
