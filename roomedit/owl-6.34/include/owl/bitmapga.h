//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TBitmapGadget
/// Definition of class TDynamicBitmapGadget
//----------------------------------------------------------------------------

#if !defined(OWL_BITMAPGA_H)
#define OWL_BITMAPGA_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/gadget.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TCelArray;

/// \addtogroup gadgets
/// @{
/// \class TBitmapGadget
// ~~~~~ ~~~~~~~~~~~~~
/// Derived from TGadget, TBitmapGadget is a simple gadget that can display an array
/// of bitmap images one at a time.
//
class _OWLCLASS TBitmapGadget : public TGadget {
  public:
    TBitmapGadget(TResId imageResIdOrIndex,
                  int    id,
                  TBorderStyle borderStyle,
                  int    numImages,
                  int    startImage = 0,
                  bool   sharedCels = false);
   ~TBitmapGadget();

    int  SelectImage(int imageNum, bool immediate);
    int  GetImage() const;

    void SysColorChange();

  protected:
    void Paint(TDC& dc);

    void SetBounds(const TRect& boundRect);
    void GetDesiredSize(TSize& size);

  private:
    TResId     ResId;					///< Resource id of the bitmap image; 0 if the common TGadgetWindow bitmap image is
															///< used for this gadget.
    TCelArray* ImageArray;		///< Bitmap images.
    int        ImageIndex;		///< Beginning index image to use in the common TGadgetWindow bitmap image for this
															///< gadget (ie. selecting image 0 selects the image at this index.
    int        CurrentImage;	///< 0-based index of the current image to display.
    int        NumImages;			///< Number of equally sized images contained in the bitmap.
    TPoint     BitmapOrigin;	///< Point that bitmap image should be drawn at.
};

//
/// \class TDynamicBitmapGadget
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~
/// Class will send EvEnable message to window, 
/// so you can set bitmap, using SetCheck() method of 
/// TCommandEnabler 
//
class _OWLCLASS TDynamicBitmapGadget: public TBitmapGadget {
  public:
    TDynamicBitmapGadget(TResId imageResIdOrIndex,
                         int id,
                         TBorderStyle borderStyle,
                         int numImages,
                         int startImage = 0,
                         bool sharedCels = false);
    public:
      virtual void CommandEnable();
};
/// @}


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//
// Inline implementations
//
/// Returns the number of the currently selected image.
//
inline int TBitmapGadget::GetImage() const {
  return CurrentImage;
}


} // OWL namespace

#endif  // OWL_BITMAPGA_H
