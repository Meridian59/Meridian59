//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TImageList, an ImageList 'common control' wrapper
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/imagelst.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommCtrl);    // Common Controls diagnostic group

//----------------------------------------------------------------------------

TImageListDrawParam::TImageListDrawParam()
{
  memset(this,0,sizeof(IMAGELISTDRAWPARAMS));
  cbSize = sizeof(IMAGELISTDRAWPARAMS);
}

//----------------------------------------------------------------------------

//
/// Constructs an empty ImageList of a given size.
/// The `imageSize` is the individual image size, not the total size of the list.
/// The `initCapacity` parameter describes the initial capacity of the list, i.e.
/// how many images it can hold before it needs to grow (perform a reallocation).
/// Note that the list remains empty after construction, regardless of capacity.
/// The `growBy` parameter specifies how much the capacity will grow when needed.
//
TImageList::TImageList(const TSize& imageSize, uint flags, int initCapacity, int growBy)
{
  if (!initCapacity)
    initCapacity = 1;
  ImageSize = TSize(imageSize.cx, imageSize.cy);
  Handle = ImageList_Create(imageSize.cx, imageSize.cy, flags, initCapacity, growBy);
  WARNX(OwlCommCtrl, !Handle, 0, "Cannot create ImageList");
  Bitmap = nullptr;
  CheckValid();
}

//
/// Constructs a TImageList from a bitmap, slicing it up into a horizontal array of
/// the given number of evenly sized images.
//
TImageList::TImageList(const TBitmap& bmp, uint flags, int imageCount, int growBy)
{
  if (!imageCount)
    imageCount = 1;
  ImageSize = TSize(bmp.Width() / imageCount, bmp.Height());
  Handle = ImageList_Create(ImageSize.cx, ImageSize.cy, flags, imageCount, growBy);
  WARNX(OwlCommCtrl, !Handle, 0, "Cannot create ImageList");
  Bitmap = nullptr;
  CheckValid();

  // Use masked support with 3dFace color as background color.
  //
  Add(bmp, TColor::Sys3dFace);
}

//
/// Constructs a TImageList from a DIB, slicing the bitmap up into a horizontal array of
/// the given number of evenly sized images.
//
TImageList::TImageList(const TDib& dib, uint flags, int imageCount, int growBy)
{
  if (!imageCount)
    imageCount = 1;
  ImageSize = TSize(dib.Width() / imageCount, dib.Height());
  Handle = ImageList_Create(ImageSize.cx, ImageSize.cy, flags, imageCount, growBy);
  WARNX(OwlCommCtrl, !Handle, 0, "Cannot create ImageList");
  Bitmap = nullptr;
  CheckValid();

  // Use masked support with 3dFace color as background color.
  //
  TPalette pal((HPALETTE)::GetStockObject(DEFAULT_PALETTE));
  Add(TBitmap(dib, &pal), TColor::Sys3dFace);
}


//
/// Constructs an ImageList from a bitmap resource.
/// `type` must be IMAGE_BITMAP. `flags` can be one of the flags specified for the 
/// ImageList_LoadImage function in the Windows API. The default is LR_CREATEDIBSECTION which will
/// load the bitmap as a device-independent bitmap (DIB) with no colour mapping performed.
/// \see http://msdn.microsoft.com/en-us/library/windows/desktop/bb761557.aspx
//
TImageList::TImageList(HINSTANCE h, TResId id, int imageWidth, int growBy, const TColor& mask, uint type, uint flags)
{
  Handle = ImageList_LoadImage(h, id, imageWidth, growBy, mask, type, flags);
  WARNX(OwlCommCtrl, !Handle, 0, "Cannot create ImageList");
  ImageSize = TSize(0);
  Bitmap = nullptr;
  CheckValid();
}


//
/// Constructs a C++ alias for an existing imagelist.
/// The object takes ownership of the given handle, i.e. it will be destroyed in the
/// destructor at the end of the object's lifetime.
//
TImageList::TImageList(HIMAGELIST imageList)
{
  Handle = imageList;
  WARNX(OwlCommCtrl, !Handle, 0, "Cannot create ImageList");
  int cx = 0;
  int cy = 0;
  ImageList_GetIconSize(Handle, &cx, &cy);
  ImageSize.cx = cx;
  ImageSize.cy = cy;
  Bitmap = nullptr;
  CheckValid();
}

//
/// Constructs a wrapper for the current drag imagelist and specifies the location
/// and hotspot of the imagelist.
//
TImageList::TImageList(TPoint& pt, TPoint& hotspot)
{
  ImageSize = TSize(0);
  Handle = ImageList_GetDragImage(&pt, &hotspot);
  WARNX(OwlCommCtrl, !Handle, 0, "Cannot create ImageList");
  Bitmap = nullptr;
  CheckValid();
}

//
/// Creates a duplicate of the given image list.
/// \sa <a href="https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-imagelist_duplicate">
/// ImageList_Duplicate</a> in the Windows API.
//
TImageList::TImageList(const TImageList& src)
  : Handle{ImageList_Duplicate(src)}, Bitmap{}, ImageSize{TImageInfo{src}.GetImageRect().Size()}
{
  WARNX(OwlCommCtrl, !Handle, 0, _T("TImageList::TImageList: ImageList_Duplicate failed"));
  CheckValid();
}

//
/// Destructs the ImageList and cleans up the image list handle.
//
TImageList::~TImageList()
{
  ImageList_Destroy(Handle);
  if (Bitmap)
    delete Bitmap;
}

//
/// Throws an exception if this image list handle is invalid
//
void
TImageList::CheckValid()
{
  if (!Handle)
    TXCommCtrl::Raise();
}

//
/// Replaces the image list by a duplicate of the given image list.
/// \sa <a href="https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-imagelist_duplicate">
/// ImageList_Duplicate</a> in the Windows API.
//
TImageList&
TImageList::operator =(const TImageList& src)
{
  if (this == &src) return *this;
  const auto d = ImageList_Duplicate(src);
  WARNX(OwlCommCtrl, !d, 0, _T("TImageList::operator =: ImageList_Duplicate failed"));
  if (!d) throw TXOwl{_T("TImageList::operator = failed")};
  if (Handle)
  {
    [[maybe_unused]] const auto ok = ImageList_Destroy(Handle) != FALSE;
    WARNX(OwlCommCtrl, !ok, 0, _T("TImageList::operator =: ImageList_Destroy failed"));
  }
  Handle = d;
  if (Bitmap)
  {
    delete Bitmap;
    Bitmap = nullptr;
  }
  ImageSize = TImageInfo{src}.GetImageRect().Size();
  return *this;
}

//
/// Returns the image bitmap used by this ImageList
//
TImageList::operator TBitmap&()
{
  // Build the bitmap object on the fly if needed
  // To work around a win95 bug, making whole copy for now.
  //
  TImageInfo ii(*this);
  if (!Bitmap || (HBITMAP)*Bitmap != ii.GetImageBM()) {
    delete Bitmap;
    Bitmap = new TBitmap(TScreenDC(), TDib(TBitmap(ii.GetImageBM())));
  }
  return *Bitmap;
}



//
/// Draws an image onto a target DC at a given coordinate and with a given style.
//
bool
TImageList::Draw(int index, TDC& dc, int x, int y, uint style, int overlay)
{
  PRECONDITION(Handle);
  if (overlay)
    style |= INDEXTOOVERLAYMASK(overlay);
  return ImageList_Draw(Handle, index, dc, x, y, style);
}


//
/// Extended version of draw that takes a foreground color and background color.
//
bool
TImageList::Draw(int index, TDC& dc, int x, int y, int dx, int dy,
                 const TColor& bgClr, const TColor& fgClr, uint style,
                 int overlay)
{
  PRECONDITION(Handle);
  if (overlay)
    style |= INDEXTOOVERLAYMASK(overlay);
  COLORREF bgCr = (bgClr == TColor::None) ? static_cast<COLORREF>(CLR_NONE) : static_cast<COLORREF>(bgClr);
  COLORREF fgCr = (fgClr == TColor::None) ? static_cast<COLORREF>(CLR_NONE) : static_cast<COLORREF>(fgClr);

  return ImageList_DrawEx(Handle, index, dc, x, y, dx, dy, bgCr, fgCr, style);
}

//
/// Returns number of images currently in this ImageList.
//
int
TImageList::GetImageCount() const
{
  PRECONDITION(Handle);
  return ImageList_GetImageCount(Handle);
}

//
/// Adds new image(s) to the ImageList. Returns index of new addition. No mask
/// bitmap is added.
//
int
TImageList::Add(const TBitmap& image)
{
  PRECONDITION(Handle);
  return ImageList_Add(Handle, image, nullptr);
}


//
/// Adds new image/mask pair(s) to the ImageList. Returns index of new addition.
//
int
TImageList::Add(const TBitmap& image, const TBitmap& mask)
{
  PRECONDITION(Handle);
  return ImageList_Add(Handle, image, mask);
}


//
/// Adds new image(s) to ImageList, specifying a mask color to generate a mask.
/// Returns index of the new addition.
//
int
TImageList::Add(const TBitmap& image, const TColor& mskColor)
{
  PRECONDITION(Handle);
  return ImageList_AddMasked(Handle, image, mskColor);
}

//
/// Adds an icon to the ImageList. Returns index of new addition.
//
int
TImageList::Add(const TIcon& icon)
{
  PRECONDITION(Handle);
  return ImageList_AddIcon(Handle, icon);
}

//
/// Removes an image (or all images if index is -1) from this ImageList.
//
bool
TImageList::Remove(int index)
{
  PRECONDITION(Handle);
  return ImageList_Remove(Handle, index);
}

//
/// Replaces an image in this ImageList.
//
bool
TImageList::Replace(int index, const TBitmap& image)
{
  PRECONDITION(Handle);
  return ImageList_Replace(Handle, index, image, nullptr);
}

