//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TRegion, a GDI Region object encapsulation
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

//
/// The default constructor creates an empty TRegion object.
/// ShouldDelete is set to true.
//
TRegion::TRegion()
{
  Handle = ::CreateRectRgn(0, 0, 0, 0);
  WARNX(OwlGDI, !Handle, 0, "Cannot create empty rect region");
  CheckValid();
}

//
/// Creates a TRegion object and sets the Handle data member to the given borrowed
/// handle. The ShouldDelete data member defaults to false, ensuring that the
/// borrowed handle is not deleted when the C++ object is destroyed. HRGN is the
/// data type representing the handle to an abstract shape.
//
TRegion::TRegion(HRGN handle, TAutoDelete autoDelete)
:
  TGdiBase(handle, autoDelete)
{
}

//
/// This public copy constructor creates a copy of the given TRegion object as in:
/// \code
/// TRegion myRegion = yourRegion;
/// \endcode
//
TRegion::TRegion(const TRegion& source)
{
  Handle = ::CreateRectRgn(0, 0, 0, 0);
  WARNX(OwlGDI, !Handle, 0, "Cannot create copy of region " <<
        uint(HRGN(source)));
  CheckValid();
  ::CombineRgn((HRGN)Handle, source, 0, RGN_COPY);
}

//
/// Creates a region object from the given TRect object as in:
/// \code
/// TRegion myRegion(rect1);
/// TRegion* pRegion;
/// pRegion = new TRegion(rect2);
/// \endcode
//
TRegion::TRegion(const TRect& rect)
{
  Handle = ::CreateRectRgnIndirect(&rect);
  WARNX(OwlGDI, !Handle, 0, "Cannot create rect region " << rect);
  CheckValid();
}

//
/// Creates the elliptical TRegion object that inscribes the given rectangle E. The
/// TEllipse argument distinguishes this constructor from the TRegion(const TRect&
/// rect) constructor.
//
TRegion::TRegion(const TRect& rect, TRegion::TEllipse)
{
  Handle = ::CreateEllipticRgnIndirect(&rect);
  WARNX(OwlGDI, !Handle, 0, "Cannot create elliptic region " << rect);
  CheckValid();
}

//
/// Creates a rounded rectangular TRegion object from the given rect corner.
//
TRegion::TRegion(const TRect& rect, const TSize& corner)
{
  Handle = ::CreateRoundRectRgn(rect.left, rect.top, rect.right, rect.bottom,
                                corner.cx, corner.cy);
  WARNX(OwlGDI, !Handle, 0, "Cannot create roundrect region " << rect << corner);
  CheckValid();
}

//
/// Creates a filled TRegion object from the polygons given by points and fillMode.
//
TRegion::TRegion(const TPoint* points, int count, int fillMode)
{
  Init(points, count, fillMode);
}

void TRegion::Init(const TPoint* points, int count, int fillMode)
{
  Handle = ::CreatePolygonRgn(points, count, fillMode);
  WARNX(OwlGDI, !Handle, 0, "Cannot create poly region with " << count << " points "
    << "@" << static_cast<const void*>(points));
  CheckValid();
}

//
/// Creates a filled TRegion object from the poly-polygons given by points and fillMode.
/// The 'points' argument should point into an array of points for all the polygons, and 
/// polyCounts should point into an array containing the number of points in each polygon.
/// The 'count' argument should specify the number of polygons.
//
TRegion::TRegion(const TPoint* points, const int* polyCounts, int count, int fillMode)
{
  Init(points, polyCounts, count, fillMode);
}

void TRegion::Init(const TPoint* points, const int* polyCounts, int count, int fillMode)
{
  Handle = ::CreatePolyPolygonRgn(points, polyCounts, count, fillMode);
  WARNX(OwlGDI, !Handle, 0, "Cannot create polypoly region with " << count << " polygons "
    << "@" << static_cast<const void*>(points));
  CheckValid();
}

//
// No orphan control for regions since they are not selectable into DCs,
// just delete
//
TRegion::~TRegion()
{
  if (ShouldDelete)
    if (!::DeleteObject(Handle))
      TXGdi::Raise(IDS_GDIDELETEFAIL, Handle);
}

//
/// Creates the intersection of this region with the given 
/// rectangle, and returns a reference to the result.
//
TRegion&
TRegion::operator &=(const TRect& source)
{
  ::CombineRgn((HRGN)Handle, (HRGN)Handle, TRegion(source), RGN_AND);
  return *this;
}

//
/// Creates the union of this region and the given rectangle, and
/// returns a reference to the result.
//
TRegion&
TRegion::operator |=(const TRect& source)
{
  ::CombineRgn((HRGN)Handle, (HRGN)Handle, TRegion(source), RGN_OR);
  return *this;
}

//
/// Creates the exclusive-or of this region and the given rectangle. 
/// Returns a reference to the resulting region object.
//
TRegion&
TRegion::operator ^=(const TRect& source)
{
  ::CombineRgn((HRGN)Handle, (HRGN)Handle, TRegion(source), RGN_XOR);
  return *this;
}

} // OWL namespace
/* ========================================================================== */

