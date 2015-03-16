//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TControl.  This defines the basic behavior of all
/// controls.
//----------------------------------------------------------------------------

#if !defined(OWL_CONTROL_H)
#define OWL_CONTROL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/window.h>

namespace owl {


// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup ctrl
/// @{
//
//
/// \class TControl
// ~~~~~ ~~~~~~~~
/// TControl unifies its derived control classes, such as TScrollBar,
/// TControlGadget, and TButton. Control objects of derived classes are used to
/// represent control interface elements. A control object must be used to create a
/// control in a parent TWindow object or a derived window. A control object can be
/// used to facilitate communication between your application and the controls of a
/// TDialog object. TControl is a streamable class.
//
class _OWLCLASS TControl : virtual public TWindow {
  public:
    TControl(TWindow*        parent,
             int             id,
             LPCTSTR          title,
             int x, int y, int w, int h,
             TModule*        module = 0);

    TControl(TWindow*        parent,
             int             id,
             const tstring& title,
             int x, int y, int w, int h,
             TModule*        module = 0);

    TControl(TWindow* parent, int resourceId, TModule* module = 0);
    TControl(TWindow* parent, int resourceId, const tstring& title, TModule* = 0); 
   ~TControl();

  protected:

    /// Constructor to alias non-OWL control
    //
    TControl(HWND hWnd, TModule* module = 0);

    // These methods are called for owner-draw controls (buttons, list boxes,
    // and combo boxes)
    //
    virtual int   CompareItem(COMPAREITEMSTRUCT & compareInfo);
    virtual void  DeleteItem(DELETEITEMSTRUCT & deleteInfo);
    virtual void  MeasureItem(MEASUREITEMSTRUCT & measureInfo);
    virtual void  DrawItem(DRAWITEMSTRUCT & drawInfo);

    // Default behavior for DrawItem is to call one of the following based on
    // the draw type:
    //
    virtual void  ODADrawEntire(DRAWITEMSTRUCT & drawInfo);
    virtual void  ODAFocus(DRAWITEMSTRUCT & drawInfo);
    virtual void  ODASelect(DRAWITEMSTRUCT & drawInfo);

    // Message response functions
    //
    void          EvPaint();
    int           EvCompareItem(uint ctrlId, const COMPAREITEMSTRUCT& comp);
    void          EvDeleteItem(uint ctrlId, const DELETEITEMSTRUCT& del);
    void          EvDrawItem(uint ctrlId, const DRAWITEMSTRUCT& draw);
    void          EvMeasureItem(uint ctrlId, MEASUREITEMSTRUCT & meas);

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TControl(const TControl&);
    TControl& operator =(const TControl&);

  DECLARE_RESPONSE_TABLE(TControl);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TControl, 2);
  DECLARE_STREAMABLE_OWL(TControl, 2);
};
/// @}
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TControl );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace


#endif  // OWL_CONTROL_H
