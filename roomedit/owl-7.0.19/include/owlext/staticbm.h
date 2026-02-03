//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// An OWL static bitmap class for dialogs
// Allows highlighting (by drawing a border), aspect resizing,
// and masking (i.e. transparency)
//
// Original code:
// Written by Richard Brown
// Richard@TowerSoftware.com
//
//-------------------------------------------------------------------
#if !defined(__OWLEXT_STATICBM_H)
#define __OWLEXT_STATICBM_H

// Grab the core OWLEXT file we need
//
#if !defined(__OWLEXT_CORE_H) && !defined (__OWLEXT_ALL_H)
# include <owlext/core.h>
#endif

// Grab some OWL headers that may or may not be included in the OWL/CORE.H file
//
#include <owl/static.h>
#include <owl/gdiobjec.h>
#include <owl/dc.h>


namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>


// -------------------------------------------------------------

class OWLEXTCLASS TStaticBitmap : public owl::TStatic {
  // Object lifetime methods
public:
  TStaticBitmap(owl::TWindow* parent, owl::TBitmap* user_bitmap, owl::TPalette* user_palette,
      int id, int x, int y, int width, int height, bool flag=false,
      bool use_aspect=false,bool use_mask=true);
  virtual ~TStaticBitmap();

  // Mutators
  //
public:
  void SetText(LPTSTR text);
  void UpdateBitmap(owl::TBitmap* user_bitmap,int x,int y,int width,int height,
      bool use_mask=true);
  bool IsOver(owl::TPoint& point);
  void Select(bool flag);

  // OWL overrides
  //
protected:
  void SetupWindow();
  void Paint(owl::TDC& dc,bool,owl::TRect&);

  // Internal data
  //
private:
  enum { BORDER_OFFSET=2 };
  owl::TBitmap*  Bitmap;
  owl::TBitmap*  MaskBitmap;
  owl::TPalette* Palette;
  int        X;
  int        Y;
  int        Width;
  int        Height;
  bool      Selected;
  bool      prevSelected;
  bool      UseAspect;
  bool      MaskUsed;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace


// Richard's code originally called this "StaticBitmap"; for consistency's sake
// within the framework, I renamed it to "TStaticBitmap", but provide this typedef
// to make existing code source-compatible. #define NO_STATICBITMAP_TYPEDEF if
// it clashes with another name. Note that this needs to be done outside the
// OwlExt namespace, or the typedef will really be OwlExt::StaticBitmap, which
// doesn't quite mimic the original code.
//
#ifndef NO_STATICBITMAP_TYPEDEF
typedef OwlExt::TStaticBitmap StaticBitmap;
#endif


#endif


