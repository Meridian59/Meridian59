//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Classes for window system geometry
//----------------------------------------------------------------------------

#if !defined(OWL_GEOMETRY_H)
#define OWL_GEOMETRY_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif


#if !defined(UNIX)
#if !defined(OWL_OBJSTRM_H)
# include <owl/objstrm.h>    // Need persist streaming classes & operators
#endif
#endif

#include <owl/private/strmdefs.h>

#include <algorithm>

namespace owl {

//
// Forward declare some of the classes defined in this file
//
class _OWLCLASS TSize;
class _OWLCLASS TRect;

//
// Integer square root for area calculations. Is fairly fast.
//
_OWLFUNC(int16)  Sqrt(int32 val);
_OWLFUNC(int)  Sqrt(int64 val);

} // OWL namespace

//
// Get base window system geometry structs compatible with MSW
//
#include <owl/wsysinc.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup graphics
/// @{

/// General use absolute 2-D rectangular location enum
//
/// TAbsLocation contains general-use absolute two-dimensional rectangular
/// locations. It is used primarily to describe the locations of a gadget window,
/// such as a toolbar or a status bar, within a decorated frame.
//
enum TAbsLocation {
  alNone   = 0,          ///< No location specified
  alTop    = 1,          ///< Refers to top edge of frame
  alBottom = 2,          ///< Refers to bottom edge of frame
  alLeft   = 3,          ///< Refers to left edge of frame
  alRight  = 4,          ///< Refers to right edge of frame
};

//
/// \class TPoint
// ~~~~~ ~~~~~~
/// TPoint is a support class, derived from tagPOINT. The tagPOINT struct is defined
/// as
/// \code
/// struct tagPOINT { 
///    int x;
///    int y;
/// };
/// \endcode
/// TPoint encapsulates the notion of a two-dimensional point that usually
/// represents a screen position. TPoint inherits two data members, the coordinates
/// x and y, from tagPOINT. Member functions and operators are provided for
/// comparing, assigning, and manipulating points. Overloaded << and >> operators
/// allow chained insertion and extraction of TPoint objects with streams.
//
class _OWLCLASS TPoint : public tagPOINT {
  public:
    // Constructors
    //
    TPoint();
    TPoint(int _x, int _y);
    TPoint(const tagPOINT & point);
    TPoint(const TPoint & point);
    TPoint(const tagSIZE & size);
    explicit TPoint(LPARAM packedPoint);

    // Information functions/operators
    //
    bool        operator ==(const TPoint& other) const;
    bool        operator !=(const TPoint& other) const;
    int         X() const;
    int         Y() const;
    int         Magnitude() const;

    // Functions/binary-operators that return points or sizes
    //
    TPoint      OffsetBy(int dx, int dy) const;
    TPoint      operator +(const TSize& size) const;
    TSize       operator -(const TPoint& point) const;
    TPoint      operator -(const TSize& size) const;
    TPoint      operator -() const;

    // Functions/assignement-operators that modify this point
    //
    TPoint&     Offset(int dx, int dy);
    TPoint&     operator +=(const TSize& size);
    TPoint&     operator -=(const TSize& size);
};

_OWLCFUNC(ipstream&) operator >>(ipstream& is, TPoint& p);
_OWLCFUNC(tistream&)  operator >>(tistream& is, TPoint& p);
_OWLCFUNC(opstream&) operator <<(opstream& os, const TPoint& p);
_OWLCFUNC(tostream&)  operator <<(tostream& os, const TPoint& p);

//
/// \class TPointL
// ~~~~~ ~~~~~~~
/// TPointL is similar to TPoint, but uses long rather than int variables.
/// \todo Do we really need this? Isn't long same as int for 32-bit? What about 64-bit
//
class _OWLCLASS TPointL : public POINTL {
  public:
    // Constructors
    //
    TPointL();
    TPointL(long _x, long _y);
    TPointL(const POINTL & point);
    TPointL(const TPointL & point);

    // Information functions/operators
    //
    bool        operator ==(const TPointL& other) const;
    bool        operator !=(const TPointL& other) const;
    long        X() const;
    long        Y() const;

    // Functions/binary-operators that return points or sizes
    //
    TPointL     OffsetBy(long dx, long dy) const;
    TPointL     operator +(const TSize& size) const;
    TPointL     operator -(const TPointL& point) const;
    TPointL     operator -(const TSize& size) const;
    TPointL     operator -() const;

    // Functions/assignement-operators that modify this point
    //
    TPointL&    Offset(long dx, long dy);
    TPointL&    operator +=(const TSize& size);
    TPointL&    operator -=(const TSize& size);
};

