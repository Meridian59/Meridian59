//
/// \file
/// Implementation of TListViewCtrl and support classes
//
// Part of OWLNext - the next generation Object Windows Library 
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
// Copyright © 2011 Vidar Hasfjord 
//
// For more information, including license details, see 
// http://owlnext.sourceforge.net
//

#include <owl/pch.h>

#include <owl/listviewctrl.h>
#include <owl/system.h>
#include <algorithm>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;

//----------------------------------------------------------------------------
// TLvFindInfo

TLvFindInfo::TLvFindInfo()
{
  Init();
}

TLvFindInfo::TLvFindInfo(const LVFINDINFO& findInfo)
{
  *(LVFINDINFO*)this = findInfo;
  if ((flags & LVFI_STRING) || (flags & LVFI_PARTIAL))
  {
    Text = findInfo.psz;
    psz = Text.c_str();
  }
}

void
TLvFindInfo::SetString(const tstring& text)
{
  flags |= LVFI_STRING;
  Text = text;
  psz = Text.c_str();
}

void
TLvFindInfo::SetSubstring(const tstring& text)
{
  flags &= ~LVFI_PARTIAL;
  SetString(text);
}

void
TLvFindInfo::SetPartial(const tstring& text)
{
  flags |= LVFI_PARTIAL;
  SetString(text);
}

void
TLvFindInfo::SetWrap(bool should_wrap)
{
  if (should_wrap) 
    flags |= LVFI_WRAP;
  else 
    flags &= ~LVFI_WRAP;
}

void
TLvFindInfo::SetData(LPARAM param)
{
  flags |= LVFI_PARAM;
  lParam = param;
}

void
TLvFindInfo::Init()
{
  flags = 0;
  psz = 0;
  lParam = 0;

  Text.resize(0);
}

//----------------------------------------------------------------------------
// TLvHitTestInfo

TLvHitTestInfo::TLvHitTestInfo()
{
  Init();
}

TLvHitTestInfo::TLvHitTestInfo(const LVHITTESTINFO& info)
{
  *(LVHITTESTINFO*)this = info;
}

TLvHitTestInfo::TLvHitTestInfo(const TPoint& p)
{
  Init();
  SetPoint(p);
}

void
TLvHitTestInfo::SetPoint(const TPoint& p)
{
  pt.x = p.x;
  pt.y = p.y;
}

int
TLvHitTestInfo::GetIndex()
{
  return iItem;
}

int
TLvHitTestInfo::GetSubItem()
{
  return iSubItem;
}

uint
TLvHitTestInfo::GetFlags()
{
  return flags;
}

void
TLvHitTestInfo::Init()
{
  pt.x = 0;
  pt.y = 0;
  flags = 0;
  iItem = 0;
}

//----------------------------------------------------------------------------
// TLvItem

TLvItem::TLvItem(uint mask_, bool allocTextBuffer, int bufferSize)
{
  Init();
  if (allocTextBuffer)
  {
    CHECK(bufferSize > 0);
    Buffer.resize(bufferSize);
    SetTextBuffer(&Buffer[0], bufferSize);
  }
  mask = mask_;
}

TLvItem::TLvItem(const tstring& text, int subitemIndex)
{
  Init();
  SetText(text);
  SetSubItem(subitemIndex);
}

TLvItem::TLvItem(const TListViewCtrl& ctl, int index, int subitemIndex, uint mask_, int bufferSize)
{
  PRECONDITION(ctl.GetHandle());
  Init();
  mask = mask_;
  if (mask_ & LVIF_TEXT)
  {
    CHECK(bufferSize > 0);
    Buffer.resize(bufferSize);
    SetTextBuffer(&Buffer[0], bufferSize);
  }
  ctl.GetItem(*this, index, subitemIndex);  
}

TLvItem::TLvItem(const LVITEM& item)
{
  *this = item;
}

TLvItem::TLvItem(const TLvItem& item)
{
  *this = item;
}

TLvItem& TLvItem::operator =(const LVITEM& item)
{
  *static_cast<LVITEM*>(this) = item;
  if ((mask & LVIF_TEXT) && pszText != LPSTR_TEXTCALLBACK && pszText)
    SetText(pszText);
  return *this;
}

