//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
// Revised by Yura Bidus 11/23/1998
/// \file
/// Implementation of class TButtonGadget.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/buttonga.h>
#include <owl/gadgetwi.h>
#include <owl/celarray.h>
#include <owl/uihelper.h>
#include <owl/uimetric.h>
#include <owl/system.h>
#include <owl/theme.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGadget);

#  if defined(BI_COMP_BORLANDC)
#    pragma warn -inl
#  endif

/// \addtogroup enabler
/// @{
//
/// Command enabler for button gadgets
//
/// Derived from TCommandEnabler, TButtonGadgetEnabler serves as a command enabler
/// for button gadgets. The functions in this class modify the text, check state,
/// and appearance of a button gadget.
//
class /*_OWLCLASS*/ TButtonGadgetEnabler : public TCommandEnabler {
  public:
/// Constructs a TButtonGadgetEnabler for the specified gadget. hReceiver is the
/// window receiving the message.
    TButtonGadgetEnabler(TWindow::THandle hReceiver, TButtonGadget* g)
    :
      TCommandEnabler(g->GetId(), hReceiver),
      Gadget(g)
    {
    }

    // Override TCommandEnabler virtuals
    //
    void  Enable(bool);
    void  SetText(LPCTSTR);
    void  SetCheck(int);

  protected:
/// The button gadget being enabled or disabled.
    TButtonGadget*  Gadget;
};
/// @}

#  if defined(BI_COMP_BORLANDC)
#    pragma warn .inl
#  endif

//
/// Overrides TCommandEnable::Enable. Enables or disables the keyboard, mouse input,
/// and appearance of the corresponding button gadget.
//
void
TButtonGadgetEnabler::Enable(bool enable)
{
  TCommandEnabler::Enable(enable);
  Gadget->SetEnabled(enable);
}

//
/// Handle the SetText request for a button gadget. Currently does nothing.
//
void
TButtonGadgetEnabler::SetText(LPCTSTR)
{
//# pragma warn -ccc
//# pragma warn -rch
//  CHECK(true);
//# pragma warn .rch
//# pragma warn .ccc
}

//
// Set the check-state for the button gadget
//
void
TButtonGadgetEnabler::SetCheck(int state)
{
  Gadget->SetButtonState(TButtonGadget::TState(state));
}

//----------------------------------------------------------------------------

//
/// Construct a button gadget that loads its own bitmap resource
//
/// Constructs a TButtonGadget object using the specified bitmap ID, button gadget
/// ID, and type, with enabled set to false and in a button-up state. The button
/// isn't enabled - its initial state before command enabling occurs. sharedGlyph
/// should be set true if the glyph is being held by the window instead of being
/// loaded from a resource.
//
TButtonGadget::TButtonGadget(TResId glyphResIdOrIndex,
                             int    id,
                             TType  type,
                             bool   enabled,
                             TState state,
                             bool   sharedGlyph)
:
  TGadget(id, ButtonUp),
  SharingGlyph(sharedGlyph)
{
  // If we are sharing the glyph held by our window, then we won't need to load
  // the resource
  //
  if (sharedGlyph) {
    ResId = 0;
    GlyphIndex = (int)(LPCTSTR)glyphResIdOrIndex;
    GlyphCount = 1;
  }
  else {
    ResId = glyphResIdOrIndex.IsString() ?
                strnewdup(glyphResIdOrIndex) : (LPCTSTR)glyphResIdOrIndex;
    GlyphIndex = -1;   // Need to get during BuildCelArray
    GlyphCount = 4;
  }

  CelArray = 0;
  BitmapOrigin.x = BitmapOrigin.y = 0;
  Type = type;
//  Repeat = repeat;
  SetTrackMouse(true);
  State = state;
  NotchCorners = true;
  Pressed = false;
  SetEnabled(enabled);
  SetAntialiasEdges(true);

  TRACEX(OwlGadget, OWL_CDLEVEL, "TButtonGadget constructed @" << (void*)this);
}

