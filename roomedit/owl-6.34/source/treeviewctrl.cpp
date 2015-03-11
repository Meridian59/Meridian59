//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implements TTreeViewCtrl, TTreeNode, TTvItem
//
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/treeviewctrl.h>
#include <owl/system.h>
#include <tchar.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommCtrl);          // CommonCtrl diagnostic group

namespace
{

  const uint NodeTextCacheSize = _MAX_PATH;

} // namespace


//
/// Construct a new node given the node's text
//
/// Parameters:
/// \arg \c \b tree	A TTreeViewCtrl object to associate with the node.
/// \arg \c \b text	A text label for the node.
///
/// Remarks:
/// - Initializes the TreeView data member to the address of tree.
/// - Calls the SetText function, passing it text. In 32-bit
/// applications, the text is not displayed in the Tree-View control until the the
/// TTreeNode object encapsulates an item (the ItemStruct.hItem data member is
/// valid) and the Tree-View control is updated (the SetItem function is called, for
/// example).
//
TTreeNode::TTreeNode(TTreeViewCtrl& tree, LPCTSTR text)
:
  TreeView(&tree)
{
  SetText(text, false);
}

//
/// Construct a new node given the node's text and image info
///
/// Parameters:
/// \arg \c \b tree	A TTreeViewCtrl object to associate with the node.
/// \arg \c \b text	A text label for the node.
/// \arg \c \b index	The index of the image in the TTreeViewCtrl image list that corresponds to
/// the node's unselected state.
/// \arg selIndex	The index of the image in the TTreeViewCtrl image list that corresponds
/// to the node's selected state.
///
/// Remarks:
/// - Initializes the TreeView data member to the address of tree.
/// - Calls the SetText function, passing it text. The text is not
/// displayed in the Tree-View control until the the TTreeNode object encapsulates
/// an item (the ItemStruct.hItem data member is valid) and the Tree-View control is
/// updated (the SetItem function is called, for example).
/// - Calls the SetImageIndex function, passing it index.
/// - Calls the SetSelectedImageIndex function, passing it selIndex.
//
TTreeNode::TTreeNode(TTreeViewCtrl& tree, LPCTSTR text, int index,
  int selIndex)
:
  TreeView(&tree)
{
  SetText(text, false);
  SetImageIndex(index, false);
  SetSelectedImageIndex(selIndex, false);
}

//
/// Construct based on an item.
///
/// Parameters:
/// \arg \c \b tree	A TTreeViewCtrl object to associate with the node.
/// \arg \c \b item	A TVITEM object representing the item.
///
/// Remarks:
/// - Initializes the TreeView data member to the address of tree.
/// - Initializes the ItemStruct data member to item.
/// - If the node has a valid label (if the TVIF_TEXT flag of item.mask
/// is set), the SetText function is called (it is passed item.pszText).
//
TTreeNode::TTreeNode(TTreeViewCtrl& tree, TVITEM item)
:
  ItemStruct(item),
  TreeView(&tree)
{
  if (item.mask & TVIF_TEXT) {
    SetText(item.pszText);
  }
}

//
/// Parameters:
/// \arg \c \b tree	A TTreeViewCtrl object to associate with the node.
/// \arg \c \b hItem	A handle to a TVITEM object representing the item, a Tree-View Insert
/// constant (32-bit applications), or a THowToInsert value.
/// 
/// Remarks:
/// - Initializes the TreeView data member to the address of tree.
/// - Displays a warning message if hItem is null.
/// - Initializes the ItemStruct.hItem data member to hItem.
/// - Initializes the ItemStruct.mask data member to TVIF_HANDLE.
//
TTreeNode::TTreeNode(TTreeViewCtrl& tw, HTREEITEM hItem)
:
  TreeView(&tw)
{
  WARNX(OwlCommCtrl, !hItem, 0, "Constructed TTreeNode passing a null hItem");
  ItemStruct.hItem = hItem;
  ItemStruct.mask = TVIF_HANDLE;
}

//
/// Copy constructor: create a new node by copying another node
/// Called implicitly by functions that return a TTreeNode by value;
/// otherwise, shouldn't be needed.
///
/// Parameters
/// \arg \c \b other	A TTreeNode object to copy.
//
TTreeNode::TTreeNode(const TTreeNode& other)
{
  CopyNode(other);
}