int
TLvItem::GetIndex() const
{
  return iItem;
}

int
TLvItem::GetSubItem() const
{
  return iSubItem;
}

LPCTSTR
TLvItem::GetText() const
{
  // Return early if we have no text to return.
  //
  if (!(mask & LVIF_TEXT) || pszText == LPSTR_TEXTCALLBACK || cchTextMax <= 0) return 0;
  
  // Check the length of the text; throw if it lacks termination within bounds.
  //
  LPTSTR i = std::find(pszText, pszText + cchTextMax, _T('\0'));
  size_t n = i - pszText;
  size_t m = static_cast<size_t>(cchTextMax);
  CHECK(n <= m);
  if (n == m) throw TXOwl(_T("TLvItem::GetText: Buffer overflow"));
  return pszText;
}

void
TLvItem::GetText(LPTSTR buffer, size_t bufferSize) const
{
  PRECONDITION(buffer);

  // First, check that there is room in the buffer; if not, then throw.
  // We could just return here, but that would leave the buffer in an undefined state,
  // with no indication that there was a problem.
  //
  if (bufferSize == 0) throw TXOwl(_T("TLvItem::GetText: Buffer size is 0"));

  // Now initialize the buffer, then return early if we have no text to copy.
  //
  buffer[0] = _T('\0');
  LPCTSTR text = GetText();
  if (!text) return;

  // Finally, we are ready to copy. 
  // We give a warning if we truncate the result here (but, ideally, we should throw).
  //
  size_t textLength = _tcslen(pszText);
  CHECK(textLength < static_cast<size_t>(cchTextMax));
  WARN(textLength >= bufferSize, _T("TLvItem::GetText: Truncation"));
  size_t n = std::min(textLength, bufferSize - 1);
  std::copy(pszText, pszText + n, buffer);
  buffer[n] = _T('\0');
}

void
TLvItem::SetTextBuffer(LPTSTR buffer, int bufferSize)
{
  mask |= LVIF_TEXT;
  pszText = buffer;
  cchTextMax = bufferSize;
}

void
TLvItem::SetText(const tstring& text)
{
  if (!IsRepresentable<int>(text.size() + 1))
    throw TXOwl(_T("TLvItem::SetText: The text argument is too large (>INT_MAX)"));

  Buffer.assign(text.begin(), text.end());
  Buffer.push_back(_T('\0'));
  SetTextBuffer(&Buffer[0], static_cast<int>(Buffer.size()));
}

void
TLvItem::SetIndex(int index)
{
  iItem = index;
}

void
TLvItem::SetSubItem(int subitemIndex)
{
  iSubItem = subitemIndex;
}

void
TLvItem::SetItemData(LPARAM data)
{
  mask  |= LVIF_PARAM;
  lParam = data;
}

LPARAM
TLvItem::GetItemData() const
{
  if (mask & LVIF_PARAM)
    return lParam;
  return 0;
}

int
TLvItem::GetImageIndex() const
{
  if (mask & LVIF_IMAGE)
    return iImage;
  return 0;
}

void
TLvItem::SetImageIndex(int index)
{
  mask |= LVIF_IMAGE;
  iImage = index;
}

int
TLvItem::GetState() const
{
  return (mask & LVIF_STATE) ? state : 0;
}

void
TLvItem::SetState(TListState newState)
{
  mask |= LVIF_STATE;
  stateMask |= state;
  state = newState;
}

int
TLvItem::GetStateImage() const
{
  return ((mask & LVIF_STATE) && (stateMask & LVIS_STATEIMAGEMASK)) ?
    (state >> 12) - 1 : 
    -1;
}

void
TLvItem::SetStateImage(int index)
{
  mask |= LVIF_STATE;
  stateMask |= LVIS_STATEIMAGEMASK;
  state = INDEXTOSTATEIMAGEMASK(index+1);
}

int
TLvItem::GetIndent() const
{
  return (mask & LVIF_INDENT) ? iIndent : -1;
}

void
TLvItem::SetIndent(int indent)
{
  mask |= LVIF_INDENT;
  iIndent = indent;
}

void
TLvItem::Init()
{
  memset(static_cast<LVITEM*>(this), 0, sizeof(LVITEM));
}

