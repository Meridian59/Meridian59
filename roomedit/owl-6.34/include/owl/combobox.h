//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TComboBox and TComboBoxData the base class for all
/// combobox controls.
//----------------------------------------------------------------------------

#if !defined(OWL_COMBOBOX_H)
#define OWL_COMBOBOX_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/listbox.h>
#include <utility>


namespace owl {

class _OWLCLASS TComboBoxData;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \addtogroup ctrl
/// @{
/// \class TComboBox
// ~~~~~ ~~~~~~~~~
/// You can use TComboBox to create a combo box or a combo box control in a parent
/// TWindow, or to facilitate communication between your application and the combo
/// box controls of TDialog. TComboBox objects inherit most of their behavior from
/// TListBox. This class is streamable.
/// 
/// There are three types of combo boxes: simple, drop down, and drop down list.
/// These types are governed by the style constants CBS_SIMPLE, CBS_DROPDOWN, and
/// CBS_DROPDOWNLIST. These constants, supplied to the constructor of a TComboBox,
/// indicate the type of combo box element to create.
//
/// \todo Add support for new Windows Vista messages CB_SETCUEBANNER and CB_GETCUEBANNER
//
class _OWLCLASS TComboBox 
  : public TListBox 
{
  public:
    TComboBox(TWindow* parent, int id, int x, int y, int w, int h, uint32 style, uint textLimit, TModule* module = 0);
    TComboBox(TWindow* parent, int resourceId, uint textLimit = 0, TModule* module = 0);
    TComboBox(THandle hWnd, TModule* module = 0);
   ~TComboBox();

    // For combo box's edit subcontrol
    int           GetTextLen() const;
    int           GetText(LPTSTR str, int maxChars) const;
    tstring    GetText() const {return GetWindowText();}
    void          SetText(LPCTSTR str);
    void          SetText(const tstring& str) {SetText(str.c_str());}

    uint          GetTextLimit();
    void          SetTextLimit(uint textlimit);

    int           GetEditSel(int& startPos, int& endPos);
    std::pair<int, int> GetEditSel();
    int           SetEditSel(int startPos, int endPos);

    void          Clear();

    // For drop down combo boxes
    //
    void          ShowList(bool show = true);
    void          HideList();

    void          GetDroppedControlRect(TRect& Rect) const;
    bool          GetDroppedState() const;
    bool          GetExtendedUI() const;
    int           SetExtendedUI(bool Extended);
    uint          GetDroppedWidth() const;
    int           SetDroppedWidth(uint width);

    // Combo's List box virtual functions
    //
    virtual int   AddString(LPCTSTR str);
    using TListBox::AddString; // Inject string-aware overload.
    virtual int   InsertString(LPCTSTR str, int index);
    using TListBox::InsertString; // Inject string-aware overload.
    virtual int   DeleteString(int index);

    virtual void  ClearList();
    virtual int   DirectoryList(uint attrs, LPCTSTR fileSpec);
    using TListBox::DirectoryList; // Inject string-aware overload.

    virtual int   GetCount() const;
    virtual int   FindString(LPCTSTR find, int startIndex = -1) const;
    using TListBox::FindString; // Inject string-aware overload.
    virtual int   FindStringExact(LPCTSTR find, int startIndex = -1) const;
    using TListBox::FindStringExact; // Inject string-aware overload.
    virtual int   SetSelStringExact(LPCTSTR find, int startIndex = -1); //DLN added
    using TListBox::SetSelStringExact; // Inject string-aware overload.

    virtual int   GetStringLen(int index) const;
    virtual int   GetString(LPTSTR str, int index) const;
    using TListBox::GetString; // Inject string-aware overload.

    virtual int   GetTopIndex() const;
    virtual int   SetTopIndex(int index);
    virtual int   GetHorizontalExtent() const;
    virtual void  SetHorizontalExtent(int horzExtent);

    virtual int   GetSelIndex() const;
    virtual int   SetSelIndex(int index);
    virtual int   SetSelString(LPCTSTR findStr, int startIndex = -1);
    using TListBox::SetSelString; // Inject string-aware overload.
    virtual LPARAM GetItemData(int index) const;
    virtual int   SetItemData(int index, LPARAM itemData);

    int           GetItemHeight(int index) const;
    int           SetItemHeight(int index, int height);

    int           InitStorage(int numItemsToAdd, uint32 memoryToAllocate);

    // Safe overload
    //
    void          Transfer(TComboBoxData& data, TTransferDirection direction) {Transfer(&data, direction);}

  protected:

    // Override TWindow virtual member functions
    //
    virtual uint Transfer(void* buffer, TTransferDirection direction);

  protected:
    // Override TWindow virtual member functions
    //
    virtual TGetClassNameReturnType GetClassName();
    void          SetupWindow();
    void          CleanupWindow();

  public_data:
    uint TextLimit; ///< Maximum length of text in edit subcontrol

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TComboBox(const TComboBox&);
    TComboBox& operator =(const TComboBox&);

  DECLARE_STREAMABLE_OWL(TComboBox, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TComboBox );

//
// combo box notification macros. methods are: void method()
//
// EV_CBN_CLOSEUP(id, method)
// EV_CBN_DBLCLK(id, method)
// EV_CBN_DROPDOWN(id, method)
// EV_CBN_EDITCHANGE(id, method)
// EV_CBN_EDITUPDATE(id, method)
// EV_CBN_ERRSPACE(id, method)
// EV_CBN_KILLFOCUS(id, method)
// EV_CBN_SELCHANGE(id, method)
// EV_CBN_SELENDCANCEL(id, method)
// EV_CBN_SELENDOK(id, method)
// EV_CBN_SETFOCUS(id, method)

//
///  \class TComboBoxData
//  ----- -------------
/// An interface object that represents a transfer buffer for a TComboBox.
//
class _OWLCLASS TComboBoxData {
  public:
    TComboBoxData();
   ~TComboBoxData();

		/// 30.05.2007 - Submitted by Frank Rast:    
    /// TComboBoxData needs a copy constructor because the
    /// default copy constructor does not deep copy the
    /// protected data of this class. For the same reason a
    /// assignment operator is needed.
    TComboBoxData(const TComboBoxData& tCBD);
		TComboBoxData &operator=(const TComboBoxData& tCBD);
   

    TStringArray&  GetStrings();
    TLParamArray& GetItemDatas();
    int            GetSelIndex();
    tstring&    GetSelection();

    void    AddString(LPCTSTR str, bool isSelected = false);
    void    AddString(const tstring& str, bool isSelected = false) {AddString(str.c_str(), isSelected);}
    void AddStringItem(LPCTSTR str, LPARAM itemData, bool isSelected = false);
    void AddStringItem(const tstring& str, LPARAM itemData, bool isSelected = false) {AddStringItem(str.c_str(), itemData, isSelected);}
    void    Clear();

    void    Select(int index);
    void    SelectString(LPCTSTR str);
    void    SelectString(const tstring& str) {SelectString(str.c_str());}
    int     GetSelCount() const;
    void    ResetSelections();
    int     GetSelStringLength() const;
    void    GetSelString(LPTSTR buffer, int bufferSize) const;
    const tstring& GetSelString() const {return Selection;}

		
  protected_data:
    TStringArray*  Strings;				///< Array of class string to transfer to or from a combo box's associated list box.
    TLParamArray* ItemDatas;			///< Array of data associated with the items of the combo box.
    tstring    Selection;			///< The currently selected string to transfer to or from a combo box.
    int            SelIndex;			///< The zero based index to the currently selected string; negative if no string is
																	///< selected.

};
/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns the text length (excluding the terminating zero) of the edit control or
/// static portion of the combo box.
//
inline int TComboBox::GetTextLen() const {
  PRECONDITION(GetHandle());
  return GetWindowTextLength();
}

//
/// Return the limit of new characters that can be entered into the edit
/// control portion of the combobox.
//
inline uint TComboBox::GetTextLimit() {
  return TextLimit;
}

//
/// Hides the drop down list of a drop down or drop down list combo box.
//
inline void TComboBox::HideList() {
  ShowList(false);
}

//
/// Retrieves up to maxChars characters in the edit or static portion of the combo
/// box. The number of characters retrieved is returned
//
inline int TComboBox::GetText(LPTSTR str, int maxChars) const {
  PRECONDITION(GetHandle());
  return GetWindowText(str, maxChars);
}

//
/// Selects characters that are between startPos and endPos in the edit control of
/// the combo box. Returns CB_ERR if the combo box does not have an edit control.
//
inline int TComboBox::SetEditSel(int startPos, int endPos) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(CB_SETEDITSEL, 0, MkParam2(startPos, endPos));
}

//
/// Clear the text
//
inline void TComboBox::Clear() {
  SetText(_T(""));
}

//
/// For drop down combo boxes, determines if a list box is visible.
//
inline bool TComboBox::GetDroppedState() const {
  PRECONDITION(GetHandle());
  return (bool)CONST_CAST(TComboBox*,this)->SendMessage(CB_GETDROPPEDSTATE);
}

//
/// Determines if the combo box has the extended user interface, which differs from
/// the default user interface in the following ways:
/// - Displays the list box if the user clicks the static text field.
/// - Displays the list box if the user presses the Down key.
/// - Disables scrolling in the static text field if the item list is
/// not visible.
/// 
/// Returns true if the combo box has the extended user interface;  otherwise
/// returns false.
//
inline bool TComboBox::GetExtendedUI() const {
  PRECONDITION(GetHandle());
  return (bool)CONST_CAST(TComboBox*,this)->SendMessage(CB_GETEXTENDEDUI);
}

//
/// Specifies whether the combo box uses the extended user interface or the default
/// user interface. A value of true selects the extended user interface; a value of
/// false selects the standard user interface. If the operation succeeds, the return
/// value is CB_OKAY. If an error occurs, it is CB_ERR.
/// 
/// By default, the F4 key opens or closes the list and the DOWN ARROW changes the
/// current selection. In the extended user interface, the F4 key is disabled and
/// the DOWN ARROW key opens the drop-down list. 
//
inline int TComboBox::SetExtendedUI(bool extended) {
  PRECONDITION(GetHandle());
  return (bool)SendMessage(CB_SETEXTENDEDUI, extended);
}

//
/// Returns the minimum allowable width, in pixels, of the list box of a
/// combo box with the CBS_DROPDOWN or CBS_DROPDOWNLIST style on success; otherwise
/// returns CB_ERR.
/// 
/// By default, the minimum allowable width of the drop-down list box is 0. The
/// width of the list box is either the minimum allowable width or the combo box
/// width, whichever is larger.
//
inline uint TComboBox::GetDroppedWidth() const
{
  PRECONDITION(GetHandle());
  return (uint)CONST_CAST(TComboBox*,this)->SendMessage(CB_GETDROPPEDWIDTH);
}

//
/// Sets the maximum allowable width, in pixels, of the list box of a
/// combo box with the CBS_DROPDOWN or CBS_DROPDOWNLIST style. Returns the new width
/// of the box on success; otherwise returns CB_ERR.
/// 
/// By default, the minimum allowable width of the drop-down list box is 0. The
/// width of the list box is either the minimum allowable width or the combo box
/// width, whichever is larger.
//
inline int TComboBox::SetDroppedWidth(uint width)
{
  PRECONDITION(GetHandle());
  return (int)SendMessage(CB_SETDROPPEDWIDTH, width, 0);
}
//
/// Adds a string to an associated list part of a combo box. Returns the index of
/// the string in the list. The first entry is at index zero. Returns a negative
/// value if an error occurs.
//
inline int TComboBox::AddString(LPCTSTR str) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(CB_ADDSTRING, 0, TParam2(str));
}

//
/// Insert a string in list part of the associated combobox at the passed
/// index, returning the index of the string in the list
/// A negative value is returned if an error occurs
//
inline int TComboBox::InsertString(LPCTSTR str, int index) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(CB_INSERTSTRING, index, TParam2(str));
}

//
/// Deletes the string at the passed index in the associated list part of a combo
/// box. Returns a count of the entries remaining in the list or a negative value if
/// an error occurs.
//
inline int TComboBox::DeleteString(int index) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(CB_DELETESTRING, index);
}

//
/// Fills the combo box with file names from a specified directory.
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
inline int TComboBox::DirectoryList(uint attrs, LPCTSTR fileSpec) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(CB_DIR, attrs, TParam2(fileSpec));
}

//
/// Clear all the entries in list part of the associated combobox
//
inline void TComboBox::ClearList() {
  PRECONDITION(GetHandle());
  SendMessage(CB_RESETCONTENT);
}

//
/// Return the number of entries in list part of the associated combobox. A
/// negative value is returned if an error occurs
//
inline int TComboBox::GetCount() const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TComboBox*,this)->SendMessage(CB_GETCOUNT);
}

//
/// Return the index of the first string in list part of the associated
/// combobox which begins with the passed string.
//
/// Search for a match beginning at the passed startIndex. If a match is not
/// found after the last string has been compared, the search continues from
/// the beginning of the list until a match is found or until the list has been
/// completely traversed.
//
/// Search from beginning of list when -1 is passed as the startIndex.
//
/// Return the index of the selected string.  A negative value is returned if an
/// error occurs.
//
inline int TComboBox::FindString(LPCTSTR find, int startIndex) const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TComboBox*,this)->
           SendMessage(CB_FINDSTRING, startIndex,TParam2(find));
}

/// Return the index of the first string in list part of the associated
/// combobox which is exactly same with the passed string.
//
/// Search for a exact match  at the passed SearchIndex. If a match is not
/// found after the last string has been compared, the search continues from
/// the beginning of the list until a match is found or until the list has been
/// completely traversed.
//
/// Search from beginning of list when -1 is passed as the index
//
/// Return the index of the selected string.  A negative value is returned if an
/// error occurs
//
/// \note If you create the combo box with an owner-drawn style but without the
/// CBS_HASSTRINGS style, what FindStringExact does depends on whether your
/// application uses the CBS_SORT style. If you use the CBS_SORT style,
/// WM_COMPAREITEM messages are sent to the owner of the combo box to determine
/// which item matches the specified string. If you do not use the CBS_SORT style,
/// FindStringExact searches for a list item that matches the value of the find
/// parameter.
//
inline int TComboBox::FindStringExact(LPCTSTR find, int indexStart) const{
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TComboBox*,this)->
           SendMessage(CB_FINDSTRINGEXACT, indexStart,TParam2(find));
}

//
/// Retrieve the contents of the string at the passed index of list part of the
/// associated combobox, returning the length of the string (in bytes excluding
/// the terminating 0) as the value of the call
//
/// A negative value is returned if the passed index is not valid
//
/// The buffer must be large enough for the string and the terminating 0
//
inline int TComboBox::GetString(LPTSTR str, int index) const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TComboBox*,this)->
           SendMessage(CB_GETLBTEXT, index, TParam2(str));
}

//
/// Return the length of the string at the passed index in the associated combo
/// list excluding the terminating 0
//
/// A negative value is returned if an error occurs
//
inline int TComboBox::GetStringLen(int index) const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TComboBox*,this)->
           SendMessage(CB_GETLBTEXTLEN, index);
}

//
/// Returns the zero-based index of the first visible item in the list box portion
/// of a combo box. Initially the item with index 0 is at the top of the list box,
/// but if the list box contents have been scrolled, another item may be at the top.
/// CB_ERR is returned on failure.
//
inline int TComboBox::GetTopIndex() const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TComboBox*,this)->SendMessage(CB_GETTOPINDEX);
}

//
/// The system scrolls the list box contents so that either the string specified by
/// index appears at the top of the list box or the maximum scroll range has been
/// reached. On success 0 is returned; on failure CB_ERR is returned.
//
inline int TComboBox::SetTopIndex(int index) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(CB_SETTOPINDEX, index);
}

//
/// Retrieves from a combo box the width, in pixels, by which the list box can be
/// scrolled horizontally (the scrollable width). This is applicable only if the
/// list box has a horizontal scroll bar.
//
inline int TComboBox::GetHorizontalExtent() const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TComboBox*,this)->SendMessage(CB_GETHORIZONTALEXTENT);
}

//
/// Set the width, in pixels, by which a list box can be scrolled horizontally (the
/// scrollable width). If the width of the list box is smaller than this value, the
/// horizontal scroll bar horizontally scrolls items in the list box. If the width
/// of the list box is equal to or greater than this value, the horizontal scroll
/// bar is hidden or, if the combo box has the CBS_DISABLENOSCROLL style, disabled.
//
inline void TComboBox::SetHorizontalExtent(int horzExtent) {
  PRECONDITION(GetHandle());
  SendMessage(CB_SETHORIZONTALEXTENT, horzExtent);
}

//
/// Returns the index of the list selection or a negative value if none exists.
//
inline int TComboBox::GetSelIndex() const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TComboBox*,this)->SendMessage(CB_GETCURSEL);
}

//
/// Selects a string of characters in a combo box. index specifies the index of the
/// string of characters in the list box to select. If the index is 0, the first
/// line in the list box is selected. If the index is -1, the current selection is
/// removed. If an error occurs, a negative value is returned.
//
inline int TComboBox::SetSelIndex(int index) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(CB_SETCURSEL, index);
}

//
/// Selects a string of characters in the associated list box and sets the contents
/// of the associated edit control to the supplied string.
//
inline int TComboBox::SetSelString(LPCTSTR findStr, int startIndex) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(CB_SELECTSTRING, startIndex, TParam2(findStr));
}

//
/// DLN added
//
inline int TComboBox::SetSelStringExact(LPCTSTR find, int startIndex) 
{
  PRECONDITION(GetHandle());
  int index = static_cast<int>(CONST_CAST(TComboBox*,this)->
           SendMessage(CB_FINDSTRINGEXACT, startIndex,(LPARAM)find));
  return SetSelIndex(index);
}


//
/// Returns the 32-bit value associated with the combo box's item.
//
inline LPARAM TComboBox::GetItemData(int index) const {
  PRECONDITION(GetHandle());
  return static_cast<LPARAM>(const_cast<TComboBox*>(this)->SendMessage(CB_GETITEMDATA, index));
}

//
/// Sets the 32-bit value associated with the TComboBox's item. If an error occurs,
/// returns a negative value.
//
inline int TComboBox::SetItemData(int index, LPARAM itemData) {
  PRECONDITION(GetHandle());
  return static_cast<int>(SendMessage(CB_SETITEMDATA, index, itemData));
}

//
/// Returns the height in pixels of the Combo box's list items. If an error occurs,
/// returns a negative value.
//
inline int TComboBox::GetItemHeight(int index) const {
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TComboBox*,this)->
           SendMessage(CB_GETITEMHEIGHT, index);
}

//
/// Sets the height of the list items or the edit control portion in a combo box. If
/// the index or height is invalid, returns a negative value.
//
inline int TComboBox::SetItemHeight(int index, int height) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(CB_SETITEMHEIGHT, index, MkParam2(height, 0));
}

//
/// Allocates memory for storing combo box items. An application sends
/// this message before adding a large number of items to a combo box.
/// 
/// numItemsToAdd specifies the number of items to add. Windows 95: The wParam
/// parameter is limited to 16-bit values. This means combo boxes cannot contain
/// more than 32,767 items. Although the number of items is restricted, the total
/// size in bytes of the items in a combo box is limited only by available memory.
/// 
/// memoryToAllocate specifies the amount of memory, in bytes, to allocate for item
/// strings.
/// 
/// The return value is the maximum number of items that the memory object can store
/// before another memory reallocation is needed, if successful. It is LB_ERRSPACE
/// if not enough memory is available.
/// 
/// LB_INITSTORAGE Remarks:
/// 
/// Windows 95:	This message helps speed up the initialization of list boxes that
/// have a large number of items (more than 100). It reserves the specified amount
/// of memory so that subsequent CB_ADDSTRING, CB_INSERTSTRING, CB_DIR, and
/// CB_ADDFILE messages take the shortest possible time. You can use estimates for
/// the cItems and cb parameters. If you overestimate, the extra memory is
/// allocated; if you underestimate, the normal allocation is used for items that
/// exceed the requested amount.
/// 
/// Windows NT:	This message is not needed on Windows NT. It does not reserve the
/// specified amount of memory, because available memory is virtually unlimited. The
/// return value is always the value specified in the numItemsToAdd parameter. 
//
inline int TComboBox::InitStorage(int numItemsToAdd, uint32 memoryToAllocate) {
  PRECONDITION(GetHandle());
  return (int)SendMessage(CB_INITSTORAGE, numItemsToAdd, memoryToAllocate);
}

//
/// Returns the array of strings (the Strings data member) to transfer to or from a
/// combo box's associated list box.
//
inline TStringArray& TComboBoxData::GetStrings() {
  return *Strings;
}

//
/// Returns the array of DWORDs to transfer to or from a combo box's associated list
/// box.
//
inline TLParamArray& TComboBoxData::GetItemDatas() {
  return *ItemDatas;
}

//
/// Returns the index (the SelIndex data member) of the selected item in the strings
/// array.
//
inline int TComboBoxData::GetSelIndex() {
  return SelIndex;
}

//
/// Returns the currently selected string (the Selection data member) to transfer to
/// or from a combo box.
//
inline tstring& TComboBoxData::GetSelection() {
  return Selection;
}

//
/// Returns the number of items selected, either 0 or 1.
//
inline int TComboBoxData::GetSelCount() const {
  return SelIndex == CB_ERR ? 0 : 1;
}

//
/// Resets the index of the selected item and the currently selected string.
//
inline void TComboBoxData::ResetSelections() {
  SelIndex = CB_ERR;
  Selection = _T("");
}


} // OWL namespace


#endif  // OWL_COMBOBOX_H
