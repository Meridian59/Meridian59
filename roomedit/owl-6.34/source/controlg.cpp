//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TControlGadget.
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/controlg.h>
#include <owl/gadgetwi.h>
#include <owl/tooltip.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGadget);

//
/// Creates a TControlGadget object associated with the specified TControl window.
//
TControlGadget::TControlGadget(TWindow& control, TBorderStyle border)
:
  TGadget(control.GetId(), border)
{
  Control = &control;
  Control->ModifyStyle(0, WS_CLIPSIBLINGS);  // Make sure relayout paints OK
  TRACEX(OwlGadget, OWL_CDLEVEL, "TControlGadget constructed @" << (void*)this);
}

//
/// Destroys a TControlGadget object and removes it from the associated window.
//
TControlGadget::~TControlGadget()
{
  if(Control)
    Control->Destroy(0);
  delete Control;
  TRACEX(OwlGadget, OWL_CDLEVEL, "TControlGadget destructed @" << (void*)this);
}

//
/// Calls TGadget::SetBounds and passes the dimensions of the control gadget's
/// rectangle. SetBounds informs the control gadget of a change in its bounding
/// rectangle.
//
void
TControlGadget::SetBounds(const TRect& bounds)
{
  TRACEX(OwlGadget, 1, "TControlGadget::SetBounds() enter @" << (void*)this <<
    " bounds = " << bounds);

  // Set the gadget bounds, then move & repaint the control
  //
  TGadget::SetBounds(bounds);
  Control->SetWindowPos(0, Bounds, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);

  TRACEX(OwlGadget, 1, "TControlGadget::SetBounds() leave @" << (void*)this <<
    " bounds = " << bounds);
}

//
/// Calls TGadget::GetDesiredSize and passes the size of the control gadget. Use
/// GetDesiredSize to find the size the control gadget needs to be in order to
/// accommodate the borders and margins as well as the highest and widest control
/// gadget.
//
void
TControlGadget::GetDesiredSize(TSize& size)
{
  TRACEX(OwlGadget, 1, "TControlGadget::GetDesiredSize() enter @" << (void*)this <<
    " size = " << size);
  TGadget::GetDesiredSize(size);

  // !CQ get from Handle if created?
  if(Control->GetHandle()){
    TRect rect = Control->GetWindowRect();
    if (ShrinkWrapWidth)
      size.cx += rect.Width();
    if (ShrinkWrapHeight)
      size.cy += rect.Height();
  }
  else{
    if (ShrinkWrapWidth)
      size.cx += Control->GetWindowAttr().W;
    if (ShrinkWrapHeight)
      size.cy += Control->GetWindowAttr().H;
  }
  TRACEX(OwlGadget, 1, "TControlGadget::GetDesiredSize() leave @" << (void*)this <<
    " size = " << size);
}

//
/// Virtual called after the window holding a gadget has been created
//
void
TControlGadget::Created()
{
  PRECONDITION(GetGadgetWindow());
  PRECONDITION(GetGadgetWindow()->GetHandle());

  // Create control is necessary
  //
  Control->SetParent(GetGadgetWindow());
  if (GetGadgetWindow()->GetHandle() && !Control->GetHandle()) {
    Control->Create();
    Control->ShowWindow(SW_SHOWNA);
  }

  // Register control with the tooltip window (if there's one)
  //
  TTooltip* tooltip = GetGadgetWindow()->GetTooltip();
  if (tooltip) {
    CHECK(tooltip->GetHandle());

    // Register the control with the tooltip
    //
    if (Control->GetHandle()) {
      TToolInfo toolInfo(GetGadgetWindow()->GetHandle(), Control->GetHandle());
      tooltip->AddTool(toolInfo);
    }
  }
}

//
/// Called when the control gadget is inserted in the parent window. Displays the
/// window in its current size and position.
//
/// Override the Inserted() virtual to take the oportunity to make sure that the
/// control window has been created and shown
//
void
TControlGadget::Inserted()
{
  TRACEX(OwlGadget, 1, "TControlGadget::Inserted @" << (void*)this);
  Control->SetParent(GetGadgetWindow());

  if (GetGadgetWindow()->GetHandle())
  {
    if (!Control->GetHandle())
    {
      Control->Create();
    }

    Control->ShowWindow(SW_SHOWNA);
  }
}

//
/// Called when the control gadget is removed from the parent window.
//
/// Override the Remove() virtual to take the oportunity to unparent the
/// control window from the owning Window
//
void
TControlGadget::Removed()
{
  TRACEX(OwlGadget, 1, "TControlGadget::Removed @" << (void*)this);
  Control->ShowWindow(SW_HIDE); // Sirma Update
  Control->SetParent(0);
  // Should we destroy the control at this point??
  // Since it's no longer in the parent's child-list, there's a potential
  // leak. However, the semantics of this function is 'Removed' - therefore
  // one could be removing the control to be reinserted in another 
  // gadgetwindow.

  // Unregister ourself with the tooltip window (if there's one)
  //
  if (GetGadgetWindow() && GetGadgetWindow()->GetHandle()) {
    TTooltip* tooltip = GetGadgetWindow()->GetTooltip();
    if (tooltip) {
      CHECK(tooltip->GetHandle());

      TToolInfo toolInfo(GetGadgetWindow()->GetHandle(), Control->GetHandle());
      tooltip->DeleteTool(toolInfo);
    }
  }
}

//
/// Invalidate a rectangle within this gadget by invalidating the rect in the
/// control window in addition to the owning Window
//
void
TControlGadget::InvalidateRect(const TRect& rect, bool erase)
{
  TGadget::InvalidateRect(rect, erase);
  if (Control->GetHandle())
    Control->InvalidateRect(rect, erase);
}

//
/// Updates the client area of the specified window by immediately sending a
/// WM_PAINT message.
//
void
TControlGadget::Update()
{
  if (Control->GetHandle())
    Control->UpdateWindow();
}


} // OWL namespace
/* ========================================================================== */
