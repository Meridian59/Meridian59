//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of classes TTabItem and TTabControl.
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/tabctrl.h>
#include <owl/uihelper.h>
#include <owl/dc.h>
#include <owl/gdiobjec.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommCtrl);        // CommonCtrl Diagnostic group

// Let the compiler know that the following template instances will be defined elsewhere.
//#pragma option -Jgx

//
// Constants used in OWL's implementation of Tab Controls
//
const int InitSize     = 5;         // Initial size of Tab Collection
const int InitDelta    = 5;         // Inc. when expanding collection
const int HorzPad      = 6;         // Pad on each side of tab label
const int VertPad      = 4;         // Pad above/below each tab label
const int HorzSelInc   = 1;         // Inc. for selected tab
const int VertSelInc   = 1;         // Inc. for selected tab
const int HorzMargin   = 3;         // Margin on left/right of control
const int VertMargin   = 3;         // Margin above control
const int ClientMargin = 2;         // Inner margin of tab's client area
const int ID_Updown    = 0x100;     // Ctl. identifier of scroller;

//
/// The item is initialized with the state of an actual tab in
/// a created tab control.
//
TTabItem::TTabItem(const TTabControl& ctl, int index, uint msk,
                   int buffLen, LPTSTR buffer)
{
  PRECONDITION(ctl.GetHandle());

  // When retrieving text, a buffer and length must be supplied
  //
  PRECONDITION(!(msk & TCIF_TEXT) || (buffLen && buffer));

  mask = msk;
  if (buffer && buffLen)
    SetLabel(buffer, buffLen);

  ctl.GetItem(index, *this);
}

//
/// The tab item's label field is initialized to the specified buffer and extra
/// parameter set to the 'param' parameter.
//
TTabItem::TTabItem(LPCTSTR str, int buffLen, TParam2 param)
{
  mask = 0;
  iImage = -1;

  SetParam(param);
  SetLabel(str, buffLen);
}

//
/// The tab item is initialized with the IMAGELIST index and the extra parameter
/// specified.
//
TTabItem::TTabItem(int imageIndex, TParam2 param)
{
  mask = 0;
  pszText = 0;
  cchTextMax = 0;

  SetParam(param);
  SetIcon(imageIndex);
}

//
/// The tab item is initialized with the IMAGELIST index and label specified.
//
TTabItem::TTabItem(int imageIndex, LPCTSTR str)
{
  mask = 0;
  SetIcon(imageIndex);
  SetLabel(str);
}

//
// Initializes the structure member representing the tab's text
//
void
TTabItem::SetLabel(LPCTSTR str, int len)
{
  pszText = CONST_CAST(LPTSTR, str);
  cchTextMax = len ? len : ::_tcslen(str);
  mask |= TCIF_TEXT;
}

// Version 4.70
void
TTabItem::SetState(uint32 state, uint32 mask)
{
  dwState      = state;
  dwStateMask  = mask;
}

//----------------------------------------------------------------------------
DEFINE_RESPONSE_TABLE1(TTabControl, TControl)
  EV_WM_HSCROLL,
  EV_WM_VSCROLL,
END_RESPONSE_TABLE;


//
/// Constructor for TTabControl. Initializes its data fields using parameters passed
/// and default values by default. A TabControl associated with the TTabControl
/// will:
/// - be visible upon creation
/// - display only one row of tabs.
//
TTabControl::TTabControl(TWindow*   parent,
                         int        id,
                         int x, int y, int w, int h,
                         TModule*   module)
:
  TControl(parent, id, 0, x, y, w, h, module)
{
  Attr.Style |= (TCS_SINGLELINE|TCS_TABS|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);

  CHECK(TCommCtrl::IsAvailable());
}

//
/// Constructor for a TTabControl object associated with a Tab Control specified in
/// a dialog resource.
//
TTabControl::TTabControl(TWindow*   parent,
                         int        resourceId,
                         TModule*   module)
:
  TControl(parent, resourceId, module)
{
  CHECK(TCommCtrl::IsAvailable());
}

//
/// Constructor for a TTabControl object to be associated with 
/// an already created Tab Control.
//
TTabControl::TTabControl(HWND hwnd) : TControl(hwnd)
{
  PRECONDITION(hwnd && ::IsWindow(hwnd));

  // Here we'll assume if the Common Control Library is available, the
  // window passed is being backed by the CommonControl Library (i.e.
  // OWL does not need to provide the underlying implementation for
  // tab controls.
}

