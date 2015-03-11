//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Bidus Yura, All Rights Reserved
//
/// \file
/// Implementation of the TComboBoxEx class
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/combobex.h>

#include <owl/imagelst.h>

#include <tchar.h>

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommCtrl);        // CommonCtrl Diagnostic group


DEFINE_RESPONSE_TABLE1(TComboBoxEx, TComboBox)
  EV_WM_COMPAREITEM,
  EV_WM_DELETEITEM,
  EV_WM_DRAWITEM,
  EV_WM_MEASUREITEM,
END_RESPONSE_TABLE;


//
//
//
TComboBoxEx::TComboBoxEx(TWindow* parent, int id, int x, int y, int w, int h,
                uint32 style, uint textLimit, TModule* module)
:
  TComboBox(parent, id, x, y, w, h,style, textLimit, module),
  ImageList(0),
  ShouldDelete(false)
{
  ModifyExStyle(WS_EX_CLIENTEDGE, 0);  // Creates 3d sunken inside edge

  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionWin95)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_USEREX_CLASSES);
}

//
//

TComboBoxEx::TComboBoxEx(TWindow* parent, int resourceId, uint textLen,
                          TModule* module)
:
  TComboBox(parent, resourceId, textLen, module),
  ImageList(0),
  ShouldDelete(false)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionWin95)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_USEREX_CLASSES);
}

//
/// Constructs an extended combo box object to encapsulate (alias) an existing control.
// 
TComboBoxEx::TComboBoxEx(THandle hWnd, TModule* module)
:
  TComboBox(hWnd, module),
  ImageList(0),
  ShouldDelete(false)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionWin95)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_USEREX_CLASSES);
}

//
//
//
TComboBoxEx::~TComboBoxEx()
{
  if(ShouldDelete)
    delete ImageList;
}

//
/// Return the proper class name.
/// Windows class: WC_COMBOBOXEX is defined in commctrl.h
TWindow::TGetClassNameReturnType
TComboBoxEx::GetClassName()
{
  return WC_COMBOBOXEX;
}

//
//
//
void
TComboBoxEx::SetupWindow()
{
  TComboBox::SetupWindow();
  if(ImageList)
    SendMessage(CBEM_SETIMAGELIST, 0, (LPARAM)(HIMAGELIST)*ImageList);
}

//
//
//
bool
TComboBoxEx::GetItem(TComboBoxExItem& item, INT_PTR index)
{
  PRECONDITION(GetHandle());
  ((tstring&)item.Text()).reserve(MAX_PATH);
  COMBOBOXEXITEM exitem;
  item.InitItem(exitem, MAX_PATH);
  if (index != -1)
    exitem.iItem = index; // Use provided index.
  if(SendMessage(CBEM_GETITEM, 0, TParam2(&exitem)) != 0){
    item.ReFillItem(exitem);
    return true;
  }
  return false;
}

//
/// Functional style overload
//
TComboBoxExItem 
TComboBoxEx::GetItem(INT_PTR index) 
{
  TComboBoxExItem item; 
  GetItem(item, index); 
  return item;
}


//
//
//
int
TComboBoxEx::InsertItem(const TComboBoxExItem& item)
{
  PRECONDITION(GetHandle());
  COMBOBOXEXITEM exitem;
  item.InitItem(exitem);
  return (int)SendMessage(CBEM_INSERTITEM, 0, TParam2(&exitem));
}

//
//
//
bool
TComboBoxEx::SetItem(const TComboBoxExItem& item, INT_PTR index)
{
  PRECONDITION(GetHandle());
  COMBOBOXEXITEM exitem;
  item.InitItem(exitem);
  if (index != -1)
    exitem.iItem = index; // Use provided index.
  return SendMessage(CBEM_SETITEM, 0, TParam2(&exitem)) != 0;
}

//
//
//
void
TComboBoxEx::SetImageList(TImageList* list, TAutoDelete del)
{
  if(GetHandle())
    SendMessage(CBEM_SETIMAGELIST, 0, (LPARAM)(HIMAGELIST)*list);

  if(ShouldDelete)
    delete ImageList;
  ShouldDelete = del == AutoDelete;
  ImageList = list;
}


//
/// Transfers the items and selection of the combo box to or from a transfer buffer
/// if tdSetData or tdGetData, respectively, is passed as the direction. buffer is
/// expected to point to a TComboBoxExData structure.
/// 
/// Transfer returns the size of the TComboBoxExData structure. To retrieve the size
/// without transferring data, your application must pass tdSizeData as the
/// direction.
//
uint
TComboBoxEx::Transfer(void* buffer, TTransferDirection direction)
{
  if (!buffer) return 0;
  TComboBoxExData& data = *static_cast<TComboBoxExData*>(buffer);

  if (direction == tdGetData) 
  {
    // Clear out data and fill with contents of list box part.
    //
    data.Clear();
    const int count = GetCount();
    for(int i = 0; i != count; ++i) 
      data.AddItem(GetItem(i));

    // Get the selected string from the list by index, or if no selection,
    // get the selection string from the edit box.
    //
    int selIndex = GetSelIndex();
    if (selIndex >= 0)
      data.Select(selIndex);
    else
      data.SelectString(GetText());
  }
  else if (direction == tdSetData) 
  {
    // Fill the list box part.
    //
    ClearList();
    const int count = static_cast<int>(data.Size());
    for (int i = 0; i != count; ++i)
      InsertItem(data.GetItem(i));

    // Set the selected item, if any, otherwise set the edit part.
    //
    if (data.GetSelIndex() >= 0)
      SetSelIndex(data.GetSelIndex());
    else
      SetText(data.GetSelection());
  }
  return sizeof(TComboBoxExData);
}