//----------------------------------------------------------------------------
// TLvColumn

TLvColumn::TLvColumn(uint mask_, int subitemIndex, int bufferSize)
{
  Init();
  mask = mask_;
  SetSubItem(subitemIndex);
  if (mask_ & LVCF_TEXT)
  {
    CHECK(bufferSize > 0);
    Buffer.resize(bufferSize);
    SetTextBuffer(&Buffer[0], bufferSize);
  }
}

TLvColumn::TLvColumn(const tstring& text, int width, TFormat how, int subitemIndex)
{
  Init();
  SetText(text);
  SetWidth(width, text);
  SetFormat(how);
  SetSubItem(subitemIndex);
}

TLvColumn::TLvColumn(const TListViewCtrl& ctl, int index, uint mask_, int subitemIndex, int bufferSize) 
{
  PRECONDITION(ctl.GetHandle());
  Init();
  mask = mask_;
  SetSubItem(subitemIndex);
  if (mask_ & LVCF_TEXT)
  {
    CHECK(bufferSize > 0);
    Buffer.resize(bufferSize);
    SetTextBuffer(&Buffer[0], bufferSize);
  }
  ctl.GetColumn(index, *this);
}

TLvColumn::TLvColumn(const LVCOLUMN& column)
{
  *this = column;
}

TLvColumn::TLvColumn(const TLvColumn& column)
{
  *this = column;
}

TLvColumn& TLvColumn::operator =(const LVCOLUMN& column)
{
  *static_cast<LVCOLUMN*>(this) = column;
  if ((mask & LVCF_TEXT) && pszText)
    SetText(pszText);
  return *this;
}

void
TLvColumn::SetTextBuffer(LPTSTR buffer, int bufferSize)
{
  mask |= LVCF_TEXT;
  pszText = buffer;
  cchTextMax = bufferSize;
}

void
TLvColumn::SetText(const tstring& text)
{
  if (!IsRepresentable<int>(text.size() + 1))
    throw TXOwl(_T("TLvColumn::SetText: The text argument is too large (>INT_MAX)"));

  Buffer.assign(text.begin(), text.end());
  Buffer.push_back(_T('\0'));
  SetTextBuffer(&Buffer[0], static_cast<int>(Buffer.size()));
}

void
TLvColumn::SetFormat(TFormat how)
{
  mask |= LVCF_FMT;
  fmt = static_cast<int>(how);
}

void
TLvColumn::SetWidth(int pixels, const tstring& text)
{
  mask |= LVCF_WIDTH;

  // Use user provided pixel size, or compute from text - if provided.
  //
  cx = (pixels) ? pixels :
    (text.length() != 0) ? TDefaultGUIFont().GetTextExtent(text).cx :
    0;
}

void
TLvColumn::SetSubItem(int item)
{
  mask |= LVCF_SUBITEM;
  iSubItem = item;
}

void
TLvColumn::SetImage(int image)
{
  mask |= LVCF_IMAGE;
  iImage = image;
}

void
TLvColumn::SetOrder(int order)
{
  mask |= LVCF_ORDER;
  iOrder = order;
}

void
TLvColumn::Init()
{
  memset(static_cast<LVCOLUMN*>(this), 0, sizeof(LVCOLUMN));
}

//----------------------------------------------------------------------------
// TListViewCtrl

TListViewCtrl::TListViewCtrl(TWindow* parent, int id, int x, int y, int w, int h, TModule* module)
:
  TControl(parent, id, 0, x, y, w, h, module)
{
  if (!TCommCtrl::IsAvailable())
    throw TXCommCtrl();

  Attr.ExStyle |= WS_EX_CLIENTEDGE;
}

TListViewCtrl::TListViewCtrl(TWindow* parent, int resourceId, TModule* module)
:
  TControl(parent, resourceId, module)
{
  if (!TCommCtrl::IsAvailable())
    throw TXCommCtrl();
}

TLvItem
TListViewCtrl::GetItem(int index, int subitemIndex) const
{
  TLvItem item;
  bool r = GetItem(item, index, subitemIndex);
  WARN(!r, _T("TListViewCtrl::GetItem failed for index ") << index << _T(".") << subitemIndex); InUse(r);
  return item;
}

