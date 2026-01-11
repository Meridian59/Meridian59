//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of classes TMenu and TXMenu, Window Menu encapsulation class
/// and its associated exception
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/menu.h>
#include <owl/module.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlMenu, 1, 0); // diag. group for menus
DIAG_DECLARE_GROUP(OwlWin);                     // diag. group for windows

//
//
//
TMenuItemInfo::TMenuItemInfo(uint msk)
{
  memset(LPMENUITEMINFO(this), 0, sizeof(MENUITEMINFO));
  cbSize = sizeof(MENUITEMINFO);
  fMask = msk;
}

//
//
//
TMenuItemInfo::TMenuItemInfo(TMenu& menu, uint posOrId, bool isPos, uint msk)
{
  PRECONDITION(menu.GetHandle());
  memset(LPMENUITEMINFO(this), 0, sizeof(MENUITEMINFO));
  cbSize = sizeof(MENUITEMINFO);
  fMask = msk;
  menu.GetMenuItemInfo(posOrId, isPos ? TRUE : FALSE, *this);
}

//
//
//
TMenuItemInfo::TMenuItemInfo(TMenu& menu, uint posOrId, bool isPos,
                             void* buffer, uint size, uint msk)
{
  PRECONDITION(menu.GetHandle());
  memset(LPMENUITEMINFO(this), 0, sizeof(MENUITEMINFO));
  cbSize = sizeof(MENUITEMINFO);
  fMask = msk;
  dwTypeData = LPTSTR(buffer);
  cch = size;
  menu.GetMenuItemInfo(posOrId, isPos ? TRUE : FALSE, *this);
}


//
/// Creates an empty menu and sets autoDelete, by default, so that the menu is
/// automatically deleted when the object is destroyed.
//
TMenu::TMenu(TAutoDelete autoDelete)
:
  Handle(::CreateMenu()),
  ShouldDelete(autoDelete)
{
  WARNX(OwlMenu, !Handle, 0, "Cannot Create Menu");
  CheckValid();
}

//
/// Creates a deep copy of an existing menu and sets autoDelete, by default, so
/// that the menu is automatically deleted when the object is destroyed.
//
TMenu::TMenu(const TMenu& original, TAutoDelete autoDelete)
:
  Handle(::CreateMenu()),
  ShouldDelete(autoDelete)
{
  WARNX(OwlMenu, !Handle, 0, "Cannot Create Menu for Copy");
  CheckValid();
  DeepCopy(*this, original);
}

//
/// Creates a menu object from an already loaded menu and sets autoDelete, by
/// default, so the menu is not automatically deleted when the object is destroyed.
//
TMenu::TMenu(HMENU handle, TAutoDelete autoDelete)
:
  Handle(handle),
  ShouldDelete(autoDelete)
{
}

//
/// Creates a menu object representing the window's current menu and sets
/// autoDelete, by default, so that the menu is not automatically deleted when the
/// object is destroyed.
//
TMenu::TMenu(HWND hWnd, TAutoDelete autoDelete)
:
  Handle(::GetMenu(hWnd)),
  ShouldDelete(autoDelete)
{
  PRECONDITION(hWnd);
  WARNX(OwlMenu, !Handle, 0, "Cannot Get Menu from " << static_cast<void*>(hWnd));
  CheckValid();
}

//
/// Creates a menu object from a menu template in memory. This constructor is not
/// available under Presentation Manager.
//
TMenu::TMenu(const void * menuTemplate)
{
  PRECONDITION(menuTemplate);
  Handle = ::LoadMenuIndirect(menuTemplate);
  WARNX(OwlMenu, !Handle, 0, "Cannot Load Menu Indirect " << menuTemplate);
  CheckValid();
  ShouldDelete = true;
}

//
/// Creates a menu object from a specified resource ID.
//
TMenu::TMenu(HINSTANCE resInstance, TResId resId)
{
  PRECONDITION(resInstance && resId);
  Handle = ::LoadMenu(resInstance, resId);
  WARNX(OwlMenu, !Handle, 0, "Cannot Load Menu " << static_cast<void*>(resInstance) << " " << resId);
  CheckValid();
  ShouldDelete = true;
}

//
/// Copies an existing menu onto this menu, using DeepCopy.
//
TMenu&
TMenu::operator =(const TMenu& original)
{
  // Delete all items and submenus
  // Look at possible alternatives for !ShouldDelete menus? Maybe use Remove
  // then?
  //
  while (GetMenuItemCount())
    DeleteMenu(0, MF_BYPOSITION);
  DeepCopy(*this, original);
  return *this;
}

//
// Destruct the menu by destroying the handle if appropriate.
//
TMenu::~TMenu()
{
  if (ShouldDelete && Handle)
    ::DestroyMenu(Handle);
}

//
/// Returns the ID of the menu item at the position specified by posItem.
/// If it's a regular menu item just returns its id.
/// If the menu is a pop-up menu, first attempt to retrieve a user-specified Id
/// via the MENUITEMINFO structure. Otherwise, use the Id of
/// first menuitem minus 1.
//
uint
TMenu::GetMenuItemID(int pos) const
{
  // Retrieve identifier
  //
  uint  id = ::GetMenuItemID(Handle, pos);
  if (id != uint(-1)) {
    TRACEX(OwlWin, 1, "GetMenuItemID at pos. " << pos << '=' <<  id);
    return id;
  }

// !JGD Popups don't have separators
// !  // Skip separators
// !  //
// !  uint state = GetMenuState(pos, MF_BYPOSITION);
// !  if (state & MF_SEPARATOR) {
// !    TRACEX(OwlWin, 1, "MenuItem at " << pos << "is a separator, id=0");
// !    return 0;
// !  }

  // Try to retrieve a pop-up menu handle
  //
  HMENU popup = GetSubMenu(pos);
  if (popup) {
    TMenu subMenu(popup);

    // Here could try to retrieve the 'true' id of the popup if the user
    // provided one before proceeding with the 'Id of firt Item -1' OWL
    // enhancement. Need more information about MENUITEMINFO etc.
    // !!

    // Recurse within sub menu to retrieve popup Id.
    //
    id = subMenu.GetMenuItemID(0) - 1;
    TRACEX(OwlWin, 1, "TMenu::GetMenuItemID - returns faked [Id of first "\
                       "menu item - 1, id=" << dec << id);
    return id;
  }
  TRACEX(OwlWin, 1, "TMenu::GetMenuItemID - unable to determine id - "\
                     "returns 0");
  return 0;
}

//
/// For use with CopyText
//

struct TMenuGetMenuString
{
  const TMenu& menu;
  uint item;
  uint flags;
  TMenuGetMenuString(const TMenu& m, uint i, uint f) : menu(m), item(i), flags(f) {}

  int operator()(LPTSTR buf, int bufSize)
  {return menu.GetMenuString(item, buf, bufSize, flags);}
};

//
/// String-aware overload
//
tstring
TMenu::GetMenuString(uint item, uint flags) const
{
  int length = ::GetMenuString(GetHandle(), item, nullptr, 0, flags);
  return CopyText(length, TMenuGetMenuString(*this, item, flags));
}

#if !defined(BI_COMP_GNUC)
#pragma warn -par  // resId param is never used in small model
#endif
//
/// Throws a TXMenu exception if the menu object is invalid.
//
void
TMenu::CheckValid(uint resId)
{
  if (!Handle)
    TXMenu::Raise(resId);
}
#if !defined(BI_COMP_GNUC)
#pragma warn .par
#endif

//
/// MeasureItem is used by owner-drawn controls to store the dimensions of the
/// specified item.
//
void
TMenu::MeasureItem(MEASUREITEMSTRUCT &)
{
}

//
/// DrawItem responds to a message forwarded to a drawable control by TWindow when
/// the control needs to be drawn.
//
void
TMenu::DrawItem(DRAWITEMSTRUCT &)
{
}

//
//
//
uint
TMenu::GetDefaultItem(bool getPos, uint flags) const
{
  PRECONDITION(Handle);
  return ::GetMenuDefaultItem(Handle, getPos ? TRUE : FALSE, flags);
}

//
//
//
bool
TMenu::SetDefaultItem(uint posOrId, bool isPos)
{
  PRECONDITION(Handle);
  return ::SetMenuDefaultItem(Handle, posOrId, isPos ? TRUE : FALSE);
}

//
//
//
bool
TMenu::CheckRadioItem(uint first, uint last, uint check, uint flags)
{
  PRECONDITION(Handle);
  return ::CheckMenuRadioItem(Handle, first, last, check, flags) != FALSE;
}

//
//
//
bool
TMenu::GetMenuItemInfo(uint posOrId, bool isPos, TMenuItemInfo& mi) const
{
  PRECONDITION(Handle);
  return ::GetMenuItemInfo(Handle, posOrId, isPos ? TRUE : FALSE, &mi) != FALSE;
}

//
//
//
bool
TMenu::SetMenuItemInfo(uint posOrId, bool isPos, TMenuItemInfo& mi)
{
  PRECONDITION(Handle);
  return ::SetMenuItemInfo(Handle, posOrId, isPos ? TRUE : FALSE, &mi) != FALSE;
}

//
//
//
bool
TMenu::InsertMenuItem(uint posOrId, bool isPos, TMenuItemInfo& mi)
{
  PRECONDITION(Handle);
  return ::InsertMenuItem(Handle, posOrId, isPos ? TRUE : FALSE, &mi) != FALSE;
}

//
/// \overload
//
void TMenu::DeepCopy(TMenu& dst, const TMenu& src, int srcOffset, int itemCount)
{
  DeepCopy(dst, dst.GetMenuItemCount(), src, srcOffset, itemCount);
}

//
/// Copies menu items from the \p src menu to the \p dst menu.
/// 
/// If \p itemCount is negative (the default), all of the source menu items from \p srcOffset
/// onward are copied.
/// 
/// \note The function isn't doing a truly deep copy. Only a recursive shallow copy is made of the
/// menu items and submenus. In particular, a deep copy of bitmaps is *not* made. The function only
/// copies the handles as part of the menu item structure, which means that bitmaps are in effect
/// shared between the \p src and \p dst menus after the copy.
//
void TMenu::DeepCopy(TMenu& dst, int dstOffset, const TMenu& src, int srcOffset, int itemCount)
{
  PRECONDITION(src.Handle);
  PRECONDITION(dst.Handle);
  PRECONDITION(srcOffset >= 0);

  if (dstOffset < 0)
    dstOffset = dst.GetMenuItemCount(); // Effectively, this means we will append items.

  if (itemCount < 0)
    itemCount = src.GetMenuItemCount() - srcOffset;

  for (int i = 0; i < itemCount; i++)
  {
    const uint srcPos = srcOffset + i;
    const uint dstPos = dstOffset + i;

    uint state = src.GetMenuState(srcPos, MF_BYPOSITION);
    if (state == static_cast<uint>(-1)) // Item does not exist?
      return;

    tchar str[256];
    src.GetMenuString(srcPos, str, COUNTOF(str), MF_BYPOSITION);

    if (state & MF_POPUP)
    {
      state &= (MF_STRING | MF_POPUP);
      TMenu subMenu(src.GetSubMenu(srcPos));
      TMenu newSubMenu(NoAutoDelete);
      DeepCopy(newSubMenu, subMenu);
      dst.InsertMenu(dstPos, state | MF_BYPOSITION, newSubMenu, str);
    }
    else
    {
      // Get all the known information about the item, except the string, which we have already got.
      //
      TMenuItemInfo info(MIIM_STATE | MIIM_ID | MIIM_SUBMENU | MIIM_CHECKMARKS | MIIM_DATA | MIIM_BITMAP | MIIM_FTYPE);
      const bool ok = src.GetMenuItemInfo(srcPos, true, info);
      WARNX(OwlMenu, !ok, 0, _T("TMenu::DeepCopy: GetMenuItemInfo failed!"));
      if (!ok) continue;
      info.dwTypeData = str;
      info.fMask |= MIIM_STRING;

      dst.InsertMenuItem(dstPos, true, info);
    }
  }
}

//----------------------------------------------------------------------------

//
/// Constructs a system menu object. If revert is true, then the menu created is a
/// default system menu. Otherwise, it is the menu currently in the window.
//
TSystemMenu::TSystemMenu(HWND hWnd, bool revert)
:
  TMenu(::GetSystemMenu(hWnd, revert), NoAutoDelete)
{
}

//----------------------------------------------------------------------------

//
/// Constructs an empty pop-up menu.
//
TPopupMenu::TPopupMenu(TAutoDelete autoDelete)
:
  TMenu(::CreatePopupMenu(), autoDelete)
{
}

//
/// Creates a popup menu based on an existing popup menu.
//
TPopupMenu::TPopupMenu(HMENU handle, TAutoDelete autoDelete)
:
  TMenu(handle, autoDelete)
{
}

//
/// Creates a popup menu from an existing menu.
//
TPopupMenu::TPopupMenu(const TMenu& menu, TAutoDelete autoDelete)
:
  TMenu(::CreatePopupMenu(), autoDelete)
{
  DeepCopy(*this, menu);
}

//----------------------------------------------------------------------------

//
/// Constructs a TXMenu exception object with a default IDS_GDIFAILURE message.
//
TXMenu::TXMenu(uint resId)
:
  TXOwl(resId)
{
}


//
/// Create a copy of the TXMenu exception.
/// It will be rethrown at a safer time.
//
TXMenu*
TXMenu::Clone()
{
  return new TXMenu(*this);
}


//
/// Throws a TXMenu exception.
//
void
TXMenu::Throw()
{
  throw *this;
}

//
/// Creates a TXMenu exception and throws it.
//
void
TXMenu::Raise(uint resId)
{
  TXMenu(resId).Throw();
}


} // OWL namespace
/* ========================================================================== */

