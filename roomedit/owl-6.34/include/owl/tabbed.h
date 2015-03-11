//----------------------------------------------------------------------------
//  ObjectWindows
//  Copyright © 1998 by Bidus Yura. All Rights Reserved.
//  OVERVIEW
//  ~~~~~~~~
/// \file
///  Class definition for TTabbedBase .
//
//
//----------------------------------------------------------------------------

#if !defined(OWL_TABBED_H)  // Sentry, use file only if it's not already included.
#define OWL_TABBED_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/layoutwi.h>
#include <owl/notetab.h>

#include <owl/tabbed.rh>


#if !defined(RC_INVOKED)

#include <owl/docview.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TNoteTab;

////////////////////////////////////////////////////////////////////////////////
/// \class TTabbedBase 
// ~~~~~ ~~~~~~~~~~~
// 
/// TTabbedBase implements a tabbed window, designed to be 
/// mixed in with TWindow derived. 
//
/// Example:
// ~~~~~~~~
/// \code
///class TTabbedDialog : public TDialog, public TTabbedBase{
///  public:
///    TTabbedDialog(...);
///     SetupWindow();
/// };
///TTabbedDialog::TTabbedDialog(...)
///{
///  Add(new TDialog(this, IDD_DLG1));
///  Add(new TDialog(this, IDD_DLG2));
///  Add(new TDialog(this, IDD_DLG3));
///  GetTabControl()->SetSel(0);
///}
///TTabbedDialog::SetupWindow()
///{
///  TDialog::SetupWindow();
///  SetupPages();
///}
/// \endcode
//
class _OWLCLASS TTabbedBase : virtual public TEventHandler
{
  public:
    //
    enum TTabPosition{ // only tpBottom currently
      tpLeft    = alLeft,
      tpRight    = alRight,
      tpBottom  = alBottom,
      tpTop      = alTop,
    };

    TTabbedBase(TWindow* self, int x, int y, int w, int h, TTabPosition=tpBottom, TNoteTab* tab = 0);
    TTabbedBase(TWindow* self, TTabPosition=tpBottom, TNoteTab* tab = 0);

    int Add(TWindow& wnd, LPCTSTR titles = 0);
    int Add(TWindow& wnd, const tstring& titles) {return Add(wnd, titles.c_str());}
    
    void SetupPages();
    TNoteTab* GetTabControl() { return NoteTab; }
    TWindow*  GetPage(int index = -1);

  protected:
    void EvTabControlSelChange(TNotify& tcn);
    bool EvTabControlSelChanging(TNotify& tcn);

    virtual void AdjustPage(TWindow& page);


    TNoteTab*      NoteTab;
    TTabPosition  Position;

  DECLARE_RESPONSE_TABLE(TTabbedBase);
  DECLARE_CASTABLE;
  // add streaming???
};

////////////////////////////////////////////////////////////////////////////////
/// \class TTabbedWindow 
// ~~~~~ ~~~~~~~~~~~~~
/// Tabbed windows it is used in tabbed window.
//
class _OWLCLASS TTabbedWindow: public TLayoutWindow, public TTabbedBase {
  public:
    TTabbedWindow(TWindow* parent=0, LPCTSTR title = 0, TModule* module = 0);
    TTabbedWindow(TWindow* parent, const tstring& title, TModule* = 0);

    // Need at least one page in order to add a page
    int    AddPage(TWindow& wnd, LPCTSTR titles = 0);
    bool  DeletePage(TWindow& wnd);
  
  public:
    virtual bool Create();

  DECLARE_RESPONSE_TABLE(TTabbedWindow);
  // add streaming
};


////////////////////////////////////////////////////////////////////////////////
/// \class TTabbedView
// ~~~~~ ~~~~~~~~~~~ 
//
class _OWLCLASS TTabbedView: public TTabbedWindow, public TView {
  public:
    TTabbedView(TDocument& doc, TWindow* parent = 0);
   ~TTabbedView();

    static LPCTSTR StaticName();  // put in resource

    // Override virtuals from TWindow
    //
    bool     CanClose();

    // Override virtuals from TView
    //
    LPCTSTR   GetViewName();
    TWindow*   GetWindow();
    bool       SetDocTitle(LPCTSTR docname, int index);

  private:
    // Event handlers
    //
    bool     VnIsWindow(HWND hWnd);

  DECLARE_RESPONSE_TABLE(TTabbedView);
  //DECLARE_STREAMABLE (_OWLCLASS, owl::TTabbedView,1);
};


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

// inlines
//
inline TTabbedView::~TTabbedView()
{
}

//
inline LPCTSTR TTabbedView::StaticName() {
  return _T("Tabbed View");
}

//
/// Only query document if this is the last view open to it.
//
inline bool TTabbedView::CanClose() {
  return TTabbedWindow::CanClose() && (Doc->NextView(this) 
          || Doc->NextView(0) != this || Doc->CanClose());
}

//
inline LPCTSTR TTabbedView::GetViewName() {
  return StaticName();
}

//
inline TWindow* TTabbedView::GetWindow() {
  return (TWindow*)this;
}

//
inline bool TTabbedView::SetDocTitle(LPCTSTR docname, int index) {
  return TTabbedWindow::SetDocTitle(docname, index);
}

} // OWL namespace

#endif  // !RC_INVOKED

#endif  // OWL_TABBED_H sentry.

