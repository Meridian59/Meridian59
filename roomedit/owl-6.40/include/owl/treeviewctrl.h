//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Declares TTreeViewCtrl, TTreeNode, and TTvItem
//----------------------------------------------------------------------------

#if !defined(OWL_TREEVIEWCTRL_H)
#define OWL_TREEVIEWCTRL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/commctrl.h>
#include <owl/control.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TTreeViewCtrl;
class _OWLCLASS TTreeNode;
class _OWLCLASS TTvItem;

class _OWLCLASS TTvHitTestInfo;
class _OWLCLASS TTvComparator;

//
/// \class TTvItem
// ~~~~~ ~~~~~~~~~
/// Used to represent the data to be stored in the TTreeViewCtrl.
/// TODO: Make the object hold a copy of the item text. See TLvItem.
//
class _OWLCLASS TTvItem : public TVITEM {
  public:
    TTvItem();               ///< this is called by some TTreeNode constructors
    TTvItem(TVITEM item);
    TTvItem(LPCTSTR, int len = 0);
    TTvItem(LPCTSTR, int index, int selIndex);

    /// Initialize the text
    //
    void SetText(LPCTSTR, int len = -1);
    void GetText(LPTSTR, int len);

    // Set/Get the "magic cookie"
    //
    void      SetHTreeItem(HTREEITEM hItem);
    HTREEITEM GetHTreeitem() const;

    /// Set the imagelist index
    //
    void SetImageIndex(int index);

    /// Set the selected image index
    //
    void SetSelectedImageIndex(int index);

    /// Store additional information
    //
    void SetItemData(LPARAM);
    LPARAM GetItemData() const;

  protected:
    void Init();
};

//
/// \class TTvComparator
// ~~~~~ ~~~~~~~~~~~~~
/// A base class for comparisons to sort items.
//
class _OWLCLASS TTvComparator {
  public:
    /// return value is < 0 if item1 < item2
    ///                 = 0 if item1 == item2
    ///                 > 0 if item1 > item2
    /// lParam is user-defined value, pass to TTreeNode::SortChildren
    //
    virtual int Compare(LPARAM item1, LPARAM item2, LPARAM lParam) const;
};
typedef TTvComparator TTwComparator; // Old alternative name - deprecated.

//
/// \class TTreeNode
// ~~~~~ ~~~~~~~~~
/// Use this class to navigate the TTreeViewCtrl.
/// Each node conceptually contains a pointer to a TTvItem.
//
class _OWLCLASS TTreeNode {
  public:

		/// Specifies how to insert a new item into a Tree-View control. In 32-bit
		/// applications, the THowToInsert values correspond to Tree-View Insert constants.
    //
    enum THowToInsert {
#if defined __GNUC__   //a cast to a type other than an integral or enumeration type cannot appear in a constant-expression
      First           = -65535,   //TVI_FIRST: First child
      Last            = -65534,   //TVI_LAST: Last child
      Sort            = -65533,   //TVI_SORT: Sort order
#else
      First           = (int)TVI_FIRST,      ///< First child
      Last            = (int)TVI_LAST,       ///< Last child
      Sort            = (int)TVI_SORT        ///< Sort order
#endif
    };

    // Constructors
    //

    // The next two constructors are for creating new nodes.  Both create only
    // the C++ TTreeNode object.  Call SetItem to make the new node appear in
    // the treeview control.
    //
    TTreeNode(TTreeViewCtrl& tree, LPCTSTR text);
    TTreeNode(TTreeViewCtrl& tree, LPCTSTR text, int index, int selIndex);

    // The next two constructors create a node object to represent a node
    // that already exists.

    // Use this constructor in event handlers. The event message carries
    // a TTvNotify structure.
    //
    TTreeNode(TTreeViewCtrl& tree, TVITEM item);

    // This constructor is useful for enumerating nodes on a tree.
    // Passing just the first parameter creates the node object for
    // the tree's root, a good starting point.
    //
    TTreeNode(TTreeViewCtrl& tree, HTREEITEM hItem = TVI_ROOT);

    // Construct the node neighboring a given node.  A TVGN_* flag indicates
    // whether to create the next, previous, parent, or first child node.
    //
    TTreeNode(const TTreeNode& node, uint32 flag);

