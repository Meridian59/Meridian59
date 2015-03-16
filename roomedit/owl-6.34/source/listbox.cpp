//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TListBox and TListBoxData.
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/listbox.h>
#include <stdlib.h>
#include <vector>

#if defined(__BORLANDC__) && __BORLANDC__ < 0x600
# pragma option -w-csu // Disable warning in standard library; "Comparison of signed and unsigned".
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;

// Let the compiler know that the following template instances will be defined elsewhere. 
//#pragma option -Jgx


#define MULTIPLESEL    (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)

//
// TListBoxData constructor
/// Constructs Strings and SelStrings. Initializes SelCount to 0.
//
TListBoxData::TListBoxData()
{
  Strings = new TStringArray;     
  ItemDatas = new TLParamArray;   
  SelIndices = new TIntArray;  
}

//
// TListBoxData destructor
/// Deletes the space allocated for Strings and SelStrings.
//
TListBoxData::~TListBoxData()
{
  delete Strings;
  delete ItemDatas;
  delete SelIndices;
}
//
/// Resets the list box by flushing the ItemDatas and Strings arrays and calling
/// ResetSelections.
void TListBoxData::Clear() 
{
  Strings->Flush();
  ItemDatas->Flush();
  ResetSelections();
}
//
/// Returns the number of items in SelIndices.
int TListBoxData::GetSelCount() const 
{
  return SelIndices->Size();
}
//
/// Flushes SelIndices.
void TListBoxData::ResetSelections() 
{
  SelIndices->Flush();
}

//
/// Adds the specified string to Strings. If IsSelected is true, the string is
/// selected.
//
void
TListBoxData::AddString(LPCTSTR str, bool isSelected)
{
  Strings->Add(str);
  if (isSelected)
    Select(Strings->Size()-1);
}

//
/// Adds a string to the Strings array, optionally selects it, and adds itemData to
/// the ItemDatas array.
//
void
TListBoxData::AddStringItem(LPCTSTR str, LPARAM itemData, bool isSelected)
{
  ItemDatas->Add(itemData);
  AddString(str, isSelected);
}

//
/// Selects the string at the given index.
//
void
TListBoxData::Select(int index)
{
  if (index != LB_ERR && SelIndices->Find(index)==(int)NPOS)
    SelIndices->Add(index);
}

//
/// Adds the index of the string matching str to SelIndices.
//
void
TListBoxData::SelectString(LPCTSTR str)
{
  for (uint i = 0; i < Strings->Size(); i++)
    if (_tcscmp((*Strings)[i].c_str(), str) == 0) {
      Select(i);
      break;
    }
}

//
/// Returns the length (excluding the terminating NULL) of the string at the
/// specified index in SelIndices.
//
int
TListBoxData::GetSelStringLength(int index) const
{
  if (index >= 0 && index < GetSelCount())
    return (*Strings)[(*SelIndices)[index]].length();
  return -1;
}

//
/// Locates the string at the specified index in SelIndices and copies it into
/// buffer. bufferSize includes the terminating NULL.
//
void
TListBoxData::GetSelString(tchar * buffer, int bufferSize, int index) const
{
  if (bufferSize > 0) {
    if (index < 0 || index >= GetSelCount())
      *buffer = 0;

    else {
      _tcsncpy(buffer, (*Strings)[(*SelIndices)[index]].c_str(), bufferSize-1);
      buffer[bufferSize - 1] = 0;
    }
  }
}

//
/// Locates the string at the specified index in SelIndices and copies it into str.
//
void
TListBoxData::GetSelString(tstring& str, int index) const
{
  if (index >= 0 && index < GetSelCount())
    str = (*Strings)[(*SelIndices)[index]];
  else
    str = _T("");
}

//----------------------------------------------------------------------------

