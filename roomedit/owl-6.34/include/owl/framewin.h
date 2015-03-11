//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TFrameWindow.
//----------------------------------------------------------------------------

#if !defined(OWL_FRAMEWIN_H)
#define OWL_FRAMEWIN_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/window.h>
#include <owl/menu.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
// MDI window IDs
//
const unsigned int IDW_MDICLIENT     = 32000;  ///< ID of MDI client window
const unsigned int IDW_FIRSTMDICHILD = 32001;  ///< child window IDs, 32 max to 32032

/// \addtogroup enabler
/// @{
/// \class TMenuItemEnabler
// ~~~~~ ~~~~~~~~~~~~~~~~
/// Derived from TCommandEnabler, TMenuItemEnabler is a command enabler for menu
/// items. The functions in this class modify the text, check state, and appearance
/// of a menu item.
//
class _OWLCLASS TMenuItemEnabler : public TCommandEnabler {
  public:
    TMenuItemEnabler(HMENU hMenu, uint id, HWND hWndReceiver, int position);

    // Override member functions of TCommandEnabler
    //
    void    Enable(bool enable);
    void    SetText(LPCTSTR text);
    using TCommandEnabler::SetText; ///< String-aware overload
    void    SetCheck(int check);

    // Menu specific member functions
    //
    HMENU   GetMenu();
    int     GetPosition();

  protected_data:
  	
/// The menu that holds the item being enabled or disabled.
    HMENU   HMenu;
    
/// The position of the menu item.
    int     Position;
};
/// @}

/// \addtogroup frame
/// @{

class _OWLCLASS TBarDescr;
//
/// \class TFrameWindow
// ~~~~~ ~~~~~~~~~~~~
/// Derived from TWindow, TFrameWindow controls such window-specific behavior as
/// keyboard navigation and command processing for client windows. For example, when
/// a window is reactivated, TFrameWindow is responsible for restoring a window's
/// input focus and for adding menu bar and icon support. TFrameWindow is a
/// streamable class.
/// 
/// In terms of window areas, the frame area consists of the border, system menus,
/// toolbars and status bars whereas the client area excludes these areas. Although
/// frame windows can support a client window, the frame window remains separate
/// from the client window so that you can change the client window without
/// affecting the frame window.
/// 
/// ObjectWindows uses this frame and client structure for both TFrameWindow and
/// TMDIChild classes. Both these classes can hold a client class. Having a separate
/// class for the client area of the window adds more flexibility to your program.
/// For example, this separate client area, which might be a dialog box, can be
/// moved into another frame window, either a main window or an MDI child window.
/// 
/// TFrameWindow adds the notion of a client window, keyboard navigation, and special
/// processing for commands (see member function EvCommand() )
///
/// See TFloatingFrame for a description of a floating frame with the same default
/// functionality as a frame window.
//
class _OWLCLASS TFrameWindow : virtual public TWindow {
  public:
    TFrameWindow(TWindow*        parent,
                 LPCTSTR title = 0,
                 TWindow*        clientWnd = 0,
                 bool            shrinkToClient = false,
                 TModule*        module = 0);

    TFrameWindow(
      TWindow* parent,
      const tstring& title,
      TWindow* client = 0,
      bool shrinkToClient = false,
      TModule* = 0);

    TFrameWindow(HWND hWnd, TModule* module = 0);
   ~TFrameWindow();

    // Menubar manipulating functions
    //
    virtual bool    AssignMenu(TResId menuResId);
    virtual bool    SetMenu(HMENU newMenu);
    
    void            SetMenuDescr(const TMenuDescr& menuDescr);
    const TMenuDescr* GetMenuDescr() const;
    virtual bool    MergeMenu(const TMenuDescr& childMenuDescr);
    bool            RestoreMenu();

    void            SetBarDescr(TBarDescr* barDescr, TAutoDelete = AutoDelete);
    const TBarDescr* GetBarDescr() const;
    virtual bool    MergeBar(const TBarDescr& childBarDescr);
    virtual bool    RestoreBar();

    TModule*        GetMergeModule();