bool
TListViewCtrl::SetItemState(int index, uint state, uint mask)
{
  LVITEM item;
  item.state = state;
  item.stateMask = mask;
  return ToBool(SendMessage(LVM_SETITEMSTATE, index, TParam2(&item)));
}

namespace
{

  //
  // Create a temporary structure to store additional information for the
  // comparison object.
  //
  struct TListCompareThunk
  {
    const TListViewCtrl::TCompareFunc* This;
    LPARAM ItemData;
  };

  int CALLBACK OwlListViewCompare(LPARAM itemData1, LPARAM itemData2, LPARAM lParam)
  {
    TListCompareThunk* ct = reinterpret_cast<TListCompareThunk*>(lParam);
    return ct->This->Compare(itemData1, itemData2, ct->ItemData);
  }

}

bool
TListViewCtrl::SortItems(const TCompareFunc& comparator, LPARAM lParam)
{
  TListCompareThunk ct;
  ct.This = &comparator;
  ct.ItemData = lParam;
  return ToBool(SendMessage(LVM_SORTITEMS, TParam1(&ct), TParam2(OwlListViewCompare)));
}

bool
TListViewCtrl::SortItemsEx(const TCompareFunc& comparator, LPARAM lParam)
{
  TListCompareThunk ct;
  ct.This = &comparator;
  ct.ItemData = lParam;
  return ToBool(SendMessage(LVM_SORTITEMSEX, TParam1(&ct), TParam2(OwlListViewCompare)));
}

int
TListViewCtrl::GetSelIndex() const
{
  return (GetStyle() & LVS_SINGLESEL) ?
    static_cast<int>(SendMessage(LVM_GETNEXTITEM, static_cast<TParam1>(-1), MkParam2(LVNI_ALL | LVNI_SELECTED, 0))) :
    -1;
}

int
TListViewCtrl::GetSelCount() const
{
  return static_cast<int>(SendMessage(LVM_GETSELECTEDCOUNT));
}

int
TListViewCtrl::GetSelIndexes(int* indexes, int maxCount) const
{
  if (GetStyle() & LVS_SINGLESEL)
    return -1;

  // Get the index of the first selected item.
  //
  int index = static_cast<int>(
    SendMessage(LVM_GETNEXTITEM, TParam1(-1), MkParam2(LVNI_ALL|LVNI_SELECTED, 0)));

  // Loop while index indicates a selected item.
  //
  int count = 0; // number of selected items found
  while (index != -1 && count < maxCount) 
  {
    indexes[count] = index;
    count++;

    // Get the next selected item.
    //
    index = static_cast<int>(
      SendMessage(LVM_GETNEXTITEM, TParam1(index), MkParam2(LVNI_ALL|LVNI_SELECTED, 0)));
  }
  return count;
}

int
TListViewCtrl::GetSelStrings(tchar** strs, int maxCount, int maxChars, int subitemIndex) const
{
  // This function is for multiselect list views only.
  //
  if (GetStyle() & LVS_SINGLESEL)
    return -1;

  // Get the index of the first selected item.
  //
  int index = static_cast<int>(
    SendMessage(LVM_GETNEXTITEM, TParam1(-1), MkParam2(LVNI_ALL|LVNI_SELECTED, 0)));

  // Loop through selected items.
  //
  int count = 0;
  while (index != -1) {
    GetItemText(index, subitemIndex,
      strs[count], maxChars);
    count++;
    if (count > maxCount)
      break;

    // Get the next selected item.
    //
    index = static_cast<int>(
      SendMessage(LVM_GETNEXTITEM, TParam1(index), MkParam2(LVNI_ALL|LVNI_SELECTED, 0)));
  }
  return count;
}

bool
TListViewCtrl::IsSelected(int index) const
{
  return GetItemState(index, LVIS_SELECTED);
}

bool
TListViewCtrl::SetSel(int index, bool select)
{
  return SetItemState(index, select ? LVIS_SELECTED : 0, LVIS_SELECTED);
}

bool
TListViewCtrl::SetSelIndexes(int* indexes, int count, bool select)
{
  // This command is only for multiselect list views.
  //
  if (GetStyle() & LVS_SINGLESEL)
    return false;

  for (int i = 0; i < count; i++)
    if (!SetSel(indexes[i], select))
      return false;

  return true;
}