_OWLCFUNC(ipstream&) operator >>(ipstream& is, TPointL& p);
_OWLCFUNC(tistream&)  operator >>(tistream& is, TPointL& p);
_OWLCFUNC(opstream&) operator <<(opstream& os, const TPointL& p);
_OWLCFUNC(tostream&)  operator <<(tostream& os, const TPointL& p);

//
/// Base struct for the TPointF class
//
struct tPOINTF {
  float x;
  float y;
};

//
/// \class TPointF
// ~~~~~ ~~~~~~~
/// TPointF is similar to TPoint, but uses floating variables rather than integers.
//
class _OWLCLASS TPointF : public tPOINTF {
  public:
    // Constructors
    //
    TPointF();
    TPointF(float _x, float _y);
//    TPointF(const tagPOINTF & point) {x = point.x; y = point.y;}
    TPointF(const TPointF & point);

    // Information functions/operators
    //
    bool        operator ==(const TPointF& other) const;
    bool        operator !=(const TPointF& other) const;
    float       X() const;
    float       Y() const;

    // Functions/binary-operators that return points or sizes
    //
    TPointF     OffsetBy(float dx, float dy) const;
    TPointF     operator +(const TPointF& size) const;
    TPointF     operator -(const TPointF& point) const;
    TPointF     operator -() const;

    // Functions/assignement-operators that modify this point
    //
    TPointF&    Offset(float dx, float dy);
    TPointF&    operator +=(const TPointF& size);
    TPointF&    operator -=(const TPointF& size);
};

_OWLCFUNC(ipstream&) operator >>(ipstream& is, TPointF& p);
_OWLCFUNC(tistream&) operator >>(tistream& is, TPointF& p);
_OWLCFUNC(opstream&) operator <<(opstream& os, const TPointF& p);
_OWLCFUNC(tostream&) operator <<(tostream& os, const TPointF& p);

//
/// \class TSize
// ~~~~~ ~~~~~
/// The tagSIZE  struct is defined as
/// \code
/// struct tagSIZE { 
///    int cx;
///    int cy;
/// };
/// \endcode
/// TSize encapsulates the notion of a two-dimensional quantity that usually
/// represents a displacement or the height and width of a rectangle. TSize inherits
/// the two data members cx and cy from tagSIZE. Member functions and operators are
/// provided for comparing, assigning, and manipulating sizes. Overloaded << and >>
/// operators allow chained insertion and extraction of TSize objects with streams.
//
// !CQ look at phasing out this class & using only TPoint
//
class _OWLCLASS TSize : public tagSIZE {
  public:
    // Constructors
    //
    TSize();
    TSize(int dx, int dy);
    TSize(const tagPOINT & point);
    TSize(const tagSIZE & size);
    TSize(const TSize & size);
    explicit TSize(DWORD packedExtents);

    // Information functions/operators
    //
    bool        operator ==(const TSize& other) const;
    bool        operator !=(const TSize& other) const;
    int         X() const;
    int         Y() const;
    int         Magnitude() const;

    // Functions/binary-operators that return sizes
    //
    TSize      operator +(const TSize& size) const;
    TSize      operator -(const TSize& size) const;
    TSize      operator -() const;

    // Functions/assignement-operators that modify this size
    //
    TSize&     operator +=(const TSize& size);
    TSize&     operator -=(const TSize& size);
};

_OWLCFUNC(ipstream&)   operator >>(ipstream& is, TSize& s);
_OWLCFUNC(tistream&)   operator >>(tistream& os, TSize& s);
_OWLCFUNC(opstream&)   operator <<(opstream& os, const TSize& s);
_OWLCFUNC(tostream&)   operator <<(tostream& os, const TSize& s);

