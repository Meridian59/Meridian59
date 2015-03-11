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
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/framewin.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
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
    bool       PreProcessMsg(MSG& msg);
    bool       ShowWindow(int cmdShow);
    bool       EnableWindow(bool enable);
    void       Destroy(int retVal = 0);

  protected:
    virtual TPerformCreateReturnType PerformCreate(int deprecated_argument = 0);
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
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TMDIChild, 1);
  DECLARE_STREAMABLE_OWL(TMDIChild, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TMDIChild );

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>

typedef TMDIChild TMdiChild;

/// @}

} // OWL namespace


#endif  // OWL_MDICHILD_H