bool
TListViewCtrl::SetSelItemRange(bool select, int first, int last)
{
  // This command is only for multiselect list views.
  //
  if (GetStyle() & LVS_SINGLESEL)
    return false;

  for (int i = first; i <= last; i++)
    if (!SetSel(i, select))
      return false;

  return true;
}

int
TListViewCtrl::AddItem(const TLvItem& item)
{
  PRECONDITION(GetHandle());
  return InsertItem(item, GetItemCount());
}

int  
TListViewCtrl::AddItem(const tstring& newItemText)
{
  TLvItem item(newItemText.c_str());
  item.iItem =  GetItemCount();
  return InsertItem(item);
}

int
TListViewCtrl::InsertItem(const TLvItem& item, int index)
{
  PRECONDITION(GetHandle());
  if (index != -1)
  {
    LVITEM temp = item; // Make s shallow copy, just to pass to LVM_INSERTITEM
    temp.iItem = index;
    return static_cast<int>(SendMessage(LVM_INSERTITEM, 0, TParam2(&temp)));
  }
  else
    return static_cast<int>(SendMessage(LVM_INSERTITEM, 0, TParam2(&item)));
}

int
TListViewCtrl::InsertItem(const tstring& text, int index)
{
  TLvItem item(text.c_str());
  item.iItem = index;
  return InsertItem(item);
}

bool
TListViewCtrl::Arrange(TArrangeCode code)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_ARRANGE, TParam1(code)));
}

HIMAGELIST
TListViewCtrl::CreateDragImage(int itemIndex, TPoint* upLeft)
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HIMAGELIST>(SendMessage(LVM_CREATEDRAGIMAGE, TParam1(itemIndex), TParam2(upLeft)));
}

bool
TListViewCtrl::DeleteColumn(int colNum)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_DELETECOLUMN, TParam1(colNum)));
}

bool
TListViewCtrl::DeleteAllItems()
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_DELETEALLITEMS));
}

bool
TListViewCtrl::DeleteItem(int itemIndex)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_DELETEITEM, TParam1(itemIndex)));
}

HWND
TListViewCtrl::EditLabel(int itemIndex)
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HWND>(SendMessage(LVM_EDITLABEL, TParam1(itemIndex)));
}

bool
TListViewCtrl::EnsureVisible(int index, bool partialOk)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_ENSUREVISIBLE, TParam1(index), TParam2(partialOk)));
}

int
TListViewCtrl::FindItem(int startIndex, const TLvFindInfo& findInfo)
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_FINDITEM, TParam1(startIndex), TParam2(&findInfo)));
}

int
TListViewCtrl::InsertColumn(int colNum, const TLvColumn& colItem)
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_INSERTCOLUMN, TParam1(colNum), TParam2(&colItem)));
}

uint
TListViewCtrl::GetCallBackMask() const
{
  PRECONDITION(GetHandle());
  return static_cast<uint>(SendMessage(LVM_GETCALLBACKMASK));
}

bool
TListViewCtrl::GetColumn(int index, TLvColumn& column) const
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETCOLUMN, TParam1(index), TParam2(&column)));
}

int
TListViewCtrl::GetColumnWidth(int index) const
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETCOLUMNWIDTH, TParam1(index)));
}

int
TListViewCtrl::GetCountPerPage() const
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETCOUNTPERPAGE));
}

HWND
TListViewCtrl::GetEditControl() const
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HWND>(SendMessage(LVM_GETEDITCONTROL));
}

HIMAGELIST
TListViewCtrl::GetImageList(TImageListType type)
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HIMAGELIST>(SendMessage(LVM_GETIMAGELIST, TParam1(type)));
}

bool
TListViewCtrl::GetItem(TLvItem& item, int index, int subitemIndex) const
{
  PRECONDITION(GetHandle());
  if (index != -1)
    item.iItem = index;
  if (subitemIndex != -1)
    item.iSubItem = subitemIndex;
  return ToBool(SendMessage(LVM_GETITEM, 0, TParam2(&item)));
}

int
TListViewCtrl::GetItemCount() const
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETITEMCOUNT));
}

