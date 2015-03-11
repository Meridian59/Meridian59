//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of Window Menu encapsulation class
//----------------------------------------------------------------------------

#if !defined(OWL_MENU_H)
#define OWL_MENU_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/gdiobjec.h>
#include <owl/wsyscls.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup menus
/// @{

class _OWLCLASS TXMenu;
class _OWLCLASS TMenu;


#if 0  // !CQ A nice idea, but has a couple of snags yet
/// \cond
//
// class TMenuItem
// ~~~~~ ~~~~~~~~~
// Representation of a menu item which can be a uint Id/position or a popup
// menu handle.
//
class TMenuItem {
  public:
    TMenuItem(uint id) {Item = id;}
    TMenuItem(HMENU popup) {Item = reinterpret_cast<uint>(popup);}
    operator uint() {return Item;}

  private:
    uint Item;
};
/// \endcond
#else
// Alias of an unsigned integer which identifies a menu item.
//
typedef uint TMenuItem;
#endif


// All-including mask for MENUITEMINFO
//
#if !defined(MIIM_ALL)
# define MIIM_ALL  (MIIM_CHECKMARKS|MIIM_DATA|MIIM_ID|MIIM_STATE|MIIM_SUBMENU|MIIM_TYPE)
#endif


//
/// \class TMenuItemInfo
// ~~~~~ ~~~~~~~~~~~~~
class _OWLCLASS TMenuItemInfo : public MENUITEMINFO {
  public:
    TMenuItemInfo(uint msk = 0);

    /// Construct MENUITEMINFO from a specific item of a menu object.
    /// Defaults to retrieving all information except for the type, which
    /// require a destination buffer.
    //
    TMenuItemInfo(TMenu& menu, uint posOrId, bool isPos,
                  uint msk = MIIM_CHECKMARKS|MIIM_DATA|
                             MIIM_ID|MIIM_STATE|MIIM_SUBMENU);

    /// Construct MENUITEMINFO from a specific item of a menu object.
    /// Similar to prior except that it also retrieves the type data.
    //
    TMenuItemInfo(TMenu& menu, uint posOrId, bool isPos, void* buffer, 
                  uint size, uint msk = MIIM_ALL);
};


//
/// \class TMenu
// ~~~~~ ~~~~~
/// The TMenu class encapsulates window menus. You can use TMenu member functions to
/// construct, modify, query, and create menu objects. You can also use TMenu to add
/// bitmaps to your menu or to specify if a menu item is checked or unchecked. TMenu
/// includes two versions of a helper function, DeepCopy , designed to make copies
/// of menus and insert them at a specified position on the menu bar. See the
/// ObjectWindows Programmer's Guide for information about how to create menu
/// objects.
//
class _OWLCLASS TMenu {
  public:
    TMenu(TAutoDelete autoDelete = AutoDelete);
    TMenu(const TMenu& original, TAutoDelete autoDelete = AutoDelete);
    TMenu(HMENU handle, TAutoDelete autoDelete = NoAutoDelete);
    TMenu(HWND hWnd, TAutoDelete autoDelete = NoAutoDelete);
    TMenu(const void * menuTemplate);
    TMenu(HINSTANCE instance, TResId resId);
    virtual ~TMenu();

    TMenu& operator =(const TMenu&);

    void        CheckValid(uint redId = IDS_MENUFAILURE);
    virtual HMENU GetHandle() const;

    operator    uint() const;
    operator    HMENU() const;

    bool        IsOK() const;

    /// \name HMENU encapsulated functions
    /// @{
    bool        AppendMenu(uint flags, TMenuItem newItem=static_cast<unsigned int>(-1), LPCTSTR newStr=0); //JJH added static cast
    bool        AppendMenu(uint flags, TMenuItem newItem, const tstring& newStr) {return AppendMenu(flags, newItem, newStr.c_str());}
    bool        AppendMenu(uint flags, TMenuItem newitem, const TBitmap& newBmp);
    bool        CheckMenuItem(TMenuItem item, uint check);
    bool        DeleteMenu(TMenuItem item, uint flags);
    bool        EnableMenuItem(TMenuItem item, uint enable);
    uint        GetMenuItemCount() const;
    uint        GetMenuItemID(int posItem) const;
    uint        GetMenuState(TMenuItem item, uint flags) const;
    int         GetMenuString(TMenuItem item, LPTSTR str, int count, uint flags) const;
    tstring  GetMenuString(TMenuItem item, uint flags) const;
    HMENU       GetSubMenu(int posItem) const;
    bool        InsertMenu(TMenuItem item, uint flags, TMenuItem newItem=-1, LPCTSTR newStr=0);
    bool        InsertMenu(TMenuItem item, uint flags, TMenuItem newItem, const tstring& newStr) {return InsertMenu(item, flags, newItem, newStr.c_str());}
    bool        InsertMenu(TMenuItem item, uint flags, TMenuItem newItem, const TBitmap& newBmp);
    bool        ModifyMenu(TMenuItem item, uint flags, TMenuItem newItem=-1, LPCTSTR newStr=0);
    bool        ModifyMenu(TMenuItem item, uint flags, TMenuItem newItem, const tstring& newStr) {return ModifyMenu(item, flags, newItem, newStr.c_str());}
    bool        ModifyMenu(TMenuItem item, uint flags, TMenuItem newItem, const TBitmap& newBmp);
    bool        RemoveMenu(TMenuItem item, uint flags);
    bool        SetMenuItemBitmaps(TMenuItem item, uint flags,
                                   const TBitmap* bmpUnchecked=0,
                                   const TBitmap* bmpChecked=0);
    /// @}

    /// \name Encapsulation of new functionality introduced with Win95
    /// @{
    uint        GetDefaultItem(bool getPos, uint flags) const;
    bool        SetDefaultItem(uint posOrId, bool isPos);
    bool        CheckRadioItem(uint first, uint last, uint check, uint flags);
		/// @}

    /// \name MENUITEMINFO related API
    /// @{
    bool        GetMenuItemInfo(uint posOrId, bool isPos, TMenuItemInfo& mi) const;
    bool        SetMenuItemInfo(uint posOrId, bool isPos, TMenuItemInfo& mi);
    bool        InsertMenuItem(uint posOrId, bool isPos, TMenuItemInfo& mi);
    /// @}

    static bool  GetMenuCheckMarkDimensions(TSize& size);
    static TSize GetMenuCheckMarkDimensions();

    /// \name Virtual menu functions
    /// @{
    virtual void  MeasureItem(MEASUREITEMSTRUCT & measureItem);
    virtual void  DrawItem(DRAWITEMSTRUCT & drawItem);
    /// @}

  protected:
    static void DeepCopy(TMenu& dest, const TMenu& source, int offset = 0, int count = -1);
    static void DeepCopy(TMenu& dst, int dstOff, const TMenu& src, int srcOff = 0, int count = -1);

  protected_data:
    HMENU       Handle;					///< Holds the handle to the menu.
    bool        ShouldDelete;   ///< ShouldDelete is set to true if the Destructor needs to delete the handle to the menu.
};

//
/// \class TXMenu
// ~~~~~ ~~~~~~
/// A nested class, TXMenu describes an exception that occurs when a menu item
/// cannot be constructed.
class _OWLCLASS TXMenu : public TXOwl {
  public:
    TXMenu(uint resId = IDS_MENUFAILURE);

    virtual TXMenu* Clone() const; // override
    void Throw();

    static void Raise(uint resId = IDS_MENUFAILURE);
};

//
/// \class TSystemMenu
// ~~~~~ ~~~~~~~~~~~
/// TSystemMenu creates a system menu object that then becomes the existing system
/// menu.
class _OWLCLASS TSystemMenu : public TMenu {
  public:
    TSystemMenu(HWND wnd, bool revert= false);

  private:
    TSystemMenu();
    TSystemMenu(const TSystemMenu&);
};

//
/// \class TPopupMenu
// ~~~~~ ~~~~~~~~~~
/// TPopupMenu creates an empty pop-up menu to add to an existing window or pop-up
/// menu.
class _OWLCLASS TPopupMenu : public TMenu {
  public:
    TPopupMenu(TAutoDelete autoDelete = AutoDelete);
    TPopupMenu(HMENU handle, TAutoDelete autoDelete = NoAutoDelete);
    TPopupMenu(const TMenu& menu, TAutoDelete autoDelete = AutoDelete);
    int TrackPopupMenu(uint flags, int x, int y, int rsvd, HWND wnd, const TRect* rect=0);
    int TrackPopupMenu(uint flags, const TPoint& point, int rsvd, HWND wnd, const TRect* rect=0);

  private:
    TPopupMenu(const TPopupMenu&);
};

//
/// \class TMenuDescr
// ~~~~~ ~~~~~~~~~~
/// Menu with information used to allow merging
//
class _OWLCLASS TMenuDescr : public TMenu {
  public:
/// Used by TMenuDescr, the TGroup enum describes the following constants that
/// define the index of the entry in the GroupCount array.
    enum TGroup {
      FileGroup,			///< Index of the File menu group count
      EditGroup,			///< Index of the Edit menu group count
      ContainerGroup,	///< Index of the Container menu group count
      ObjectGroup,		///< Index of the Object group count
      WindowGroup,		///< Index of the Window menu group count
      HelpGroup,			///< Index of the Help menu group count
      NumGroups				///< Total number of groups
    };
    TMenuDescr();
    TMenuDescr(const TMenuDescr& original);
    TMenuDescr(TResId id, int fg, int eg, int cg, int og, int wg, int hg,
               TModule* module = &GetGlobalModule());
    TMenuDescr(TResId id, TModule* module = &GetGlobalModule());
    TMenuDescr(HMENU hMenu, int fg, int eg, int cg, int og, int wg, int hg,
               TModule* module = &GetGlobalModule());
   ~TMenuDescr();

    TMenuDescr& operator =(const TMenuDescr& original);

    HMENU    GetHandle() const;
    TModule* GetModule() const;
    void     SetModule(TModule* module);
    TResId   GetId() const;
    int      GetGroupCount(int group) const;
    
    void     ClearServerGroupCount(); ///< Not implemented?
    void     ClearContainerGroupCount(); ///< Not implemented?

    bool     Merge(const TMenuDescr& sourceMenuDescr);
    bool     Merge(const TMenuDescr& sourceMenuDescr, TMenu& destMenu);

  protected:
    bool     ExtractGroups();

  protected_data:
/// Points to the TModule object that owns this TMenuDescr.
    TModule* Module;       
    
/// The resource ID for the menu. The resource ID is passed in the constructors to
/// identify the menu resource.
    TResId   Id;           
    
/// An array of values indicating the number of pop-up menus in each group on the
/// menu bar.
    int      GroupCount[NumGroups];

  friend _OWLCFUNC(ipstream&) operator >>(ipstream& is, TMenuDescr& m);
  friend _OWLCFUNC(opstream&) operator <<(opstream& os, const TMenuDescr& m);
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Gets the handle to the menu, possibly causing any deferred menu acquisition to
/// occur.
inline HMENU TMenuDescr::GetHandle() const
{
  return TMenu::GetHandle();
}

//
/// Returns a pointer to the module object.
inline TModule* TMenuDescr::GetModule() const
{
  return Module;
}

//
/// Sets the default module object for this menu descriptor.
inline void TMenuDescr::SetModule(TModule* module)
{
  Module = module;
}

//
/// Gets the menu resource ID used to construct the menu descriptor.
inline TResId TMenuDescr::GetId() const
{
  return Id;
}

//
/// Gets the number of menus in a specified group within the menu bar. There are a
/// maximum of six functional groups as defined by the TGroup enum. These groups
/// include FileGroup,  EditGroup, ContainerGroup, ObjectGroup, WindowGroup, and
/// HelpGroup.
inline int TMenuDescr::GetGroupCount(int group) const
{
  return GroupCount[group];
}

//
/// Returns the handle to the menu.
inline HMENU TMenu::GetHandle() const
{
  return Handle;
}

//
/// Returns the menu's handle. This function provides compatibility with functions
/// that require a uint menu parameter.
inline TMenu::operator uint() const
{
  return uint(GetHandle());
}

//
/// Returns the menu's handle.
inline TMenu::operator HMENU() const
{
  return GetHandle();
}

//
/// Returns true if the menu has a valid handle.
inline bool TMenu::IsOK() const
{
  return GetHandle() != 0;
}

//
/// Adds a text menu item to the end of the menu.  See TMenu::GetMenuState for a
/// description of the flag values that specify the attributes of the menu, for
/// example, menu item is checked, menu item is a a bitmap, and so on.
inline bool TMenu::AppendMenu(uint flags, TMenuItem newItem, LPCTSTR newStr)
{
  PRECONDITION(Handle);
  return ::AppendMenu(Handle, flags, newItem, newStr);
}

//
/// Adds a bitmap menu item at the end of the menu. See TMenu::GetMenuState for a
/// description of the flag values that specify the attributes of the menu; for
/// example, menu item is checked, menu item is disabled, and so on.
inline bool TMenu::AppendMenu(uint flags, TMenuItem newItem, const TBitmap& newBmp)
{
  PRECONDITION(Handle);
  return ::AppendMenu(Handle, flags|MF_BITMAP, newItem,
         (LPCTSTR)HBITMAP(newBmp));
}

//
/// Checks or unchecks the menu item. By combining flags with the bitwise OR
/// operator (|) check specifies both the position of item (MF_BYCOMMAND,
/// MF_BYPOSITION) and whether item is to be checked (MF_CHECKED) or unchecked
/// (MF_UNCHECKED).
inline bool TMenu::CheckMenuItem(TMenuItem item, uint check)
{
  PRECONDITION(Handle);
  return ::CheckMenuItem(Handle, item, check);
}

//
/// Removes the menu item (item) from the menu or deletes the menu item if it's a
/// pop-up menu. flags is used to identify the position of the menu item by its
/// relative position in the menu (MF_BYPOSITION) or by referencing the handle to
/// the top-level menu (MF_BYCOMMAND).
inline bool TMenu::DeleteMenu(TMenuItem item, uint flags)
{
  PRECONDITION(Handle);
  return ::DeleteMenu(Handle, item, flags);
}

//
/// Enables, disables, or grays the menu item specified in the item parameter. If a
/// menu item is enabled (the default state), it can be selected and used as usual.
/// If a menu item is grayed, it appears in grayed text and cannot be selected by
/// the user. If a menu item is disabled, it is not displayed. Returns true if
/// successful.
inline bool TMenu::EnableMenuItem(TMenuItem item, uint enable)
{
  PRECONDITION(Handle);
  return ::EnableMenuItem(Handle, item, enable);
}

//
/// Returns the number of items in a top-level or pop-up menu.
inline uint TMenu::GetMenuItemCount() const
{
  PRECONDITION(Handle);
  return ::GetMenuItemCount(Handle);
}

//
/// Returns the menu flags for the menu item specified by item. flags specifies how
/// the item is interpreted, and is one of the following values:
/// - \c \b  MF_BYCOMMAND	Interpret item as a menu command ID. Default it neither
/// - \c \b  MF_BYCOMMAND nor MF_BYPOSITION is specified.
/// - \c \b  MF_BYPOSITION	Interpret item as the zero-base relative postion of the menu item
/// within the menu.
///
/// If item is found, and is a pop-up menu, the low-order byte of the return value
/// contains the flags associated with item, and the high-order byte contains the
/// number of items in the pop-up menu. If itemis not a pop-up menu, the return
/// value specifies a combination of these flags:
/// 
/// - \c \b  MF_BITMAP	Menu item is a a bitmap.
/// - \c \b  MF_CHECKED	Menu item is checked (pop-up menus only).
/// - \c \b  MF_DISABLED	Menu item is disabled.
/// - \c \b  MF_ENABLED	Menu item is enabled. \note this constant's value is 0.
/// - \c \b  MF_GRAYED	Menu item is disabled and grayed.
/// - \c \b  MF_MENUBARBREAK	Same as MF_MENUBREAK except pop-up menu columns are separated by
/// a vertical dividing line.
/// - \c \b  MF_MENUBREAK	Static menu items are placed on a new line, pop-up menu items are
/// placed in a new column, without separating columns.
/// - \c \b  MF_SEPARATOR	A horizontal dividing line is drawn, which cannot be enabled,
/// checked, grayed, or highlighted. Both item and flags are ingonred.
/// - \c \b  MF_UNCHECKED	Menu item check mark is removed (default). \note this constant
/// value is 0.
///
/// Returns -1 if item doesn't exist.
//
inline uint TMenu::GetMenuState(TMenuItem item, uint flags) const
{
  PRECONDITION(Handle);
  return ::GetMenuState(Handle, item, flags);
}

//
/// Returns the label (str) of the menu item (item).
inline int TMenu::GetMenuString(TMenuItem item, LPTSTR str, int count, uint flags) const
{
  PRECONDITION(Handle);
  return ::GetMenuString(Handle, item, str, count, flags);
}

//
/// Returns the handle of the menu specified by posItem.
inline HMENU TMenu::GetSubMenu(int posItem) const
{
  PRECONDITION(Handle);
  return ::GetSubMenu(Handle, posItem);
}

//
/// Inserts a new text menu item or pop-up menu into the menu after the menu item
/// specified in item. The flagsparameter contains either the MF_BYCOMMAND or
/// MF_BYPOSITION values that indicate how to interpret the item parameter. If
/// MF_BYCOMMAND, item is a command ID; if MF_BYPOSITION, item holds a relative
/// position within the menu.
inline bool TMenu::InsertMenu(TMenuItem item, uint flags, TMenuItem newItem, LPCTSTR newStr)
{
  PRECONDITION(Handle);
  return ::InsertMenu(Handle, item, flags|MF_STRING, newItem, newStr);
}

//
/// Adds a bitmap menu item after the menu item specified in item. The flags
/// parameter contains either the MF_BYCOMMAND or MF_BYPOSITION values that indicate
/// how to interpret the item parameter. If MF_BYCOMMAND, item is a command ID; if
/// MF_BYPOSITION, item holds a relative position within the menu.
inline bool TMenu::InsertMenu(TMenuItem item, uint flags, TMenuItem newItem, const TBitmap& newBmp)
{
  return ::InsertMenu(Handle, item, flags|MF_BITMAP, newItem,
         (LPCTSTR)HBITMAP(newBmp));
}

//
/// Changes an existing menu item from the item specified in item to newItem. The
/// flags parameter contains either the MF_BYCOMMAND or MF_BYPOSITION values that
/// indicate how to interpret the item parameter. If MF_BYCOMMAND, item is a command
/// ID; if MF_BYPOSITION, item holds a relative position within the menu.
inline bool TMenu::ModifyMenu(TMenuItem item, uint flags, TMenuItem newItem, LPCTSTR newStr)
{
  PRECONDITION(Handle);
  return ::ModifyMenu(Handle, item, flags|MF_STRING, newItem, newStr);
}

//
/// Changes an existing menu item into a bitmap. The flags parameter contains either
/// the MF_BYCOMMAND or MF_BYPOSITION values that indicate how to interpret the item
/// parameter. If MF_BYCOMMAND, item is a command ID; if MF_BYPOSITION, item holds a
/// relative position within the menu.
inline bool TMenu::ModifyMenu(TMenuItem item, uint flags, TMenuItem newItem, const TBitmap& newBmp)
{
  PRECONDITION(Handle);
  return ::ModifyMenu(Handle, item, flags|MF_BITMAP, newItem,
         (LPCTSTR)HBITMAP(newBmp));
}

//
/// Removes the menu item from the menu but does not delete it if it is a submenu.
inline bool TMenu::RemoveMenu(TMenuItem item, uint flags)
{
  PRECONDITION(Handle);
  return ::RemoveMenu(Handle, item, flags);
}

//
/// Specifies the bitmap to be displayed when the menu item is checked and
/// unchecked. item indicates the menu item to be associated with the bitmap. flags
/// indicates how the size parameter is interpreted (whether by MF_BYPOSITION or by
/// MF_BYCOMMAND). GetMenuCheckMarkDimensions gets the size of the bitmap.
inline bool TMenu::SetMenuItemBitmaps(TMenuItem item, uint flags,
                                      const TBitmap* bmpUnchecked,
                                      const TBitmap* bmpChecked)
{
  PRECONDITION(Handle);
  return ::SetMenuItemBitmaps(Handle, item, flags,
                              bmpUnchecked ? HBITMAP(*bmpUnchecked) : 0,
                              bmpChecked ? HBITMAP(*bmpChecked) : 0);
}

//
/// Gets the size of the bitmap used to display the default check mark on checked
/// menu items.
/// Always returns true. The size reference stores the dimensions of the checkmark
/// bitmaps.
//
inline bool TMenu::GetMenuCheckMarkDimensions(TSize& size)
{
  size = TSize(::GetMenuCheckMarkDimensions());
  return true;
}

//
/// Return the dimensions of the check mark bitmaps.
//
inline TSize TMenu::GetMenuCheckMarkDimensions()
{
  return TSize(::GetMenuCheckMarkDimensions());
}

//
/// Allows the application to create a pop-up menu at the specified location in the
/// specified window. flags specifies a screen position and can be one of the
/// TPM_xxxx values (TPM_CENTERALIGN, TPM_LEFTALIGN, TPM_RIGHTALIGN, TPM_LEFTBUTTON,
/// or TPM_RIGHTBUTTON). wnd is the handle to the window that receives messages
/// about the menu. x specifies the horizontal position in screen coordinates of the
/// left side of the menu. y species the vertical position in screen coordinates of
/// the top of the menu (for example, 0,0 specifies that a menu's left corner is in
/// the top left corner of the screen). rect defines the area that the user can
/// click without dismissing the menu.
///
/// If you specify TPM_RETURNCMD in the flags parameter, the return value is the 
/// menu-item identifier of the item that the user selected. If the user cancels 
/// the menu without making a selection, or if an error occurs, then the return 
/// value is zero.
///
/// If you do not specify TPM_RETURNCMD in the flags parameter, the return value 
/// is nonzero if the function succeeds and zero if it fails. To get extended 
/// error information, call GetLastError.
//
inline int TPopupMenu::TrackPopupMenu(uint flags, int x, int y, int rsvd,
                                       HWND wnd, const TRect* rect)
{
  PRECONDITION(GetHandle());
  return ::TrackPopupMenu(GetHandle(), flags, x, y, rsvd, wnd, rect);
}

//
/// TPoint-aware overload
//
inline int TPopupMenu::TrackPopupMenu(uint flags, const TPoint& point,
                                       int rsvd, HWND wnd, const TRect* rect)
{
  PRECONDITION(GetHandle());
  return ::TrackPopupMenu(GetHandle(), flags, point.x, point.y, rsvd, wnd, rect);
}

} // OWL namespace


#endif  // OWL_MENU_H