////////////////////////////////////////////////////////////////////////////////
//
//
//
TComboBoxExItem::TComboBoxExItem()
{
  Init();
}

//
//
//
TComboBoxExItem::TComboBoxExItem(const tstring& str, INT_PTR item, int image)
{
  Init();
  Item = item;
  Text = str;
  Image = image;
  Selected = image; // By default use same image for both selected and unselected items
}

TComboBoxExItem::TComboBoxExItem(const tstring& str, INT_PTR item, int image, int selectedImage, int overlayImage, int indent, LPARAM param)
{
  Init();
  Item = item;
  Text = str;
  Image = image;
  Selected = selectedImage; 
  Overlay  = overlayImage;
  Indent   = indent;
  Param    = param;
  
}

//
/// Copies the property data, not the properties themselves.
//
TComboBoxExItem::TComboBoxExItem(const TComboBoxExItem& item)
{
  operator =(item);
}

//
/// Assigns the property data, not the properties themselves.
//
TComboBoxExItem& 
TComboBoxExItem::operator =(const TComboBoxExItem& item)
{
  Mask = item.Mask();
  Item = item.Item();
  Text = item.Text();
  Image = item.Image();
  Selected = item.Selected();
  Overlay = item.Overlay();
  Indent = item.Indent();
  Param = item.Param();
  return *this;
}

//
//
//
void
TComboBoxExItem::InitItem(COMBOBOXEXITEM& item, uint tsize) const
{
  memset(&item,0,sizeof(item));
  item.mask  = Mask();
  item.iItem = Item();
  if(Image() != -1){
    item.mask |= CBEIF_IMAGE;
    item.iImage |= Image();
  }
  if(Indent() != -1){
    item.mask |= CBEIF_INDENT;
    item.iIndent = Indent();
  }
  if(Param() != 0){
    item.mask |= CBEIF_LPARAM;
    item.lParam = Param();
 }
  if(Overlay() != -1){
    item.mask |= CBEIF_OVERLAY;
    item.iOverlay = Overlay();
  }
  if(Selected() != -1){
    item.mask |= CBEIF_SELECTEDIMAGE;
    item.iSelectedImage = Selected();
  }
  if(Text().length() || tsize){
    item.mask |= CBEIF_TEXT;
    item.pszText     = (LPTSTR)Text().c_str();
    item.cchTextMax  = static_cast<int>(Text().length() > tsize ? Text().length() : tsize);
  }
}

//
//
//
void
TComboBoxExItem::ReFillItem(const COMBOBOXEXITEM& item)
{
  Mask = item.mask;
  Item = item.iItem;

  if(item.mask & CBEIF_IMAGE)
    Image = item.iImage;
  else
    Image = -1;

  if(item.mask & CBEIF_INDENT)
    Indent = item.iIndent;
  else
    Indent = -1;

  if(item.mask & CBEIF_LPARAM)
    Param = item.lParam;
  else
    Param = 0;

  if(item.mask & CBEIF_OVERLAY)
    Overlay = item.iOverlay;
  else
    Overlay = -1;

  if(item.mask & CBEIF_SELECTEDIMAGE)
    Selected = item.iSelectedImage;
  else
    Selected = -1;

  if(item.mask & CBEIF_TEXT)
    Text = tstring(item.pszText);
  else
    Text = tstring(_T(""));
}


//
//
//
void
TComboBoxExItem::Init()
{
  Mask      = 0;   //
  Item      = -1; //
  Text      = tstring(_T(""));
  Image      = -1;
  Selected  = -1;
  Overlay    = -1;
  Indent    = -1;
  Param      = 0;
}

//  class TComboBoxExData
//  ----- ---------------
//
TComboBoxExData::TComboBoxExData()
:
  Selection(_T("")),
  SelIndex(CB_ERR)
{
  Items = new TComboBoxExItemArray;
}

//
//
//
TComboBoxExData::~TComboBoxExData()
{
  delete Items;
}

//
//
//
int
TComboBoxExData::DeleteItem(int index)
{
  return Items->Destroy(index);
}

//
//
//
int
TComboBoxExData::AddItem(const TComboBoxExItem& item)
{
  return Items->Add(item);
}

//
//
//
TComboBoxExItem&
TComboBoxExData::GetItem(int index)
{
  PRECONDITION(Items->Size() > (uint)index);
  return (*Items)[index];
}

//
//
//
void
TComboBoxExData::Clear()
{
  Items->Flush();
}

//
//
//
uint
TComboBoxExData::Size()
{
  return Items->Size();
}
//
/// Copies the selected string into Buffer. BufferSize includes the terminating 0
//
void 
TComboBoxExData::GetSelString(LPTSTR buffer, int bufferSize) const
{
  _tcsncpy(buffer, Selection.c_str(), bufferSize);
}
//
/// Selects "str", marking the matching String entry (if any) as selected
//
void TComboBoxExData::SelectString(LPCTSTR str)
{
  int numStrings = Items->Size();
  SelIndex = CB_ERR;
  for (int i = 0; i < numStrings; i++)
    if (_tcscmp((*Items)[i].Text().c_str(), str) == 0) 
    {
      SelIndex = i;
      break;
    }
  if (Selection != str)
    Selection = str;
}


} // OWL namespace
/* ========================================================================== */
