//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------
#include <owl/pch.h>

#if !defined(OWL_COLMNHDR_H)
#include <owl/colmnhdr.h>
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommCtrl);

//
// Default size of string and bitmap items, both measured in pixels
//

/// Default size of string item.
int  THdrItem::DefStringItemSize = 100;

/// Default size of bitmap item.
int  THdrItem::DefBitmapItemSize = 32;


/// API to allow user to update ObjectWindows' default string size.
/// \note Since the variable maintaining that information is shared
///       by every instance of this class, use this function with
///       caution.
//
void
THdrItem::SetDefStringSize(int size)
{
  DefStringItemSize = size;
}

//
/// API to allow the user to update ObjectWindows' default bitmap
/// item size.
/// \note Since the variable maintaining that information is shared
///       by every instance of this class, use this function with
///       caution.
void
THdrItem::SetDefBitmapSize(int size)
{
  DefBitmapItemSize = size;
}

//
/// Constructs a THdrItem object for an item consisting of
/// both a string and a bitmapped image.
//
THdrItem::THdrItem(LPCTSTR str, HBITMAP hbm)
{
  mask = HDI_FORMAT;
  fmt  = HDF_LEFT;
  SetText(str);
  SetWidth(DefStringItemSize);
  SetBitmap(hbm);
  lParam = 0;
}

//
/// Constructs a THdrItem object for an item which has a string
/// Defaults to left alignment and the DefaultStringItemSize.
//
THdrItem::THdrItem(LPCTSTR str)
{
  mask = HDI_FORMAT;
  fmt  = HDF_LEFT;
  SetText(str);
  SetWidth(DefStringItemSize);
  lParam = 0;
}

//
/// Constructs a THdrItem object for an item which has a bitmapped image
/// Defaults to left alignment and the default bitmap item size.
//
THdrItem::THdrItem(HBITMAP hbm)
{
  mask = HDI_FORMAT;
  fmt  = HDF_LEFT;
  SetBitmap(hbm);
  SetWidth(DefBitmapItemSize);
  lParam = 0;
}

//
/// Constructs an 'empty' THdrItem with the specified 'msk' enabled.
/// This flavour of the constructor is mainly used to construct an object
/// which is used to retrieve information about an existing item.
//
THdrItem::THdrItem(uint msk)
{
  mask = msk;
}

//
/// Constructs an 'empty' THdrItem with the specified 'msk' enabled.
/// This flavour of the constructor is mainly used to construct an object
/// which is used to retrieve information about an existing item. 'buffer'
/// specifies a location which will receive the text of the item and 'len'
/// specifies the size of the buffer.
//
THdrItem::THdrItem(LPTSTR buffer, int len, uint msk)
{
  mask = msk;
  cchTextMax = len;
  pszText = buffer;
  hbm = 0;
  lParam = 0;
}

//
/// Sets the text of the HeaderItem object.
/// \note The format flags is not updated to contain any alignment flags
//
void
THdrItem::SetText(LPCTSTR str)
{
  mask      |= (HDI_TEXT|HDI_FORMAT);
  fmt       |= (HDF_STRING);
  pszText    = CONST_CAST(LPTSTR, str);
  cchTextMax = static_cast<int>(::_tcslen(str)+1);   // This is probably superfluous,
                                // albeit harmless
}

//
/// Sets the bitmap handle of the HeaderItem object.
/// \note The format flags is not updated to contain any alignment flags
//
void
THdrItem::SetBitmap(HBITMAP bitmap)
{
  mask |= (HDI_BITMAP|HDI_FORMAT);
  fmt  |= HDF_BITMAP;
  hbm   = bitmap;
}

//
/// Sets the width of the item
//
void
THdrItem::SetWidth(int width)
{
  cxy = width;
  mask &= ~HDI_HEIGHT;
  mask |=  HDI_WIDTH;
}

//
/// Sets the height of the item
//
void
THdrItem::SetHeight(int height)
{
  cxy = height;
  mask &= ~HDI_WIDTH;
  mask |=  HDI_HEIGHT;
}

//
/// Sets the application-defined item data attached to this item object
//
void
THdrItem::SetItemData(TParam2 data)
{
  lParam = data;
  mask  |= HDI_LPARAM;
}

//
/// Version 4.70. Zero-based index of an image within the image list
//
void
THdrItem::SetImage(int item)
{
  iImage  = item;
  mask  |= HDI_IMAGE;

}

//
/// Version 4.70. Order in which the item appears within the header control,
/// from left to right.
//
void
THdrItem::SetOrder(int order)
{
  iOrder  = order;
  mask  |= HDI_ORDER;
}
//----------------------------------------------------------------------------

//
/// Constructor for TColumnHeader
//
/// Initializes its data fields using parameters passed and default values.
/// By default, a ColumnHeader associated with the TColumnHeader will:
///   - be visible upon creation
///   - have a border, divider tracks
///   - be a horizontal header window
//
TColumnHeader::TColumnHeader(TWindow*   parent,
                             int        id,
                             int x, int y, int w, int h,
                             TModule*   module)
:
  TControl(parent, id, 0, x, y, w, h, module)
{
  // By default we'll use the native implementation if it's available
  // here's a strange: although Win95 defines HDS_HORZ, it is defined as '0',
  // which would indicate that the only column headings supported are horizontal
  // headings.  Also, there is no HDS_VERT.
  // In any case, we explicitly 'test' for HDS_HORZ in the code below and we
  // try to forsee what could happen if (and when) HDS_VERT gets defined.
  Attr.Style |= HDS_HORZ;

  // When OWL is built with the NATIVECTRL_ALWAYS option, the
  // Common Control library MUST be available....
  //
  CHECK(TCommCtrl::IsAvailable());

  TRACEX(OwlCommCtrl, OWL_CDLEVEL, "TColumnHeader constructed @" << (void*)this);
}