bool
TListViewCtrl::GetItemPosition(int index, TPoint& pt) const
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETITEMPOSITION, TParam1(index), TParam2(&pt)));
}

bool
TListViewCtrl::GetItemRect(int index, TRect& r, TItemRectType type)
{
  PRECONDITION(GetHandle());
  r.left = type;
  return ToBool(SendMessage(LVM_GETITEMRECT, TParam1(index), TParam2(&r)));
}

uint
TListViewCtrl::GetItemState(int index, uint mask) const
{
  PRECONDITION(GetHandle());
  return static_cast<uint>(SendMessage(LVM_GETITEMSTATE, TParam1(index), TParam2(mask)));
}

int
TListViewCtrl::GetItemText(int index, int subitemIndex, LPTSTR buffer, int bufferSize) const
{
  PRECONDITION(GetHandle());
  TLvItem item;
  item.SetTextBuffer(buffer, bufferSize);
  item.SetSubItem(subitemIndex);
  return static_cast<int>(SendMessage(LVM_GETITEMTEXT, TParam1(index), TParam2(&item)));
}

tstring
TListViewCtrl::GetItemText(int index, int subitemIndex) const
{
  PRECONDITION(GetHandle());
  TLvItem item;
  item.SetSubItem(subitemIndex);
  SendMessage(LVM_GETITEMTEXT, TParam1(index), TParam2(&item));
  return item.GetText();
}

int
TListViewCtrl::GetNextItem(int index, TNextItemCode code) const
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETNEXTITEM, TParam1(index), MkParam2(code, 0)));
}

bool
TListViewCtrl::GetOrigin(TPoint & p)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETORIGIN, 0, TParam2(&p)));
}

int
TListViewCtrl::GetStringWidth(LPTSTR text)
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETSTRINGWIDTH, 0, TParam2(text)));
}

int
TListViewCtrl::GetTopIndex() const
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETTOPINDEX));
}

bool
TListViewCtrl::GetViewRect(TRect & r)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETVIEWRECT, 0, TParam2(&r)));
}

int
TListViewCtrl::HitTest(TLvHitTestInfo& info)
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_HITTEST, 0, TParam2(&info)));
}

bool
TListViewCtrl::RedrawItems(int startIndex, int endIndex)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_REDRAWITEMS, TParam1(startIndex), TParam2(endIndex)));
}

bool
TListViewCtrl::Scroll(int dx, int dy)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SCROLL, 0, MkParam2(dx, dy)));
}

bool
TListViewCtrl::SetBkColor(const TColor& c)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETBKCOLOR, 0, TParam2(c.GetValue())));
}

bool
TListViewCtrl::SetCallBackMask(uint mask)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETCALLBACKMASK, mask));
}

bool
TListViewCtrl::SetColumn(int index, const TLvColumn& column)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETCOLUMN, TParam1(index), TParam2(&column)));
}

bool
TListViewCtrl::SetColumnWidth(int index, int width)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETCOLUMNWIDTH, TParam1(index), MkParam2(width, 0)));
}

bool
TListViewCtrl::SetImageList(HIMAGELIST list, TImageListType type)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETIMAGELIST, TParam1(type), TParam2(list)));
}

bool
TListViewCtrl::SetItem(const TLvItem& item, int index, int subitemIndex)
{
  PRECONDITION(GetHandle());
  
  if (index != -1 || subitemIndex != -1)
  {
    LVITEM temp = item; // Make s shallow copy, just to pass to LVM_INSERTITEM
  
    if (index != -1)
      temp.iItem = index;
    if (subitemIndex != -1)
      temp.iSubItem = subitemIndex;

    return ToBool(SendMessage(LVM_SETITEM, 0, TParam2(&temp)));
  }
  else
    return ToBool(SendMessage(LVM_SETITEM, 0, TParam2(&item)));
}

void
TListViewCtrl::SetItemCount(int numItems, uint32 flags)
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_SETITEMCOUNT, TParam1(numItems), flags);
}

