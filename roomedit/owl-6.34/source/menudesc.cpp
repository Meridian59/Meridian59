//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TMenuDescr
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/menu.h>
#include <owl/module.h>

namespace owl {

OWL_DIAGINFO;

/// \class TMenuDescr
/// Derived from TMenu, TMenuDescr describes your menu bar and its functional
/// groups. TMenuDescr provides an easy way for you to group menus on your menu bar
/// and to add new groups to an existing menu bar. It uses a resource ID to identify
/// the menu resource and an array of count values to indicate the number of pop-up
/// menus in each group on the menu bar.
/// 
/// The TGroup enum enumerates the six basic functional groups on a menu bar: File,
/// Edit, Container, Object, Window, and Help. TMenuDescr's constructors simply
/// initialize the members based on the arguments passed. 
/// 
/// TFrameWindow::MergeMenu actually performs the real work of merging the menu
/// groups.
/// 
/// For a single document application (SDI), the menus are merged as soon as you
/// load the application. However, for a multiple-document application (MDI), you do
/// not see the final menu until you invoke a child menu. See the sample program,
/// mdifile.cpp, for an example of MDI menu merging.
/// 
/// One technique you can use to create a menu involves invoking the TMenuDescr
/// constructor and passing the number of group counts for each menu selection.
/// 
/// For example, if your original menu groups included these items:
/// \image html bm78.BMP
/// you might use the following group counts:
/// 
/// - \c \b  Group	Count	Menu
/// - \c \b  FileGroup	1	File
/// - \c \b  EditGroup	2	Edit Search
/// - \c \b  ContainerGroup	1	View
/// - \c \b  ObjectGroup	3	Page Paragraph Word
/// - \c \b  WindowGroup	1	Window
/// - \c \b  HelpGroup	1	Help
/// 
/// You would then invoke the constructor this way:
/// \code
/// TMenuDescr(IDM_MYMENU, 1, 2, 1, 3, 1, 1)
/// \endcode
/// You can build the previous menu by merging two menus. When a zero is passed in
/// the parent menu's constructor, the group indicated by the zero is filled in by
/// the child menu's group, if an item is specified, when the menu merging occurs. 
/// Set your application's parent frame menu bar by specifying these menu groups:
/// \image html bm79.BMP
/// and passing these group counts in the constructor:
/// \code
/// TMenuDescr(IDM_FRAME, 1, 0, 1, 0, 1, 1)
/// \endcode
/// Set the word-processor child menu bar this way:
/// \image html bm80.BMP
/// and pass these values in the constructor:
/// \code
/// TMenuDescr(IDM_WPROC, 0, 2, 0, 3, 0, 1)
/// \endcode
/// If no child is active, only the frame menu will be active. When the word
/// processor's child window becomes active, the child menu bar is merged with the
/// frame menu. Every group that is 0 in the child menu bar leaves the parent's
/// group intact. The previous example interleaves every group except for the last
/// group, the Help group, in which the child replaces the frame menu.
/// 
/// By convention, the even groups (File, Container, Window) usually belong to the
/// outside frame or container, and the odd groups (Edit, Object, Help) belong to
/// the child or contained group.
/// 
/// If a -1 is used in a group count, the merger eliminates the parent's group
/// without replacing it. For example, another child menu bar, such as a calculator,
/// could be added to your application in this way:
/// \image html bm81.BMP
/// \code
/// TMenuDescr(IDM_WCALC, 0, 1, -1, 1, 0, 1)
/// \endcode
/// In this example, the child's menu group contributes nothing from the container
/// group, and the parent's container group is removed. This produces a merged menu
/// (with the View menu selection eliminated as a result of the -1) that looks like
/// this:
/// \image html bm82.BMP
/// 
/// If you want to merge the following parent menu groups
/// \image html bm79.BMP
/// with these paint window menu groups,
/// \image html bm83.BMP
/// pass the following group counts in the constructor:
/// \code
/// TMenuDescr(IDM_WPAINT, 0, 1, 0, 2, 0, 1)
/// \endcode
/// This produces the following merged menu:
/// \image html bm84.BMP
/// 
/// The simplest way to add groups to a menu bar involves defining the menu groups
/// and adding separators in a resource file. Insert the term MENUITEM SEPARATOR
/// between each menu group and an additional separator if one of the menu groups is
/// not present. For example, the resource file for Step 14 of the ObjectWindows
/// tutorial defines the following menu groups and separators:
/// \code
/// IDM_MDICMNDS MENU
/// { 
/// // Display a grayed File menu
///  MENUITEM "File",   0,GRAYED  ;placeholder for File menu from DocManager
///  MENUITEM SEPARATOR
///  MENUITEM "Edit",   CM_NOEDIT ;placeholder for Edit menu from View
///  MENUITEM SEPARATOR
///  MENUITEM SEPARATOR
///  MENUITEM SEPARATOR
///  POPUP "&Window"
/// { 
/// // Options within the Window menu group
///   MENUITEM  "&Cascade",       CM_CASCADECHILDREN
///   MENUITEM  "&Tile",          CM_TILECHILDREN
///   MENUITEM  "Arrange &Icons", CM_ARRANGEICONS
///   MENUITEM  "C&lose All",     CM_CLOSECHILDREN
///   MENUITEM  "Add &View",      CM_VIEWCREATE
///  }
///  MENUITEM SEPARATOR
///  POPUP "&Help"
///  { 
///   MENUITEM "&About",   CM_ABOUT
///  }
/// }
/// \endcode
/// You can see the separators by loading Step14.rc into Resource Workshop and
/// disabling the View as Popup Option in the View menu. This resource file defines
/// an Edit group, a File group, a Window group, and a Help group, but no entries
/// for Container or Object groups.
/// Step14.cpp uses these commands from the resource file to set the main window and
/// its menu, passing IDM_MDICMNDS as the parameter to SetMenuDescrfunction, as
/// follows:
/// \code
/// SetMainWindow(frame);
/// GetMainWindow()->SetMenuDescr(TMenuDescr(IDM_MDICMNDS));
/// \endcode
/// It produces the following menu groups:
/// \image html bm85.BMP
/// 
/// TMenuDescr's functions let you perform menu merging similar to that of OLE 2.
/// That is, you can merge menus from a container's document (the MDI frame window)
/// with those of an embedded object (the MDI child window). When the embedded
/// object is activated in place by double-clicking the mouse, the menu of the child
/// window merges with that of the frame window.




//
// Construct a default, empty menu descriptor
//
/// Default constructor for a TMenuDescr object. No menu resources or groups are
/// specified. Constructs an empty menu bar.
TMenuDescr::TMenuDescr()
:
  TMenu()
{
  Id = 0;
  Module = &GetGlobalModule();
  for (int i = 0; i < NumGroups; i++)
    GroupCount[i] = 0;
}

//
/// Copies the menu descriptor object specified in the other parameter.
//
TMenuDescr::TMenuDescr(const TMenuDescr& other)
:
  TMenu(other)
{
  Id = other.Id;
  Module = other.Module;
  for (int i = 0; i < NumGroups; i++)
    GroupCount[i] = other.GroupCount[i];
}

//
/// Constructs a menu descriptor from the resource indicated by id. Places the
/// pop-up menus in groups according the values of the fg, eg, cg, of, wg, and hg
/// parameters. The fg, eg, cg, of, wg, and hg parameters represent the functional
/// groups identified by the TGroup enum. Calls the function ExtractGroups to
/// extract the group counts based on the separator items in the menu bar.
//
TMenuDescr::TMenuDescr(TResId id,
                       int fg, int eg, int cg, int og, int wg, int hg,
                       TModule* module)
:
  TMenu(*module, id),
  Module(module),
  Id(id)
{
  if (!ExtractGroups()) {
    GroupCount[FileGroup] = fg;
    GroupCount[EditGroup] = eg;
    GroupCount[ContainerGroup] = cg;
    GroupCount[ObjectGroup] = og;
    GroupCount[WindowGroup] = wg;
    GroupCount[HelpGroup] = hg;
  }
}

//
/// Creates a menu descriptor from the menu resource specified in the id parameter.
/// Calls the function ExtractGroups to extract the group counts based on the
/// separator items in the menu bar.
//
TMenuDescr::TMenuDescr(TResId id, TModule* module)
:
  TMenu(*module, id),
  Module(module),
  Id(id)
{
  ExtractGroups();
}

//
/// Constructs a menu descriptor from the menu handle indicated in the hMenu
/// parameter. The menu descriptor can have zero or more pop-up menus in more than
/// one functional group. The fg, eg, cg, of, wg, and hg parameters represent the
/// functional groups identified by the TGroup enum.  Calls the function
/// ExtractGroups to extract the group counts based either on the separator items in
/// the menu bar or on the group count parameters specified if there are no
/// separators in the menu bar.
//
TMenuDescr::TMenuDescr(HMENU hMenu,
                       int fg, int eg, int cg, int og, int wg, int hg,
                       TModule* module)
:
  TMenu(hMenu, NoAutoDelete),
  Module(module)
{
  if (!ExtractGroups()) {
    GroupCount[FileGroup] = fg;
    GroupCount[EditGroup] = eg;
    GroupCount[ContainerGroup] = cg;
    GroupCount[ObjectGroup] = og;
    GroupCount[WindowGroup] = wg;
    GroupCount[HelpGroup] = hg;
  }
}

//
// destructor
//
TMenuDescr::~TMenuDescr()
{
}

//
// Assign another menu descriptor on to this one
//
TMenuDescr&
TMenuDescr::operator =(const TMenuDescr& other)
{
  *STATIC_CAST(TMenu*,this) = *STATIC_CAST(const TMenu*,&other);
  Id = other.Id;
  Module = other.Module;
  for (int i = 0; i < NumGroups; i++)
    GroupCount[i] = other.GroupCount[i];
  return *this;
}

//
/// Extracts the group counts from the loaded menu bar by counting the number of
/// menus between separator items. After the group counts are extracted, the
/// separators are removed.
//
bool
TMenuDescr::ExtractGroups()
{
  if (!Handle)
    return false;  // no menu to extract from...

  // walk menu & remove separators, setting up count as we go.
  //
  int itemCount = GetMenuItemCount();
  int g = 0;
  int count = 0;
  for (int i = 0; i < itemCount; ) {
    uint s = GetMenuState(i, MF_BYPOSITION);
    if ((s & MF_SEPARATOR) && !(s & MF_POPUP)) {
      if (g < NumGroups)
        GroupCount[g++] = count;
      count = 0;
      RemoveMenu(i, MF_BYPOSITION);
      itemCount--;
    }
    else {
      i++;
      count++;
    }
  }
  // Leave if no separators were found
  //
  if (!g)
    return false;

  // Get trailing group
  //
  if (g < NumGroups)
    GroupCount[g++] = count;

  // Finish zeroing groups
  //
  for (; g < NumGroups; g++)
    GroupCount[g] = 0;
  return true;
}

//
/// Merges the functional groups of another menu descriptor into this menu
/// descriptor.
///
/// Popups are DeepCopied and are then owned by this menu
/// Group counts are merged too.
//
bool
TMenuDescr::Merge(const TMenuDescr& srcMenuDescr)
{
  int thisOffset = 0;
  int srcOffset = 0;

  for (int i = 0; i < NumGroups; i++) {
    if (srcMenuDescr.GroupCount[i] != 0) {
      // Delete same menu group in the dest. menudescr.
      for (int j = GroupCount[i] - 1; j >= 0; j--) {
        DeleteMenu(thisOffset+j, MF_BYPOSITION);
      }
      GroupCount[i] = 0;

      if (srcMenuDescr.GroupCount[i] > 0) {
        DeepCopy(*this, thisOffset, srcMenuDescr, srcOffset, srcMenuDescr.GroupCount[i]);
        srcOffset += srcMenuDescr.GroupCount[i];
        GroupCount[i] += srcMenuDescr.GroupCount[i];
      }
    }

    if (GroupCount[i] > 0)
      thisOffset += GroupCount[i];
  }
  return true;
}

//
/// Merges the functional groups of this menu descriptor and another menu descriptor
/// into an empty menu.
///
/// Popups are DeepCopied and are then owned by the destMenu.
//
bool
TMenuDescr::Merge(const TMenuDescr& srcMenuDescr, TMenu& dstMenu)
{
  int thisOffset = 0;
  int srcOffset = 0;

  for (int i = 0; i < NumGroups; i++) {
    if (srcMenuDescr.GroupCount[i] > 0) {
      DeepCopy(dstMenu, srcMenuDescr, srcOffset, srcMenuDescr.GroupCount[i]);
      srcOffset += srcMenuDescr.GroupCount[i];
    }
    else if (srcMenuDescr.GroupCount[i] == 0 && GroupCount[i] > 0) {
      DeepCopy(dstMenu, *this, thisOffset, GroupCount[i]);
    }
    // else don't copy either

    if (GroupCount[i] > 0)
      thisOffset += GroupCount[i];
  }
  return true;
}


#if !defined(BI_NO_OBJ_STREAMING)
//
// Extract the menu descriptor from the persistent stream.
//
_OWLCFUNC(ipstream&)
operator >>(ipstream& is, TMenuDescr& m)
{
  is >> m.Id;
  is >> m.Module;
  for (int i = 0; i < TMenuDescr::NumGroups; i++)
    is >> m.GroupCount[i];
  return is;
}

//
// Insert the menu descriptor into the persistent stream.
//
_OWLCFUNC(opstream&)
operator <<(opstream& os, const TMenuDescr& m)
{
  os << m.Id;
  os << m.Module;
  for (int i = 0; i < TMenuDescr::NumGroups; i++)
    os << m.GroupCount[i];
  return os;
}

#endif  // #if !defined(BI_NO_OBJ_STREAMING)


} // OWL namespace
/* ========================================================================== */