//
// Constructor for TListBox
//
//
/// Constructs a list box object with the supplied parent window (parent) library ID
/// (module), position (x, y) relative to the origin of the parent window's client
/// area, width (w), and height (h). Invokes a TControl constructor. Adds
/// LBS_STANDARD to the default styles for the list box to provide it with
/// - A border (WS_BORDER)
/// - A vertical scroll bar (WS_VSCROLL)
/// - Automatic alphabetic sorting of list items (LBS_SORT)
/// - Parent window notification upon selection (LBS_NOTIFY)
/// The TListBox member functions that are described as being for single-selection
/// list boxes are inherited by TComboBox and can also be used by combo boxes. Also,
/// these member functions return -1 for multiple-selection list boxes.
//
TListBox::TListBox(TWindow*   parent,
                   int        id,
                   int x, int y, int w, int h,
                   TModule*   module)
:
  TControl(parent, id, 0, x, y, w, h, module)
{
  Attr.Style |= LBS_STANDARD;
  Attr.ExStyle |= WS_EX_CLIENTEDGE;  // Creates 3d sunken inside edge
}

//
/// Constructs a TListBox object to be associated with a list box control of a
/// TDialog object. Invokes the TControl constructor with similar parameters. The
/// module parameter must correspond to a list box resource that you define.
//
TListBox::TListBox(TWindow*   parent,
                   int        resourceId,
                   TModule*   module)
:
  TControl(parent, resourceId, module)
{
}

//
/// Constructs a TListBox object to encapsulate (alias) an existing control.
//
TListBox::TListBox(THandle hWnd, TModule* module)
:
  TControl(hWnd, module)
{
}

//
/// Returns the name of TListBox's registration class, "LISTBOX".
//
TWindow::TGetClassNameReturnType
TListBox::GetClassName()
{
  return _T("LISTBOX");
}

//
//
/// Transfers the items and selection(s) of the list box to or from a transfer
/// buffer if tdSetData or tdGetData, respectively, is passed as the direction.
/// buffer is expected to point to a pointer to a TListBoxData structure.
/// 
/// Transfer returns the size of the TListBoxData structure. To retrieve the size
/// without transferring data, your application must pass tdSizeData as the
/// direction.
//
uint
TListBox::Transfer(void* buffer, TTransferDirection direction)
{
  long           style = GetStyle();
  TListBoxData*  listBoxData = (TListBoxData*)buffer;

  if (direction == tdGetData) {

    // First, clear out Strings array and fill with contents of list box
    //
    listBoxData->Clear();

    // Pre-calculate max string length so that one big buffer can be used
    //
    int  count = GetCount();
    int  maxStrLen = 0;
    int  i;
    for (i = 0; i < count; i++) {
      int  strLen = GetStringLen(i);
      if (strLen > maxStrLen)
        maxStrLen = strLen;
    }

    // Get each string and item data in the listbox & add to listboxdata
    //
    TAPointer<tchar> tmpStr(new tchar[maxStrLen+1]);
    for (i = 0; i < GetCount(); i++) {
      GetString(tmpStr, i);
      listBoxData->AddStringItem(tmpStr, GetItemData(i), false);
    }

    // Update transfer data with new selected item(s)
    //
    listBoxData->ResetSelections();

    if (!(style & MULTIPLESEL)) {
      // Single selection
      //
      listBoxData->Select(GetSelIndex());
    }
    else {
      // Multiple selection
      //
      int  selCount = GetSelCount();
      if (selCount > 0) {
        int*  selections = new int[selCount];

        GetSelIndexes(selections, selCount);

        // Select each item by index
        //
        for (int selIndex = 0; selIndex < selCount; selIndex++)
          listBoxData->Select(selections[selIndex]);

        delete[] selections;
      }
    }
  }
  else if (direction == tdSetData) {
    ClearList();

    // Add each string, item data and selections in listBoxData to list box
    //
    const int noSelection = -1;
    uint  selCount = listBoxData->GetSelCount();  // for multi selection
    int  selIndex = noSelection;                 // for single selection
    for (uint i = 0; i < listBoxData->GetStrings().Size(); i++) {
      // Index may be different from i when the listbox is sorted.
      //
      int index = AddString(listBoxData->GetStrings()[i]);
      if(i < listBoxData->GetItemDatas().Size())
        SetItemData(index, listBoxData->GetItemDatas()[i]);
      if (style & MULTIPLESEL) {
        for (uint j = 0; j < selCount; j++)
          if (listBoxData->GetSelIndices()[j] == (int)i) {
            SetSel(index, true);
            break;
          }
      }
      else {
        if (selCount && (uint)listBoxData->GetSelIndices()[0] == i)
          selIndex = index;
        else
          // Inserted something before item and the item to select has been
          // pushed further down in the list.
          //
          if (selIndex != noSelection && index <= selIndex)
            selIndex++;
      }
    }
    if (selIndex != noSelection && !(style & MULTIPLESEL))
      SetSelIndex(selIndex);
  }

  return sizeof(TListBoxData);
}

