//
/// \file gdiplus.h
/// Utilities for GDI+ interoperability
//
// Part of OWLNext - the next generation Object Windows Library 
// Copyright (c) 2014 Vidar Hasfjord 
//
// For more information, including license details, see 
// http://owlnext.sourceforge.net
//

#ifndef OWL_GDIPLUS_H
#define OWL_GDIPLUS_H

#include <owl/geometry.h>
#include <owl/color.h>

//
// GDI+ assumes Windows' macros `min` and `max` are defined, but these are
// disabled by OWLNext. We pull in std::min and max instead here.
//
#include <algorithm>
using std::min;
using std::max; 

#include <gdiplus.h>
#include <cmath>

#pragma comment(lib, "gdiplus")

namespace owl {

namespace detail {

  //
  // TODO: Eliminate this helper when all compilers support std::lround.
  //
  inline long lround(float v)
  {
    return static_cast<long>(v < 0 ? v - 0.5f : v + 0.5f);
  }

} // namespace

//
/// ConvertToGdiplus template;
/// Converts any of TSize, TPoint and TRect to its counter-part in GDI+;
/// Gdiplus::Size/SizeF, Point/PointF and Rect/RectF.
/// @{
//
template <class TDest, class T>
TDest ConvertToGdiplus(const T&); 

template <>
inline Gdiplus::Size ConvertToGdiplus<Gdiplus::Size>(const TSize& v)
{
  return Gdiplus::Size(v.cx, v.cy);
}

template <>
inline Gdiplus::SizeF ConvertToGdiplus<Gdiplus::SizeF>(const TSize& v)
{
  return Gdiplus::SizeF(static_cast<Gdiplus::REAL>(v.cx), static_cast<Gdiplus::REAL>(v.cy));
}

template <>
inline Gdiplus::Point ConvertToGdiplus<Gdiplus::Point>(const TPoint& v)
{
  return Gdiplus::Point(v.x, v.y);
}

template <>
inline Gdiplus::PointF ConvertToGdiplus<Gdiplus::PointF>(const TPoint& v)
{
  return Gdiplus::PointF(static_cast<Gdiplus::REAL>(v.x), static_cast<Gdiplus::REAL>(v.y));
}

template <>
inline Gdiplus::Rect ConvertToGdiplus<Gdiplus::Rect>(const TRect& v)
{
  return Gdiplus::Rect(v.left, v.top, v.Width(), v.Height());
}

template <>
inline Gdiplus::RectF ConvertToGdiplus<Gdiplus::RectF>(const TRect& v)
{
  return Gdiplus::RectF(
    static_cast<Gdiplus::REAL>(v.left), 
    static_cast<Gdiplus::REAL>(v.top),
    static_cast<Gdiplus::REAL>(v.Width()), 
    static_cast<Gdiplus::REAL>(v.Height())
    );
}

/// @}

//
/// ConvertToGdiplus overload;
/// Converts TColor to Gdiplus::Color with the given alpha channel. If the
/// latter is not specified, the alpha channel is fully saturated (opaque).
//
inline Gdiplus::Color ConvertToGdiplus(const TColor& v, BYTE alphaChannel = 255)
{
  return Gdiplus::Color(alphaChannel, v.Red(), v.Green(), v.Blue()); 
}

//
/// ConvertFromGdiplus overloads;
/// Converts any of Gdiplus::Size/SizeF, Point/PointF, Rect/RectF and Color to 
/// its counter-part in OWLNext; TSize, TPoint, TRect and TColor.
/// @{ 
//
inline TSize ConvertFromGdiplus(const Gdiplus::Size& v)
{
  return TSize(v.Width, v.Height);
}

inline TSize ConvertFromGdiplus(const Gdiplus::SizeF& v)
{
  return TSize(detail::lround(v.Width), detail::lround(v.Height));
}

inline TPoint ConvertFromGdiplus(const Gdiplus::Point& v)
{
  return TPoint(v.X, v.Y);
}

inline TPoint ConvertFromGdiplus(const Gdiplus::PointF& v)
{
  return TPoint(detail::lround(v.X), detail::lround(v.Y));
}

inline TRect ConvertFromGdiplus(const Gdiplus::Rect& v)
{
  return TRect(TPoint(v.X, v.Y), TSize(v.Width, v.Height));
}

inline TRect ConvertFromGdiplus(const Gdiplus::RectF& v)
{
  return TRect(
    TPoint(detail::lround(v.X), detail::lround(v.Y)), 
    TSize(detail::lround(v.Width), detail::lround(v.Height)));
}

//
/// Converts Gdiplus::Color to TColor. Note that the alpha channel of the color
/// value is discarded, since TColor does not support alpha blending.
//
inline TColor ConvertFromGdiplus(const Gdiplus::Color& v)
{
  return TColor(v.GetRed(), v.GetGreen(), v.GetBlue()); 
}

/// @}

//
/// gdiplus_cast template;
/// Simulates a cast operator that can convert to and from any of OWLNext types
/// TSize, TPoint, TRect and TColor, and its counter-part in GDI+; 
/// Gdiplus::Size/SizeF, Point/PointF, Rect/RectF and Color. This template is
/// an alternative to the more specific conversion functions ConvertToGdiplus 
/// and ConvertFromGdiplus.
/// @{
//
template <class TDest, class T>
TDest gdiplus_cast(const T&);

template <>
inline Gdiplus::Size gdiplus_cast<Gdiplus::Size>(const TSize& v)
{
  return ConvertToGdiplus<Gdiplus::Size>(v);
}

template <>
inline Gdiplus::SizeF gdiplus_cast<Gdiplus::SizeF>(const TSize& v)
{
  return ConvertToGdiplus<Gdiplus::SizeF>(v);
}

template <>
inline Gdiplus::Point gdiplus_cast<Gdiplus::Point>(const TPoint& v)
{
  return ConvertToGdiplus<Gdiplus::Point>(v);
}

template <>
inline Gdiplus::PointF gdiplus_cast<Gdiplus::PointF>(const TPoint& v)
{
  return ConvertToGdiplus<Gdiplus::PointF>(v);
}

template <>
inline Gdiplus::Rect gdiplus_cast<Gdiplus::Rect>(const TRect& v)
{
  return ConvertToGdiplus<Gdiplus::Rect>(v);
}

template <>
inline Gdiplus::RectF gdiplus_cast<Gdiplus::RectF>(const TRect& v)
{
  return ConvertToGdiplus<Gdiplus::RectF>(v);
}

template <>
inline Gdiplus::Color gdiplus_cast<Gdiplus::Color>(const TColor& v)
{
  return ConvertToGdiplus(v, 255);
}

template <>
inline TSize gdiplus_cast<TSize>(const Gdiplus::Size& v)
{
  return ConvertFromGdiplus(v);
}

template <>
inline TSize gdiplus_cast<TSize>(const Gdiplus::SizeF& v)
{
  return ConvertFromGdiplus(v);
}

template <>
inline TPoint gdiplus_cast<TPoint>(const Gdiplus::Point& v)
{
  return ConvertFromGdiplus(v);
}

template <>
inline TPoint gdiplus_cast<TPoint>(const Gdiplus::PointF& v)
{
  return ConvertFromGdiplus(v);
}

template <>
inline TRect gdiplus_cast<TRect>(const Gdiplus::Rect& v)
{
  return ConvertFromGdiplus(v);
}

template <>
inline TRect gdiplus_cast<TRect>(const Gdiplus::RectF& v)
{
  return ConvertFromGdiplus(v);
}

template <>
inline TColor gdiplus_cast<TColor>(const Gdiplus::Color& v)
{
  return ConvertFromGdiplus(v);
}

/// @}

} // OWL namespace

#endif
