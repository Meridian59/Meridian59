// ****************************************************************************
// Copyright (C) 1999 by Dieter Windau
// All rights reserved
//
// colorhls.h  : header file
// Version:      1.0
// Date:         04/13/1999
// Author:       Dieter Windau
//
// TColorHLS is a freeware OWL class that represents a color in the HLS modell
//
// Portions of code are based on MFC class CColor written by Christian
// Rodemeyer. Very special thanks.
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// The code was tested using Microsoft Visual C++ 6.0 SR2 with OWL6 patch 5
// and Borland C++ 5.02 with OWL 5.02. Both under Windows NT 4.0 SP4.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://www.members.aol.com/softengage/index.htm
//
// ****************************************************************************
#if !defined(__OWLEXT_COLORHLS_H) && !defined(COLORHLS_H)
#define __OWLEXT_COLORHLS_H
#define COLORHLS_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H) && !defined (__OWLEXT_ALL_H)
# include <owlext/core.h>
#endif


namespace OwlExt {

class OWLEXTCLASS TColorHLS;

OWLEXTFUNC(void) DrawGradient(owl::TDC& dc, int x, int y, TColorHLS c1, TColorHLS c2,
                              int width, int height);

inline owl::ipstream& operator >>(owl::ipstream& is, TColorHLS& c);
inline owl::opstream& operator <<(owl::opstream& os, const TColorHLS& c);

// *************************** TColorHLS **************************************

class OWLEXTCLASS TColorHLS {
  public:
    // Same Constructors as TColor
    //
    TColorHLS();
    TColorHLS(const owl::TColor& src);
    TColorHLS(COLORREF value);
    TColorHLS(long value);
    TColorHLS(int r, int g, int b);
    TColorHLS(int r, int g, int b, int f);
    TColorHLS(int index);
    TColorHLS(const PALETTEENTRY & pe);
    TColorHLS(const RGBQUAD & q);
    TColorHLS(const RGBTRIPLE & t);

    // New Constructors
    //
    TColorHLS(const TColorHLS& src);
    TColorHLS(float hue, float luminance, float saturation);

    // Conversion operator & value accessor
    //
    operator      COLORREF() const;
    COLORREF      GetValue() const;
    void          SetValue(const COLORREF& value);

    // Comparison operators
    //
    bool operator ==(const TColorHLS& other) const;
    bool operator !=(const TColorHLS& other) const;
    bool operator ==(const owl::TColor& other) const;
    bool operator !=(const owl::TColor& other) const;

    TColorHLS& operator =(const TColorHLS& src);

    // Accessors
    //
    owl::TColor Rgb() const;
    owl::uint8  Red() const;
    owl::uint8  Green() const;
    owl::uint8  Blue() const;

    // RGB - functions
    //
    void  SetRed(owl::uint8 red);     // 0..255
    void  SetGreen(owl::uint8 green); // 0..255
    void  SetBlue(owl::uint8 blue);   // 0..255
    void  SetRGB(owl::uint8 red, owl::uint8 blue, owl::uint8 green);

    // HLS - functions
    //
    void  SetHue(float hue);               // 0.0 .. 360.0
    void  SetLuminance(float luminance);   // 0.0 .. 1.0
    void  SetSaturation(float saturation); // 0.0 .. 1.0
    void  SetHLS(float hue, float luminance, float saturation);
    float GetHue() const;        // 0.0 .. 360.0
    float GetLuminance() const;  // 0.0 .. 1.0
    float GetSaturation() const; // 0.0 .. 1.0

    static owl::TColor    HLSToRGB(const TColorHLS& colorHLS);
    static TColorHLS RGBToHLS(const owl::TColor& colorRGB);
    static owl::uint8     ToRGB1(float rm1, float rm2, float rh);

  private:
    float Hue;         // 0.0 .. 360.0  // angle
    float Luminance;   // 0.0 .. 1.0    // percent
    float Saturation;  // 0.0 .. 1.0    // percent