#if 0 // Obsoleted by FindStringExact

//
/// Returns the index of the first string in the associated listbox
/// which is the same as the passed string
//
/// Starting at the line number passed in searchIndex, searches the list box for an
/// exact match with the string str. If a match is not found after the last string
/// has been compared, the search continues from the beginning of the list until a
/// match has been found or until the list has been completely traversed. Searches
/// from the beginning of the list when -1 is supplied as searchIndex. Returns the
/// index of the first string found if successful, or a negative value if an error
/// occurs.
/// 
int
TListBox::FindExactString(LPCTSTR findStr, int startIndex) const
{
  bool  found = false;
  int   firstMatch = indexStart = FindString(findStr, startIndex);
  do {
    if (startIndex > -1) {
      TAPointer<tchar> tmpStr = new tchar[GetStringLen(startIndex) + 1];

      GetString(tmpStr, startIndex);

      if (_tcscmp(tmpStr, findStr) == 0)
        found = true;
      else
        startIndex = FindString(findStr, startIndex);
    }
  } while (!found && startIndex != firstMatch);

  return found ? startIndex : -1;
}

#endif

//
/// For single-selection list boxes. Retrieves the currently selected item, putting
/// up to maxChars of it in str. GetSelString returns one of the following: the
/// string length, a negative value if an error occurs, or -1 if no string is
/// selected. For multiple-selection list boxes, it returns -1.
//
/// \note Since the Windows function is not passed a size parameter, we have to
/// allocate a string to hold the largest string (gotten from a query), and
/// copy a part of it
//
int
TListBox::GetSelString(LPTSTR str, int maxChars) const
{
  int  index = GetSelIndex();

  if (index > -1) {
    int  length = GetStringLen(index);

    if (maxChars >= length)
      return GetString(str, index);
    else if(length > 0){
      TAPointer<tchar> tmpStr(new tchar[length + 1]);
      GetString(tmpStr, index);
      _tcsncpy(str, tmpStr, maxChars);
      return maxChars;
    }
  }
  return -1;
}

//
/// Returns the number of selected items in the single- or multiple-selection list
/// box or combo box.
//
int
TListBox::GetSelCount() const
{
  if (!(GetStyle() & MULTIPLESEL))
    return GetSelIndex() < 0 ? 0 : 1;

  // Multiple-selection list box
  //
  return (int)CONST_CAST(TListBox*,this)->SendMessage(LB_GETSELCOUNT);
}