    HICON           GetIcon() const;
    HICON           GetIconSm() const;
    bool            SetIcon(TModule* iconModule, TResId iconResId);
    bool            SetIconSm(TModule* iconModule, TResId iconResIdSm);

    // Client and child window manipulation
    //
    virtual TWindow* GetClientWindow();
    virtual TWindow* SetClientWindow(TWindow* clientWnd);
    void            RemoveChild(TWindow* child);
    virtual HWND    GetCommandTarget();

    // Sets flag indicating that the receiver has requested "keyboard
    // handling" (translation of keyboard input into control selections)
    //
    bool            GetKeyboardHandling() const;
    void            SetKeyboardHandling(bool kh=true);
    void            EnableKBHandler();

    // Override virtual functions defined by TWindow
    //
    bool            PreProcessMsg(MSG& msg);
    bool            IdleAction(long idleCount);
    bool            HoldFocusHWnd(HWND hWndLose, HWND hWndGain);
    bool            SetDocTitle(LPCTSTR docname, int index);
    using TWindow::SetDocTitle; ///< String-aware overload

  protected:
    // Constructor & subsequent initializer for use with virtual derivations
    // Immediate derivitives must call Init() before constructions are done.
    //
    TFrameWindow();
    void            Init(TWindow* clientWnd, bool shrinkToClient);

    // The event handler functions that are virtual, and not dispatched thru
    // response tables
    //
    // Extra processing for commands: starts with the focus window and gives
    // it and its parent windows an opportunity to handle the command.
    //
    TResult         EvCommand(uint id, HWND hWndCtl, uint notifyCode);
    void            EvCommandEnable(TCommandEnabler& ce);

    // Tell child windows frame has minimized/maximized/restored
    //
    void            BroadcastResizeToChildren(uint sizeType, const TSize& size);

    // Message response functions
    //
    void            EvInitMenuPopup(HMENU hPopupMenu, uint index, bool sysMenu);
    void            EvPaint();
    bool            EvEraseBkgnd(HDC);
    HANDLE          EvQueryDragIcon();
    void            EvSetFocus(HWND hWndLostFocus);
    void            EvSize(uint sizeType, const TSize& size);
    void            EvParentNotify(uint event, TParam1, TParam2);
    void            EvPaletteChanged(THandle hWndPalChg);
    bool            EvQueryNewPalette();

    // Override virtual functions defined by TWindow
    //
    void            SetupWindow();
    void            CleanupWindow();

    // Accesors to data members
    //
    HWND            GetHWndRestoreFocus();
    void            SetHWndRestoreFocus(HWND hwndRestoreFocus);
    int             GetDocTitleIndex() const;
    void            SetDocTitleIndex(int index);
    void            SetMergeModule(TModule* module);

  public_data:
/// Indicates if keyboard navigation is required.
    bool            KeyboardHandling;

  protected_data:
/// Stores the handle of the child window whose focus gets restored.
    THandle         HWndRestoreFocus;
    
/// ClientWnd points to the frame's client window.
    TWindow*        ClientWnd;
    
/// Holds the index number for the document title.
    int             DocTitleIndex;
    
/// Tells the frame window which module the menu comes from. TDecoratedFrame uses
/// this member to get the menu hints it displays at the bottom of the screen. It
/// assumes that the menu hints come from the same place the menu came from.
    TModule*        MergeModule;

  private:
    TMenuDescr*     MenuDescr;
    TBarDescr*       BarDescr;
    bool            DeleteBar;
    TModule*        IconModule;
    TResId          IconResId;
    TModule*        IconSmModule;
    TResId          IconSmResId;
    TPoint          MinimizedPos;
    HICON           CurIcon;
    HICON           CurIconSm;

    void            Init(TWindow* clientWnd);
    bool            ResizeClientWindow(bool redraw = true);
    TWindow*        FirstChildWithTab();

    // Hidden to prevent accidental copying or assignment
    //
    TFrameWindow(const TFrameWindow&);
    TFrameWindow& operator =(const TFrameWindow&);

  DECLARE_RESPONSE_TABLE(TFrameWindow);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TFrameWindow, 3);
  DECLARE_STREAMABLE_OWL(TFrameWindow, 3);
};
/// @}
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TFrameWindow );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Constructs a TMenuItemEnabler with the specified command ID for the menu item,
/// message responder (hWndReceiver), and position on the menu.
//
inline TMenuItemEnabler::TMenuItemEnabler(HMENU hMenu, uint id,
                                          HWND hWndReceiver, int position)
:
  TCommandEnabler(id, hWndReceiver),
  HMenu(hMenu),
  Position(position)
{
  if (::GetMenuItemID(HMenu, Position) == uint(-1))
    Flags = NonSender;
}

//
/// Returns the menu that holds the item being enabled or disabled.
//
inline HMENU TMenuItemEnabler::GetMenu()
{
  return HMenu;
}

//
/// Returns the position of the menu item.
//
inline int TMenuItemEnabler::GetPosition()
{
  return Position;
}

//
/// Retrieve handle of icon of frame window
/// \note Only retrieves icon set via the 'SetIcon' API - does not retrieve
///       the small HICON associated with the HWND.
//
inline HICON TFrameWindow::GetIcon() const
{
    return CurIcon;
}

//
/// Retrieve handle of icon of frame window
/// \note Only retrieves icon set via the 'SetIconSm' API - does not retrieve
///       the HICON associated with the HWND.
//
inline HICON TFrameWindow::GetIconSm() const
{
  return CurIconSm;
}

//
/// Returns a pointer to the menu descriptor for the frame window.
//
inline const TMenuDescr* TFrameWindow::GetMenuDescr() const
{
  return MenuDescr;
}

//
/// Returns a pointer to the control bar descriptor.
//
inline const TBarDescr* TFrameWindow::GetBarDescr() const
{
  return BarDescr;
}


//
/// Do nothing. Return false. Overriden in TDecoratedFrame
//
inline bool TFrameWindow::MergeBar(const TBarDescr& /*childBarDescr*/)
{
  return false;
}

//
/// Do nothing in TFrameWindow. Overriden in TDecoratedFrame.
//
inline bool TFrameWindow::RestoreBar()
{
  return false;
}

//
/// Returns flag indicating that the receiver has requested "keyboard handling"
/// (translation of keyboard input into control selections).
//
inline bool TFrameWindow::GetKeyboardHandling() const
{
  return KeyboardHandling;
}

//
/// Sets flag indicating that the receiver has requested "keyboard handling"
/// (translation of keyboard input into control selections).
//
inline void TFrameWindow::SetKeyboardHandling(bool kh)
{
  KeyboardHandling = kh;
}

//
/// Sets the keyboard handling flag to true.
//
/// Sets a flag indicating that the receiver has requested keyboard navigation
/// (translation of keyboard input into control selections). By default, the
/// keyboard interface, which lets users use the tab and arrow keys to move between
/// the controls, is disabled for windows and dialog boxes.
//
inline void TFrameWindow::EnableKBHandler()
{
  SetKeyboardHandling(true);
}

//
/// Returns the handle of the window to restore the focus to.
//
inline HWND TFrameWindow::GetHWndRestoreFocus()
{
  return HWndRestoreFocus;
}

//
/// Sets the remembered focused window.
//
inline void TFrameWindow::SetHWndRestoreFocus(HWND hwndrestorefocus)
{
  HWndRestoreFocus = hwndrestorefocus;
}

//
/// Returns the document title index.
//
inline int TFrameWindow::GetDocTitleIndex() const
{
  return DocTitleIndex;
}

//
/// Sets the current document's title index.
//
inline void TFrameWindow::SetDocTitleIndex(int index)
{
  DocTitleIndex = index;
}

//
/// Returns the module of the merge menu.
//
inline TModule* TFrameWindow::GetMergeModule()
{
  return MergeModule;
}

//
/// Remembers where the merged menu came from.
//
inline void TFrameWindow::SetMergeModule(TModule* module)
{
  MergeModule = module;
}

} // OWL namespace


#endif  // OWL_FRAMEWIN_H