//
/// \class TRect
// ~~~~~ ~~~~~
/// TRect is a mathematical class derived from tagRect. The tagRect  struct is
/// defined as
/// \code
/// struct tagRECT { 
///    int left;
///    int top;
///    int right;
///    int bottom;
/// };
/// \endcode
/// TRect encapsulates the properties of rectangles with sides parallel to the x-
/// and y-axes. In ObjectWindows, these rectangles define the boundaries of windows,
/// boxes, and clipping regions. TRect inherits four data members from tagRect left,
/// top, right, and bottom. These represent the top left and bottom right (x, y)
/// coordinates of the rectangle. Note that x increases from left to right, and y
/// increases from top to bottom.
/// 
/// TRect places no restrictions on the relative positions of top left and bottom
/// right, so it is legal to have left > right and top > bottom. However, many
/// manipulations--such as determining width and height, and forming unions and
/// intersections--are simplified by normalizing the TRect objects involved.
/// Normalizing a rectangle means interchanging the corner point coordinate values
/// so that left < right and top < bottom. Normalization does not alter the physical
/// properties of a rectangle. myRect.Normalized creates normalized copy of myRect
/// without changing myRect, while myRect.Normalize changes myRect to a normalized
/// format. Both members return the normalized rectangle.
/// 
/// TRect constructors are provided to create rectangles from either four ints, two
/// TPoint objects, or one TPoint and one TSize object. In the latter case, the
/// TPoint object specifies the top left point (also known as the rectangle's
/// origin) and the TSize object supplies the width and height of the rectangle.
/// Member functions perform a variety of rectangle tests and manipulations.
/// Overloaded << and >> operators allow chained insertion and extraction of TRect
/// objects with streams.
//
class _OWLCLASS TRect : public tagRECT {
  public:
    // Constructors
    //
    TRect();
    TRect(const tagRECT & rect);
    TRect(const TRect & rect);
    TRect(int _left, int _top, int _right, int _bottom);
    TRect(const TPoint& upLeft, const TPoint& loRight);
    TRect(const TPoint& origin, const TSize& extent);

    // (Re)Initializers
    //
    void        SetNull();
    void        SetEmpty();
    void        Set(int _left, int _top, int _right, int _bottom);
    void        SetWH(int _left, int _top, int w, int h);

    // Type Conversion operators
    //
    operator    const TPoint*() const;
    operator    TPoint*();

    // Testing functions
    //
    bool        IsEmpty() const;
    bool        IsNull() const;
    bool        operator ==(const TRect& other) const;
    bool        operator !=(const TRect& other) const;

    // Information/access functions (const and non-const)
    //
    int         Left() const;
    int         X() const;
    int         Top() const;
    int         Y() const;
    int         Right() const;
    int         Bottom() const;

    const TPoint& TopLeft() const;
    TPoint&     TopLeft();
    TPoint      TopRight() const;
    TPoint      BottomLeft() const;
    const TPoint& BottomRight() const;
    TPoint&     BottomRight();

    int         Width() const;
    int         Height() const;
    TSize       Size() const;
    long        Area() const;

    bool        Contains(const TPoint& point) const;
    bool        Contains(const TRect& other) const;
    bool        Touches(const TRect& other) const;
    TRect       OffsetBy(int dx, int dy) const;
    TRect       operator +(const TSize& size) const;
    TRect       operator -(const TSize& size) const;
    TRect       MovedTo(int x, int y);
    TRect       InflatedBy(int dx, int dy) const;
    TRect       InflatedBy(const TSize& size) const;
    TRect       Normalized() const;
    TRect       operator &(const TRect& other) const;
    TRect       operator |(const TRect& other) const;

    int         Subtract(const TRect& other, TRect result[]) const;

    // Manipulation functions/operators
    //
    TRect&      Normalize();
    TRect&      Offset(int dx, int dy);
    TRect&      operator +=(const TSize& delta);
    TRect&      operator -=(const TSize& delta);
    TRect&      MoveTo(int x, int y);
    TRect&      Inflate(int dx, int dy);
    TRect&      DeflateRect(int dx, int dy); //DLN MFC look-alike
    TRect&      Inflate(const TSize& delta);
    TRect&      DeflateRect(const TSize& delta); //DLN MFC look-alike
    TRect&      operator &=(const TRect& other);
    TRect&      operator |=(const TRect& other);
};

_OWLCFUNC(ipstream&) operator >>(ipstream& is, TRect& r);
_OWLCFUNC(tistream&)  operator >>(tistream& is, TRect& r);
_OWLCFUNC(opstream&) operator <<(opstream& os, const TRect& r);
_OWLCFUNC(tostream&)  operator <<(tostream& os, const TRect& r);

/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inlines
//

//----------------------------------------------------------------------------
// TPoint
//




//
/// Constructs an uninitialized point.
//
inline
TPoint::TPoint()
{
  x = 0;
  y = 0;
}

//
/// Creates a TPoint object with the given coordinates.
//
inline
TPoint::TPoint(int _x, int _y)
{
  x = _x;
  y = _y;
}

//
/// Creates a TPoint object with x = point.x,  y = point.y.
//
inline
TPoint::TPoint(const tagPOINT & point)
{
  x = point.x;
  y = point.y;
}

//
/// Makes a copy of the point.
//
inline
TPoint::TPoint(const TPoint & point)
{
  x = point.x;
  y = point.y;
}

