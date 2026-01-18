//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TNoteTab
// Added Scroll handling. Contributed by Jogy
// Added Font setting. Contributed by Jogy
// Added support for tab image. Contributed by Jogy
//----------------------------------------------------------------------------

#if !defined(OWL_NOTETAB_H)
#define OWL_NOTETAB_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

// Macro defining class name (usable in resource definition)
//
#if !defined(OWL_NOTETAB)
#  if defined(UNICODE) // for Resource Workshop
#    define OWL_NOTETAB L"OWL_Notetab"
#  else
#    define OWL_NOTETAB "OWL_Notetab"
#  endif
#endif

#if !defined(RC_INVOKED)

#include <owl/control.h>
#include <owl/commctrl.h>
#include <owl/celarray.h>
#include <owl/gdiobjec.h>
#include <vector>
#include <memory>

namespace owl {

#include <owl/preclass.h>

class _OWLCLASS TUpDown;

/// \addtogroup ctrl
/// @{
/// \class TNoteTabItem
// ~~~~~ ~~~~~~~~~~~~
/// TNoteTabItem holds information about each tab in a notetab control. For example,
/// the structure contains information about the title and size of each tab item.
//
struct TNoteTabItem {
    TNoteTabItem(const tstring& label, INT_PTR clientData = 0, int imageIdx = -1, TAbsLocation imageLoc = alLeft);
    TNoteTabItem();

    TRect          Rect;       ///< Location of tab [client-area base coords]
    tstring    Label;      ///< Label of tab
    TSize          LabelSize;  ///< Width and height of label
    INT_PTR        ClientData; ///< User-defined data associated with item
    int            ImageIdx;   ///< Index of tab image
    TAbsLocation  ImageLoc;    ///< Placement of tab image
};

//
/// \class TNoteTab
// ~~~~~ ~~~~~~~~
/// TNoteTab encapsulates a tab control with each tab item along the bottom
/// of the window.
//
class _OWLCLASS TNoteTab : public TControl {
  public:
    TNoteTab(TWindow*   parent,
             int        id,
             int x, int y, int w, int h,
             TWindow*   buddy = 0,
             bool       dialogBuddy = true,
             TModule*   module = 0);

    TNoteTab(TWindow* parent,
             int resourceId,
             TWindow* buddy = 0,
             bool     dialogBuddy = true,
             TModule* module = 0);

    // Add/remove tab items
    //
    int Add(
      LPCTSTR txt,
      INT_PTR clientData = 0,
      int imageIdx = -1,
      TAbsLocation imageLoc = alLeft,
      bool shouldSelect = true);

    int Add(
      const tstring& txt,
      INT_PTR clientData = 0,
      int imageIdx = -1,
      TAbsLocation imageLoc = alLeft,
      bool shouldSelect = true)
    {
      return Add(txt.c_str(), clientData, imageIdx, imageLoc, shouldSelect);
    }

    int Insert(
      LPCTSTR txt,
      int index,
      INT_PTR clientData = 0,
      int imageIdx = -1,
      TAbsLocation imageLoc = alLeft,
      bool shouldSelect = true);

    int Insert(
      const tstring& txt,
      int index,
      INT_PTR clientData = 0,
      int imageIdx = -1,
      TAbsLocation imageLoc = alLeft,
      bool shouldSelect = true)
    {
      return Insert(txt.c_str(), index, clientData, imageIdx, imageLoc, shouldSelect);
    }

    bool        Delete(int index);
    bool        DeleteAll();

    // Set/Query attributes of TabControl
    //
    int         GetCount() const;
    int         GetSel() const;
    int         SetSel(int index);
    void        SetWindowFace(bool);
    bool        GetWindowFace() const;
    void        SetStyle3d(bool);
    bool        GetStyle3d() const;
    void        EnableThemes(bool);
    bool        AreThemesEnabled() const {return ShouldUseThemes;}
    int         GetFirstVisibleTab() const;
    void        SetFirstVisibleTab(int index);
    bool        EnsureVisible(int index);

    //
    /// Returns the minimal control height for which tabs are not clipped.
    //
    int GetMinimalHeight();

    //
    /// Returns the amount of vertical space above the tabs.
    //
    TSize GetMargin() const {return Margin;}
    void SetMargin(const TSize&);

    //
    /// Returns the amount of padding around the tab label.
    //
    TSize GetLabelMargin() const {return LabelMargin;}
    void SetLabelMargin(const TSize&);

    //
    /// Returns the horizontal spacing between image and text in the label.
    //
    int GetLabelImageMargin() const {return LabelImageMargin;}
    void SetLabelImageMargin(int);

    //
    /// Returns the margin around the focus rectangle for the selected tab.
    //
    TSize GetFocusMargin() const {return FocusMargin;}
    void SetFocusMargin(const TSize&);

    //
    /// Returns the horizontal distance between two tabs.
    //
    int GetTabSpacing() const {return TabSpacing;}
    void SetTabSpacing(int);

    //
    /// Returns the amount of narrowing on each side of the tab towards the bottom.
    //
    int GetTabTapering() const {return TabTapering;}
    void SetTabTapering(int);

    //
    /// Returns the amount of extra height of the selected tab.
    //
    int GetSelectedTabProtrusion() const {return SelectedTabProtrusion;}
    void SetSelectedTabProtrusion(int);

    const TFont& GetTabFont() const;
    void SetTabFont(const TFont&);

    const TFont& GetSelectedTabFont() const;
    void SetSelectedTabFont(const TFont&);

    //
    /// Returns the fill color used to paint the tabs.
    //
    TColor GetTabColor() const {return TabColor;}
    void SetTabColor(const TColor&);

    //
    /// Returns the fill color used to paint the selected tab.
    /// This color is only used when WindowFace mode is selected.
    //
    TColor GetSelectedTabColor() const {return SelectedTabColor;}
    void SetSelectedTabColor(const TColor&);

    //
    /// Returns the pen color used to draw the edges of the tabs.
    //
    TColor GetEdgeColor() const {return EdgeColor;}
    void SetEdgeColor(const TColor&);

    // Set/Query attributes of Tab Items
    //
    bool        GetItem(int index, TNoteTabItem& item) const;
    TNoteTabItem GetItem(int index) const;
    bool        SetItem(int index, const TNoteTabItem& item);
    bool        IsVisible(int index) const;
    bool        IsFullyVisible(int index) const;
    TAbsLocation GetScrollLocation() const;
    void        SetScrollLocation(TAbsLocation pos);
    void        SetCelArray(TCelArray* array, TAutoDelete = AutoDelete);
    TCelArray*  GetCelArray();
    void        SetCelArrayTransparentColor(const TColor&);
    TColor      GetCelArrayTransparentColor() const;

    // Set/Query buddy window
    //
    HWND        GetBuddy() const;
    void        SetBuddy(HWND buddy);

    // Override TWindow virtual member function to handle transfers
    //
    auto Transfer(void* buffer, TTransferDirection) -> uint override;

  protected:

    // Override TWindow virtual member functions
    //
    auto GetWindowClassName() -> TWindowClassName override;
    void SetupWindow() override;
    void Paint(TDC&, bool erase, TRect&) override;

    // Message Handlers
    //
    void        EvSize(uint sizeType, const TSize& size);
    void        EvLButtonDown(uint modKeys, const TPoint& point);
    uint        EvGetDlgCode(const MSG* msg);
    void        EvKeyDown(uint key, uint repeatCount, uint flags);
    void        EvSetFocus(THandle hWndLostFocus);
    void        EvKillFocus(THandle hwndGetFocus);
    void         EvHScroll(uint scrollCode, uint thumbPos, HWND hWndCtl);
    void         EvPaint();
    bool EvEraseBkgnd(HDC);

    // Routines handling underlying implementation
    //
    void        InitCtrl();
    void        SetTabRects(int firstTab);
    void        InvalidateTab(int index);
    void        SetTabSize(int index);
    int         TabFromPoint(const TPoint& pt) const;
    TRect       GetScrollingTabsArea() const;
    TRect       GetScrollerArea() const;
    bool        NotifyAndSelect(int index);
    void        Update();

  protected_data:
    bool WindowFace;
    bool Style3d;
    bool ShouldUseThemes;
    TSize Margin;
    TSize LabelMargin;
    int LabelImageMargin;
    TSize FocusMargin;
    int TabSpacing;
    int TabTapering;
    int SelectedTabProtrusion;
    std::unique_ptr<TFont> TabFont;
    std::unique_ptr<TFont> SelectedTabFont;
    TColor TabColor;
    TColor SelectedTabColor;
    TColor EdgeColor;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TNoteTab(const TNoteTab&);
    TNoteTab& operator =(const TNoteTab&);

    TWindow*    Buddy;
    THandle     BuddyHandle;
    int         FirstVisibleTab;
    int         SelectedTab;
    typedef std::vector<TNoteTabItem> TNoteTabItemArray;
    TNoteTabItemArray TabList;
    TUpDown* ScrollButtons;
    TAbsLocation ScrollLoc;
    TCelArray*  CelArray;
    std::unique_ptr<TCelArray> OwnedCelArray;
    TColor TransparentColor;
    TRect LastClientRectPainted;
    TRect EffectiveTabsArea;

    TRect GetBoundingRect(const TRect& tabRect) const;
    TRect CalculateTabRect(const TNoteTabItem& tab, const TPoint& p, bool isSelected) const;
    typedef std::vector<TRect> TRects;
    TRects CalculateTabRects(int firstTab, const TRect& area) const;
    void AssignTabRect(TNoteTabItem&, const TRect& newRect);

    template <class TPartRenderer>
    void PaintTabs(TDC& dc, const TRect& rect);

  DECLARE_RESPONSE_TABLE(TNoteTab);
};

//@}

#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations

//
/// Constructor of Notetab Item object. Initializes object with specified
/// string label and optional user-defined data.
//
inline
TNoteTabItem::TNoteTabItem(const tstring& label, INT_PTR clientData,
                           int imageIdx, TAbsLocation imageLoc)
:
  Label(label),
  ClientData(clientData),
  ImageIdx(imageIdx),
  ImageLoc(imageLoc)
{
}

//
/// Default constructor of Notetab Item object.
//
inline
TNoteTabItem::TNoteTabItem()
:
  ClientData(0),
  ImageIdx(-1),
  ImageLoc(alLeft)
{
}

//
/// Specifies whether active tab should use the system's window color. If the 'wf'
/// parameter is true, the system's window color is used.
//
inline void
TNoteTab::SetWindowFace(bool wf)
{
  WindowFace = wf;
  Update();
}

//
/// Returns the flag specifying whether the active tab should use the system's
/// window color.
//
inline bool
TNoteTab::GetWindowFace() const
{
  return WindowFace;
}

//
/// Specifies whether the note tab should draw a 3D edge. If 'st' is true, the
/// control displays a 3D edge.
//
inline void
TNoteTab::SetStyle3d(bool st)
{
  Style3d = st;
  Update();
}

//
/// Returns the flag specifying whether the notetab control should draw a 3D border.
//
inline bool
TNoteTab::GetStyle3d() const
{
  return Style3d;
}

//
/// Specifies whether the note tab should use Windows visual styles (themes).
//
inline void
TNoteTab::EnableThemes(bool st)
{
  ShouldUseThemes = st;
  Update();
}

//
/// Returns FirstVisibleTab.
//
inline int
TNoteTab::GetFirstVisibleTab() const
{
  return FirstVisibleTab;
}

//
/// Get the scroller location.
//
inline TAbsLocation
TNoteTab::GetScrollLocation() const
{
  return ScrollLoc;
}
//
/// Get a pointer to the array of bitmaps used for the tabs.
//
inline TCelArray*
TNoteTab::GetCelArray()
{
  return CelArray;
}
//
/// Returns the color assigned to denote transparency in the bitmaps used for the
/// tabs (see SetCelArray).
//
inline TColor
TNoteTab::GetCelArrayTransparentColor() const
{
  return TransparentColor;
}

} // OWL namespace

#endif  // !RC_INVOKED

#endif  //  OWL_NOTETAB_H
