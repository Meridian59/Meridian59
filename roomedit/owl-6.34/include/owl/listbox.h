//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TListBox and TlistBoxData.  This defines the basic
/// behavior of all list box controls.
//----------------------------------------------------------------------------

#if !defined(OWL_LISTBOX_H)
#define OWL_LISTBOX_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/control.h>
#include <owl/contain.h>
#include <owl/template.h>


namespace owl {

class _OWLCLASS TListBoxData;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \addtogroup ctrl
/// @{
/// \class TListBox
// ~~~~~ ~~~~~~~~
/// An interface object that represents a corresponding list box element. A TListBox
/// must be used to create a list box control in a parent TWindow. A TListBox can be
/// used to facilitate communication between your application and the list box
/// controls of a TDialog object. TListBox's member functions also serve instances
/// of its derived class TComboBox. From within MDI child windows, you can access a
/// TListBox object by using TFrameWindow::GetClientWindow. TListBox is a streamable
/// class.
//
class _OWLCLASS TListBox : public TControl 
{
  public:
    TListBox(TWindow* parent, int id, int x, int y, int w, int h, TModule* module = 0);
    TListBox(TWindow* parent, int resourceId, TModule* module = 0);
    TListBox(THandle hWnd, TModule* module = 0);

    /// \name List box attributes
    /// @{
    virtual int   GetCount() const;
    virtual int   FindString(LPCTSTR str, int startIndex = -1) const;
    int           FindString(const tstring& str, int startIndex = -1) const {return FindString(str.c_str(), startIndex);}
    virtual int   FindStringExact(LPCTSTR str, int startIndex = -1) const;
    int           FindStringExact(const tstring& str, int startIndex = -1) const {return FindStringExact(str.c_str(), startIndex);}
    virtual int   SetSelStringExact(LPCTSTR str, int startIndex = -1); //DLN added
    int           SetSelStringExact(const tstring& str, int startIndex = -1) {return SetSelStringExact(str.c_str(), startIndex);}
    virtual int   GetTopIndex() const;
    virtual int   SetTopIndex(int index);
    bool          SetTabStops(int numTabs, int * tabs);

    virtual int   GetHorizontalExtent() const;
    virtual void  SetHorizontalExtent(int horzExtent);
    void          SetColumnWidth(int width);
    int           GetCaretIndex() const;
    int           SetCaretIndex(int index, bool partScrollOk);
    /// @}

    /// \name Query individual list items
    /// @{
    virtual int   GetStringLen(int index) const;
    virtual int   GetString(LPTSTR str, int index) const;
    tstring       GetString(int index) const;
    virtual LPARAM GetItemData(int index) const;
    virtual int SetItemData(int index, LPARAM itemData);

    int           GetItemRect(int index, TRect& rect) const;
    virtual int   GetItemHeight(int index) const;
    virtual int   SetItemHeight(int index, int height);
    /// @}

    /// \name Operations on the list box itself
    /// @{
    virtual void  ClearList();
    virtual int   DirectoryList(uint attrs, LPCTSTR fileSpec);
    int           DirectoryList(uint attrs, const tstring& fileSpec) {return DirectoryList(attrs, fileSpec.c_str());}
    /// @}

    /// \name Operations on individual list box items
    /// @{
    virtual int   AddString(LPCTSTR str);
    int           AddString(const tstring& str) {return AddString(str.c_str());}
    virtual int   InsertString(LPCTSTR str, int index);
    int           InsertString(const tstring& str, int index) {return InsertString(str.c_str(), index);}
    virtual int   DeleteString(int index);
    /// @}

    /// \name Single selection list boxes only (combos overload these)
    /// @{
    virtual int   GetSelIndex() const;
    virtual int   SetSelIndex(int index);
    virtual int   GetSelString(LPTSTR str, int maxChars) const;
    tstring       GetSelString() const;
    virtual int   SetSelString(LPCTSTR str, int startIndex = -1);
    int           SetSelString(const tstring& str, int startIndex = -1) {return SetSelString(str.c_str(), startIndex);}
    /// @}

