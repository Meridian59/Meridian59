//
/// \file
/// Implementation of TListViewCtrl and support classes
//
// Part of OWLNext - the next generation Object Windows Library
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
// Copyright (c) 2011 Vidar Hasfjord
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

//
/// Constructs with an empty string and parameter.
//
TLvFindInfo::TLvFindInfo()
{
  flags = 0;
  psz = nullptr;
  lParam = 0;

  Text.resize(0);
}

//
/// Constructs a copy of another class instance.
//
/// @param[in] findInfo is the class instance to copy.
//
TLvFindInfo::TLvFindInfo(const LVFINDINFO& findInfo)
{
  *static_cast<LVFINDINFO*>(this) = findInfo;
  if ((flags & LVFI_STRING) || (flags & LVFI_PARTIAL))
  {
    Text = findInfo.psz;
    psz = Text.c_str();
  }
}

//
/// Sets a string to be used for the search.
//
/// @param[in] text is the string to use.
//
/// \return none.
//
auto
TLvFindInfo::SetString(const tstring& text) -> void
{
  flags |= LVFI_STRING;
  Text = text;
  psz = Text.c_str();
}

//
/// Sets a string to be used for an exact search.
//
/// @param[in] text is the string to use.
//
/// \return none.
//
auto
TLvFindInfo::SetSubstring(const tstring& text) -> void
{
  flags &= ~LVFI_PARTIAL;
  SetString(text);
}

//
/// Sets a string to be used for a partial search.
//
/// @param[in] text is the string to use.
//
/// \return none.
//
auto
TLvFindInfo::SetPartial(const tstring& text) -> void
{
  flags |= LVFI_PARTIAL;
  SetString(text);
}

//
/// Sets the setting for wrap-around during a search.
//
/// @param[in] wrap specifies whether the search continues at the beginning when the end has been reached.
//
/// \return none.
//
auto
TLvFindInfo::SetWrap(bool wrap) -> void
{
  if (wrap)
    flags |= LVFI_WRAP;
  else
    flags &= ~LVFI_WRAP;
}

//
/// Sets extra application-specific information.
//
/// @param[in] param is the extra information to set.
//
/// \return none.
//
auto
TLvFindInfo::SetData(LPARAM param) -> void
{
  flags |= LVFI_PARAM;
  lParam = param;
}

//----------------------------------------------------------------------------
// TLvItem

//
/// Constructs an empty parameter package.
//
/// This constructor is typically used to retrieve information about an existing item.
/// For example,
//
/// \code
///   TLvItem item;
///   bool success = ListWindow.GetItem(item, index, subitemIndex);
///   if (success && item.GetText()) ShowString(item.GetText());
/// \endcode
//
/// @param[in] mask_ is the mask to use.
/// @param[in] allocTextBuffer if true, then a character buffer is allocated, and pszText is set to point to it.
/// @param[in] bufferSize is the size of the text characters to allocate and use if allocTextBuffer is true.
//
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

//
/// Constructs a parameter package with the given text.
//
/// The given text is copied to the internal buffer, and pszText will point to the internal copy.
/// For example,
//
/// \code
///   TLvItem item("Item");
///   ListWindow.AddItem(item);
/// \endcode
//
/// @param[in] text is the string to use.
/// @param[in] subitemIndex is the subitem index to use.
//
TLvItem::TLvItem(const tstring& text, int subitemIndex)
{
  Init();
  SetText(text);
  SetSubItem(subitemIndex);
}

//
/// Constructs a parameter package based on the item attributes of an existing control.
//
/// @param[in] ctl is an existing list-view control from which to copy.
/// @param[in] index is the item index of the existing list-view control to use for copying.
/// @param[in] subitemIndex is the subitem index to use.
/// @param[in] mask_ is the mask to use for copying.
/// @param[in] bufferSize is the size of the text characters to allocate and use, provided the mask includes LVIF_TEXT.
//
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

//
/// Constructs a parameter package from an existing parameter package.
//
/// A deep copy is performed; see the assignment operator for `const LVITEM&` for details.
//
/// @param[in] item is an existing list-view item class from which to copy.
//
TLvItem::TLvItem(const LVITEM& item)
{
  *this = item;
}

//
/// Constructs a parameter package from an existing parameter package.
//
/// A deep copy is performed; see the copy assignment operator for details.
//
/// @param[in] item is an existing list-view item class from which to copy.
//
TLvItem::TLvItem(const TLvItem& item)
{
  *this = item;
}

//
/// Overloads the assignment operator to perform a deep copy using a separate string buffer if applicable.
//
/// @param[in] item is an existing list-view item class from which to copy.
//
auto
TLvItem::operator =(const LVITEM& item) -> TLvItem&
{
  *static_cast<LVITEM*>(this) = item;
  if ((mask & LVIF_TEXT) && (pszText != LPSTR_TEXTCALLBACK) && pszText)
    SetText(pszText);
  return *this;
}

//
/// Copies the given parameter package.
//
/// A deep copy is performed; see the assignment operator for `const LVITEM&` for details.
//
auto
TLvItem::operator =(const TLvItem& item) -> TLvItem&
{
  return operator =(static_cast<const LVITEM&>(item));
}

//
/// Retrieves the current text for the item.
//
/// \note This might point to the internal text buffer, or to an external buffer.
//
/// \return a pointer to the text which will be nullptr if not used.
//
/// \sa SetTextBuffer.
//
auto
TLvItem::GetText() const -> LPCTSTR
{
  // Return early if we have no text to return.
  //
  if (!(mask & LVIF_TEXT) || (pszText == LPSTR_TEXTCALLBACK) || (cchTextMax <= 0)) return nullptr;

  // Check the length of the text; throw if it lacks termination within bounds.
  //
  LPTSTR i = std::find(pszText, pszText + cchTextMax, _T('\0'));
  size_t n = i - pszText;
  size_t m = static_cast<size_t>(cchTextMax);
  CHECK(n <= m);
  if (n == m) throw TXOwl(_T("TLvItem::GetText: Buffer overflow"));
  return pszText;
}

//
/// Copies the current text for the item.
//
/// @param[out] buffer upon return will contain the text string.
/// @param[in] bufferSize specifies the maximum number of characters to copy;
/// to determine an adequate size use (_tcslen(GetText()) + 1).
//
/// \return none.
//
auto
TLvItem::GetText(LPTSTR buffer, size_t bufferSize) const -> void
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

//
/// Overrides the internal buffer and assigns an external text buffer.
//
/// Sets the pszText and cchTextMax members of LVITEM, and enables the LVIF_TEXT flag.
//
/// \note This function does not copy the text!
//
/// @param[in] buffer pointer to a text string buffer to use.
/// @param[in] bufferSize specifies the size of the text string buffer.
//
/// \return none.
//
auto
TLvItem::SetTextBuffer(LPTSTR buffer, int bufferSize) -> void
{
  mask |= LVIF_TEXT;
  pszText = buffer;
  cchTextMax = bufferSize;
}

//
/// Copies the given text into the internal text buffer.
//
/// Sets the pszText and cchTextMax members of LVITEM, and enables the LVIF_TEXT flag.
//
/// \note If the string is larger than what can be represented by the class, i.e. INT_MAX, then
/// a TXOwl exception is thrown.
//
/// @param[in] text is the string to copy.
//
/// \return none.
//
auto
TLvItem::SetText(const tstring& text) -> void
{
  if (!IsRepresentable<int>(text.size() + 1))
    throw TXOwl(_T("TLvItem::SetText: The text argument is too large (>INT_MAX)"));

  Buffer.assign(text.begin(), text.end());
  Buffer.push_back(_T('\0'));
  SetTextBuffer(&Buffer[0], static_cast<int>(Buffer.size()));
}

//
/// Retrieves the item index.
//
/// \return the item index.
//
auto
TLvItem::GetIndex() const -> int
{
  return iItem;
}

//
/// Sets the item index.
//
/// @param[in] index the item index.
//
/// \return none.
//
auto
TLvItem::SetIndex(int index) -> void
{
  iItem = index;
}

//
/// Retrieves the subitem index.
//
/// \return the subitem index.
//
auto
TLvItem::GetSubItem() const -> int
{
  return iSubItem;
}

//
/// Sets the subitem index.
//
/// @param[in] subitemIndex the subitem index.
//
/// \return none.
//
auto
TLvItem::SetSubItem(int subitemIndex) -> void
{
  iSubItem = subitemIndex;
}

//
/// Retrieves the application-defined data.
//
/// \return the application-defined data, or 0 if not used.
//
auto
TLvItem::GetItemData() const -> LPARAM
{
  return (mask & LVIF_PARAM) ? lParam : 0;
}

//
/// Sets the application-defined data.
//
/// @param[in] param the application-defined data.
//
/// \return none.
//
auto
TLvItem::SetItemData(LPARAM param) -> void
{
  mask |= LVIF_PARAM;
  lParam = param;
}

//
/// Retrieves the current image index used for the item.
//
/// \return the image index, or -1 if not used.
//
auto
TLvItem::GetImageIndex() const -> int
{
  return (mask & LVIF_IMAGE) ? iImage : -1;
}

//
/// Sets the image index within the image list to use.
//
/// @param[in] image is the image index to use.
//
/// \return none.
//
auto
TLvItem::SetImageIndex(int image) -> void
{
  mask |= LVIF_IMAGE;
  iImage = image;
}

//
/// Retrieves the current state used for the item.
//
/// \return the state, or 0 if not used.
//
auto
TLvItem::GetState() const -> int
{
  return (mask & LVIF_STATE) ? state : 0;
}

//
/// Sets the state to use.
//
/// @param[in] newState is the state to use.
//
/// \return none.
//
auto
TLvItem::SetState(TListState newState) -> void
{
  mask |= LVIF_STATE;
  stateMask |= state;
  state = newState;
}

//
/// Retrieves the current state image index used for the item.
//
/// \return the state image index, or -1 if not used.
//
auto
TLvItem::GetStateImage() const -> int
{
  return ((mask & LVIF_STATE) && (stateMask & LVIS_STATEIMAGEMASK)) ? (state >> 12) - 1 : -1;
}

//
/// Sets the state image index to use.
//
/// @param[in] stateIndex is the state image index to use.
//
/// \return none.
//
auto
TLvItem::SetStateImage(int stateIndex) -> void
{
  mask |= LVIF_STATE;
  stateMask |= LVIS_STATEIMAGEMASK;
  state = INDEXTOSTATEIMAGEMASK(stateIndex + 1);
}

//
/// Retrieves the current number of image widths to indent the item.
//
/// \return the number of image widths, or -1 if not used.
//
auto
TLvItem::GetIndent() const -> int
{
  return (mask & LVIF_INDENT) ? iIndent : -1;
}

//
/// Sets the number of image widths to indent the item.
//
/// @param[in] indent the number of image widths to use.
//
/// \return none.
//
auto
TLvItem::SetIndent(int indent) -> void
{
  mask |= LVIF_INDENT;
  iIndent = indent;
}

//
/// Initializes member data; used by internal routines.
//
/// \return none.
//
auto
TLvItem::Init() -> void
{
  memset(static_cast<LVITEM*>(this), 0, sizeof(LVITEM));
}

//----------------------------------------------------------------------------
// TLvColumn

//
/// Constructs an empty parameter package.
//
/// This constructor is typically used to retrieve information about an existing column.
/// For example,
//
/// \code
///    TLvColumn col;
///    bool success = ListWnd.GetColumn(1, col);
///    if (success && col.GetText()) ShowString(col.GetText());
/// \endcode
//
/// @param[in] mask_ is the mask to use.
/// @param[in] subitemIndex is the subitem index to use.
/// @param[in] bufferSize is the size of the text characters to allocate and use, provided the mask includes LVCF_TEXT.
//
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