//
/// Replaces an image and mask in the ImageList.
//
bool
TImageList::Replace(int index, const TBitmap& image, const TBitmap& mask)
{
  PRECONDITION(Handle);
  return ImageList_Replace(Handle, index, image, mask);
}

//
/// Creates and retrieves an icon from an image and mask in the ImageList.
//
HICON
TImageList::GetIcon(int index, uint flags) const
{
  PRECONDITION(Handle);
  return ImageList_GetIcon(Handle, index, flags);
}

//
/// Replaces the image at index 'index' with the icon or cursor.
//
int
TImageList::ReplaceIcon(int index, HICON icon)
{
  PRECONDITION(Handle);
  return ImageList_ReplaceIcon(Handle, index, icon);
}

//
/// Returns the icon size.
/// \todo What about a TSize TImageList::GetIconSize() version?
bool
TImageList::GetIconSize(int& cx, int& cy)
{
  PRECONDITION(Handle);
  return ImageList_GetIconSize(Handle, &cx, &cy);
}

//
/// Gets general information about a given image.
//
bool
TImageList::GetImageInfo(int index, TImageInfo& imageInfo) const
{
  PRECONDITION(Handle);
  return ImageList_GetImageInfo(Handle, index, static_cast<IMAGEINFO*>(&imageInfo));
}

//
/// Functional-style overload
//
TImageInfo
TImageList::GetImageInfo(int index) const
{
  auto i = TImageInfo{};
  const auto r = GetImageInfo(index, i);
  if (!r) throw TXOwl{_T("GetImageInfo failed")};
  return i;
}

//
/// Gets the current background color for this ImageList.
//
TColor
TImageList::GetBkColor() const
{
  PRECONDITION(Handle);
  return ImageList_GetBkColor(Handle);
}

//
/// Sets the current background color for this ImageList, returning the previous
/// color.
//
TColor
TImageList::SetBkColor(const TColor& color)
{
  PRECONDITION(Handle);
  return ImageList_SetBkColor(Handle, color);
}

//
/// Selects an image for use as an overlay. Up to four can be selected.
//
bool
TImageList::SetOverlayImage(int index, int overlay)
{
  PRECONDITION(Handle);
  return ImageList_SetOverlayImage(Handle, index, overlay);
}


//
/// Combines the current drag image with another image in the list. Typically, a
/// mouse cursor would be added to the image list and merged with the drag image
/// list.
//
bool
TImageList::SetDragCursorImage(int drag, int dxHotspot, int dyHotspot)
{
  PRECONDITION(Handle);
  return ImageList_SetDragCursorImage(Handle, drag, dxHotspot, dyHotspot);
}

//
/// BeginDrag sets this imagelist to be the drag imagelist. There can only be one
/// drag imagelist at any time.
//
bool
TImageList::BeginDrag(int index, int dxHotspot, int dyHotspot)
{
  PRECONDITION(Handle);
  return ImageList_BeginDrag(Handle, index, dxHotspot, dyHotspot);
}

//
/// Typically, this function is called in response to a WM_LBUTTONDOWN message. The
/// 'x' and 'y' parameters are relative to the upper-left corner of the window's
/// rectangle and NOT the client area. The window 'hWndLock' is locked from further
/// updates.
//
bool
TImageList::DragEnter(HWND hWndLock, int x, int y)
{
  return ImageList_DragEnter(hWndLock, x, y);
}

//
/// DragMove is typically called when receiving a WM_MOUSEMOVE message. The 'x' and
/// 'y' parameters are generally passed from the message to this function.
//
bool
TImageList::DragMove(int x, int y)
{
  return ImageList_DragMove(x, y);
}

//
/// DragLeave is typically called when receiving a WM_LBUTTONUP message. The
/// 'hWndLock' window is unlocked from updates.
//
bool
TImageList::DragLeave(HWND hWndLock)
{
  return ImageList_DragLeave(hWndLock);
}

//
/// EndDrag removes the current drag imagelist from the system.
//
void
TImageList::EndDrag()
{
  ImageList_EndDrag();
}

//
/// Locks or unlocks the window from updates.
//
bool
TImageList::DragShowNolock(bool show)
{
  return ImageList_DragShowNolock(show);
}

//
// Version 4.70
//
bool
TImageList::Copy(TImageList& src, int ifrom, int ito, uint flags)
{
  PRECONDITION(Handle);
  return ImageList_Copy(Handle, ito, src, ifrom, flags);
}

//
// Version 4.70
//
bool
TImageList::DrawIndirect(const TImageListDrawParam& a)
{
  TImageListDrawParam ca = a;
  return ImageList_DrawIndirect(&ca);
}

//
// Version 4.71
//
TImageList*
TImageList::Duplicate()
{
  PRECONDITION(Handle);
  HIMAGELIST hImg = ImageList_Duplicate(Handle);
  return (hImg) ? new TImageList(hImg) : nullptr;
}

//
// Version 4.70
//
bool
TImageList::SetImageCount(uint newcount)
{
  return ImageList_SetImageCount(Handle, newcount);
}

} // OWL namespace

