//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// OWL Static bitmap class for dialogs
//
// Original code:
// Written by Richard Brown
// Richard@TowerSoftware.com
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/staticbm.h>

using namespace owl;

namespace OwlExt {


// -------------------------------------------------------------
TStaticBitmap::TStaticBitmap(TWindow* parent,TBitmap* user_bitmap,
               TPalette* user_palette, int id, int x, int y, int width, int height,
               bool selected, bool use_aspect, bool use_mask)
               :
TStatic(parent,id),
Bitmap(user_bitmap),
Palette(user_palette),
X(x), Y(y), Width(width), Height(height),
Selected(selected),
UseAspect(use_aspect),
prevSelected(false),
MaskBitmap(NULL),
MaskUsed(use_mask)
{ }
TStaticBitmap::~TStaticBitmap()
{
  delete MaskBitmap;
}

// -------------------------------------------------------------

void
TStaticBitmap::UpdateBitmap(TBitmap* user_bitmap,int x,int y,int width,
              int height,bool use_mask)
{
  if (user_bitmap)
    Bitmap=user_bitmap;
  X=x;
  Y=y;
  Width=width;
  Height=height;
  delete MaskBitmap;
  MaskBitmap=NULL;
  MaskUsed=use_mask;
  Invalidate();
}
void TStaticBitmap::SetupWindow()
{
  TStatic::SetupWindow();
  ClearFlag(wfPredefinedClass); // Allow painting
}
void TStaticBitmap::SetText(LPTSTR text)
{
  SetRedraw(false);       // Supress the text painting
  TStatic::SetText(text); // Base class text setting
  SetRedraw(true);        // Allow repainting
  Invalidate();           // Force repaint
}
void TStaticBitmap::Select(bool flag)
{
  if (flag==false)
    prevSelected=true;
  Selected=flag;
  Invalidate();
}

/* -------------------------------------------------------------
Paint the static bitmap
Uses a mask and aspect ratio if required
The supplied TRect is ignored, because its not always possible
to stretch into a partial rectangle
------------------------------------------------------------- */
void TStaticBitmap::Paint(TDC& dc,bool,TRect&)
{
  if (Bitmap)
  {
    TMemoryDC memDC(dc);
    TRect destRect=GetClientRect();
    if (Palette)
    {
      dc.SelectObject(*Palette);
      dc.RealizePalette();
      memDC.SelectObject(*Palette);
      memDC.RealizePalette();
    }
    memDC.SelectObject(*Bitmap);
    TRect srcRect(X,Y,X+Width,Y+Height);
    int x=0;
    int y=0;
    int dest_width=destRect.Width();
    int dest_height=destRect.Height();
    long aspect=((long)Width<<8)/Height;
    long dest_aspect=((long)dest_width<<8)/dest_height;
    TBrush brush0(TColor(GetSysColor(COLOR_BTNFACE)));
    dc.SelectObject(brush0);
    dc.PatBlt(destRect,PATCOPY);
    if (UseAspect)
    {
      if (aspect<256)
      {
        // bitmap is taller than wide
        if (aspect>dest_aspect)
        {
          dest_aspect=dest_width;
          dest_aspect<<=8;
          dest_aspect/=aspect;
          dest_height=dest_aspect;
          y+=(destRect.Height()-dest_height)>>1;
        }
        else
        {
          dest_aspect=dest_height;
          dest_aspect*=aspect;
          dest_aspect>>=8;
          dest_width=dest_aspect;
          x+=(destRect.Width()-dest_width)>>1;
        }
      }
      else
      {
        // source bitmap is wider than tall
        if (dest_aspect>aspect)
        {
          dest_aspect=aspect;
          dest_aspect*=dest_height;
          dest_aspect>>=8;
          dest_width=dest_aspect;
          x+=(destRect.Width()-dest_width)>>1;
        }
        else
        {
          dest_aspect=dest_width;
          dest_aspect<<=8;
          dest_aspect/=aspect;
          dest_height=dest_aspect;
          y+=(destRect.Height()-dest_height)>>1;
        }
      }
    }
    if (MaskUsed)
    {
      TMemoryDC maskDC(dc);
      if (MaskBitmap==NULL)
      {
        MaskBitmap=new TBitmap(Width,Height,1,1,NULL);
        maskDC.SelectObject(*MaskBitmap);
        memDC.SelectObject(*Bitmap);
        memDC.SetBkColor(TColor::Black);
        maskDC.BitBlt(0,0,Width,Height,memDC,X,Y,SRCCOPY);      // Build initial mask
      }

      maskDC.SelectObject(*MaskBitmap);
      memDC.SetBkColor(TColor::Black);
      if (Palette)
      {
        dc.SelectObject(*Palette,false);
        dc.RealizePalette();
        memDC.SelectObject(*Palette,false);
      }
      dc.SetStretchBltMode(COLORONCOLOR);
      dc.StretchBlt(x,y,dest_width,dest_height,maskDC,0,0,Width,Height,SRCAND);  // apply mask to screen
      dc.StretchBlt(x,y,dest_width,dest_height,memDC,X,Y,Width,Height,SRCPAINT);  // apply image to screen
    }
    else
    {
      // Render without a mask...
      TRect destRect2(x,y,dest_width+x,dest_height+y);
      dc.SetStretchBltMode(COLORONCOLOR);
      dc.StretchBlt(destRect2,memDC,srcRect);
    }

    // If Selected we draw a border...
    if (Selected || prevSelected)
    {
      prevSelected=false;
      dc.SetROP2(R2_NOT);
      int x=destRect.left-1;
      int y=destRect.top-1;
      dc.MoveTo(x-BORDER_OFFSET,y-BORDER_OFFSET);
      dc.LineTo(destRect.right+BORDER_OFFSET,y-BORDER_OFFSET);
      dc.LineTo(destRect.right+BORDER_OFFSET,destRect.bottom+BORDER_OFFSET);
      dc.LineTo(x-BORDER_OFFSET,destRect.bottom+BORDER_OFFSET);
      dc.LineTo(x-BORDER_OFFSET,y-BORDER_OFFSET);
    }
    if (Palette)
      dc.RestorePalette();
  }
}
bool TStaticBitmap::IsOver(TPoint& point)
{
  TRect destRect=GetClientRect();
  int left_x=Attr.X;
  int top_y=Attr.Y;
  if (point.x>=left_x && point.x<left_x+destRect.Width() &&
    point.y>=top_y && point.y<top_y+destRect.Height())
    return true;
  else
    return false;
}


} // OwlExt namespace