//
//  Assignment operator
//
TTreeNode& TTreeNode::operator=(const TTreeNode& other)
{
  CopyNode(other);
  return *this;
}

//
//  Reset node to make it a copy of another node
//
TTreeNode&
TTreeNode::CopyNode(const TTreeNode& node)
{
  TreeView    = node.TreeView;
  ItemStruct  = node.ItemStruct;
  FlushCache();

  // can't do "n.CacheText" on a const object (TAPointer prevents it)
  //
  TTreeNode& n = CONST_CAST(TTreeNode&, node);
  if (n.CacheText) {
    CacheText = new tchar[::_tcslen(n.CacheText) + 1];
    ::_tcscpy(CacheText, n.CacheText);
    ItemStruct.pszText    = CacheText;
  }
  return *this;
}

//
/// Construct the node neighboring a given node.  The flag indicates
/// whether to create the next, previous, parent, or first child node.
///
/// Parameters:
/// \arg node	A TTreeNode object having a relationship to the TTreeNode object to be
/// created.
/// \arg flag	A Tree-View Get Next constant (32-bit applications) or a TNextCode value
/// specifying the relationship between node and the TTreeNode object to be created.
///
/// Remarks:
/// - Initializes the TreeView data member to node.TreeView.
/// - Initializes the ItemStruct data member to node.ItemStruct.
/// - In 32-bit applications, determines which item in the Tree-View
/// control to associate with the new TTreeNode object by sending a TVM_GETNEXTITEM
/// message to the control.
//
TTreeNode::TTreeNode(const TTreeNode& tn, uint32 flag)
:
  ItemStruct(tn.ItemStruct),
  TreeView(tn.TreeView)
{
  TreeView->GetNextItem(flag, *this);
}

//
/// Inserts a new sibling item into the Tree-View (TreeView) control. (Adds the item above this item.)
// 
/// Parameters
/// \arg \c \b node	A TTreeNode object encapsulating an item that will be a sibling of the new
/// item.
///
/// Return Value
/// Returns a TTreeNode object encapsulating the new item. Returns NULL upon
/// failure. (In 32-bit applications, you can check for a NULL handle using the
/// HTREEITEM() operator.)
//
TTreeNode
TTreeNode::AddSibling(const TTvItem& item) const
{
  return InsertItem(item);
}


TTreeNode
TTreeNode::AddSibling(const TTreeNode& node) const
{
   return InsertItem( node );
}

//
// Inserts a child before the passed item.
//
// If the function fails, the handle in object returned is NULL.  Here's how
// to check for errors:
//
//    node = InsertChild(TTvItem("node text"), Last);
//    if (!node)                // the HTREEITEM() conversion operator kicks in
//      DoErrorHandling();
//
// Incidentally, the node returned has only the TVIF_HANDLE mask bit
// set, even if the item passed in contained more attributes.  All the
// attributes are correctly sent to the control, but only a handle is
// returned.
//
TTreeNode
TTreeNode::InsertChild(const TTvItem& item, THowToInsert how) const
{
  TV_INSERTSTRUCT tvis;
  tvis.hParent      = *this;
  tvis.hInsertAfter = (HTREEITEM)how;
  tvis.item         = (TVITEM&)item;
  HTREEITEM hItem = TreeView->InsertItem(&tvis);
  return TTreeNode(*TreeView, hItem);
}

//
// Inserts an item before this item.
//
TTreeNode
TTreeNode::InsertItem(const TTvItem& item) const
{
  TV_INSERTSTRUCT tvis;
  TTreeNode parent = GetParent();

  tvis.hParent      = parent;
  tvis.hInsertAfter = *this;
  tvis.item         = (TVITEM&)item;
  HTREEITEM hItem = TreeView->InsertItem(&tvis);
  return TTreeNode(*TreeView, hItem);
}

//
// Create a temporary structure to store additional information for the
// comparison object.
//
struct TreeCompareThunk {
  const TTvComparator* This;
  LPARAM ItemData;
};

