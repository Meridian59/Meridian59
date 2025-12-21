//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// TDirDialog v. 1.0 - (C) Copyright 1996 by Kent Reisdorph, All Rights Reserved
// Copyright (c) 1998 by Yura Bidus. All Rights Reserved.
//
// Original code by Kent Reisdorph, 75522,1174
//
//  OVERVIEW
//  ~~~~~~~~
// Source for class TDirDialog - A dialog class that allows
// directory selection similar to Win95's Explorer
//
//----------------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owl/treeviewctrl.h>
#include <owl/imagelst.h>
#include <owl/static.h>
#include <owl/filename.h>

//#include <stdlib.h>

#include <owlext/dirdialg.h>
#include <owlext/dirdialg.rh>

namespace OwlExt {

using namespace owl;

// private function
TModule* FindResourceModule(TWindow* parent, TModule* module, TResId resId, LPCTSTR type);

// constants
const int EXPANDED   = 1;
const int QUICK     = 0;
const int DETAIL     = 1;
const int DRIVES     = 2;
const int CDROM     = 3;

// Constructor for TData. I decided to structure things using a
// TData class in keeping with the conventions used by the OWL
// common dialog classes. This class is not completely necessary
// at this point but allows for additional functionality later.

// Allocates storage to hold the directory path selected.
TDirDialog::TData::TData()
{
  DirPath = new _TCHAR[MAX_PATH];
}

// delete the DirPath member
TDirDialog::TData::~TData()
{
  delete[] DirPath;
}

// Response table. TTreeView notifications for the selection of
// an item and for the expanding of a node.
DEFINE_RESPONSE_TABLE1(TDirDialog, TDialog)
EV_BN_CLICKED(IDOK, CmOk),
EV_TVN_SELCHANGED(IDC_DIRTREE,   TvnSelChanged),
EV_TVN_ITEMEXPANDED(IDC_DIRTREE,TvnItemExpanded),
END_RESPONSE_TABLE;

// constructor
TDirDialog::TDirDialog(TWindow* parent, TData& data, TModule* module)
:
TDialog(parent, IDD_DIRDLG,
    FindResourceModule(parent,module,IDD_DIRDLG,RT_DIALOG)),
    Data(data)
{
  // pointers for the TTreeViewCtrl and the static control where
  // the current directory is displayed
  DirTree    = new TTreeViewCtrl(this, IDC_DIRTREE);
  CurrentDir = new TStatic(this, IDC_CURRENTDIR);

  // a TImageList object which holds our bitmaps
  Images = new TImageList(*GetModule(), IDB_DIRDLG_TREEIMAGE, 20, 2,
    TColor(255, 255, 255), IMAGE_BITMAP, 0);
  // set level to 0 initially
  level = 0;
}

// Destructor. Deletes the image list object
TDirDialog::~TDirDialog()
{
  delete Images;
}

// SetupWindow(). Sets the image list of the tree window and then
// calls LoadTree() to add the initial directory nodes to the
// tree.
void
TDirDialog::SetupWindow()
{
  TDialog::SetupWindow();
  DirTree->SetImageList(TTreeViewCtrl::Normal, *Images);
  LoadTree();
}

// CmOk(). Gets the path from the CurrentDir static control
// and places it in TData::DirPath.
void
TDirDialog::CmOk()
{
  CurrentDir->GetText(Data.DirPath, MAX_PATH);
  TDialog::CmOk();
}

// TvnSelChanged(). This is the selection change notification
// handler for the tree window.
void
TDirDialog::TvnSelChanged(TTvNotify &)
{
  // get the selected node
  TTreeNode sel = DirTree->GetSelection();
  owl::tstring s;
  // build the path name based on the selection
  BuildPath(s, sel);
  // set the static control's text to the complete path
  CurrentDir->SetText(s.c_str());
}

// TvnItemExpanded(). This is the item expanded notification
// handler for the tree window.
void
TDirDialog::TvnItemExpanded(TTvNotify & twn)
{
  // get the node that was expanded or contracted
  TTreeNode node(*DirTree, twn.itemNew.hItem);

  // get the TTvItem for the node
  TTvItem item;
  // must do this so that TTvItem knows we will be using the
  // item data
  item.SetItemData(0);
  node.GetItem(&item);

  // if the node was contracted then set the item data so that
  // later we know not to rebuild the node.
  if (twn.action == TVE_COLLAPSE) {
    item.SetItemData(EXPANDED);
    node.SetItem(&item);
    return;
  }

  // If the node was already built once then we don't need to do
  // it again. If we do, we'll have duplicate entries
  if (item.GetItemData() == EXPANDED)
    return;

  // remove the first child because we're going to add it again
  // remember that we have to have already added one child per
  // node so that the plus sign gets added to the node
  TTreeNode child = node.GetChild();
  child.Delete();

  // add this node and all subnodes
  AddNode(0, node, DETAIL);

  return;
}
// BuildPath(). This function builds the path name by recursing
// backwards through the tree. Use the string class to make
// manipulation of the string easy.
void
TDirDialog::BuildPath(owl::tstring& path, TTreeNode& node)
{
  TAPointer<_TCHAR> buff(new _TCHAR [MAX_PATH]);

  // get the parent of the current node
  TTreeNode parent = node.GetParent();

  // create a TTvItem with the buffer and get the item
  // from the node. 'buff' will now contain the directory name.
  buff[0] = _T('\0');
  TTvItem item(buff, MAX_PATH);
  node.GetItem(&item);

  // add the backslash to the node
  _tcscat(buff, _T("\\"));

  // add (prepend) the current directory name to the front of
  // the path. Remember we are stepping backwards through the
  // directory tree.
  path.insert(0, buff);

  // if this node has a parent then we have to recurse until
  // no parent is found. This is how we will know that we are
  // done building the directory path string.
  if (parent)
    BuildPath(path, parent);

  // If the parent is found then we need to strip the
  // "(" and ")" from the string. We added the parens
  // to distinguish the drive nodes.
  else
  {
    auto pos = path.find(_T("("));
    if (pos != path.npos)
      path.erase(pos, 1);
    pos = path.find(_T(")"));
    if (pos != path.npos)
      path.erase(pos, 1);
  }
}

// LoadTree(). Adds the drive nodes. We are only using fixed
// disks and CDROMS.
void
TDirDialog::LoadTree()
{
  TAPointer<_TCHAR> drives(new _TCHAR[MAX_PATH]);

  // Get the drive string.
  if (::GetLogicalDriveStrings(MAX_PATH, drives)) {
    _TCHAR* aDrive = drives;

    // parse the string
    do {
      int length = static_cast<int>(_tcslen(aDrive)) + 1;
      uint type = ::GetDriveType(aDrive);

      // only HDs and CDROMS   //??????????????
      if (type == DRIVE_FIXED || type == DRIVE_CDROM) {

        // we'll need to have the type in the AddNode() function
        // so we can use the proper bitmap next to the node name
        if (type == DRIVE_FIXED)
          type = DRIVES;
        else
          type = CDROM;

        // get a drive
        if (aDrive[_tcslen(aDrive) - 1] == _T('\\'))
          aDrive[_tcslen(aDrive) - 1] = 0;

        // add the node
        AddNode(_tcsupr(aDrive), DirTree->GetRoot(), type);
      }
      aDrive += length;
    } while(*aDrive);
  }
}

// AddNode(). This function adds the nodes to the tree. The first
// time we display a node we don't want to add each and every
// sub-directory to the tree. We only want to add nodes when
// necessary or it will take forever for the dialog box to come
// up initially. But, we have to add at least one node so that
// the + sign shows up indicating that sub-directories exist.
void
TDirDialog::AddNode(LPCTSTR path, const TTreeNode& node, int mode)
{
  // add one to the level we are on
  level++;

  // temporary storage
  TAPointer<_TCHAR> buff(new _TCHAR [MAX_PATH]);

  // by default we will be using image 0 for the node and
  // image 1 for the selected node
  int image1 = 0, image2 = 1;

  // need a string object to build the directory path
  owl::tstring dirPath;
  TTreeNode newNode = node;

  // if a node name was passed to us then we will add the node
  // if no node name was passed then we need to get a directory
  // name first and then recurse
  if (path) {

    // if the mode indicates that we are adding a drive node
    // then we need to:
    // add the parens around the drive name...
    // set the image to correspond to the type of drive...
    // reset the mode to QUICK
    if (mode >= DRIVES) {
      wsprintf(buff, _T("(%s)"), path);
      image1 = image2 = mode;
      mode = QUICK;
    }

    // otherwise just use the node name passed
    else
      _tcscpy(buff, path);

    // Add the node by creating a TTvItem based on the string,
    // the image, and the select image. Use InsertChild so that
    // the list is sorted as we go.
    newNode = node.InsertChild( TTvItem(buff, image1, image2),
      TTreeNode::Sort );
  }

  // first, build a path name based on the node and add wildcards
  BuildPath(dirPath, newNode);
  dirPath += _T("*.*");
#if 1
  TFileNameIterator itr(dirPath);
  while(itr){
    if((*itr).attribute & TFile::Directory){
      if (_tcscmp(itr, _T(".")) == 0)
        ; // don't want it
      else if (_tcscmp(itr, _T("..")) == 0)
        ; // don't want this either
      else {
        // add the node by recursion
        AddNode(itr, newNode, mode);

        // update the tree so that we can see the nodes being added
        DirTree->UpdateWindow();

        // only want one sub-node per node for now in the QUICK mode
        if (mode == QUICK)
          break;

        // In the detail mode we need to fill this branch completely
        // but only one sub-node per node under this branch. Again,
        // we have to do this so the + sign shows up next to the node.
        if (mode == DETAIL && level > 1)
          break;
      }
    }
    ++itr;
  }
#else
  // use findfirst & findnext to get the directories
  ffblk fb;
  bool done = findfirst(dirPath.c_str(), &fb, FA_DIREC);
  while (!done) {
    if (fb.ff_attrib & FA_DIREC) {
      if (strcmp(fb.ff_name, ".") == 0)
        ; // don't want it
      else if (strcmp(fb.ff_name, "..") == 0)
        ; // don't want this either
      else {

        // add the node by recursion
        AddNode(fb.ff_name, newNode, mode);

        // update the tree so that we can see the nodes being added
        DirTree->UpdateWindow();

        // only want one sub-node per node for now in the QUICK mode
        if (mode == QUICK)
          break;

        // In the detail mode we need to fill this branch completely
        // but only one sub-node per node under this branch. Again,
        // we have to do this so the + sign shows up next to the node.
        if (mode == DETAIL && level > 1)
          break;
      }
    }

    // do it again
    done = findnext(&fb);
  }
#endif

  // decrement the level counter
  level--;
}

} // OwlExt namespace