bool 
TListViewCtrl::SetItemPosition(int index, const TPoint& pt) 
{
  PRECONDITION(GetHandle());
  if (!IsRepresentable<uint16>(pt.x) || !IsRepresentable<uint16>(pt.y))
    throw TXOwl(_T("TListViewCtrl:SetItemPosition: Argument is outside valid range"));
  return ToBool(SendMessage(LVM_SETITEMPOSITION, TParam1(index), MkParam2(pt.x, pt.y)));
}

bool
TListViewCtrl::SetItemText(int index, const TLvItem& item)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETITEMTEXT, TParam1(index), TParam2(&item)));
}

bool
TListViewCtrl::SetItemText(int index, int subitemIndex, LPCTSTR text)
{
  PRECONDITION(GetHandle());
  return SetItemText(index, TLvItem(text, subitemIndex));
}

bool
TListViewCtrl::SetItemText(int index, int subitemIndex, const tstring& text)
{
  PRECONDITION(GetHandle());
  return SetItemText(index, subitemIndex, text.c_str());
}

bool
TListViewCtrl::SetTextBkColor(const TColor& c)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETTEXTBKCOLOR, 0, TParam2(c.GetValue())));
}

bool
TListViewCtrl::SetTextColor(const TColor& c)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETTEXTCOLOR, 0, TParam2(c.GetValue())));
}

bool
TListViewCtrl::Update(int index)
{
  PRECONDITION(GetHandle());

  // If the list window has LVS_AUTOARRANGE, the items are automatically
  // arranged to their proper locations.

  return ToBool(SendMessage(LVM_UPDATE, TParam1(index)));
}

TSize
TListViewCtrl::GetApproxRect(int x, int y, int count) const
{
  PRECONDITION(GetHandle());
  if (x != -1 && !IsRepresentable<uint16>(x) || y != -1 && !IsRepresentable<uint16>(y))
    throw TXOwl(_T("TListViewCtrl:GetApproxRect: Argument is outside valid range"));
  TResult r = SendMessage(LVM_APPROXIMATEVIEWRECT, TParam1(count), MkParam2(x, y));
  return TPoint(static_cast<DWORD>(r));
}

TSize
TListViewCtrl::GetApproxRect(const TPoint& pt, int count) const
{
  PRECONDITION(GetHandle());
  return GetApproxRect(pt.x, pt.y, count);
}

bool
TListViewCtrl::GetBkImage(TLvBkImage& bkimg) const
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETBKIMAGE, 0, TParam2(&bkimg)));
}

bool
TListViewCtrl::SetBkImage(const TLvBkImage& bkimg)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETBKIMAGE, 0, TParam2(&bkimg)));
}

bool
TListViewCtrl::GetColumnOrder(int count, int* array) const
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETCOLUMNORDERARRAY, TParam1(count), TParam2(array)));
}

bool
TListViewCtrl::SetColumnOrder(int count, int* array)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETCOLUMNORDERARRAY, TParam1(count), TParam2(array)));
}

uint32
TListViewCtrl::GetExtStyle() const
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE));
}

void
TListViewCtrl::SetExtStyle(uint32 mask, uint32 style)
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, TParam1(mask), TParam2(style));
}

HWND
TListViewCtrl::GetHeaderCtrl() const
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HWND>(SendMessage(LVM_GETHEADER));
}

HCURSOR
TListViewCtrl::GetHotCursor() const
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HCURSOR>(SendMessage(LVM_GETHOTCURSOR));
}

void
TListViewCtrl::SetHotCursor(HCURSOR cur)
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_SETHOTCURSOR, 0, TParam2(cur));
}

int
TListViewCtrl::GetHotItem() const
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETHOTITEM));
}

void
TListViewCtrl::SetHotItem(int item)
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_SETHOTITEM, TParam1(item));
}

uint32
TListViewCtrl::GetHoverTime() const
{
  PRECONDITION(GetHandle());
  return static_cast<uint32>(SendMessage(LVM_GETHOVERTIME));
}

void
TListViewCtrl::SetHoverTime(uint32 tm)
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_SETHOVERTIME, 0, TParam2(tm));
}

uint
TListViewCtrl::GetNumOfWorkAreas() const
{
  PRECONDITION(GetHandle());
  uint retval;
  SendMessage(LVM_GETNUMBEROFWORKAREAS, 0, TParam2(&retval));
  return retval;
}

