//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TImageList, an ImageList Common Control wrapper.
//----------------------------------------------------------------------------

#if !defined(OWL_IMAGELST_H)
#define OWL_IMAGELST_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/gdiobjec.h>
#include <owl/commctrl.h>


namespace owl {

class _OWLCLASS TImageList;

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup commctrl
/// @{
//
/// \class TImageInfo
// ~~~~~ ~~~~~~~~~~
/// TImageInfo is a wrapper class for a structure that describes an image within an
/// image list.
//
/// DLN made this derive from IMAGEINFO to simplify porting code
///    using existing _IMAGEINFO structure. Of course, new code
///    should avoid directly accessing these fields.
//
class _OWLCLASS TImageInfo : public _IMAGEINFO
{
  public:
    TImageInfo();
    TImageInfo(const TImageList& list, int index = 0);

    HBITMAP GetImageBM() const;
    HBITMAP GetMaskBM() const;
    TRect   GetImageRect() const;

//JJH this operator is never used in gcc
//Jogy - is this still the case?
#if !defined(__GNUC__)
    operator _IMAGEINFO&();
#endif

//  private:
//    _IMAGEINFO rep;

  friend class _OWLCLASS TImageList;
};

class _OWLCLASS TImageListDrawParam : public IMAGELISTDRAWPARAMS{
  public:
    TImageListDrawParam();
};



//
/// \class TImageList
// ~~~~~ ~~~~~~~~~~
/// TImageList is a wrapper class for the ImageList common "control".
//
class _OWLCLASS TImageList {
  public:
    // Constructors
    //
    TImageList(const TSize& imageSize, uint flags, int initCapacity, int growBy);
    TImageList(const TBitmap& bmp, uint flags, int imageCount, int growBy);
    TImageList(const TDib& dib, uint flags, int imageCount, int growBy);

    TImageList(HINSTANCE hI, TResId resName, int imageWidth, int growBy,
               const TColor& mask, uint type, uint flags);
    TImageList(const TImageList& src);

    TImageList(HIMAGELIST imageList);

    TImageList(TPoint& pt, TPoint& hotspot);

    /*virtual*/ ~TImageList();

    TImageList& operator =(const TImageList&);
    operator TBitmap&();

    operator HIMAGELIST() const;

    // 'Get' accessors
    //
    int        GetImageCount() const;
    TSize      GetImageSize() const;
    TPoint     GetImageOffset(int cel) const;
    TRect      GetImageRect(int cel) const;
    TRect      operator [](int cel) const;

    // Add an image into the list
    //
    int        Add(const TBitmap& image);
    int        Add(const TBitmap& image, const TBitmap& mask);
    int        Add(const TBitmap& image, const TColor& mask);
    int        Add(const TIcon&);

    // Remove an image from the list
    bool       Remove(int index);
    bool       RemoveAll();

    // Replace the image at an index with another
    //
    bool       Replace(int index, const TBitmap& image);
    bool       Replace(int index, const TBitmap& image, const TBitmap& mask);

    // Get an icon or cursor from an image based on masks
    //
    int        ReplaceIcon(int index, HICON hicon);
    HICON      GetIcon(int index, uint flags) const;
    bool       GetIconSize(int& cx, int& cy);

    bool       GetImageInfo(int index, TImageInfo& celInfo) const;

    TColor     GetBkColor() const;
    TColor     SetBkColor(const TColor& newColor);

    // Draw the image
    //
    bool       Draw(int index, TDC&, int x, int y, uint style = ILD_NORMAL,
                    int overlay = 0);

    bool       Draw(int index, TDC&, int x, int y, int dx, int dy,
                    const TColor& bgClr, const TColor& fgClr,
                    uint style = ILD_NORMAL, int overlay = 0);

    bool       SetOverlayImage(int index, int overlay);

    // Routines related to dragging
    //
    bool        SetDragCursorImage(int drag, int dxHotspot, int dyHotspot);
    bool        BeginDrag(int index, int dxHotspot, int dyHotspot);
    static bool DragEnter(HWND hwndLock,  int x, int y);
    static bool DragMove(int x, int y);
    static bool DragLeave(HWND hwndLock);
    static void EndDrag();
    static bool DragShowNolock(bool show);

    // Version 4.70
    bool Copy(TImageList& imgsrc, int ifrom, int ito, uint flags=ILCF_MOVE);
    // Version 4.70
    bool DrawIndirect(const TImageListDrawParam& imldp);
    // Version 4.71
    TImageList* Duplicate();
    // Version 4.70
    bool SetImageCount(uint newcount);

  protected:
    void        CheckValid();

  protected_data:
    HIMAGELIST  Handle;      ///< Handle of ImageList

  private:

    TBitmap*    Bitmap;      ///< Copy of image BM to work around Win95 BUG
    TSize       ImageSize;   ///< Size of images in ImageList
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


//------------------------------------------------------------------------
// Inline implementations


//
/// Constructs an empty image info.
//
inline TImageInfo::TImageInfo() {
}

//
/// Construct based on an image within the image list.
//
inline TImageInfo::TImageInfo(const TImageList& array, int index) {
  array.GetImageInfo(index, *this);
}

//
/// Gets the image bitmap
//
inline HBITMAP TImageInfo::GetImageBM() const {
  return hbmImage;
}

//
/// Gets the image's mask
//
inline HBITMAP TImageInfo::GetMaskBM() const {
  return hbmMask;
}

//
/// Gets the area of the image
//
inline TRect TImageInfo::GetImageRect() const {
  return rcImage;
}

//This operator is never used in gcc
#if !defined(__GNUC__)
//
/// Converts to an _IMAGEINFO structure
//
inline TImageInfo::operator _IMAGEINFO&() {
  return *this;
}
#endif


//
/// Returns the size each image.
///
/// Each image of the list must be the same size.
//
inline TSize TImageList::GetImageSize() const {
  return ImageSize;
}

//
/// Returns the offset of a given image in the ImageList's bitmap
//
inline TPoint TImageList::GetImageOffset(int index) const {
  return TPoint(index*ImageSize.cx, 0);
}

//
/// Returns the bounding rect of a given image in the ImageList's bitmap
//
inline TRect TImageList::GetImageRect(int index) const {
  return TRect(TPoint(index*ImageSize.cx, 0), ImageSize);
}

//
/// Returns the rectangle of the image at index 'index'.
//
inline TRect TImageList::operator [](int index) const {
  return GetImageRect(index);
}

//
/// Removes all images from the list.
//
inline bool TImageList::RemoveAll() {
  return Remove(-1);
}

//
/// Converts to the HIMAGELIST structure.
//
inline TImageList::operator HIMAGELIST() const {
  return Handle;
}


} // OWL namespace

#endif  // OWL_IMAGELST_H