//
/// Creates a TPoint object with x = size.cx and y = size.cy.
//
inline
TPoint::TPoint(const tagSIZE & size)
{
  x = size.cx;
  y = size.cy;
}

//
/// Returns the x coordinate of the point.
//
inline int
TPoint::X() const
{
  return x;
}

//
/// Returns the Y coordinate of the point.
//
inline int
TPoint::Y() const
{
  return y;
}

//
/// Calculates an offset to this point using the given displacement arguments.
/// Returns the point (x + dx, y + dy). This point is not changed.
//
inline TPoint
TPoint::OffsetBy(int dx, int dy) const
{
  return TPoint(x + dx, y + dy);
}

//
/// Returns the point (-x, -y). This point is not changed.
//
inline TPoint
TPoint::operator -() const
{
  return TPoint(-x, -y);
}

//
/// Returns true if this point is equal to the other point; otherwise returns false.
//
inline bool
TPoint::operator ==(const TPoint& other) const
{
  return ToBool(other.x == x && other.y == y);
}

//
/// Returns false if this point is equal to the other point; otherwise returns true.
//
inline bool
TPoint::operator !=(const TPoint& other) const
{
  return ToBool(other.x != x || other.y != y);
}

//
/// Calculates an offset to this point using the given size argument as the
/// displacement. Returns the point (x + size.cx, y + size.cy). This point is not
/// changed.
//
inline TPoint
TPoint::operator +(const TSize& size) const
{
  return TPoint(x + size.cx, y + size.cy);
}

//
/// Calculates a distance from this point to the point argument. Returns the TSize
/// object (x - point.x, y - point.y). This point is not changed.
//
inline TSize
TPoint::operator -(const TPoint& point) const
{
  return TSize(x - point.x, y - point.y);
}

//
/// Calculates a negative offset to this point using the given size argument as the
/// displacement. Returns the point (x - size.cx, y - size.cy). This point is not
/// changed.
//
inline TPoint
TPoint::operator -(const TSize& size) const
{
  return TPoint(x - size.cx, y - size.cy);
}

//
/// Offsets this point by the given delta arguments. This point is changed to (x +
/// dx, y + dy). Returns a reference to this point.
//
inline TPoint&
TPoint::Offset(int dx, int dy)
{
  x += dx;
  y += dy;
  return *this;
}

//
/// Offsets this point by the given size argument. This point is changed to(x +
/// size.cx, y + size.cy). Returns a reference to this point.
//
inline TPoint&
TPoint::operator +=(const TSize& size)
{
  x += size.cx;
  y += size.cy;
  return *this;
}

//
/// Negatively offsets this point by the given size argument. This point is changed
/// to (x - size.cx, y - size.cy). Returns a reference to this point.
//
inline TPoint&
TPoint::operator -=(const TSize& size)
{
  x -= size.cx;
  y -= size.cy;
  return *this;
}

//----------------------------------------------------------------------------
// TPointL
//

//
/// Default constructor that does nothing.
//
inline
TPointL::TPointL()
{
  x = 0;
  y = 0;
}

//
/// Constructs the point to a specific location.
//
inline
TPointL::TPointL(long _x, long _y)
{
  x = _x;
  y = _y;
}

//
/// Alias constructor that initializes from an existing point.
//
inline
TPointL::TPointL(const POINTL & point)
{
  x = point.x;
  y = point.y;
}

//
/// Copy constructor. Makes a copy of the location.
//
inline
TPointL::TPointL(const TPointL & point)
{
  x = point.x;
  y = point.y;
}

//
/// Returns the X component of the point.
//
inline long
TPointL::X() const
{
  return x;
}

//
/// Returns the Y component of the point.
//
inline long
TPointL::Y() const
{
  return y;
}

//
/// Returns the new point (x+dx, y+dy).  Creates a new point shifted by the offset,
/// preserving the original point.
//
inline TPointL
TPointL::OffsetBy(long dx, long dy) const
{
  return TPointL(x + dx, y + dy);
}

//
/// Returns the negative of the point.
//
inline TPointL
TPointL::operator -() const
{
  return TPointL(-x, -y);
}

//
/// Returns true if positions are the same.
//
inline bool
TPointL::operator ==(const TPointL& other) const
{
  return ToBool(other.x == x && other.y == y);
}

//
/// Returns true if the positions are not the same.
//
inline bool
TPointL::operator !=(const TPointL& other) const
{
  return ToBool(other.x != x || other.y != y);
}

//
/// Returns the new point (x+cx, y+cy).
//
inline TPointL
TPointL::operator +(const TSize& size) const
{
  return TPointL(x + size.cx, y + size.cy);
}

//
/// Returns the difference between the two points.
//
inline TPointL
TPointL::operator -(const TPointL& point) const
{
  return TPointL(x - point.x, y - point.y);
}

//
/// Return the new point (x-cx, y-cy).
//
inline TPointL
TPointL::operator -(const TSize& size) const
{
  return TPointL(x - size.cx, y - size.cy);
}

//
/// Returns the point (x+dx, y+dy), shifting the point by the offset.
//
inline TPointL&
TPointL::Offset(long dx, long dy)
{
  x += dx;
  y += dy;
  return *this;
}

//
/// Return the point (x+cx, y+cy).
//
inline TPointL&
TPointL::operator +=(const TSize& size)
{
  x += size.cx;
  y += size.cy;
  return *this;
}

//
/// Return the point (x-cx, y-cy).
//
inline TPointL&
TPointL::operator -=(const TSize& size)
{
  x -= size.cx;
  y -= size.cy;
  return *this;
}


//----------------------------------------------------------------------------
// TPointF
//

//
/// Default constructor that does nothing.
//
inline
TPointF::TPointF()
{
  x = 0.0f;
  y = 0.0f;
}

//
/// Constructor that initializes the location.
//
inline
TPointF::TPointF(float _x, float _y)
{
  x = _x;
  y = _y;
}

//
/// Constructor that copies the location.
//
inline
TPointF::TPointF(const TPointF & point)
{
  x = point.x;
  y = point.y;
}

//
/// Returns X component of the point.
//
inline float
TPointF::X() const
{
  return x;
}

//
/// Returns Y component of the point.
//
inline float
TPointF::Y() const
{
  return y;
}

//
/// Moves the point by an offset.
//
inline TPointF
TPointF::OffsetBy(float dx, float dy) const
{
  return TPointF(x + dx, y + dy);
}

//
/// Returns the negative of the point.
//
inline TPointF
TPointF::operator -() const
{
  return TPointF(-x, -y);
}

//
/// Returns true if the points are at the same location.
//
inline bool
TPointF::operator ==(const TPointF& other) const
{
  return ToBool(other.x == x && other.y == y);
}

//
/// Return true if the points are not at the same location.
//
inline bool
TPointF::operator !=(const TPointF& other) const
{
  return ToBool(other.x != x || other.y != y);
}

//
/// Returns a new point (x+cx, y+cy).
//
inline TPointF
TPointF::operator +(const TPointF& size) const
{
  return TPointF(x + size.x, y + size.y);
}

//
/// Returns a new point subtracted from the current point.
//
inline TPointF
TPointF::operator -(const TPointF& point) const
{
  return TPointF(x - point.x, y - point.y);
}

//
/// Moves the point by an offset.
//
inline TPointF&
TPointF::Offset(float dx, float dy)
{
  x += dx;
  y += dy;
  return *this;
}

//
/// Returns the new point moved by the offset.
//
inline TPointF&
TPointF::operator +=(const TPointF& size)
{
  x += size.x;
  y += size.y;
  return *this;
}

//
/// Returns the new point subtracted from the current.
//
inline TPointF&
TPointF::operator -=(const TPointF& size)
{
  x -= size.x;
  y -= size.y;
  return *this;
}

//----------------------------------------------------------------------------
// TRect
//

//
/// Default constructor that does nothing.
//
inline
TRect::TRect()
{
  SetNull();
}

//
/// Empties this rectangle by setting left, top, right, and bottom to 0.
//
inline void
TRect::SetEmpty()
{
  SetNull();
}

//
/// Type conversion operators converting the pointer to this rectangle to type
/// pointer to TPoint.
/// Return an array of two points (upperleft and bottomright)
//
inline
TRect::operator const TPoint*() const
{
  return (const TPoint*)this;
}

//
/// Type conversion operators converting the pointer to this rectangle to type
/// pointer to TPoint.
/// Return an array of two points (upperleft and bottomright)
//
inline
TRect::operator TPoint*()
{
  return (TPoint*)this;
}

//
/// Returns the left value.
//
inline int
TRect::Left() const
{
  return left;
}

//
/// Returns the left value.
//
inline int
TRect::X() const
{
  return left;
}

//
/// Returns the top value.
//
inline int
TRect::Top() const
{
  return top;
}

//
/// Returns the top value.
//
inline int
TRect::Y() const
{
  return top;
}

//
/// Returns the right value.
//
inline int
TRect::Right() const
{
  return right;
}

//
/// Returns the bottom value.
//
inline int
TRect::Bottom() const
{
  return bottom;
}

//
/// Returns the upperleft point.
//
inline const TPoint&
TRect::TopLeft() const
{
  return *(TPoint*)&left;
}

//
/// Returns the upperleft point.
//
inline TPoint&
TRect::TopLeft()
{
  return *(TPoint*)&left;
}

//
/// Returns the upperright point.
//
inline TPoint
TRect::TopRight() const
{
  return TPoint(right, top);
}

//
/// Returns the TPoint object representing the bottom left corner of this rectangle.
//
inline TPoint
TRect::BottomLeft() const
{
  return TPoint(left, bottom);
}

//
/// Returns the TPoint object representing the bottom right corner of this
/// rectangle.
//
inline const TPoint&
TRect::BottomRight() const
{
  return *(TPoint*)&right;
}

//
/// Returns the TPoint object representing the bottom right corner of this
/// rectangle.
//
inline TPoint&
TRect::BottomRight()
{
  return *(TPoint*)&right;
}

//
/// Returns the width of this rectangle (right - left).
//
inline int
TRect::Width() const
{
  return right-left;
}

//
/// Returns the height of this rectangle (bottom - top).
//
inline int
TRect::Height() const
{
  return bottom-top;
}

//
/// Returns the size of rectangle.
//
inline TSize
TRect::Size() const
{
  return TSize(Width(), Height());
}

//
/// Returns the area of this rectangle.
//
inline long
TRect::Area() const
{
  return long(Width())*long(Height());
}

//
/// Sets the left, top, right, and bottom of the rectangle to 0.
//
inline void
TRect::SetNull()
{
  left = 0;
  top = 0;
  right = 0;
  bottom = 0;
}

//
/// Repositions and resizes this rectangle to the given values.
//
inline void
TRect::Set(int _left, int _top, int _right, int _bottom)
{
  left = _left;
  top = _top;
  right = _right;
  bottom = _bottom;
}

//
/// Determines the rectangle, given its upperleft point, width, and height
//
inline void
TRect::SetWH(int _left, int _top, int w, int h)
{
  left = _left;
  top = _top;
  right = _left + w;
  bottom = _top + h;
}

//
/// Copy from an existing rectangle.
//
inline
TRect::TRect(const tagRECT & rect)
{
  *(tagRECT *)this = rect;
}

//
/// Copy from an existing rectangle.
//
inline
TRect::TRect(const TRect & rect)
{
  *(tagRECT *)this = *(tagRECT *)&rect;
}

//
/// Constructor that sets all the values explicitly.
//
inline
TRect::TRect(int _left, int _top, int _right, int _bottom)
{
  Set(_left, _top, _right, _bottom);
}

//
/// Creates a rectangle with the given top left and bottom right points.
//
inline
TRect::TRect(const TPoint& topLeft, const TPoint& bottomRight)
{
  Set(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
}

//
/// Creates a rectangle with its origin (top left) at origin, width at extent.cx,
/// height at extent.cy.
//
inline
TRect::TRect(const TPoint& origin, const TSize& extent)
{
  Set(origin.x, origin.y, origin.x + extent.cx, origin.y + extent.cy);
}

//
/// Returns true if left >= right or top >= bottom; otherwise, returns false.
//
inline bool
TRect::IsEmpty() const
{
  return ToBool(left >= right || top >= bottom);
}

//
/// Returns true if left, right, top, and bottom are all 0; otherwise, returns
/// false.
//
inline bool
TRect::IsNull() const
{
  return ToBool(!left && !right && !top && !bottom);
}

//
/// Returns true if this rectangle has identical corner coordinates to the other
/// rectangle; otherwise, returns false.
//
inline bool
TRect::operator ==(const TRect& other) const
{
  return ToBool(other.left==left   && other.top==top
             && other.right==right && other.bottom==bottom);
}

//
/// Returns false if this rectangle has identical corner coordinates to the other
/// rectangle; otherwise, returns true.
//
inline bool
TRect::operator !=(const TRect& other) const
{
  return ToBool(!(other==*this));
}

//
/// Returns true if the given point lies within this rectangle; otherwise, it
/// returns false. If point is on the left vertical or on the top horizontal borders
/// of the rectangle, Contains also returns true, but if point is on the right
/// vertical or bottom horizontal borders, Contains returns false.
//
inline bool
TRect::Contains(const TPoint& point) const
{
  return ToBool(point.x >= left && point.x < right
             && point.y >= top  && point.y < bottom);
}

//
/// Returns true if the other rectangle lies on or within this rectangle; otherwise,
/// it returns false.
//
inline bool
TRect::Contains(const TRect& other) const
{
  return ToBool(other.left >= left && other.right <= right
             && other.top >= top   && other.bottom <= bottom);
}

//
/// Returns true if the other rectangle shares any interior points with this
/// rectangle; otherwise, returns false.
//
inline bool
TRect::Touches(const TRect& other) const
{
  return ToBool(other.right > left && other.left < right
             && other.bottom > top && other.top < bottom);
}

//
/// Returns a rectangle with the corners offset by the given delta values. The
/// returned rectangle has a top left corner at (left + dx, top + dy) and a right
/// bottom corner at (right + dx, bottom + dy). The calling rectangle object is unchanged
//
inline TRect
TRect::OffsetBy(int dx, int dy) const
{
  return TRect(left+dx, top+dy, right+dx, bottom+dy);
}

//
/// Returns a rectangle offset positively by the delta values' given sizes. The
/// returned rectangle has a top left corner at (left + size.x, top + size.y) and a
/// right bottom corner at (right + size.x, bottom + size.y). The calling rectangle
/// object is unchanged.
//
inline TRect
TRect::operator +(const TSize& size) const
{
  return OffsetBy(size.cx, size.cy);
}

//
/// Returns a rectangle offset negatively by the delta values' given sizes. The
/// returned rectangle has a top left corner at (left - size.cx, top - size.cy) and
/// a right bottom corner at (right - size.cx, bottom - size.cy). The calling
/// rectangle object is unchanged.
//
inline TRect
TRect::operator -(const TSize& size) const
{
  return OffsetBy(-size.cx, -size.cy);
}

//
/// Moves the upper left point of the rectangle while maintaining the
/// current dimension.
/// \todo This should be const, to indicate that the original rectangle is not modified,
/// just like InflatedBy()
//
inline TRect
TRect::MovedTo(int x, int y)
{
  return TRect(x, y, x + Width(), y + Height());
}

//
/// Returns a rectangle inflated by the given delta arguments. 
/// The top left corner of the returned rectangle is (left - dx, top - dy), while
/// its bottom right corner is (right + dx, bottom + dy). The calling rectangle object is unchanged.
//
inline TRect
TRect::InflatedBy(int dx, int dy) const
{
  return TRect(left-dx, top-dy, right+dx, bottom+dy);
}

//
/// Returns a rectangle inflated by the given delta arguments. 
/// The new corners are (left - size.cx, top - size.cy) and (right + size.cx, bottom +
/// size.cy). The calling rectangle object is unchanged.
//
inline TRect
TRect::InflatedBy(const TSize& size) const
{
  return InflatedBy(size.cx, size.cy);
}

//
/// Returns a normalized rectangle with the top left corner at (Min(left, right),
/// Min(top, bottom)) and the bottom right corner at (Max(left, right), Max(top,
/// bottom)). The calling rectangle object is unchanged. A valid but nonnormal
/// rectangle might have left > right or top > bottom or both. In such cases, many
/// manipulations (such as determining width and height) become unnecessarily
/// complicated. Normalizing a rectangle means interchanging the corner point values
/// so that left < right and top < bottom. The physical properties of a rectangle
/// are unchanged by this process.
/// Note that many calculations assume a normalized rectangle. Some Windows API
/// functions behave erratically if an inside-out Rect is passed.
//
inline TRect
TRect::Normalized() const
{
  return TRect(std::min(left, right), std::min(top, bottom),
               std::max(left, right), std::max(top, bottom));
}

//
/// Returns the intersection of this rectangle and the other rectangle. The calling
/// rectangle object is unchanged. Returns a NULL rectangle if the two don't
/// intersect.
//
inline TRect
TRect::operator &(const TRect& other) const
{
  if (Touches(other))
    return TRect(std::max(left, other.left), std::max(top, other.top),
                 std::min(right, other.right), std::min(bottom, other.bottom));
  return TRect(0, 0, 0, 0);
}

//
/// Returns the union of this rectangle and the other rectangle. The calling
/// rectangle object is unchanged.
//
inline TRect
TRect::operator |(const TRect& other) const
{
  return TRect(std::min(left, other.left), std::min(top, other.top),
               std::max(right, other.right), std::max(bottom, other.bottom));
}

//
/// Changes this rectangle so its corners are offset by the given delta values,
/// delta.x and delta.y. The revised rectangle has a top left corner at (left +
/// delta.x, top + delta.y) and a right bottom corner at (right + delta.x, bottom +
/// delta.y). The revised rectangle is returned.
//
inline TRect&
TRect::operator +=(const TSize& delta)
{
  Offset(delta.cx, delta.cy);
  return *this;
}

//
/// Changes this rectangle so its corners are offset negatively by the given delta
/// values, delta.x and delta.y. The revised rectangle has a top left corner at
/// (left - delta.x, top - delta.y) and a right bottom corner at (right - delta.x,
/// bottom - delta.y). The revised rectangle is returned.
//
inline TRect&
TRect::operator -=(const TSize& delta)
{
  return *this += -delta;
}

//
/// Moves the upper left corner of the rectangle to a new location and
/// maintains the current dimension.
//
inline TRect&
TRect::MoveTo(int x, int y)
{
  right = x + Width();
  bottom = y + Height();
  left = x;
  top = y;
  return *this;
}

//
/// Inflates a rectangle inflated by the given delta arguments. 
/// The new corners are (left - size.cx, top - size.cy) and (right +
/// size.cx, bottom + size.cy).
//
inline TRect&
TRect::Inflate(const TSize& delta)
{
  return Inflate(delta.cx, delta.cy);
}

/// DLN MFC look-alike
inline TRect&
TRect::DeflateRect(const TSize& delta)
{
  return Inflate(-delta.cx, -delta.cy);
}

//----------------------------------------------------------------------------
// TSize
//

//
/// Default constructor that does nothing.
//
inline
TSize::TSize()
{
  cx  = 0;
  cy  = 0;
}

//
/// Creates a TSize object with cx = dx and cy = dy.
//
inline
TSize::TSize(int dx, int dy)
{
  cx = dx;
  cy = dy;
}

//
/// Creates a TSize object with cx = point.x and cy = point.y.
//
inline
TSize::TSize(const tagPOINT & point)
{
  cx = point.x;
  cy = point.y;
}

//
/// Creates a TSize object with cx = size.cx and cy = size.cy.
//
inline
TSize::TSize(const tagSIZE & size)
{
  cx = size.cx;
  cy = size.cy;
}

//
/// Creates a TSize object with cx = size.cx and cy = size.cy.
//
inline
TSize::TSize(const TSize & size)
{
  cx = size.cx;
  cy = size.cy;
}

//
/// Constructor that assumes the high-word of a long is the width
/// and the low-word of a long is the height.
///
/// Creates a TSize object with cx = LOWORD(dw) and cy = HIWORD(dw)).
//
inline
TSize::TSize(DWORD packedExtents)
{
  cx = LOWORD(packedExtents);
  cy = HIWORD(packedExtents);
}

//
/// Returns the width.
//
inline int
TSize::X() const
{
  return cx;
}

//
/// Returns the height.
//
inline int
TSize::Y() const
{
  return cy;
}

//
/// Returns the TSize object (-cx, -cy). This object is not changed.
//
inline TSize
TSize::operator -() const
{
  return TSize(-cx, -cy);
}

//
/// Returns true if this TSize object is equal to the other TSize object; otherwise
/// returns false.
//
inline bool
TSize::operator ==(const TSize& other) const
{
  return ToBool(other.cx==cx && other.cy==cy);
}

//
/// Returns false if this TSize object is equal to the other TSize object; otherwise
/// returns true.
//
inline bool
TSize::operator !=(const TSize& other) const
{
  return ToBool(other.cx!=cx || other.cy!=cy);
}

//
/// Calculates an offset to this TSize object using the given size argument as the
/// displacement. Returns the object (cx + size.cx, cy + size.cy). This TSize object
/// is not changed.
//
inline TSize
TSize::operator +(const TSize& size) const
{
  return TSize(cx+size.cx, cy+size.cy);
}

//
/// Calculates a negative offset to this TSize object using the given size argument
/// as the displacement. Returns the point (cx - size.cx, cy - size.cy). This object
/// is not changed.
//
inline TSize
TSize::operator -(const TSize& size) const
{
  return TSize(cx-size.cx, cy-size.cy);
}

//
/// Offsets this TSize object by the given size argument. This TSize object is
/// changed to (cx + size.cx, cy + size.cy). Returns a reference to this object.
//
inline TSize&
TSize::operator +=(const TSize& size)
{
  cx += size.cx;
  cy += size.cy;
  return *this;
}

//
/// Negatively offsets this TSize object by the given size argument. This object is
/// changed to (cx - size.cx, cy - size.cy). Returns a reference to this object.
//
inline TSize&
TSize::operator -=(const TSize& size)
{
  cx -= size.cx;
  cy -= size.cy;
  return *this;
}

} // OWL namespace

#endif  // OWL_GEOMETRY_H