//
/// Destruct this button gadget, freeing up allocated resources
//
TButtonGadget::~TButtonGadget()
{
  delete CelArray;
  if (ResId.IsString())
    delete[] (LPSTR)(LPCTSTR)ResId;

  TRACEX(OwlGadget, OWL_CDLEVEL, "TButtonGadget destructed @" << (void*)this);
}

//
/// Initiate a command enable for this button gadget
//
/// Enables the button gadget to capture messages. Calls SendMessage to send a
/// WM_COMMAND_ENABLE message to the gadget window's parent, passing a
/// TCommandEnable: EvCommandEnable message for this button.
//
void
TButtonGadget::CommandEnable()
{
  PRECONDITION(Window);

  // Must send, not post here, since a ptr to a temp is passed
  //
  // This might be called during idle processing before the
  // HWND has created.  Therefore, confirm handle exists.
  //
  if (GetGadgetWindow()->GetHandle()) {
    TButtonGadgetEnabler ge(*GetGadgetWindow(), this);
    GetGadgetWindow()->HandleMessage(WM_COMMAND_ENABLE, 0, TParam2(&ge));
  }
}

//
/// SysColorChange responds to an EvSysColorChange message forwarded by the owning
/// TGadgetWindow by setting the dither brush to zero. If a user-interface bitmap
/// exists, SysColorchange deletes and rebuilds it to get the new button colors.
//
void
TButtonGadget::SysColorChange()
{
  TGadget::SysColorChange();
  delete CelArray;
  CelArray = 0;
  BuildCelArray();
}