//
/// Constructs a parameter package based on the given text, width, format, and subitem.
//
/// @param[in] text is the string to use.
/// @param[in] width is the pixel width to use the column; if 0, a width will be calculated by SetWidth.
/// @param[in] how specifies the formatting to use for the string.
/// @param[in] subitemIndex is the subitem index to use.
//
TLvColumn::TLvColumn(const tstring& text, int width, TFormat how, int subitemIndex)
{
  Init();
  SetText(text);
  SetWidth(width, text);
  SetFormat(how);
  SetSubItem(subitemIndex);
}

//
/// Constructs a parameter package based on the column attributes of an existing control.
//
/// @param[in] ctl is an existing list-view control from which to copy.
/// @param[in] index is the item index of the existing list-view control to use for copying.
/// @param[in] mask_ is the mask to use for copying.
/// @param[in] subitemIndex is the subitem index to use.
/// @param[in] bufferSize is the size of the text characters to allocate and use, provided the mask includes LVCF_TEXT.
//
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

//
/// Constructs a parameter package from an existing parameter package.
//
/// A deep copy is performed; see the assignment operator for `const LVCOLUMN&` for details.
//
/// @param[in] column is an existing list-view column class from which to copy.
//
TLvColumn::TLvColumn(const LVCOLUMN& column)
{
  *this = column;
}

//
/// Constructs a parameter package from an existing parameter package.
//
/// A deep copy is performed; see the copy assignment operator for details.
//
/// @param[in] column is an existing list-view column class from which to copy.
//
TLvColumn::TLvColumn(const TLvColumn& column)
{
  *this = column;
}

//
/// Overloads the assignment operator to perform a deep copy using a separate string buffer if applicable.
//
/// @param[in] column is an existing list-view column class from which to copy.
//
auto
TLvColumn::operator =(const LVCOLUMN& column) -> TLvColumn&
{
  *static_cast<LVCOLUMN*>(this) = column;
  if ((mask & LVCF_TEXT) && pszText)
    SetText(pszText);
  return *this;
}

//
/// Copies the given parameter package.
//
/// A deep copy is performed; see the assignment operator for `const LVCOLUMN&` for details.
//
auto
TLvColumn::operator =(const TLvColumn& column) -> TLvColumn&
{
  return operator =(static_cast<const LVCOLUMN&>(column));
}

//
/// Overrides the internal buffer and assigns an external text buffer.
//
/// Sets the pszText and cchTextMax members of LVCOLUMN, and enables the LVCF_TEXT flag.
//
/// \note This function does not copy the text!
//
/// @param[in] buffer pointer to a text string buffer to use.
/// @param[in] bufferSize specifies the size of the text string buffer.
//
/// \return none.
//
auto
TLvColumn::SetTextBuffer(LPTSTR buffer, int bufferSize) -> void
{
  mask |= LVCF_TEXT;
  pszText = buffer;
  cchTextMax = bufferSize;
}

//
/// Copies the given text into the internal text buffer.
//
/// Sets the pszText and cchTextMax members of LVCOLUMN, and enables the LVCF_TEXT flag.
//
/// \note If the string is larger than what can be represented by the class, i.e. INT_MAX, then
/// a TXOwl exception is thrown.
//
/// @param[in] text is the string to copy.
//
/// \return none.
//
auto
TLvColumn::SetText(const tstring& text) -> void
{
  if (!IsRepresentable<int>(text.size() + 1))
    throw TXOwl(_T("TLvColumn::SetText: The text argument is too large (>INT_MAX)"));

  Buffer.assign(text.begin(), text.end());
  Buffer.push_back(_T('\0'));
  SetTextBuffer(&Buffer[0], static_cast<int>(Buffer.size()));
}

//
/// Sets the alignment format for the column.
//
/// @param[in] how specifies the formatting to use for the string.
//
/// \return none.
//
auto
TLvColumn::SetFormat(TFormat how) -> void
{
  if (how != Unspecified)
  {
    mask |= LVCF_FMT;
    fmt = static_cast<int>(how);
  }
}

//
/// Sets the width of the column.
//
/// @param[in] width is the pixel width to use the column; if 0, a width will be calculated by
/// using the default GUI system font for the string.
/// @param[in] text is the string to use for calculating a width when width is 0.
//
/// \return none.
//
/// \sa TListViewCtrl::GetStringWidth(const tstring&) const
/// \sa TListViewCtrl::CalculateColumnWidth(const tstring&, int padding) const
//
auto
TLvColumn::SetWidth(int width, const tstring& text) -> void
{
  mask |= LVCF_WIDTH;
  cx = (width || !text.length()) ? width : TDefaultGuiFont(TDefaultGuiFont::sfiIcon).GetTextExtent(text).cx;
}

//
/// Sets the subitem index.
//
/// @param[in] subitemIndex is the subitem index to use.
//
/// \return none.
//
auto
TLvColumn::SetSubItem(int subitemIndex) -> void
{
  mask |= LVCF_SUBITEM;
  iSubItem = subitemIndex;
}

//
/// Retrieves the current text for the column.
//
/// \return a pointer to the text which will be nullptr if not used.
//
auto
TLvColumn::GetText() const -> LPCTSTR
{
  return pszText;
}

//
/// Retrieves the current alignment format for the column.
//
/// \return the alignment format.
//
auto
TLvColumn::GetFormat() const -> TFormat
{
  return (mask & LVCF_FMT) ? TFormat(fmt) : Unspecified;
}

//
/// Retrieves the current width for the column.
//
/// \return the pixel width, or -1 if not used.
//
auto
TLvColumn::GetWidth() const -> int
{
  return (mask & LVCF_WIDTH) ? cx : -1;
}

//
/// Retrieves the current subitem used for the column.
//
/// \return the subitem index, or -1 if not used.
//
auto
TLvColumn::GetSubItem() const -> int
{
  return (mask & LVCF_SUBITEM) ? iSubItem : -1;
}

//
/// Retrieves the current image index used for the column.
//
/// \return the image index, or -1 if not used.
//
auto
TLvColumn::GetImage() const -> int
{
  return (mask & LVCF_IMAGE) ? iImage : -1;
}

//
/// Sets the image index within the image list to use.
//
/// @param[in] image is the image index to use.
//
/// \return none.
//
auto
TLvColumn::SetImage(int image) -> void
{
  mask |= LVCF_IMAGE;
  iImage = image;
}

//
/// Retrieves the current column order offset used for the column.
//
/// \return the column order offset, or -1 if not used.
//
auto
TLvColumn::GetOrder() const -> int
{
  return (mask & LVCF_ORDER) ? iOrder : -1;
}

//
/// Sets the column order offset.
//
/// @param[in] order is the zero-based left-to-right column offset to use.
//
/// \return none.
//
auto
TLvColumn::SetOrder(int order) -> void
{
  mask |= LVCF_ORDER;
  iOrder = order;
}

//
/// Initializes member data; used by internal routines.
//
/// \return none.
//
auto
TLvColumn::Init() -> void
{
  memset(static_cast<LVCOLUMN*>(this), 0, sizeof(LVCOLUMN));
}

//----------------------------------------------------------------------------
// TLvTileViewInfo

//
/// Constructs an automatic-sized tile view.
//
TLvTileViewInfo::TLvTileViewInfo()
{
  cbSize = sizeof(LVTILEVIEWINFO);
  dwMask = 0;
  dwFlags = LVTVIF_AUTOSIZE;
  sizeTile = {0, 0};
  cLines = 0;
  rcLabelMargin = {0, 0, 0, 0};
}

//
/// Constructs an automatic-sized tile view specified with
/// the maximum number of text lines in each item label.
//
/// @param[in] lines the maximum number of text lines in each item label.
//
TLvTileViewInfo::TLvTileViewInfo(int lines)
{
  cbSize = sizeof(LVTILEVIEWINFO);
  dwMask = LVTVIM_COLUMNS;
  dwFlags = LVTVIF_AUTOSIZE;
  sizeTile = {0, 0};
  cLines = lines;
  rcLabelMargin = {0, 0, 0, 0};
}

//
/// Constructs an automatic-sized tile view specified with
/// the coordinates of the label margin.
//
/// @param[in] labelMargin the coordinates of the label margin.
//
TLvTileViewInfo::TLvTileViewInfo(const TRect& labelMargin)
{
  cbSize = sizeof(LVTILEVIEWINFO);
  dwMask = LVTVIM_LABELMARGIN;
  dwFlags = LVTVIF_AUTOSIZE;
  sizeTile = {0, 0};
  cLines = 0;
  rcLabelMargin = labelMargin;
}

//
/// Constructs an automatic-sized tile view specified with
/// the maximum number of text lines in each item label
/// and
/// the coordinates of the label margin.
//
/// @param[in] lines the maximum number of text lines in each item label.
/// @param[in] labelMargin the coordinates of the label margin.
//
TLvTileViewInfo::TLvTileViewInfo(int lines, const TRect& labelMargin)
{
  cbSize = sizeof(LVTILEVIEWINFO);
  dwMask = LVTVIM_COLUMNS | LVTVIM_LABELMARGIN;
  dwFlags = LVTVIF_AUTOSIZE;
  sizeTile = {0, 0};
  cLines = lines;
  rcLabelMargin = labelMargin;
}

//
/// Constructs an fixed-sized tile view.
//
/// @param[in] size the tile size.
/// @param[in] fixedSize the fixed-size type.
//
TLvTileViewInfo::TLvTileViewInfo(const TSize& size, TLvTileViewInfo::TTileSize fixedSize)
{
  cbSize = sizeof(LVTILEVIEWINFO);
  dwMask = LVTVIM_TILESIZE;
  dwFlags = static_cast<DWORD>(fixedSize);
  sizeTile = size;
  cLines = 0;
  rcLabelMargin = {0, 0, 0, 0};
}

//
/// Constructs an fixed-sized tile view specified with
/// the maximum number of text lines in each item label.
//
/// @param[in] size the tile size.
/// @param[in] fixedSize the fixed-size type.
/// @param[in] lines the maximum number of text lines in each item label.
//
TLvTileViewInfo::TLvTileViewInfo(const TSize& size, TLvTileViewInfo::TTileSize fixedSize, int lines)
{
  cbSize = sizeof(LVTILEVIEWINFO);
  dwMask = LVTVIM_TILESIZE | LVTVIM_COLUMNS;
  dwFlags = static_cast<DWORD>(fixedSize);
  sizeTile = size;
  cLines = lines;
  rcLabelMargin = {0, 0, 0, 0};
}

//
/// Constructs an fixed-sized tile view specified with
/// the coordinates of the label margin.
//
/// @param[in] size the tile size.
/// @param[in] fixedSize the fixed-size type.
/// @param[in] labelMargin the coordinates of the label margin.
//
TLvTileViewInfo::TLvTileViewInfo(const TSize& size, TLvTileViewInfo::TTileSize fixedSize, const TRect& labelMargin)
{
  cbSize = sizeof(LVTILEVIEWINFO);
  dwMask = LVTVIM_TILESIZE | LVTVIM_LABELMARGIN;
  dwFlags = static_cast<DWORD>(fixedSize);
  sizeTile = size;
  cLines = 0;
  rcLabelMargin = labelMargin;
}

