//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software
// All rights reserved.
//
//
// TExpandableComboBox class; original code by:
//----------------------------------------------------------------
// Copyright (c) Antony Corbett 1995. All Rights Reserved
// Author:  Antony Corbett
//        Compuserve  100277,330
//        INTERNET    antony@corbetteng.co.uk
//        Tel +44 (1926) 856131,  Fax +44 (1926) 856727
// Updated: Sergio G. Olivas
//          Parker Hannifin
//          Converted to ComboBox Class
//          06/10/97
//
// Description:
//    Header file for TExpandableComboBox classes.
//
//
//    TComboBox derivatives that provide disabling of selected
//    list items, horizontal scrolling
//
//-------------------------------------------------------------------
#if !defined(__OWLEXT_COMBOEXP_H)
#define __OWLEXT_COMBOEXP_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H) && !defined (__OWLEXT_ALL_H)
# include <owlext/core.h>
#endif


// Grab necessary OWL headers
//
#if !defined(OWL_COMBOBOX_H)
  #include <owl/combobox.h>
#endif

// Grab necessary OWL headers
//
#if !defined(OWL_TEMPLATE_H)
  #include <owl/template.h>
#endif

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>


// nUnitWidth
//  if +X  Fixed
//  if 0  No Change in Size
//  if -1 AutoSize to largest string, but not larger than dialog width
//  if -X Size given is maximum Width allowable, will resize to less



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                   TCBItemData
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Used to store enable status of an item in the list. Also retains 32-bit item
// data. A ptr to a TCBItemData object is associated with each item added to the
// list using SeExpItemData.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct OWLEXTCLASS TCBItemData {
  bool enabled_;
  bool useSysColor_;
  LPARAM data_;
  owl::TColor textColor_;

  TCBItemData()
    : enabled_(true), data_(0), useSysColor_(true)
  { }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                           TExpandableComboBox
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Provides horizontal scrolling, disabling of selected items. Color Highlights
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class OWLEXTCLASS TExpandableComboBox : public owl::TComboBox {
  // Enumerations
  //
  public:
    enum eExpAlign{
      eExpAlignDefault,  // Default Left Alignment
      eExpAlignAuto,     // Not Implemented
      eExpAlignLeft,     // Aligns (fixes) left side
      eExpAlignRight,    // Aligns (fixes) right side
      eExpAlignCenter,   // Not Implementd
    };

  // Object lifetime methods
  //
  public:
    TExpandableComboBox(owl::TWindow* parent, int resourceId,
                        eExpAlign fAlign = eExpAlignLeft,
                        int nUnitWidth = -1,
                        owl::TModule* module = 0);
    ~TExpandableComboBox();

  // Accessors
  //
  public:
    int            GetStringX(LPTSTR str, int nIndex) const;
    bool          GetSel(int index) const;
    int            GetSelCount() const;
    bool          GetEnabled(int index) const;
    TCBItemData*  GetItemDataPtr(int index) const;

    // TComboBox overrides
    //
    auto GetItemData(int index) const -> LPARAM override;
    auto GetSelIndex() const -> int override;

  // Mutators
  //
  public:
    void  ResizeToFit();
    int    AddDisabledString(LPCTSTR str);
    void  Enable(int index, bool enable = true);
    int    SetSel(int index, bool select);
    void  SetTextColor(int index, owl::TColor color);
    void  ResetTextColor(int index);
    bool  SetTabStops(int numTabs, int* tabs, bool bShowCompleteLine = false);
    bool  MakeSingleSel(bool single = true);

    // TComboBox overrides
    //
    auto AddString(LPCTSTR) -> int override;
    int AddString(const owl::tstring& str) {return owl::TComboBox::AddString(str);}
    auto InsertString(LPCTSTR, int index) -> int override;
    int InsertString(const owl::tstring& str, int index) {return owl::TComboBox::InsertString(str, index);}
    auto DeleteString(int index) -> int override;
    auto SetItemData(int index, LPARAM itemData) -> int override;
    auto SetSelIndex(int index) -> int override;
    void ClearList() override;

    // Misc "helper" methods
    //
  public:
    int    UnitsToPixels(int nUnits);
    int    PixelsToUnits(int nPixels);

    // OWL overridden methods
    //
  protected:
    void SetupWindow() override;
    void CleanupWindow() override;
    LRESULT DefWindowProc(owl::uint msg, WPARAM wParam, LPARAM lParam);
    void ODADrawEntire(DRAWITEMSTRUCT&) override;
    void ODAFocus(DRAWITEMSTRUCT&) override;
    void ODASelect(DRAWITEMSTRUCT&) override;
    DECLARE_RESPONSE_TABLE(TExpandableComboBox);
    void  EvMeasureItem(UINT ctrlId, MEASUREITEMSTRUCT &measure);
    void  CmKillFocus();
    void  CmSelChange();
    void  CmSelEndOk();

    virtual void    DrawListItem(DRAWITEMSTRUCT& drawInfo, LPCTSTR str);
    virtual int      FindTextExtent(LPCTSTR str);
    virtual owl::TColor  QueryTextColor(DRAWITEMSTRUCT& drawInfo) const;
    virtual owl::TColor  QueryBkColor(DRAWITEMSTRUCT& drawInfo) const;
    void            StoreStrExtent(LPCTSTR str);
    virtual void    DrawText(DRAWITEMSTRUCT& drawInfo,const owl::TRect& rc, LPCTSTR str);
    void            UpdateHorizontalExtent();
    bool            IsMultipleSel() const;

    // Internal data
    //
  private:
    HWND     hwndList;
    owl::TRect   RectList;
    int     greatestExtent;
    int     nUnitWidth_;
    int     nItemHeight;
    eExpAlign fTCBAlign_;
    bool    bShowCompleteLine_;
    int     nPasses;
    bool     bFirstRedraw;
    bool     hasHScroll_;
    int     numTabs_;
    int*     tabs_;

    typedef owl::TSortedObjectArray<int> SortedIntegers;
    SortedIntegers& textExtents_;

  protected:
    bool pseudoSingleSel_;  // forced into single-selection mode?
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations

inline bool TExpandableComboBox::IsMultipleSel() const
{
  return false;
}

} // OwlExt namespace

#endif  //__OWLEXT_COMBOEXP_H
