//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
// Portions Copyright (C) 1997, 98 by Dieter Windau from TCheckListBox class
// EMail: dieter.windau@kvwl.de
// Web:   http://www.members.aol.com/SoftEngage
//
// Added Constructors to work with TCheckListItemArray   --  07/16/1998
// Merged with OWL class and revised by Yura Bidus       --  06/17/1998,
//
/// \file
/// Implements TCheckList and TCheckListItem
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/checklst.h>
#include <owl/commctrl.h>
#include <owl/uihelper.h>
#include <string.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlControl);

#define MULTIPLESEL    (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)


#  if defined(BI_COMP_BORLANDC)
#    pragma warn -inl
#  endif
TCheckListData::TCheckListData()
{
  SelIndices = new TIntArray;  // Contains State of every item
}
//-----------------------------------------------------------------------------
TCheckListData::~TCheckListData()
{
  delete SelIndices;
}
#  if defined(BI_COMP_BORLANDC)
#    pragma warn .inl
#  endif
//-----------------------------------------------------------------------------
int
TCheckListData::AddString(const tstring& str, bool isSelected)
{
  int index = Items.Add(new TCheckListItem(str));
  if(isSelected)
    SelIndices->Add(index);
  return index;
}
//-----------------------------------------------------------------------------
int
TCheckListData::AddStringItem(const tstring& str, UINT_PTR itemData, bool isSelected)
{
  TCheckListItem* item = new TCheckListItem(str);
  item->SetData(itemData);
  int index = Items.Add(item);
  if(isSelected)
    SelIndices->Add(index);
  return index;
}
//-----------------------------------------------------------------------------
void
TCheckListData::Clear(bool del)
{
  Items.Flush(del);
  ResetSelections();
}
//-----------------------------------------------------------------------------
void
TCheckListData::Select(int index)
{
  SelIndices->Add(index);
}
//-----------------------------------------------------------------------------
void
TCheckListData::SelectString(const tstring& str)
{
  for (int i = 0; i < static_cast<int>(Items.size()); ++i)
    if (Items[i]->GetText() == str)
    {
      SelIndices->Add(i);
      break;
    }
}
//-----------------------------------------------------------------------------
int
TCheckListData::GetSelCount() const
{
  return SelIndices->size();
}
//-----------------------------------------------------------------------------
void
TCheckListData::ResetSelections()
{
  SelIndices->Flush();
}

//
/// Creates a checklist.
/// \note The constructor does not take ownership of the items. The lifetime and memory of the 
/// items must be handled by the caller, which must ensure the items outlive the checklist.
//
TCheckList::TCheckList(TWindow* parent, int id, int x, int y, int w, int h,
                       TCheckListItem* items, int numItems,
                       TModule* module)
:
  TListBox(parent, id, x, y, w, h, module)
{
  Attr.Style |= LBS_NOTIFY | LBS_OWNERDRAWVARIABLE | LBS_NOINTEGRALHEIGHT;
  Attr.Style &= ~LBS_HASSTRINGS;
  Attr.Style &= ~LBS_SORT;

  for(int i = 0; i < numItems; i++)
    Items.Add(&items[i]);

  EnableTransfer();

  TRACEX(OwlControl, OWL_CDLEVEL, "TCheckList constructed @" << (void*)this);
}

//
/// Creates a checklist.
/// \note The constructor does not take ownership of the items. The lifetime and memory of the 
/// items must be handled by the caller, which must ensure the items outlive the checklist.
//
TCheckList::TCheckList(TWindow* parent, int id, int x, int y, int w, int h,
                       TCheckListArray& items, TModule* module)
:
  TListBox(parent, id, x, y, w, h, module)
{
  Attr.Style |= LBS_NOTIFY | LBS_OWNERDRAWVARIABLE | LBS_NOINTEGRALHEIGHT;
  Attr.Style &= ~LBS_HASSTRINGS;
  Attr.Style &= ~LBS_SORT;

  for(int i = 0; i < static_cast<int>(items.Size()); i++)
    Items.Add(items[i]);

  EnableTransfer();

  TRACEX(OwlControl, OWL_CDLEVEL, "TCheckList constructed @" << (void*)this);
}

//
/// Creates a checklist based on the given resource identifier.
/// \note The constructor does not take ownership of the items. The lifetime and memory of the 
/// items must be handled by the caller, which must ensure the items outlive the checklist.
//
TCheckList::TCheckList(TWindow* parent, int resourceId,
                       TCheckListItem* items, int numItems,
                       TModule* module)
:
  TListBox(parent, resourceId, module)
{
  for(int i = 0; i < numItems; i++)
    Items.Add(&items[i]);

  EnableTransfer();

  TRACEX(OwlControl, OWL_CDLEVEL, "TCheckList constructed from resource @" << (void*)this);
}

//
/// Creates a checklist based on the given resource identifier.
/// \note The constructor does not take ownership of the items. The lifetime and memory of the 
/// items must be handled by the caller, which must ensure the items outlive the checklist.
//
TCheckList::TCheckList(TWindow* parent, int resourceId, TCheckListArray& items,
                       TModule* module)
:
  TListBox(parent, resourceId, module)
{
  for(int i = 0; i < static_cast<int>(items.Size()); i++)
    Items.Add(items[i]);

  EnableTransfer();

  TRACEX(OwlControl, OWL_CDLEVEL, "TCheckList constructed from resource @" << (void*)this);
}

//
/// Destructs the checklist.
/// \note The destructor does not delete the associated TCheckListItem objects. The owner of the
/// checklist is responsible for the lifetime and destruction of the items.
//
TCheckList::~TCheckList()
{
  TRACEX(OwlControl, OWL_CDLEVEL, "TCheckList destructed @" << (void*)this);
}

DEFINE_RESPONSE_TABLE1(TCheckList, TListBox)
  EV_WM_LBUTTONDOWN,
  EV_WM_CHAR,
END_RESPONSE_TABLE;

//
/// Adds the item into the checklist.
//
/// Adds string of item to the list box, returning its position in the list
/// (0 is the first position). Returns a negative value if an error occurs.
/// The list items are automatically sorted unless the style LBS_SORT
/// is not used for list box creation.
/// \note This function does not take ownership of the item. The lifetime and memory of the item
/// must be handled by the caller, which must ensure the item outlives the checklist box.
//
int
TCheckList::AddItem(TCheckListItem* item)
{
   if(item){
    if (GetStyle() & LBS_SORT){
      int i=0;
      while(i < static_cast<int>(Items.Size())){
        if(*Items[i] > *item)
          break;
        i++;
      }
      if(i >= static_cast<int>(Items.Size()))
        i = -1;
      return InsertItem(item, i);
    }
    else{
      if(GetHandle() && AddString(reinterpret_cast<LPTSTR>(item)) < 0)
        return -1;
      return Items.Add(item);
    }
  }
  return -1;
}

//
/// Inserts the item into the checklist box at the given position.
//
/// Inserts string of item in the list box at the position supplied in index,
/// and returns the item's actual position (starting at 0) in the list.
/// A negative value is returned if an error occurs. The list is not resorted.
/// If index is -1, the string is appended to the end of the list.
/// \note This function does not take ownership of the item. The lifetime and memory of the item
/// must be handled by the caller, which must ensure the item outlives the checklist box.
//
int
TCheckList::InsertItem(TCheckListItem* item, int idx)
{
  if(item){
    if(GetHandle() && InsertString(reinterpret_cast<LPCTSTR>(item), idx) < 0)
       return -1;
    if(idx == -1)
      idx = Items.Add(item);
    else
      Items.AddAt(item, idx);
    return idx;
  }
  return -1;
}

//
/// Detaches the item in the list at the given position (starting at 0).
/// Returns the address of the associated TCheckListBoxItem, or nullptr if an error occurs.
/// \note The caller is responsible for deleting the returned item.
//
TCheckListItem*
TCheckList::DetachItem(int idx)
{
  TCheckListItem* item = GetItem(idx);
  if(item){
    Items.DetachItem(item);
    if(GetHandle())
      DeleteString(idx);
     return item;
  }
  return nullptr;
}

//
/// Toggles the checked state when the space key is pressed.
//
void
TCheckList::EvChar(uint key, uint /*repeatCount*/, uint /*flags*/)
{
  TCheckListItem* item = GetItem(GetCaretIndex());

  if (item && item->IsEnabled() && key == _T(' ')) {
    item->Toggle();
    Update();
  }
//  else     /??? Eat char ???????????
//    TListBox::EvChar(key, repeatCount, flags);
}

//
/// Toggles the checked state when the mouse is clicked in the checkbox.
//
void
TCheckList::EvLButtonDown(uint modKeys, const TPoint& point)
{
  TListBox::EvLButtonDown(modKeys, point);
  TCheckListItem* item = GetItem(GetCaretIndex());
  if(item && item->IsEnabled() && point.x < CheckList_BoxWidth){
    item->Toggle();
    Update();
    // Inform parent of change with BN_CLICKED message
    if(GetParentO())
      GetParentO()->HandleMessage(WM_COMMAND,
                                  MkParam2(Attr.Id, BN_CLICKED),
                                  TParam2(GetHandle()));
  }
}

int SortCheckList(const void *a, const void *b)
{
  return _tcscmp((*(TCheckListItem**)a)->Text.c_str(),
                 (*(TCheckListItem**)b)->Text.c_str());
}

static void SortArray(TCheckListItem** items, uint size)
{
  qsort(reinterpret_cast<void*>(items), static_cast<size_t>(size), sizeof(TCheckListItem*), SortCheckList);
}

//
/// Adds the strings into the listbox.
//
void
TCheckList::SetupWindow()
{
  TRACEX(OwlControl, 1, "TCheckList::SetupWindow() @" << (void*)this);

  // Call the base class
  //
  TListBox::SetupWindow();

  // if sorted ListBox
  if ((GetStyle() & LBS_SORT)){
    if(Items.Size())
      SortArray(&Items[0],Items.Size());
     for (uint i = 0; i < Items.Size(); i++)
      InsertString(reinterpret_cast<LPCTSTR>(Items[i]), i);
  }
  else{
     for (uint i = 0; i < Items.Size(); i++)
       AddString(reinterpret_cast<LPCTSTR>(Items[i]));
  }
}

void
TCheckList::ClearList()
{
  TListBox::ClearList();
  Items.Flush();
}

//
/// Refreshes the window.
//
void
TCheckList::Update()
{
  int topIndex = GetTopIndex();
  int selIndex = GetSelIndex();
  SendMessage(WM_SETREDRAW, false);
  Invalidate();
  SetTopIndex(topIndex);
  if (selIndex != LB_ERR)
    SetSelIndex(selIndex);
  SendMessage(WM_SETREDRAW, true);
}

//
/// Repaints the item entirely.
//
void
TCheckList::ODAFocus(DRAWITEMSTRUCT & drawInfo)
{
  PaintItem(drawInfo);
}

//
// Repaints the item entirely.
//
void
TCheckList::ODASelect(DRAWITEMSTRUCT & drawInfo)
{
  PaintItem(drawInfo);
}

//
/// Repaints the item entirely.
//
void
TCheckList::ODADrawEntire(DRAWITEMSTRUCT & drawInfo)
{
  PaintItem(drawInfo);
}

