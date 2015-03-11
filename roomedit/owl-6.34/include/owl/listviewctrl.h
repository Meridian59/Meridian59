//
/// \file
/// Definition of TListViewCtrl class
//
// Part of OWLNext - the next generation Object Windows Library 
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
// Copyright © 2011 Vidar Hasfjord 
//
// For more information, including license details, see 
// http://owlnext.sourceforge.net
//

#if !defined(OWL_LISTVIEWCTRL_H)
#define OWL_LISTVIEWCTRL_H

#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
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
// Constants representing all mask flags of LV_COLUMN and LV_ITEM
//
#if !defined(LVCF_ALL)
# define LVCF_ALL  (LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH)
#endif
#if !defined(LVIF_ALL)
# define LVIF_ALL  (LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM|LVIF_STATE)
#endif

//
/// \class TLvFindInfo
/// A structure to find an item in a TListViewCtrl
//
class _OWLCLASS TLvFindInfo 
  : public LVFINDINFO 
{
public:

  TLvFindInfo();
  TLvFindInfo(const LVFINDINFO&);

  //
  /// Sets string for search.
  //
  void SetString(const tstring&);

  //
  /// Sets string for exact substring search.
  //
  void SetSubstring(const tstring&);

  //
  /// Sets string for partial search.
  //
  void SetPartial(const tstring&);

  //
  /// Sets wrap-around flag.
  /// If true, the search continues at the beginning if the end has been reached.
  //
  void SetWrap(bool = true);

  //
  /// Sets extra application-specific information.
  //
  void SetData(LPARAM param);

protected:

  tstring Text;

  void Init();
};


//
/// \class TLvHitTestInfo
/// Determines if a point is somewhere on an item or not.
//
class _OWLCLASS TLvHitTestInfo 
  : public LVHITTESTINFO 
{
public:

  TLvHitTestInfo();
  explicit TLvHitTestInfo(const TPoint& p);
  TLvHitTestInfo(const LVHITTESTINFO&);

  //
  /// Sets the point information.
  //
  void SetPoint(const TPoint& p);

  //
  /// Returns the hit item's index.
  //
  int GetIndex();

  //
  /// Returns the sub-item-index for the hit item.
  //
  int GetSubItem();

  //
  /// Returns the flags for the hit item.
  //
  uint GetFlags();

protected:

  void Init();
};

//
/// \class TLvItem
/// Parameter package representing the attributes of an item in a TListViewCtrl
///
/// Contains information about the item's icon, label, state and application-defined value.
/// This class is a thin encapsulation of LVITEM used to pass or retrieve item attributes.
/// The class manages an internal text buffer so that the user is freed from manual text
/// buffer management.
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
    Focus = LVIS_FOCUSED, ///< Only one item has focus
    Selected = LVIS_SELECTED, ///< Marked as selected
    Cut = LVIS_CUT, ///< Marked for cut & paste
    DropHilited = LVIS_DROPHILITED, ///< Marked as drop target
    Activating = LVIS_ACTIVATING, ///< (Ver 4.71) The item is being activated in an LVN_ITEMACTIVATE notification.
    OverlayMask = LVIS_OVERLAYMASK, ///< Retrieve one-based overlay image index
    StateImageMask = LVIS_STATEIMAGEMASK,  ///< Retrieve one-based state image index
  };

  //
  /// Constructs an empty parameter package; typically used for retrieving information about an 
  /// existing item. For example,
  /// \code
  ///   TLvItem item;
  ///   bool success = ListWindow.GetItem(item, index, subitemIndex);
  ///   if (success && item.GetText()) ShowString(item.GetText());
  /// \endcode
  /// If allocTextBuffer is true, then a buffer with room for bufferSize characters is allocated,
  /// and pszText is set to point to it.
  //
  explicit TLvItem(uint mask = LVIF_ALL, bool allocTextBuffer = true, int bufferSize = 1000);

  //
  /// Constructs a parameter package with the given text.
  /// The given text is copied to the internal buffer, and pszText will point to the internal copy.
  /// For example,
  /// \code
  ///   TLvItem item("Item");
  ///   ListWindow.AddItem(item);
  /// \endcode
  //
  explicit TLvItem(const tstring& text, int subitemIndex = 0);

  //
  /// Constructs a parameter package based on the item attributes of an existing control.
  /// If the mask includes LVIF_TEXT, then a buffer with room for bufferSize characters is allocated,
  /// and pszText is set to point to it.
  //
  TLvItem(const TListViewCtrl& ctl, int index, int subitemIndex, uint mask = LVIF_ALL, int bufferSize = 1000);

  //
  /// Copies the attributes from an existing parameter package.
  /// A deep copy is performed; see the copy constructor for details.
  //
  TLvItem(const LVITEM&);

  //
  /// A deep copy is performed; if the passed item has the LVIF_TEXT flag set, the text pointed to 
  /// by the passed item's pszText is copied into the internal text buffer, and pszText is set to 
  /// point to the internal copy.
  //
  TLvItem(const TLvItem&);

  //
  /// A deep copy is performed; see the copy constructor for details.
  //
  TLvItem& operator =(const LVITEM&);

  //
  /// Retrieves the text pointed to by the internal buffer pointer (LVITEM::pszText). 
  /// This might point to the internal text buffer, or to an external buffer.
  /// See SetTextBuffer.
  // 
  LPCTSTR GetText() const;

  //
  /// Copies the text from the currently set text buffer. The text buffer may be
  /// the internal buffer or an external buffer. To determine the required size of the 
  /// passed destination buffer, use (_tcslen(GetText()) + 1).
  //
  void GetText(LPTSTR buffer, size_t bufferSize) const;

  //
  /// Overrides the internal buffer and assigns an external text buffer.
  /// Sets LVITEM::pszText and cchTextMax, and enables the LVIF_TEXT flag.
  /// Note: This function does not copy the text!
  //
  void SetTextBuffer(LPTSTR buffer, int bufferSize);

  //
  /// Copies the given text into the internal text buffer.
  /// Sets LVITEM::pszText and cchTextMax, and enables the LVIF_TEXT flag.
  /// Note: If the string is larger than what can be represented by the class, i.e. INT_MAX, then
  /// a TXOwl exception is thrown.
  //
  void SetText(const tstring&);

  //
  /// \name Index is the 0-based "row"
  /// @{

  int GetIndex() const;
  void SetIndex(int);

  /// @}
  /// \name Column number
  /// @{

  int GetSubItem() const;
  void SetSubItem(int);

  /// @}
  /// \name Extra data
  /// @{

  LPARAM GetItemData() const;
  void SetItemData(LPARAM);

  /// @}
  /// \name ImageList index
  /// @{

  int GetImageIndex() const;
  void SetImageIndex(int index);

  /// @}
  /// \name Item state
  /// @{

  int GetState() const;
  void SetState(TListState state);

  /// @}
  /// \name ImageList index for the state image
  /// @{

  int GetStateImage() const;
  void SetStateImage(int index);

  /// @}
  /// \name Set/Get indent (Version 4.70)
  /// @{

  int GetIndent() const;
  void SetIndent(int index);

  /// @}

protected:

  typedef std::vector<tchar> TBuffer;
  TBuffer Buffer;

  void Init();
};

//
/// \class TLvColumn
/// Parameter package representing the attributes of a column in a TListViewCtrl
///
/// This class is a thin encapsulation of LVCOLUMN used to pass or retrieve column attributes.
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
    Left = LVCFMT_LEFT, ///< Left aligned
    Center = LVCFMT_CENTER, ///< Centered
    Right = LVCFMT_RIGHT, ///< Right aligned
  };

  //
  /// Constructs an empty parameter package; typically used to retrieve information about an 
  /// existing column. For example,
  /// \code
  ///    TLvColumn col;
  ///    bool success = ListWnd.GetColumn(1, col);
  ///    if (success && col.GetText()) ShowString(col.GetText());
  /// \endcode
  /// If the mask includes LVCF_TEXT, then a cache with room for bufferSize characters is allocated,
  /// and pszText is set to point to it.
  //
  explicit TLvColumn(uint mask = LVCF_ALL, int subitemIndex = 0, int bufferSize = 1000);

  //
  /// Constructs a parameter package based on the given text, width and format.
  //
  TLvColumn(const tstring& text, int width, TFormat how = Left, int subitemIndex = 0);

  //
  /// Constructs a parameter package based on the column attributes of an existing control. 
  /// If the mask includes LVCF_TEXT, then a cache with room for bufferSize characters is allocated,
  /// and pszText is set to point to it.
  //
  TLvColumn(const TListViewCtrl& ctl, int index, uint mask = LVCF_ALL, int subitemIndex = 0, int bufferSize = 1000);

  //
  /// Copies the attributes from an existing parameter package.
  /// A deep copy is performed; see the copy constructor for details.
  //
  TLvColumn(const LVCOLUMN&);       

  //
  /// A deep copy is performed; if the passed item has the LVCF_TEXT flag set, the text pointed to 
  /// by the passed item's pszText is copied into the internal text buffer, and pszText is set to 
  /// point to the internal copy.
  //
  TLvColumn(const TLvColumn&);

  //
  /// A deep copy is performed; see the copy constructor for details.
  //
  TLvColumn& operator =(const LVCOLUMN&);

  //
  /// \name Member initializers
  /// @{

  //
  /// Overrides the internal buffer and assigns an external text buffer.
  /// Sets LVITEM::pszText and cchTextMax, and enables the LVIF_TEXT flag.
  /// Note: This function does not copy the text!
  //
  void SetTextBuffer(LPTSTR buffer, int bufferSize);

  //
  /// Copies the given text into the internal text buffer.
  /// Sets LVCOLUMN::pszText and cchTextMax, and enables the LVCF_TEXT flag.
  /// Note: If the string is larger than what can be represented by the class, i.e. INT_MAX, then
  /// a TXOwl exception is thrown.
  //
  void SetText(const tstring&);

  //
  /// Sets the alignment for the column
  //
  void SetFormat(TFormat how);

  //
  /// Sets the width of the column.
  /// \note Will compute a default width using the system's default GUI
  //  font and the text parameter if 'pixels=0' and 'txt != 0'.
  //
  void SetWidth(int pixels, const tstring& txt = tstring());

  //
  /// Sets the sub-item index.
  //
  void SetSubItem(int subitemIndex);
  
  /// @}
  /// \name Information retrieval
  /// \note These members only return static data currently in the structure.
  /// @{

  LPCTSTR GetText() const {return pszText;}
  TFormat GetFormat() const {return TFormat(fmt);}
  int GetWidth() const {return cx;}
  int GetSubItem() const {return iSubItem;}

  /// @}
  /// \name New IE 3.0 data
  /// @{

  void SetImage(int image);
  void SetOrder(int order);

  /// @}

protected:

  typedef std::vector<tchar> TBuffer;
  TBuffer Buffer;

  void Init();
};

//
/// \class TListViewCtrl
/// Encapsulates the ListView control, a window that displays a collection of items, each item 
/// consisting of an icon and a label. 
///
/// List view controls provide several ways of arranging items and displaying individual items. 
/// For example, additional information about each item can be displayed in columns to the right of
/// the icon and label.
///
/// The control has four views: Icon,  Small Icon, Report, and List. 
/// The view is determined by the style attribute, set either with Attr.Style or SetWindowLong.
/// The constants are LVS_ICON, LVS_SMALLICON, LVS_REPORT and LVS_LIST.
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
    SelectBounds = LVIR_SELECTBOUNDS///< Union of Icon+Label but no colmns
  };

  //
  /// Describes the next item to retrieve from the current item.
  /// Can have only one relational property, but can have multiple search states.
  //
  enum TNextItemCode 
  {
    // Relational properties

    Above = LVNI_ABOVE, ///< Directly above
    All = LVNI_ALL, ///< By index
    Below = LVNI_BELOW, ///< Directly below
    ToLeft = LVNI_TOLEFT, ///< Left of
    ToRight = LVNI_TORIGHT, ///< Right of

    // Search states

    Cut = LVNI_CUT, ///< Marked for cut & paste
    DropHilited = LVNI_DROPHILITED, ///< Marked for drop target
    Focused = LVNI_FOCUSED, ///< Marked as having focus
    Selected = LVNI_SELECTED, ///< Marked as selected
  };

  //
  /// \class TCompareFunc
  /// Base class for comparing sort items
  ///
  /// Implement this class to compare two items for sorting purposes.
  /// The return value of Compare should be 
  ///   < 0 if item1 < item2
  ///   = 0 if item1 == item2
  ///   > 0 if item1 > item2
  /// lParam is a application-specific value.
  //
  class TCompareFunc 
  {
  public:
    virtual int Compare(LPARAM item1, LPARAM item2, LPARAM lParam) const = 0;
  };

  TListViewCtrl(TWindow* parent, int id, int x, int y, int w, int h, TModule* module = 0);
  TListViewCtrl(TWindow* parent, int resourceId, TModule* module = 0);

  //
  /// \name Column manipulation
  /// @{

  int InsertColumn(int colNum, const TLvColumn&);
  bool DeleteColumn(int colNum);
  bool GetColumn(int index, TLvColumn& column) const;
  int GetColumnWidth(int index) const;
  bool SetColumn(int index, const TLvColumn&);
  bool SetColumnWidth(int index, int width);

  /// @}
  /// \name Item accessors and mutators
  /// @{

  TLvItem GetItem(int index = -1, int subitem = -1) const;
  bool GetItem(TLvItem&, int index = -1, int subitem = -1) const;
  bool SetItem(const TLvItem&, int index = -1, int subitemIndex = -1);
  int GetNextItem(int index, TNextItemCode code) const;
  int GetItemCount() const;
  void SetItemCount(int numItems, uint32 flags=0);
  bool GetItemPosition(int index, TPoint& pt) const;

  //
  /// If the coordinates are outside valid range, a TXOwl exception is thrown.
  //
  bool SetItemPosition(int index, const TPoint& pt);

  bool GetItemRect(int index, TRect&, TItemRectType);
  uint GetItemState(int index, uint mask) const;
  bool SetItemState(int index, uint state, uint mask);
  bool SetItemText(int index, const TLvItem& item);
  bool SetItemText(int index, int subitemIndex, LPCTSTR text);
  bool SetItemText(int index, int subitemIndex, const tstring& text);

  //
  /// Returns the text of the item.
  ///
  /// @param[in] index is the index of the item in the ListView.
  /// @param[in] subitemIndex is the index of the subitem of the ListView item. If it is 0, the
  /// item's label is retrieved. If it is not 0, the text of the subitem is retrieved.
  /// @param[out] buffer points to a buffer to hold the retrieved text.
  /// @param[in] bufferSize is the size of the buffer pointed to by text.
  /// 
  /// \return Returns the size of the retrieved string.
  //
  int GetItemText(int index, int subitemIndex, LPTSTR buffer, int bufferSize) const;

  //
  /// String class overload
  //
  tstring GetItemText(int index, int subitemIndex = 0) const;

  /// @}
  /// \name Item insertion and deletion
  /// @{

  //
  /// Add item to the end of the list.
  //
  int AddItem(const TLvItem&);

  //
  /// Add a new item with the given text to the end of the list.
  //
  int AddItem(const tstring&);

  //
  /// Inserts the item into the position indicated by index or item.iItem.
  /// \note If index is < 0, then the value provided in item.iItem must be valid.
  /// If index is >=0, it will be set as iItem member before inserting the item. 
  //
  int InsertItem(const TLvItem&, int index = -1);

  //
  /// Insert a new item with the given text to the list.
  /// \note If index is < 0, then the value provided in item.iItem must be valid.
  /// If index is >=0, it will be set as iItem member before inserting the item. 
  //
  int InsertItem(const tstring&, int index = -1);

  bool DeleteItem(int index);
  bool DeleteAllItems();

  /// @}
  /// \name Find/Sort/Arrange
  /// @{

  int FindItem(int startIndex, const TLvFindInfo& findInfo);

  //
  /// Sort the items within the List Window.
  /// TCompareFunc is the base class for the comparison function
  //
  bool SortItems(const TCompareFunc& Comparator, LPARAM lParam = 0);
  
  //
  /// Sort the items within the List Window. 
  /// TCompareFunc is the base class for the comparison function.
  ///
  /// Difference from SortItems is that the comparator received the indexes of the items instead of the item data
  //
  bool SortItemsEx(const TCompareFunc& Comparator, LPARAM lParam = 0);

  bool Arrange(TArrangeCode code);

  /// @}
  /// \name Item selection
  /// @{

  //
  ///  Determine selection state of an item.
  ///  Return true if the item is selected.
  //
  bool IsSelected(int index) const;

  //
  /// Returns the index of the selected item, provided the list view has the LVS_SINGLESEL style.
  /// If no item is selected, or the list view does not have the LVS_SINGLESEL style, -1 is returned.
  //
  int GetSelIndex() const;

  //
  /// Return the number of items currently selected.
  /// (Useful before calling GetSelIndexes.)
  //
  int GetSelCount(void) const;

  //
  ///  Fill an integer array with indexes of selected items.
  ///  (Note GetSelCount returns the number of selected items.)
  ///  Returns the number of indexes placed in the indexes array.
  ///  If the list view uses the LVS_SINGLESEL, multiple selection
  ///  has been disabled and the function returns -1.
  //
  int GetSelIndexes(int* indexes, int maxCount) const;

  //
  /// Fill an array with strings from the selected items.
  /// @param[out] strs array of strings.
  /// @param[in] maxCount is the size of the strs[] array.  
  /// @param[in] maxChars is the size of each array element.  
  /// @param[in] subitemIndex indicates which item string to return.
  ///
  /// \return Returns the number of strings retrieved, or -1 for errors.
  //
  int GetSelStrings(tchar** strs, int maxCount, int maxChars, int subitemIndex = 0) const;

  //
  ///  Select or deselect an item.
  ///  Returns true for success.
  //
  bool SetSel(int index, bool select);

  //
  ///  Select or deselect all the items whose indexes appear in indexes[]
  ///  Returns true for success.
  //
  bool SetSelIndexes(int* indexes, int numSelections, bool select);

  //
  ///  Select or deselect all the items in the given range.
  ///  Returns true for success.
  //
  bool SetSelItemRange(bool select, int first, int last);

  /// @}
  /// \name Image list manipulation
  /// @{

  HIMAGELIST CreateDragImage(int itemIndex, TPoint* upLeft);
  HIMAGELIST GetImageList(TImageListType);
  bool SetImageList(HIMAGELIST list, TImageListType type);

  /// @}
  /// \name Color manipulation
  /// @{

  bool SetBkColor(const TColor& c);
  bool SetTextBkColor(const TColor& c);
  bool SetTextColor(const TColor& c);

  /// @}
  /// \name Callbacks
  /// @{

  uint GetCallBackMask() const;
  bool SetCallBackMask(uint mask);

  /// @}
  /// \name Miscellaneous
  /// @{

  int GetTopIndex() const;
  HWND EditLabel(int itemIndex);
  HWND GetEditControl() const;
  bool Update(int index);
  int HitTest(TLvHitTestInfo&);
  bool Scroll(int dx, int dy);
  bool GetOrigin(TPoint& pt);
  int GetStringWidth(LPTSTR text);
  bool GetViewRect(TRect&);
  bool EnsureVisible(int index, bool partialOk);
  bool RedrawItems(int startIndex, int endIndex);
  int  GetCountPerPage() const;

  /// @}
  /// \name New commctrl messages - version 4.70
  /// @{

  //
  /// Calculates the approximate width and height required to display a given number of items.
  /// Pass -1 for a coordinate to use
  /// If the coordinates are outside valid range, a TXOwl exception is thrown.
  //
  TSize GetApproxRect(int x = -1, int y = -1, int count=-1) const;

  //
  /// If the coordinates are outside valid range, a TXOwl exception is thrown.
  //
  TSize GetApproxRect(const TPoint& pt, int count=-1) const;

  bool GetColumnOrder(int count, int* array) const;
  bool SetColumnOrder(int count, int* array);
  uint32 GetExtStyle() const;
  void SetExtStyle(uint32 mask, uint32 style);
  HWND GetHeaderCtrl() const;
  HCURSOR GetHotCursor() const; 
  void SetHotCursor(HCURSOR cur);
  int GetHotItem() const;
  void SetHotItem(int item);
  bool GetSubItemRect(TRect& rect, int subidx=0, int paridx=0, int flag=LVIR_BOUNDS) const;
  int SubItemTest(const TLvHitTestInfo& info) const;
  void SetIconSpacing(int x, int y);
  void ResetIconSpacing() {SetIconSpacing(0, -1);}
  void SetIconSpacing(const TPoint& pt);

  /// @}
  /// \name New commctrl messages - version 4.71
  /// @{

  bool GetBkImage(TLvBkImage& bkimg) const; 
  bool SetBkImage(const TLvBkImage& bkimg);
  uint32 GetHoverTime() const;
  void SetHoverTime(uint32 tm);
  uint GetNumOfWorkAreas() const;
  void GetWorkAreas(int count, TRect* areas) const;
  void SetWorkAreas(int count, TRect* areas);
  int GetSelectionMark() const;
  void SetSelectionMark(int index);

  /// @}

#if defined(OWL5_COMPAT)

  bool SetBkColor(COLORREF c);
  bool SetTextBkColor(COLORREF c);
  bool SetTextColor(COLORREF c);
  COLORREF GetBkColor();
  COLORREF GetTextBkColor();
  COLORREF GetTextColor();
  bool GetOrigin(POINT*);
  bool GetViewRect(RECT*);
  bool GetColumn(int index, LVCOLUMN*);
  bool GetItemPosition(int index, POINT*);
  bool GetItemRect(int index, RECT*, TItemRectType);
  bool DeleteAnItem(int index) {return DeleteItem(index);}
  int FindItem(int startIndex, const TLvFindInfo* findInfo) {return FindItem(startIndex, *findInfo);}

#else

  TColor GetBkColor();
  TColor GetTextBkColor();
  TColor GetTextColor();

#endif

protected:

  virtual TGetClassNameReturnType GetClassName();
  using TControl::DeleteItem; // Inject virtual overload (DELETEITEMSTRUCT&), otherwise hidden by DeleteItem(int).

private:

  TListViewCtrl(const TListViewCtrl&);
  TListViewCtrl& operator=(const TListViewCtrl&);
};

/// @}

#include <owl/posclass.h>

} // OWL namespace

#endif



