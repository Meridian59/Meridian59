//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TMDIChild.
//----------------------------------------------------------------------------

#if !defined(OWL_MDICHILD_H)
#define OWL_MDICHILD_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/framewin.h>


namespace owl {

#include <owl/preclass.h>

/// \addtogroup frame
/// @{

class _OWLCLASS TMDIClient;

//
/// \class TMDIChild
// ~~~~~ ~~~~~~~~~
/// TMDIChild defines the basic behavior of all MDI child windows. Child windows can
/// be created inside the client area of a parent window. Because child windows
/// exist within, and are restricted to the parent window's borders,  the parent
/// window defined before the child is defined. For example, a dialog box is a
/// window that contains child windows, often referred to as dialog box controls.
///
/// To be used as MDI children, classes must be derived from TMDIChild. MDI children
/// can inherit keyboard navigation, focus handling, and icon support from
/// TFrameWindow. TMDIChild is a streamable class.
//
class _OWLCLASS TMDIChild : virtual public TFrameWindow {
  public:
    TMDIChild(TMDIClient&     parent,
              LPCTSTR         title = 0,
              TWindow*        clientWnd = 0,
              bool            shrinkToClient = false,
              TModule*        module = 0);

    TMDIChild(TMDIClient&     parent,
              const tstring&  title,
              TWindow*        clientWnd = 0,
              bool            shrinkToClient = false,
              TModule*        module = 0);

    TMDIChild(HWND hWnd, TModule* module = 0);

   ~TMDIChild();

    // Override virtual methods defined by TWindow
    //
    auto PreProcessMsg(MSG&) -> bool override;
    auto ShowWindow(int cmdShow) -> bool override;
    auto EnableWindow(bool enable) -> bool override;
    void Destroy(int retVal = 0) override;

  protected:
    auto PerformCreate() -> THandle override;
    virtual TResult DefWindowProc(TMsgId, TParam1, TParam2);

    void       EvMDIActivate(HWND hWndActivated, HWND hWndDeactivated);
    bool       EvNCActivate(bool active);
    void        EvMenuSelect(uint menuItemId, uint flags, HMENU hMenu);
    void       EvEnterIdle(uint source, HWND hWndDlg);

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TMDIChild(const TMDIChild&);
    TMDIChild& operator =(const TMDIChild&);

  DECLARE_RESPONSE_TABLE(TMDIChild);
  DECLARE_STREAMABLE_OWL(TMDIChild, 1);
};

DECLARE_STREAMABLE_INLINES(owl::TMDIChild);

#include <owl/posclass.h>

typedef TMDIChild TMdiChild;

/// @}

} // OWL namespace


#endif  // OWL_MDICHILD_H