//
/// Cleans up if underlying Tab support was provided by ObjectWindows.
//
TTabControl::~TTabControl()
{
}

//
/// Returns the class name of the underlying control associated with the TTabControl
/// object.
TWindow::TGetClassNameReturnType
TTabControl::GetClassName()
{
  PRECONDITION(TCommCtrl::IsAvailable());
  return WC_TABCONTROL;
}

//
/// Transfer is not implemented in TTabControl, given that each item interacts with
/// settings outside of the TC_ITEM members. (For example, the image index points to
/// the ImageList).
//
uint
TTabControl::Transfer(void* /*buffer*/, TTransferDirection /*direction*/)
{
  TRACEX(OwlCommCtrl, 0, "TTabControl::Transfer is not implemented");
  return 0;
}

//
/// Keep TWindow from rerouting this message - it must be left as is
/// for the tab control as it may originate from the control's spin.
//
void
TTabControl::EvVScroll(uint, uint, THandle)
{
  DefaultProcessing();
}

//
/// Keeps TWindow from rerouting this message.It must be left as is for the tab
/// control, as it may originate from the control's spin.
//
void
TTabControl::EvHScroll(uint, uint, THandle)
{
  DefaultProcessing();
}



//
/// Inserts a new tab with the caption 'tabText' at the 
/// specified 'index'. Returns the index of the new tab, if successful.
//
int
TTabControl::Insert(LPCTSTR tabText, int index)
{
  return Insert(TTabItem(tabText), index);
}




//
// If the 'clientInWindowOut' parameter is false, this method calculates the
// display area of a tab control's display from a window rectangle specified by the
// 'rect' parameter. Otherwise, the method calculates the window rectangle that
// would correspond to the display area specified by the 'rect' parameter. The
// 'rect' parameter receives the newly computed rectangle.
//
void
TTabControl::AdjustRect(bool clientInWindowOut, TRect& rect)
{
    SendMessage(TCM_ADJUSTRECT, TParam1(clientInWindowOut), TParam2(&rect));
}

//
/// Inserts a new tab described by the 'item' parameter to the tab control at the
/// position specified by the 'index' parameter. The return value is the index of
/// the new tab or -1 in case of error.
//
int
TTabControl::Add(const TTabItem& item)
{
  return Insert(item, GetCount());
}

//
// Adds a new tab with the 'tabText' caption to the tab control Returns the index
// of the new tab, if successful or -1 otherwise.
//
int
TTabControl::Add(LPCTSTR tabText)
{
  return Insert(tabText, GetCount());
}

//
// Inserts a new tab described by the 'item' parameter to the tab
// control at the position specified by the 'index' parameter.
// The return value is the index of the new tab or -1 in case of error.
//
int
TTabControl::Insert(const TTabItem& item, int index)
{
    return (int)SendMessage(TCM_INSERTITEM, index, TParam2((const TC_ITEM *)(&item)));
}

//
// Removes the item at the position specified by the 'index' parameter.
// Returns 'true' if successful or 'false' otherwise.
//
bool
TTabControl::Delete(int index)
{
    return SendMessage(TCM_DELETEITEM, index) != 0;
}

//
// Removes all items from the tab control. Returns 'true' if successful or
// 'false' otherwise.
//
bool
TTabControl::DeleteAll()
{
    return SendMessage(TCM_DELETEALLITEMS) != 0;
}

//
// Returns the number of tab items in the tab control.
//
int
TTabControl::GetCount() const
{
    return (int)CONST_CAST(TTabControl*, this)->SendMessage(TCM_GETITEMCOUNT);
}

//
// Retrieves the current number of rows in the tab control.
// NOTE: Only tabs with the TCS_MULTILINE style can have multiple rows.
//
int
TTabControl::GetRowCount() const
{
    return (int)CONST_CAST(TTabControl*, this)->SendMessage(TCM_GETROWCOUNT);
}

//
// Returns the index of the currently selected tab item in the tab control.
// Returns -1 if no tab is selected.
//
int
TTabControl::GetSel() const
{
    return (int)CONST_CAST(TTabControl*, this)->SendMessage(TCM_GETCURSEL);
}