  // Don't work under VC++
    friend owl::ipstream& operator >>(owl::ipstream& is, TColorHLS& c);
    friend owl::opstream& operator <<(owl::opstream& os, const TColorHLS& c);

};

// ************************** inlines *****************************************

//
// Construct a null color.
//
inline
TColorHLS::TColorHLS()
{
  Hue = Luminance = Saturation = 0.0f;
}

//
// Construct a color as a copy of another RGB color.
//
inline
TColorHLS::TColorHLS(const owl::TColor& src)
{
  *this = RGBToHLS(src);
}

//
// Construct a color with an existing COLORREF value.
//
inline
TColorHLS::TColorHLS(COLORREF value)
{
  *this = RGBToHLS(owl::TColor(value));
}

//
// Construct a color given a particular system value representation.
//
inline
TColorHLS::TColorHLS(long value)
{
  *this = RGBToHLS(owl::TColor(static_cast<COLORREF>(value)));
}

//
// Construct a color given the red, green, and blue components of color.
//
inline
TColorHLS::TColorHLS(int r, int g, int b)
{
  *this = RGBToHLS(owl::TColor(r, g, b));
}

//
// Construct a color given red, green, blue, and flag components of color.
// The flag component is defined in the Windows SDK reference.
//
inline
TColorHLS::TColorHLS(int r, int g, int b, int f)
{
  *this = RGBToHLS(owl::TColor(r, g, b, f));
}

//
// Construct a color that is an index into a palette.
//
inline
TColorHLS::TColorHLS(int index)
{
  *this = RGBToHLS(owl::TColor::CreateFromPaletteIndex(index));
}

//
// Construct a color based on an existing PALETTEENTRY.
//
inline
TColorHLS::TColorHLS(const PALETTEENTRY & pe)
{
  *this = RGBToHLS(owl::TColor(pe));
}

//
// Construct a color based on an existing RGBQUAD.
//
inline
TColorHLS::TColorHLS(const RGBQUAD & q)
{
  *this = RGBToHLS(owl::TColor(q));
}

//
// Construct a color based on an existing RGBTRIPLE.
//
inline
TColorHLS::TColorHLS(const RGBTRIPLE & t)
{
  *this = RGBToHLS(owl::TColor(t));
}

//
// Construct a color as a copy of another HLS color.
//
inline
TColorHLS::TColorHLS(const TColorHLS& src)
{
  *this = src;
}

//
// Construct a color based on an existing hue, luminance and saturation
//
inline
TColorHLS::TColorHLS(float hue, float luminance, float saturation)
{
  SetHLS(hue, luminance, saturation);
}

//
// Convert the stored color into a COLORREF.
//
inline
TColorHLS::operator COLORREF() const
{
  return HLSToRGB(*this);
}

inline COLORREF
TColorHLS::GetValue() const
{
  return HLSToRGB(*this).GetValue();
}

//
// Change the color after it has been constructed.
//
inline void
TColorHLS::SetValue(const COLORREF& value)
{
  *this = RGBToHLS(owl::TColor(value));
}

inline bool
TColorHLS::operator !=(const TColorHLS& other) const
{
  return !(*this == other);
}

//
// Return true if two colors are not equal.
// See additional comments in TColor::operator ==.
//
inline bool
TColorHLS::operator !=(const owl::TColor& other) const
{
  return !(*this == other);
}

//
// Set the value of color after it has been constructed.
//
inline TColorHLS&
TColorHLS::operator =(const TColorHLS& src)
{
  if(this != &src){
    Hue = src.Hue;
    Saturation = src.Saturation;
    Luminance = src.Luminance;
  }
  return *this;
}

//
// Return the RGB color value.
//
inline owl::TColor
TColorHLS::Rgb() const
{
  return HLSToRGB(*this);
}

inline owl::uint8
TColorHLS::Red() const
{
  return HLSToRGB(*this).Red();
}

inline owl::uint8
TColorHLS::Green() const
{
  return HLSToRGB(*this).Green();
}

inline owl::uint8
TColorHLS::Blue() const
{
  return HLSToRGB(*this).Blue();
}

// HLS - functions

inline void
TColorHLS::SetHue(float hue)
{
  PRECONDITION(hue >= 0.0 && hue <= 360.0);
  Hue = hue;
}

inline void
TColorHLS::SetLuminance(float luminance)
{
  PRECONDITION(luminance >= 0.0 && luminance <= 1.0);
  Luminance = luminance;
}

inline void
TColorHLS::SetSaturation(float saturation)
{
  PRECONDITION(saturation >= 0.0 && saturation <= 1.0);
  Saturation = saturation;
}

inline float
TColorHLS::GetHue() const
{
  return Hue;
}

inline float
TColorHLS::GetLuminance() const
{
  return Luminance;
}

inline float
TColorHLS::GetSaturation() const
{
  return Saturation;
}

//
// Extract the color value from a persistent input stream.
//
inline owl::ipstream& operator >>(owl::ipstream& is, TColorHLS& c)
{
  is >> c.Hue >> c.Luminance >> c.Saturation;
  return is;
}

//
// Insert the color value into a persistent output stream.
//
inline owl::opstream& operator <<(owl::opstream& os, const TColorHLS& c)
{
  os << c.Hue << c.Luminance << c.Saturation;
  return os;
}

} // OwlExt namespace

#endif // __OWLEXT_COLORHLS_H

