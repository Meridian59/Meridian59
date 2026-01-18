//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// Original code by:
//----------------------------------------------------------------
// Copyright (c) Antony Corbett 1995. All Rights Reserved
// Author:  Antony Corbett
//        Compuserve  100277,330
//        Tel +44 (1926) 856131,  Fax +44 (1926) 856727
//
// Description:
//    TExpandableComboBox implementation.
//
//    TComboBox derivatives that provide disabling of selected
//    list items, horizontal scrolling and dynamic switching
//    between single & multiple selection.
//
//
//    History:
//      15 Dec 1995      Initial release
//
//      21 Dec 1995       Added support for Tab Stops
//                  Added MakeSingleSel()
//                  Removed oldCaret_
//                  Added colored items support
//
//       22 Dec 1995      Modified SetSel()
//                  Added SetSelIndex()
//
//      29 Dec 1995      Fixed bug in TExpandableComboBox::SetSel()
//                    (Thanks to Greg Bullock)
//
//       10 Jun 1997       Updated for Inphorm
//                         Converted Class to ComboBox Derivative
//
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owl/shddel.h>

#include <owlext/comboexp.h>

#include <algorithm>

using namespace owl;
using namespace std;

namespace OwlExt {

//***************************************
//    TExpandableComboBox
//***************************************

DEFINE_RESPONSE_TABLE1(TExpandableComboBox, TComboBox)

// owl routes this message from parent back to control...
EV_WM_MEASUREITEM,

// The CBN_SELCHANGE notification is sent for a
//  multiple-selection list box whenever the user
// presses an arrow key, even if the selection
// does not change....
EV_NOTIFY_AT_CHILD(CBN_SELCHANGE, CmSelChange),
EV_NOTIFY_AT_CHILD(CBN_KILLFOCUS, CmKillFocus),
// EV_NOTIFY_AT_CHILD(CBN_SELENDOK, CmSelEndOk),

END_RESPONSE_TABLE;


//----------------------------------------------------------------
// Description:
//    c'tor. Constructs a TExpandableComboBox control to be associated
//    with a TDialog.
//
// Arguments:
//    useHScroll = flag indicating if we should use a
//    horizontal scroll bar.
//
//----------------------------------------------------------------
TExpandableComboBox::TExpandableComboBox(TWindow* parent, int resourceId,
                     eExpAlign fTCBAlign,
                     int nUnitWidth,
                     TModule* module  // = 0
                     ) :
TComboBox(parent, resourceId, 0, module),
textExtents_(*new SortedIntegers),  // store extents of strings
pseudoSingleSel_(FALSE)
{
  tabs_ = new int[1];
  numTabs_ = 1;
  hwndList = 0;
  fTCBAlign_ = fTCBAlign;
  nUnitWidth_ = nUnitWidth;
  nItemHeight = 0;
  bFirstRedraw = true;

  hasHScroll_ = true;
  greatestExtent = 0;
  nPasses = 0;
  bShowCompleteLine_ = false;
}

//----------------------------------------------------------------
// Description:
//    d'tor. Deletes the integer array used to hold text extents.
//
//----------------------------------------------------------------
TExpandableComboBox::~TExpandableComboBox()
{
  Destroy();
  delete &textExtents_;
  delete[] tabs_;
}

// Setupwindow -- Used to to set the position of the default tab
void
TExpandableComboBox::SetupWindow()
{
  TComboBox::SetupWindow();

  TRect cbRect = GetWindowRect();
  int  nDlgUnits = LOWORD (GetDialogBaseUnits());
  int nThumb = GetSystemMetrics (SM_CXHTHUMB);

  int nDlgTabUnits = (((cbRect.Width()-nThumb) * nDlgUnits) / (4));
  tabs_[0] = nDlgTabUnits / 2;
}



//****************************************************
//
// operations on individual list box items:
//
//    int AddString(const char *str);
//    int AddDisabledString(const char *str);
//    int InsertString(const char *str, int index);
//    int DeleteString(int index);
//    void Enable(int index, BOOL enable = TRUE);
//    int SetItemData(int index, uint32 data);
//    int SetSel(int index, BOOL select);
//    int SetSelIndex(int index);
//    void SetTextColor(int index, TColor color);
//    void ReSetTextColor(int index);
//
//****************************************************


//----------------------------------------------------------------
// Description:
//    Adds a string to the list.
//
// Arguments:
//    str = string to add
//
// Returns:
//    zero-based index of added string.
//----------------------------------------------------------------
int
TExpandableComboBox::AddString(LPCTSTR str)
{
  // add the string...
  int index = TComboBox::AddString(str);

  if(index >= 0){
    // store the extent of string in sorted container...
    StoreStrExtent(str);
    // now the item data...
    TComboBox::SetItemData(index, reinterpret_cast<LPARAM>(new TCBItemData()));
  }
  return index;
}


//----------------------------------------------------------------
// Description:
//    Adds a string to the list and disables it.
//
// Arguments:
//    str = string to add
//
// Returns:
//    zero-based index of added string.
//----------------------------------------------------------------
int
TExpandableComboBox::AddDisabledString(LPCTSTR str)
{
  int index = AddString(str);
  if(index >= 0)
    Enable(index, FALSE);
  return index;
}


//----------------------------------------------------------------
// Description:
//    Inserts a string at the given position in the list.
//
// Arguments:
//    str = string to insert
//    index = zero-based index giving insertion position.
//    If index == -1 the string is appended to end of list.
//
// Returns:
//    zero-based index of inserted string.
//----------------------------------------------------------------
int
TExpandableComboBox::InsertString(LPCTSTR str, int index)
{
  int actualIndex = TComboBox::InsertString(str, index);
  if(actualIndex >= 0){
    // store the extent of string in sorted container...
    StoreStrExtent(str);
    // now the item data...
    TComboBox::SetItemData(actualIndex, reinterpret_cast<LPARAM>(new TCBItemData()));
  }
  return actualIndex;
}


//----------------------------------------------------------------
// Description:
//    Deletes a string in the list.
//
// Arguments:
//    index = zero-based index of string to remove.
//
// Returns:
//    Number of remaining items or -ve if an error.
//----------------------------------------------------------------
int
TExpandableComboBox::DeleteString(int index)
{
  // first remove any item data...
  TCBItemData* id = GetItemDataPtr(index);
  CHECK(id);
  delete id;

  // find extent of string to be deleted...
  LPTSTR tempStr = new _TCHAR[256];
  GetString(tempStr, index);

  int length = FindTextExtent(tempStr);
  delete[] tempStr;

  textExtents_.DestroyItem(length);

  UpdateHorizontalExtent();

  return (TComboBox::DeleteString(index));
}


//----------------------------------------------------------------
// Description:
//    Changes the enable state of an item in the list.
//
// Arguments:
//    index = zero-based index of item to enable/disable.
//      If index == -1 the all items are enabled/disabled.
//
//    enable = TRUE/FALSE flag indicating if item
//    should be enabled or disabled. Defaults to TRUE
//
//----------------------------------------------------------------
void
TExpandableComboBox::Enable(int index, bool enable/* = true*/)
{
  if(index == -1){
    for(int n=0; n<GetCount(); n++){
      if(GetEnabled(n) != enable){
        // change in status required...
        GetItemDataPtr(n)->enabled_ = enable;
      }
    }
    Invalidate();
  }
  else{
    if(GetEnabled(index) != enable){
      // change in status required...
      GetItemDataPtr(index)->enabled_ = enable;

      // now update the display...
      TRect rc;
      int rv = GetItemRect(index, rc);
      if(rv != CB_ERR)
        InvalidateRect(rc);
    }
  }
}

//----------------------------------------------------------------
// Description:
//    Sets list item data.
//    NB - we've replaced the standard behaviour to allow us
//    to store the item's enable status.
//
// Arguments:
//    index = zero-based index of item.
//    data = custom data to associate with the item.
//
// Returns:
//    CB_ERR if an error.
//----------------------------------------------------------------
int
TExpandableComboBox::SetItemData(int index, LPARAM data)
{
  // the data is actually a ptr to a TCBItemData struct.
  GetItemDataPtr(index)->data_ = data;
  return !CB_ERR;
}


//----------------------------------------------------------------
// Description:
//    Selects an item at the position specified in index.
//
// Arguments:
//    index = zero-based index of item to select.
//    select = TRUE/FALSE
//
// Returns:
//    CB_ERR if an error.
//    If the item is disabled, we do not select and no error
//    is returned.
//----------------------------------------------------------------
int
TExpandableComboBox::SetSel(int index, bool select)
{
  if(!(select && !GetEnabled(index)))
    return (TComboBox::SetSel(index, select));
  return !CB_ERR;
}




//----------------------------------------------------------------
// Description:
//    For single-selection list boxes. Forces the selection
//    of the item at the zero-based index
//
// Arguments:
//    index = zero-based index of item to select.
//      If index is -1, the list box is cleared
//      of any selection.
//
// Returns:
//    A negative number if an error occurs.
//----------------------------------------------------------------
int
TExpandableComboBox::SetSelIndex(int index)
{
  return (TComboBox::SetSelIndex(index));
}


//----------------------------------------------------------------
// Description:
//    Sets the color of an item or all items in the list
//
// Arguments:
//    index = zero-based index of item to color.
//      If index == -1 then all items are set to the
//      specified colour
//    color = color of item text
//
//----------------------------------------------------------------
void
TExpandableComboBox::SetTextColor(int index, TColor color)
{
  if(index == -1){
    for(int n=0; n<GetCount(); n++){
      TCBItemData *id = GetItemDataPtr(n);
      id->textColor_ = color;
      id->useSysColor_ = FALSE;
    }
    Invalidate();
  }
  else{
    TCBItemData *id = GetItemDataPtr(index);
    id->textColor_ = color;
    id->useSysColor_ = FALSE;

    TRect rc;
    if(GetItemRect(index, rc) != CB_ERR)
      InvalidateRect(rc);
  }
}


//----------------------------------------------------------------
// Description:
//    Sets the color of an item or all items to the
//    default system color.
//
// Arguments:
//    index = zero-based index of item to reset color for.
//      If index == -1 then all items are reset.
//
//----------------------------------------------------------------
void
TExpandableComboBox::ResetTextColor(int index)
{
  if(index == -1){
    for(int n=0; n<GetCount(); n++){
      TCBItemData *id = GetItemDataPtr(n);
      id->textColor_ = TColor();
      id->useSysColor_ = TRUE;
    }
    Invalidate();
  }
  else{
    TCBItemData *id = GetItemDataPtr(index);
    id->textColor_ = TColor();
    id->useSysColor_ = TRUE;

    TRect rc;
    if(GetItemRect(index, rc) != CB_ERR)
      InvalidateRect(rc);
  }
}


//****************************************************
//
// operations on listbox:
//
//      void ClearList();
//      BOOL SetTabStops(int numTabs, int * tabs, bool bShowCompleteLine);
//      BOOL MakeSingleSel(BOOL single = TRUE);
//
//****************************************************

//----------------------------------------------------------------
// Description:
//    Clears the list of all items. We override to make sure
//    all the TCBItemData objects are deleted and the hscrolling
//    cleaned up.
//
//----------------------------------------------------------------
void
TExpandableComboBox::ClearList()
{
  for(int n=0; n<GetCount(); n++)
    delete GetItemDataPtr(n);

  textExtents_.Flush();
  UpdateHorizontalExtent();

  TComboBox::ClearList();
}


//----------------------------------------------------------------
// Description:
//    Sets tab stops.
//
// Arguments:
//   numTabs = number of tab stops
//    tabs = array of integers representing the tab positions
//
// Returns:
//    non-zero if all tab stops could be set
//----------------------------------------------------------------
bool
TExpandableComboBox::SetTabStops(int numTabs, int * tabs, bool bShowCompleteLine)
{
  bool rv = !0;

  bShowCompleteLine_ = bShowCompleteLine;
  numTabs_ = numTabs;
  delete[] tabs_;
  tabs_ = new int[numTabs];

  // check they're in size order...
  for(int n=0, lastTab = -1; n<numTabs; n++){
    if(tabs[n] <= lastTab){
      // out of sequence!
      rv = 0;
      numTabs_ = n;  // modify num tabs
      break;
    }
    lastTab = tabs[n];
    tabs_[n] = tabs[n];
  }

  return rv;
}

//----------------------------------------------------------------
// Description:
//    Gets the data stored for the specified item
//
// Arguments:
//    index = zero-based index of item to get data for
//
// Returns:
//    data value
//----------------------------------------------------------------
LPARAM
TExpandableComboBox::GetItemData(int index) const
{
  // the actual data is a ptr to a TCBItemData struct
  // which contains a member 'data'
  return (GetItemDataPtr(index)->data_);
}


//----------------------------------------------------------------
// Description:
//    Returns the selection state of the listbox item at
//    given index.
//
// Arguments:
//    index = zero-based index of item to test.
//
// Returns:
//    TRUE/FALSE
//----------------------------------------------------------------
bool
TExpandableComboBox::GetSel(int index) const
{
  // if the item _is_ selected but is disabled,
  // we say it isn't selected.
  return (TComboBox::GetSel(index)?  GetEnabled(index) : FALSE);
}



//----------------------------------------------------------------
// Description:
//    Gets the enable status of an item.
//
// Arguments:
//    index = zero-based index of item to get status for.
//
// Returns:
//    TRUE/FALSE
//----------------------------------------------------------------
bool
TExpandableComboBox::GetEnabled(int index) const
{
  if (index < 0)
    return true;
  else
    return (GetItemDataPtr(index)->enabled_);
}



//----------------------------------------------------------------
// Description:
//    Gets a ptr to the TCBItemData object associated with
//    the given item. We retain a ptr to the TCBItemData object
//    in the 32-bit item data.
//
// Arguments:
//    zero-based index of item to get TCBItemData object for.
//
// Returns:
//    TCBItemData ptr
//----------------------------------------------------------------
TCBItemData*
TExpandableComboBox::GetItemDataPtr(int index) const
{
  if (index < 0)
    index = 0;
  return ((TCBItemData *)(TComboBox::GetItemData(index)));
}



//****************************************************
//
// query:
//
//    int GetSelCount() const;
//    int GetSelIndex() const;
//
//****************************************************


//----------------------------------------------------------------
// Description:
//    Returns the number of selected items in the list.
//
//----------------------------------------------------------------
int
TExpandableComboBox::GetSelCount() const
{
  return (GetSelIndex() < 0 ? 0 : 1);  // We've overridden GetSelIndex
}

//----------------------------------------------------------------
// Description:
//    For single-sel listboxes. Returns the index of the
//    current selection.
//
// Returns:
//    zero-based index of current selection or -ve number if
//    no selection.
//----------------------------------------------------------------
int
TExpandableComboBox::GetSelIndex() const
{
  int index = TComboBox::GetSelIndex();
  return (GetEnabled(index)? index : CB_ERR);
}



//****************************************************
// protected methods...
//****************************************************

//----------------------------------------------------------------
// Description:
//    Cleans up interface elements
//
//----------------------------------------------------------------
void
TExpandableComboBox::CleanupWindow()
{
  ClearList();
  TComboBox::CleanupWindow();
}


//----------------------------------------------------------------
// Description:
//    Finds the extent of a given listbox string. Used in
//    calculating horizontal scrolling.
//
// Arguments:
//    str = string to find extent of
//
// Returns:
//    extent of string in logical units
//----------------------------------------------------------------
int
TExpandableComboBox::FindTextExtent(LPCTSTR str)
{
  PRECONDITION(str);
  TSize extent;

  TClientDC dc(GetHandle());
  HFONT hfont = GetWindowFont();

  if(hfont)  // non system font...
    dc.SelectObject(hfont);

  extent = dc.GetTabbedTextExtent(str, lstrlen(str), numTabs_, tabs_);

  if(hfont)
    dc.RestoreFont();

  return extent.cx;
}


//----------------------------------------------------------------
// Description:
//    Updates the extent of horizontal scrolling
//
//----------------------------------------------------------------
void
TExpandableComboBox::UpdateHorizontalExtent()
{
  //int greatestExtent;

  // find the extent of the largest string and
  // set the horizontal extent accordingly...
  int lastElement = textExtents_.GetItemsInContainer() - 1;
  if(lastElement < 0){
    // no elements in listbox.
    greatestExtent = 0;
  }
  else
    greatestExtent = textExtents_[lastElement];

  // add a bit of space so that last char is visible
  // when we scroll to right...
  TClientDC dc(GetHandle());
  greatestExtent += (dc.GetTextExtent(_T("X"), 1)).cx;

  // if longest str fits completely then scroll
  // listbox to the left, so Windows will hide
  // the scrollbar...
  TRect rect;
  GetClientRect(rect);

  int listWidth = rect.right - rect.left;
  if(listWidth >= greatestExtent)
    HandleMessage(WM_HSCROLL, SB_TOP, 0);

  SetHorizontalExtent(greatestExtent);
}



//----------------------------------------------------------------
// Description:
//    Stores the extent of the given string in our integer array.
//    Used by horizontal scrolling mechanism.
//
// Arguments:
//    str = string to store extent of.
//
//----------------------------------------------------------------
void
TExpandableComboBox::StoreStrExtent(LPCTSTR str)
{
  int length = FindTextExtent(str);
  textExtents_.Add(length);
  UpdateHorizontalExtent();
}



//----------------------------------------------------------------
// Description:
//    Responds to notification to draw entire control (actually
//    just a single item in the list)
//
//----------------------------------------------------------------
void
TExpandableComboBox::ODADrawEntire(DRAWITEMSTRUCT& drawInfo)
{
  // get text to display...
  int len = TComboBox::GetStringLen(drawInfo.itemID);
  LPTSTR buffer;

  if (len < 0){
    buffer = new _TCHAR[5];
    _tcscpy (buffer, _T(""));
  }
  else{
    buffer = new _TCHAR[len+1];
  }

  if (GetHandle() == drawInfo.hwndItem){

    //TRect cbRect = GetWindowRect();
    //int nThumb = GetSystemMetrics (SM_CXHTHUMB);
    //drawInfo.rcItem.right = drawInfo.rcItem.left + cbRect.Width() - nThumb -2 ;
    if (bShowCompleteLine_)
      GetString (buffer, drawInfo.itemID);
    else
      GetStringX (buffer, drawInfo.itemID);
  }
  else
    TComboBox::GetString(buffer, drawInfo.itemID);

  DrawListItem(drawInfo, buffer);

  delete[] buffer;
}


//----------------------------------------------------------------
// Description:
//    Responds to notification that focus has shifted to or from
//    an item.
//
//----------------------------------------------------------------
void
TExpandableComboBox::ODAFocus(DRAWITEMSTRUCT &drawInfo)
{
  TDC DC(drawInfo.hDC);
  TRect rc(drawInfo.rcItem);
  DC.DrawFocusRect(rc);
}


//----------------------------------------------------------------
// Description:
//    Responds to notification that selection state of item
//    has changed
//
//----------------------------------------------------------------
void
TExpandableComboBox::ODASelect(DRAWITEMSTRUCT &drawInfo)
{
  ODADrawEntire(drawInfo);
}


//----------------------------------------------------------------
// Description:
//    Handles WM_MEASUREITEM msg. owl sends this message from
//    parent back to the control.
//
//----------------------------------------------------------------
void
TExpandableComboBox::EvMeasureItem(uint /*ctrlId*/, MEASUREITEMSTRUCT &measure)
{
  TWindowDC dc(GetHandle());

  // get the height of the font...
  HFONT font = GetWindowFont();

  if(font == NULL) // system font is being used...
    dc.SelectStockObject(SYSTEM_FONT);
  else
    dc.SelectObject(font);

  TEXTMETRIC tm;
  dc.GetTextMetrics(tm);

  // use tmHeight to give a larger spacing,
  // but use tmAscent to duplicate Windows behaviour...
  measure.itemHeight = tm.tmHeight;
  nItemHeight = tm.tmHeight;

  dc.RestoreFont();
}



//----------------------------------------------------------------
// Description:
//    Query the background color to use for given item.
//
// Returns:
//    Color to use.
//----------------------------------------------------------------
TColor
TExpandableComboBox::QueryBkColor(DRAWITEMSTRUCT &drawInfo) const
{
  if((drawInfo.itemState & ODS_SELECTED) &&  GetEnabled(drawInfo.itemID)){
    // item is selected and
    // not disabled and
    // it's not a pseduo single-sel list with
    // this item unfocused...

    return TColor::SysHighlight;
  }
  else
    return TColor::SysWindow;
}




//----------------------------------------------------------------
// Description:
//    Query the text color to use for given item.
//
// Returns:
//    Text color.
//----------------------------------------------------------------
TColor
TExpandableComboBox::QueryTextColor(DRAWITEMSTRUCT &drawInfo) const
{
  TColor color;

  if(!GetEnabled(drawInfo.itemID)){  // item disabled
    // note we ignore the case where the device doesn't
    // support solid gray...
    TColor gray = TColor::SysGrayText;
    TDC dc(drawInfo.hDC);
    if(dc.GetBkColor() == TColor(gray)){
      // listbox background is same color as graytext
      // so choose the other gray...
      gray = (dc.GetBkColor() == TColor::LtGray? TColor::Gray : TColor::LtGray);
    }
    color = gray;
  }
  else{
    TCBItemData *id = GetItemDataPtr(drawInfo.itemID);
    if (!id || reinterpret_cast<INT_PTR>(id) == -1){
      color = (drawInfo.itemState & ODS_SELECTED?
        TColor::SysHighlightText : TColor::SysWindowText);
    }
    else{
      color = (drawInfo.itemState & ODS_SELECTED?
        TColor::SysHighlightText :
      (id->useSysColor_? TColor::SysWindowText : id->textColor_));
    }
  }
  return color;
}



//----------------------------------------------------------------
// Description:
//    Draws an item in the list.
//
// Arguments:
//    str = text to draw.
//
//----------------------------------------------------------------
void
TExpandableComboBox::DrawListItem(DRAWITEMSTRUCT &drawInfo, LPCTSTR str)
{
  TDC dc(drawInfo.hDC);
  TRect rc(drawInfo.rcItem);

  // first erase the item...
  TBrush brush(QueryBkColor(drawInfo));
  dc.FillRect(rc, brush);

  // make some space between left border of listbox and
  // first character of item. (This seems to match the space
  // allowed for in a standard listbox).
  rc.left += (2 * ::GetSystemMetrics(SM_CXBORDER));
  DrawText(drawInfo, rc, str);
}


//----------------------------------------------------------------
// Description:
//    Draws an item's text as part of the DrawListItem operation.
//
// Arguments:
//    rc = bounding rect
//    str = string to draw
//
//----------------------------------------------------------------
void
TExpandableComboBox::DrawText(DRAWITEMSTRUCT &drawInfo, const TRect& rc,
                LPCTSTR str)
{
  PRECONDITION(str);
  TDC DC(drawInfo.hDC);
  DC.SetBkMode(TRANSPARENT);

  DC.SetTextColor(QueryTextColor(drawInfo));

  // need to take care of tabs...
  DC.TabbedTextOut(rc.TopLeft(), str, -1, numTabs_, tabs_, 0);
}



void
TExpandableComboBox::CmKillFocus()
{
  static bool forParent = false; // to prevent infinite loop

  if(forParent)
    DefaultProcessing();
  else{
    // clean up the focus rect...
    TRect rc;
    if(GetItemRect(GetCaretIndex(), rc) != CB_ERR)
      InvalidateRect(rc);

    forParent = true;

    // notify parent...
    Parent->SendMessage(WM_COMMAND, Attr.Id, MAKELPARAM(GetHandle(), LBN_KILLFOCUS));
    forParent = false;
  }
}


//----------------------------------------------------------------
// Description:
//    Responds to LBN_SELCHANGE
//
//----------------------------------------------------------------
void
TExpandableComboBox::CmSelChange()
{
  static bool forParent = false; // to prevent infinite loop

  if(forParent)
    DefaultProcessing();
  else{
    forParent = true;
    // notify parent...

    if(Attr.Style & LBS_NOTIFY)
      Parent->SendMessage(WM_COMMAND, Attr.Id, MAKELPARAM(GetHandle(), LBN_SELCHANGE));
    forParent = false;
  }
}

//----------------------------------------------------------------
// Description:
//    Responds to LBN_SELENDOK
//
//----------------------------------------------------------------
void
TExpandableComboBox::CmSelEndOk()
{
  static bool forParent = false; // to prevent infinite loop

  if(forParent)
    DefaultProcessing();
  else{
    forParent = true;

    // notify parent...
    if(Attr.Style & LBS_NOTIFY)
      Parent->SendMessage(WM_COMMAND, Attr.Id,MAKELPARAM(GetHandle(), CBN_SELENDOK));
    forParent = false;
  }
}


///////////////////////
// New Code
LRESULT
TExpandableComboBox::DefWindowProc(uint msg, WPARAM wParam, LPARAM lParam)
{
  if (msg == WM_CTLCOLORLISTBOX) {
    if (hwndList == 0) {
      bFirstRedraw = true;
      hwndList = reinterpret_cast<HWND>(lParam);
      ::GetWindowRect(hwndList, &RectList);
      ::SendMessage (hwndList, WM_HSCROLL, SB_TOP, 0);
    }
    ResizeToFit();
  }
  return TComboBox::DefWindowProc(msg, wParam, lParam);
}



void
TExpandableComboBox::ResizeToFit()
{
  static bool bResizing = false;

  nPasses++;

  if (bResizing)
    return;
  bResizing = true;

  if (hwndList == 0){
    bResizing = false;
    MessageBeep(0);
    return;
  }

  TRect cbRect = GetWindowRect();
  int  nThumb = GetSystemMetrics (SM_CXHTHUMB);
  int  cbWidth = cbRect.Width();

  TRect ParentRect(Parent->GetWindowRect());

  // Sel
  int nlbPixels = 0;
  if (nUnitWidth_ > 0)
    nlbPixels = UnitsToPixels (nUnitWidth_);
  else if (nUnitWidth_ == 0){
    nlbPixels = cbRect.Width();
  }
  else if (nUnitWidth_ == -1){
    nlbPixels = greatestExtent;
    nlbPixels = std::max(cbWidth, nlbPixels);
  }
  else if (nUnitWidth_ < 0){
    nlbPixels = UnitsToPixels (nUnitWidth_);
    nlbPixels = std::min(greatestExtent, nlbPixels);
    nlbPixels = std::max(cbWidth, nlbPixels);
  }

  int nItems = GetCount();
  if (nItems <= 0)
    nItems = 1;
  int ht = nItems * nItemHeight;
  //

  TRect PopRect;
  ::GetWindowRect (hwndList, &PopRect);

  // Need to Adjust size for Left/Right/Top/Bottom Margins


  // Adjust for Direct Placement
  TRect NewRectSize;
  NewRectSize.left = cbRect.left;
  NewRectSize.top = PopRect.top; // cbRect.bottom+2;
  NewRectSize.right = NewRectSize.left + nlbPixels;

  //
  switch (fTCBAlign_){
    case eExpAlignDefault:
    case eExpAlignLeft:
      NewRectSize.right = std::min(NewRectSize.right, ParentRect.right - nThumb);
      break;
    case eExpAlignRight:     {
      int nShift = NewRectSize.Width() - cbRect.Width();
      OffsetRect(&NewRectSize, -nShift, 0);
      NewRectSize.left = std::max (NewRectSize.left, ParentRect.left + nThumb);
                 }
                 break;
    case eExpAlignCenter:
      break;
    default:
      break;
  }

  // Set the Horizontal Extents based on the new size of the box
  int nExtent;

  long wWord = ::GetWindowLong (hwndList, GWL_STYLE);
  if (greatestExtent > NewRectSize.Width()){
    // Add Scroll bar, if needed
    ht += GetSystemMetrics (SM_CYHSCROLL)+1;
    wWord |= WS_HSCROLL | LBS_USETABSTOPS;
    ::SetWindowLong (hwndList, GWL_STYLE, wWord);
    nExtent = greatestExtent;
  }
  else{
    ht += 2;
    wWord &= ~WS_HSCROLL;
    wWord |= LBS_USETABSTOPS;
    ::SetWindowLong (hwndList, GWL_STYLE, wWord);
    nExtent = 0;
  }
  NewRectSize.bottom = NewRectSize.top + ht;

  // Now Adjust the bottom size, so it doesn't go over the Dialog Box
  if (NewRectSize.bottom > ParentRect.bottom - nThumb){
    wWord |= WS_VSCROLL;
    NewRectSize.bottom = std::min(NewRectSize.bottom, ParentRect.bottom - nThumb);
  }
  else
    wWord &= ~WS_VSCROLL;

  ::SetWindowLong (hwndList, GWL_STYLE, wWord);

  // Never Size Less Than the Original Combo Box
  RectList = NewRectSize;

  ::SendMessage (hwndList, LB_SETHORIZONTALEXTENT, nExtent, TRUE);
  SetHorizontalExtent(nExtent);
  ::MoveWindow (hwndList, RectList.left, RectList.top, RectList.Width(), RectList.Height(), TRUE);

  if (bFirstRedraw){
    ::InvalidateRect (hwndList, NULL, FALSE);
    bFirstRedraw = false;
  }
  bResizing = false;
}



int
TExpandableComboBox::UnitsToPixels(int nUnits)
{
  int nDlgUnits = LOWORD (GetDialogBaseUnits());
  int nPixels = (nUnits * nDlgUnits) / (4);
  return nPixels;
}


int
TExpandableComboBox::PixelsToUnits(int nUnits)
{
  int nDlgUnits = LOWORD (GetDialogBaseUnits());
  int nPixels = (nUnits * (4)) / nDlgUnits;
  return nPixels;
}



int
TExpandableComboBox::GetStringX (LPTSTR str, int nIndex) const
{
  if (nIndex < 0)
    _tcscpy (str, _T(""));
  else{
    int nLen = GetStringLen(nIndex);
    _TCHAR* szTemp = new _TCHAR [nLen+2];

    GetString(szTemp, nIndex);
    _TCHAR* pTab = _tcschr(szTemp, _T('\t'));
    if (pTab)
      *pTab = NULL;
    _tcscpy (str, szTemp);
    delete []szTemp;
  }
  return static_cast<int>(_tcslen(str));
}


} // OwlExt namespace
//==============================================================================
