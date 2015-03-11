//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TTabItem and TTabControl.
//----------------------------------------------------------------------------

#if !defined(OWL_TABCTRL_H)
#define OWL_TABCTRL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/commctrl.h>
#include <owl/contain.h>
#include <owl/updown.h>


namespace owl {

// Mentioned in documentation but currently missing from system headers
//
#if !defined(TCIF_ALL)
# define TCIF_ALL  (TCIF_TEXT|TCIF_IMAGE|TCIF_PARAM)
#endif

// Forward ref.
//
class _OWLCLASS TFont;
class _OWLCLASS TTabControl;
class _OWLCLASS TTabEntryInternal;

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

//
/// \class TTabItem
// ~~~~~ ~~~~~~~~
///  TTabItem encapsulates the attributes of a single tab within a 
///  tab control. For example, it holds a weak pointer to the string containing the
///  tab's text.
/// TODO: Store the tab label text, and provide string-aware overloads.
//
class _OWLCLASS TTabItem : public TCITEM {
  public:
    TTabItem(uint mask);
    TTabItem(const TC_ITEM& tbItem);
    TTabItem(const TTabControl& ctl, int index, uint mask,
             int buffLen = 0, tchar * buffer = 0);

    // Used for setting label
    //
    TTabItem(LPCTSTR str, int buffLen = 0, TParam2 param = 0);

    // Used for setting image
    //
    TTabItem(int imageIndex, TParam2 param);

    // Used for setting both image and label
    //
    TTabItem(int imageIndex, LPCTSTR str);

    void    SetState(uint32 state, uint32 mask); // Version 4.70
    void    SetLabel(LPCTSTR str, int len = 0);
    void    SetIcon(int imageIndex);
    void    SetParam(TParam2 lp);
};


//
/// \class TTabHitTestInfo
// ~~~~~ ~~~~~~~~~~~~~~~
///  TTabHitTestInfo is a thin [very thin] wrapper around
///  the TC_HITTESTINFO structure. It's a place-holder for
///  future ObjectWindows enhancements for tabcontrol hit
///  testing..
//
class _OWLCLASS TTabHitTestInfo : public TC_HITTESTINFO {
  public:
    TTabHitTestInfo(){}
};

//
/// \class TTabControl
// ~~~~~ ~~~~~~~~~~~
///  TTabControl encapsulates the tab control - a window that provides
///  a user interface analogous to dividers in a notebook.
//
class _OWLCLASS TTabControl : public TControl {
  public:
    TTabControl(TWindow* parent,
                int             id,
                int x, int y, int w, int h,
                TModule*        module = 0);

    TTabControl(TWindow* parent, int resourceId, TModule* module = 0);
    TTabControl(HWND hwnd);
   ~TTabControl();

    /// \name Add/remove tab items
    /// @{
    int         Add(const TTabItem&);
    int         Add(LPCTSTR tabText); ///< The string pointed to must outlive the control.
    int         Insert(const TTabItem&, int index);
    int         Insert(LPCTSTR tabText, int index); ///< The string pointed to must outlive the control.
    bool        Delete(int index);
    bool        DeleteAll();
    /// @}

    /// \name Set/Querry attributes of TabControl
    /// @{
    int         GetCount() const;
    int         GetRowCount() const;
    int         GetSel() const;
    int         SetSel(int index);
    /// @}

    /// \name Set/Querry attributes of Tab Items
    /// @{
    bool        GetItem(int index, TTabItem& item) const;
    bool        GetItemRect(int index, TRect& rect) const;
    bool        SetItem(int index, const TTabItem& item);
    bool        SetItemExtra(int extra);
    TSize       SetItemSize(const TSize& size);
    void        SetPadding(const TSize& size);
    /// @}

    /// \name Set/Querry attributes of control window
    /// @{
    void        AdjustRect(bool clientInWindowOut, TRect& rect);
    HIMAGELIST  GetImageList() const;
    HIMAGELIST  SetImageList(HIMAGELIST);
    void        RemoveImage(int index);
    /// @}

    // Tooltip
    //
    HWND        GetToolTips() const;
    void        SetToolTips(HWND toolTip);

    int         HitTest(TTabHitTestInfo&);

    /// \name New messages
    /// @{
    void        DeselectAll(bool exclFocus=true); // Version 4.70

    uint32      GetExtendedStyle() const;         // Version 4.71
    uint32       SetExtendedStyle(uint32 mask, uint32 style);

    bool        HighLightItem(int item, bool highlight=true); //Version 4.71

    void        SetMinTabWidht(int width); // Version 4.71
    /// @}

    // Override TWindow virtual member function to handle transfers
    //
    uint        Transfer(void* buffer, TTransferDirection direction);

  protected:

    // Override TWindow virtual member functions
    //
    virtual TGetClassNameReturnType GetClassName();

    // Override TWindow handlers
    //
    void        EvHScroll(uint scrollCode, uint thumbPos, THandle hWndCtl);
    void        EvVScroll(uint scrollCode, uint thumbPos, THandle hWndCtl);

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TTabControl(const TTabControl&);
    TTabControl& operator =(const TTabControl&);

  DECLARE_RESPONSE_TABLE(TTabControl);
// !BB  DECLARE_STREAMABLE(_OWLCLASS, owl::TTabControl, 1);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inlines implementations
//

//
/// Constructor for a Tab Item:
/// This constructor is useful when creating a TabItem (TC_ITEM) structure
/// which will be filled with information about an existing tab in a
/// tab control. For example,
/// \code
///              TTabItem item(TCIF_IMAGE|TCIF_PARAM);
///              tabCtrl.GetItem(index, item);
/// \endcode
inline TTabItem::TTabItem(uint msk) {
  mask = msk;
}

//
/// Constructs a 'TTabItem' object from a 'TC_ITEM' structure using the
/// assignment operator.
/// \note  Default assignment operator is fine even if we get a shallow copy
///        for 'pszText' since the effective lifetime of a 'TTabItem' is
///        rather short and the underlying control copies/caches the item's
///        label
//
inline TTabItem::TTabItem(const TC_ITEM& tbItem)
{
  *((TC_ITEM*)this) = tbItem;
}

//
/// Sets the index of the image assiciated with the tab represented
/// by this item structure.
//
inline void TTabItem::SetIcon(int imageIndex)
{
  iImage = imageIndex;
  mask |= TCIF_IMAGE;
}

//
/// Sets the user-defined data associated with the tab represented
/// by this item structure.
//
inline void TTabItem::SetParam(TParam2 param)
{
  lParam = param;
  mask |= TCIF_PARAM;
}

// Version 4.70
inline void TTabControl::DeselectAll(bool exclFocus)
{
  PRECONDITION(GetHandle());
  SendMessage(TCM_DESELECTALL, TParam1((BOOL)exclFocus));
}

// Version 4.71
inline uint32 TTabControl::GetExtendedStyle() const
{
  PRECONDITION(GetHandle());
  return CONST_CAST(TTabControl*,this)->SendMessage(TCM_GETEXTENDEDSTYLE);
}

// Version 4.71
inline uint32 TTabControl::SetExtendedStyle(uint32 mask, uint32 style)
{
  PRECONDITION(GetHandle());
  return (uint32)SendMessage(TCM_SETEXTENDEDSTYLE, mask, style);
}

// Version 4.71
inline bool TTabControl::HighLightItem(int item, bool highlight)
{
  PRECONDITION(GetHandle());
  return SendMessage(TCM_HIGHLIGHTITEM,item, MkUint32((uint16)(BOOL)highlight,0));
}

// Version 4.71
inline void TTabControl::SetMinTabWidht(int width)
{
  PRECONDITION(GetHandle());
  SendMessage(TCM_SETMINTABWIDTH,0, width);
}

} // OWL namespace


#endif  // OWL_TABCTRL_H