    // Copy constructor. Used by functions such as TTreeViewCtrl::GetRoot
    // that return a TTreeNode by value.
    //
    TTreeNode(const TTreeNode&);

    // Assignment operator
    //
    TTreeNode& operator=(const TTreeNode& other);

    /// Returns a reference to the associated tree view.
    //
    TTreeViewCtrl& GetTreeView() const {return *TreeView;}

    // Navigation
    //
    TTreeNode GetParent() const;
    TTreeNode GetChild() const;
    TTreeNode GetNextSibling() const;
    TTreeNode GetPrevSibling() const;
    TTreeNode GetNextVisible() const;
    TTreeNode GetPrevVisible() const;
    TTreeNode GetNextItem(uint32 flag) const;

    // Adding and deleting items from the tree
    //
    TTreeNode AddChild(const TTreeNode& node) const;
    TTreeNode AddSibling(const TTreeNode& node) const;
    TTreeNode InsertChild(const TTreeNode& node, THowToInsert how) const;
    TTreeNode InsertItem(const TTreeNode& node) const;
    bool      Delete(void);

    // These older add/delete methods use TTvItem.  They have
    // been replaced by the versions that operate on the node
    // itself.  They are retained for backward compatibility.
    //
    TTreeNode AddChild(const TTvItem&) const;         // add item at end
    TTreeNode AddSibling(const TTvItem&) const;
    TTreeNode InsertChild(const TTvItem&, THowToInsert) const;
    TTreeNode InsertItem(const TTvItem&) const;

    // These operations affect only information stored in the node object,
    // not the Windows treeview control itself.
    //
    void       SetMask(uint mask);
    uint       GetMask(void) const;
    void       SetStateMask(uint mask);
    uint       GetStateMask(void) const;
    void       SetHTreeItem(HTREEITEM hItem);
    HTREEITEM  GetHTreeItem() const;

    // These operations send messages to the control.  The message delivery
    // might fail, so they return bool.
    //
    bool       SetHasChildren(int hasChildren, bool sendNow = true);
    bool       GetHasChildren(int& hasChildren, bool getNew = true);
    bool       SetState(uint state, bool sendNow = true);
    bool       GetState(uint& state, bool getNew = true);
    bool       SetImageIndex(int index, bool sendNow = true);
    bool       SetSelectedImageIndex(int index, bool sendNow = true);
    bool       SetItemData(LPARAM data, bool sendNow = true);
    bool       GetItemData(LPARAM& data, bool getNew = true);
    bool       GetItemRect(TRect& rect, bool textOnly = true) const;
    void Check(bool state = true);
    bool IsChecked() const;

    // Set/GetText store the text in the node's internal buffer so the
    // caller doesn't have to worry about string length and buffer
    // ownership.
    //
    bool       SetText(LPCTSTR text, bool sendNow = true);
    bool SetText(const tstring& text, bool sendNow = true) {return SetText(text.c_str(), sendNow);}

    // This GetText copies the string into the caller's buffer
    //
    bool       GetText(LPTSTR text, uint length, bool getNew = false);

    // This GetText returns a pointer to the node's own internal text buffer
    //
    LPCTSTR    GetText(bool getNew = false);

    // The default cache size is _MAX_PATH.  The cache size is used only in
    // Get functions when allocating buffers for strings received from the
    // control. For a control with strings longer than _MAX_PATH, be sure to
    // bump the cache size before calling Get.  (The buffer size is kept
    // in a static variable, so any alteration affects all TTreeNodes.)
    //
    void       SetCacheSize(uint size);
    void       FlushCache(void);              // empty this node's text cache

    // The Set/Get accessors for individual node attributes send a message
    // to put data into, or receive data from, the treeview control.  The
    // sendNow and getNew parameters make it possible to accumulate requests
    // and send them in a single message.
    //
    //    SetItemData(myData, false);   // this sends no message
    //    SetText("MyText", false);     // this sends no message
    //    SetHasChildren(1, true);      // this sends all 3 attributes

    // The recommended way to get and set attributes is with the access 
    // functions for individual attributes.  Those commands call these
    // two function to interact with the Windows control.
    // 
    bool       GetItem(void);
    bool       SetItem(void);

