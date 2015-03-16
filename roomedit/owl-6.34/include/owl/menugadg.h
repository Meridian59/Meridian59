//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition TMenuGadget class
//----------------------------------------------------------------------------

#if !defined(OWL_MENUGADGET_H)
#define OWL_MENUGADGET_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/textgadg.h>
#include <owl/menu.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup gadgets
/// @{
/// \class TMenuGadget
// ~~~~~ ~~~~~~~~~~~
/// TMenuGadget is a text gadget that, when pressed, acts as a pop-up menu.
class _OWLCLASS TMenuGadget : public TTextGadget {
  public:
    TMenuGadget(TMenu& menu, TWindow* window, int id = 0,
                TBorderStyle borderStyle = TGadget::ButtonUp,
                LPTSTR text = 0, TFont* font = new TGadgetWindowFont);
   ~TMenuGadget();

    // Override from TGadget
    //
    virtual void LButtonDown(uint modKeys, const TPoint& p);
    /// Added support for Flat Style
    //
    virtual void PaintBorder(TDC& dc);
    virtual void GetDesiredSize(TSize& size);

    //
    /// Added support for Hot text
    virtual TColor GetEnabledColor() const;

  private:
    TPopupMenu* PopupMenu;
    TWindow*    CmdTarget;
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_MENUGADGET_H
