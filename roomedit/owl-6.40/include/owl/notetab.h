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
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
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


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TFont;
class _OWLCLASS TScrollBar;
class _OWLCLASS TCelArray;

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

    // operators to make STL happy
    bool operator==(const TNoteTabItem& str) const;
    bool operator<(const TNoteTabItem& str) const;

    TRect          Rect;       ///< Location of tab [client-area base coords]
    tstring    Label;      ///< Label of tab
    TSize          LabelSize;  ///< Width and height of label
    INT_PTR        ClientData; ///< User-defined data associated with item
    int            ImageIdx;   ///< Index of tab image
    TAbsLocation  ImageLoc;    ///< Placement of tab image
};

class TNoteTabItemArray;
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
   ~TNoteTab();

    // Add/remove tab items
    //
    int         Add(LPCTSTR txt, INT_PTR clientData = 0, int imageIdx = -1,
                    TAbsLocation imageLoc = alLeft);

    int Add(const tstring& txt, INT_PTR clientData = 0, int imageIdx = -1, TAbsLocation imageLoc = alLeft)
    {return Add(txt.c_str(), clientData, imageIdx, imageLoc);}

    int         Insert(LPCTSTR txt, int index, INT_PTR clientData = 0, 
                       int imageIdx = -1,TAbsLocation imageLoc = alLeft);

    int Insert(const tstring& txt, int index, INT_PTR clientData = 0, int imageIdx = -1, TAbsLocation imageLoc = alLeft)
    {return Insert(txt.c_str(), index, clientData, imageIdx, imageLoc);}

 
    // Added by DLN (3/15/2000)

    int AddNoSelect(LPCTSTR label, INT_PTR clientData = 0);

    int AddNoSelect(const tstring& label, INT_PTR clientData = 0)
    {return AddNoSelect(label.c_str(), clientData);}

    int InsertNoSelect(LPCTSTR label, int index, INT_PTR clientData = 0);

    int InsertNoSelect(const tstring& label, int index, INT_PTR clientData = 0)
    {return InsertNoSelect(label.c_str(), index, clientData);}

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
    int         GetFirstVisibleTab() const;
    void        SetFirstVisibleTab(int index);
    bool        EnsureVisible(int index);

    // Set/Query attributes of Tab Items
    //
    bool        GetItem(int index, TNoteTabItem& item) const;
    TNoteTabItem GetItem(int index) const;
    bool        SetItem(int index, const TNoteTabItem& item);
    bool        IsVisible(int index) const;
    void        SetTabFont(HFONT font, bool redraw = true);
    int         GetTabHeight() const;
    TAbsLocation GetScrollLocation() const;
    void        SetScrollLocation(TAbsLocation pos);
    void        SetCelArray(TCelArray* array, TAutoDelete = AutoDelete);
    TCelArray*  GetCelArray();

    // Set/Query buddy window
    //
    HWND        GetBuddy() const;
    void        SetBuddy(HWND buddy);

    // Override TWindow virtual member function to handle transfers
    //
    uint        Transfer(void* buffer, TTransferDirection direction);

  protected:

    // Override TWindow virtual member functions
    //
    virtual TGetClassNameReturnType GetClassName();
    void        SetupWindow();
    void        Paint(TDC& dc, bool erase, TRect& rect);

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


    // Routines handling underlying implementation
    //
    void        InitCtrl();
    void        SetupFont(HFONT font = 0);
    void        SetTabRects(int firstTab);
    void        InvalidateTabRect(int index);
    void        SetTabSize(int index);
    int         TabFromPoint(const TPoint& pt);
    void        GetTabsArea(TRect& rect);
    void        GetScrollerArea(TRect& rect);
    void        NotifyAndSelect(int index);

  protected_data:
    bool        WindowFace;         ///< True if active tab should use window color.
    bool        Style3d;            ///< If true draws with 3D style.
    int         TopMargin;          ///< Amount of a extra space to be left between the tab items and 
                                    ///< the top of the control when computing the location of the tabs.

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TNoteTab(const TNoteTab&);
    TNoteTab& operator =(const TNoteTab&);

    TFont*      TabFont;
    TWindow*    Buddy;
    THandle     BuddyHandle;
    int         FirstVisibleTab;
    int         SelectedTab;
    TNoteTabItemArray* TabList;
    TScrollBar*  ScrollBar;
    int         TabHeight;
    TAbsLocation ScrollLoc;
    TCelArray*  CelArray;
    bool         ShouldDelete;

    void        DrawFocusRect(TDC& dc);

    friend class TNoteTabItemArray;

  DECLARE_RESPONSE_TABLE(TNoteTab);
};

//@}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
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
/// Returns FirstVisibleTab.
//
inline int
TNoteTab::GetFirstVisibleTab() const
{
  return FirstVisibleTab;
}
//
/// Returns the height of the tab.
//
inline int
TNoteTab::GetTabHeight() const
{
  return TabHeight;
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
//
//

} // OWL namespace

#endif  // !RC_INVOKED

#endif  //  OWL_NOTETAB_H