//
/// Constructs an fixed-sized tile view specified with
/// the maximum number of text lines in each item label
/// and
/// the coordinates of the label margin.
//
/// @param[in] size the tile size.
/// @param[in] fixedSize the fixed-size type.
/// @param[in] lines the maximum number of text lines in each item label.
/// @param[in] labelMargin the coordinates of the label margin.
//
TLvTileViewInfo::TLvTileViewInfo(const TSize& size, TLvTileViewInfo::TTileSize fixedSize, int lines, const TRect& labelMargin)
{
  cbSize = sizeof(LVTILEVIEWINFO);
  dwMask = LVTVIM_TILESIZE | LVTVIM_COLUMNS | LVTVIM_LABELMARGIN;
  dwFlags = static_cast<DWORD>(fixedSize);
  sizeTile = size;
  cLines = lines;
  rcLabelMargin = labelMargin;
}

//
/// Sets the tile view size to automatic.
//
/// \return none.
//
auto
TLvTileViewInfo::SetSizeAutomatic() -> void
{
  dwFlags = LVTVIF_AUTOSIZE;
  dwMask &= ~static_cast<DWORD>(LVTVIM_TILESIZE);
}

//
/// Sets the tile view size to fixed-width and/or fixed-height.
//
/// @param[in] size the tile size.
/// @param[in] fixedSize the fixed-size type.
//
/// \return none.
//
auto
TLvTileViewInfo::SetSizeFixed(const TSize& size, TLvTileViewInfo::TTileSize fixedSize) -> void
{
  dwFlags = static_cast<DWORD>(fixedSize);
  sizeTile = size;
  dwMask |= LVTVIM_TILESIZE;
}

//
/// Sets the specified maximum number of text lines in each item label.
//
/// @param[in] lines the maximum number of text lines in each item label.
//
/// \return none.
//
auto
TLvTileViewInfo::SetMaxTextLines(int lines) -> void
{
  cLines = lines;
  dwMask |= LVTVIM_COLUMNS;
}

//
/// Removes the specified maximum number of text lines in each item label.
//
/// \return none.
//
auto
TLvTileViewInfo::RemoveMaxTextLines() -> void
{
  cLines = 0;
  dwMask &= ~static_cast<DWORD>(LVTVIM_COLUMNS);
}

//
/// Sets the specified coordinates of the label margin.
//
/// @param[in] labelMargin the coordinates of the label margin.
//
/// \return none.
//
auto
TLvTileViewInfo::SetLabelMargin(const TRect& labelMargin) -> void
{
  rcLabelMargin = labelMargin;
  dwMask |= LVTVIM_LABELMARGIN;
}

//
/// Removes the specified coordinates of the label margin.
//
/// \return none.
//
auto
TLvTileViewInfo::RemoveLabelMargin() -> void
{
  rcLabelMargin = {0, 0, 0, 0};
  dwMask &= ~static_cast<DWORD>(LVTVIM_LABELMARGIN);
}

//
/// Sets the specified tile size.
//
/// @param[in] size the tile size.
//
/// \return none.
//
auto
TLvTileViewInfo::SetTileSize(const TSize& size) -> void
{
  sizeTile = size;
}

//----------------------------------------------------------------------------
// TListViewCtrl

//
/// Constructor that simply passes the parameters to the parent
/// \link TControl::TControl(TWindow*, int, LPCTSTR, int, int, int, int, TModule*) \endlink
/// and adds the extended style WS_EX_CLIENTEDGE.
//
TListViewCtrl::TListViewCtrl(TWindow* parent, int id, int x, int y, int w, int h, TModule* module)
:
  TControl(parent, id, 0, x, y, w, h, module)
{
  InitializeCommonControls(ICC_LISTVIEW_CLASSES);
  Attr.ExStyle |= WS_EX_CLIENTEDGE;
}

//
/// Constructor that simply passes the parameters to the parent
/// \link TControl::TControl(TWindow*, int, TModule*) \endlink.
//
TListViewCtrl::TListViewCtrl(TWindow* parent, int resourceId, TModule* module)
:
  TControl(parent, resourceId, module)
{
  InitializeCommonControls(ICC_LISTVIEW_CLASSES);
}

//
/// Inserts a new column in a list-view control.
//
/// @param[in] colNum is the column number.
/// @param[in] column contains the column attributes.
//
/// \note Columns are visible only in report (details) view.
//
/// \return column number if successful, otherwise -1.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761101.aspx
//
auto
TListViewCtrl::InsertColumn(int colNum, const TLvColumn& column) -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_INSERTCOLUMN, TParam1(colNum), TParam2(&column)));
}

//
/// Deletes a column in a list-view control.
//
/// @param[in] colNum is the column number.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774894.aspx
//
auto
TListViewCtrl::DeleteColumn(int colNum) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_DELETECOLUMN, TParam1(colNum)));
}

//
/// Retrieves the attributes of a list-view control's column.
//
/// @param[in] colNum is the column number.
/// @param[out] column contains the column attributes.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774911.aspx
//
auto
TListViewCtrl::GetColumn(int colNum, TLvColumn& column) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETCOLUMN, TParam1(colNum), TParam2(&column)));
}

//
/// Retrieves the width of a column in report or list view.
//
/// @param[in] colNum is the column number.
//
/// \return the column width.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774915.aspx
//
auto
TListViewCtrl::GetColumnWidth(int colNum) const -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETCOLUMNWIDTH, TParam1(colNum)));
}

//
/// Sets the attributes of a list-view control's column.
//
/// @param[in] colNum is the column number.
/// @param[in] column contains the column attributes.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761159.aspx
//
auto
TListViewCtrl::SetColumn(int colNum, const TLvColumn& column) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETCOLUMN, TParam1(colNum), TParam2(&column)));
}

//
/// Sets the width of a column in report or list view.
//
/// @param[in] colNum is the column number.
/// @param[in] width is the new column width.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761163.aspx
//
auto
TListViewCtrl::SetColumnWidth(int colNum, int width) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETCOLUMNWIDTH, TParam1(colNum), width));
}

//
/// Retrieves a list-view item's attributes.
//
/// @param[in,out] item specifies the attributes to retrieve, and upon return contains those attributes.
/// @param[in] index is the item index, which overrides the setting in item unless -1.
/// @param[in] subitemIndex is the subitem index, which overrides the setting in item unless -1.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774953.aspx
//
auto
TListViewCtrl::GetItem(TLvItem& item, int index, int subitemIndex) const -> bool
{
  PRECONDITION(GetHandle());
  if (index != -1)
    item.iItem = index;
  if (subitemIndex != -1)
    item.iSubItem = subitemIndex;
  return ToBool(SendMessage(LVM_GETITEM, 0, TParam2(&item)));
}

//
/// TLvItem return overload for \link GetItem(TLvItem&, int, int) const \endlink.
//
/// @param[in] index is the item index, which defaults to the first item if -1.
/// @param[in] subitemIndex is the subitem index, which defaults to no subitem if -1.
//
/// \return TLvItem.
//
auto
TListViewCtrl::GetItem(int index, int subitemIndex) const -> TLvItem
{
  TLvItem item;
  auto r = GetItem(item, index, subitemIndex);
  WARN(!r, _T("TListViewCtrl::GetItem failed for index ") << index << _T(".") << subitemIndex); InUse(r);
  return item;
}

//
/// Sets a list-view item's attributes.
//
/// @param[in] item contains the attributes to set.
/// @param[in] index is the item index, which overrides the setting in item unless -1.
/// @param[in] subitemIndex is the subitem index, which overrides the setting in item unless -1.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761186.aspx
//
auto
TListViewCtrl::SetItem(const TLvItem& item, int index, int subitemIndex) -> bool
{
  PRECONDITION(GetHandle());

  if (index != -1 || subitemIndex != -1)
  {
    LVITEM temp = item; // Make s shallow copy, just to pass to LVM_SETITEM

    if (index != -1)
      temp.iItem = index;
    if (subitemIndex != -1)
      temp.iSubItem = subitemIndex;

    return ToBool(SendMessage(LVM_SETITEM, 0, TParam2(&temp)));
  }
  else
    return ToBool(SendMessage(LVM_SETITEM, 0, TParam2(&item)));
}

//
/// Searches for a list-view item that has the specified properties and bears
/// the specified relationship to a specified item.
//
/// @param[in] index is the item index to begin the search; -1 specifies to find the first item.
/// @param[in] flags is a combination of TNextItemCode flags that specify the relationship to the item given by index.
//
/// \return item index if found, otherwise -1.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761057.aspx, TListViewCtrl::TNextItemCode
//
auto
TListViewCtrl::GetNextItem(int index, uint flags) const -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETNEXTITEM, TParam1(index), flags));
}

//
/// Retrieves the number of items in the list-view control.
//
/// \return number of items.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761044.aspx
//
auto
TListViewCtrl::GetItemCount() const -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETITEMCOUNT));
}

//
/// Sets the number of items that the list-view control will ultimately contain.
//
/// Causes the list-view control to allocate memory for the number of items
/// specified in `numItems` or sets the virtual number of items for a virtual
/// list-view control depending on how the control was created, and returns
/// whether successful.
//
/// @param[in] numItems is the new number of items.
/// @param[in] behavior is the behavior of the list-view control after setting the item count.
//
/// \note
/// -# If the list-view control was created without the LVS_OWNERDATA style,
/// sending this message causes the control to allocate its internal data
/// structures for the specified number of items. This prevents the control from
/// having to allocate the data structures every time an item is added.
/// -# If the list-view control was created with the LVS_OWNERDATA style (a virtual
/// list-view), sending this message sets the virtual number of items that the
/// control contains.
/// -# The `behavior` parameter is intended only for controls
/// that use the LVS_OWNERDATA and LVS_REPORT or LVS_LIST styles.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761188.aspx
//
auto
TListViewCtrl::SetItemCount(int numItems, TSetItemCountBehavior behavior) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETITEMCOUNT, TParam1(numItems), behavior));
}

//
/// Retrieves the position of a list-view item.
//
/// @param[in] index is the item index.
/// @param[out] pt is where the position will be stored.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761048.aspx
//
auto
TListViewCtrl::GetItemPosition(int index, TPoint& pt) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETITEMPOSITION, TParam1(index), TParam2(&pt)));
}

//
/// TPoint return overload for \link GetItemPosition(int, TPoint&) const \endlink.
//
/// @param[in] index is the item index.
//
/// \return position of the list-view item.
//
auto
TListViewCtrl::GetItemPosition(int index) const -> TPoint
{
  TPoint pt;
  auto r = GetItemPosition(index, pt);
  WARN(!r, _T("TListViewCtrl::GetItemPosition failed for index ") << index); InUse(r);
  return pt;
}

//
/// Sets the position of a list-view item.
//
/// @param[in] index is the item index.
/// @param[in] pt is the new item position.
//
/// \note If the coordinates are outside valid range, a TXOwl exception is thrown. Use
/// \link SetItemPosition32 \endlink to avoid this.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761192.aspx
//
auto
TListViewCtrl::SetItemPosition(int index, const TPoint& pt) -> bool
{
  PRECONDITION(GetHandle());
  WARN(true, _T("TListViewCtrl::SetItemPosition: Use TListViewCtrl::SetItemPosition32 instead to avoid possible exception"));
  if (!IsRepresentable<uint16>(pt.x) || !IsRepresentable<uint16>(pt.y))
    throw TXOwl(_T("TListViewCtrl:SetItemPosition: Argument is outside valid range"));
  return ToBool(SendMessage(LVM_SETITEMPOSITION, TParam1(index), MkParam2(pt.x, pt.y)));
}

//
/// Sets the position of a list-view item.
//
/// \note Must be in icon or small icon view.
//
/// @param[in] index is the item index.
/// @param[in] pt is the new item position.
//
/// \return none.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761194.aspx
//
auto
TListViewCtrl::SetItemPosition32(int index, const TPoint& pt) -> void
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_SETITEMPOSITION32, TParam1(index), TParam2(&pt));
}

