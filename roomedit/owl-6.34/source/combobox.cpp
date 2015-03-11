//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TComboBox & TComboBoxData.
//----------------------------------------------------------------------------

#include <owl/pch.h>

#include <owl/combobox.h>

#if defined(__BORLANDC__) && __BORLANDC__ < 0x600
# pragma option -w-inl // Disable warning in standard library.
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlControl);


//
/// Constructs a TComboBoxData object, initializes Strings and ItemDatas to empty
/// arrays, and initializes Selection and SelIndex to 0.
//
TComboBoxData::TComboBoxData()
{
  Strings = new TStringArray;
  ItemDatas = new TLParamArray;
  SelIndex = 0;
}

//
/// Destructor for TComboBoxData. Deletes Strings, ItemDatas, and Selection.
//
TComboBoxData::~TComboBoxData()
{
  delete Strings;
  delete ItemDatas;
}

///30.05.2007 - Submitted by Frank Rast:    
///TComboBoxData needs a copy constructor because the
///default copy constructor does not deep copy the
///protected data of this class. For the same reason a
///assignment operator is needed.
TComboBoxData::TComboBoxData(const TComboBoxData& tCBD)
{
  Strings = new TStringArray;
  ItemDatas = new TLParamArray;
  SelIndex = 0;
	*this = tCBD;
}

TComboBoxData &TComboBoxData::operator=(const TComboBoxData& tCBD)
{
	*Strings = *tCBD.Strings;
	*ItemDatas = *tCBD.ItemDatas;
	Selection = tCBD.Selection;
  SelIndex = tCBD.SelIndex;
	return *this;
}



//
/// Flushes the Strings and ItemDatas members. Resets the index and selected string
/// values.
//
void TComboBoxData::Clear() 
{
  Strings->Flush();
  ItemDatas->Flush();
  ResetSelections();
}


//
/// Adds the specified string to the array of Strings. If IsSelected is true,
/// AddString deletes Selection and copies str into Selection.
//
void
TComboBoxData::AddString(LPCTSTR str, bool isSelected)
{
  Strings->Add(str); // add to end
  if (isSelected)
    Select(Strings->Size()-1);
}


//
/// Adds a given string and uint32 item to the "Strings" and "ItemDatas"
/// array and copies the string into "Selection" if "isSelected" is true
//
void
TComboBoxData::AddStringItem(LPCTSTR str, LPARAM itemData, bool isSelected)
{
  ItemDatas->Add(itemData);
  AddString(str, isSelected);
}


//
/// Selects an item at a given index.
//
void
TComboBoxData::Select(int index)
{
  if (index != CB_ERR) {
    SelIndex = index;
    if (index < (int)Strings->Size())
      Selection = (*Strings)[index];
  }
}


//
/// Selects "str", marking the matching String entry (if any) as selected
//
void
TComboBoxData::SelectString(LPCTSTR str)
{
  int numStrings = Strings->Size();
  SelIndex = CB_ERR;
  for (int i = 0; i < numStrings; i++)
    if (_tcscmp((*Strings)[i].c_str(), str) == 0) {
      SelIndex = i;
      break;
    }
  if (Selection != str)
    Selection = str;
}

//
/// Returns the length of the selection string excluding the terminating 0
//
int
TComboBoxData::GetSelStringLength() const
{
  return static_cast<int>(Selection.length());
}


//
/// Copies the selected string into Buffer. BufferSize includes the terminating 0
//
void
TComboBoxData::GetSelString(tchar * buffer, int bufferSize) const
{
  if (bufferSize > 0) {
    _tcsncpy(buffer, Selection.c_str(), bufferSize-1);
    buffer[bufferSize - 1] = 0;
  }
}


//----------------------------------------------------------------------------

//
// Constructors for a TComboBox object
//
/// Constructs a combo box object with the specified parent window (parent),
/// control ID (Id), position (x, y) relative to the origin of the parent window's
/// client area, width (w), height (h), style (style), and text length (textLimit).
/// 
/// Invokes the TListBox constructor with similar parameters. Then sets Attr.Style
/// as follows:
/// \code
/// Attr.Style = WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | CBS_SORT |
/// CBS_AUTOHSCROLL | WS_VSCROLL | style;
/// \endcode
/// One of the following combo box style constants must be among the styles set in
/// style: CBS_SIMPLE, CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_OWNERDRAWFIXED, or
/// CBS_OWNERDRAWVARIABLE.
///
/// By default, an MS-Windows combobox associated with the TComboBox will have
/// a vertical scrollbar and will maintain its entries in alphabetical order
//
TComboBox::TComboBox(TWindow*        parent,
                     int             id,
                     int x, int y, int w, int h,
                     uint32          style,
                     uint            textLimit,
                     TModule*        module)
:
  TListBox(parent, id, x, y, w, h, module),
  TextLimit(textLimit)
{
  Attr.Style = WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP |
               CBS_SORT | CBS_AUTOHSCROLL | WS_VSCROLL | style;
  TRACEX(OwlControl, OWL_CDLEVEL, _T("TComboBox constructed @") << (void*)this);
}


//
/// Constructs a default combo box with the given parent window control ID
/// text length.
//
TComboBox::TComboBox(TWindow*   parent,
                     int        resourceId,
                     uint       textLimit,
                     TModule*   module)
:
  TListBox(parent, resourceId, module),
  TextLimit(textLimit)
{
  TRACEX(OwlControl, OWL_CDLEVEL, _T("TComboBox constructed from resource @") << (void*)this);
}

//
/// Constructs a combo box object to encapsulate (alias) an existing control.
//
TComboBox::TComboBox(THandle hWnd, TModule* module)
:
  TListBox(hWnd, module),
  TextLimit(-1) // TODO: Retrieve the current text limit from the control (how?).
{
  TRACEX(OwlControl, OWL_CDLEVEL, _T("TComboBox constructed as an alias @") << (void*)this);
}


//
//
//
TComboBox::~TComboBox()
{
  TRACEX(OwlControl, OWL_CDLEVEL, _T("TComboBox destructed @") << (void*)this);
}


//
/// Selects the first string in the associated list box that begins with the
/// supplied str. If there is no match, SetText sets the contents of the associated
/// edit control to the supplied string and selects it.
//
void
TComboBox::SetText(LPCTSTR str)
{
  // If str is 0, then use empty str
  //
  if (!str)
    str = _T("");

  // If not in listbox, then set the edit/static portion
  //
  if (SetSelStringExact(str, -1) < 0)//DLN was SetSelString, which would use partial matches 
    {
    SetWindowText(str);
    SetEditSel(0, static_cast<int>(::_tcslen(str)));
  }
}


//
/// Sets the text length limit member and associated control
//
void
TComboBox::SetTextLimit(uint textlimit)
{
  TextLimit = textlimit;
  if (GetHandle() && TextLimit != 0)
    SendMessage(CB_LIMITTEXT, TextLimit-1);
}


//
/// Returns, in the supplied reference parameters, the starting and
/// ending positions of the text selected in the associated edit control
//
/// Returns CB_ERR is the combo box has no edit control
//
int
TComboBox::GetEditSel(int& startPos, int& endPos)
{
  TResult  retValue = SendMessage(CB_GETEDITSEL);

  startPos = LoUint16(retValue);
  endPos   = HiUint16(retValue);

  return (int)retValue;
}

//
/// Functional style overload
//
std::pair<int, int> 
TComboBox::GetEditSel() 
{
  std::pair<int, int> r(0,0); 
  GetEditSel(r.first, r.second);
  return r;
}


//
/// Shows or hides the drop down or drop down list combo box depending on the value
/// of show. If show is true, shows the list; if show is false, hides the list.
//
void
TComboBox::ShowList(bool show)
{
  if ((GetWindowLong(GWL_STYLE) & CBS_DROPDOWN) == CBS_DROPDOWN)
    SendMessage(CB_SHOWDROPDOWN, show);
}


//
/// For combo boxes, gets the screen coordinates of the dropped down list box.
//
void 
TComboBox::GetDroppedControlRect(TRect& rect) const 
{
  PRECONDITION(GetHandle());
  CONST_CAST(TComboBox*,this)->
    SendMessage(CB_GETDROPPEDCONTROLRECT, 0, TParam2((TRect *)&rect));

  // BUG suggested by Luigi Bianchi
  int vertRes = TDesktopDC().GetDeviceCaps(VERTRES);
  if(rect.Bottom() >= vertRes) 
    rect.Offset(0, -rect.Height());
}


//
/// Transfers the items and selection of the combo box to or from a transfer buffer
/// if tdSetData or tdGetData, respectively, is passed as the direction. buffer is
/// expected to point to a TComboBoxData structure.
/// 
/// Transfer returns the size of the TComboBoxData structure. To retrieve the size
/// without transferring data, your application must pass tdSizeData as the
/// direction.
//
uint
TComboBox::Transfer(void* buffer, TTransferDirection direction)
{
  if (!buffer) return 0;
  TComboBoxData& data = *static_cast<TComboBoxData*>(buffer);

  if (direction == tdGetData) 
  {
    // Get the strings and associated item data from the combo box.
    //
    data.Clear();
    const int n = GetCount();
    for (int i = 0; i != n; ++i) 
      data.AddStringItem(GetString(i), GetItemData(i), false);

    // Get the selected string from the list by index, or
    // if the combobox has no selection, get the string from the edit part.
    //
    int selIndex = GetSelIndex();
    if (selIndex >= 0)
      data.Select(selIndex);
    else
      data.SelectString(GetText());
  }
  else if (direction == tdSetData) 
  {
    // Fill the combo box with strings and associated item data.
    //
    ClearList();
    TStringArray& s = data.GetStrings();
    TLParamArray& d = data.GetItemDatas();
    const int n = s.GetItemsInContainer();
    const int m = d.GetItemsInContainer();
    for (int i = 0; i != n; ++i)
    {
      int index = AddString(s[i]);
      if (i < m)
        SetItemData(index, d[i]); // Set the data of the item for the inserted string.
    }

    // Set the selection and content of the edit part.
    //
    int selIndex = data.GetSelIndex();
    if (selIndex >= 0)
      SetSelIndex(selIndex);
    else
      SetText(data.GetSelection());
  }
  return sizeof(TComboBoxData);
}


//
/// Returns the name of TComboBox's registration class, "ComboBox."
//
TWindow::TGetClassNameReturnType
TComboBox::GetClassName()
{
  return _T("COMBOBOX");
}

//
/// Limits the amount of text that the user can enter in the combo box's
/// edit control to the value of TextLimit minus 1
//
/// Creates plain TWindow aliases for the children in the combo box so that
/// TWindow can handle kill focus messages for focus support.
//
void
TComboBox::SetupWindow()
{
  TRACEX(OwlControl, 1, _T("TComboBox::SetupWindow() @ ") << (void*)this);

  TListBox::SetupWindow();

  SetTextLimit(TextLimit);

  THandle hWnd = ::GetWindow(GetHandle(), GW_CHILD);
  while (hWnd) {
    if (!GetWindowPtr(hWnd))
      (new TWindow(hWnd))->SetParent(this);
    hWnd = ::GetWindow(hWnd, GW_HWNDNEXT);
  }
}

//
/// Cleanup aliases created in SetupWindow
//
void
TComboBox::CleanupWindow()
{
  HWND hWnd = ::GetWindow(GetHandle(), GW_CHILD);
  while (hWnd) {
    TWindow* wnd = GetWindowPtr(hWnd);
    delete wnd;
    hWnd = ::GetWindow(hWnd, GW_HWNDNEXT);
  }

  // call base class
  //
  TListBox::CleanupWindow();

  TRACEX(OwlControl, 1, _T("TComboBox::CleanupWindow() @ ") << (void*)this);
}


IMPLEMENT_STREAMABLE1(TComboBox, TListBox);

#if !defined(BI_NO_OBJ_STREAMING)

//
/// Reads an instance of TComboBox from the supplied ipstream
//
void*
TComboBox::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TListBox*)GetObject(), is);
  is >> GetObject()->TextLimit;
  return GetObject();
}

//
/// Writes the TComboBox to the supplied opstream
//
void
TComboBox::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TListBox*)GetObject(), os);
  os << GetObject()->TextLimit;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)


} // OWL namespace