//
/// Paints the item entirely.
//
void
TCheckList::PaintItem(DRAWITEMSTRUCT & drawInfo)
{
  TCheckListItem* item = GetItem(drawInfo.itemID);
  if (item == nullptr)
    return;

  const bool disabled = !item->IsEnabled() || (drawInfo.itemState & ODS_DISABLED);

  // Prepare DC
  //
  TDC dc(drawInfo.hDC);

  // Erase entire line
  //
  TRect rect(drawInfo.rcItem);
  TBrush bkgnd(TColor::SysWindow);
  dc.FillRect(rect, bkgnd);

  // Draw checkbox
  //
  TRect checkboxRect(rect.left+1,rect.top+1,
                     rect.left+CheckList_BoxWidth,rect.bottom-1);

  // Draw checkbox in 3D Windows Style
  //
  uint state;
  if (item->IsIndeterminate())
    state = TUIPart::Button3State|TUIPart::Checked;//TUIPart::Pushed;
  else
    state = TUIPart::ButtonCheck;

  if(item->IsChecked())
    state |= TUIPart::Checked;

  if (disabled)
    state |= TUIPart::Inactive;

  TUIPart().Paint(dc, checkboxRect, TUIPart::uiButton, static_cast<TUIPart::TState>(state));

  // Draw select state with hightlight color
  //
  TRect textRect = rect;
  textRect.left = checkboxRect.right + 2;

  if (disabled)
  {
    dc.SetTextColor(TColor::SysGrayText);
    dc.SetBkColor(TColor::SysWindow);
  }
  else if (drawInfo.itemState & ODS_SELECTED)
  {
    TBrush fillBrush(TColor::SysHighlight);
    dc.FillRect(textRect, fillBrush);
    dc.SetTextColor(TColor::SysHighlightText);
    dc.SetBkColor(TColor::SysHighlight);
  }
  else
  {
    dc.SetTextColor(TColor::SysWindowText);
    dc.SetBkColor(TColor::SysWindow);
  }

  // Draw Text
  //
  textRect.left++;
  PaintText(dc, textRect, item->Text);
  textRect.left--;

  // Draw focus and selected states
  //
  if (drawInfo.itemState & ODS_FOCUS)
    dc.DrawFocusRect(textRect);
}

//
/// Returns the item at the specified index.
//
TCheckListItem*
TCheckList::GetItem(int index)
{
  if(GetHandle()){
    if (index < 0 || index >= GetCount())
      return nullptr;
    return reinterpret_cast<TCheckListItem*>(GetItemData(index));
  }
  else{
    if (index < 0 || index >= static_cast<int>(Items.Size()))
      return nullptr;
    return Items[index];
  }
}

uint
TCheckList::Transfer(void* buffer, TTransferDirection direction)
{
  if (!buffer && direction != tdSizeData) return 0;
  long            style          = GetStyle();
  TCheckListData* checkListData = reinterpret_cast<TCheckListData*>(buffer);

  if (direction == tdGetData){
    // First, clear out Strings array and fill with contents of list box
    //
    checkListData->Clear(false);

    int count = GetCount();
    for(int i =0; i < count; i++)
      checkListData->AddItem(GetItem(i));

    // Update transfer data with new selected item(s)
    //
    checkListData->ResetSelections();

    if (!(style & MULTIPLESEL)) {
      // Single selection
      //
      checkListData->Select(GetSelIndex());
    }
    else {
      // Multiple selection
      //
      int selCount = GetSelCount();
      if (selCount > 0) {
        TAPointer<int> selections(new int[selCount]);

        GetSelIndexes(selections, selCount);

        // Select each item by index
        //
        for (int selIndex = 0; selIndex < selCount; selIndex++)
          checkListData->Select(selections[selIndex]);
      }
    }
  }
  else if (direction == tdSetData){
    ClearList();

    // Add each string, item data and selections in listBoxData to list box
    //
    const int noSelection = -1;
    uint selCount = checkListData->GetSelCount();  // for multi selection
    int  selIndex  = noSelection;               // for single selection
    for (uint i = 0; i < checkListData->ItemCount(); i++){
      // Index may be different from i when the listbox is sorted.
      //
       int index =  AddString(reinterpret_cast<LPCTSTR>(checkListData->GetItem(i)));
      if (style & MULTIPLESEL) {
        for (uint j = 0; j < selCount; j++)
          if (checkListData->GetSelIndices()[j] == static_cast<int>(i)) {
            SetSel(index, true);
            break;
          }
      }
      else {
        if (selCount && static_cast<uint>(checkListData->GetSelIndices()[0]) == i)
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
  return sizeof(TCheckListData);
}

//----------------------------------------------------------------------------

//
/// Sets tab stops.
/// If numTabs > 0 and tabs == 0, the function returns false.
/// \note Doesn't send LB_SETTABSTOP, because the checklist is ownerdrawn.
//
bool
TTabCheckList::SetTabStops(int numTabs, int * tabs)
{
  PRECONDITION(GetStyle() & LBS_USETABSTOPS);

   TabStops.Flush();

  if (numTabs == 0){
    // Set Default Tabstops
    //
    int nBaseUnit = GetAverageCharWidths()/2;
    TabStops.Add(nBaseUnit);
    return true;
  }
  else if (numTabs == 1 && tabs != nullptr){
    // Set all tabs to the same width defined in tabs
    //
    int nBaseUnit = (tabs[0] * GetAverageCharWidths())/4;
    TabStops.Add(nBaseUnit);
    return true;
  }
  else if(numTabs > 1 && tabs != nullptr){
    // Set all tabs to different width defined in tabs
    //
    int nStop, nBaseUnit = GetAverageCharWidths();
    int nPrevStop = 0;
    for(int i=0; i<numTabs; i++){
      nStop = (tabs[i] * nBaseUnit) / 4;
      if(nStop > nPrevStop){
        TabStops.Add(nStop);
        nPrevStop = nStop;
      }
      else
        return false;
    }
    return true;
  }
  else
    return false;
}

void
TTabCheckList::SetupWindow()
{
  // If the listbox has the LBS_USETABSTOPS style, set the default tab stops
  // Can't do this in constuctor, because the window font is not valid
  //
  SetTabStops();
  TCheckList::SetupWindow();
}

void
TTabCheckList::PaintText(TDC& dc, const TRect& textRect, const tstring& text)
{
  dc.TextRect(textRect);
  dc.TabbedTextOut(textRect.TopLeft(), text, -1, TabStops.Size(), &TabStops[0], textRect.left);
}

int
TTabCheckList::GetAverageCharWidths()
{
  LOGFONT logFont = TFont(GetWindowFont()).GetObject();
  int nBaseUnit = logFont.lfWidth;
  if(nBaseUnit == 0)
    nBaseUnit = LoUint16(::GetDialogBaseUnits());
  return nBaseUnit;
}

//----------------------------------------------------------------------------
// TCheckListItem
//

//
/// Toggles the state of the item.
/// If the item has three states, the cycle goes from unchecked -> checked -> indeterminate -> back to unchecked.
/// Otherwise the state toggles between unchecked and checked.
//
void
TCheckListItem::Toggle()
{
  if (!IsEnabled())
    return;
  if (HasThreeStates) {
    if (IsIndeterminate())
      Uncheck();
    else if (IsChecked())
      SetIndeterminate();
    else
      Check();
  }
  else {
   if (IsChecked())
     Uncheck();
   else
     Check();
  }
}

//
/// Checks the item.
//
void
TCheckListItem::Check()
{
  if (IsEnabled())
    State = BF_CHECKED;
}

//
/// Unchecks the item.
//
void
TCheckListItem::Uncheck()
{
  if (IsEnabled())
    State = BF_UNCHECKED;
}

//
/// Makes the item indeterminate.
//
void
TCheckListItem::SetIndeterminate()
{
  if (IsEnabled()){
    State = BF_GRAYED;
    HasThreeStates = true;
  }
}

//
/// Sets the three-state property.
//
void
TCheckListItem::SetThreeStates(bool hasThreeStates)
{
  if (IsEnabled()){
    HasThreeStates = hasThreeStates;
    if (IsIndeterminate() && !hasThreeStates)
      Check();
  }
}

//
/// Returns the text of the item.
//
const tstring&
TCheckListItem::GetText()
{
  return Text;
}

TCheckListItem&
TCheckListItem::operator =(const TCheckListItem& d)
{
  Text = d.Text;
  State = d.State;
  HasThreeStates = d.HasThreeStates;
  Enabled = d.Enabled;
  return *this;
}

bool
TCheckListItem::operator ==(const TCheckListItem& d) const
{
  return (Text==d.Text && State == d.State &&
          HasThreeStates == d.HasThreeStates && Enabled == d.Enabled);
}

} // OWL namespace
//////////////////////////////////////////////