//
//
//
int CALLBACK
OwlTreeWindCompare(LPARAM itemData1, LPARAM itemData2, LPARAM lParam)
{
  TreeCompareThunk* ct = reinterpret_cast<TreeCompareThunk*>(lParam);
  return ct->This->Compare(itemData1, itemData2, ct->ItemData);
}

//
// Recursively sort the children of the nodes.
//
bool
TTreeNode::SortChildren(const TTvComparator& comparator, bool recurse, LPARAM lParam)
{
  TreeCompareThunk ct;
  ct.This = &comparator;
  ct.ItemData = lParam;
  return TreeView->SortChildren(OwlTreeWindCompare, *this, recurse, reinterpret_cast<LPARAM>(&ct));
}

//
//
//
bool
TTreeNode::GetState(uint& state, bool getNew)
{
  PRECONDITION(ItemStruct.hItem || !getNew);
  PRECONDITION(TreeView || !getNew);

  // Send a request message to the control if a) the user asked for the
  // message to be sent, or b) the ItemStruct doesn't yet contain
  // the state data.
  //
  if(getNew || (ItemStruct.mask & TVIF_STATE)==0){
    ItemStruct.mask |= TVIF_STATE;
    if (!GetItem()) {
      ItemStruct.mask &= ~TVIF_STATE;
      return false;
    }
  }

  state = ItemStruct.state;
  return true;
}

//
//
//
bool
TTreeNode::SetState(uint state, bool sendNow /*=true*/ )
{
  PRECONDITION(ItemStruct.hItem || !sendNow);
  PRECONDITION(TreeView || !sendNow);

  ItemStruct.state = state;
  ItemStruct.stateMask |= state;
  ItemStruct.mask |= TVIF_STATE;

  if (sendNow)
    if (!SetItem())
      return false;

  return true;
}

//
// Set the node's text.
// If sendNow is false, the text is merely cached.  It will be
// sent to the control on the next call to SetItem.
//
bool
TTreeNode::SetText(LPCTSTR text, bool sendNow /*=true*/)
{
  PRECONDITION(TreeView || !sendNow);

  if( (uint)ItemStruct.cchTextMax < ::_tcslen(text) + 1 )
     FlushCache();

  if (!CacheText) {
    CacheText = new tchar[::_tcslen(text) + 1];
  }
  size_t length = ::_tcslen(text) + 1;
  _tcsncpy(CacheText, text, length);
  ItemStruct.pszText    = CacheText;
  ItemStruct.cchTextMax = length;
  ItemStruct.mask      |= TVIF_TEXT;
  return sendNow ? SetItem() : true;
}

//
// Get the node's text.
//
// The text is copied into the "text" buffer.  The caller is responsible
// for creating and managing this buffer.
//
// If getNew if false, the text is simply retrieved from the cache.
// If true, GetText queries the control for the node's current text.
// If the TTreeNode doesn't yet have any text cached, it always ignores
// getNew and queries the control directly.
//
bool
TTreeNode::GetText(LPTSTR text, uint length, bool getNew /*=false*/)
{
  // Update the cache if necessary
  //
  GetText(getNew);

  if (CacheText) {
    _tcsncpy(text, CacheText, length);
  }
  return CacheText != 0;
}

// Retrieve a pointer to the node's text string.  The node object
// owns the buffer pointed to.  The caller should not delete it.
// If GetText fails, it returns 0.
//
LPCTSTR
TTreeNode::GetText(bool getNew /*=false*/)
{
  PRECONDITION(TreeView || !getNew);

  // Send a request message to the control if a) the user asked for the
  // message to be sent, or b) the ItemStruct hasn't yet received
  // the text data.
  //
  if (getNew || !CacheText || !(ItemStruct.mask & TVIF_TEXT))  {
    FlushCache();
    CacheText             = new tchar[NodeTextCacheSize];
    ItemStruct.mask      |= TVIF_TEXT;
    ItemStruct.pszText    = CacheText;
    ItemStruct.cchTextMax = NodeTextCacheSize;
    if (!GetItem()) {
      ItemStruct.mask &= ~TVIF_TEXT;
      return 0;
    }
  }
  return CacheText;
}

//
//  Set the node's image indexes.
//
bool
TTreeNode::SetImageIndex(int index, bool sendNow /*=true*/)
{
  PRECONDITION(TreeView || !sendNow);
  ItemStruct.mask  |= TVIF_IMAGE;
  ItemStruct.iImage = index;
  return sendNow ? SetItem() : true;
}