//
/// Perform an exclusive checking of this gadget by unchecking the neighboring
/// exclusive button gadgets
//
void
TButtonGadget::CheckExclusively()
{
  PRECONDITION(Window);
#if defined(BI_COMP_GNUC)  
  TState state = State;
  if (state != Down) {
#else
  if ((TState)State != Down) {
#endif
    if (GetGadgetWindow()) {
      TButtonGadget*  first = 0;
      TButtonGadget*  last = this;

      // Look for the start of the group in which this button is located
      //
      for (TGadget* g = GetGadgetWindow()->FirstGadget(); g && g != this;
        g = GetGadgetWindow()->NextGadget(*g)) {

        TButtonGadget* bg = TYPESAFE_DOWNCAST(g, TButtonGadget);

        if (!bg || (bg->Type != Exclusive && bg->Type != SemiExclusive))
          first = 0;
        else if (!first)
          first = bg;
      }

      if (!first)
        first = this;

      // Look for the end of the group in which this button is located
      //
      while (GetGadgetWindow()->NextGadget(*last)) {
        TButtonGadget* bg = TYPESAFE_DOWNCAST(GetGadgetWindow()->NextGadget(*last), TButtonGadget);

        if (!bg || (bg->Type != Exclusive && bg->Type != SemiExclusive))
          break;
        else
          last = bg;
      }

      // Walk all the gadget between first and last (inclusive) and pop them up
      // if they are down
      //
      while (true) {
#if defined(BI_COMP_GNUC)  
        TState state = first->State;
        if (state == Down) {
#else
        if (first->State == Down) {
#endif
          first->State = Up;
          first->Invalidate();
          first->Update();
        }

        if (first == last)  // All done
          break;

        first = TYPESAFE_DOWNCAST(first->NextGadget(), TButtonGadget);
      }
    }

    State = Down;
  }
}

//
/// Set the state of a button. Handles setting down on an exclusive button to
/// pop out the neighbors
//
void
TButtonGadget::SetButtonState(TState newState)
{
#if defined(BI_COMP_GNUC)  
  TState state = State;
  if (newState != state) {
#else
  if (newState != (TState) State) {
#endif
    if ((Type == Exclusive || Type == SemiExclusive) && newState == Down)
      CheckExclusively();
    else
      State = newState;
    Invalidate();
    Update();
  }
}

//
/// Gets the size of the bitmap, calls TGadget::SetBounds to set the boundary of the
/// rectangle, and centers the bitmap within the button's rectangle.
//
void
TButtonGadget::SetBounds(const TRect& bounds)
{
  PRECONDITION(GetGadgetWindow());
  TRACEX(OwlGadget, 1, _T("TButtonGadget::SetBounds() enter @") << this <<
    _T(" bounds ") << bounds);

  TGadget::SetBounds(bounds);

  // Center the glyph within the inner bounds
  //
  TRect  innerRect;
  GetInnerRect(innerRect);

  TSize  bitmapSize = CelArray ? CelArray->CelSize() : GetGadgetWindow()->GetCelArray().CelSize();
  BitmapOrigin.x = innerRect.left + (innerRect.Width() - bitmapSize.cx) / 2;
  BitmapOrigin.y = innerRect.top + (innerRect.Height() - bitmapSize.cy) / 2;
  TRACEX(OwlGadget, 1, _T("TButtonGadget::SetBounds() leave @") << this <<
    _T(" bounds ") << bounds);
}

//
/// Find out how big this button gadget wants to be. Calculated using the base
/// size to get the borders, etc. plus the glyph size.
//
/// Stores the width and height (measured in pixels) of the button gadget in size.
/// Calls TGadget::GetDesiredSize to calculate the relationship between one
/// rectangle and another.
//
void
TButtonGadget::GetDesiredSize(TSize& size)
{
  PRECONDITION(GetGadgetWindow());
  TRACEX(OwlGadget, 1, _T("TButtonGadget::GetDesiredSize() enter @") << this <<
    _T(" size ") << size);

  TGadget::GetDesiredSize(size);

  // Build our CelArray or CelArray entry if not done already
  //
  if (!CelArray && GlyphIndex < 0)
    BuildCelArray();

  size += CelArray ? CelArray->CelSize() : GetGadgetWindow()->GetCelArray().CelSize();
  TRACEX(OwlGadget, 1, _T("TButtonGadget::GetDesiredSize() leave @") << this <<
    _T(" size ") << size);
}


//
/// Virtual function responsible for supplying the dib for the glyph. Can be
/// overriden to get dib from elsewhere, cache it, map colors differently, etc.
//
/// Returns 0 if no resource was specified for this button
//
TDib*
TButtonGadget::GetGlyphDib()
{
  PRECONDITION(GetGadgetWindow());

  if (ResId) {
    TDib* glyph = new TDib(*GetGadgetWindow()->GetModule(), ResId);
    glyph->MapUIColors( TDib::MapFace | TDib::MapText | TDib::MapShadow |
                          TDib::MapHighlight );
    return glyph;
  }
  return 0;
}

//
/// Virtual function responsible for releasing glyph dib as needed based on how
/// GetGlyphDib() got it (if different from new/delete).
//
void
TButtonGadget::ReleaseGlyphDib(TDib* glyph)
{
  delete glyph;
}

//
/// Build the CelArray member using the resource bitmap as the base glyph
/// CelArray may contain an existing cel array that should be deleted if
/// replaced.
//
/// The CelArray and glyph painting can work in one of the following ways:
///  - 1. ResId ctor is used, ...., glyph states are cached in this CelArray
///  - 2. ResId ctor is used, CelArray holds single glyph and paints state on
///      the fly
///  - 3. ResId ctor is used, glyph is added to Window's CelArray and paints
///      state on the fly
///  - 4. glyphIndex ctor is used, uses Window's CelArray glyph and paints state
///      on the fly
///
void
TButtonGadget::BuildCelArray()
{
  //PRECONDITION(GetGadgetWindow());

  TDib*  glyph = GetGlyphDib();

  // Case 4, no resource of our own--using a cel in our Window's CelArray
  //
  if (!glyph)
    return;

  // Case 3, add (or replace old) our glyph to our Window's CelArray
  //
  
	bool case2 = true; 
	if (SharingGlyph) 
	{ 
		CHECK(Window); 
		TSize glyphSize(glyph->Width(), glyph->Height()); 
		TCelArray& celArray = GetGadgetWindow()->GetCelArray(glyph->Width(), glyph->Height()); 
 
		if (glyphSize == TSize(celArray.CelSize().cx,celArray.CelSize().cy*celArray.NumRows())) 
		{ 
			case2 = false; 
			if (GlyphIndex >= 0) 
				celArray.Replace(GlyphIndex, *glyph); 
			else 
				GlyphIndex = celArray.Add(*glyph); 
		} 
	} 
 
	if (case2) 
	{   
    // Case 2, build a CelArray containing only the normal glyph state
    //
    delete CelArray;
    CelArray = new TCelArray(*glyph, 1);
    GlyphIndex = 0;
    SharingGlyph = false;
  }

  ReleaseGlyphDib(glyph);

  // Case 1, for compatibility for now...
  //
}

//
//
//
void          
TButtonGadget::SetGlyphIndex(int index)
{
  if(SharingGlyph)
    GlyphIndex = index;
}

//
/// Perform all of the painting for this button gadget
//
/// Gets the width and height of the window frame (in pixels), calls GetImageSize to
/// retrieve the size of the bitmap, and sets the inner rectangle to the specified
/// dimensions. Calls TGadget::PaintBorder to perform the actual painting of the
/// border of the control. Before painting the control, Paint determines whether the
/// corners of the control are notched, and then calls GetSysColor to see if
/// highlighting or shadow colors are used. Paint assumes the border style is plain.
/// Then, Paint draws the top, left, right, and bottom of the control, adjusts the
/// position of the bitmap, and finishes drawing the control using the specified
/// embossing, fading, and dithering.
//
void
TButtonGadget::Paint(TDC& dc)
{
  PRECONDITION(Window);

  PaintBorder(dc);

  // Get the inner rect to use as the button face
  //
  TRect  faceRect;
  GetInnerRect(faceRect);

  PaintFace(dc, faceRect);
}

void
TButtonGadget::PaintBorder(TDC& dc)
{
  // Paint the button frame depending on the active styles.
  //
  bool flat = GetGadgetWindow()->GetFlatStyle() & TGadgetWindow::FlatStandard;
  if (flat)
  {
    // Calculate state and border/background style.
    //
    bool ok = GetEnabled();
    bool down = ok && (Pressed || State == Down);
    bool hover = ok && (IsHaveMouse() || GetGadgetWindow()->GadgetGetCaptured() == this);
    ChangeBorderStyle(down ? Recessed : hover ? Raised : None);

    bool xpstyle = GetGadgetWindow()->IsThemed();
    if (xpstyle)
    {
      // Draw themed background. This ignores all our border styles.
      //
      int state = down ? TS_PRESSED : hover ? TS_HOT : 0;
      TThemePart part(GetGadgetWindow()->GetHandle(), (LPCWSTR)L"TOOLBAR", TP_BUTTON, state);
      TRect rect(0, 0, GetBounds().Width(), GetBounds().Height());
      part.DrawTransparentBackground(GetGadgetWindow()->GetHandle(), dc, rect);
    }
    else 
    {
      // Themes are not available. Draw old flat style.
      //
      TGadget::PaintBorder(dc);
    }
  }
  else // !flat
  {
    ChangeBorderStyle((Pressed || State == Down) ? ButtonDn : ButtonUp);
    TGadget::PaintBorder(dc);
  }
}

//
/// Paints the face of the button.
//
void
TButtonGadget::PaintFace(TDC& dc, const TRect& rect)
{
  // Determine which CelArray to paint from: ours or our Window's
  //
  TCelArray& celArray = CelArray ? *CelArray : GetGadgetWindow()->GetCelArray();

  // set Current Row if enabled style FlatGrayButtons
  if(GetGadgetWindow()->GetFlatStyle()&TGadgetWindow::FlatGrayButtons){
    if(IsHaveMouse())
      celArray.SetCurRow(0);// normal
    else if(celArray.NumRows()>1)
      celArray.SetCurRow(1); // set gray row
  }

  // Calc the source rect from the celarray. The dest point of the glyph is
  // relative to the face rect.
  //
  TRect  srcRect(celArray.CelRect(GlyphIndex));
  TPoint dstPt(BitmapOrigin - rect.TopLeft());

  // Create a UI Face object for this button & let it paint the button face
  //
  TUIFace face(rect, celArray, TColor::Sys3dFace);
  bool flat = GetGadgetWindow()->GetFlatStyle() & TGadgetWindow::FlatStandard;
  TUIFace::TState state = TUIFace::Disabled;
  bool pressed = false;
  if (GetEnabled()) switch (State)
  {
    case Indeterminate:
      state = TUIFace::Indeterm;
      pressed = Pressed;
      break;

    case Down:
      if (flat)
      {
        bool hover = !IsHaveMouse() && GetGadgetWindow()->GadgetGetCaptured() != this;
        state = hover ? TUIFace::Down : TUIFace::Normal;
        pressed = hover ? true : Pressed;
  }
  else
      {
        state = Pressed ? TUIFace::Down : TUIFace::Normal;
        pressed = Pressed;
    }
      break;

    default:
      state = TUIFace::Normal;
      pressed = Pressed;
  }
  face.Paint(dc, srcRect, dstPt, state, pressed, false); // no fill (transparent)
}

//
/// Begin button pressed state, repaint & enter menuselect state
//
/// When the mouse button is pressed, BeginPressed sets Pressed to true, paints the
/// pressed button, and sends menu messages to the gadget window's parent.
void
TButtonGadget::BeginPressed(const TPoint&)
{
  Pressed = true;
  SetInMouse(true);
  Invalidate();
  Update();
  if (GetGadgetWindow()->GetHintMode() == TGadgetWindow::PressHints)
    GetGadgetWindow()->SetHintCommand(GetId());
}

//
/// Cancel pressed state, repaint & end menuselect state
//
/// When the mouse button is released, CancelPressed sets Pressed to false, paints
/// the button, and sends menu messages to the gadget window's parent. Sets InMouse
/// to mstate.
//
void
TButtonGadget::CancelPressed(const TPoint&, bool mstate)
{
  Pressed = false;
  SetInMouse(mstate);
  Invalidate(true);
  Update();
  if (GetGadgetWindow()->GetHintMode() == TGadgetWindow::PressHints)
    GetGadgetWindow()->SetHintCommand(-1);
}

//
/// The action method called on a completed 'click', generates WM_COMMAND
//
/// Invoked by mouse-up event inside the Gadget. Sets member data "Pressed"
/// to false, changes state for attribute buttons, and paints the button
/// in its current state.
//
/// Command buttons just send the command
/// Exclusive buttons check themselves and uncheck neighbors.
/// NonExclusive buttons toggle their check state.
/// SemiExclusive uncheck neighbors on press, but can also be unpressed.
//
/// Invoked when the mouse is in the "up" state, Activate sets Pressed to false,
/// changes the state for attribute buttons, and paints the button in its current
/// state. To do this, it calls CancelPressed(), posts a WM_COMMAND message to the
/// gadget window's parent, and sends menu messages to the gadget window's parent.
//
void
TButtonGadget::Activate(const TPoint& pt)
{
#if defined(BI_COMP_GNUC)  
  TState state = State;
  TType type   = Type;
#endif
  
  switch (Type) {
    case Exclusive:
#if defined(BI_COMP_GNUC)  
      if (state != Down)
#else
      if ((TState)State != Down)
#endif
        CheckExclusively();
      break;

    case NonExclusive:
#if defined(BI_COMP_GNUC)  
      State = state == Up ? Down : Up;
#else
      State = State == Up ? Down : Up;
#endif
      break;

    case SemiExclusive:
#if defined(BI_COMP_GNUC)  
      if (state != Down)
#else
      if ((TState)State != Down)
#endif
        CheckExclusively();
      else
        State = Up;
      break;
    default:
    case Command:
    case RepeatCmd:
      break;
  }

  // Unpress the button
  //
  CancelPressed(pt, true);


  // Send the associated command for all enabled buttons except for exclusive
  // buttons that are being poped up
  //
#if defined(BI_COMP_GNUC)  
  if (!((type == Exclusive || type == SemiExclusive) && state != Down) &&
#else
  if (!((Type == Exclusive || Type == SemiExclusive) && (TState)State != Down) &&
#endif
      GetEnabled())
    GetGadgetWindow()->PostMessage(WM_COMMAND, GetId());
}

//
/// Overrides TGadget member function and responds to a left mouse button click by
/// calling BeginPressed.
//
void
TButtonGadget::LButtonDown(uint modKeys, const TPoint& pt)
{
  TGadget::LButtonDown(modKeys, pt);
  BeginPressed(pt);
}

//
/// Mouse has entered this button, (not pressed). Show hints if that style
/// is enabled
//
/// Called when the mouse enters the boundary of the button gadget. modKeys
/// indicates the virtual key information and can be any combination of the
/// following values: MK_CONTROL, MK_LBUTTON, MK_MBUTTON, MK_RBUTTON,  or MK_SHIFT.
/// p indicates where the mouse entered the button gadget.
//
void
TButtonGadget::MouseEnter(uint modKeys, const TPoint& pt)
{
  TGadget::MouseEnter(modKeys, pt);
  if (GetGadgetWindow()->GetHintMode() == TGadgetWindow::EnterHints)
    GetGadgetWindow()->SetHintCommand(GetId());
}

//
/// Mouse has moved (back) into this gadget. Push or pull the button up & down
/// as needed.
//
/// Calls TGadget::MouseMove in response to the mouse being dragged. If the mouse
/// moves off the button, MouseMove calls CancelPressed(). If the mouse moves back
/// onto the button, MouseMove calls BeginPressed().
///
void
TButtonGadget::MouseMove(uint modKeys, const TPoint& pt)
{
  TGadget::MouseMove(modKeys, pt);

  bool hit = PtIn(pt);
  if (Pressed) {
    if (!hit)
      CancelPressed(pt, false);
  }
  else if (hit)
    BeginPressed(pt);
}

//
/// Mouse has left this button, (not pressed). Hide hints if that style
/// is enabled
//
/// Called when the mouse leaves the boundary of the button gadget. modKeys
/// indicates the virtual key information and can be any combination of the
/// following values: MK_CONTROL, MK_LBUTTON, MK_MBUTTON, MK_RBUTTON,  or MK_SHIFT.
/// p indicates the place where the mouse left the button gadget.
//
void
TButtonGadget::MouseLeave(uint modKeys, const TPoint& pt)
{
  TGadget::MouseLeave(modKeys, pt);
  if (GetGadgetWindow()->GetHintMode() == TGadgetWindow::EnterHints)
    GetGadgetWindow()->SetHintCommand(-1);
}

//
/// Handle the mouse left button up & leave the pressed state. If the mouse is
/// still on this button, i.e. it is still pressed, then perform the button
/// action.
//
/// Overrides TGadget member functions and responds to a release of the left mouse
/// button by calling Activate.
//
void
TButtonGadget::LButtonUp(uint modKeys, const TPoint& pt)
{
  TGadget::LButtonUp(modKeys, pt);
  if (Pressed)
    Activate(pt);
  else if(GetGadgetWindow()->GetFlatStyle()&TGadgetWindow::FlatStandard)
    Invalidate();
}

} // OWL namespace

//==============================================================================