//
/// Retrieves the bounding rectangle of a list-view item.
//
/// @param[in] index is the item index.
/// @param[out] rect will contain the bounding rectangle.
/// @param[in] type specifies the type of rectangle to retrieve.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761049.aspx
//
auto
TListViewCtrl::GetItemRect(int index, TRect& rect, TItemRectType type) const -> bool
{
  PRECONDITION(GetHandle());
  rect.left = type;
  return ToBool(SendMessage(LVM_GETITEMRECT, TParam1(index), TParam2(&rect)));
}

//
/// TRect return overload for \link GetItemRect(int, TRect&, TItemRectType) const \endlink.
//
/// @param[in] index is the item index.
/// @param[in] type specifies the type of rectangle to retrieve.
//
/// \return the bounding rectangle.
//
auto
TListViewCtrl::GetItemRect(int index, TItemRectType type) const -> TRect
{
  TRect rect;
  auto r = GetItemRect(index, rect, type);
  WARN(!r, _T("TListViewCtrl::GetItemRect failed for index ") << index); InUse(r);
  return rect;
}

//
/// Retrieves the current state for an item.
//
/// @param[in] index is the item index.
/// @param[in] mask specifies which state attributes to return.
//
/// \return current state of the item.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761053.aspx
//
auto
TListViewCtrl::GetItemState(int index, uint mask) const -> TLvItem::TListState
{
  PRECONDITION(GetHandle());
  return static_cast<TLvItem::TListState>(SendMessage(LVM_GETITEMSTATE, TParam1(index), TParam2(mask)));
}

//
/// Sets the current state for an item.
//
/// @param[in] index is the item index.
/// @param[in] state is the new item state.
/// @param[in] mask specifies which state attributes to set.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761196.aspx
//
auto
TListViewCtrl::SetItemState(int index, TLvItem::TListState state, uint mask) -> bool
{
  LVITEM item;
  item.state = state;
  item.stateMask = mask;
  return ToBool(SendMessage(LVM_SETITEMSTATE, index, TParam2(&item)));
}

//
/// Retrieves the text of the item.
//
/// @param[in] index is the item index.
/// @param[in,out] item is the class where the buffer, buffer size, and subitem
/// index are specified. Before calling this function:
/// -# call item.SetTextBuffer(buffer, bufferSize) to set the buffer and buffer size
/// where the text will get stored.
/// -# call item.SetSubItem(subitemIndex) where subitemIndex is 0 to retrieve the
/// item text rather than the text of a subitem.
//
/// \return length of the retrieved string.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761055.aspx
//
auto
TListViewCtrl::GetItemText(int index, TLvItem& item) const -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETITEMTEXT, TParam1(index), TParam2(&item)));
}

//
/// C string overload for \link GetItemText(int, TLvItem&) const \endlink.
//
/// @param[in] index is the item index.
/// @param[in] subitemIndex is the subitem index; 0 specifies to retrieve the
/// item text rather than the text of a subitem.
/// @param[out] buffer points to a buffer to hold the retrieved text.
/// @param[in] bufferSize is the size of the buffer pointed to by text.
//
/// \return length of the retrieved string.
//
auto
TListViewCtrl::GetItemText(int index, int subitemIndex, LPTSTR buffer, int bufferSize) const -> int
{
  PRECONDITION(GetHandle());
  TLvItem item;
  item.SetTextBuffer(buffer, bufferSize);
  item.SetSubItem(subitemIndex);
  return GetItemText(index, item);
}

//
/// String return overload for \link GetItemText(int, TLvItem&) const \endlink.
//
/// @param[in] index is the item index.
/// @param[in] subitemIndex is the subitem index; 0 specifies to retrieve the
/// item text rather than the text of a subitem.
//
/// \return retrieved string.
//
auto
TListViewCtrl::GetItemText(int index, int subitemIndex) const -> tstring
{
  PRECONDITION(GetHandle());
  TLvItem item;
  item.SetSubItem(subitemIndex);
  SendMessage(LVM_GETITEMTEXT, TParam1(index), TParam2(&item));
  return item.GetText();
}

//
/// Sets the text of a list-view item or subitem.
//
/// @param[in] index is the item index.
/// @param[in] item is the class where the buffer, buffer size, and subitem
/// index are specified. Before calling this function:
/// -# call item.SetTextBuffer(buffer, bufferSize) to set the buffer and buffer size
/// of the text to set.
/// -# call item.SetSubItem(subitemIndex) where subitemIndex is 0 to set the
/// item text rather than the text of a subitem.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761198.aspx
//
auto
TListViewCtrl::SetItemText(int index, const TLvItem& item) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETITEMTEXT, TParam1(index), TParam2(&item)));
}

//
/// C string overload for \link SetItemText(int, const TLvItem&) \endlink.
//
/// @param[in] index is the item index.
/// @param[in] subitemIndex is the subitem index; 0 specifies to set the
/// item text rather than the text of a subitem.
/// @param[in] text is the text to set.
//
/// \return true if successful.
//
auto
TListViewCtrl::SetItemText(int index, int subitemIndex, LPCTSTR text) -> bool
{
  PRECONDITION(GetHandle());
  return SetItemText(index, TLvItem(text, subitemIndex));
}

//
/// String overload for \link SetItemText(int, const TLvItem&) \endlink.
//
/// @param[in] index is the item index.
/// @param[in] subitemIndex is the subitem index; 0 specifies to set the
/// item text rather than the text of a subitem.
/// @param[in] text is the text to set.
//
/// \return true if successful.
//
auto
TListViewCtrl::SetItemText(int index, int subitemIndex, const tstring& text) -> bool
{
  PRECONDITION(GetHandle());
  return SetItemText(index, subitemIndex, text.c_str());
}

//
/// Adds a new item to the end of the list.
//
/// @param[in] item is the class containing the item.
//
/// \return the index of the new item if successful, otherwise -1.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761107.aspx
//
auto
TListViewCtrl::AddItem(const TLvItem& item) -> int
{
  PRECONDITION(GetHandle());
  return InsertItem(item, GetItemCount());
}

//
/// Adds a new item with the given text to the end of the list.
//
/// @param[in] text is the text to add.
//
/// \return the index of the new item if successful, otherwise -1.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761107.aspx
//

auto
TListViewCtrl::AddItem(const tstring& text) -> int
{
  TLvItem item(text.c_str());
  item.iItem =  GetItemCount();
  return InsertItem(item);
}

//
/// Inserts a new item to the list.
//
/// @param[in] item is the class containing the item.
/// @param[in] index is the item index to place the new item, which overrides
/// the setting in item unless -1.
//
/// \return the index of the new item if successful, otherwise -1.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761107.aspx
//
auto
TListViewCtrl::InsertItem(const TLvItem& item, int index) -> int
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

//
/// Inserts a new item with the given text to the list.
//
/// @param[in] text is the text to add.
/// @param[in] index is the item index to place the new item.
//
/// \return the index of the new item if successful, otherwise -1.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761107.aspx
//
auto
TListViewCtrl::InsertItem(const tstring& text, int index) -> int
{
  TLvItem item(text.c_str());
  item.iItem = index;
  return InsertItem(item);
}

//
/// Deletes an item from the list.
//
/// @param[in] index is the item index to delete.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774895.aspx
//
auto
TListViewCtrl::DeleteItem(int index) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_DELETEITEM, TParam1(index)));
}

//
/// Deletes all items from the list.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774891.aspx
//
auto
TListViewCtrl::DeleteAllItems() -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_DELETEALLITEMS));
}

//
/// Searches for an item with the specified characteristics.
//
/// @param[in] index is the item index from where to begin the search,
/// non-inclusive. If -1 the search will start from the beginning.
/// @param[in] findInfo is the class containing the search characteristics.
//
/// \return the index of the item if found, otherwise -1.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774903.aspx
//
auto
TListViewCtrl::FindItem(int index, const TLvFindInfo& findInfo) const -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_FINDITEM, TParam1(index), TParam2(&findInfo)));
}

// Hidden from public structure and function by using anonymous namespace.
//
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

//
/// Sorts the list items.
//
/// @param[in] comparator is an application-defined derived class of TCompareFunc that performs the comparison.
/// @param[in] lParam is an application-defined value that will be passed to the comparison function.
//
/// \return true if successful.
//
/// \note
/// -# The first two parameters passed to the comparison function are the values
/// returned by TLvItem::GetItemData for the two items to be compared.
/// -# During the sorting process, the list-view contents are unstable. If the
/// comparator class causes any messages to be sent to the list-view control
/// that modify the list-view contents, the results are unpredictable.
//
/// \sa SortItemsEx
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761227.aspx
//
auto
TListViewCtrl::SortItems(const TCompareFunc& comparator, LPARAM lParam) -> bool
{
  TListCompareThunk ct;
  ct.This = &comparator;
  ct.ItemData = lParam;
  return ToBool(SendMessage(LVM_SORTITEMS, TParam1(&ct), TParam2(OwlListViewCompare)));
}

//
/// Sorts the list items.
//
/// This function differs from \link SortItems \endlink only in that the first two
/// parameters passed to the comparison function are the item indexes for the two
/// items to be compared.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761228.aspx
//
auto
TListViewCtrl::SortItemsEx(const TCompareFunc& comparator, LPARAM lParam) -> bool
{
  TListCompareThunk ct;
  ct.This = &comparator;
  ct.ItemData = lParam;
  return ToBool(SendMessage(LVM_SORTITEMSEX, TParam1(&ct), TParam2(OwlListViewCompare)));
}

//
/// Arranges items in icon view.
//
/// @param[in] code is the class specifying the type of arrangement.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774884.aspx
//
auto
TListViewCtrl::Arrange(TArrangeCode code) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_ARRANGE, TParam1(code)));
}

//
/// Queries if an item is selected.
//
/// @param[in] index is the item index.
//
/// \return true if item is selected.
//
auto
TListViewCtrl::IsSelected(int index) const -> bool
{
  return GetItemState(index, LVIS_SELECTED);
}

//
/// Searches for the next selected item after a given index.
//
/// @param[in] index is the item index to begin the search; -1 specifies to find the first item.
//
/// \return the item index of the next selected item, or -1 if there isn't one.
//
auto
TListViewCtrl::GetNextSelIndex(int index) const -> int
{
  return GetNextItem(index, static_cast<TNextItemCode>(MkParam2(LVNI_ALL | LVNI_SELECTED, 0)));
}

//
/// Retrieves the selected item index.
//
/// \note If the list-view control does not have the LVS_SINGLESEL style, the
/// first selected item index is retrieved.
//
/// \return the item index of the selected item, or -1 if there isn't one.
//
auto
TListViewCtrl::GetSelIndex() const -> int
{
  return GetNextSelIndex();
}

//
/// Retrieves the number of selected items.
//
/// \note Useful before calling \link GetSelIndexes \endlink and \link GetSelStrings \endlink.
//
/// \return the number of selected items, or -1 if there are no selected items.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761069.aspx
//
auto
TListViewCtrl::GetSelCount() const -> int
{
  return static_cast<int>(SendMessage(LVM_GETSELECTEDCOUNT));
}

//
/// Retrieves the indexes of the selected items.
//
/// @param[out] indexes is an array where the item indexes will be stored.
/// @param[in] maxCount is the maximum number of indexes to retrieve.
//
/// \note If the list-view control has the LVS_SINGLESEL style, at most only one index will be retrieved.
//
/// \return the number of indexes retrieved, or -1 if there are no selected items.
//
/// \sa GetSelCount
//
auto
TListViewCtrl::GetSelIndexes(int* indexes, int maxCount) const -> int
{
  // Set the index to get the first selected item.
  //
  int index = -1;

  // Number of selected items found.
  //
  int count;

  // Loop until count reached or no more selected items.
  //
  for (count = 0; count < maxCount; ++count)
  {
    // Get the next selected item.
    //
    index = GetNextSelIndex(index);
    if (index == -1) break;
    indexes[count] = index;
  }
  return count;
}

//
/// Retrieves the selected item or item subitem text.
//
/// @param[out] str is a character array where the text string will be stored.
/// @param[in] maxChars is the maximum number of characters to store including the null terminator.
/// @param[in] subitemIndex is the subitem index, which if 0 specifies to retrieve the item text.
//
/// \note If the list-view control does not have the LVS_SINGLESEL style, at most
/// only the first selected item string will be retrieved.
//
/// \return true if an item is selected.
//
auto
TListViewCtrl::GetSelString(tchar* str, int maxChars, int subitemIndex) const -> bool
{
  // Get the selected item.
  auto index = GetNextSelIndex();
  if (index == -1)
    return false;

  // Get the selected string.
  GetItemText(index, subitemIndex, str, maxChars);
  return true;
}

//
/// String overload for \link GetSelString(tchar*, int, int) const \endlink.
//
/// \return string of selected item, which will be empty if no item is selected.
//
auto
TListViewCtrl::GetSelString(int subitemIndex) const -> tstring
{
  // Get the selected item.
  auto index = GetNextSelIndex();
  if (index == -1)
    return tstring();

  // Get the selected string.
  return GetItemText(index, subitemIndex);
}

//
/// Retrieves the item or subitem strings of the selected items.
//
/// @param[out] strs is an array of text strings where the strings will be stored.
/// @param[in] maxCount is the maximum number of strings to retrieve.
/// @param[in] maxChars is the size of each string in the array including the null terminator.
/// @param[in] subitemIndex is the subitem index, which if 0 specifies to retrieve the item text.
//
/// \note If the list-view control has the LVS_SINGLESEL style, at most only one string will be retrieved.
//
/// \return the number of strings retrieved, or -1 if there are no selected items.
//
/// \sa GetSelCount
//
auto
TListViewCtrl::GetSelStrings(tchar** strs, int maxCount, int maxChars, int subitemIndex) const -> int
{
  // Set the index to get the first selected item.
  //
  int index = -1;

  // Number of selected items found.
  //
  int count;

  // Loop until count reached or no more selected items.
  //
  for (count = 0; count < maxCount; ++count)
  {
    // Get the next selected item.
    //
    index = GetNextSelIndex(index);
    if (index == -1) break;
    GetItemText(index, subitemIndex, strs[count], maxChars);
  }
  return count;
}

//
/// Selects or deselects an item.
//
/// @param[in] index is the item index.
/// @param[in] select specifies whether to select or deselect the item.
//
/// \return true if successful.
//
auto
TListViewCtrl::SetSel(int index, bool select) -> bool
{
  return SetItemState(index, select ? TLvItem::TListState::Selected : TLvItem::TListState::Unspecified, LVIS_SELECTED);
}

//
/// Selects or deselects items.
//
/// @param[in] indexes is the array of item indexes.
/// @param[in] count is the size of the item indexes array.
/// @param[in] select specifies whether to select or deselect the items.
//
/// \note If the list-view control has the LVS_SINGLESEL style, this function
/// will not succeed if multiple items are designated to be selected.
//
/// \return true if successful.
//
auto
TListViewCtrl::SetSelIndexes(int* indexes, int count, bool select) -> bool
{
  // Single-select list-views cannot select more than one.
  //
  if ((GetStyle() & LVS_SINGLESEL) && select && (count > 1))
    return false;

  for (int i = 0; i < count; i++)
    if (!SetSel(indexes[i], select))
      return false;

  return true;
}

//
/// Selects or deselects a range of items.
//
/// @param[in] select specifies whether to select or deselect the items.
/// @param[in] first is the item index of the first item to select or deselect.
/// @param[in] last is the item index of the last item to select or deselect.
//
/// \note If the list-view control has the LVS_SINGLESEL style, this function
/// will not succeed if multiple items are designated to be selected.
//
/// \return true if successful.
//
auto
TListViewCtrl::SetSelItemRange(bool select, int first, int last) -> bool
{
  // Single-select list-views cannot select more than one.
  //
  if ((GetStyle() & LVS_SINGLESEL) && select && (first < last))
    return false;

  for (int i = first; i <= last; i++)
    if (!SetSel(i, select))
      return false;

  return true;
}

//
/// Retrieves the currently selected column number.
//
/// \return the currently selected column number, or -1 if no column is selected.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761067.aspx
//
auto
TListViewCtrl::GetSelectedColumn() const -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETSELECTEDCOLUMN));
}

//
/// Sets the selected column number.
//
/// @param[in] colNum is the column number to select; -1 specifies to not select a column.
//
/// \return none.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761202.aspx
//
auto
TListViewCtrl::SetSelectedColumn(int colNum) -> void
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_SETSELECTEDCOLUMN, TParam1(colNum));
}

//
/// Creates a drag image list for an item.
//
/// @param[in] index is the item index.
/// @param[in] upLeft is the initial location of the upper-left corner of the image, in view coordinates.
//
/// \return the handle to the drag image list if successful, otherwise NULL.
//
/// \note The application is responsible for destroying the image list when it is no longer needed.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774890.aspx
//
auto
TListViewCtrl::CreateDragImage(int index, TPoint* upLeft) -> HIMAGELIST
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HIMAGELIST>(SendMessage(LVM_CREATEDRAGIMAGE, TParam1(index), TParam2(upLeft)));
}

//
/// Retrieves the handle to an image list used for drawing list-view items.
//
/// @param[in] type specifies the type of image list to retrieve.
//
/// \return the handle to the image list if successful, otherwise NULL.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774943.aspx
//
auto
TListViewCtrl::GetImageList(TImageListType type) const -> HIMAGELIST
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HIMAGELIST>(SendMessage(LVM_GETIMAGELIST, TParam1(type)));
}

//
/// Assigns an image list to a list-view control.
//
/// @param[in] list the handle to the image list.
/// @param[in] type specifies the type of image list to set.
//
/// \return the handle to the image list previously associated with the control if successful, or NULL otherwise.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761178.aspx
//
auto
TListViewCtrl::SetImageList(HIMAGELIST list, TImageListType type) -> HIMAGELIST
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HIMAGELIST>(SendMessage(LVM_SETIMAGELIST, TParam1(type), TParam2(list)));
}

//
/// Retrieves the background color of a list-view control.
//
/// \return the background color.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774905.aspx
//
auto
TListViewCtrl::GetBkColor() const -> TColor
{
  PRECONDITION(GetHandle());
  return static_cast<COLORREF>(SendMessage(LVM_GETBKCOLOR));
}

//
/// Sets the background color of a list-view control.
//
/// @param[in] c the color to use.
//
/// \note Specify TColor::None for no background color.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761153.aspx
//
auto
TListViewCtrl::SetBkColor(const TColor& c) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETBKCOLOR, 0, TParam2(c.GetValue())));
}

//
/// Retrieves the text background color of a list-view control.
//
/// \return the text background color.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761077.aspx
//
auto
TListViewCtrl::GetTextBkColor() const -> TColor
{
  PRECONDITION(GetHandle());
  return static_cast<COLORREF>(SendMessage(LVM_GETTEXTBKCOLOR));
}

//
/// Sets the background color of text in a list-view control.
//
/// @param[in] c the color to use.
//
/// \note Specify TColor::None for no text background color.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761206.aspx
//
auto
TListViewCtrl::SetTextBkColor(const TColor& c) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETTEXTBKCOLOR, 0, TParam2(c.GetValue())));
}

//
/// Retrieves the text color of a list-view control.
//
/// \return the text color.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761079.aspx
//
auto
TListViewCtrl::GetTextColor() const -> TColor
{
  PRECONDITION(GetHandle());
  return static_cast<COLORREF>(SendMessage(LVM_GETTEXTCOLOR));
}

//
/// Sets the color of text in a list-view control.
//
/// @param[in] c the color to use.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761208.aspx
//
auto
TListViewCtrl::SetTextColor(const TColor& c) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETTEXTCOLOR, 0, TParam2(c.GetValue())));
}

//
/// Retrieves the callback mask for a list-view control.
//
/// \return the callback mask.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774909.aspx
//
auto
TListViewCtrl::GetCallBackMask() const -> TLvItem::TListState
{
  PRECONDITION(GetHandle());
  return static_cast<TLvItem::TListState>(SendMessage(LVM_GETCALLBACKMASK));
}

//
/// Sets the callback mask for a list-view control.
//
/// \note The callback mask of a list-view control specifies the item states for
/// which the application, rather than the control, stores the current data. The
/// callback mask applies to all of the control's items, unlike the callback item
/// designation, which applies to a specific item. The callback mask is Unspecified
/// by default, meaning that the list-view control stores all item state information.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761157.aspx
//
auto
TListViewCtrl::SetCallBackMask(TLvItem::TListState mask) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETCALLBACKMASK, mask));
}

//
/// Retrieves the topmost visible item index when in list or report view.
//
/// \return the topmost visible item index if successful; 0 if the list-view control
/// is in icon or small icon view, or if the list-view control is in details view
/// with groups enabled.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761087.aspx
//
auto
TListViewCtrl::GetTopIndex() const-> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETTOPINDEX));
}

//
/// Begins in-place editing of the list-view item's text.
//
/// @param[in] index is the item index, or -1 to cancel editing.
//
/// \return the handle to the edit control that is used to edit the item text
/// if successful, otherwise NULL.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774898.aspx
//
auto
TListViewCtrl::EditLabel(int index)-> HWND
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HWND>(SendMessage(LVM_EDITLABEL, TParam1(index)));
}

//
/// Retrieves the handle to the edit control used to edit a list-view item's text.
//
/// \return the handle to the edit control if successful, otherwise NULL.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774919.aspx
//
auto
TListViewCtrl::GetEditControl() const-> HWND
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HWND>(SendMessage(LVM_GETEDITCONTROL));
}

//
/// Cancels an item text editing operation.
//
/// \return none.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774886.aspx
//
auto
TListViewCtrl::CancelEditLabel() -> void
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_CANCELEDITLABEL);
}

//
/// Updates a list-view item.
//
/// @param[in] index is the item index to update.
//
/// \note If the list window has LVS_AUTOARRANGE, the items are automatically
/// arranged to their proper locations.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761230.aspx
//
auto
TListViewCtrl::Update(int index) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_UPDATE, TParam1(index)));
}

//
/// Determines whether a list-view item is at a specified position.
//
/// @param[in,out] info is a class containing the position to check, and upon
/// return contains results of the test.
//
/// \return item index found at the specified position if any, otherwise -1.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761099.aspx
//
auto
TListViewCtrl::HitTest(TLvHitTestInfo& info) const -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_HITTEST, -1, TParam2(&info)));
}

//
/// Functional-style overload
//
auto TListViewCtrl::HitTest(const TPoint& p) const -> TLvHitTestInfo
{
  auto r = TLvHitTestInfo{p};
  HitTest(r);
  return r;
}

//
/// Scrolls the content of a list-view control.
//
/// @param[in] dx is the pixel amount of horizontal scrolling relative to the current position.
/// @param[in] dy is the pixel amount of vertical scrolling relative to the current position.
//
/// \note When the list-view control is in report view, the control can only be scrolled vertically
/// in whole line increments so the vertical scrolling amount will be rounded to the nearest number
/// of pixels that form a whole line increment.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761151.aspx
//
auto
TListViewCtrl::Scroll(int dx, int dy) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SCROLL, dx, dy));
}

//
/// Retrieves the current view origin for a list-view control.
//
/// @param[out] pt upon return will contain the view origin.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761063.aspx
//
auto
TListViewCtrl::GetOrigin(TPoint& pt) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETORIGIN, 0, TParam2(&pt)));
}

//
/// Calculates the exact width in pixels of the given string using the control's
/// current font.
//
/// @param[in] text is the string to analyze.
//
/// \note Use \link CalculateColumnWidth \endlink instead of this function to set
/// the column width using \link InsertColumn \endlink or \link SetColumnWidth \endlink
/// to avoid truncation.
//
/// \return pixel width of the string.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761073.aspx
//
auto
TListViewCtrl::GetStringWidth(LPCTSTR text) const -> int
{
  PRECONDITION(GetHandle());
  PRECONDITION(text);
  return static_cast<int>(SendMessage(LVM_GETSTRINGWIDTH, 0, TParam2(text)));
}

//
/// String overload for \link GetStringWidth(LPCTSTR) const \endlink.
//
/// @param[in] text is the string to analyze.
//
/// \return pixel width of the string.
//
auto
TListViewCtrl::GetStringWidth(const tstring& text) const -> int
{
  PRECONDITION(GetHandle());
  return GetStringWidth(text.c_str());
}

//
/// Calculates a suitable column pixel width for the given string.
//
/// This value can be used to set a column width using \link InsertColumn \endlink
/// or \link SetColumnWidth \endlink.
//
/// @param[in] text is the string to analyze.
/// @param[in] padding is the number of pixels to add for a suitable column width.
//
/// \return column pixel width.
//
auto
TListViewCtrl::CalculateColumnWidth(LPCTSTR text, int padding) const -> int
{
  PRECONDITION(GetHandle());

  // TODO: Ideally, we should calculate the correct column width based on the control's rendering
  // here, i.e. take account of the scale of its GUI elements. However, this information is not
  // readily available, so for now, we simply add the given padding to the width of the string.
  //
  return GetStringWidth(text) + padding;
}

//
/// String overload for \link CalculateColumnWidth(LPCTSTR, int) const \endlink.
//
/// @param[in] text is the string to analyze.
/// @param[in] padding is the number of pixels to add for a suitable column width.
//
/// \return column pixel width.
//
auto
TListViewCtrl::CalculateColumnWidth(const tstring& text, int padding) const -> int
{
  PRECONDITION(GetHandle());
  return CalculateColumnWidth(text.c_str(), padding);
}

//
/// Retrieves the bounding rectangle of all items in the list-view control when in icon or small icon view.
//
/// @param[out] rect upon return will contain the bounding rectangle.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761093.aspx
//
auto
TListViewCtrl::GetViewRect(TRect& rect) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETVIEWRECT, 0, TParam2(&rect)));
}

//
/// Queries whether an item is visible.
//
/// @param[in] index is the item index.
//
/// \return true if visible.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761135.aspx
//
auto
TListViewCtrl::IsItemVisible(int index) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_ISITEMVISIBLE, TParam1(index)));
}

//
/// Ensures that a list-view item is either partially or entirely visible, scrolling
/// the list-view control if necessary.
//
/// @param[in] index is the item index.
/// @param[in] partialOk specifies whether to ensure partially or entirely visible.
//
/// \note The function will not succeed if the window style has LVS_NOSCROLL.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774902.aspx
//
auto
TListViewCtrl::EnsureVisible(int index, bool partialOk) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_ENSUREVISIBLE, TParam1(index), TParam2(partialOk)));
}

//
/// Forces a list-view control to redraw a range of items.
//
/// @param[in] startIndex is the first item index to redraw.
/// @param[in] endIndex is the last item index to redraw.
//
/// \note The items are not actually redrawn until the list-view window receives a
/// WM_PAINT message to repaint. To repaint immediately, call the UpdateWindow()
/// function after calling this function.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761145.aspx
//
auto
TListViewCtrl::RedrawItems(int startIndex, int endIndex) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_REDRAWITEMS, TParam1(startIndex), TParam2(endIndex)));
}

//
/// Calculates the number of items that can fit vertically in the visible area
/// of a list-view control when in list or report view.
//
/// \return the number of fully visible items if successful; if in icon or small
/// icon view, the total number of items.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774917.aspx
//
auto
TListViewCtrl::GetCountPerPage() const -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETCOUNTPERPAGE));
}

//
/// Retrieves the incremental search string.
//
/// \return the incremental search string, which will be empty if the list-view
/// control is not in incremental search mode.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774951.aspx
//
auto
TListViewCtrl::GetISearchString() const -> tstring
{
  PRECONDITION(GetHandle());
  tstring s;

  // Call to get size of string.
  //
  auto size = static_cast<int>(SendMessage(LVM_GETISEARCHSTRING));
  if (!size) return s;

  // Resize so string is large enough, including null terminator.
  //
  s.resize(size + 1);

  // Call to get string.
  //
  SendMessage(LVM_GETISEARCHSTRING, 0, TParam2(&s[0]));

  // Return the string result.
  //
  return s;
}

//
/// Retrieves the item index of the item that has the focus.
//
/// \return the item index, or -1 if no item has the focus.
//
auto
TListViewCtrl::GetFocusItem() -> int
{
  auto n = GetItemCount();
  for (auto i = 0; i < n; ++i)
    if (GetItemState(i, TLvItem::Focus) == TLvItem::Focus) return i;
  return -1;
}

//
/// Sets or removes the focus for an item.
//
/// @param[in] index is the item index.
/// @param[in] focused specifies whether the focus is set or removed.
//
/// \return true if successful.
//
auto
TListViewCtrl::SetFocusItem(int index, bool focused) -> bool
{
  return SetItemState(index, focused ? TLvItem::Focus : TLvItem::Unspecified, TLvItem::Focus);
}

//
/// Calculates the approximate width and height required to display a given number of items.
//
/// @param[in] x is the proposed x-dimension of the list-view control; -1 specifies to use the current width.
/// @param[in] y is the proposed y-dimension of the list-view control; -1 specifies to use the current height.
/// @param[in] count is the number of items to be displayed in the list-view control; -1 specifies to use the
/// total number of items.
//
/// \note
/// -# Setting the size of the list-view control based on the dimensions provided by this message can optimize
/// redraw and reduce flicker.
/// -# If the coordinates are outside a valid range, a TXOwl exception is thrown.
//
/// \return the calculated size.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774883.aspx
//
auto
TListViewCtrl::GetApproxRect(int x, int y, int count) const -> TSize
{
  PRECONDITION(GetHandle());
  if ((x != -1 && !IsRepresentable<uint16>(x)) || (y != -1 && !IsRepresentable<uint16>(y)))
    throw TXOwl(_T("TListViewCtrl:GetApproxRect: Argument is outside valid range"));
  TResult r = SendMessage(LVM_APPROXIMATEVIEWRECT, TParam1(count), MkParam2(x, y));
  return TPoint(static_cast<LPARAM>(r));
}

//
/// Retrieves the current left-to-right order of columns in a list-view control.
//
/// @param[in] count is the number of columns to be retrieved.
/// @param[out] array is an array where the columns will be stored.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774913.aspx
//
auto
TListViewCtrl::GetColumnOrder(int count, int* array) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETCOLUMNORDERARRAY, TParam1(count), TParam2(array)));
}

//
/// Sets the current left-to-right order of columns in a list-view control.
//
/// @param[in] count is the number of columns to be set.
/// @param[in] array is an array of columns to set.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761161.aspx
//
auto
TListViewCtrl::SetColumnOrder(int count, const int* array) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETCOLUMNORDERARRAY, TParam1(count), TParam2(array)));
}

//
/// Retrieves the extended styles that are currently in use for a given list-view control.
//
/// \return the extended styles.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774923.aspx
//
auto
TListViewCtrl::GetExtStyle() const -> uint32
{
  PRECONDITION(GetHandle());
  return static_cast<uint32>(SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE));
}

//
/// Sets the extended styles to use for a given list-view control.
//
/// @param[in] mask specifies the styles that are to be affected in style; 0 means all the styles given.
/// @param[in] style specifies the styles.
//
/// \return the previously used extended styles.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761165.aspx
//
auto
TListViewCtrl::SetExtStyle(uint32 mask, uint32 style) -> uint32
{
  PRECONDITION(GetHandle());
  return static_cast<uint32>(SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, TParam1(mask), TParam2(style)));
}

//
/// Retrieves the handle to the header control used by the list-view control.
//
/// \return the handle to the header control.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774937.aspx
//
auto
TListViewCtrl::GetHeaderCtrl() const -> HWND
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HWND>(SendMessage(LVM_GETHEADER));
}

//
/// Retrieves the cursor handle used when the pointer is over an item while hot tracking is enabled.
//
/// \note A list-view control uses hot tracking and hover selection when the LVS_EX_TRACKSELECT style is set.
//
/// \return the handle to the cursor.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774938.aspx
//
auto
TListViewCtrl::GetHotCursor() const -> HCURSOR
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HCURSOR>(SendMessage(LVM_GETHOTCURSOR));
}

//
/// Sets the cursor handle to use when the pointer is over an item while hot tracking is enabled.
//
/// @param[in] cur the new cursor handle to use.
//
/// \note A list-view control uses hot tracking and hover selection when the LVS_EX_TRACKSELECT style is set.
//
/// \return the previous cursor handle used.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774938.aspx
//
auto
TListViewCtrl::SetHotCursor(HCURSOR cur) -> HCURSOR
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HCURSOR>(SendMessage(LVM_SETHOTCURSOR, 0, TParam2(cur)));
}

//
/// Retrieves the index of the hot item.
//
/// \return the hot item index.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774939.aspx
//
auto
TListViewCtrl::GetHotItem() const -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETHOTITEM));
}

//
/// Sets the index of the hot item.
//
/// \return the previous hot item index.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761172.aspx
//
auto
TListViewCtrl::SetHotItem(int index) -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_SETHOTITEM, TParam1(index)));
}

//
/// Retrieves the bounding rectangle for a subitem in a list-view control when in report view.
//
/// @param[out] rect upon return contains the bounding rectangle.
/// @param[in] subitemIndex the subitem index.
/// @param[in] index the item index.
/// @param[in] type the type of bountding rectangle to retrieve.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761075.aspx
//
auto
TListViewCtrl::GetSubItemRect(TRect& rect, int subitemIndex, int index, TItemRectType type) const -> bool
{
  PRECONDITION(GetHandle());

  // SelectBounds isn't allowed but is synonymous with Bounds in this context.
  //
  rect = { static_cast<int>((type == SelectBounds) ? Bounds : type), subitemIndex, 0, 0 };
  return ToBool(SendMessage(LVM_GETSUBITEMRECT, TParam1(index), TParam2(&rect)));
}

//
/// Determines which list-view item or subitem is at a given position.
//
/// @param[in,out] info is a class containing the position to check, and upon
/// return contains results of the test.
//
/// \return item or subitem index found at the specified position if any, otherwise -1.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761229.aspx
//
auto
TListViewCtrl::SubItemHitTest(TLvHitTestInfo& info) const -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_SUBITEMHITTEST, -1, TParam2(&info)));
}

//
/// Functional-style overload
//
auto TListViewCtrl::SubItemHitTest(const TPoint& p) const -> TLvHitTestInfo
{
  auto r = TLvHitTestInfo{p};
  SubItemHitTest(r);
  return r;
}

//
/// Retrieves the amount of horizontal and vertical spacing between items.
//
/// @param[in] smallIcon if true, the spacing refers to small icon view rather
/// than normal icon view.
//
/// \return the item spacing.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761051.aspx
//
auto
TListViewCtrl::GetItemSpacing(bool smallIcon) const -> TSize
{
  PRECONDITION(GetHandle());
  TResult r = SendMessage(LVM_GETITEMSPACING, TParam1(smallIcon));
  return TSize(LoUint16(r), HiUint16(r));
}

//
/// Sets the spacing between icons in list-view controls that have the LVS_ICON style.
//
/// @param[in] x is the horizontal spacing to use.
/// @param[in] y is the vertical spacing to use.
//
/// \note
/// -# The spacing values must include the icon size if overlapping is to be avoided.
/// -# If the spacing values are outside a valid range, a TXOwl exception is thrown.
//
/// \return the previous icon spacing used.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761176.aspx
//
auto
TListViewCtrl::SetIconSpacing(int x, int y) -> TSize
{
  PRECONDITION(GetHandle());
  if ((y != -1) && ((x < 0) || (y < 0)))
    throw TXOwl(_T("TListViewCtrl:SetIconSpacing: Argument is outside valid range"));
  TResult r = SendMessage(LVM_SETICONSPACING, TParam1(x), y);
  return TSize(LoUint16(r), HiUint16(r));
}

//
/// Retrieves the background image in a list-view control.
//
/// @param[out] bkimg is the class that will contain the background image information.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774907.aspx
//
auto
TListViewCtrl::GetBkImage(TLvBkImage& bkimg) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETBKIMAGE, 0, TParam2(&bkimg)));
}

//
/// Sets the background image in a list-view control.
//
/// @param[in] bkimg is the class containing the background image information.
//
/// \return true if successful and a background image is used.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761155.aspx
//
auto
TListViewCtrl::SetBkImage(const TLvBkImage& bkimg) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETBKIMAGE, 0, TParam2(&bkimg)));
}

//
/// Retrieves the amount of time that the mouse cursor must hover over an item
/// before it is selected.
//
/// \note The hover time only affects list-view controls that have the
/// LVS_EX_TRACKSELECT, LVS_EX_ONECLICKACTIVATE, or LVS_EX_TWOCLICKACTIVATE
/// extended list-view style.
//
/// \return the amount of time in milliseconds, which is the maximum value for
/// this data type if the default time is used.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774941.aspx
//
auto
TListViewCtrl::GetHoverTime() const -> uint32
{
  PRECONDITION(GetHandle());
  return static_cast<uint32>(SendMessage(LVM_GETHOVERTIME));
}

//
/// Sets the amount of time that the mouse cursor must hover over an item
/// before it is selected.
//
/// @param[in] tm is the amount of time in milliseconds; if this is the maximum
/// value for this data type the default time is used.
//
/// \return the previous hover time used.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761174.aspx
//
auto
TListViewCtrl::SetHoverTime(uint32 tm) -> uint32
{
  PRECONDITION(GetHandle());
  return static_cast<uint32>(SendMessage(LVM_SETHOVERTIME, 0, TParam2(tm)));
}

//
/// Retrieves the number of working areas in a list-view control.
//
/// \return the number of working areas.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761061.aspx
//
auto
TListViewCtrl::GetNumOfWorkAreas() const -> uint
{
  PRECONDITION(GetHandle());
  uint retval;
  SendMessage(LVM_GETNUMBEROFWORKAREAS, 0, TParam2(&retval));
  return retval;
}

//
/// Retrieves the working areas from a list-view control.
//
/// @param[in] count is the maximum number of working areas to retrieve.
/// @param[out] areas upon return will contain the client coordinates of the working areas.
//
/// \return none.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761095.aspx
//
auto
TListViewCtrl::GetWorkAreas(int count, TRect* areas) const -> void
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_GETWORKAREAS, TParam1(count), TParam2(areas));
}

//
/// Sets the working areas for a list-view control.
//
/// @param[in] count is the number of working areas.
/// @param[in] areas is the list of the client coordinates of the working areas;
/// if a null pointer is passed, the working area will be set to the client area
/// of the control.
//
/// \return none.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761223.aspx
//
auto
TListViewCtrl::SetWorkAreas(int count, TRect* areas) -> void
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_SETWORKAREAS, TParam1(count), TParam2(areas));
}

//
/// Retrieves the selection mark from a list-view control.
//
/// The selection mark is the item index from which a multiple selection starts.
//
/// \return the zero-based selection mark, or -1 if there is no selection mark.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761071.aspx
//
auto
TListViewCtrl::GetSelectionMark() const -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_GETSELECTIONMARK));
}

//
/// Sets the selection mark for a list-view control.
//
/// The selection mark is the item index from which a multiple selection starts.
//
/// @param[in] index is the zero-based index of the new selection mark; if -1,
/// the selection mark is removed.
//
/// \return the previous selection mark, or -1 if there was no previous selection mark.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761204.aspx
//
auto
TListViewCtrl::SetSelectionMark(int index) -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_SETSELECTIONMARK, 0, TParam2(index)));
}

//
/// Sets the bounding rectangle for a subitem.
//
/// @param[in] index is the item index.
/// @param[in] group is the group number.
/// @param[in] subitemIndex is the subitem index.
/// @param[in] rect is the bounding rectangle to set.
/// @param[in] type specifies the type of rectangle to set.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761046.aspx
//
auto
TListViewCtrl::GetItemIndexRect(int index, int group, int subitemIndex, TRect& rect, TItemRectType type) const -> bool
{
  PRECONDITION(GetHandle());

  // SelectBounds isn't allowed but is synonymous with Bounds in this context.
  //
  rect = { static_cast<int>((type == SelectBounds) ? Bounds : type), subitemIndex, 0, 0 };
  LVITEMINDEX itemIndex = { index, group };
  return ToBool(SendMessage(LVM_GETITEMINDEXRECT, TParam1(&itemIndex), TParam2(&rect)));
}

//
/// Searches for a list-view item that has the specified properties and bears
/// the specified relationship to a specified item.
//
/// @param[in] index is the item index to begin the search; -1 specifies to find the first item.
/// @param[in] group is the group number.
/// @param[in] flags is a combination of TNextItemCode flags that specify the relationship to the item given by index.
//
/// \return item index if found, otherwise -1.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761059.aspx, TListViewCtrl::TNextItemCode
//
auto
TListViewCtrl::GetNextItemIndex(int index, int group, uint flags) const -> int
{
  PRECONDITION(GetHandle());
  LVITEMINDEX itemIndex = { index, group };
  return SendMessage(LVM_GETNEXTITEMINDEX, TParam1(&itemIndex), TParam2(flags)) != FALSE ? itemIndex.iItem : -1;
}

//
/// Sets the state of a list-view item.
//
/// @param[in] item is the class containing the state attributes to set.
/// @param[in] index is the item index.
/// @param[in] group is the group number.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761190.aspx
//
auto
TListViewCtrl::SetItemIndexState(const TLvItem& item, int index, int group) -> bool
{
  PRECONDITION(GetHandle());
  LVITEMINDEX itemIndex = { index, group };
  return SendMessage(LVM_SETITEMINDEXSTATE, TParam1(&itemIndex), TParam2(&item)) == S_OK;
}

//
/// Retrieves the string to display when the list-view control appears empty.
//
/// \return the string to display.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774921.aspx
//
auto
TListViewCtrl::GetEmptyText() const -> tstring
{
  PRECONDITION(GetHandle());
  tstring s;

  // Resize arbitrarily so hopefully string is large enough, as Windows does
  // not provide a method for getting the needed size.
  //
  s.resize(1024);

  // Call to get string.
  //
  SendMessage(LVM_GETEMPTYTEXT, TParam1(s.capacity()), TParam2(&s[0]));

  // Return the string result.
  //
  return s;
}

//
/// Retrieves information about the footer of a list-view control.
//
/// @param[in,out] info contains the information to retrieve on the footer and will contain those values upon return.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774927.aspx
//
auto
TListViewCtrl::GetFooterInfo(TLvFooterInfo& info) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETFOOTERINFO, 0, TParam2(&info)));
}

//
/// Footer item count return overload for \link GetFooterInfo(TLvFooterInfo&) const \endlink.
//
auto
TListViewCtrl::GetFooterItems() const -> int
{
  TLvFooterInfo info;
  info.mask = LVFF_ITEMCOUNT;
  auto r = GetFooterInfo(info);
  WARN(!r, _T("TListViewCtrl::GetFooterInfo failed")); InUse(r);
  return static_cast<int>(info.cItems);
}

//
/// Retrieves information for a footer item in a list-view control.
//
/// @param[in] index is the item index.
/// @param[in,out] item contains the information to retrieve on a footer item and will contain those values upon return.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774928.aspx
//
auto
TListViewCtrl::GetFooterItem(int index, TLvFooterItem& item) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETFOOTERITEM, TParam1(index), TParam2(&item)));
}

//
/// Footer item text string return overload for \link GetFooterItem(int, TLvFooterItem&) const \endlink.
/// Returns an empty string on failure.
//
auto
TListViewCtrl::GetFooterItemText(int index) const -> std::wstring
{
  // Allocate a string of arbitrary size ("large enough"), as Windows does
  // not provide a method for getting the needed size.
  //
  std::wstring s(1024, L'\0');

  // Get the footer string, resize it and return it.
  //
  TLvFooterItem item;
  item.iItem = index;
  item.mask = LVFIF_TEXT;
  item.pszText = &s[0];
  item.cchTextMax = static_cast<int>(s.size());
  auto r = GetFooterItem(index, item);
  WARN(!r, _T("TListViewCtrl::GetFooterItem failed for index ") << index); InUse(r);
  s.resize(wcslen(s.data()));
  s.shrink_to_fit();
  return s;
}

//
/// Footer item state return overload for \link GetFooterItem(int, TLvFooterItem&) const \endlink.
//
/// @param[in] mask contains the mask of state values to return.
//
auto
TListViewCtrl::GetFooterItemState(int index, TLvFooterItem::TState mask) const -> TLvFooterItem::TState
{
  TLvFooterItem item;

  // Set Footer Item Attributes
  item.iItem = index;
  item.mask = LVFIF_STATE;
  item.stateMask = static_cast<UINT>(mask);

  // Get state.
  //
  auto r = GetFooterItem(index, item);
  WARN(!r, _T("TListViewCtrl::GetFooterItem failed for index ") << index); InUse(r);

  // Return state
  return static_cast<TLvFooterItem::TState>(item.state);
}

//
/// Retrieves the coordinates of a footer for an item in a list-view control.
//
/// @param[in] index is the item index.
/// @param[out] rect contains the coordinates of the footer upon return.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774929.aspx
//
auto
TListViewCtrl::GetFooterItemRect(int index, TRect& rect) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETFOOTERITEMRECT, TParam1(index), TParam2(&rect)));
}

//
/// TRect return overload for \link GetFooterItemRect(int, TRect&) const \endlink.
//
auto
TListViewCtrl::GetFooterItemRect(int index) const -> TRect
{
  TRect rect;
  auto r = GetFooterItemRect(index, rect);
  WARN(!r, _T("TListViewCtrl::GetFooterItemRect failed for index ") << index); InUse(r);
  return rect;
}

//
/// Retrieves the coordinates of the footer for a list-view control.
//
/// @param[out] rect contains the coordinates of the footer upon return.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774930.aspx
//
auto
TListViewCtrl::GetFooterRect(TRect& rect) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETFOOTERRECT, 0, TParam2(&rect)));
}

//
/// TRect return overload for \link GetFooterRect(TRect&) const \endlink.
//
auto
TListViewCtrl::GetFooterRect() const -> TRect
{
  TRect rect;
  auto r = GetFooterRect(rect);
  WARN(!r, _T("TListViewCtrl::GetFooterRect failed")); InUse(r);
  return rect;
}

