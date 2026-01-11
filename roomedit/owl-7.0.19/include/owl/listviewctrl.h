//
/// \file
/// Definition of TListViewCtrl class
//
// Part of OWLNext - the next generation Object Windows Library
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
// Copyright (c) 2011 Vidar Hasfjord
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//

#if !defined(OWL_LISTVIEWCTRL_H)
#define OWL_LISTVIEWCTRL_H

#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/private/defs.h>
#include <owl/defs.h>
#include <owl/commctrl.h>
#include <vector>

namespace owl {

#include <owl/preclass.h>

/// \addtogroup commctrl
/// @{

class _OWLCLASS TListViewCtrl;

//
/// Encapsulates structure LVFINDINFO, used to find an item in a \link TListViewCtrl \endlink.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774745.aspx
//
class _OWLCLASS TLvFindInfo
  : public LVFINDINFO
{
public:

  //
  /// \name Constructors
  /// @{

  TLvFindInfo();
  TLvFindInfo(const LVFINDINFO& info);

  /// @}
  /// \name Manipulators
  /// @{

  auto SetString(const tstring& text) -> void;
  auto SetSubstring(const tstring& text) -> void;
  auto SetPartial(const tstring& text) -> void;
  auto SetWrap(bool wrap = true) -> void;
  auto SetData(LPARAM param) -> void;

  /// @}

protected:
  //
  /// String used for searching.
  //
  tstring Text;

}; // class TLvFindInfo

//
/// Encapsulates structure LVHITTESTINFO, used to contain information about a hit test in a \link TListViewCtrl \endlink.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774754.aspx
//
class _OWLCLASS TLvHitTestInfo
  : public LVHITTESTINFO
{
public:

  //
  /// \name Constructors
  /// @{

  //
  /// Constructs with a zero point for the hit test.
  //
  TLvHitTestInfo()
  { Init(); }

  //
  /// Constructs with a specified point for the hit test.
  //
  /// @param[in] p is the point to use.
  //
  explicit TLvHitTestInfo(const TPoint& p)
  { Init(); SetPoint(p); }

  //
  /// Constructs a copy of another class instance.
  //
  /// @param[in] info is the class instance to copy.
  //
  explicit TLvHitTestInfo(const LVHITTESTINFO& info)
  { *static_cast<LVHITTESTINFO*>(this) = info; }

  /// @}
  /// \name Accessors
  /// @{

  //
  /// Retrieves the hit item's index.
  //
  /// \return the item index.
  //
  auto GetIndex() -> int
  { return iItem; }

  //
  /// Retrieves the subitem index for the hit item.
  //
  /// \return the subitem index.
  //
  auto GetSubItem() -> int
  { return iSubItem; }

  //
  /// Retrieves the group index for the hit item.
  //
  /// \return the group index.
  //
  auto GetGroup() -> int
  { return iGroup; }

  //
  /// Retrieves the flags for the hit item.
  //
  /// \return the flags.
  //
  auto GetFlags() -> uint
  { return flags; }

  /// @}
  /// \name Manipulators
  /// @{

  //
  /// Sets a point for the hit test.
  //
  /// @param[in] p is the point to use.
  //
  /// \return none.
  //
  auto SetPoint(const TPoint& p) -> void
  { pt.x = p.x; pt.y = p.y; }

  /// @}

protected:

  void Init()
  {
    pt = POINT{0, 0};
    flags = 0;
    iItem = iSubItem = iGroup = -1;
  }

}; // class TLvHitTestInfo

//
/// Encapsulates structure LVITEM, used to describe an item in a \link TListViewCtrl \endlink.
//
/// Contains information about the item's icon, label, state and application-defined value.
/// This class is a thin encapsulation used to pass or retrieve item attributes.
/// The class manages an internal text buffer so that the user is freed from manual text
/// buffer management.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774760.aspx
//
class _OWLCLASS TLvItem
  : public LVITEM
{
public:

  //
  /// TListState is used to describe the state of an item.
  //
  enum TListState
  {
    Unspecified = 0, ///< Unspecified state
    Focus = LVIS_FOCUSED, ///< Only one item has focus
    Selected = LVIS_SELECTED, ///< Marked as selected
    Cut = LVIS_CUT, ///< Marked for cut & paste
    DropHilited = LVIS_DROPHILITED, ///< Marked as drop target
    Activating = LVIS_ACTIVATING, ///< (Ver 4.71) The item is being activated in an LVN_ITEMACTIVATE notification.
    OverlayMask = LVIS_OVERLAYMASK, ///< Retrieve one-based overlay image index
    StateImageMask = LVIS_STATEIMAGEMASK,  ///< Retrieve one-based state image index
  };

  //
  /// Flags for setting the initialisation mask.
  /// \sa TLvItem::TLvItem, LVITEM::mask, http://msdn.microsoft.com/en-us/library/windows/desktop/bb774760.aspx
  //
  enum TMaskFlag
  {
    lvifAll = 0xFFFFFFFF
  };

  //
  /// \name Constructors
  /// @{

  explicit TLvItem(uint mask_ = lvifAll, bool allocTextBuffer = true, int bufferSize = 1000);
  explicit TLvItem(const tstring& text, int subitemIndex = 0);
  TLvItem(const TListViewCtrl& ctl, int index, int subitemIndex, uint mask_ = lvifAll, int bufferSize = 1000);
  TLvItem(const LVITEM& item);
  TLvItem(const TLvItem& item);

  /// @}
  /// \name Overload operations
  /// @{

  auto operator =(const LVITEM& item) -> TLvItem&;
  auto operator =(const TLvItem& item) -> TLvItem&;

  /// @}
  /// \name Text operations
  /// @{

  auto GetText() const -> LPCTSTR;
  auto GetText(LPTSTR buffer, size_t bufferSize) const -> void;
  auto SetTextBuffer(LPTSTR buffer, int bufferSize) -> void;
  auto SetText(const tstring& text) -> void;

  /// @}
  /// \name Index is the 0-based "row"
  /// @{

  auto GetIndex() const -> int;
  auto SetIndex(int index) -> void;

  /// @}
  /// \name Column number
  /// @{

  auto GetSubItem() const -> int;
  auto SetSubItem(int subitemIndex) -> void;

  /// @}
  /// \name Extra data
  /// @{

  auto GetItemData() const -> LPARAM;
  auto SetItemData(LPARAM param) -> void;

  /// @}
  /// \name ImageList index
  /// @{

  auto GetImageIndex() const -> int;
  auto SetImageIndex(int image) -> void;

  /// @}
  /// \name Item state
  /// @{

  auto GetState() const -> int;
  auto SetState(TListState state) -> void;

  /// @}
  /// \name ImageList index for the state image
  /// @{

  auto GetStateImage() const -> int;
  auto SetStateImage(int stateIndex) -> void;

  /// @}
  /// \name Set/Get indent (Version 4.70)
  /// @{

  auto GetIndent() const -> int;
  auto SetIndent(int indent) -> void;

  /// @}

protected:

  typedef std::vector<tchar> TBuffer;
  TBuffer Buffer;

  void Init();

}; // class TLvItem

//
/// Encapsulates structure LVCOLUMN, used to pass or retrieve column attributes in a \link TListViewCtrl \endlink.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774743.aspx
//
class _OWLCLASS TLvColumn
  : public LVCOLUMN
{
public:

  //
  /// TFormat is used to describe the alignment of a column in a list window.
  //
  enum TFormat
  {
    Unspecified = 0, ///< Unspecified
    Left = LVCFMT_LEFT, ///< Left aligned
    Center = LVCFMT_CENTER, ///< Centered
    Right = LVCFMT_RIGHT, ///< Right aligned
  };

  //
  /// Flags for setting the initialisation mask.
  /// \sa TLvColumn::TLvColumn, LVCOLUMN::mask, https://msdn.microsoft.com/en-us/library/windows/desktop/bb774743.aspx
  //
  enum TMaskFlag
  {
    lvcfAll = 0xFFFFFFFF
  };

  //
  /// \name Constructors
  /// @{

  explicit TLvColumn(uint mask_ = lvcfAll, int subitemIndex = 0, int bufferSize = 1000);
  TLvColumn(const tstring& text, int width, TFormat how = Left, int subitemIndex = 0);
  TLvColumn(const TListViewCtrl& ctl, int index, uint mask_ = lvcfAll, int subitemIndex = 0, int bufferSize = 1000);
  TLvColumn(const LVCOLUMN& column);
  TLvColumn(const TLvColumn& column);

  /// @}
  /// \name Overload operations
  /// @{

  auto operator =(const LVCOLUMN& column) -> TLvColumn&;
  auto operator =(const TLvColumn& column) -> TLvColumn&;

  /// @}
  /// \name Information changing operations
  /// @{

  auto SetTextBuffer(LPTSTR buffer, int bufferSize) -> void;
  auto SetText(const tstring& text) -> void;
  auto SetFormat(TFormat how) -> void;
  auto SetWidth(int width, const tstring& text = tstring()) -> void;
  auto SetSubItem(int subitemIndex) -> void;

  /// @}
  /// \name Information retrieval operations
  /// @{

  auto GetText() const -> LPCTSTR;
  auto GetFormat() const -> TFormat;
  auto GetWidth() const -> int;
  auto GetSubItem() const -> int;

  /// @}
  /// \name New IE 3.0 data
  /// @{

  auto GetImage() const -> int;
  auto SetImage(int image) -> void;
  auto GetOrder() const -> int;
  auto SetOrder(int order) -> void;

  /// @}

protected:

  //
  /// Character buffer to hold text.
  //
  typedef std::vector<tchar> TBuffer;
  TBuffer Buffer;

  auto Init() -> void;

}; // class TLvColumn

//
/// Encapsulates structure LVTILEVIEWINFO, used to pass or retrieve tile view information in a \link TListViewCtrl \endlink.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774768.aspx
//
class _OWLCLASS TLvTileViewInfo
  : public LVTILEVIEWINFO
{
public:

  //
  /// TTileSize describes the sizing of tiles in a list view control.
  //
  enum TTileSize
  {
    FixedWidth = LVTVIF_FIXEDWIDTH, ///< Applies a fixed width to the tiles
    FixedHeight = LVTVIF_FIXEDHEIGHT, ///< Applies a fixed height to the tiles
    FixedSize = LVTVIF_FIXEDSIZE, ///< Applies a fixed height and width to the tiles
  };

  //
  /// \name Constructors
  /// @{

  TLvTileViewInfo();
  TLvTileViewInfo(int lines);
  TLvTileViewInfo(const TRect& labelMargin);
  TLvTileViewInfo(int lines, const TRect& labelMargin);
  TLvTileViewInfo(const TSize& size, TTileSize fixedSize);
  TLvTileViewInfo(const TSize& size, TTileSize fixedSize, int lines);
  TLvTileViewInfo(const TSize& size, TTileSize fixedSize, const TRect& labelMargin);
  TLvTileViewInfo(const TSize& size, TTileSize fixedSize, int lines, const TRect& labelMargin);

  /// @}
  /// \name Information changing operations
  /// @{

  auto SetTileSize(const TSize& size) -> void;
  auto SetSizeAutomatic() -> void;
  auto SetSizeFixed(const TSize& size, TTileSize fixedSize) -> void;
  auto SetMaxTextLines(int lines) -> void;
  auto RemoveMaxTextLines() -> void;
  auto SetLabelMargin(const TRect& labelMargin) -> void;
  auto RemoveLabelMargin() -> void;

  /// @}
  /// \name Query operations
  /// @{

  //
  /// Checks if tile view size is automatic.
  //
  /// \return true if tile view size is automatic.
  //
  auto IsSizeAutomatic() const -> bool
  { return dwFlags == LVTVIF_AUTOSIZE; }

  //
  /// Checks if tile view size is fixed-width.
  //
  /// \return true if tile view size is fixed-width.
  //
  auto IsSizeFixedWidth() const -> bool
  { return dwFlags == static_cast<DWORD>(FixedWidth); }

  //
  /// Checks if tile view size is fixed-height.
  //
  /// \return true if tile view size is fixed-height.
  //
  auto IsSizeFixedHeight() const -> bool
  { return dwFlags == static_cast<DWORD>(FixedHeight); }

  //
  /// Checks if tile view size is fixed-height and fixed-width.
  //
  /// \return true if tile view size is fixed-height and fixed-width.
  //
  auto IsSizeFixed() const -> bool
  { return dwFlags == static_cast<DWORD>(FixedSize); }

  /// @}
  /// \name Information retrieval operations
  /// @{

  //
  /// Retrieves the specified coordinates of the label margin.
  //
  /// \return the coordinates.
  //
  auto GetLabelMargin() const -> TRect
  { return rcLabelMargin; }

  //
  /// Retrieves the specified tile size.
  //
  /// \return the tile size.
  //
  auto GetTileSize() const -> TSize
  { return sizeTile; }

  //
  /// Retrieves the specified maximum number of text lines in each item label.
  //
  /// \returns the number of lines.
  //
  auto GetMaxTextLines() const -> int
  { return cLines; }

  /// @}

}; // class TLvTileViewInfo

//
/// Encapsulates structure LVFOOTERINFO, used to pass or retrieve footer information in a \link TListViewCtrl \endlink.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774748.aspx
//
class _OWLCLASS TLvFooterInfo
  : public LVFOOTERINFO
{
public:

  //
  /// \name Constructors
  /// @{

  //
  /// Constructs with zero values.
  //
  TLvFooterInfo()
  { memset(static_cast<LVFOOTERINFO*>(this), 0, sizeof(LVFOOTERINFO)); }

  //
  /// Constructs a copy of another class instance.
  //
  /// @param[in] info is the class instance to copy.
  //
  TLvFooterInfo(const LVFOOTERINFO& info)
  { *static_cast<LVFOOTERINFO*>(this) = info; }

  /// @}

  // TODO!! More elaborate methods

}; // class TLvFooterInfo

//
/// Encapsulates structure LVFOOTERITEM, used to pass or retrieve footer item information in a \link TListViewCtrl \endlink.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774750.aspx
//
class _OWLCLASS TLvFooterItem
  : public LVFOOTERITEM
{
public:

  //
  /// TState is used to describe the state of a footer item in a list window control.
  //
  enum TState
  {
    Focus = LVFIS_FOCUSED, ///< Focus state
    All = LVFIS_FOCUSED, ///< All states
  };

  //
  /// \name Constructors
  /// @{

  //
  /// Constructs with zero values.
  //
  TLvFooterItem()
  { memset(static_cast<LVFOOTERITEM*>(this), 0, sizeof(LVFOOTERITEM)); }

  //
  /// Constructs a copy of another class instance.
  //
  /// @param[in] item is the class instance to copy.
  //
  TLvFooterItem(const LVFOOTERITEM& item)
  { *static_cast<LVFOOTERITEM*>(this) = item; }

  /// @}

  // TODO!! More elaborate methods

}; // class TLvFooterItem

//
/// Encapsulates the ListView control, a window that displays a collection of items, each item
/// consisting of an icon and a label.
///
/// List view controls provide several ways of arranging items and displaying individual items.
/// For example, additional information about each item can be displayed in columns to the right of
/// the icon and label.
//
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb774737.aspx
//
class _OWLCLASS TListViewCtrl
  : public TControl
{
public:

  //
  /// TArrangeCode is used to describe how to arrange the items in a list window control.
  //
  enum TArrangeCode
  {
    Default = LVA_DEFAULT, ///< Use default for control style
    Left = LVA_ALIGNLEFT, ///< Align items to the left edge
    Top = LVA_ALIGNTOP, ///< Align items to the top edge
    SnapToGrid = LVA_SNAPTOGRID, ///< Snap icons to nearest grid position
  };

  //
  /// Describes the type of image list for use with the list window control.
  //
  enum TImageListType
  {
    Normal = LVSIL_NORMAL, ///< Normal image list.
    Small = LVSIL_SMALL, ///< Small icons for LVS_SMALLICON
    State = LVSIL_STATE, ///< State image
  };

  //
  /// Describes the type of rectangle boundaries to retrieve.
  //
  enum TItemRectType
  {
    Bounds = LVIR_BOUNDS, ///< Entire boundary (icon and label)
    Icon = LVIR_ICON, ///< Only the icon
    Label = LVIR_LABEL, ///< Only the label
    SelectBounds = LVIR_SELECTBOUNDS, ///< Union of Icon+Label but no colmns
  };

  //
  /// Describes the next item to retrieve from the current item.
  /// Can have only one relational property, but can have multiple search states.
  //
  /// \note Flags within the DirectionMask are mutually exclusive.
  /// Flags within the the StateMask are not mutually exclusive.
  //
  enum TNextItemCode
  {
    // Relational properties

    Above = LVNI_ABOVE, ///< Directly above
    All = LVNI_ALL, ///< By index
    Below = LVNI_BELOW, ///< Directly below
    Previous = LVNI_PREVIOUS, ///< Ordered before
    ToLeft = LVNI_TOLEFT, ///< Left of
    ToRight = LVNI_TORIGHT, ///< Right of
    DirectionMask = LVNI_DIRECTIONMASK, ///< Directional flag mask combining Above, Below, ToLeft, and ToRight

    // Search states

    Cut = LVNI_CUT, ///< Marked for cut & paste
    DropHilited = LVNI_DROPHILITED, ///< Marked for drop target
    Focused = LVNI_FOCUSED, ///< Marked as having focus
    Selected = LVNI_SELECTED, ///< Marked as selected
    StateMask = LVNI_STATEMASK, ///< State flag mask combining Cut, DropHilited, Focused, and Selected

    // Appearance properties

    VisibleOrder = LVNI_VISIBLEORDER, ///< Search the visible order
    VisibleOnly = LVNI_VISIBLEONLY, ///< Search the visible items
    SameGroupOnly = LVNI_SAMEGROUPONLY, ///< Search the current group
  };

  //
  /// Describes the type of views.
  //
  enum TViewType
  {
    Details = LV_VIEW_DETAILS, ///< Detailed view
    NormalIcon = LV_VIEW_ICON, ///< Icon view
    List = LV_VIEW_LIST, ///< List view
    SmallIcon = LV_VIEW_SMALLICON, ///< Small icon view
    Tile = LV_VIEW_TILE ///< Tile view
  };

  //
  /// Describes the behavior for the control when setting the item count.
  //
  enum TSetItemCountBehavior
  {
    Unspecified = 0, ///< No behavior specified.
    NoInvalidateAll = LVSICF_NOINVALIDATEALL, ///< Do not repaint unless affected items are currently in view.
    NoScroll = LVSICF_NOSCROLL, ///< Do not change the scroll position.
  };

  //
  /// Pure virtual base class for comparing sort items.
  ///
  /// \note Application must implement this class to compare two items used for sorting purposes.
  //
  /// \sa SortItems
  /// \sa SortItemsEx
  //
  class TCompareFunc
  {
  public:
    //
    /// Performs a comparison of two items.
    //
    /// @param[in] item1 is either:
    /// - The value of TLvItem::GetItemData for the first item to be compared when TListViewCtrl::SortItems is used.
    /// - The index of the first item to be compared when TListViewCtrl::SortItemsEx is used.
    /// @param[in] item2 is either:
    /// - The value of TLvItem::GetItemData for the second item to be compared when TListViewCtrl::SortItems is used.
    /// - The index of the second item to be compared when TListViewCtrl::SortItemsEx is used.
    /// @param[in] lParam is an application-defined value that will be passed to the comparison function.
    //
    /// \return the result of the comparison:
    /// - < 0 if item1 < item2
    /// - = 0 if item1 == item2
    /// - > 0 if item1 > item2
    //
    virtual int Compare(LPARAM item1, LPARAM item2, LPARAM lParam) const = 0;
  };

  //
  /// \name Constructors
  /// @{

  TListViewCtrl(TWindow* parent, int id, int x, int y, int w, int h, TModule* module = 0);
  TListViewCtrl(TWindow* parent, int resourceId, TModule* module = 0);

  /// @}
  /// \name Column manipulation
  /// @{

  auto InsertColumn(int colNum, const TLvColumn& column) -> int;
  auto DeleteColumn(int colNum) -> bool;
  auto GetColumn(int colNum, TLvColumn& column) const -> bool;
  auto GetColumnWidth(int colNum) const -> int;
  auto SetColumn(int colNum, const TLvColumn& column) -> bool;
  auto SetColumnWidth(int colNum, int width) -> bool;

  /// @}
  /// \name Item accessors and mutators
  /// @{

  auto GetItem(TLvItem& item, int index = -1, int subitemIndex = -1) const -> bool;
  auto GetItem(int index = -1, int subitemIndex = -1) const -> TLvItem;
  auto SetItem(const TLvItem& item, int index = -1, int subitemIndex = -1) -> bool;
  auto GetNextItem(int index, uint flags = TNextItemCode::All) const -> int;
  auto GetItemCount() const -> int;
  auto SetItemCount(int numItems, TSetItemCountBehavior behavior = Unspecified) -> bool;
  auto GetItemPosition(int index, TPoint& pt) const -> bool;
  auto GetItemPosition(int index) const -> TPoint;
  auto SetItemPosition(int index, const TPoint& pt) -> bool;
  auto SetItemPosition32(int index, const TPoint& pt) -> void;
  auto GetItemRect(int index, TRect& rect, TItemRectType type) const -> bool;
  auto GetItemRect(int index, TItemRectType type) const -> TRect;
  auto GetItemState(int index, uint mask) const -> TLvItem::TListState;
  auto SetItemState(int index, TLvItem::TListState state, uint mask) -> bool;
  auto GetItemText(int index, TLvItem& item) const -> int;
  auto GetItemText(int index, int subitemIndex, LPTSTR buffer, int bufferSize) const -> int;
  auto GetItemText(int index, int subitemIndex = 0) const -> tstring;
  auto SetItemText(int index, const TLvItem& item) -> bool;
  auto SetItemText(int index, int subitemIndex, LPCTSTR text) -> bool;
  auto SetItemText(int index, int subitemIndex, const tstring& text) -> bool;

  /// @}
  /// \name Item insertion and deletion
  /// @{

  auto AddItem(const TLvItem& item) -> int;
  auto AddItem(const tstring& text) -> int;
  auto InsertItem(const TLvItem& item, int index = -1) -> int;
  auto InsertItem(const tstring& text, int index = 0) -> int;
  auto DeleteItem(int index) -> bool;
  auto DeleteAllItems() -> bool;

  /// @}
  /// \name Find/Sort/Arrange
  /// @{

  auto FindItem(int index, const TLvFindInfo& findInfo) const -> int;
  auto SortItems(const TCompareFunc& Comparator, LPARAM lParam = 0) -> bool;
  auto SortItemsEx(const TCompareFunc& Comparator, LPARAM lParam = 0) -> bool;
  auto Arrange(TArrangeCode code) -> bool;

  /// @}
  /// \name Item selection
  /// @{

  auto IsSelected(int index) const -> bool;
  auto GetNextSelIndex(int index = -1) const -> int;
  auto GetSelIndex() const -> int;
  auto GetSelCount(void) const -> int;
  auto GetSelIndexes(int* indexes, int maxCount) const -> int;
  auto GetSelString(tchar* str, int maxChars, int subitemIndex = 0) const -> bool;
  auto GetSelString(int subitemIndex = 0) const -> tstring;
  auto GetSelStrings(tchar** strs, int maxCount, int maxChars, int subitemIndex = 0) const -> int;
  auto SetSel(int index, bool select) -> bool;
  auto SetSelIndexes(int* indexes, int numSelections, bool select) -> bool;
  auto SetSelItemRange(bool select, int first, int last) -> bool;
  auto GetSelectedColumn() const -> int;
  auto SetSelectedColumn(int colNum) -> void;

  /// @}
  /// \name Image list manipulation
  /// @{

  auto CreateDragImage(int index, TPoint* upLeft) -> HIMAGELIST;
  auto GetImageList(TImageListType type) const -> HIMAGELIST;
  auto SetImageList(HIMAGELIST list, TImageListType type) -> HIMAGELIST;

  /// @}
  /// \name Color accessors and manipulation
  /// @{

  auto GetBkColor() const -> TColor;
  auto SetBkColor(const TColor& c) -> bool;
  auto GetTextBkColor() const -> TColor;
  auto SetTextBkColor(const TColor& c) -> bool;
  auto GetTextColor() const -> TColor;
  auto SetTextColor(const TColor& c) -> bool;

  /// @}
  /// \name Callbacks
  /// @{

  auto GetCallBackMask() const -> TLvItem::TListState;
  auto SetCallBackMask(TLvItem::TListState mask) -> bool;

  /// @}
  /// \name Miscellaneous
  /// @{

  auto GetTopIndex() const -> int;
  auto EditLabel(int index) -> HWND;
  auto GetEditControl() const -> HWND;
  auto CancelEditLabel() -> void;
  auto Update(int index) -> bool;
  auto HitTest(TLvHitTestInfo&) const -> int;
  auto HitTest(const TPoint&) const -> TLvHitTestInfo;
  auto Scroll(int dx, int dy) -> bool;
  auto GetOrigin(TPoint& pt) const -> bool;
  auto GetStringWidth(LPCTSTR text) const -> int;
  auto GetStringWidth(const tstring& text) const -> int;
  auto CalculateColumnWidth(LPCTSTR text, int padding = 12) const -> int;
  auto CalculateColumnWidth(const tstring& text, int padding = 12) const -> int;
  auto GetViewRect(TRect& rect) const -> bool;
  auto IsItemVisible(int index) const -> bool;
  auto EnsureVisible(int index, bool partialOk) -> bool;
  auto RedrawItems(int startIndex, int endIndex) -> bool;
  auto GetCountPerPage() const -> int;
  auto GetISearchString() const -> tstring;
  auto GetFocusItem() -> int;
  auto SetFocusItem(int index, bool focused = true) -> bool;

  /// @}
  /// \name New commctrl messages - version 4.70
  /// @{

  auto GetApproxRect(int x = -1, int y = -1, int count = -1) const -> TSize;

  //
  /// TPoint overload for \link GetApproxRect(int, int, int) const \endlink.
  //
  auto GetApproxRect(const TPoint& pt, int count = -1) const -> TSize
  { return GetApproxRect(pt.x, pt.y, count); }

  auto GetColumnOrder(int count, int* array) const -> bool;
  auto SetColumnOrder(int count, const int* array) -> bool;
  auto GetExtStyle() const -> uint32;
  auto SetExtStyle(uint32 mask, uint32 style) -> uint32;
  auto GetHeaderCtrl() const -> HWND;
  auto GetHotCursor() const -> HCURSOR;
  auto SetHotCursor(HCURSOR cur) -> HCURSOR;
  auto GetHotItem() const -> int;
  auto SetHotItem(int index) -> int;
  auto GetSubItemRect(TRect& rect, int subitemIndex = 0, int index = 0, TItemRectType type = Bounds) const -> bool;
  auto SubItemHitTest(TLvHitTestInfo& info) const -> int;
  auto SubItemHitTest(const TPoint&) const -> TLvHitTestInfo;
  auto GetItemSpacing(bool smallIcon) const -> TSize;
  auto SetIconSpacing(int x, int y) -> TSize;

  //
  /// TPoint overload for \link SetIconSpacing(int, int) \endlink.
  //
  auto SetIconSpacing(const TPoint& pt) -> TSize
  { return SetIconSpacing(pt.x, pt.y); }

  //
  /// Resets the icon spacing to the default spacing.
  //
  /// \return the previous icon spacing used.
  //
  auto ResetIconSpacing() -> TSize
  { return SetIconSpacing(-1, -1); }

  /// @}
  /// \name New commctrl messages - version 4.71
  /// @{

  auto GetBkImage(TLvBkImage& bkimg) const -> bool;
  auto SetBkImage(const TLvBkImage& bkimg) -> bool;
  auto GetHoverTime() const -> uint32;
  auto SetHoverTime(uint32 tm) -> uint32;
  auto GetNumOfWorkAreas() const -> uint;
  auto GetWorkAreas(int count, TRect* areas) const -> void;
  auto SetWorkAreas(int count, TRect* areas) -> void;
  auto GetSelectionMark() const -> int;
  auto SetSelectionMark(int index) -> int;

  /// @}
  /// \name New commctrl messages - version 6.00
  /// @{

  auto GetItemIndexRect(int index, int group, int subitemIndex, TRect& rect, TItemRectType type) const -> bool;
  auto GetNextItemIndex(int index, int group, uint flags = TNextItemCode::All) const -> int;
  auto SetItemIndexState(const TLvItem& item, int index, int group) -> bool;
  auto GetEmptyText() const -> tstring;
  auto GetFooterInfo(TLvFooterInfo& info) const -> bool;
  auto GetFooterItems() const -> int;
  auto GetFooterItem(int index, TLvFooterItem& item) const -> bool;
  auto GetFooterItemText(int index) const -> std::wstring;
  auto GetFooterItemState(int index, TLvFooterItem::TState mask = TLvFooterItem::All) const -> TLvFooterItem::TState;
  auto GetFooterItemRect(int index, TRect& rect) const -> bool;
  auto GetFooterItemRect(int index) const -> TRect;
  auto GetFooterRect(TRect& rect) const -> bool;
  auto GetFooterRect() const -> TRect;
  auto GetGroupInfo(int groupId, uint mask = 0xFFFFFFFF) -> LVGROUP;

  /// @}
  /// \name Unicode setting messages
  /// @{

  auto GetUnicodeFormat() const -> bool;
  auto SetUnicodeFormat(bool useUnicode = true) -> bool;

  /// @}
  /// \name Unique item identifier messages
  /// @{

  //
  /// Defines the type of a unique identifier for an item.
  //
  using TItemId = TParam1;

  auto MapIndexToId(int index) const -> TItemId;
  auto MapIdToIndex(TItemId id) const -> int;

  /// @}
  /// \name Tooltip messages
  /// @{

  auto GetToolTips() const -> HWND;
  auto SetToolTips(THandle handle) -> HWND;
  auto SetInfoTip(LPCWSTR text, int index, int subitemIndex = 0) -> bool;
  auto SetInfoTip(LPCSTR text, int index, int subitemIndex = 0) -> bool;
  auto SetInfoTip(const tstring& text, int index, int subitemIndex = 0) -> bool;

  /// @}
  /// \name View type messages
  /// @{

  auto GetView() const -> TViewType;
  auto SetView(TViewType viewType) -> bool;

  /// @}
  /// \name Tile view messages
  /// @{

  auto GetTileInfo(PLVTILEINFO lvTileInfo) const -> void;
  auto GetTileInfo(LVTILEINFO& lvTileInfo) const -> void;
  auto SetTileInfo(PLVTILEINFO lvTileInfo) -> bool;
  auto SetTileInfo(LVTILEINFO& lvTileInfo) -> bool;
  auto GetTileViewInfo(TLvTileViewInfo& tileViewInfo) const -> void;
  auto SetTileViewInfo(const TLvTileViewInfo& tileViewInfo) -> bool;

  /// @}
  /// \name Border outline color messages
  /// @{

  auto GetOutlineColor() const -> TColor;
  auto SetOutlineColor(const TColor& color) -> TColor;

  /// @}

#if defined(OWL5_COMPAT)

  auto GetOrigin(POINT* pt) const -> bool;
  auto GetViewRect(RECT* rect) const -> bool;
  auto GetColumn(int index, LVCOLUMN* column) const -> bool;
  auto GetItemPosition(int index, POINT* pt) const -> bool;
  auto GetItemRect(int index, RECT* rect, TItemRectType type) const -> bool;
  auto DeleteAnItem(int index) -> bool { return DeleteItem(index); }
  auto FindItem(int index, const TLvFindInfo* findInfo) const -> int;

#endif

protected:

  virtual auto GetWindowClassName() -> TWindowClassName;
  using TControl::DeleteItem; // Inject virtual overload (DELETEITEMSTRUCT&), otherwise hidden by DeleteItem(int).

private:

  TListViewCtrl(const TListViewCtrl&);
  TListViewCtrl& operator=(const TListViewCtrl&);
}; // class TListViewCtrl

/// @}

#include <owl/posclass.h>

} // owl namespace

#endif