void
TListViewCtrl::GetWorkAreas(int count, TRect* areas) const
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_GETWORKAREAS, TParam1(count), TParam2(areas));
}

void
TListViewCtrl::SetWorkAreas(int count, TRect* areas)
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_SETWORKAREAS, TParam1(count), TParam2(areas));
}


int
TListViewCtrl::GetSelectionMark() const
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETSELECTIONMARK));
}

bool
TListViewCtrl::GetSubItemRect(TRect& rect, int subidx, int paridx, int flag) const
{
  PRECONDITION(GetHandle());
  rect.top = subidx;
  rect.left = flag;
  return ToBool(SendMessage(LVM_GETSUBITEMRECT, TParam1(paridx), TParam2(&rect)));
}

int
TListViewCtrl::SubItemTest(const TLvHitTestInfo& info) const
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_SUBITEMHITTEST, 0, TParam2(&info)));
}

void
TListViewCtrl::SetIconSpacing(int x, int y)
{
  PRECONDITION(GetHandle());
  if (y != -1 && (x < 0 || y < 0))
    throw TXOwl(_T("TListViewCtrl:SetIconSpacing: Argument is outside valid range"));
  SendMessage(LVM_SETICONSPACING, TParam1(x), y);
}

void
TListViewCtrl::SetIconSpacing(const TPoint& pt)
{
  PRECONDITION(GetHandle());
  SetIconSpacing(pt.x, pt.y);
}

void
TListViewCtrl::SetSelectionMark(int index)
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_SETSELECTIONMARK, 0, TParam2(index));
}

#if defined(OWL5_COMPAT)

COLORREF
TListViewCtrl::GetBkColor()
{
  PRECONDITION(GetHandle());
  return static_cast<COLORREF>(SendMessage(LVM_GETBKCOLOR));
}

COLORREF
TListViewCtrl::GetTextBkColor()
{
  PRECONDITION(GetHandle());
  return static_cast<COLORREF>(SendMessage(LVM_GETTEXTBKCOLOR));
}

COLORREF
TListViewCtrl::GetTextColor()
{
  PRECONDITION(GetHandle());
  return static_cast<COLORREF>(SendMessage(LVM_GETTEXTCOLOR));
}

bool
TListViewCtrl::GetColumn(int index, LVCOLUMN* column)
{
  PRECONDITION(column);
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETCOLUMN, TParam1(index), TParam2(column)));
}

bool
TListViewCtrl::GetItemPosition(int index, POINT * p)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETITEMPOSITION, TParam1(index), TParam2(p)));
}

bool
TListViewCtrl::GetItemRect(int index, RECT* r, TItemRectType type)
{
  PRECONDITION(GetHandle());
  r->left = type;
  return ToBool(SendMessage(LVM_GETITEMRECT, TParam1(index), TParam2(r)));
}

bool
TListViewCtrl::GetOrigin(POINT* p)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETORIGIN, 0, TParam2(p)));
}

bool
TListViewCtrl::GetViewRect(RECT* r)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETVIEWRECT, 0, TParam2(r)));
}

bool
TListViewCtrl::SetBkColor(COLORREF c)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETBKCOLOR, 0, TParam2(c)));
}

bool
TListViewCtrl::SetTextBkColor(COLORREF c)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETTEXTBKCOLOR, 0, TParam2(c)));
}

bool
TListViewCtrl::SetTextColor(COLORREF c)
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETTEXTCOLOR, 0, TParam2(c)));
}

#else

TColor
TListViewCtrl::GetBkColor()
{
  PRECONDITION(GetHandle());
  return static_cast<COLORREF>(SendMessage(LVM_GETBKCOLOR));
}

TColor
TListViewCtrl::GetTextBkColor()
{
  PRECONDITION(GetHandle());
  return static_cast<COLORREF>(SendMessage(LVM_GETTEXTBKCOLOR));
}

TColor
TListViewCtrl::GetTextColor()
{
  PRECONDITION(GetHandle());
  return static_cast<COLORREF>(SendMessage(LVM_GETTEXTCOLOR));
}

#endif

TWindow::TGetClassNameReturnType
TListViewCtrl::GetClassName()
{
  return WC_LISTVIEW;
}

} // OWL namespace

