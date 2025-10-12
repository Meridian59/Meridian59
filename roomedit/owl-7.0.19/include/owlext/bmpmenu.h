// ****************************************************************************
// OWL Extensions (OWLEXT) Class Library
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// bmpmenu.cpp: implementation file
// Version:     1.7
// Date:        04/24/1999
// Author:      Dieter Windau
//
// TBmpMenu is a freeware OWL class that shows bitmaps
// on the left side of menus like MS Office 97
//
// Portions of code are based on MFC class CMenuSpawn written by Iuri
// Apollonio and BCMenu written by Brent Corkum. Very special thanks.
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// The code was tested using Microsoft Visual C++ 6.0 SR2 with OWL6 patch 5
// and Borland C++ 5.02 with OWL 5.02. Both under Windows NT 4.0 SP4.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// Many thanks to Riho Ellermaa, Michael Mogensen, Jurgen Welzenbach,
// Rob Beckers, Jo Parrello, Mark Hatsell an Yura Bidus for their help
// in testing and fixing the TBmpMenu classes.
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://www.members.aol.com/softengage/index.htm
//
// ****************************************************************************
#if !defined (__OWLEXT_BMPMENU_H) && !defined (BMPMENU_H)
#define __OWLEXT_BMPMENU_H
#define __BMPMENU_H
#define BMPMENU_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H) && !defined (__OWLEXT_ALL_H)
# include <owlext/core.h>
#endif

#if !defined(OWL_IMAGELST_H)
#include <owl/imagelst.h>
#endif
#if !defined(OWL_CONTAIN_H)
#include <owl/contain.h>
#endif
#include <owl/template.h>
#if !defined(OWL_DECFRAME_H)
#include <owl/decframe.h>
#endif

namespace OwlExt {

const int MaxMenuStringLen    = 128;

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

// ********************* TBmpMenuItem *****************************************

class OWLEXTCLASS TBmpMenuItem {
  public:
    HMENU  Menu;
    int   ImageIdx;
    int   Cmd;
    owl::uint  Pos;
    TCHAR Text[MaxMenuStringLen];

  public:
    bool operator ==(const TBmpMenuItem& d) const;
};

// ********************* TBmpMenuImageItem ************************************

class OWLEXTCLASS TBmpMenuImageItem {
  public:
    int    ImageIdx;
    int    Cmd;

  public:
    TBmpMenuImageItem(int imageIdx, int cmd);

  public:
    bool operator ==(const TBmpMenuImageItem& d) const;
};

// *************************** TBmpMapEntry ***********************************

class OWLEXTCLASS TBmpMapEntry {
  public:
    int CommandId;
    int BmpId;

  public:
    TBmpMapEntry(int commandId, int bmpId);

  public:
    bool operator ==(const TBmpMapEntry& d) const;
};

// *************************** TBmpMenu ***************************************

class OWLEXTCLASS TBmpMenu {
  public:
    TBmpMenu(owl::TModule* module = 0);
    ~TBmpMenu();

  public:
    void RemapMenu(HMENU hMenu, bool isPopup=false);
    // Remap a simple menu created with Windows API functions or with OWL TMenu
    // class to a menu with bitmaps on the left side. Should be called from e.g:
    // SetupWindow(), EvInitMenu(), EvInitMenuPopup() functions and before the
    // TrackPopupMenu() function.
    // If the hMenu is a popupMenu set isPopup = true

    bool DrawItem(DRAWITEMSTRUCT & DIS);
    // To be called from the TWindow EvDrawItem(..)
    // Call the base class if it return false

    bool MeasureItem(MEASUREITEMSTRUCT & MIS);
    // To be called from the TWindow EvMeasureItem(..)
    // Call the base class if it return false

    bool FindKeyboardShortcut(owl::uint nChar, owl::uint nFlags, HMENU hMenu, LRESULT& lRes);
    // To be called from the TWindow EvMenuChar(..)
    // Call the base class if it return false

    int PreGetHintText(HMENU hmenu, owl::uint id, LPTSTR buf, int size, owl::THintText hintType);
    // Retrieves the hint text associated with a particular Id
    // To be called from DecoratedFrame GetHintText(...)
    // Call the base class if it return 0.

    bool IsDrawRecessed();
    void SetDrawRecessed(bool b);
    // Get/Set the style how to draw top level items

    bool IsWithAccel();
    void SetWithAccel(bool b);
    // Get/Set the showing of accelerators in tooltips

    bool IsBackBitmap();
    bool SetBackBitmap(owl::uint resId);
    // Set the background bitmap. If resId == 0 all menus will be show normal.
    // return true if successful

    bool IsRadioBitmap();
    bool SetRadioBitmap(owl::uint resId);
    // Set the radio dot bitmap. If resId == 0 a default radio dot will be draw.
    // return true if successful

    bool IsCheckBitmap();
    bool SetCheckBitmap(owl::uint resId);
    // Set the checkbox bitmap. If resId == 0 a default checkbox will be draw.
    // return true if successful

    owl::TFont* GetMenuFont();
    void   SetMenuFont(owl::TFont* menuFont);
    // Get/Set the menu font. The Font will be deleted in this class
    // By default the windows font for menus is used

    owl::TColor GetMaskColor();
    void   SetMaskColor(const owl::TColor& maskColor);
    // Get/Set the color of pixels that will be mask to the menu color
    // By default the mask color is light gray

    void ExcludeBitmap(int resId);
    // Exclude the bitmap with resId. It will not be shown in menus
    void IncludeBitmap(int resId);
    // Include the bitmap with resId. By default all bitmaps in module
    // will be shown in menus

    int  AddMapEntry(TBmpMapEntry* entry);
    // Adds a entry into the array MapEntries

    void RemoveMapEntry(int loc);
    // Removes entry at loc, and reduces the array by one element

    // ------ Some simple functions to change menu on the fly
    //
    // If you want to change more, use OWL or Windows API function.
    // But then you must remap the menu.
    //
    bool SetMenuItemText(HMENU hMenu, UINT pos, LPCTSTR txt);
    // Set the menu item text like Win API function:
    //   ModifyMenu(hMenu, pos, MF_BYPOSITION | MF_STRING, .., txt)

  protected:
    void RemapMenuRecursiv(HMENU hMenu);
    void DrawCheckmark(owl::TDC& hdc, int x, int y, bool enable, bool select);
    void DrawRadioDot(owl::TDC& dc, int x, int y, bool enable, bool select);
    int  FindImageItem(int cmd);
    TBmpMenuItem* AddMenuItem(HMENU hMenu, LPCTSTR txt, int cmd, UINT pos);
    void AddImageItem(int idx, WORD cmd);
    bool TryToLoadBmpResource(unsigned int resId);
    void SyncronizeAllMenuItems();

  protected:
    bool        bShowAccInTooltips; // show accelerators in tooltips, e.g. New (Strg+N)
    bool        bDrawRecessed;    // show top level menu items recessed
    bool        bIsPopup;         // true if the remap menu is a popup menu
    owl::TSize       ImageSize;        // the size that is used for all bitmaps
    owl::TImageList* ImageList;        // contains all used bitmaps (each bmp one time)
    owl::TBitmap*    BackBitmap;       // background bitmap
    owl::uint        RadioBitmapId;    // resource id of radio dot bitmap
    owl::uint        CheckBitmapId;    // resource id of checkbox bitmap
    owl::TFont*      MenuFont;         // MenuFont to display text in all menus
    owl::TColor      MaskColor;        // Color of pixels that will be mask to the menu
    owl::TColor      Old3dFaceColor;
    owl::TModule*    Module;

  private:
    owl::TIPtrArray<TBmpMapEntry*>*      MapEntries;
    owl::TIntArray*                      ExcludedBitmaps;
    owl::TIPtrArray<TBmpMenuItem*>*      MenuItems;
    owl::TIPtrArray<TBmpMenuImageItem*>*  ImageItems;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif   // __OWLEXT_BMPMENU_H