//
/// Gets group information.
//
/// @param[in] mask determines which fields to retrieve (optional).
//
/// \sa http://docs.microsoft.com/en-us/windows/desktop/Controls/lvm-getgroupinfo
//
auto owl::TListViewCtrl::GetGroupInfo(int groupId, uint mask) -> LVGROUP
{
  auto g = LVGROUP{sizeof(LVGROUP), mask};
  const auto r = static_cast<int>(SendMessage(LVM_GETGROUPINFO, groupId, reinterpret_cast<TParam2>(&g)));
  WARN(r < 0, _T("TListViewCtrl::GetGroupInfo failed")); InUse(r);
  return g;
}

//
/// Queries whether the Unicode character set is being used instead of ANSI.
//
/// \return true if Unicode is being used.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761089.aspx
//
auto
TListViewCtrl::GetUnicodeFormat() const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETUNICODEFORMAT));
}

//
/// Sets whether the Unicode character set is being used instead of ANSI.
//
/// @param[in] useUnicode if true, Unicode will be used.
//
/// \return the previous setting.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761218.aspx
//
auto
TListViewCtrl::SetUnicodeFormat(bool useUnicode) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETUNICODEFORMAT, TParam1(useUnicode)));
}

//
/// Associates a unique identifier with an item.
//
/// List-view controls internally track items by index which can present
/// problems because indexes can change during the control's lifetime.
/// Using a unique identifier allows an item to be easily referenced
/// independent of index changes.
//
/// @param[in] index is the item index.
//
/// \note In a multithreaded environment, the index is only guaranteed on
/// the thread that hosts the list-view control, not on background threads.
/// Not supported under the LVS_OWNERDATA style.
//
/// \return a unique identifier for the item.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761139.aspx
//
auto
TListViewCtrl::MapIndexToId(int index) const -> TItemId
{
  PRECONDITION(GetHandle());
  return SendMessage(LVM_MAPINDEXTOID, TParam1(index));
}

//
/// Retrieves the current index for the item with the given unique identifier.
//
/// @param[in] id is the unique identifier for the item.
//
/// \note In a multithreaded environment, the index is only guaranteed on
/// the thread that hosts the list-view control, not on background threads.
/// Not supported under the LVS_OWNERDATA style.
//
/// \return the item index.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761137.aspx
//
auto
TListViewCtrl::MapIdToIndex(TListViewCtrl::TItemId id) const -> int
{
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LVM_MAPIDTOINDEX, TParam1(id)));
}

//
/// Retrieves the handle of the tooltip control that the list-view control uses
/// to display tooltips.
//
/// \return the handle of the tooltip control.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761085.aspx
//
auto
TListViewCtrl::GetToolTips() const -> HWND
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HWND>(SendMessage(LVM_GETTOOLTIPS));
}

//
/// Sets the handle of the tooltip control that the list-view control uses
/// to display tooltips.
//
/// @param[in] handle is the handle of the tooltip control to use.
//
/// \return the handle of the previous tooltip control that was used.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761216.aspx
//
auto
TListViewCtrl::SetToolTips(THandle handle) -> HWND
{
  PRECONDITION(GetHandle());
  PRECONDITION(handle);
  return reinterpret_cast<HWND>(SendMessage(LVM_SETTOOLTIPS, TParam1(handle)));
}

//
/// Sets tooltip text in delayed response to the LVN_GETINFOTIP notification.
//
/// This function lets an application calculate infotips in the background by
/// performing the following steps:
/// -# In response to the LVN_GETINFOTIP notification, set the `pszText` member
/// of the `TLvGetInfoTip` structure to an empty string and return 0.
/// -# In the background, compute the infotip.
/// -# After computing the infotip, call this function with the infotip and item
/// and sub-item indexes to which the infotip applies.
//
/// \note The intotip appears only if the specified item and sub-item are still
/// in a state that requires an infotip.
//
/// @param[in] text is the wide-char tooltip text to set; a null pointer results
/// in an empty string used.
/// @param[in] index is the item index.
/// @param[in] subitemIndex is the subitem index.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761180.aspx
//
auto
TListViewCtrl::SetInfoTip(LPCWSTR text, int index, int subitemIndex) -> bool
{
  PRECONDITION(GetHandle());
  LVSETINFOTIP lvSetInfoTip = { sizeof(LVSETINFOTIP), 0, const_cast<LPWSTR>(text ? text : L""), index, subitemIndex };
  return ToBool(SendMessage(LVM_SETINFOTIP, 0, TParam2(&lvSetInfoTip)));
}

//
/// C single-byte string overload for \link SetInfoTip(LPCWSTR, int, int) \endlink.
//
auto
TListViewCtrl::SetInfoTip(LPCSTR text, int index, int subitemIndex) -> bool
{
  PRECONDITION(GetHandle());

  // Define wide-char string to use.
  //
  LPWSTR wtext = nullptr;

  // Convert to wide-char if single-byte string is not null.
  //
  if (text && *text)
  {
    // Compute buffer size needed.
    //
    auto wsize = ::MultiByteToWideChar(CP_ACP, 0, text, -1, nullptr, 0);
    if (!wsize)
    {
      WARN(true, _T("TListViewCtrl::SetInfoTip ::MultiByteToWideChar returned zero size for text string"));
      return false;
    }

    // Allocate buffer.
    //
    wtext = new wchar_t [wsize];
    if (!wtext)
    {
      WARN(true, _T("TListViewCtrl::SetInfoTip Memory for wide char buffer failed"));
      return false;
    }

    // Convert to wide-char string.
    //
    if (!::MultiByteToWideChar(CP_ACP, 0, text, -1, wtext, wsize))
    {
      WARN(true, _T("TListViewCtrl::SetInfoTip ::MultiByteToWideChar conversion failed for text string"));
      delete [] wtext;
      return false;
    }
  }

  // Set the infotip.
  //
  auto ret = SetInfoTip(wtext, index, subitemIndex);

  // Release wide-char string if created.
  //
  if (wtext) delete [] wtext;

  // Return result.
  //
  return ret;
}

//
/// String overload for \link SetInfoTip(LPCWSTR, int, int) \endlink.
//
auto
TListViewCtrl::SetInfoTip(const tstring& text, int index, int subitemIndex) -> bool
{
  PRECONDITION(GetHandle());
  return SetInfoTip(text.c_str(), index, subitemIndex);
}

//
/// Retrieves the view type.
//
/// \return the view type.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761091.aspx
//
auto
TListViewCtrl::GetView() const -> TViewType
{
  PRECONDITION(GetHandle());
  return static_cast<TViewType>(SendMessage(LVM_GETVIEW));
}

//
/// Sets the view type.
//
/// @param[in] viewType is the view type ot set.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761220.aspx
//
auto
TListViewCtrl::SetView(TListViewCtrl::TViewType viewType) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETVIEW, TParam1(viewType)));
}

//
/// Retrieves information about a tile in a list-view control.
//
/// @param[in,out] lvTileInfo is the class which specifies the tile item
/// information to retrieve, and upon return will contain the tile information.
//
/// \return none.
//
/// \note This function is not supported under the LVS_OWNERDATA style.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761081.aspx
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774766.aspx
//
auto
TListViewCtrl::GetTileInfo(PLVTILEINFO lvTileInfo) const -> void
{
  PRECONDITION(GetHandle());
  PRECONDITION(lvTileInfo);
  SendMessage(LVM_GETTILEINFO, 0, TParam2(lvTileInfo));
}

//
/// Reference overload for \link GetTileInfo(PLVTILEINFO) const \endlink.
//
auto
TListViewCtrl::GetTileInfo(LVTILEINFO& lvTileInfo) const -> void
{
  GetTileInfo(&lvTileInfo);
}

//
/// Sets information about a tile in a list-view control.
//
/// @param[in] lvTileInfo is the class containing the tile information to set.
//
/// \note This function is not supported under the LVS_OWNERDATA style.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761210.aspx
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774766.aspx
//
auto
TListViewCtrl::SetTileInfo(PLVTILEINFO lvTileInfo) -> bool
{
  PRECONDITION(GetHandle());
  PRECONDITION(lvTileInfo);
  return ToBool(SendMessage(LVM_SETTILEINFO, 0, TParam2(lvTileInfo)));
}

//
/// Reference overload for \link SetTileInfo(PLVTILEINFO) \endlink.
//
auto
TListViewCtrl::SetTileInfo(LVTILEINFO& lvTileInfo) -> bool
{
  return SetTileInfo(&lvTileInfo);
}

//
/// Retrieves tile view information.
//
/// @param[in,out] tileViewInfo is the class which specifies the tile view item
/// information to retrieve, and upon return will contain the tile view information.
//
/// \return none.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761083.aspx
//
auto
TListViewCtrl::GetTileViewInfo(TLvTileViewInfo& tileViewInfo) const -> void
{
  PRECONDITION(GetHandle());
  SendMessage(LVM_GETTILEVIEWINFO, 0, TParam2(&tileViewInfo));
}

//
/// Sets tile view information.
//
/// @param[in] tileViewInfo is the class containing the tile view information to set.
//
/// \return true if successful.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761212.aspx
//
auto
TListViewCtrl::SetTileViewInfo(const TLvTileViewInfo& tileViewInfo) -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_SETTILEVIEWINFO, 0, TParam2(&tileViewInfo)));
}

//
/// Retrieves the list-view control border color.
//
/// \note Only valid if the LVS_EX_BORDERSELECT extended window style is set.
//
/// \return the control border color.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761065.aspx
//
auto
TListViewCtrl::GetOutlineColor() const -> TColor
{
  PRECONDITION(GetHandle());
  return TColor(static_cast<COLORREF>(SendMessage(LVM_GETOUTLINECOLOR)));
}

//
/// Sets the list-view control border color.
//
/// @param[in] color is the color to set.
//
/// \note Only valid if the LVS_EX_BORDERSELECT extended window style is set.
//
/// \return the control border color.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb761200.aspx
//
auto
TListViewCtrl::SetOutlineColor(const TColor& color) -> TColor
{
  PRECONDITION(GetHandle());
  return TColor(static_cast<COLORREF>(SendMessage(LVM_SETOUTLINECOLOR, 0, TParam2(COLORREF(color)))));
}

// OWL version 5 compatibility; pointers to references
//
#if defined(OWL5_COMPAT)

auto
TListViewCtrl::GetOrigin(POINT* p) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETORIGIN, 0, TParam2(p)));
}

auto
TListViewCtrl::GetViewRect(RECT* r) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETVIEWRECT, 0, TParam2(r)));
}

auto
TListViewCtrl::GetColumn(int index, LVCOLUMN* column) const -> bool
{
  PRECONDITION(column);
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETCOLUMN, TParam1(index), TParam2(column)));
}

auto
TListViewCtrl::GetItemPosition(int index, POINT* p) const -> bool
{
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(LVM_GETITEMPOSITION, TParam1(index), TParam2(p)));
}

auto
TListViewCtrl::GetItemRect(int index, RECT* r, TItemRectType type) const -> bool
{
  PRECONDITION(GetHandle());
  r->left = type;
  return ToBool(SendMessage(LVM_GETITEMRECT, TParam1(index), TParam2(r)));
}

auto
TListViewCtrl::FindItem(int index, const TLvFindInfo* findInfo) const -> int
{
  PRECONDITION(findInfo);
  return FindItem(index, *findInfo);
}

#endif

auto TListViewCtrl::GetWindowClassName() -> TWindowClassName
{
  return TWindowClassName{WC_LISTVIEW};
}

} // OWL namespace

