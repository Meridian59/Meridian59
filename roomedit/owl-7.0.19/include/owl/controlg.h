//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Class TControlGadget definition. TControlGadget is a gadget that takes a
/// TWindow and allows it to be inserted into a gadget window. In effect it is
/// an adapter that allows arbitrary controls to be used like gadgets in a tool
/// bar, status bar, etc.
//----------------------------------------------------------------------------

#if !defined(OWL_CONTROLG_H)
#define OWL_CONTROLG_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/gadget.h>


namespace owl {

#include <owl/preclass.h>

/// \addtogroup gadgets
/// @{
/// \class TControlGadget
// ~~~~~ ~~~~~~~~~~~~~~
/// TControlGadget serves as a surrogate for TControl so that you can place TControl
/// objects such as edit controls, buttons, sliders, gauges, or third-party
/// controls, into a gadget window. If necessary, TControlGadget sets a parent
/// window and creates the control gadget. See TGadget for more information about
/// gadget objects.
//
class _OWLCLASS TControlGadget : public TGadget {
  public:
    TControlGadget(TWindow& control, TBorderStyle = None);
   ~TControlGadget() override;

   void SetVisible(bool visible) override;

  protected:
    void Created() override;
    void Inserted()  override;
    void Removed() override;

    void           InvalidateRect(const TRect& rect, bool erase = true);
    void           Update();  // Paint now if possible

    void SetBounds(const TRect&) override;
    void GetDesiredSize(TSize&) override;

  public:
    TWindow*       GetControl() const;
    void           SetControl(TWindow* control);

  protected_data:
/// Points to the control window that is managed by this TControlGadget.
    TWindow*       Control;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TControlGadget(const TControlGadget&);
    TControlGadget& operator =(const TControlGadget&);
};
/// @}

#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline Implementations
//

//
/// Return the control that is simulating a gadget.
//
inline TWindow* TControlGadget::GetControl() const {
  return Control;
}

//
/// Set the control that is simulating a gadget.
//
inline void TControlGadget::SetControl(TWindow* control) {
  Control = control;
}

} // OWL namespace


#endif  // OWL_CONTROLG_H
