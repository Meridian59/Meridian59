// ****************************************************************************
// OWL Extensions (OWLEXT) Class Library
// Copyright (C) 1999 by Dieter Windau
// All rights reserved
//
// colorhls.cpp: implementation file
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
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/colorhls.h>

namespace OwlExt {

using namespace owl;
using namespace std;


//
OWLEXTFUNC(void) DrawGradient(TDC& dc, int x, int y,
                const TColorHLS& color1, const TColorHLS& color2,
                int width, int height)
{
  PRECONDITION(width > 0);
  TColorHLS c1(color1);
  TColorHLS c2(color2);

  float dh = (c2.GetHue() - c1.GetHue()) / width;
  float dl = (c2.GetLuminance() - c1.GetLuminance()) / width;
  float ds = (c2.GetSaturation() - c1.GetSaturation()) / width;
  for (int i = 0; i < width; ++i) {
    TPen pen(TColor(c1.GetValue()));
    dc.SelectObject(pen);
    dc.MoveTo(x + i, y);
    dc.LineTo(x + i, y + height);
    dc.RestorePen();
    c1.SetHue(c1.GetHue() + dh);
    c1.SetLuminance(c1.GetLuminance() + dl);
    c1.SetSaturation(c1.GetSaturation() + ds);
  }
}

// *************************** TColorHLS **************************************

bool
TColorHLS::operator ==(const TColorHLS& other) const
{
  return (Hue == other.Hue &&
    Saturation == other.Saturation &&
    Luminance == other.Luminance);
}

//
// Return true if two colors are equal.
// This function compares between two binary representation of colors, it
// does not compare colors logically.
//
// For example, if palette entry 4 is solid red (rgb components (255, 0, 0)),
// The following will return false:
//   if (TColor(4) == TColor(255, 0, 0))
//     /* colors match */
//   else
//     /* colors do not match */
//
//  To actually compare the RGB values of two TColor objects, use the
//  operator == (COLORREF). For example,
//  TColor colorA, colorB;
//  if (colorA == colorB.GetValue()) {
//      // ColorA & ColorB
//  }
//

bool
TColorHLS::operator ==(const TColor& other) const
{
  return *this == RGBToHLS(other);
}

// RGB - functions

void
TColorHLS::SetRed(uint8 red)
{
  TColor colorRGB = HLSToRGB(*this);
  colorRGB.SetValue(RGB(red, colorRGB.Green(), colorRGB.Blue()));
  *this = RGBToHLS(colorRGB);
}

void
TColorHLS::SetGreen(uint8 green)
{
  TColor colorRGB = HLSToRGB(*this);
  colorRGB.SetValue(RGB(colorRGB.Red(), green, colorRGB.Blue()));
  *this = RGBToHLS(colorRGB);
}

void
TColorHLS::SetBlue(uint8 blue)
{
  TColor colorRGB = HLSToRGB(*this);
  colorRGB.SetValue(RGB(colorRGB.Red(), colorRGB.Green(), blue));
  *this = RGBToHLS(colorRGB);
}

void
TColorHLS::SetRGB(uint8 red, uint8 blue, uint8 green)
{
  *this = RGBToHLS(TColor(red, green, blue));
}

void
TColorHLS::SetHLS(float hue, float luminance, float saturation)
{
  SetHue(hue);
  SetLuminance(luminance);
  SetSaturation(saturation);
}
//
TColorHLS
TColorHLS::RGBToHLS(const TColor& colorRGB)
{
  uint8 red    = colorRGB.Red();
  uint8 green  = colorRGB.Green();
  uint8 blue   = colorRGB.Blue();
  uint8 minval = std::min(red, std::min(green, blue));
  uint8 maxval = std::max(red, std::max(green, blue));
  float mdiff  = float(maxval) - float(minval);
  float msum   = float(maxval) + float(minval);

  TColorHLS colorHLS;
  colorHLS.SetLuminance(msum / 510.0f);

  if (maxval == minval)
  {
    colorHLS.SetSaturation(0.0f);
    colorHLS.SetHue(0.0f);
  }
  else
  {
    float rnorm = (maxval - colorRGB.Red()  ) / mdiff;
    float gnorm = (maxval - colorRGB.Green()) / mdiff;
    float bnorm = (maxval - colorRGB.Blue() ) / mdiff;

    float saturation = ((colorHLS.GetLuminance() <= 0.5f) ?
      (mdiff / msum) : (mdiff / (510.0f - msum)));
    colorHLS.SetSaturation(saturation);

    float hue;
    if (colorRGB.Red() == maxval)   hue = 60.0f * (6.0f + bnorm - gnorm);
    if (colorRGB.Green() == maxval) hue = 60.0f * (2.0f + rnorm - bnorm);
    if (colorRGB.Blue() == maxval)  hue = 60.0f * (4.0f + gnorm - rnorm);
    while (hue > 360.0f)
      hue -= 360.0f;
    while (hue < 0.0f)
      hue += 360.0f;
    colorHLS.SetHue(hue);
  }
  return colorHLS;
}

//
TColor
TColorHLS::HLSToRGB(const TColorHLS& colorHLS)
{
  if (colorHLS.GetSaturation() == 0.0f)
  {
    return TColor(RGB((uint8)(colorHLS.GetLuminance() * 255.0f),
      (uint8)(colorHLS.GetLuminance() * 255.0f),
      (uint8)(colorHLS.GetLuminance() * 255.0f)));
  }
  else
  {
    float rm1, rm2;

    if (colorHLS.GetLuminance() <= 0.5f)
      rm2 = colorHLS.GetLuminance() +
      colorHLS.GetLuminance() * colorHLS.GetSaturation();
    else
      rm2 = colorHLS.GetLuminance() + colorHLS.GetSaturation() -
      colorHLS.GetLuminance() * colorHLS.GetSaturation();
    rm1 = 2.0f * colorHLS.GetLuminance() - rm2;
    return TColor(RGB(ToRGB1(rm1, rm2, colorHLS.GetHue() + 120.0f),
      ToRGB1(rm1, rm2, colorHLS.GetHue()),
      ToRGB1(rm1, rm2, colorHLS.GetHue() - 120.0f)));
  }
}

//
uint8
TColorHLS::ToRGB1(float rm1, float rm2, float rh)
{
  while (rh > 360.0f)
    rh -= 360.0f;
  while (rh < 0.0f)
    rh += 360.0f;

  if (rh <  60.0f)
    rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;
  else if (rh < 180.0f)
    rm1 = rm2;
  else if (rh < 240.0f)
    rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;

  return (uint8)(rm1 * 255.0f);
}

} // OwlExt namespace
//==========================================================