    // Preserved for 5.0 code that still uses the TTvItem struct.
    //
    bool       GetItem(TTvItem* item);
    bool       SetItem(TTvItem* item);

    // Miscellaneous
    //
    HIMAGELIST CreateDragImage();
    HWND       EditLabel();
    bool       EnsureVisible();
    bool       ExpandItem(uint32 flag);
    bool       SelectItem(uint32 flag);

    // Sort the nodes of the subtree
    //
    bool       SortChildren(bool recurse = false);
    bool       SortChildren(const TTvComparator& Comparator, bool recurse = false, LPARAM extraParam = 0);

    // The HTREEITEM() operator is useful when enumerating nodes.
    // For example, to enumerate all the top-level nodes:
    //
    // for (TTreeNode& node = GetRoot().GetChild();
    //      node;               // evaluates to null handle after last node
    //      node = node.GetNextSibling())
    // { /* loop code */ };
    //                          
    operator  HTREEITEM() const;

    // Codes for TTreeNode (TExpandCode, TNextCode)
    //
    // Retained for compatiblity, but no longer recommended.
    // Use the TVE_ and TVGN_ constants directly instead of
    // these enums.

    /// How to expand the item.
    //
    enum TExpandCode {
      Collapse        = TVE_COLLAPSE,       ///< Always collapse
      Expand          = TVE_EXPAND,         ///< Always expand
      Toggle          = TVE_TOGGLE,         ///< Toggle between collapse and expand
      CollapseReset   = TVE_COLLAPSERESET   ///< Collapse this node and all children
    };

    /// How to retrieve the next node
    //
    enum TNextCode {
      Root            = TVGN_ROOT,              ///< Get the root node
      Next            = TVGN_NEXT,              ///< Get the next sibling
      Previous        = TVGN_PREVIOUS,          ///< Get the prev sibling
      Parent          = TVGN_PARENT,            ///< Get the parent of this node
      Child           = TVGN_CHILD,             ///< Get the first child
      FirstVisible    = TVGN_FIRSTVISIBLE,      ///< Get the first visible item
      NextVisible     = TVGN_NEXTVISIBLE,       ///< Get the next visible item
      PreviousVisible = TVGN_PREVIOUSVISIBLE,   ///< Get the prev visible item
      DropHilite      = TVGN_DROPHILITE,        ///< Get the item that is the drop target
      Caret           = TVGN_CARET              ///< Get the item with the caret
    };

  protected:
    TTvItem     ItemStruct;          ///< contains a TVITEM with HTREEITEM
    TTreeViewCtrl*   TreeView;            ///< wrapper for item of this tree

  private:
    TAPointer<tchar> CacheText;       ///< buffer for node text

    /// Clone function used by copy constructor
    //
    TTreeNode& CopyNode(const TTreeNode& node);

    /// default constructor disallowed
    //
    TTreeNode(void);
};

//
/// \class TTreeViewCtrl
// ~~~~~ ~~~~~~~~~~~
/// Encapsulates the TreeView common control.
//
/// A TreeWindow displays information in a hierarchical manner.
/// Each item in the tree can contain text and a picture.
//
class _OWLCLASS TTreeViewCtrl : public TControl {
  public:
    /// The type of the image list
    //
    enum TImageListType {
      Normal = TVSIL_NORMAL,    ///< Normal imagelist for selected and non-selected items
      State  = TVSIL_STATE      ///< Imagelist contains images for user-defined states
    };

    // Constructors and destructor
    //
    TTreeViewCtrl(TWindow* parent, int id, int x, int y, int w, int h,
                uint32 style = 0, TModule* module = 0);
    TTreeViewCtrl(TWindow* parent, int resourceId, TModule* module = 0);
   ~TTreeViewCtrl();

    // Style management
    //
    void SetStyle(uint32 style);
    bool HasStyle(uint32 style);

    // TreeNode retrieval
    //
    TTreeNode GetRoot();
    TTreeNode GetSelection();
    TTreeNode GetDropHilite();
    TTreeNode GetFirstVisible();

    // Indent level
    //
    uint GetIndent();
    void SetIndent(uint);