//
//
//
bool
TTreeNode::SetSelectedImageIndex(int index, bool sendNow)
{
  PRECONDITION(TreeView || !sendNow);
  ItemStruct.mask |= TVIF_SELECTEDIMAGE;
  ItemStruct.iSelectedImage = index;
  return sendNow ? SetItem() : true;
}

//
// Set and Get the user-defined node data.
//

bool
TTreeNode::SetItemData(LPARAM data, bool sendNow)
{
  PRECONDITION(TreeView || !sendNow);
  ItemStruct.mask  |= TVIF_PARAM;
  ItemStruct.lParam = data;
  return sendNow ? SetItem() : true;
}

//
//
//
bool
TTreeNode::GetItemData(LPARAM& data, bool getNew)
{
  PRECONDITION(ItemStruct.hItem || !getNew);
  PRECONDITION(TreeView || !getNew);

  // Send a request message to the control if a) the user asked for the
  // message to be sent, or b) the ItemStruct doesn't yet contain
  // the lParam data.
  //
  if (getNew || (ItemStruct.mask & TVIF_PARAM)==0) {
    ItemStruct.mask |= TVIF_PARAM;
    if (!GetItem()) {
      ItemStruct.mask &= ~TVIF_PARAM;
      return false;
    }
  }

  data = ItemStruct.lParam;
  return true;
}

//
// Set and Get the node's HasChildren info (TVITEM.cChildren).
//

bool
TTreeNode::GetHasChildren(int& hasChildren, bool getNew)
{
  PRECONDITION(ItemStruct.hItem || !getNew);
  PRECONDITION(TreeView || !getNew);

  // Send a request message to the control if a) the user asked for the
  // message to be sent, or b) the ItemStruct doesn't yet contain
  // the cChildren data.
  //
  if(getNew || (ItemStruct.mask & TVIF_CHILDREN)==0){
    ItemStruct.mask |= TVIF_CHILDREN;
    if(!GetItem()){
      ItemStruct.mask &= ~TVIF_CHILDREN;
      return false;
    }
  }

  hasChildren = ItemStruct.cChildren;
  return true;
}

//
//
//
bool
TTreeNode::SetHasChildren(int count, bool sendNow)
{
  PRECONDITION(TreeView || !sendNow);
  ItemStruct.mask |= TVIF_CHILDREN;
  ItemStruct.cChildren = count;
  return sendNow ? SetItem(&ItemStruct) : true;
}

//
// Return the selected node.
//
TTreeNode
TTreeViewCtrl::GetSelection()
{
  return GetRoot().GetNextItem(TTreeNode::Caret);
}

//
// Return the drop target node.
//
TTreeNode
TTreeViewCtrl::GetDropHilite()
{
  return GetRoot().GetNextItem(TTreeNode::DropHilite);
}

//
// Return the first visible node.
//
TTreeNode
TTreeViewCtrl::GetFirstVisible()
{
  return GetRoot().GetNextItem(TTreeNode::FirstVisible);
}

//
// Retrieve an item's bounding rectangle
//
bool
TTreeNode::GetItemRect(TRect& rect, bool textOnly /*=true*/) const
{
  PRECONDITION(TreeView);
  PRECONDITION(ItemStruct.hItem);
  
  // The control expects to receive the HTREEITEM in the LPARAM
  //

// Expanded by Val Ovechkin 12:50 PM 6/3/98
  void *p = &rect;
  *(REINTERPRET_CAST(HTREEITEM*, p)) = ItemStruct.hItem;

//  *(REINTERPRET_CAST(HTREEITEM*, &rect)) = ItemStruct.hItem;
  
  return TreeView->SendMessage(TVM_GETITEMRECT, TParam1(textOnly), TParam2(&rect));
}

//
/// Sets the node's checked state.
/// Assumes that the tree has the TVS_CHECKBOXES style, or has state images manually implemented
/// in a similar manner to represent checked state.
//
void 
TTreeNode::Check(bool state)
{
  PRECONDITION(TreeView);
  PRECONDITION(ItemStruct.hItem);
  WARNX(OwlCommCtrl, !TreeView->HasStyle(TVS_CHECKBOXES), 1, "Check called for tree without the TVS_CHECKBOXES style.");

  TreeView_SetCheckState(TreeView->GetHandle(), GetHTreeItem(), state);
}

