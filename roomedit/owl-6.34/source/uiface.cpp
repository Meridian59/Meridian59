//---------------------------------------------------------------------------- 
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implements TUIFace
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/uihelper.h>
#include <owl/gdiobjec.h>

#if defined(BI_MULTI_THREAD_RTL)
#include <owl/thread.h>
#endif



namespace owl {

OWL_DIAGINFO;

const long RopPSDPxax = 0x00B8074AL;  //
const long RopDSPDxax = 0x00E20746L;  //

struct TUIFaceData 
#if defined(BI_MULTI_THREAD_RTL)
                : public TLocalObject
#endif
{
  TUIFaceData()
    :Brush(THatch8x8Brush::Hatch11F1,TColor::Sys3dFace,TColor::Sys3dHilight),
    DitherColor(TColor::Sys3dHilight)
    {
    }
  ~TUIFaceData()
    {
    }
  
  THatch8x8Brush& GetDitherBrush();
  TBitmap&        GetGlyphMask(const TSize& minSize);
  
  THatch8x8Brush    Brush;
  TColor            DitherColor;
  TSize              MaskSize;   // Current mask size
  TPointer<TBitmap> MaskBm;      // Current mask bm

#if defined(BI_MULTI_THREAD_RTL)
//  TMRSWSection  Lock;
#endif
};

//
// Static instance of the colors 
//
static TUIFaceData& GetUIFaceData() 
{
#if defined(BI_MULTI_THREAD_RTL)
  static TTlsContainer<TUIFaceData> uiFaceData;
  return uiFaceData.Get();
#else
  static TUIFaceData uiFaceData;
  return uiFaceData;
#endif
};

namespace
{
  //
  // Ensure singleton initialization at start-up (single-threaded, safe).
  //
  TUIFaceData& InitUIFaceData = GetUIFaceData(); 
}

#if defined(BI_MULTI_THREAD_RTL)
#define LOCKBRUSH //TMRSWSection::TLock Lock(GetUIFaceData().Lock);
#else
#define LOCKBRUSH 
#endif


THatch8x8Brush& TUIFaceData::GetDitherBrush()
{
  LOCKBRUSH

  // Reconstruct the brush in case the system colors have changed
  // since the brush was constructed
  //
  if (DitherColor != TColor::Sys3dHilight) {
    Brush.Reconstruct(THatch8x8Brush::Hatch11F1,
                                TColor::Sys3dFace,TColor::Sys3dHilight);
    DitherColor = TColor::Sys3dHilight;
  }
  return Brush;
}
//
TBitmap& TUIFaceData::GetGlyphMask(const TSize& minSize)
{
  LOCKBRUSH
  // Start off with no bitmap, allocate below with requested or default size
  //
  // (Re)allocate mask bitmap if we need a bigger one
  //
  if (minSize.cx > MaskSize.cx || minSize.cy > MaskSize.cy) {
    if (MaskSize.cx < minSize.cx)
      MaskSize.cx = minSize.cx;
    if (MaskSize.cy < minSize.cy)
      MaskSize.cy = minSize.cy;
    MaskBm = new TBitmap(MaskSize.cx, MaskSize.cy, 1, 1, 0);
  }
  return *MaskBm;
}

/// Internal static helper function that can be used alone.
//
/// Return a brush created using a hatched [checkerboard] pattern of the button
/// highlight color. This brush is commonly used for rendering 'indeterminate'
/// or 'mixed-value' appearances.
///
/// \note Automatically reconstructs the brush if the relevant system color
///       changed during a session...
//
THatch8x8Brush&
TUIFace::GetDitherBrush()
{
  // Return ref. to static brush
  //
  return GetUIFaceData().GetDitherBrush();
}

/// Internal static helper function that can be used alone.
//
/// Return a reference to a static monochrome bitmap. The optional TSize
/// pointer allows the routine to ensure that the bitmap is big enough for a
/// given use.
//
TBitmap&
TUIFace::GetGlyphMask(const TSize& minSize)
{
  return GetUIFaceData().GetGlyphMask(minSize);
}

/// Internal static helper function that can be used alone.
//
/// Build a monochrome mask bitmap for the glyph that has 1's where color
/// maskedColor is and 0's everywhere else.
///
/// \note Assumes DC's are already setup - i.e. maskDC has the destination
///       monochrome bitmap selected in it and glyphDC has the image selected
///       in it..
//
void
TUIFace::BuildMask(TDC& maskDC, const TPoint& maskDst, const TSize& maskSize,
                   TDC& glyphDC, const TRect& glyphRect,
                   const TColor& maskedColor)
{
  maskDC.PatBlt(0, 0, maskSize.cx, maskSize.cy, WHITENESS);
  if (maskedColor != TColor::None) {
    TColor bkColor = glyphDC.SetBkColor(maskedColor);
    maskDC.BitBlt(maskDst.x, maskDst.y, glyphRect.Width(), glyphRect.Height(),
                  glyphDC, glyphRect.left, glyphRect.top, SRCCOPY);
    glyphDC.SetBkColor(bkColor);
  }
}

/// Internal static helper function that can be used alone.
//
/// Paint onto a given DC withing a given rect using a monochrome BM in a dc as
/// a stencil and an arbitrary brush
//
void
TUIFace::FillBackground(TDC& dc, TDC& maskDC, const TRect& dstRect,
                        const TBrush& brush)
{
  dc.SelectObject(brush);
  TColor txtClr = dc.SetTextColor(TColor::Black);
  TColor bkClr = dc.SetBkColor(TColor::White);

  dc.BitBlt(dstRect, maskDC, TPoint(0, 0), RopDSPDxax);

  dc.SetTextColor(txtClr);
  dc.SetBkColor(bkClr);
  dc.RestoreBrush();
}

/// Internal static helper function that can be used alone.
//
/// Tiles the rectangle with a even dithered (checkerboard) pattern
/// maskDC determines the stencil area of dither
//
void
TUIFace::DitherBackground(TDC& dc, TDC& maskDC, const TRect& dstRect)
{
  FillBackground(dc, maskDC, dstRect, GetDitherBrush());
}

/// Internal static helper function that can be used alone.
//
/// An extension to ::DrawText that draws etched text by writing twice, the
/// first one offset down & right in a light color, and the second not offset
//
int
TUIFace::DrawTextEtched(TDC& dc, const tstring& str, int count,
                        const TRect& rect, uint16 format)
{
  int mode = dc.SetBkMode(TRANSPARENT);
  TColor txtColor = dc.SetTextColor(TColor::Sys3dHilight);
  int height = dc.DrawText(str, count, rect.OffsetBy(1, 1), format);
  if (!(format & DT_CALCRECT)) {
    dc.SetTextColor(TColor::Sys3dShadow);
    height = dc.DrawText(str, count, rect, format);
  }
  dc.SetTextColor(txtColor);
  dc.SetBkMode(mode);
  return height;
}

//
/// Internal bitmap painting function
//
void
TUIFace::PaintBm(const TBitmap& glyph, const TRect& glyphSrcRect,
                 TDC& dc, const TRect& faceRect, const TPoint& dstPoint,
                 TState state, bool pressed, bool fillFace,
                 const TColor& maskColor, const TColor& fillColor)
{
  // Select our glyph in a working memory DC.
  //
  TMemoryDC memDC(CONST_CAST(TBitmap&,glyph));
  memDC.SetTextColor(TColor::Black);
  memDC.SetBkColor(TColor::White);

  // Calculate the glyph's destination rect.
  // Calculate the rect to fill in, depending on fill-face flag.
  //
  TRect glyphDstRect(faceRect.TopLeft() + dstPoint, glyphSrcRect.Size());
  glyphDstRect &= faceRect;
  TPoint dstPt(dstPoint);

  // When the glyph is down, offset it's origin one pixel to the lower right.
  // Make sure the glyph rect doesn't hang outside the face.
  //
  if (pressed || state == Down) {
    dstPt.Offset(1,1);
    glyphDstRect.Offset(1,1);
    glyphDstRect &= faceRect;
  }

  // Build the mask. The mask is always the size of the face, while
  // the glyph may be offset (given by dstPt) within the mask.
  //
  TBitmap& mask = GetGlyphMask(faceRect.Size());
  TMemoryDC maskDC(mask);
  BuildMask(maskDC, dstPt, faceRect.Size(), memDC, glyphSrcRect, maskColor);

  // Draw according the specified state.
  //
  if (state == Disabled)
  {
    // Convert the highlight color to 1's on the mask.
    // This in effect makes all the highlighted (white) areas transparent
    // and hence more readable.
        //
    memDC.SetBkColor(TColor::Sys3dHilight);
    maskDC.BitBlt(glyphSrcRect, memDC, TPoint(0,0), SRCPAINT);

    // Gray the image using the standard button shadow color.
    // (glyphDstRect in dc <-- dstPt in maskDC)
        //
    TBrush shadowBrush(TColor::Sys3dShadow);
    dc.SelectObject(shadowBrush);
    TColor bg = dc.SetBkColor(TColor::White);
    dc.BitBlt(glyphDstRect, maskDC, dstPt, RopPSDPxax);
    dc.SetBkColor(bg);
      }
  else // not Disabled
  {
    // Do a transparent blit using the mask. 
    // (glyphDstRect in dc <-- glyphSrcRect.TopLeft in memDC/mask)
      //
    const uint32 DSTCOPY = 0x00AA0029; // Undocumented ROP
    dc.MaskBlt
    (
      glyphDstRect, 
      memDC, glyphSrcRect.TopLeft(), 
      mask, dstPt, 
      MAKEROP4(DSTCOPY, SRCCOPY)
    );
    }

  // Do any filling determined by the state and fill arguments.
      //
  if (state == Indeterm)
  {
    // Dither the background everywhere except where the glyph is.
    // (faceRect in dc <-- masked dithering)
      //
      DitherBackground(dc, maskDC, faceRect);
    }
  else if (fillFace && fillColor != TColor::None) 
  {
    // Fill background around mask with given button face color.
    // (faceRect in dc <-- masked fill)
      //
    TBrush btnFaceBrush(fillColor);
    FillBackground(dc, maskDC, faceRect, btnFaceBrush);
  }

      maskDC.RestoreBitmap();
}


/// Internal static helper function that can be used alone.
void
TUIFace::PaintIcon(const TIcon& icon, const TRect& glyphSrcRect,
                   TDC& dc, const TRect& faceRect, const TPoint& dstPt,
                   TState state, bool pressed, bool fillFace,
                   const TColor& maskColor, const TColor& fillColor)
{
  ICONINFO ii = icon.GetIconInfo();
  TBitmap glyph(TBitmap(ii.hbmColor)); // not work for B/W icon !!!!!!!!!!!!!!!!!!!!!

  PaintBm(glyph, glyphSrcRect,dc, faceRect, dstPt, state, pressed, fillFace,
          maskColor, fillColor);
}

//
/// Internal static helper function that can be used alone.
//
void
TUIFace::PaintText(const tstring& text, TDC& dstDC, const TRect& faceRect,
                   const TPoint& dstPnt, TState state, bool pressed,
                   bool fillFace, const TColor& fillColor, uint16 format)
{
  TRect   textDstRect(faceRect);
  TPoint   dstPt(dstPnt);

  int mode = dstDC.SetBkMode(fillFace ? OPAQUE  : TRANSPARENT);

  if(pressed || state == Down){
    dstPt.Offset(1,1);
    textDstRect.left++;
    if(format & DT_CENTER)
      textDstRect.left++;
    textDstRect.top++;
    if(format & DT_VCENTER)
      textDstRect.top++;
  }

  // Draw according the specified state
  //
  switch (state) {

    case Normal: {
      TColor bkColor;
      if (fillFace){
        bkColor = dstDC.SetBkColor(fillColor);
        dstDC.ExtTextOut(0, 0, ETO_OPAQUE, &faceRect, _T(""), 0);
      }
      dstDC.DrawText(text,-1, textDstRect, format);
      if (fillFace)
         dstDC.SetBkColor(bkColor);
      break;
    }

    case Down: {

      dstDC.SetBkMode(TRANSPARENT);

      TColor bkColor = dstDC.GetBkColor();
      if (fillFace){
        bkColor = dstDC.SetBkColor(fillColor);
        if(pressed)
          dstDC.ExtTextOut(dstPt, ETO_OPAQUE, &faceRect, _T(""), 0);
      }

      // Build a mask of the glyph & dither the background around the glyph
      //
      if(!pressed){

        TMemoryDC memDC(dstDC);
        TMemoryDC maskDC(GetGlyphMask(faceRect.Size()));
        BuildMask(maskDC, dstPt, faceRect.Size(), memDC, faceRect,
                  fillFace?fillColor:bkColor);
         DitherBackground(dstDC, maskDC, faceRect);
        dstDC.DrawText(text, -1, textDstRect, format);
      }
      else
        dstDC.DrawText(text, -1, textDstRect, format);

      if (fillFace)
         dstDC.SetBkColor(bkColor);
      break;
    }

    case Indeterm: {
      TColor bkColor;
      if (fillFace){
        bkColor = dstDC.SetBkColor(fillColor);
        dstDC.ExtTextOut(dstPt, ETO_OPAQUE, &faceRect, _T(""), 0);
      }
      dstDC.GrayString(TBrush(dstDC.GetTextColor()),0,text, -1, textDstRect);
      if (fillFace)
         dstDC.SetBkColor(bkColor);
      break;
    }

    case Disabled: {
      TColor bkColor;
      if (fillFace){
        bkColor = dstDC.SetBkColor(fillColor);
        dstDC.ExtTextOut(dstPt, ETO_OPAQUE, &faceRect, _T(""), 0);
      }
      TUIFace::DrawTextEtched(dstDC, text, -1, textDstRect, format);
      if (fillFace)
         dstDC.SetBkColor(bkColor);
      break;
    }
    case Default:
      dstDC.DrawText(text,-1, textDstRect, format);
      break;
  }
  dstDC.SetBkMode(mode);
}

//
/// Paint the face of a button onto a DC.
/// Assumes the entire bitmap is the face.
//
void
TUIFace::Paint(TDC& dc, const TPoint& pt, TState state, bool pressed,
               bool fillFace)
{
  if (SourceType==sText)
    PaintText(Text, dc, Rect, pt, state, pressed, fillFace, FillColor, (uint16)Format);
  else if (SourceType==sBitmap)
    PaintBm(*Bm, TRect(TPoint(0,0),Bm->Size()), dc, Rect, pt, state, pressed,
            fillFace, BkgndColor, FillColor);
  else if (SourceType==sIcon)
    PaintIcon(*Icon, TRect(TPoint(0,0),Bm->Size()), dc, Rect, pt, state, pressed,
              fillFace, BkgndColor, FillColor);
}

//
/// Paint a portion of the source bitmap onto a DC.
/// This allows one bitmap to be the source for multiple TUIFaces.
//
void
TUIFace::Paint(TDC& dc, const TRect& srcRect, const TPoint& pt, TState state,
               bool pressed, bool fillFace)
{
  if (SourceType==sText)
    PaintText(Text, dc, Rect, pt, state, pressed, fillFace, FillColor, (uint16)Format);
  else if (SourceType==sBitmap)
    PaintBm(*Bm, srcRect, dc, Rect, pt, state, pressed,
            fillFace, BkgndColor, FillColor);
  else if (SourceType==sIcon)
    PaintIcon(*Icon, srcRect, dc, Rect, pt, state, pressed,
              fillFace, BkgndColor, FillColor);
}

#if !defined(BI_COMP_GNUC)
#pragma warn -par
#endif
//
/// Encapsulates and emulate the 32-bit DrawState API.
//
bool
TUIFace::Draw(TDC& dc, HBRUSH hbr, LPARAM lp, WPARAM wp,
              int x, int y, int cx, int cy, uint flags)
{
  static bool hasDrawState = true;

  // Try once to see if the API call is available. If not, do ourselves.
  //
  if (hasDrawState) {
    if (::DrawState(dc, hbr, 0, lp, wp, x, y, cx, cy, flags))
      return true;
    if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
      hasDrawState = false;  // No DrawState(). Don't try again, just emulate
    else
      return false;  // Some other error, just return
  }

  return false;
}
#if !defined(BI_COMP_GNUC)
#pragma warn .par
#endif

/// Internal static helper function that can be used alone.
//
// !CQ needs updating
/// Paint the mask onto a DC.
//
void
TUIFace::PaintMaskBm(const TBitmap& glyph, TDC& dc, const TPoint& pt,
                     const TColor& maskColor)
{
  // Create a DC to hold the image bitmap
  //
  TMemoryDC memDC;
  memDC.SelectObject(glyph);

  // Create a DC to hold an "AND mask" of image and build the mask
  //
  TMemoryDC maskDC;
  maskDC.SelectObject(GetGlyphMask(glyph.Size()));

  // Rectangle to hold mask of glyph
  //
  TRect  maskRect(TPoint(0,0), glyph.Size());

  // Build the mask
  //
  BuildMask(maskDC, TPoint(0, 0), glyph.Size(), memDC, maskRect, maskColor);

  // Make sure area under highlight color ends up fill color
  //
  // dc.TextRect(maskRect, FillColor);

  // Inactivate mask--convert the highlight color to 1's on existing mask
  //
  memDC.SetBkColor(TColor::Sys3dHilight);
  maskDC.BitBlt(maskRect, memDC, maskRect.TopLeft(), SRCPAINT);

  dc.BitBlt(TRect(pt, maskRect.Size()), maskDC, TPoint(0,0));
}

//
/// Paint the mask onto a DC.
//
void
TUIFace::PaintMask(TDC& dc, const TPoint& pt)
{
  // ::DrawState()...

  if(SourceType==sBitmap)
    PaintMaskBm(*Bm, dc, pt, BkgndColor);
  //else if (SourceType==sText)
  //  PaintMaskText(Text, dc, pt);
}


} // OWL namespace
/* ========================================================================== */