//
/// Constructor of a ColumnHeader object which aliases a control found
/// within a dialog.
//
TColumnHeader::TColumnHeader(TWindow*   parent,
                             int        resourceId,
                             TModule*   module)
:
  TControl(parent, resourceId, module)
{

  CHECK(TCommCtrl::IsAvailable());

  TRACEX(OwlCommCtrl, OWL_CDLEVEL, "TColumnHeader constructed from resource @" << (void*)this);
}


TColumnHeader::~TColumnHeader()
{
}


//
/// Returns the class name of the underlying control associated with
/// the TColumnHeader object.
/// \note The logic used depends on the availability of native
///       Common Control support. In the case where OWL provides
///       the underlying support, we'll specify a TColumnHeader -specific
///       classname although that's not necessary [it eases debugging]
//
TWindow::TGetClassNameReturnType
TColumnHeader::GetClassName()
{
  PRECONDITION(TCommCtrl::IsAvailable());
  return WC_HEADER;
}

//
/// If successful this method returns the number of items in the
/// header control. In case of failure it returns -1.
//
int
TColumnHeader::GetCount() const
{
    return (int)CONST_CAST(TColumnHeader*,this)->SendMessage(HDM_GETITEMCOUNT);
}

//
/// This method retrieves the size and position of a header control
/// within a given rectangle. It determines the appropriate dimensions
/// of a new header control that is to occupy the given rectangle.
/// Upon entry the 'boundingRect' parameter specifies the rectangle
/// within which the columnHeader must lie. The control then updates
/// the WINDOWPOS structure to contain the desired/appropriate dimensions
/// for the control to occupy within the specified rectangle.
//
bool
TColumnHeader::Layout(TRect& boundingRect, WINDOWPOS& wp)
{
    HD_LAYOUT hdl;
    hdl.prc   = &boundingRect;
    hdl.pwpos = &wp;
    return SendMessage(HDM_LAYOUT, 0, TParam2(&hdl)) != 0;
}

//
/// This method repositions the columnHeader control within the client area
/// of its parent window by taking advantage of the columnHeader's ability
/// to specify its desired/appropriate position from a specified bounding
/// rectangle. This method assumes that the control will occupy the full
/// width of the client area of its parent.
//
bool
TColumnHeader::Layout(uint swpFlags)
{
  WINDOWPOS wp;
  TRect rect = GetParentO()->GetClientRect();
  if (Layout(rect, wp)) {
    SetWindowPos(wp.hwndInsertAfter, wp.x, wp.y, wp.cx, wp.cy,
                 wp.flags|swpFlags);
    return true;
  }
  return false;
}

//
/// Adds a new item to the column header.
/// The following illustrates a typical use of the 'Add' method:
/// \code
///      THdrItem hdrItem("&Employee Names");
///      hdr.Add(hdrItem);
/// \endcode
int
TColumnHeader::Add(const THdrItem& item)
{
  return Insert(item, GetCount());
}

//
/// Inserts a new item after the specified location, 'index', in the column Header
/// control. The new item is inserted at the end of the header control if index is
/// greater than or equal to the number of items in the control. If index is zero,
/// the new item is inserted at the beginning of the header control. The following
/// illustrates a typical use of the 'Insert' method:
/// \code
/// THdrItem hdrItem(GetModule().LoadBitmap(IDB_COMPANYLOGO));
/// hdr.Insert(hdrItem, 0);
/// \endcode
/// If the operation succeeds, the return value is the index of the new item. If the
/// operation fails, the return value is - 1.
//
int
TColumnHeader::Insert(const THdrItem& item, int index)
{
    return SendMessage(HDM_INSERTITEM, index, TParam2(&item)) != 0;

}

//
/// Deletes the item at the specified 'index' from the header control.
//
bool
TColumnHeader::Delete(int index)
{
    return SendMessage(HDM_DELETEITEM, index) != FALSE;
}

//
/// Retrieves information about the item at the specified index by filling
/// out the 'itemInfo' structure passed in. The 'msk' contains one or more
/// HDI_xxxx constants and can be used to specify which information should
/// be copied.
//
bool
TColumnHeader::GetItem(THdrItem& itemInfo, int index, uint msk)
{
  itemInfo.mask |= msk;
    return SendMessage(HDM_GETITEM, index, TParam2(&itemInfo)) != 0;
}

//
/// Updates the attribute(s) a the item at the specified 'index'. The
/// 'itemInfo' structure contains the new attributes of the item.
//
bool
TColumnHeader::SetItem(const THdrItem& itemInfo, int index)
{
    return SendMessage(HDM_SETITEM, index, TParam2(&itemInfo)) != 0;
}

//
/// Transfers are not implemented for ColumnHeaders. Simply return 0.
//

uint
TColumnHeader::Transfer(void* /*buffer*/, TTransferDirection /*direction*/)
{
  TRACEX(OwlCommCtrl, OWL_CDLEVEL, "TColumnHeader::Transfer is not"\
                                   "implemented!");
  return 0;
}

//
/// Tests a point to determine which header item, if any, is at the
/// specified point.
/// Returns the index of the item at the specified position, if any, or – 1
/// otherwise. ht contains the position to test and receives information about the
/// results of the test.
/// \note The coordinates are specified via the 'pt' member of the
///       THeaderHitTestInfo parameter.
//
int
TColumnHeader::HitTest(THeaderHitTestInfo& ht)
{
    return (int)SendMessage(HDM_HITTEST, 0, TParam2(&ht));
}


} // OWL namespace