//
// Selects the tab item at the position specified by the 'index' parameter.
// The return value is the index of the previously selected tab item if
// successful or -1 otherwise.
// NOTE: A tab control does not send TCN_SELCHANGING or TCN_SELCHANGE 
//       notifications when a tab item is selected via this method.
//
int
TTabControl::SetSel(int index)
{
    return (int)SendMessage(TCM_SETCURSEL, index);
}

//
// Retrieves information about the tab at the position specified by
// the 'index' parameter. Returns true if successful or false otherwise.
// NOTE: The 'mask' member of the 'item' structure specifies which
//       attributes of the tab to return. When spefying TCIF_TEXT, item's
//       pszText member must point to a valid buffer and cchTextMax must
//       specify the size of the buffer.
//
bool
TTabControl::GetItem(int index, TTabItem& item) const
{
    return CONST_CAST(TTabControl*, this)->SendMessage(TCM_GETITEM, index, TParam2((TC_ITEM *)&item)) != 0;
}

//
// Retrieves the bounding rectangle of a tab within a tab control.
// Returns true if successful or false otherwise. 
// NOTE: 'rect' receives the position in viewport coordinates.
//
bool
TTabControl::GetItemRect(int index, TRect& rect) const
{
    return CONST_CAST(TTabControl*, this)->SendMessage(TCM_GETITEMRECT, index, TParam2((LPRECT)&rect)) != 0;
}

//
// Sets some or all of a tab's attributes. The 'mask' member of the
// 'item' parameter specifies which attributes to set.
// Returns true if successful or false otherwise.
//
bool
TTabControl::SetItem(int index, const TTabItem& item)
{
    return SendMessage(TCM_SETITEM, index, TParam2((const TC_ITEM *)&item)) != 0;
}

//
// Sets the number of bytes per tab reserved for application-defined
// data in a tab control. Returns true if successful or false otherwise.
// NOTE: This method should be invoked only when the tab control does not
//       contain any tabs.
//
bool
TTabControl::SetItemExtra(int extra)
{
    return SendMessage(TCM_SETITEMEXTRA, extra) != 0;
}

//
// Sets the size (width/height) of tabs in a fixed-width or owner-draw
// tab control. Returns a TSize object containing the old width and height.
//
TSize
TTabControl::SetItemSize(const TSize& size)
{
    return TSize(SendMessage(TCM_SETITEMSIZE, 0, MkParam2(size.cx, size.cy)));
}

//
// Sets the amount of space around each tab's icon and label in a tab 
// control. 
//
void
TTabControl::SetPadding(const TSize& size)
{
    SendMessage(TCM_SETPADDING, 0, MkParam2(size.cx, size.cy));
}

//
// Retrieves the ImageList associated with a tab control. Returns 0 if
// unsuccessful.
//
HIMAGELIST
TTabControl::GetImageList() const
{
    return (HIMAGELIST)CONST_CAST(TTabControl*, this)->SendMessage(TCM_GETIMAGELIST);
}

//
// Assigns an imagelist to the tab control. Returns the handle of the
// previous imagelist or 0 if there is no previous image list.
//
HIMAGELIST
TTabControl::SetImageList(HIMAGELIST himl)
{
    return (HIMAGELIST)SendMessage(TCM_SETIMAGELIST, 0, TParam2(himl));
}

//
// Removes the image at the position specified by 'index' from the
// imagelist associated with the tab control.
// NOTE: The tab automatically updates each tab's image index so each
//       tab remains associated with the same image it had been.
//
void
TTabControl::RemoveImage(int index)
{
    SendMessage(TCM_REMOVEIMAGE, index);
}

//
// Retrieves the handle of the tooltip control associated with the
// tab control. Returns 0 if unsuccessful.
//
TWindow::THandle
TTabControl::GetToolTips() const
{
    return THandle(CONST_CAST(TTabControl*,this)->SendMessage(TCM_GETTOOLTIPS));
}

//
// Assigns a tooltip control to the tab control.
//
void
TTabControl::SetToolTips(THandle toolTip)
{
    SendMessage(TCM_SETTOOLTIPS, TParam1(toolTip));
}

//
// Determines the index of the tab which is at the location
// specified in the 'pt' member of the 'htInfo' parameter.
// Returns -1 if no tab is at the specified position.
//
int
TTabControl::HitTest(TTabHitTestInfo& htInfo)
{
    return (int)SendMessage(TCM_HITTEST, 0, TParam2((TC_HITTESTINFO *)&htInfo));
}

//----------------------------------------------------------------------------


} // OWL namespace
/* ========================================================================== */