//
/// Retrieves the node's checked state.
/// Assumes that the tree has the TVS_CHECKBOXES style, or has state images manually implemented
/// in a similar manner to represent checked state.
//
bool 
TTreeNode::IsChecked() const
{
  PRECONDITION(TreeView);
  PRECONDITION(ItemStruct.hItem);
  WARNX(OwlCommCtrl, !TreeView->HasStyle(TVS_CHECKBOXES), 1, "IsChecked called for tree without the TVS_CHECKBOXES style.");

  uint state = TreeView_GetCheckState(TreeView->GetHandle(), GetHTreeItem());
  WARNX(OwlCommCtrl, state > 1, 0, "IsChecked: State not in [0, 1].");
  return state == 1;
}

//
// Empty the node's text cache
//
void
TTreeNode::FlushCache()
{
  CacheText = 0;          // CacheText is a smart pointer
}

//
// Delete the item from the control.
//
bool
TTreeNode::Delete()
{
  PRECONDITION(TreeView);
  if (TreeView->Delete(*this)) {
    ItemStruct.hItem = 0;
    ItemStruct.mask &= ~TVIF_HANDLE;
    return true;
  }
  else
    return false;
}

//----------------------------------------------------------------------------
// TTreeViewCtrl

DEFINE_RESPONSE_TABLE1(TTreeViewCtrl, TControl)
  EV_WM_KEYDOWN,
  EV_WM_SYSKEYDOWN,
END_RESPONSE_TABLE;

//
// Dynamically create the window.
//
TTreeViewCtrl::TTreeViewCtrl(TWindow* parent, int id, int x, int y, int w, int h,
                         uint32 style, TModule* module)
:
  TControl(parent, id, 0, x, y, w, h, module)
{
  if (!TCommCtrl::IsAvailable())
    throw TXCommCtrl();

  SetStyle(WS_CHILD | WS_VISIBLE | style);

  uint32 exStyle = GetExStyle();
  SetExStyle(exStyle |= WS_EX_CLIENTEDGE);
}

//
// Create the TTreeViewCtrl object from a resource.
//
TTreeViewCtrl::TTreeViewCtrl(TWindow* parent, int resourceId, TModule* module)
:
  TControl(parent, resourceId, module)
{
  if (!TCommCtrl::IsAvailable())
    throw TXCommCtrl();
}

//
// Destructor
//
TTreeViewCtrl::~TTreeViewCtrl()
{
/*
+ // GWC mods begin
+ // If we do not call DeleteAllItems() here, then the application tries
+ // to delete them at shutdown. That causes a crash because there is no HWND.
+   DeleteAllItems();
+ // GWC mods end
*/
}

//
// Sets the style of the control.
//
void
TTreeViewCtrl::SetStyle(uint32 style)
{
  TWindow::SetStyle(WS_CHILD | WS_VISIBLE | style);
}

//
// Returns true if a particular style is set.
//
bool
TTreeViewCtrl::HasStyle(uint32 style)
{
  return (GetStyle() & style) ? true : false;
}

//
// Returns the common control class name WC_TREEVIEW
//
TWindow::TGetClassNameReturnType
TTreeViewCtrl::GetClassName()
{
  return WC_TREEVIEW;
}

//
// Recursively sort the children nodes.
//
bool
TTreeViewCtrl::SortChildren(PFNTVCOMPARE func, HTREEITEM parent, bool recurse, LPARAM lParam)
{
  TV_SORTCB cb;
  cb.hParent     = parent;
  cb.lpfnCompare = func;
  cb.lParam      = lParam;

  return ToBool(SendMessage(TVM_SORTCHILDRENCB, TParam1(recurse), TParam2(&cb)));
}
//
uint   
TTreeViewCtrl::Transfer(void* /*buffer*/, TTransferDirection /*direction*/)
{
  return 0;
}

