// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// gadgetex.cpp: implementation file
// Version:      1.6
// Date:         08.11.1998
// Author:       Dieter Windau
//
// Freeware OWL classes that extents the dockable and gadget system
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// Tested with Borland C++ 5.02, OWL 5.02, OWL6 patch #3 and with Windows
// NT 4.0 SP3 but I think the class should work with Windows 95 too.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// This package contains many classes and source that are based on other OWL
// developers work. Very special thanks to Alan Chambers, Christopher Kohlhoff,
// Jo Parrello, Mark Hatsell, Michael Mogensen and Yura Bidus
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://members.aol.com/softengage/index.htm
//
// ****************************************************************************

#include <owlext\pch.h>
#pragma hdrstop

#include <owl/celarray.h>
#include <owl/uihelper.h>
#include <owl/tooltip.h>
#include <owl/rcntfile.h>
#include <owl/rcntfile.rh>

#include <owl/uimetric.h>

#include <owlext/util.h>
#include <owlext/harborex.h>
#include <owlext/dockingex.h>
#include <owlext/gadgetex.h>
#include <owlext/gadgctrl.h>

using namespace owl;

namespace OwlExt {

DIAG_DEFINE_GROUP(GadgetEx, 1, 0);

// ******************** TInvisibleGadgetEx ************************************

TInvisibleGadgetEx::TInvisibleGadgetEx()
:
TGadget(CM_INVISIBLEGADGET)
{
  ShrinkWrapWidth = ShrinkWrapHeight = false;
  SetEnabled(false);
  SetVisible(true);

  Bounds.right = Bounds.bottom = 22;
  TRACEX(GadgetEx, 0, "TInvisibleGadgetEx constructed @" << (void*)this);
}

TInvisibleGadgetEx::~TInvisibleGadgetEx()
{
  TRACEX(GadgetEx, 0, "TInvisibleGadgetEx destructed @" << (void*)this);
}

// ******************** TButtonGadgetEnablerEx *******************************

//
// Command enabler for my extended button gadgets
//
class TButtonGadgetEnablerEx : public TCommandEnabler {
public:
  TButtonGadgetEnablerEx(TWindow::THandle hReceiver, TButtonGadgetEx* g)
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
  TButtonGadgetEx*  Gadget;
};

//
// Enable or disable the button gadget
//
void
TButtonGadgetEnablerEx::Enable(bool enable)
{
  TCommandEnabler::Enable(enable);
  Gadget->SetEnabled(enable);
}

//
// Handle the SetText request for a button gadget. Currently does nothing.
//
void
TButtonGadgetEnablerEx::SetText(LPCTSTR)
{
# pragma warn -ccc
# pragma warn -rch
  CHECK(true);
# pragma warn .rch
# pragma warn .ccc
}

//
// Set the check-state for the button gadget
//
void
TButtonGadgetEnablerEx::SetCheck(int state)
{
  // Call my SetButtonState
  Gadget->SetButtonState(TButtonGadget::TState(state));
}

// ******************** TButtonGadgetEx ***************************************

TButtonGadgetEx::TButtonGadgetEx(TResId glyphResIdOrIndex,
                 int    id,
                 TType  type,
                 bool   enabled, // initial state before cmd enabling
                 TState state,
                 bool   sharedGlyph):
TButtonGadget(glyphResIdOrIndex, id, type, enabled, state, sharedGlyph)
{
  MouseInGadget = false;
  TRACEX(GadgetEx, 0, "TButtonGadgetEx constructed @" << (void*)this);
}

TButtonGadgetEx::~TButtonGadgetEx()
{
  TRACEX(GadgetEx, 0, "TButtonGadgetEx destructed @" << (void*)this);
}

//
// Overwrite to call my BetterChecckExclusively() and my SyncronizeButtons()
// It is not a virtual function, but they are called from TButtonGadgetEnablerEx
//
void
TButtonGadgetEx::SetButtonState(TState newState)
{
  if (newState != State) {
    switch (Type)
    {
    case Exclusive:
      if (newState == Down)
        BetterCheckExclusively();
      break;

    case NonExclusive:
      State = newState;
      SyncronizeButtons(GetId(), newState);
      break;

    case SemiExclusive:
      BetterCheckExclusively();
      break;

    case Command:
      State = newState;
      Invalidate(); // Call my not virtual Invalidate() function
      Update();
      break;

    default: // TODO: RepeatCmd is not handled; handle or clarify.
      WARN(true, _T("TButtonGadgetEx::SetButtonState: Switch doesn't handle Type: ") << Type);
      break;
    }
  }
}

//
// Set resId of shown bitmap if Sharing is false or
// set glyphIndex of shown bitmap if Sharing is true
//
void
TButtonGadgetEx::SetResId(TResId glyphResIdOrIndex)
{
  // The user can't set no bitmap
  //
  if (glyphResIdOrIndex == 0)
    return;

  delete CelArray;
  if (ResId.IsString())
    delete[] ResId.GetString();

  // If we are sharing the glyph held by our window, then we won't need to load
  // the resource
  //
  if (SharingGlyph) {
    GlyphIndex = glyphResIdOrIndex.GetInt();
  }
  else {
    ResId = glyphResIdOrIndex.IsString() ?
      TResId(strnewdup(glyphResIdOrIndex)) : glyphResIdOrIndex;
  }
  CelArray = 0;

  if (Window) {
    BuildCelArray();
    Invalidate();  // Call my not virtual Invalidate() function
    Update();
  }
}

//
// Reset the members to default values.
//
void TButtonGadgetEx::Reset()
{
  TButtonGadgetDesc* bgd = TYPESAFE_DOWNCAST(
    GadgetDescriptors->Find(GetId()), TButtonGadgetDesc);
  if (bgd) {
    SetResId(bgd->BmpResId);
  }
}

//
// Returns true, if the members have the default values
//
bool TButtonGadgetEx::HasDefaultValues()
{
  TButtonGadgetDesc* bgd = TYPESAFE_DOWNCAST(
    GadgetDescriptors->Find(GetId()), TButtonGadgetDesc);
  if (bgd && TResId(bgd->BmpResId) == ResId)
    return true;
  return false;
}

//
// Initiate a command enable for this button gadget
//
void
TButtonGadgetEx::CommandEnable()
{
  PRECONDITION(Window);

  TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(Window, TDockableControlBarEx);
  if (dcb) {
    THarborEx* harbor = TYPESAFE_DOWNCAST(dcb->GetHarbor(), THarborEx);
    if (harbor && harbor->IsCustomizeMode()) {
      SetEnabled(true);
      return;
    }
  }

  // Must send, not post here, since a ptr to a temp is passed
  //
  // This might be called during idle processing before the
  // HWND has created.  Therefore, confirm handle exists.
  //
  if (Window->GetHandle()) {
    TButtonGadgetEnablerEx tempEnabler(*Window, this);
    Window->HandleMessage(
      WM_COMMAND_ENABLE,
      0,
      // Importand: !!! use my TButtonGadgetEnablerEx !!!
      TParam2(&tempEnabler)
      );
  }
}


//
// Overwrite to erase the background for flat style
// It is not a virtual function, but they are called from the most important
// functions in TButtonGadgetEx
//
void
TButtonGadgetEx::Invalidate()
{
  TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(Window, TDockableControlBarEx);
  if (dcb) {
    if(dcb->GetFlatStyle())
      TGadget::Invalidate(true); // is flat style -> repaint gadget TWindow
    else
      TGadget::Invalidate(false);
  }
}

//
// Make sure our style matches our state, then paint our gadget borders
//
void TButtonGadgetEx::CheckStyleAndState()
{
  TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(Window, TDockableControlBarEx);
  if (dcb) {
    BorderStyle = None;
    if (dcb->GetFlatStyle()) {
      if (GetEnabled()) {
        if (MouseInGadget)
          BorderStyle = (Pressed || State == Down )? Recessed : Raised;
        else if(State == Down)
          BorderStyle = Recessed;
      }
    }
    else
      BorderStyle = (Pressed || State == Down) ? ButtonDn : ButtonUp;
  }
}

//
// Perform all of the painting for this button gadget
//
void TButtonGadgetEx::Paint(TDC& dc)
{
  TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(Window, TDockableControlBarEx);
  if (dcb == 0) {
    TButtonGadget::Paint(dc);
    return;
  }
  else {

    // Make sure our style matches our state, then paint our gadget borders
    // This do the same as OWL60 paint source fragment
    //
    CheckStyleAndState();

    PaintBorder(dc);

    // Determine which CelArray to paint from: ours or our Window's
    //
    TCelArray& celArray = CelArray ? *CelArray : Window->GetCelArray();

    // Get the inner rect to use as the button face
    //
    TRect  faceRect;
    GetInnerRect(faceRect);

    // Calc the source rect from the celarray. The dest point of the glyph is
    // relative to the face rect.
    //
    TRect  srcRect(celArray.CelRect(GlyphIndex));
    TPoint dstPt(BitmapOrigin - faceRect.TopLeft());

    // Create a UI Face object for this button & let it paint the button face
    //
    TUIFace face(faceRect, celArray);

    if (!GetEnabled()) {
      face.Paint(dc, srcRect, dstPt, TUIFace::Disabled, false);
      return;
    }
    else if (State == Indeterminate) { ///Y.B ??????????
      face.Paint(dc, srcRect, dstPt, TUIFace::Indeterm, Pressed);
      return;
    }
    else if (State == Down) {
      if(dcb->GetFlatStyle()) {
        face.Paint(dc, srcRect, MouseInGadget ? dstPt+TPoint(1,1) : dstPt,
          MouseInGadget ? TUIFace::Normal : TUIFace::Down, Pressed);
        return;
      }
      else if (!Pressed) {
        face.Paint(dc, srcRect, dstPt, TUIFace::Down, false);
        return;
      }
    }
    face.Paint(dc, srcRect, dstPt, TUIFace::Normal, Pressed);
  }
}

//
// Paint the border of the gadget based on the BorderStyle member
//
void
TButtonGadgetEx::PaintBorder(TDC& dc)
{
  TButtonGadget::PaintBorder(dc);
}

//
// Mouse is entering this gadget. Called by gadget window if no other gadget
// has capture
//
void
TButtonGadgetEx::MouseEnter(uint modKeys, const TPoint& point)
{
  //
  // for flat ala IE 3.0 toolbar
  //
  TButtonGadget::MouseEnter(modKeys, point);
  TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(Window, TDockableControlBarEx);
  if (dcb) {
    MouseInGadget = false;
    THarborEx* harbor = TYPESAFE_DOWNCAST(dcb->GetHarbor(), THarborEx);
    if (harbor && harbor->IsCustomizeMode() == false) {
      MouseInGadget = true;
      Invalidate(); // Call my not virtual Invalidate() function
    }
  }
}


//
// Overwrite to call my BetterChecckExclusively() and my SyncronizeButtons()
//
void
TButtonGadgetEx::Activate(const TPoint& pt)
{
  switch (Type)
  {
  case Exclusive:
    if (State != Down)
      BetterCheckExclusively();
    break;

  case NonExclusive:
    State = State == Up ? Down : Up;
    SyncronizeButtons(GetId(), State);
    break;

  case SemiExclusive:
    BetterCheckExclusively();
    break;

  default: // TODO: Command and RepeatCmd is not handled; handle or clarify.
    WARN(true, _T("TButtonGadgetEx::Activate: Switch doesn't handle Type: ") << Type);
    break;
  }

  // Unpress the button
  //
  CancelPressed(pt);

  // Send the associated command for all enabled buttons except for exclusive
  // buttons that are being poped up
  //
  if (!((Type == Exclusive || Type == SemiExclusive) && State != Down) &&
    GetEnabled())
    Window->PostMessage(WM_COMMAND, GetId());
}

//
// Syncronize the newState of a Exlusive, SemiExclusive and NonExclusive
// button gadget with id
//
void
TButtonGadgetEx::SyncronizeButtons(int id, TState newState)
{
  if (Window) {

    // Syncronize the descriptors for next "Customize"
    //
    TButtonGadgetDesc* desc = TYPESAFE_DOWNCAST(
      GadgetDescriptors->Find(id), TButtonGadgetDesc);
    if (desc && desc->ButtonGadgetType != Command)
      desc->ButtonGadgetState = newState;

    // Syncronize the real gadgets in all dockable toolbars
    //
    TDockableGadgetWindow* dockWin = TYPESAFE_DOWNCAST(Window,
      TDockableGadgetWindow);
    THarborEx* harbor = TYPESAFE_DOWNCAST(dockWin->GetHarbor(), THarborEx);
    if (harbor) {
      for (uint i=0; i<harbor->DCBData.GetItemsInContainer(); i++) {
        TDockableControlBarEx* dcb = harbor->DCBData[i]->DCB;
        if (dcb) {
          for (TGadget* g=dcb->FirstGadget(); g; g=dcb->NextGadget(*g)) {
            TButtonGadgetEx* bg = TYPESAFE_DOWNCAST(g, TButtonGadgetEx);
            if (bg && bg->GetId() == id) {
              bg->State = newState;
              bg->Invalidate(); // Call my not virtual Invalidate() function
              bg->Update();
            }
          }
        }
      }
    }
  }
}

//
// Similar to private OWL function CheckExclusively().
// Fix OWL bug with SemiExclusive button gadgets
// Syncronize the state of all buttons
//
void
TButtonGadgetEx::BetterCheckExclusively()
{
  PRECONDITION(Window);

  if (Window && (Type == Exclusive || Type == SemiExclusive)) {

    if (Type == SemiExclusive)
      State = (State == Up) ? Down : Up;

    TButtonGadgetGroup* group = GadgetDescriptors->FindGroup(GetId());
    if (group) {

      for (uint i=0; i<group->Array.GetItemsInContainer(); i++) {

        int id = group->Array[i];
        if (id == GetId()) {

          // change myself
          //
          if (Type == Exclusive)
            SyncronizeButtons(id, Down);
          else
            SyncronizeButtons(id, State);
        }
        else {

          // change the buttons in the group
          //
          if (!(Type == SemiExclusive && State == Up))
            SyncronizeButtons(id, Up);
        }
      }
    }
  }
}

// ******************** TButtonTextGadgetEx ***********************************

TButtonTextGadgetEx::TButtonTextGadgetEx(
  LPCTSTR      commandText,
  TDisplayType disptype,
  TResId       glyphResIdOrIndex, // see TButtonGadgetEx
  int          id,                // see TButtonGadgetEx
  TType        type,              // see TButtonGadgetEx
  bool         enabled,           // see TButtonGadgetEx
  TState       state,             // see TButtonGadgetEx
  bool         sharedGlyph):      // see TButtonGadgetEx
TButtonGadgetEx(glyphResIdOrIndex, id, type, enabled, state, sharedGlyph)
{
  if (commandText)
    CommandText = commandText;
  else
    CommandText = _T("");

  DispType = disptype;

  // Be sure that the DisplayType is text if there is no bitmap
  //
  if (ResId == 0)
    DispType = Text;
}

TButtonTextGadgetEx::~TButtonTextGadgetEx()
{
}

void TButtonTextGadgetEx::SetDisplayType(TDisplayType dispType)
{
  if (dispType != DispType) {

    // Be sure that the DisplayType is text if there is no bitmap
    //
    if (dispType != Text && ResId == 0)
      return;

    DispType = dispType;
    if (Window) {
      Window->Invalidate();
      Window->GadgetChangedSize(*this);
    }
  }
}

void TButtonTextGadgetEx::SetCommandText(const owl::tstring& commandText)
{
  CommandText = commandText;
  if (Window)
    Window->GadgetChangedSize(*this);
}

void TButtonTextGadgetEx::GetDesiredSize(TSize& size)
{
  TButtonGadgetEx::GetDesiredSize(size);

  if (DispType != Bitmap) {
    if (Window && Window->GetHandle()) {

      TRect textRect; // default constructor now set all members to 0
      TClientDC dc(Window->GetHandle());
      const TFont* font = &(Window->GetFont());
      if (font)
        dc.SelectObject(*font);
      dc.DrawText(CommandText.c_str(), -1, textRect, DT_CALCRECT | DT_SINGLELINE);
      if (font)
        dc.RestoreFont();

      TBorders borders = GetBorders();
      textRect.right+=4;
      textRect.bottom+=2;

      if (DispType == Text)
        size = textRect.Size()+TSize(borders.Left+borders.Right,
        borders.Top+borders.Bottom);
      else {
        if (textRect.bottom > size.cy)
          size.cy = textRect.bottom;
        size.cx += textRect.Width();
      }
    }
  }
}

void TButtonTextGadgetEx::SetBounds(const TRect& rect)
{
  TButtonGadgetEx::SetBounds(rect);
  BitmapOrigin.x = Borders.Left; // set the bitmap origin always to left
}

//
// Reset the members to default values.
//
void TButtonTextGadgetEx::Reset()
{
  TButtonGadgetEx::Reset();

  TButtonTextGadgetDesc* bgd = TYPESAFE_DOWNCAST(
    GadgetDescriptors->Find(GetId()), TButtonTextGadgetDesc);
  if (bgd) {
    SetCommandText(bgd->Text);
    SetDisplayType(bgd->DispType);
  }
}

//
// Returns true, if the members have the default values
//
bool TButtonTextGadgetEx::HasDefaultValues()
{
  if (TButtonGadgetEx::HasDefaultValues()) {
    TButtonTextGadgetDesc* bgd = TYPESAFE_DOWNCAST(
      GadgetDescriptors->Find(GetId()), TButtonTextGadgetDesc);
    if (bgd && bgd->Text == CommandText && bgd->DispType == DispType)
      return true;
  }
  return false;
}

void TButtonTextGadgetEx::Created()
{
  TButtonGadgetEx::Created();

  if (_tcslen(CommandText.c_str()) == 0)
    TryToLoadCommandText();
}

//
// Perform all of the painting for this text button gadget
//
void TButtonTextGadgetEx::Paint(TDC& dc)
{
  if (DispType == Bitmap) {
    TButtonGadgetEx::Paint(dc);
  }
  else if (DispType == Text) {
    TButtonGadgetEx::CheckStyleAndState();
    TButtonGadgetEx::PaintBorder(dc);
    PaintText(dc);
  }
  else {
    TButtonGadgetEx::Paint(dc);
    PaintText(dc);
  }
}

//
// Perform all of the painting for this button gadget
//
void TButtonTextGadgetEx::PaintText(TDC& dc)
{
  if (Window && DispType != Bitmap) {

    const TFont* font = &(Window->GetFont());
    if (font)
      dc.SelectObject(*font);

    TRect  innerRect;
    GetInnerRect(innerRect);
    innerRect.left+=1;

    if (DispType == BitmapText) {
      TCelArray& celArray = CelArray ? *CelArray : Window->GetCelArray();
      innerRect.left+=celArray.CelSize().cx;
    }

    if (DispType == Text) {
      if (State == Down)
        FillMaskRect(dc, innerRect);
      else
        dc.FillRect(innerRect, TBrush(TColor::Sys3dFace));
    }

    uint offset = (Pressed || State == Down) ? 1 : 0;
    innerRect.Offset(offset, offset);
    TColor oldtextcol;
    int oldbkmode = dc.SetBkMode(TRANSPARENT);
    if (GetEnabled() == false) {
      innerRect.Offset(1,1);
      oldtextcol = dc.SetTextColor(TColor::Sys3dHilight);
      dc.DrawText(CommandText.c_str(), -1, innerRect, DT_SINGLELINE);
      innerRect.Offset(-1,-1);
      dc.SetTextColor(oldtextcol);
    }
    oldtextcol = dc.SetTextColor(GetEnabled() ?
      TColor::SysBtnText : TColor::SysGrayText);
    dc.DrawText(CommandText.c_str(), -1, innerRect, DT_SINGLELINE);
    dc.SetTextColor(oldtextcol);
    dc.SetBkMode(oldbkmode);
    if (font)
      dc.RestoreFont();
  }
}

void TButtonTextGadgetEx::TryToLoadCommandText()
{
  TCHAR str[255];
  if (Window && Window->GetModule()) {
    if (Window->GetModule()->LoadString(GetId(), str, sizeof(str)) > 0) {
      TCHAR* ptr = _tcschr(str, _T('\n'));
      if (ptr && ptr+1) {
        TCHAR* ptr2 = _tcschr(str, _T('\t'));
        if (ptr2 == 0)
          ptr2 = _tcschr(str, _T('\a'));
        if (ptr2)
          *ptr2 = 0;
        CommandText = owl::tstring(ptr+1);

        // If we found the command text, then udpade the global descripton
        //
        if (_tcslen(CommandText.c_str()) > 0) {
          TGadgetDesc* desc = GadgetDescriptors->Find(GetId());
          if (desc) {
            TButtonTextGadgetDesc* btgd = TYPESAFE_DOWNCAST(desc,
              TButtonTextGadgetDesc);
            if (btgd) {
              btgd->Text = CommandText;
            }
          }
        }
      }
    }
  }
}

TDib* TButtonTextGadgetEx::GetGlyphDib()
{
  PRECONDITION(Window);
  if (ResId) {
    TDib* glyph;
    try {
      glyph = new TDib(*Window->GetModule(), ResId);
    }
    catch (TXGdi& ) {

      // Be sure that the DisplayType is text if there is no bitmap
      //
      ResId = 0;
      DispType = Text;
      return 0;
    }
    glyph->MapUIColors( TDib::MapFace | TDib::MapText | TDib::MapShadow |
      TDib::MapHighlight );
    return glyph;
  }
  return 0;
}

// ******************** TPopupButtonGadgetEx **********************************

const int ArrowWidth = 8;

TPopupButtonGadgetEx::TPopupButtonGadgetEx(
  TPopupType   popupType,
  LPCTSTR      commandText,       // see TButtonTextGadget
  TDisplayType disptype,          // see TButtonTextGadget
  TResId       glyphResIdOrIndex, // see TButtonGadgetEx
  int          id,                // see TButtonGadgetEx
  TType        type,              // see TButtonGadgetEx
  bool         enabled,           // see TButtonGadgetEx
  TState       state,             // see TButtonGadgetEx
  bool         sharedGlyph):      // see TButtonGadgetEx
TButtonTextGadgetEx(commandText, disptype, glyphResIdOrIndex, id, type, enabled,
          state, sharedGlyph)
{
  PopupType = popupType;
  DownArrowPressed = false;
  ButtonHasCapture = false;
}

void TPopupButtonGadgetEx::GetDesiredSize(TSize& size)
{
  TButtonTextGadgetEx::GetDesiredSize(size);
  if (PopupType != Normal)
    size.cx += ArrowWidth;
}

// Start the PopupAction if the user click
// 1) into the button if PopupType != DownArrowAction
// 2) into the arrow  if PopupType == DownArrowAction
// This function do nothing, overwrite them with functionallity
//
void TPopupButtonGadgetEx::PopupActionStart()
{
}

// This function must be called to show the button
// include the down arrow in normal state
//
void TPopupButtonGadgetEx::PopupActionEnd()
{
  if (DownArrowPressed) {
    DownArrowPressed = false;
    Invalidate();
    Update();
  }
  else {
    TPoint p;
    CancelPressed(p);
  }
}

// Invoked by mouse-down & mouse enter events. sets member data "Pressed"
// to true and highlights the button
//
void TPopupButtonGadgetEx::BeginPressed(const TPoint& p)
{
  PRECONDITION (Window);
  if (PopupType != DownArrowAction) {
    TButtonTextGadgetEx::BeginPressed(p);
    Window->GadgetReleaseCapture(*this);
    PopupActionStart();
  }
  else {
    TRect DownArrowRect = Bounds;
    DownArrowRect.Offset(-DownArrowRect.left, -DownArrowRect.top);
    DownArrowRect.left = DownArrowRect.right - ArrowWidth;

    if (ButtonHasCapture == false && DownArrowPressed == false &&
      DownArrowRect.Contains(p)) {
        Pressed = false;
        DownArrowPressed = true;
        Invalidate();
        Update();
        Window->GadgetReleaseCapture(*this);
        PopupActionStart();
    }
    else {
      TButtonTextGadgetEx::BeginPressed(p);
    }
  }
}

// Invoked by mouse exit events. sets member data "Pressed" to false and
// paints the button in its current state
//
void TPopupButtonGadgetEx::CancelPressed(const TPoint& p, bool)
{
  if (Pressed)
    TButtonTextGadgetEx::CancelPressed(p);
}

// Captures the mouse if "TrackMouse" is set.
//
void TPopupButtonGadgetEx::LButtonDown(uint modKeys, const TPoint& point)
{
  TButtonTextGadgetEx::LButtonDown(modKeys, point);
  if (Pressed)
    ButtonHasCapture = true;
}

// Releases the mouse capture if "TrackMouse" is set.
//
void TPopupButtonGadgetEx::LButtonUp(uint modKeys, const TPoint& point)
{
  TButtonTextGadgetEx::LButtonUp(modKeys, point);
  ButtonHasCapture = false;
}

// 1. PaintButton
// 2. PaintSeparator
// 3. PaintDownArrow
//
void TPopupButtonGadgetEx::Paint(TDC& dc)
{
  PaintButton(dc);
  if (PopupType == DownArrowAction)
    PaintSeparator(dc);
  if (PopupType != Normal)
    PaintDownArrow(dc);
}

// Paint the left button area
//
void TPopupButtonGadgetEx::PaintButton(TDC& dc)
{
  PRECONDITION (Window);
  TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(Window, TDockableControlBarEx);
  if (dcb) {
    if (Pressed && DownArrowPressed == false) {
      if (PopupType == DownArrowAction)
        Bounds.right -= (ArrowWidth+ (dcb->GetFlatStyle() ? 0 : 1));
      TButtonTextGadgetEx::Paint(dc);
      if (PopupType == DownArrowAction)
        Bounds.right += (ArrowWidth+ (dcb->GetFlatStyle() ? 0 : 1));
    }
    else
      TButtonTextGadgetEx::Paint(dc);
  }
}

// Paint the separator between button and down arrow
//
void TPopupButtonGadgetEx::PaintSeparator(TDC& dc)
{
  PRECONDITION (Window);
  TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(Window, TDockableControlBarEx);
  if (dcb) {

    TRect  innerRect;
    GetInnerRect(innerRect);
    TPoint dstPt(innerRect.right - (dcb->GetFlatStyle() ? 5 : 6),
      innerRect.top + (innerRect.Height()-4)/2);

    if (!(Pressed || DownArrowPressed)) {
      if (GetEnabled()) {
        if (dcb->GetFlatStyle()) {
          if (MouseInGadget) {
            TPen pen(TColor::Sys3dShadow);
            dc.SelectObject(pen);
            dc.MoveTo(dstPt.x-2, innerRect.top-2);
            dc.LineTo(dstPt.x-2, innerRect.bottom+1);
            dc.RestorePen();
            TPen pen2(TColor::Sys3dHilight);
            dc.SelectObject(pen2);
            dc.MoveTo(dstPt.x-1, innerRect.top-1);
            dc.LineTo(dstPt.x-1, innerRect.bottom+1);
            dc.RestorePen();
          }
        }
        else {
          TPen pen(TColor::Sys3dShadow);
          dc.SelectObject(pen);
          dc.MoveTo(dstPt.x-2, innerRect.top-2);
          dc.LineTo(dstPt.x-2, innerRect.bottom+1);
          dc.RestorePen();
        }
      }
    }
  }
}

// Paint the down arrow
//
void TPopupButtonGadgetEx::PaintDownArrow(TDC& dc)
{
  PRECONDITION (Window);
  TDockableControlBarEx* dcb = TYPESAFE_DOWNCAST(Window, TDockableControlBarEx);
  if (dcb) {

    // Paint pressed border around down arrow
    //
    if (Pressed == false && DownArrowPressed) {
      TRect boundsRect = TRect(0,0,Bounds.Width(), Bounds.Height());
      boundsRect.left = boundsRect.right - (ArrowWidth+2);
      if (dcb->GetFlatStyle()){
        TUIBorder::DrawEdge(dc, boundsRect, TUIBorder::SunkenOuter,
          TUIBorder::Rect);
      }
      else {
        TUIBorder(boundsRect, (TUIBorder::TStyle)ButtonDn).Paint(dc);
      }
    }

    TRect innerRect;
    GetInnerRect(innerRect);
    TPoint dstPt(innerRect.right - (dcb->GetFlatStyle() ? 5 : 6),
      innerRect.top + (innerRect.Height()-4)/2);

    // Paint down arrow
    //
    TColor col = TColor::SysBtnText;
    if (!GetEnabled())
      col = TColor::Sys3dShadow;
    TPen pen3(col);
    dc.SelectObject(pen3);
    dc.MoveTo(dstPt.x, dstPt.y);
    dc.LineTo(dstPt.x+5, dstPt.y);
    dc.MoveTo(dstPt.x+1, dstPt.y+1);
    dc.LineTo(dstPt.x+4, dstPt.y+1);
    dc.SetPixel(dstPt.x+2, dstPt.y+2, col);
    if (!GetEnabled()) {
      dc.SetPixel(dstPt.x+2, dstPt.y+3, TColor::Sys3dHilight);
      dc.SetPixel(dstPt.x+3, dstPt.y+2, TColor::Sys3dHilight);
      dc.SetPixel(dstPt.x+4, dstPt.y+1, TColor::Sys3dHilight);
    }
    dc.RestorePen();
  }
}

// ******************** TMenuButtonGadgetEx ***********************************

UINT TMenuButtonGadgetEx::Flags =
TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON;

TMenuButtonGadgetEx::TMenuButtonGadgetEx(
  TPopupType   popupType,         // see TPopupButtonGadget
  HMENU        hmenu,             // adress that points to a menu
  TWindow*     cmdTarget,         // commando target window
  LPCTSTR      commandText,       // see TButtonTextGadget
  TDisplayType disptype,          // see TButtonTextGadget
  TResId       glyphResIdOrIndex, // see TButtonGadgetEx
  int          id,                // see TButtonGadgetEx
  TType        type,              // see TButtonGadgetEx
  bool         enabled,           // see TButtonGadgetEx
  TState       state,             // see TButtonGadgetEx
  bool         sharedGlyph):      // see TButtonGadgetEx
TPopupButtonGadgetEx(popupType, commandText, disptype, glyphResIdOrIndex, id,
           type, enabled, state, sharedGlyph),
           CmdTarget(cmdTarget)
{
  hMenu = hmenu;
}

void TMenuButtonGadgetEx::PopupActionStart()
{
  if (Window && Window->GetHandle() && CmdTarget && CmdTarget->GetHandle() &&
    hMenu && IsMenu(hMenu)) {

      TPoint tlp = Bounds.TopLeft();
      TPoint brp = Bounds.BottomRight();
      TRect winRect = Window->GetWindowRect();
      tlp.Offset(winRect.left, winRect.top);

      brp.Offset(winRect.left, winRect.top);
      TPMPARAMS tpmParams;
      tpmParams.cbSize = sizeof(TPMPARAMS);
      tpmParams.rcExclude.top = tlp.y;
      tpmParams.rcExclude.left = 0;
      tpmParams.rcExclude.bottom = brp.y;
      tpmParams.rcExclude.right = 32000;
      ::TrackPopupMenuEx(hMenu, Flags,
        tlp.x, tlp.y, CmdTarget->GetHandle(), &tpmParams);

      PopupActionEnd();
  }
}

// ******************** TRecentFileButtonGadgetEx *****************************

TRecentFilesGadgetEx::TRecentFilesGadgetEx(
  LPCTSTR      commandText,       // see TButtonTextGadget
  TDisplayType disptype,          // see TButtonTextGadget
  TResId       glyphResIdOrIndex, // see TButtonGadgetEx
  int          id,                // see TButtonGadgetEx
  TType        type,              // see TButtonGadgetEx
  bool         enabled,           // see TButtonGadgetEx
  TState       state,             // see TButtonGadgetEx
  bool         sharedGlyph):      // see TButtonGadgetEx
TMenuButtonGadgetEx(DownArrowAction, 0, 0, commandText, disptype,
          glyphResIdOrIndex, id, type, enabled, state, sharedGlyph)
{
}

void TRecentFilesGadgetEx::PopupActionStart()
{
  if (Window && Window->GetHandle()) {
    TApplication* app = Window->GetApplication();
    if (app) {
      CmdTarget = app->GetMainWindow();
      TRecentFiles* rf = TYPESAFE_DOWNCAST(app, TRecentFiles);
      if (rf) {
        hMenu = ::CreatePopupMenu();
        int count=0;
        int id = CM_MRU_FIRST;
        TCHAR menustr[_MAX_PATH+3];
        TCHAR text[_MAX_PATH];
        while (rf->GetMenuText(id, text, _MAX_PATH)) {
          wsprintf(menustr, _T("&%d %s"), (count == 9) ? 0 : count+1, text);
          ::InsertMenu(hMenu, count, MF_BYPOSITION | MF_STRING, id, menustr);
          id++;
          count++;
        }
        if (count > 0)
          TMenuButtonGadgetEx::PopupActionStart();
        else
          PopupActionEnd();
        ::DestroyMenu(hMenu);
      }
    }
  }
}

// ******************** TColorButtonGadgetEx **********************************

TRect TColorButtonGadgetEx::DefaultFillRect = TRect(3,15,19,19);

TColorButtonGadgetEx::TColorButtonGadgetEx(
  TPopupType   popupType,         // see TPopupButtonGadget
  const TColorPickerData& data,   // see TColorPicker
  TColor       startColorSel,     // start color that is selected
  TRect*       fillRect,          // rect that will be filled with color
  TWindow*     parentWindow,      // parent window of picker
  LPCTSTR      commandText,       // see TButtonTextGadget
  TDisplayType disptype,          // see TButtonTextGadget
  TResId       glyphResIdOrIndex, // see TButtonGadgetEx
  int          id,                // see TButtonGadgetEx
  TType        type,              // see TButtonGadgetEx
  bool         enabled,           // see TButtonGadgetEx
  TState       state,             // see TButtonGadgetEx
  bool         sharedGlyph):      // see TButtonGadgetEx
TPopupButtonGadgetEx(popupType, commandText, disptype, glyphResIdOrIndex, id,
           type, enabled, state, sharedGlyph),
           ParentWindow(parentWindow),
           Data (data)
{
  CheckingMode = false;
  ColorSel = startColorSel;
  FillRect = fillRect;
  ColorPickerWindow = new TColorPicker(ParentWindow, Data, ColorSel, id);
}

void TColorButtonGadgetEx::Created()
{
  TPopupButtonGadgetEx::Created();
  ColorPickerWindow->Create();
}

void TColorButtonGadgetEx::PopupActionStart()
{
  if (ColorPickerWindow) {
    TRect rect = GetBounds();
    TPoint p2(rect.TopLeft());
    Window->ClientToScreen(p2);
    rect.right = p2.x + rect.Width();
    rect.bottom = p2.y + rect.Height();
    rect.left = p2.x;
    rect.top = p2.y;
    p2.y = rect.bottom;

    // We tell the picker where the picker has to be displayed.
    ColorPickerWindow->ShowPickerWindow(p2, rect);

    CheckingMode = true;
  }
}

// We check some events for this flat button gadget.
//
bool TColorButtonGadgetEx::IdleAction(long idleCount)
{
  if (idleCount == 0 && ColorPickerWindow && CheckingMode) {

    // Check if the current chosen color has changed. If it has changed,
    // we update this flat button gadget and we tell the picker to notify
    // the parent window.
    //
    TColor color = ColorPickerWindow->GetCurrentChosenColor();
    if (color != ColorSel) {
      ColorSel = color;
      ColorPickerWindow->NotifyAtParent();
      CheckingMode = false;
      PopupActionEnd();
    }

    if (ColorPickerWindow->GetHandle() &&
      ColorPickerWindow->IsWindowVisible() == false) {
        CheckingMode = false;
        PopupActionEnd();
    }
  }
  return TPopupButtonGadgetEx::IdleAction(idleCount);
}

void TColorButtonGadgetEx::PaintButton(TDC& dc)
{
  TPopupButtonGadgetEx::PaintButton(dc);
  if (FillRect && GetEnabled()) {

    // Copy only the RGB values for the color that will be draw
    //
    TColor col(ColorSel.Red(), ColorSel.Green(), ColorSel.Blue());

    TRect rect = *FillRect;
    if (col.GetValue() == TColor::Sys3dFace.GetValue())
      rect.Inflate(-1,-1);
    if (Pressed)
      dc.FillRect(rect.OffsetBy(1,1), TBrush(col));
    else
      dc.FillRect(rect, TBrush(col));
  }
}

void TColorButtonGadgetEx::Activate(const TPoint& pt)
{
  TButtonGadget::Activate(pt);

  // Send the registed color messege
  //
  if (!((Type == Exclusive || Type == SemiExclusive) && State != Down) &&
    GetEnabled() && ColorPickerWindow && ColorPickerWindow->GetHandle())
    ColorPickerWindow->NotifyAtParent();
}

// ******************** TControlGadgetEx **************************************

//
//
//
TControlGadgetEx::TControlGadgetEx(TWindow& control, TBorderStyle border):
//  TGadget(control.Attr.Id, border)
TControlGadget(control, border)
{
  IsCursorInControl = false;
  //  Control = &control;
  //  Control->ModifyStyle(0, WS_CLIPSIBLINGS);  // Make sure relayout paints OK
  //  TRACEX(GadgetEx, 0, "TControlGadgetEx constructed @" << (void*)this);
}

TControlGadgetEx::~TControlGadgetEx()
{
  // Don't delete the control here, because if the control is inside a window,
  // the OWL framework delete the control and if not the control is deleted
  // by TControlGadgetDesc
  //
  SetControl(0);

  // Control->Destroy(0);
  // delete Control;
  // TRACEX(GadgetEx, 0, "TControlGadgetEx destructed @" << (void*)this);
}


//
// Virtual called after the window holding a gadget has been created
//
void
TControlGadgetEx::Created()
{
  TControlGadget::Created();

  // Try to load tooltip. This code is stolen from Jo Parrello's
  // JPControlGadget class. Very special thanks
  //
  if (Control->GetHandle()) {
    owl::tstring text = GetHintText(Control, GetId());
    if (text.length()) {
      TTooltip* tooltip = Window->GetTooltip();
      if (tooltip && tooltip->GetHandle()) {
        TToolInfo toolInfo(Window->GetHandle(), Control->GetHandle());
        if (tooltip->GetToolInfo(toolInfo)) {
          toolInfo.SetText(text.c_str(), false);
          tooltip->SetToolInfo(toolInfo);
        }
      }
    }
  }
}

//
// Override the Inserted() virtual to take the oportunity to make sure that the
// control window has been created and shown
//
void
TControlGadgetEx::Inserted()
{
  //  TControlGadget::Inserted();

  TRACEX(GadgetEx, 1, "TControlGadgetEx::Inserted @" << (void*)this);
  Control->SetParent(Window);

  if (Window->GetHandle()) {
    if (Control->GetHandle() == 0)
      Control->Create();

    // Show the control
    //
    if (Control->IsWindowVisible() == false)
      Control->ShowWindow(SW_SHOWNA);
  }

  // Say the TControlGadgetDesc, that the control is inserted in a window
  //
  TGadgetDesc* desc = GadgetDescriptors->Find(Control->GetId());
  TControlGadgetDesc* cgdesc = TYPESAFE_DOWNCAST(desc, TControlGadgetDesc);
  if (cgdesc) {
    cgdesc->CountAdd();
  }
}

//
// Override the Remove() virtual to take the oportunity to unparent the
// control window from the owning Window
//
void
TControlGadgetEx::Removed()
{
  //  TControlGadget::Removed();

  TRACEX(GadgetEx, 1, "TControlGadgetEx::Removed @" << (void*)this);

  // Hide the control
  //
  if (Control->IsWindowVisible() == true)
    Control->ShowWindow(SW_HIDE);

  Control->SetParent(0);
  // Should we destroy the control at this point??
  // Since it's no longer in the parent's child-list, there's a potential
  // leak. However, the semantics of this function is 'Removed' - therefore
  // one could be removing the control to be reinserted in another
  // gadgetwindow.

  // Unregister ourself with the tooltip window (if there's one)
  //
  if (Window && Window->GetHandle()) {
    TTooltip* tooltip = Window->GetTooltip();
    if (tooltip) {
      CHECK(tooltip->GetHandle());

      TToolInfo toolInfo(Window->GetHandle(), Control->GetHandle());
      tooltip->DeleteTool(toolInfo);
    }
  }

  // Say the TControlGadgetDesc, that the control is removed from a window
  //
  TGadgetDesc* desc = GadgetDescriptors->Find(Control->GetId());
  TControlGadgetDesc* cgdesc = TYPESAFE_DOWNCAST(desc, TControlGadgetDesc);
  if (cgdesc) {
    cgdesc->CountSub();
  }
}

void
TControlGadgetEx::Paint(TDC& /*dc*/)
{
  if (Control->GetHandle()) {
    Control->Invalidate();
    Control->UpdateWindow();
  }
}

//
// Display tooltip and a message on status bar when you "enter" a TControlGadget
// This code is stolen from Jo Parrello's JPControlGadget class.
// Very special thanks
//
bool
TControlGadgetEx::IdleAction(long idleCount)
{
  if (idleCount == 0)  {
    if (Window) {
      TPoint crsPoint;
      Window->GetCursorPos(crsPoint);
      HWND hwnd = Window->WindowFromPoint(crsPoint);
      if (hwnd) {
        if ((hwnd != Control->GetHandle()) &&
          (::GetParent(hwnd) != Control->GetHandle())) {
            if (Window->GetHintMode() == TGadgetWindow::EnterHints) {
              if (IsCursorInControl)
                Window->SetHintCommand(-1);
              IsCursorInControl = false;
            }
        }
        else {
          if (Window->GetHintMode() == TGadgetWindow::EnterHints) {
            if (!IsCursorInControl)
              Window->SetHintCommand(Control->GetId());
            IsCursorInControl = true;
          }
        }
      }
    }
  }
  return false;
}

} // OwlExt namespace
//=====================================================================================
