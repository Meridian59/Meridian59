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
TCheckListData::AddStringItem(const tstring& str, uint32 itemData, bool isSelected)
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
/// Constructor to create a window.
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
/// Constructor to create a window.
//
TCheckList::TCheckList(TWindow* parent, int id, int x, int y, int w, int h,
                       TCheckListArray& items, TModule* module)
:
  TListBox(parent, id, x, y, w, h, module)
{
  Attr.Style |= LBS_NOTIFY | LBS_OWNERDRAWVARIABLE | LBS_NOINTEGRALHEIGHT;
  Attr.Style &= ~LBS_HASSTRINGS;
  Attr.Style &= ~LBS_SORT;

  for(int i = 0; i < (int)items.Size(); i++)
    Items.Add(items[i]);

  EnableTransfer();

  TRACEX(OwlControl, OWL_CDLEVEL, "TCheckList constructed @" << (void*)this);
}

//
/// Constructor used for resources
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
/// Constructor used for resources.
//
TCheckList::TCheckList(TWindow* parent, int resourceId, TCheckListArray& items,
                       TModule* module)
:
  TListBox(parent, resourceId, module)
{
  for(int i = 0; i < (int)items.Size(); i++)
    Items.Add(items[i]);
  
  EnableTransfer();

  TRACEX(OwlControl, OWL_CDLEVEL, "TCheckList constructed from resource @" << (void*)this);
}
//
/// Destructor for this class.
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
/// Add item into checklist box.
//
/// Adds string of item to the list box, returning its position in the list
/// (0 is the first position). Returns a negative value if an error occurs.
/// The list items are automatically sorted unless the style LBS_SORT
/// is not used for list box creation.
//
int
TCheckList::AddItem(TCheckListItem* item)
{
   if(item){
    if (GetStyle() & LBS_SORT){
      int i=0;
      while(i < (int)Items.Size()){
        if(*Items[i] > *item)
          break;
        i++;
      }
      if(i >= (int)Items.Size())
        i = -1;
      return InsertItem(item, i);
    }
    else{
      if(GetHandle() && AddString((LPTSTR)item) < 0)
        return -1;
      return Items.Add(item);
    }
  }
  return -1;
}

//
/// Insert item into CheckListBox at position.
//
/// Inserts string of item in the list box at the position supplied in index,
/// and returns the item's actual position (starting at 0) in the list.
/// A negative value is returned if an error occurs. The list is not resorted.
/// If index is -1, the string is appended to the end of the list.
//
int
TCheckList::InsertItem(TCheckListItem* item, int idx)
{
  if(item){
    if(GetHandle() && InsertString((LPCTSTR)item, idx) < 0)
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
/// Deletes the item in the list at the position (starting at 0) supplied in
/// index. DeleteItem returns the adress of the associated TCheckListBoxItem,
/// or ZERO if an error occurs. The item must be deleted by the owner
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
  return 0;
}

//
/// Toggle the "checked" state when the space is pressed.
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
/// Toggle the "checked" state when the mouse is clicked in the checkbox.
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

//
//
//
/*static*/ int SortCheckList(const void *a, const void *b)
{
  return _tcscmp((*(TCheckListItem**)a)->Text.c_str(),
                 (*(TCheckListItem**)b)->Text.c_str());
}

//
//
//
static void SortArray(TCheckListItem** items, uint size)
{
  qsort((void*)items, (size_t)size, sizeof(TCheckListItem*), SortCheckList);
}

//
/// Add the strings into the listbox.
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
      InsertString((LPCTSTR)Items[i], i);
  }
  else{
     for (uint i = 0; i < Items.Size(); i++)
       AddString((LPCTSTR)Items[i]);
  }
}

//
void 
TCheckList::ClearList()
{
  TListBox::ClearList();
  Items.Flush();
}

//
/// Refresh the window.
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
/// Repaint the item entirely.
//
void
TCheckList::ODAFocus(DRAWITEMSTRUCT & drawInfo)
{
  PaintItem(drawInfo);
}

//
// Repaint the item entirely.
//
void
TCheckList::ODASelect(DRAWITEMSTRUCT & drawInfo)
{
  PaintItem(drawInfo);
}

//
/// Repaint the item entirely.
//
void
TCheckList::ODADrawEntire(DRAWITEMSTRUCT & drawInfo)
{
  PaintItem(drawInfo);
}

//
/// Paint the item entirely.
//
void
TCheckList::PaintItem(DRAWITEMSTRUCT & drawInfo)
{
  TCheckListItem* item = GetItem(drawInfo.itemID);
  if (item == 0)
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

  TUIPart().Paint(dc, checkboxRect, TUIPart::uiButton, (TUIPart::TState)state);

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
/// Return the item at the specified index.
//
TCheckListItem*
TCheckList::GetItem(int index)
{
  if(GetHandle()){
    if (index < 0 || index >= GetCount())
      return 0;
    return (TCheckListItem*)GetItemData(index);
  }
  else{
    if (index < 0 || index >= (int)Items.Size())
      return 0;
    return Items[index];
  }
}

//-----------------------------------------------------------------------------
uint TCheckList::Transfer(void* buffer, TTransferDirection direction)
{  
  long            style          = GetStyle();
  TCheckListData* checkListData = (TCheckListData*)buffer;

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
       int index =  AddString((LPCTSTR)checkListData->GetItem(i));
      if (style & MULTIPLESEL) {
        for (uint j = 0; j < selCount; j++)
          if (checkListData->GetSelIndices()[j] == (int)i) {
            SetSel(index, true);
            break;
          }
      }
      else {
        if (selCount && (uint)checkListData->GetSelIndices()[0] == i)
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
//
//

//
/// Don't send the LB_SETTABSTOP, because the Text is ownerdrawn
/// If numTabs > 0 and tabs == 0 the function return false
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
  else if (numTabs == 1 && tabs != 0){
    // Set all tabs to the same width defined in tabs
    //
    int nBaseUnit = (tabs[0] * GetAverageCharWidths())/4;
    TabStops.Add(nBaseUnit);
    return true;
  }
  else if(numTabs > 1 && tabs != 0){
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

//
//
//
void
TTabCheckList::SetupWindow()
{
  // If the listbox has the LBS_USETABSTOPS style, set the default tab stops
  // Can't do this in constuctor, because the window font is not valid
  //
  SetTabStops();
  TCheckList::SetupWindow();
}

//
//
//
void
TTabCheckList::PaintText(TDC& dc, const TRect& textRect, const tstring& text)
{
  dc.ExtTextOut(textRect.TopLeft(), ETO_OPAQUE, &textRect, _T(""), 0, 0);
  dc.TabbedTextOut(textRect.TopLeft(), text, -1, TabStops.Size(), &TabStops[0], textRect.left);
}

//
//
//
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
/// Toggle the state of the item.
/// If the item has three states, the cycle goes from
///   unchecked -> checked -> indeterminate -> back to unchecked.
/// Otherwise the state toggles between
///   unchecked and checked.
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
/// Programmatically check the item.
//
void
TCheckListItem::Check()
{
  if (IsEnabled())
    State = BF_CHECKED;
}

//
/// Programmatically uncheck the item.
//
void
TCheckListItem::Uncheck()
{
  if (IsEnabled())
    State = BF_UNCHECKED;
}

//
/// Programmatically make the item indeterminate.
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
/// Set the three-state property.
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
/// Return the text of the item.
//
const tstring&
TCheckListItem::GetText()
{
  return Text;
}

//
//
//
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