//
// Overrides unwanted behaviour; for a tree with the TVS_CHECKBOXES style, the toggle key (space)
// enables the display of checkboxes for tree nodes that explicitly have checkboxes turned off 
// (i.e. have a state image index of 0). We want the the toggle key to be ignored in this case.
//
void 
TTreeViewCtrl::EvKeyDown(uint key, uint repeatCount, uint flags)
{
  if (HasStyle(TVS_CHECKBOXES) && key == VK_SPACE) // Toggle key?
  {
    uint s = 0;
    bool r = GetSelection().GetState(s, false);
    CHECK(r); InUse(r);
    if ((s & TVIS_STATEIMAGEMASK) == 0) // No checkbox?
      return; // Don't process the toggle key for this node.
  }
  TControl::EvKeyDown(key, repeatCount, flags);
}

//
// Overrides unwanted behaviour; for a tree with the TVS_CHECKBOXES style, the control interprets
// the system key Alt+Space as a checkbox toggle command. We want Alt+Space to be ignored by the
// control and processed by the frame window in the normal way (i.e. open the system menu).
//
void 
TTreeViewCtrl::EvSysKeyDown(uint key, uint repeatCount, uint flags)
{
  if (HasStyle(TVS_CHECKBOXES) && key == VK_SPACE) // Alt+Space?
    return; // Don't process this system key as a checkbox toggle request.
  TControl::EvSysKeyDown(key, repeatCount, flags);
}

//
// Private Init function to zero out the data members.
//
void
TTvItem::Init()
{
  memset(this, 0, sizeof(TVITEM));
}

//
// Default constructor.
//
TTvItem::TTvItem()
{
  Init();
}

//
// Initialize based on an existing item.
//
TTvItem::TTvItem(TVITEM item)
{
  Init();
  *(TVITEM*)this = item;
}

//
// Construct using only text.
//
TTvItem::TTvItem(LPCTSTR text, int len)
{
  Init();
  SetText(text, len);
}

//
// Construct based on text, an image index, and a selected index.
//
TTvItem::TTvItem(LPCTSTR text, int index, int selIndex)
{
  Init();
  SetText(text);
  SetImageIndex(index);
  SetSelectedImageIndex(selIndex);
}

//
// Sets the text of the item.
//
void
TTvItem::SetText(LPCTSTR buffer, int size)
{
  mask      |= TVIF_TEXT;
  pszText    = CONST_CAST(LPTSTR, buffer);
  cchTextMax = static_cast<int>((size < 0) ? _tcslen(buffer) + 1 : std::max(::_tcslen(buffer) + 1, static_cast<size_t>(size)));
}

//
// Returns the text of the item.
//
void
TTvItem::GetText(LPTSTR buffer, int size)
{
  if (mask & TVIF_TEXT) {
    _tcsncpy(buffer, pszText, size);
  }
}

//
// Sets the magic cookie for the item.
//
void
TTvItem::SetHTreeItem(HTREEITEM item)
{
  mask |= TVIF_HANDLE;
  hItem = item;
}

//
// Returns the magic cookie of the item.
//
HTREEITEM
TTvItem::GetHTreeitem() const
{
  return (mask & TVIF_HANDLE) ? hItem : 0;
}

//
// Sets the extra data of the item.
//
void
TTvItem::SetItemData(LPARAM data)
{
  mask  |= TVIF_PARAM;
  lParam = data;
}

//
// Sets the image index of the item.
//
void
TTvItem::SetImageIndex(int index)
{
  mask  |= TVIF_IMAGE;
  iImage = index;
}

//
// Sets the selected image index of the item.
//
void
TTvItem::SetSelectedImageIndex(int index)
{
  mask          |= TVIF_SELECTEDIMAGE;
  iSelectedImage = index;
}

//
// Returns the extra data.
//
LPARAM
TTvItem::GetItemData() const
{
  return (mask & TVIF_PARAM) ? lParam : 0;
}


#if !defined(BI_NO_OBJ_STREAMING)

IMPLEMENT_STREAMABLE1(TTreeViewCtrl, TControl);

//
// Reads an instance of TTreeViewCtrl from the passed ipstream
//
void*
TTreeViewCtrl::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TControl*)GetObject(), is);
  return GetObject();
}

//
// Writes the TTreeViewCtrl to the passed opstream
//
void
TTreeViewCtrl::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TControl*)GetObject(), os);
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */

