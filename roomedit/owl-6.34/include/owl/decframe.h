//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TDecoratedFrame, a TFrameWindow that can manage
/// decorations around the client window.
//----------------------------------------------------------------------------

#if !defined(OWL_DECFRAME_H)
#define OWL_DECFRAME_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/framewin.h>
#include <owl/layoutwi.h>
#include <owl/bardescr.h>

namespace owl {

const int IDW_TOOLBAR  = 32041;  // Standard toolbar child window ID

class _OWLCLASS TGadgetWindow;
class _OWLCLASS TDecoratedFrame;

template <class R, class T1>  class TFunctor1;
// TGadgetWndFunctor is a ptr to a fn taking nothing and returning a TGadgetWindow*
typedef TFunctor1<TGadgetWindow*,TWindow*> TGadgetWndFunctor;

#define TGadgetWnd_FUNCTOR(func) TFunctionTranslator1<TGadgetWindow*,TWindow*, TGadgetWindow* (*)(TWindow*)>(func)
#define TGadgetWnd_MFUNCTOR(obj,memberFunc) TMemberTranslator1<TGadgetWindow*,TWindow*, TGadgetWindow* (obj::*)(TWindow*)>(obj, memberFunc);

//
// Funct = TGadgetWnd_MFUNCTOR(myWndPtr,&TMyWindow::MyHandle)
// Funct = TGadgetWnd_MFUNCTOR(this,&TMyWindow::MyHandle)
//

template <class R, class T1, class T2, class T3>  class TFunctor3;
typedef TFunctor3<bool, TDecoratedFrame&, TGadgetWindow&, TAbsLocation> TBarInsertFunctor;

#define TBarInsert_FUNCTOR(func) TFunctionTranslator3<bool, TDecoratedFrame&, TGadgetWindow&, TAbsLocation, bool (*)(TDecoratedFrame&, TGadgetWindow&, TAbsLocation)>(func)
#define TBarInsert_MFUNCTOR(obj,memberFunc) TMemberTranslator3<bool, TDecoratedFrame&, TGadgetWindow&, TAbsLocation, bool (obj::*)(TDecoratedFrame&, TGadgetWindow&, TAbsLocation)>(obj, memberFunc);

/// \addtogroup decor
/// @{
/// \enum THintText
// ~~~~~~~~~~~~~~
enum THintText {
  htStatus    = 0x0001,         ///< Text displayed on status bar
  htTooltip   = 0x0002,         ///< Shorter text displayed in a tooltip
  htUserDefined                 ///< Can be used when enhancing 'GetHintText'
};                              ///< to support balloon text etc..

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TDecoratedFrame
// ~~~~~ ~~~~~~~~~~~~~~~
/// TDecoratedFrame automatically positions its client window (you must supply a
/// client window) so that it is the same size as the client rectangle. You can add
/// additional decorations like toolbars and status lines to a window.You can create
/// a TDecoratedFrame without a caption bar by clearing all of the bits in the style
/// data member of the TWindowAttr structure. TDecoratedFrame is a streamable class.
/// It is virtually derived from TFrameWindow and from TLayoutWindow.
/// 
/// For OLE-enabled applications, use TOleFrame, which creates a decorated frame and
/// manages decorations such as toolbars for the main window of an SDI (Single
/// Document Interface) OLE application.
//
class _OWLCLASS TDecoratedFrame : virtual public TFrameWindow,
                                          public TLayoutWindow {
  public:
    TDecoratedFrame(TWindow*  parent,
                    LPCTSTR   title,
                    TWindow*  clientWnd,
                    bool      trackMenuSelection = false,
                    TModule*  module = 0);
    TDecoratedFrame(TWindow* parent, const tstring& title, TWindow* client, bool trackMenuSelection = false, TModule* = 0);
    ~TDecoratedFrame();

    /// Enumeration describing the possible locations of a Gadgetwindow
    /// [Used mainly for location of Toolbar and Statusbar standalone
    ///  and in the context of docking windows]
    //
    enum TLocation {
      None   = alNone,            ///< No location specified
      Top    = alTop,             ///< Refers to top edge of frame
      Bottom = alBottom,          ///< Refers to bottom edge of frame
      Left   = alLeft,            ///< Refers to left edge of frame
      Right  = alRight,           ///< Refers to right edge of frame
    };
//    typedef TAbsLocation TLocation;

    virtual void  Insert(TWindow& decoration, TLocation location = Top);

    // Hint/Status/Tip text
    //
    virtual tstring GetHintText(uint id, THintText hintType) const;

    /// Override TFrameWindow member function to give decorations an opportunity
    /// to do pre-processing (e.g. mnemonic access)
    //
    bool      PreProcessMsg(MSG& msg);
    TWindow*  SetClientWindow(TWindow* clientWnd);

    // Virtuals from TFrameWindow
    virtual TGadgetWindow* GetControlBar();

    void SetBarCreator(const TGadgetWndFunctor& funct);
    void SetBarInserter(const TBarInsertFunctor& funct);
    
    TGadgetWndFunctor* GetBarCreator();
    TBarInsertFunctor* GetBarInserter();

    //
    // Please use SetBarCreator(), GetBarCreator()
#if 0 
    class TBarCreator{
      private:
        TDecoratedFrame* GetParent();
      public:
        TGadgetWndFunctor& operator()(){
          return *GetParent()->GetBarCreator();
        }
        TBarCreator& operator=(const TGadgetWndFunctor& funct)
        {
          GetParent()->SetBarCreator(funct);
          return *this;
        }
    }BarCreator;
#endif
    //
    // Please use SetBarInserter(), GetBarInserter()
#if 0
    class TEvBarInsert{
      private:
        TDecoratedFrame* GetParent();
      public:
        TBarInsertFunctor& operator()(){
          return *GetParent()->GetBarInserter();
        }
        TEvBarInsert& operator=(const TBarInsertFunctor& funct)
        {
          GetParent()->SetBarInserter(funct);
          return *this;
        }
    }EvBarInsert;
#endif
  protected:

    void Init(TWindow* parent, LPCTSTR title, TWindow* client, bool trackMenuSelection, TModule*);

    // Virtuals from TFrameWindow
    virtual bool MergeBar(const TBarDescr& childBarDescr);
    virtual bool RestoreBar();
    virtual bool CreateBar();

    bool      GetTrackMenuSelection() const;
    void      SetTrackMenuSelection(bool tr=true);

    uint      GetMenuItemId() const;
    void      SetMenuItemId(uint menuitemid);

    // Override TWindow virtuals
    //
    void      SetupWindow();
    void      CleanupWindow();
    void      RemoveChild(TWindow* child);

    // Automatic showing/hiding of tool bars and status bars
    //
    TResult   EvCommand(uint id, HWND hWndCtl, uint notifyCode);
    void      EvCommandEnable(TCommandEnabler& ce);

    // Message response functions
    //
    void      EvMenuSelect(uint menuItemId, uint flags, HMENU hMenu);
    void      EvEnterIdle(uint source, HWND hWndDlg);
    void      EvSize(uint sizeType, const TSize& size);

  protected_data:
    bool      TrackMenuSelection;       ///< track menu select for status hints?
    uint      MenuItemId;               ///< id of current menu selection

  private:
    void      InsertAtTop(TWindow& decoration, TWindow* insertAbove);
    void      InsertAtBottom(TWindow& decoration, TWindow* insertBelow);
    void      InsertAtLeft(TWindow& decoration, TWindow* insertLeftOf);
    void      InsertAtRight(TWindow& decoration, TWindow* insertRightOf);

    bool      SettingClient;
    
    TGadgetWndFunctor* BarCreatorFunct;
    TBarInsertFunctor* BarInserterFunct;

    // Hidden to prevent accidental copying or assignment
    //
    TDecoratedFrame(const TDecoratedFrame&);
    TDecoratedFrame& operator =(const TDecoratedFrame&);

  DECLARE_RESPONSE_TABLE(TDecoratedFrame);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TDecoratedFrame, 1);
  DECLARE_STREAMABLE_OWL(TDecoratedFrame, 1);
};
/// @}

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TDecoratedFrame );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns true if the decorated frame is tracking the menu selections.
/// Sends a message to the status bar to display the hint text.
//
inline bool
TDecoratedFrame::GetTrackMenuSelection() const {
  return TrackMenuSelection;
}

//
/// Set the flag for tracking menu selection.
//
inline void
TDecoratedFrame::SetTrackMenuSelection(bool tr) {
  TrackMenuSelection = tr;
}

//
/// Return cached id of the current menu item.
//
inline uint
TDecoratedFrame::GetMenuItemId() const {
  return MenuItemId;
}

//
/// Set the cached id of the current menu item.
//
inline void
TDecoratedFrame::SetMenuItemId(uint menuitemid) {
  MenuItemId = menuitemid;
}

//
//
//
inline TGadgetWndFunctor* 
TDecoratedFrame::GetBarCreator(){
  return BarCreatorFunct;
}
//
//
//
inline TBarInsertFunctor*
TDecoratedFrame::GetBarInserter(){
  return BarInserterFunct;
}
//

} // OWL namespace


#endif  // OWL_DECFRAME_H
