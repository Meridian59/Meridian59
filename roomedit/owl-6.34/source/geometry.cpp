//----------------------------------------------------------------------------
// Borland WinSys Library
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of classes for windowing system geometry
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/geometry.h>
#include <windowsx.h>

namespace owl {

// Let the compiler know that the following template instances will be defined elsewhere.
//#pragma option -Jgx

//
// Calculates the integer square root of a 32bit signed long. Returns a 16bit
// signed. Is fairly fast, esp. compared to FP versions
//
_OWLFUNC(int16)
Sqrt(int32 val)
{
  if (val <= 0)
    return 0;     // Could throw a math exception?

  uint mask = 0x4000;   // Bit mask to shift right
  int best = 0;         // Best estimate so far

  for (; mask; mask >>= 1)
    if (long(((uint32)(long)best+mask)*((uint32)(long)best+mask)) <= val)
      best |= mask;

  return int16(best);
}

_OWLFUNC(int)
Sqrt(int64 val)
{
  if (val <= int64(0l))
    return 0;     // Could throw a math exception?

  uint mask = 0x40000000;   // Bit mask to shift right
  int64 best = 0l;         // Best estimate so far
  int64 temp;

  for (; mask; mask >>= 1) {
    temp = best | int64(mask);
    if (temp * temp <= val)
      best = temp;
  }
  return int32(best);
}

//
/// Creates a TPoint object from a packed point.  
/// The given point must be encoded as in the LPARAM argument of a Windows message such as
/// the WM_CONTEXTMENU message.
//
TPoint::TPoint(LPARAM packedPoint)
{
  x = GET_X_LPARAM(packedPoint);
  y = GET_Y_LPARAM(packedPoint);
}

//
/// Returns the distance between the origin and the point.
//
int TPoint::Magnitude() const
{
  int64 x64 = x;
  int64 y64 = y;
  return Sqrt(x64 * x64 + y64 * y64);
}

//
/// Returns the length of the diagonal of the rectangle represented by this object.
/// The value returned is an int approximation to the square root of (cx2 + cy2).
//
int TSize::Magnitude() const
{
  int64 x64 = cx;
  int64 y64 = cy;
  return Sqrt(x64 * x64 + y64 * y64);
}

//
/// Normalizes this rectangle by switching the left and right data member values if
/// left > right, and switching the top and bottom data member values if top >
/// bottom. Normalize returns the normalized rectangle. A valid but nonnormal
/// rectangle might have left > right or top > bottom or both. In such cases, many
/// manipulations (such as determining width and height) become unnecessarily
/// complicated. Normalizing a rectangle means interchanging the corner point values
/// so that left < right and top < bottom. The physical properties of a rectangle
/// are unchanged by this process.
//
TRect&
TRect::Normalize()
{
  if (left > right)
    Swap(left, right);
  if (top > bottom)
    Swap(top, bottom);
  return *this;
}

//
/// Changes this rectangle so its corners are offset by the given delta values. The
/// revised rectangle has a top left corner at (left + dx, top + dy) and a right
/// bottom corner at (right + dx, bottom + dy). The revised rectangle is returned.
//
TRect&
TRect::Offset(int dx, int dy)
{
  left += dx;
  top += dy;
  right += dx;
  bottom += dy;
  return *this;
}

//
/// Inflates a rectangle inflated by the given delta arguments. 
/// The top left corner of the returned rectangle is (left - dx, top - dy),
/// while its bottom right corner is (right + dx, bottom + dy). 
//
TRect&
TRect::Inflate(int dx, int dy)
{
  left -= dx;
  top -= dy;
  right += dx;
  bottom += dy;
  return *this;
}
//
// Inflate the rectangle so that new top left point is (left-dx, top-dy) and
// the new bottom right point is (right+dx, bottom+dy).
//
TRect&
TRect::DeflateRect(int dx, int dy)
{
  return Inflate(-dx,-dy);
}

//
/// Changes this rectangle to its intersection with the other rectangle. This
/// rectangle object is returned. Returns a NULL rectangle if there is no
/// intersection.
//
TRect&
TRect::operator &=(const TRect& other)
{
  if (!IsNull()) {
    if (other.IsNull())
      SetNull();
    else {
      left = std::max(left, other.left);
      top = std::max(top, other.top);
      right = std::min(right, other.right);
      bottom = std::min(bottom, other.bottom);
    }
  }
  return *this;
}

//
/// Changes this rectangle to its union with the other rectangle. This rectangle
/// object is returned.
//
TRect&
TRect::operator |=(const TRect& other)
{
  if (!other.IsNull()) {
    if (IsNull())
      *this = other;
    else {
      left = std::min(left, other.left);
      top = std::min(top, other.top);
      right = std::max(right, other.right);
      bottom = std::max(bottom, other.bottom);
    }
  }
  return *this;
}

//
/// Determines the parts of this rect that do not lie within "other" region.  The
/// resulting rectangles are placed in the "result" array.
/// Returns the resulting number of rectangles.  This number will be 1, 2, 3, or 4.
//
int
TRect::Subtract(const TRect& other, TRect result[]) const
{
  // Case of non-intersection, result is just this rectangle
  //
  if (!Touches(other)) {
    result[0] = *this;
    return 1;
  }
  // Check for up to four sub-rectangles produced
  //
  else {
    int  i = 0;

    // Top piece of this rect
    //
    if (other.top > top) {
      result[i].left = left;
      result[i].top = top;
      result[i].right = right;
      result[i].bottom = other.top;
      i++;
    }

    // Bottom piece of this rect
    //
    if (other.bottom < bottom) {
      result[i].left = left;
      result[i].top = other.bottom;
      result[i].right = right;
      result[i].bottom = bottom;
      i++;
    }

    // Left piece of this rect
    //
    if (other.left > left) {
      result[i].left = left;
      result[i].top = std::max(top, other.top);
      result[i].right = other.left;
      result[i].bottom = std::min(bottom, other.bottom);
      i++;
    }

    // Right piece of this rect
    //
    if (other.right < right) {
      result[i].left = other.right;
      result[i].top = std::max(top, other.top);
      result[i].right = right;
      result[i].bottom = std::min(bottom, other.bottom);
      i++;
    }
    return i;
  }
}

} // OWL namespace