    /// \name Multiple selection list boxes only
    /// @{
    int           GetSelCount() const;
    int           GetSelStrings(LPTSTR* strs, int maxCount, int maxChars) const;
    TStringArray  GetSelStrings () const;
    int           SetSelStrings(LPCTSTR* prefixes, int numSelections, bool shouldSet);
    int           SetSelStrings(const TStringArray& prefixes, bool shouldSet);
    int           GetSelIndexes(int* indexes, int maxCount) const;
    TIntArray     GetSelIndexes () const;
    int           SetSelIndexes(int* indexes, int numSelections, bool shouldSet);
    int           SetSelIndexes(const TIntArray&, bool shouldSet);
    bool          GetSel(int index) const;
    int           SetSel(int index, bool select);
    int           SetSelItemRange(bool select, int first, int last);
    
    int           GetAnchorIndex() const;
    int           SetAnchorIndex(int index);
    /// @}

    uint32        ItemFromPoint(TPoint& p);
    int           InitStorage(int numItemsToAdd, uint32 memoryToAllocate);

    // Safe overload
    //
    void Transfer(TListBoxData& data, TTransferDirection direction) {Transfer(&data, direction);}

  protected:

    // Override TWindow virtual member functions
    //
    virtual uint Transfer(void* buffer, TTransferDirection direction);
    virtual TGetClassNameReturnType GetClassName();

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TListBox(const TListBox&);
    TListBox& operator =(const TListBox&);

  //DECLARE_STREAMABLE(_OWLCLASS, owl::TListBox, 1);
  DECLARE_STREAMABLE_OWL(TListBox, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TListBox );

//
// List box notification macros. methods are: void method()
//
// EV_LBN_DBLCLK(id, method)
// EV_LBN_ERRSPACE(id, method)
// EV_LBN_KILLFOCUS(id, method)
// EV_LBN_SELCANCEL(id, method)
// EV_LBN_SELCHANGE(id, method)
// EV_LBN_SETFOCUS(id, method)

//
/// \class TListBoxData
// ~~~~~ ~~~~~~~~~~~~
/// Used to transfer the contents of a list box.
class _OWLCLASS TListBoxData {
  public:
    TListBoxData();
   ~TListBoxData();

    TStringArray&   GetStrings();
    TLParamArray& GetItemDatas();
    TIntArray&      GetSelIndices();

    void    AddString(LPCTSTR str, bool isSelected = false);
    void    AddString(const tstring& str, bool isSelected = false) {AddString(str.c_str(), isSelected);}
    void    AddStringItem(LPCTSTR str, LPARAM itemData, bool isSelected = false);
    void    AddStringItem(const tstring& str, LPARAM itemData, bool isSelected = false) {AddStringItem(str.c_str(), itemData, isSelected);}
    void    Clear();

    void    Select(int index);
    void    SelectString(LPCTSTR str);
    void    SelectString(const tstring& str) {SelectString(str.c_str());}
    int     GetSelCount() const;
    void    ResetSelections();
    int     GetSelStringLength(int index = 0) const;
    void    GetSelString(LPTSTR buffer, int bufferSize, int index = 0) const;
    void    GetSelString(tstring& str, int index = 0) const;
    tstring GetSelString(int index = 0) const;

  protected_data:
    TStringArray*  Strings;     ///< Contains all strings in listbox
    TLParamArray* ItemDatas;   ///< Contains all custom data associated with the items in the listbox
    TIntArray*     SelIndices;  ///< Contains all selection indices
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

// --------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns the index of the first item displayed at the top of the list box.
inline int TListBox::GetTopIndex() const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TListBox*,this)->SendMessage(LB_GETTOPINDEX);
}

//
/// The system scrolls the list box contents so that either the specified item
/// appears at the top of the list box or the maximum scroll range has been reached. 
inline int TListBox::SetTopIndex(int index) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(LB_SETTOPINDEX, index);
}

//
/// Sets tab stops. numTabs is the number of tab stops. tabs is the array of
/// integers representing the tab positions.
inline bool TListBox::SetTabStops(int numTabs, int * tabs) {
  PRECONDITION(GetHandle());
  return (bool)SendMessage(LB_SETTABSTOPS, numTabs, TParam2(tabs));
}

//
/// Returns the number of pixels by which the list box can be scrolled horizontally.
inline int TListBox::GetHorizontalExtent() const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TListBox*,this)->SendMessage(LB_GETHORIZONTALEXTENT);
}

//
/// Sets the number of pixels by which the list box can be scrolled horizontally.
inline void TListBox::SetHorizontalExtent(int horzExtent) {
  PRECONDITION(GetHandle());
  SendMessage(LB_SETHORIZONTALEXTENT, horzExtent);
}

//
/// Sets the width in pixels of the items in the list box.
inline void TListBox::SetColumnWidth(int width) {
  PRECONDITION(GetHandle());
  SendMessage(LB_SETCOLUMNWIDTH, width);
}

//
/// Returns the index of the currently focused list-box item. For single-selection
/// list boxes, the return value is the index of the selected item, if one is
/// selected.
inline int TListBox::GetCaretIndex() const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TListBox*,this)->SendMessage(LB_GETCARETINDEX);
}

//
/// Sets the focus to the item specified at index. If partScrollOk is false, the
/// item is scrolled until it is fully visible; if partScrollOk is true, the item is
/// scrolled until it is at least partially visible. 
inline int TListBox::SetCaretIndex(int index, bool partScrollOk) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(LB_SETCARETINDEX, index, MkParam2(partScrollOk,0));
}

//
/// Returns the custom value associated with the item at index set by SetItemData.
//
inline LPARAM TListBox::GetItemData(int index) const {
  PRECONDITION(GetHandle());
  return static_cast<LPARAM>(const_cast<TListBox*>(this)->SendMessage(LB_GETITEMDATA, index));
}

//
/// Sets the custom value associated with the list box item at the specified index position.
//
inline int TListBox::SetItemData(int index, LPARAM itemData) {
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(LB_SETITEMDATA, index, itemData));
}

//
/// Returns the dimensions of the rectangle that surrounds a list-box item currently
/// displayed in the list-box window.
inline int TListBox::GetItemRect(int index, TRect& rect) const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TListBox*,this)->
           SendMessage(LB_GETITEMRECT, index, TParam2((TRect *)&rect));
}

//
/// Returns the height in pixels of the specified list box item.
inline int TListBox::GetItemHeight(int index) const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TListBox*,this)->
           SendMessage(LB_GETITEMHEIGHT, index);
}

//
/// Sets the height in pixels of the items in the list box. If the index or height
/// is invalid, the return value is LB_ERR.
inline int TListBox::SetItemHeight(int index, int height) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(LB_SETITEMHEIGHT, index, MkParam2(height, 0));
}

//
/// Adds a list of file names to a list box.
/// 
/// fileSpec points to the null-terminated string that specifies the filename to add
/// to the list. If the filename contains wildcards (for example, *.*), all files
/// that match the wildcards and have the attributes specified by the uAttrs
/// parameter are added to the list.
/// 
/// attrs can be any combination of:
/// - \c \b  DDL_ARCHIVE	Includes archived files.
/// - \c \b  DDL_DIRECTORY	Includes subdirectories. Subdirectory names are enclosed in square
/// brackets ([ ]).
/// - \c \b  DDL_DRIVES		Includes drives. Drives are listed in the form [-x-], where x is
/// the drive letter.
/// - \c \b  DDL_EXCLUSIVE	Includes only files with the specified attributes. By default,
/// read-write files are listed even if DDL_READWRITE is not specified.
/// - \c \b  DDL_HIDDEN		Includes hidden files.
/// - \c \b  DDL_READONLY	Includes read-only files.
/// - \c \b  DDL_READWRITE	Includes read-write files with no additional attributes.
/// - \c \b  DDL_SYSTEM		Includes system files.
//
inline int TListBox::DirectoryList(uint attrs, LPCTSTR fileSpec) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(LB_DIR, attrs, TParam2(fileSpec));
}

//
/// Adds str to the list box, returning its position in the list (0 is the first
/// position). Returns a negative value if an error occurs. The list items are
/// automatically sorted unless the style LBS_SORT is not used for list box
/// creation.
//
inline int TListBox::AddString(LPCTSTR str) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(LB_ADDSTRING, 0, TParam2(str));
}

//
/// Inserts str in the list box at the position supplied in index, and returns the
/// item's actual position (starting at 0) in the list. A negative value is returned
/// if an error occurs. The list is not resorted. If index is -1, the string is
/// appended to the end of the list.
//
inline int TListBox::InsertString(LPCTSTR str, int index) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(LB_INSERTSTRING, index, TParam2(str));
}

//
/// Deletes the item in the list at the position (starting at 0) supplied in index.
/// DeleteString returns the number of remaining list items, or a negative value if
/// an error occurs.
//
inline int TListBox::DeleteString(int index) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(LB_DELETESTRING, index);
}

//
/// Clears all the entries in the associated listbox
//
inline void TListBox::ClearList() {
  PRECONDITION(GetHandle());
  SendMessage(LB_RESETCONTENT);
}

//
/// Returns the number of items in the list box, or a negative value if an error
/// occurs.
//
//
inline int TListBox::GetCount() const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TListBox*,this)->SendMessage(LB_GETCOUNT);
}

//
/// Returns the index of the first string in the associated listbox which
/// begins with the passed string
//
/// Searches for a match beginning at the passed startIndex. If a match is
/// not found after the last string has been compared, the search continues
/// from the beginning of the list until a match is found or until the list
/// has been completely traversed
///
/// Searches from beginning of list when -1 is passed as the startIndex.
///
/// Returns the index of the selected string, a negative if an error occurs.
//
inline int TListBox::FindString(LPCTSTR find, int startIndex) const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TListBox*,this)->
           SendMessage(LB_FINDSTRING, startIndex, TParam2(find));
}

//
/// Selects the item that matches the given string exactly.
/// DLN added
//
inline int TListBox::SetSelStringExact(LPCTSTR find, int startIndex) {
  PRECONDITION(GetHandle());
  int index = static_cast<int>(SendMessage(LB_FINDSTRINGEXACT, startIndex, (TParam2)find));
  return SetSelIndex(index);
}  

//
/// Return the index of the first string in the list part 
/// which is exactly same as the passed string.
//
/// Search for a exact match  at the passed startIndex. If a match is not
/// found after the last string has been compared, the search continues from
/// the beginning of the list until a match is found or until the list has been
/// completely traversed.
//
/// Search from beginning of list when -1 is passed as the startIndex
//
/// Return the index of the selected string.  A negative value is returned if an
/// error occurs
//
/// \note If you create the list box with an owner-drawn style but without the
/// LBS_HASSTRINGS style, what FindStringExact does depends on whether your
/// application uses the LBS_SORT style. If you use the LBS_SORT style,
/// WM_COMPAREITEM messages are sent to the owner of the combo box to determine
/// which item matches the specified string. If you do not use the LBS_SORT style,
/// FindStringExact searches for a list item that matches the value of the find
/// parameter.
//
inline int TListBox::FindStringExact(LPCTSTR find, int startIndex) const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TListBox*,this)->
           SendMessage(LB_FINDSTRINGEXACT, startIndex,TParam2(find));
}

//
/// Retrieves the contents of the string at the passed index of the
/// associated listbox. Returns the length of the string (in bytes
/// excluding the terminating 0),  a negative if the passed index is not valid
///
/// \note The buffer must be large enough for the string and the terminating 0
//
inline int TListBox::GetString(LPTSTR str, int index) const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TListBox*,this)->
           SendMessage(LB_GETTEXT, index, TParam2(str));
}


//
/// Returns the string length (excluding the terminating NULL) of the item at the
/// position index supplied in Index. Returns a negative value in the case of an
/// error.
//
inline int TListBox::GetStringLen(int index) const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TListBox*,this)->SendMessage(LB_GETTEXTLEN, index);
}


//
/// Returns the selection state of the list-box item at location index. Returns true
/// if the list-box item is selected, false if not selected.
//
inline bool TListBox::GetSel(int index) const {
  PRECONDITION(GetHandle());
  return (bool)CONST_CAST(TListBox*,this)->SendMessage(LB_GETSEL, index);
}

//
/// For multiple-selection list boxes. Selects or deselects an item at the position specified in
/// index or all items if index is -1.
//
inline int TListBox::SetSel(int index, bool select) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(LB_SETSEL, select, MkParam2(index, 0));
}

//
/// Selects the range of items specified from first to last.
//
inline int TListBox::SetSelItemRange(bool select, int first, int last) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(LB_SELITEMRANGE, select, MkParam2(first, last));
}

//
/// Retrieve the zero-based index of the item nearest the specified
/// point in a list box.
inline uint32 TListBox::ItemFromPoint(TPoint& p)
{
  PRECONDITION(GetHandle());
  return (uint32)SendMessage(LB_ITEMFROMPOINT, 0, MkParam2(p.x, p.y));
}
//
/// Allocates memory for storing list box items. An application sends
/// this message before adding a large number of items to a list box.
/// 
/// numItemsToAdd specifies the number of items to add. Windows 95: The wParam
/// parameter is limited to 16-bit values. This means list boxes cannot contain more
/// than 32,767 items. Although the number of items is restricted, the total size in
/// bytes of the items in a listbox is limited only by available memory.
/// 
/// memoryToAllocate specifies the amount of memory, in bytes, to allocate for item
/// strings.
/// 
/// The return value is the maximum number of items that the memory object can store
/// before another memory reallocation is needed, if successful. It is LB_ERRSPACE
/// if not enough memory is available.
/// 
/// \note LB_INITSTORAGE Remarks:
/// - Windows 95:	This message helps speed up the initialization of list boxes that
/// have a large number of items (more than 100). It reserves the specified amount
/// of memory so that subsequent LB_ADDSTRING, LB_INSERTSTRING, LB_DIR, and
/// LB_ADDFILE messages take the shortest possible time. You can use estimates for
/// the cItems and cb parameters. If you overestimate, the extra memory is
/// allocated; if you underestimate, the normal allocation is used for items that
/// exceed the requested amount.
/// - Windows NT:	This message is not needed on Windows NT. It does not reserve the
/// specified amount of memory, because available memory is virtually unlimited. The
/// return value is always the value specified in the numItemsToAdd parameter. 
//
inline int TListBox::InitStorage(int numItemsToAdd, uint32 memoryToAllocate)
{
  PRECONDITION(GetHandle());
  return (int)SendMessage(LB_INITSTORAGE, numItemsToAdd, memoryToAllocate);
}

//
/// Retrieve the index of the anchor item -- that is, the item from
/// which a multiple selection starts. A multiple selection spans all items from the
/// anchor item to the caret item.
//
inline int TListBox::GetAnchorIndex() const
{
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TListBox*,this)->SendMessage(LB_GETANCHORINDEX, 0, 0);
}

//
/// Set the anchor item -- that is, the item from which a multiple
/// selection starts. A multiple selection spans all items from the anchor item to
/// the caret item. Returns 0 on success; LB_ERR on failure.
inline int TListBox::SetAnchorIndex(int index)
{
  PRECONDITION(GetHandle());
  return (int)SendMessage(LB_SETANCHORINDEX, index, 0);
}

//
/// Returns a reference to a TStringArray object containing the strings.
inline TStringArray& TListBoxData::GetStrings() {
  return *Strings;
}

//
/// Returns a reference to the ItemDatas array.
inline TLParamArray& TListBoxData::GetItemDatas() {
  return *ItemDatas;
}

//
/// Returns a reference to the SelIndices array.
inline TIntArray& TListBoxData::GetSelIndices() {
  return *SelIndices;
}

} // OWL namespace


#endif  // OWL_LISTBOX_H