    // Image list
    //
    HIMAGELIST GetImageList(TImageListType type);
    HIMAGELIST SetImageList(TImageListType type, HIMAGELIST newList);

    // Miscellaneous
    //
    bool DeleteAllItems();
    uint GetItemCount();
    uint GetVisibleCount();
    HWND GetEditControl();

    // New commctrl.dll functionalty
    //
    TColor   GetBkColor() const;             // Version 4.71
    void     SetBkColor(const TColor& clr);  // Version 4.71

    TColor   GetTextColor() const;           // Version 4.71
    void     SetTextColor(const TColor& clr);// Version 4.71

    TColor   GetInsertMarkColor() const;     // Version 4.71
    void    SetInsertMarkColor(const TColor& clr); // Version 4.71

    int     GetItemHeight(int /*dummy*/) const;          // Version 4.71
    int      SetItemHeight(int /*dummy*/, int height=-1); // Version 4.71

    HWND    GetToolTips() const;            // Version 4.71
    void    SetToolTips(HWND tooltip);      // Version 4.71

    bool    SetInsertMark(HTREEITEM, bool after = true); // Version 4.71


    // Provided for API compatability, use TTreeNode instead.
    // These functions are called from TTreeNode
    //
    void Update();
    bool SortChildren(PFNTVCOMPARE, HTREEITEM parent, bool recurse = false, LPARAM lParam = 0);
    bool SortChildren(HTREEITEM item, bool recurse = false);
    bool Delete(HTREEITEM);
    bool EnsureVisible(HTREEITEM);
    bool ExpandItem(uint32 flag, HTREEITEM);
    bool SelectItem(uint32 flag, HTREEITEM hItem);
    HWND EditLabel(HTREEITEM item);
    HTREEITEM HitTest(TTvHitTestInfo* info);
    HTREEITEM GetNextItem(uint32 nc, HTREEITEM);
    HTREEITEM InsertItem(TV_INSERTSTRUCT *);
    HIMAGELIST CreateDragImage(HTREEITEM item);

    /// Styles for TTreeViewCtrl
    //
    /// Retained for compatiblity, but no longer recommended.
    /// Use the TVS_* style constants directly instead of this enum.
    //
    enum TStyle {
      twsNone            = 0,                     ///< No style
      twsHasButtons      = TVS_HASBUTTONS,        ///< Each parent has a button
                                                  ///< for toggling collapse and expand
      twsHasLines        = TVS_HASLINES,          ///< There are lines between nodes
      twsLinesAtRoot     = TVS_LINESATROOT,       ///< There are lines at the root
      twsEditLabels      = TVS_EDITLABELS,        ///< Text labels for items can be edited
      twsDisableDragDrop = TVS_DISABLEDRAGDROP,   ///< Do not allow drag and drop
      twsShowSelAlways   = TVS_SHOWSELALWAYS,     ///< Always keep the selection visible
                                                  ///< scroll if needed
    };

    // Override TWindow virtual member functions
    //
    uint   Transfer(void* buffer, TTransferDirection direction);

  protected:
    // Override TWindow virtual member functions
    //
    virtual TGetClassNameReturnType GetClassName();

    // Event handlers
    //
    void EvKeyDown(uint key, uint repeatCount, uint flags);
    void EvSysKeyDown(uint key, uint repeatCount, uint flags);

  protected_data:
    uint32 Style;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TTreeViewCtrl(const TTreeViewCtrl&);
    TTreeViewCtrl& operator=(const TTreeViewCtrl&);

  DECLARE_RESPONSE_TABLE(TTreeViewCtrl);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TTreeViewCtrl, 1);
  DECLARE_STREAMABLE_OWL(TTreeViewCtrl, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TTreeViewCtrl );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementation
//

//
/// Default comparison function that makes every item equal to every
/// other item.
/// Derived classes should override this to return proper sorting codes.
//
inline int
TTvComparator::Compare(LPARAM /*item1*/, LPARAM /*item2*/, LPARAM /*lParam*/) const
{
  return 0;
}

//
/// Return the magic cookie used by the control associated with the item
//
inline
TTreeNode::operator HTREEITEM() const
{
  return ItemStruct.hItem;
}

//
/// Return the HWND of the edit control to change the text
//
inline HWND
TTreeNode::EditLabel()
{
  return TreeView->EditLabel(*this);
}

//
/// Return the image list used for a dragging purposes
//
inline HIMAGELIST
TTreeNode::CreateDragImage()
{
  return TreeView->CreateDragImage(*this);
}

//
/// Makes sure the item is visible.
/// Scroll the items if necessary.
//
inline bool
TTreeNode::EnsureVisible()
{
  return TreeView->EnsureVisible(*this);
}

//
/// Expand or contract a parent node.
/// Similar to the user clicking on the '-' or '+' area of the control.
//
inline bool
TTreeNode::ExpandItem(uint32 flag)
{
  return TreeView->ExpandItem(flag, (HTREEITEM)*this);
}

//
/// Make the next item selected.
//
inline bool
TTreeNode::SelectItem(uint32 flag)
{
  return TreeView->SelectItem(flag, *this);
}

//
/// Return the next item.
//
inline TTreeNode
TTreeNode::GetNextItem(uint32 flag) const
{
  return TTreeNode(*TreeView, TreeView->GetNextItem(flag, *this));
}

inline TTreeNode
TTreeNode::InsertChild(const TTreeNode& node, THowToInsert how) const
{
  return InsertChild(node.ItemStruct, how);
}

inline TTreeNode
TTreeNode::InsertItem(const TTreeNode& node) const
{
  return InsertItem(node.ItemStruct);
}

/// Inserts a new child item into the Tree-View (TreeView) control.
//
/// Parameters
/// \arg \c \b node	A TTreeNode object encapsulating an item that will be the parent of the
/// new item.
/// 
/// Returns a TTreeNode object encapsulating the new item. Returns NULL upon
/// failure. (In 32-bit applications, you can check for a NULL handle using the
/// HTREEITEM() operator.)
inline TTreeNode
TTreeNode::AddChild(const TTreeNode& node) const
{
  return InsertChild(node.ItemStruct, Last);
}

inline TTreeNode
TTreeNode::AddChild(const TTvItem& item) const
{
  return InsertChild(item, Last);
}

//
/// Return the parent of the current node.
//
inline TTreeNode
TTreeNode::GetParent() const
{
  return GetNextItem(Parent);
}

//
/// Get the first child of the current node.
//
inline TTreeNode
TTreeNode::GetChild() const
{
  return GetNextItem(Child);
}

//
/// Return the next sibling.
//
inline TTreeNode
TTreeNode::GetNextSibling() const
{
  return GetNextItem(Next);
}

//
/// Return the previous sibling.
//
inline TTreeNode
TTreeNode::GetPrevSibling() const
{
  return GetNextItem(Previous);
}

//
/// Return the next visible item.
//
inline TTreeNode
TTreeNode::GetNextVisible() const
{
  return GetNextItem(NextVisible);
}

//
/// Return the next previous item.
//
inline TTreeNode
TTreeNode::GetPrevVisible() const
{
  return GetNextItem(PreviousVisible);
}

//
/// Sort the children of this node
//
inline bool
TTreeNode::SortChildren(bool recurse)
{
  return TreeView->SortChildren(*this, recurse);
}

inline void
TTreeNode::SetMask(uint mask)
{
  ItemStruct.mask = mask;
}

inline uint       
TTreeNode::GetMask() const
{
  return ItemStruct.mask;
}

inline void
TTreeNode::SetStateMask(uint mask)
{
  ItemStruct.stateMask = mask;
}

inline uint
TTreeNode::GetStateMask() const
{
  return ItemStruct.stateMask;
}

//
///  Set and Get the node's item handle.
//

inline void
TTreeNode::SetHTreeItem(HTREEITEM hItem)
{
  ItemStruct.SetHTreeItem(hItem);
}

inline HTREEITEM  
TTreeNode::GetHTreeItem() const
{
  return ItemStruct.GetHTreeitem();
}

//
/// Set the item associated with this node
//
inline bool
TTreeNode::SetItem(TTvItem* item)
{
  PRECONDITION(TreeView);
  PRECONDITION(item && item->mask);
  item->SetHTreeItem(*this);
  return TreeView->SendMessage(TVM_SETITEM, 0, TParam2(item)) != 0;
}

inline bool
TTreeNode::SetItem()
{
  PRECONDITION(TreeView);
  PRECONDITION(ItemStruct.mask);
  return TreeView->SendMessage(TVM_SETITEM, 0, TParam2(&ItemStruct)) != 0;
}

//
/// Return the item associated with the node
//
inline bool
TTreeNode::GetItem(TTvItem* item)
{
  PRECONDITION(ItemStruct.hItem);
  PRECONDITION(TreeView);
  item->SetHTreeItem(*this);
  return TreeView->SendMessage(TVM_GETITEM, 0, TParam2(item)) != 0;
}

//
/// Update the item information in the node.
/// What's set in the mask determines what will be retrieved.
/// Unless the user has explicitly called SetMask, the call should
/// retrieve everything.
//
inline bool
TTreeNode::GetItem()
{
  PRECONDITION(ItemStruct.mask & TVIF_HANDLE);
  PRECONDITION(ItemStruct.hItem);
  PRECONDITION(TreeView);
  return TreeView->SendMessage(TVM_GETITEM, 0, TParam2(&ItemStruct)) != 0;
}

//----------------------------------------------------------------------------
// TTreeViewCtrl

//
/// Return the item that contains the point.
//
inline HTREEITEM
TTreeViewCtrl::HitTest(TTvHitTestInfo* info)
{
  return (HTREEITEM)SendMessage(TVM_HITTEST, 0, TParam2(info));
}

//
/// Set the image list used by the control.
//
inline HIMAGELIST
TTreeViewCtrl::SetImageList(TImageListType type, HIMAGELIST newList)
{
  return (HIMAGELIST)SendMessage(TVM_SETIMAGELIST, TParam1(type), TParam2(newList));
}

//
/// Return the image list used by the control.
//
inline HIMAGELIST
TTreeViewCtrl::GetImageList(TImageListType type)
{
  return (HIMAGELIST)SendMessage(TVM_GETIMAGELIST, TParam1(type));
}

//
/// Create a drag image.
//
inline HIMAGELIST
TTreeViewCtrl::CreateDragImage(HTREEITEM item)
{
  return (HIMAGELIST)SendMessage(TVM_CREATEDRAGIMAGE, 0, TParam2(item));
}

//
/// Return the edit control used for editing the text.
//
inline HWND
TTreeViewCtrl::GetEditControl()
{
  return (HWND)SendMessage(TVM_GETEDITCONTROL);
}

// Version 4.71
inline TColor
TTreeViewCtrl::GetBkColor() const
{
  return static_cast<COLORREF>(CONST_CAST(TTreeViewCtrl*,this)->SendMessage(TVM_GETBKCOLOR));
}

// Version 4.71
inline void
TTreeViewCtrl::SetBkColor(const TColor& clr)
{
  SendMessage(TVM_SETBKCOLOR,0,TParam2(clr.GetValue()));
}

// Version 4.71
inline TColor
TTreeViewCtrl::GetTextColor() const
{
  return static_cast<COLORREF>(CONST_CAST(TTreeViewCtrl*,this)->SendMessage(TVM_GETTEXTCOLOR));
}

// Version 4.71
inline void
TTreeViewCtrl::SetTextColor(const TColor& clr)
{
  SendMessage(TVM_SETTEXTCOLOR,0,TParam2(clr.GetValue()));
}

//Version 4.71
inline TColor
TTreeViewCtrl::GetInsertMarkColor() const
{
  return static_cast<COLORREF>(CONST_CAST(TTreeViewCtrl*,this)->SendMessage(TVM_GETINSERTMARKCOLOR));
}

//Version 4.71
inline void
TTreeViewCtrl::SetInsertMarkColor(const TColor& clr)
{
  SendMessage(TVM_SETINSERTMARKCOLOR,0,TParam2(clr.GetValue()));
}

// Version 4.71
inline int
TTreeViewCtrl::GetItemHeight(int /*dummy*/) const
{
  return int(CONST_CAST(TTreeViewCtrl*,this)->SendMessage(TVM_GETITEMHEIGHT));
}

// Version 4.71
inline int
TTreeViewCtrl::SetItemHeight(int /*dummy*/, int height)
{
  return (int)SendMessage(TVM_SETITEMHEIGHT, TParam1((int16)height));
}

// Version 4.71
inline HWND
TTreeViewCtrl::GetToolTips() const
{
  return HWND(CONST_CAST(TTreeViewCtrl*,this)->SendMessage(TVM_GETTOOLTIPS));
}

// Version 4.71
inline void
TTreeViewCtrl::SetToolTips(HWND tooltip)
{
  SendMessage(TVM_SETTOOLTIPS, TParam1(tooltip));
}

// Version 4.71
inline bool
TTreeViewCtrl::SetInsertMark(HTREEITEM item, bool after)
{
  return ToBool(SendMessage(TVM_SETINSERTMARK, TParam1((BOOL)after), TParam2(item)));
}

//
/// Enable the user to edit the text of an item.
//
inline HWND
TTreeViewCtrl::EditLabel(HTREEITEM item)
{
  return (HWND)SendMessage(TVM_EDITLABEL, 0, TParam2(item));
}

//
/// Return the next item.
//
inline HTREEITEM
TTreeViewCtrl::GetNextItem(uint32 nc, HTREEITEM hItem)
{
  if (hItem == TVI_ROOT)  // Workaround for Win95 bug.
    hItem = 0;
  return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, nc, TParam2(hItem));
}

//
/// Delete the item.
//
inline bool
TTreeViewCtrl::Delete(HTREEITEM hItem)
{
  return SendMessage(TVM_DELETEITEM, 0, TParam2(hItem)) != 0;
}

//
/// Makes sure the item is visible.
//
inline bool
TTreeViewCtrl::EnsureVisible(HTREEITEM hItem)
{
  return SendMessage(TVM_ENSUREVISIBLE, 0, TParam2(hItem)) != 0;
}

//
/// Expand and contract the parent node.
//
inline bool
TTreeViewCtrl::ExpandItem(uint32 flag, HTREEITEM hItem)
{
  return SendMessage(TVM_EXPAND, flag, TParam2(hItem)) != 0;
}

//
/// Select the next item.
//
inline bool
TTreeViewCtrl::SelectItem(uint32 flag, HTREEITEM hItem)
{
  return SendMessage(TVM_SELECTITEM, flag, TParam2(hItem)) != 0;
}

//
/// Insert an item.
//
inline HTREEITEM
TTreeViewCtrl::InsertItem(TV_INSERTSTRUCT * tvis)
{
  return (HTREEITEM)SendMessage(TVM_INSERTITEM, 0, TParam2(tvis));
}

//
/// Remove all items from the control
//
inline bool
TTreeViewCtrl::DeleteAllItems()
{
  return Delete(TVI_ROOT);
}

//
/// Return the number of items in the control.
//
inline uint
TTreeViewCtrl::GetItemCount()
{
  return (uint)SendMessage(TVM_GETCOUNT);
}

//
/// Return the number of the fully visible items in the control.
//
inline uint
TTreeViewCtrl::GetVisibleCount()
{
  return (uint)SendMessage(TVM_GETVISIBLECOUNT);
}

//
/// Return the number of pixels per indent level.
//
inline uint
TTreeViewCtrl::GetIndent()
{
  return (uint)SendMessage(TVM_GETINDENT);
}

//
/// Set the number of pixels per indent level.
//
inline void
TTreeViewCtrl::SetIndent(uint indent)
{
  SendMessage(TVM_SETINDENT, indent);
}

//
/// Sort the children of 'item'.
/// Recursively sort each children if 'recurse' is true.
//
inline bool
TTreeViewCtrl::SortChildren(HTREEITEM item, bool recurse)
{
  return SendMessage(TVM_SORTCHILDREN, recurse, TParam2(item)) != 0;
}

//
/// Return the root node.
//
inline TTreeNode
TTreeViewCtrl::GetRoot()
{
  return TTreeNode(*this);
}

//
/// Compatability for Win32
//
inline void
TTreeViewCtrl::Update()
{
}

} // OWL namespace



#endif  // OWL_TREEVIEWCTRL_H