//
/// For multiple-selection list boxes, retrieves the total number of selected items
/// for a multiselection list and copies them into the buffer. The strs parameter is
/// an array of pointers to chars. Each of the pointers to the buffers is of
/// maxChars. maxCount is the size of the array.
///
/// Returns the number of items put into Strings.  -1 is returned if this is
/// not a multiple-selection list box
///
/// \note Since the Windows function is not passed a size parameter, we have to
/// allocate a string to hold the largest string (gotten from a query), and
/// copy a part of it
//
int
TListBox::GetSelStrings(LPTSTR* strs, int maxCount, int maxChars) const
{
  if (!(GetStyle() & MULTIPLESEL))
    return -1;

  int i = GetSelCount();

  if (i < maxCount)
    maxCount = i;

  if (maxCount > 0) {
    int*  selections = new int[maxCount];

    GetSelIndexes(selections, maxCount);

    for (int selIndex = 0; selIndex < maxCount; selIndex++) {
      int  tmpStrLen = GetStringLen(selections[selIndex]);

      if (maxChars >= tmpStrLen)
        GetString(strs[selIndex], selections[selIndex]);

      else if(tmpStrLen > 0) {
        TAPointer<tchar> tmpStr(new tchar[tmpStrLen+1]);
        GetString(tmpStr, selections[selIndex]);
        _tcsncpy(strs[selIndex], tmpStr, maxChars);
      }
    }
    delete[] selections;
  }
  return maxCount;
}

//
/// Container-aware overload
//
TStringArray 
TListBox::GetSelStrings() const
{
  TStringArray s;
  TIntArray selections = GetSelIndexes();
  for (int i = 0; i != (int)selections.size(); ++i)
    s.Add(GetString(selections[i]));
  return s;
}

//
/// For single-selection list boxes. Forces the selection of the first item
/// beginning with the text supplied in str that appears beyond the position
/// (starting at 0) supplied in SearchIndex. If startIndex is -1, the entire list
/// is searched, beginning with the first item. SetSelString returns the position of
/// the newly selected item, or a negative value in the case of an error.
//
int
TListBox::SetSelString(LPCTSTR findStr, int startIndex)
{
  if (!(GetStyle() & MULTIPLESEL))
    return (int)SendMessage(LB_SELECTSTRING, startIndex, TParam2(findStr));
  return -1;
}

//
/// For multiple-selection list boxes, selects the strings in the associated list
/// box that begin with the prefixes specified in the prefixes array. For each
/// string the search begins at the beginning of the list and continues until a
/// match is found or until the list has been completely traversed. If shouldSet is
/// true, the matched strings are selected and highlighted; if shouldSet is false,
/// the highlight is removed from the matched strings and they are no longer
/// selected. Returns the number of strings successfully selected or deselected (-1
/// for single-selection list boxes and combo boxes). If numSelections is less than
/// 0, all strings are selected or deselected, and a negative value is returned on
/// failure.
//
int
TListBox::SetSelStrings(LPCTSTR* strs, int numSelections, bool shouldSet)
{
  if (!(GetStyle() & MULTIPLESEL))
    return -1;

  if (numSelections < 0)
    return SetSel(-1, shouldSet);

  int  successes = 0;
  for (int i = 0; i < numSelections; i++) {
    int  selIndex;
    if ((selIndex = FindString(strs[i], -1)) > -1)
      if (SetSel(selIndex, shouldSet) > -1)
        successes++;
  }
  return successes;
}

// 
/// Container-aware overload
//
int
TListBox::SetSelStrings(const TStringArray& strs, bool shouldSet)
{
  if (!(GetStyle() & MULTIPLESEL))
    return -1;

  int successes = 0;
  for (size_t i = 0; i < strs.size(); i++) 
  {
    int selIndex = FindString(strs[i], -1);
    if (selIndex > -1)
      if (SetSel(selIndex, shouldSet) > -1)
        successes++;
  }
  return successes;
}

//
/// For single-selection list boxes. Returns the non-negative index (starting at 0)
/// of the currently selected item, or a negative value if no item is selected.
///
/// \note A negative value is returned if this is a multiple-selection list box
//
int
TListBox::GetSelIndex() const
{
  if (!(GetStyle() & MULTIPLESEL))
    return (int)CONST_CAST(TListBox*,this)->SendMessage(LB_GETCURSEL);
  return -1;
}

//
/// For multiple-selection list boxes. Fills the indexes array with the indexes of
/// up to maxCount selected strings. Returns the number of items put in indexes (-1
/// for single-selection list boxes and combo boxes).
//
int
TListBox::GetSelIndexes(int* indexes, int maxCount) const
{
  if (!(GetStyle() & MULTIPLESEL))
    return -1;
  return (int)CONST_CAST(TListBox*,this)->SendMessage(LB_GETSELITEMS,
                                                        maxCount,
                                                        TParam2(indexes));
}

//
/// Container-aware overload
//
TIntArray 
TListBox::GetSelIndexes() const
{
  TIntArray selections;
  std::vector<int> buf(GetSelCount());
  int n = GetSelIndexes(&buf[0], buf.size());
  CHECK(n == buf.size());
  for (int i = 0; i != n; ++i)
    selections.Add(buf[i]);
  return selections;
}

//
/// For single-selection list boxes. Forces the selection of the item at the
/// position (starting at 0) supplied in index. If index is -1, the list box is
/// cleared of any selection. SetSelIndex returns a negative number if an error
/// occurs.
//
int
TListBox::SetSelIndex(int index)
{
  if (!(GetStyle() & MULTIPLESEL))
    return (int)SendMessage(LB_SETCURSEL, index);
  return -1;
}

//
/// For multiple-selection list boxes. Selects or deselects the strings in the
/// associated list box at the indexes specified in the Indexes array. If ShouldSet
/// is true, the indexed strings are selected and highlighted; if ShouldSet is false
/// the highlight is removed and they are no longer selected. Returns the number of
/// strings successfully selected or deselected (-1 for single-selection list boxes
/// and combo boxes). If NumSelections is less than 0, all strings are selected or
/// deselected, and a negative value is returned on failure.
//
int
TListBox::SetSelIndexes(int* indexes, int numSelections, bool shouldSet)
{
  int  successes = 0;

  if (!(GetStyle() & MULTIPLESEL))
    return -1;  // including if it's a combobox

  if (numSelections < 0)
    return (int)SendMessage(LB_SETSEL, shouldSet, -1);

  else {
    for (int i = 0; i < numSelections; i++)
      if ((int)SendMessage(LB_SETSEL, shouldSet, indexes[i]) > -1)
        successes++;
  }
  return successes;
}

//
/// Container-aware overload
//
int
TListBox::SetSelIndexes(const TIntArray& indexes, bool shouldSet)
{
  if (!(GetStyle() & MULTIPLESEL))
    return -1;  // including if it's a combobox

  int successes = 0;
  for (size_t i = 0; i < indexes.size(); i++)
    if ((int)SendMessage(LB_SETSEL, shouldSet, indexes[i]) > -1)
      successes++;
  return successes;
}

//
/// For use with CopyText
//
struct TListBoxGetString
{
  const TListBox& listbox;
  int index;
  TListBoxGetString(const TListBox& c, int index_) : listbox(c), index(index_) {}

  int operator()(LPTSTR buf, int buf_size)
  {return listbox.GetString(buf, index);}
};

//
/// String-aware overload
//
tstring TListBox::GetString(int index) const
{
  return CopyText(GetStringLen(index), TListBoxGetString(*this, index));
}

//
/// String-aware overload
//
tstring TListBox::GetSelString() const
{
  int i = GetSelIndex();
  return (i < 0) ? tstring() : GetString(i);
}

tstring TListBoxData::GetSelString(int index) const 
{
	tstring s; 
	GetSelString(s, index); 
	return s;
}



IMPLEMENT_STREAMABLE1(TListBox, TControl);

#if !defined(BI_NO_OBJ_STREAMING)

//
// Reads an instance of TListBox from the supplied ipstream
//
void*
TListBox::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TControl*)GetObject(), is);
  return GetObject();
}

//
// Writes the TListBox to the supplied opstream
//
void
TListBox::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TControl*)GetObject(), os);
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */

