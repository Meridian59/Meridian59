// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// dockingex.cpp: implementation file
// Version:       1.4
// Date:          25.10.1998
// Author:        Dieter Windau
//
// Freeware OWL classes that extents the dockable and gadget system
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// Tested with Borland C++ 5.02, OWL 5.02 with Windows NT 4.0 SP3 but I think
// the class should work with Windows 95 too.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// This package contains many classes and source that are based on other OWL
// developers work. Very special thanks to Alan Chambers, Christopher Kohlhoff,
// Jo Parrello and Yura Bidus
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@kvwl.de
//   Web:   http://members.aol.com/softengage/index.htm
// ****************************************************************************

#include <owlext/pch.h>
#pragma hdrstop

#include <owl/uihelper.h>  // for TUIBorder edge painting
#include <owl/registry.h>
#include <owl/profile.h>
#include <owl/filename.h>

#include <owlext/harborex.h>
#include <owlext/dockingex.h>
#include <owlext/gadgetex.h>
#include <owlext/gadgctrl.h>
#include <owlext/util.h>
#include <owlext/dockingex.rh>

#ifdef  CTXHELP
#include <owlext/ctxhelpm.h>
#endif

using namespace owl;
using namespace std;

namespace OwlExt {

// ******************************* constants ***********************************

const int GripperDistSize = 3; // Distance from gripper to gripper and size of gripper

const int FloatMarginsX   = 4; // Left and Right Margins, FloatingSlip
const int FloatMarginsY   = 3; // Top and Bottom Margins, FloatingSlip
const int EdgeMaginsX     = 5; // Left and Right Margins, EdgeSlip
const int EdgeMaginsY     = 5; // Top and Bottom Margins, EdgeSlip

// ****************** TDockableControlBarEx ***********************************

DEFINE_RESPONSE_TABLE1(TDockableControlBarEx, TDockableControlBar)
#if (OWLInternalVersion >= 0x06000000L)
EV_OWLWINDOWDOCKED,
#endif
EV_WM_LBUTTONDOWN,
EV_WM_LBUTTONDBLCLK,
EV_WM_RBUTTONDOWN,
EV_COMMAND(IDCANCEL, CmCancel),
EV_COMMAND(IDOK, CmOk),
END_RESPONSE_TABLE;

#if (OWLInternalVersion < 0x06000000L)
bool TDockableControlBarEx::FlatStyle = false;
#endif

TDockableControlBarEx::TDockableControlBarEx(TWindow* parent,
                       TTileDirection direction, TFont* font, TModule* module):
TDockableControlBar(parent, direction, font, module),
DefaultGadgets(5,0,5)
// default constructor
{
  // SetMargins(TMargins(TMargins::Pixels,0,0,0,0));
  // Margins are setting in TEdgeSlipEx::DockableInsert()
  // and TFloatingSlipEx::DockableInsert()
  Attr.Id = 0;
  Default = true;
  HasHelp = false;

  // no default position and location
  Rect = ToggleRect = TRect(-1,-1,-1,-1);
  Location = ToggleLocation = alTop;
}

TDockableControlBarEx::TDockableControlBarEx(uint id, LPCTSTR title,
                       TWindow* parent, bool _Default, bool hasHelp,
                       TTileDirection direction, TFont* font, TModule* module):
TDockableControlBar(parent, direction, font, module),
DefaultGadgets(5,0,5)
// constructor that set additional the Attr.id for context help and OLE 2,
// title for the caption, and the flag for a default controlbar
{
  Attr.Id = id;
  SetCaption(title ? title : _T(""));
  Default = _Default;
  HasHelp = hasHelp;

  // no default position and location
  Rect = ToggleRect = TRect(-1,-1,-1,-1);
  Location = ToggleLocation = alTop;
}

TDockableControlBarEx::~TDockableControlBarEx()
{
  Destroy(IDCANCEL);
}

void TDockableControlBarEx::SetDefaultGadgets(const TIntArray& defaultGadgets)
// Set the dafault gadgets with the id. All id's should be added in
// GadgetsDescriptors. Best place to set the default gadgets is after
// the toolbar is inserted in harbor
{
  DefaultGadgets.Flush();
  for (uint i=0; i<defaultGadgets.GetItemsInContainer(); i++)
    DefaultGadgets.Add(defaultGadgets[i]);
  InsertDefaultGadgets();
}

void TDockableControlBarEx::SetDefaultGadgets(const int iArray[], uint numGadgets)
// Set the dafault gadgets with the id. All id's should be added in
// GadgetsDescriptors. Best place to set the default gadgets is after
// the toolbar is inserted in harbor
{
  PRECONDITION(iArray);
  DefaultGadgets.Flush();
  for (uint i=0; i<numGadgets; i++)
    DefaultGadgets.Add(iArray[i]);
  InsertDefaultGadgets();
}

void TDockableControlBarEx::InsertDefaultGadgets()
// Remove all existing gadgets and insert the default gadgets in the tool bar
// If layoutSession=true the toolbar will be updated
{
  PRECONDITION(GadgetDescriptors);
  THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
  if (harbor) {
    RemoveAllGadgets();
    if (DefaultGadgets.GetItemsInContainer() > 0) {
      for (uint i=0; i<DefaultGadgets.GetItemsInContainer(); i++) {
        int id = DefaultGadgets[i];
        harbor->CheckBeforeInsertDefaultGadget(id);
        TGadget* gadget = GadgetDescriptors->ConstructGadget(id);
        if (gadget)
          Insert(*gadget);
      }
      LayoutSession();
    }
    CheckOnLastGadget();
  }
}

#if (OWLInternalVersion >= 0x06000000L)
bool TDockableControlBarEx::Create()
{
  bool retValue = TDockableControlBar::Create();

  // Don't create FlatHandleGadget because. Gripper is drawn by this class
  //
  TGadget* gadget = FirstGadget();
  if (gadget && gadget->GetId() == IDG_FLATHANDLE) {
    delete Remove(*gadget);
    delete Cursor;
    Cursor = 0;
  }

  return retValue;
}
#endif

void TDockableControlBarEx::Hide()
{
  THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
  if (harbor) {
    harbor->Remove(*this);
    harbor->UpdateShowHideCtlBar();
  }
}

void TDockableControlBarEx::Show()
{
  THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
  if (harbor) {
    harbor->Insert(*this, Location, &Rect.TopLeft());

    // If the inserted docking slip is floating one with location alNone
    // compute the real size. Thanks to Jo Parrello
    //
    TFloatingSlipEx* flslip = TYPESAFE_DOWNCAST(this, TFloatingSlipEx);
    TSize size(Rect.Width(), Rect.Height());
    if (flslip && (Location == alNone)) {
      TSize newsize = flslip->ComputeSize(alNone, &size);
      flslip->Layout(alNone, &newsize);
    }

    //D: This works also not correct
    //  harbor->Move(*this, Location, &Rect.TopLeft());
    harbor->UpdateShowHideCtlBar();
  }
}

void TDockableControlBarEx::PreRemoved()
// Called by TFloatingSlipEx or TEdgeSlipEx if the conrolbar will be removed
{
  THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
  if (harbor) {
    if (harbor->GetCustWindow() == this)
      harbor->SetCustGadgetNull();
  }
}

TResult TDockableControlBarEx::WindowProc(TMsgId msg, TParam1 p1, TParam2 p2)
// Processes incoming messages when toolbar is hide
{
  if (msg == WM_SHOWWINDOW && !p1) // we are being hidden
    StorePosAndLocation();
  return TDockableControlBar::WindowProc(msg, p1, p2);
}

void TDockableControlBarEx::PaintGadgets(TDC& dc, bool b, TRect& rect)
// Overridden to paint the actual customize gadget with a black rectangle
{
  TDockableControlBar::PaintGadgets(dc, b, rect);
  THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
  if (harbor && harbor->GetCustWindow() == this) {
    TGadget* gadget = FirstGadget();
    while (gadget) {
      if (gadget == harbor->GetCustGadget()) {

        TRect bounds = gadget->GetBounds();
        TControlGadgetEx* ctrlgad = TYPESAFE_DOWNCAST(gadget, TControlGadgetEx);
        if (ctrlgad) {
          TWindow* win = ctrlgad->GetControl();
          TGadgetComboBox* gcb = TYPESAFE_DOWNCAST(win, TGadgetComboBox);
          if (gcb) {
            gcb->Invalidate();
            gcb->UpdateWindow();
          }
          else {
            TRect winRect;
            GetRect(winRect);
            bounds.Offset(winRect.left+1, winRect.top+1);
            bounds.right--;
            bounds.bottom--;

            // If the actual customize gadget is a control gadget, then draw
            // directly into the screen.
            //
            TScreenDC sdc;
            DrawCustomizeRect(sdc, bounds);
            return;
          }
        }
        else {
          bounds.left++;
          bounds.top++;
          DrawCustomizeRect(dc, bounds);
          return;
        }
      }
      gadget = gadget->NextGadget();
    }
  }
}

void TDockableControlBarEx::Paint(TDC& dc, bool erase, TRect& rect)
// Overridden to paint the etched toolbar border and the gripper
{
  TDockableControlBar::Paint(dc, erase, rect);

  TWindowDC MyDC(*this);
  TDockingSlip* Slip = TYPESAFE_DOWNCAST(Parent, TDockingSlip);

  // If toolbar is floating, we draw the horizontal divider lines and
  // divider gadgets. Very special thanks to Jo Parrello
  //
  if (Slip && Slip->GetLocation() == alNone) {
    TFloatingSlipEx* flslip = TYPESAFE_DOWNCAST(Slip, TFloatingSlipEx);
    if (flslip && GetFlatStyle() != NonFlatNormal) {
      TRect sliprect;
      TPoint startpoint;
      flslip->GetRect(sliprect);

      // Be sure that the non-client left area will be erased.
      //
      TBrush brush(TColor::Sys3dFace);
      TRect crect = flslip->GetClientRect();
      crect.left = 0;
      crect.right = Margins.Left;
      MyDC.FillRect(crect, brush);
      int maxwidth = sliprect.Width() - Margins.Right - Margins.Left;
      int numrows, numcol, maxy, prevrowheight;
      numrows = numcol = maxy = prevrowheight = 0;
      int miny = 20000;
      TGadget* firstgadg = 0;
      for (TGadget* gad = FirstGadget(); gad; gad = gad->NextGadget()) {
        if (numcol == 0)
          firstgadg = gad;
        numcol++;
        TRect boundrect = gad->GetBounds();
        if (boundrect.top < miny)
          miny = boundrect.top;
        if (boundrect.bottom > maxy)
          maxy = boundrect.bottom;
        bool islastgadget = gad->IsEndOfRow();
        if (islastgadget || !gad->NextGadget()) {
          if ((numrows > 0) && (miny > prevrowheight)) {
            int middley = (miny - prevrowheight) / 2 + prevrowheight;
            TPen pen (TColor::Sys3dShadow);
            MyDC.SelectObject(pen);
            MyDC.MoveTo(0, middley - 1);
            MyDC.LineTo(maxwidth, middley - 1);
            MyDC.RestorePen();
            TPen pen2 (TColor::Sys3dHilight);
            MyDC.SelectObject(pen2);
            MyDC.MoveTo(0, middley);
            MyDC.LineTo(maxwidth, middley);
            MyDC.RestorePen();
          }
          DrawVerticalDividers(MyDC, firstgadg, gad, miny, maxy, false);
          numcol = 0;
          numrows++;
          prevrowheight = maxy;
          miny = 20000;
          maxy = 0;
        }
      }
    }
  }

  // When docked, this paints an etched border just
  // inside the client area of the control bar
  //
  if (Slip && Slip->GetLocation() != alNone) {
    TRect ThisClientRect = GetClientRect();

    /*D: I think this looks not perfect, because the algorithm of A. Chambers
    makes the rectangle 2 pixels on each side bigger. I try to find a solution
    if (GetFlatStyle()) {

    // Draw toolbar's edges in a flat mode like Office
    // Very special thanks to Jo Parrello
    //
    TRect crect = ThisClientRect;
    crect.Inflate(-2, -2);
    TUIBorder border(crect, TUIBorder::TEdge(TUIBorder::RaisedInner),
    TUIBorder::Rect);
    border.Paint(MyDC);
    }
    else {
    */
    TUIBorder(ThisClientRect, TUIBorder::EdgeEtched, TUIBorder::Left).Paint(MyDC);
    TUIBorder(ThisClientRect, TUIBorder::EdgeEtched, TUIBorder::Right).Paint(MyDC);
    TUIBorder(ThisClientRect, TUIBorder::EdgeEtched, TUIBorder::Bottom).Paint(MyDC);
    TUIBorder(ThisClientRect, TUIBorder::EdgeEtched, TUIBorder::Top).Paint(MyDC);
    //    }

    //D: I think I must draw the space beetween gadgets and border here.
    // Because in some situations there are some rest pixels in customize mode

    // Draw the gripper
    //
    THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
    if (harbor->IsDrawGripper()) {

      TRect gripperRect = ThisClientRect;
      if (Slip->GetLocation() == alTop || Slip->GetLocation() == alBottom) {
        gripperRect.Inflate(-GripperDistSize-1, -GripperDistSize);
        gripperRect.right = gripperRect.left + GripperDistSize;
        Draw3dRect(MyDC, gripperRect, TColor::Sys3dHilight, TColor::Sys3dShadow);
        gripperRect.Offset(GripperDistSize, 0);
        Draw3dRect(MyDC, gripperRect, TColor::Sys3dHilight, TColor::Sys3dShadow);
      }
      else {
        gripperRect.Inflate(-GripperDistSize, -GripperDistSize-1);
        gripperRect.bottom = gripperRect.top + GripperDistSize;
        Draw3dRect(MyDC, gripperRect, TColor::Sys3dHilight, TColor::Sys3dShadow);
        gripperRect.Offset(0, GripperDistSize);
        Draw3dRect(MyDC, gripperRect, TColor::Sys3dHilight, TColor::Sys3dShadow);
      }
    }

    // We draw the divider gadgets if toolbar is vertical or horizontal.
    // Very special thanks to Jo Parrello
    //
    if (GetFlatStyle()) {
      if (Slip->GetLocation() == alTop || Slip->GetLocation() == alBottom)
        DrawVerticalDividers(MyDC, FirstGadget(), 0, Margins.Top,
        ThisClientRect.bottom - Margins.Bottom);
      else if (Slip->GetLocation() == alLeft || Slip->GetLocation() == alRight)
        DrawHorizontalDividers(MyDC, FirstGadget(), 0, Margins.Left,
        ThisClientRect.right - Margins.Right);
    }
  }
}

void TDockableControlBarEx::ToggleSlip()
// toggles it from docked position to floating position and back again
{
  // Check that we're parented to the right window. In OLE server situation,
  // the toolbar could have been reparented to another HWND [i.e. container's
  // window]
  //
  TWindow* w = GetParentO();
  if (w && ::GetParent(*this) != w->GetHandle())
    return;

  StorePosAndLocation();
  bool ToggleSlips = false;
  TAbsLocation setLoc;
  if (TYPESAFE_DOWNCAST(w, TEdgeSlipEx) && Location != alNone) {
    // Change from EdgeSlip to FloatingSlip
    //
    setLoc         = alNone;
    ToggleLocation = Location;
    ToggleSlips    = true;
  }
  else {
    if (TYPESAFE_DOWNCAST(w, TFloatingSlipEx) &&
      Location == alNone && ToggleLocation != alNone) {
        // Change from FloatingSlip to EdgeSlip
        //
        setLoc         = ToggleLocation;
        ToggleLocation = alNone;
        ToggleSlips    = true;
    }
  }

  if (ToggleSlips) {
    // get and set the rect before toggle the slips
    //
    TRect setRect;
    bool noDefaultPos = false;
    if (ToggleRect == TRect(-1,-1,-1,-1))
      noDefaultPos = true;
    else
      setRect = ToggleRect;
    ToggleRect = Rect;

    THarbor* harbor = GetHarbor();
    if (harbor) {
      harbor->Remove(*this);
      TDockingSlip* dslip = harbor->Insert(*this,
        setLoc, noDefaultPos ? 0 : &setRect.TopLeft());

      // If the inserted docking slip is floating one with location alNone
      // compute the real size. Thanks to Jo Parrello
      //
      TFloatingSlipEx* flslip = TYPESAFE_DOWNCAST(dslip, TFloatingSlipEx);
      TSize size(setRect.Width(), setRect.Height());
      if (flslip && (setLoc == alNone)) {
        TSize newsize = flslip->ComputeSize(alNone, &size);
        flslip->Layout(alNone, &newsize);
      }
    }
  }
}

TGadget* TDockableControlBarEx::RemoveEx(TGadget* gadget, bool del)
{
  TGadget* gad = 0;
  THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
  CheckOnNeighbourSeparatorGadgets(gadget);
  if (harbor && harbor->GetCustGadget() == gadget)
    harbor->SetCustGadgetNull();
  if (del) {
    delete Remove(*gadget);
  }
  else
    gad = Remove(*gadget);
  LayoutSession();
  CheckOnLastGadget();
  Invalidate();
  UpdateWindow();
  return gad;
}

void TDockableControlBarEx::InsertEx(TGadget& gadget, TPlacement placement,
                   TGadget* sibling)
{
  Insert(gadget, placement, sibling);

  // If the sibling gadget is the invisible gadget remove and delete them
  //
  if (sibling && sibling->GetId() == CM_INVISIBLEGADGET) {
    delete Remove(*sibling);
  }
  LayoutSession();
}

void TDockableControlBarEx::SetupWindow()
{
  TDockableControlBar::SetupWindow();
  StorePosAndLocation();
}

void TDockableControlBarEx::RemoveAllGadgets()
// Remove all gadgets from toolbar
{
  THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
  TGadget* gadget = FirstGadget();
  while (gadget) {
    TGadget* delGadget = gadget;
    gadget = gadget->NextGadget();
    if (harbor && harbor->GetCustGadget() == delGadget)
      harbor->SetCustGadgetNull();
    delete Remove(*delGadget);
  }
}

TGadget* TDockableControlBarEx::PrevGadget(TGadget* gadget) const
// Returns previous gadget
{
  PRECONDITION(gadget);
  TGadget* prevGadget = FirstGadget();
  while (prevGadget) {
    if (prevGadget->NextGadget() == gadget)
      return prevGadget;
    prevGadget = prevGadget->NextGadget();
  }
  return 0;
}

TGadget* TDockableControlBarEx::GetAndSetCustomizeGadget(TPoint& point)
// Get the actual customize gadget and set them in harbor or returns 0
{
  THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
  if (harbor && harbor->IsCustomizeMode()) {
    TGadget* gad = Capture ? Capture : GadgetFromPoint(point);
    if (gad) {
      harbor->SetCustGadget(this, gad);
      return gad;
    }
  }
  return 0;
}

void TDockableControlBarEx::CheckOnLastGadget()
// If the controlbar has no visible gadgets a "invisible" gadget is inserted
{
  bool novisible = true;
  TGadget* gad = FirstGadget();
  while (gad) {
    if (gad->IsVisible()) {
      novisible = false;
      break;
    }
    gad = gad->NextGadget();
  }
  if (novisible) {
    TGadget* gadget = GadgetDescriptors->ConstructGadget(CM_INVISIBLEGADGET);
    CHECK(gadget);
    if (gadget)
      Insert(*gadget);
    LayoutSession();
  }
}

void TDockableControlBarEx::CheckOnNeighbourSeparatorGadgets(TGadget* actGadget)
// Check the neighbours gadgets of actGadget. If they are seperator gadgets
// that are not used they will be removed
{
  PRECONDITION(actGadget);

  TGadgetDesc* desc;
  bool isValid, nextGadgetIsSeparator, beforeGadgetIsSeparator;
  isValid = nextGadgetIsSeparator = beforeGadgetIsSeparator = false;
  TGadget* beforeGadget = 0;
  TGadget* nextGadget = actGadget->NextGadget();

  TGadget* gad = FirstGadget();
  while(gad) {
    if (gad == actGadget)
      isValid = true;
    if (gad->NextGadget() == actGadget)
      beforeGadget = gad;
    gad = gad->NextGadget();
  }

  if (isValid) {
    if (nextGadget) {
      desc = GadgetDescriptors->Find(nextGadget->GetId());
      if (desc && desc->Type == TGadgetDesc::SeparatorGadgetDesc)
        nextGadgetIsSeparator = true;
    }
    if (beforeGadget) {
      desc = GadgetDescriptors->Find(beforeGadget->GetId());
      if (desc && desc->Type == TGadgetDesc::SeparatorGadgetDesc)
        beforeGadgetIsSeparator = true;
    }
    if ((nextGadget == 0 || nextGadgetIsSeparator) &&
      beforeGadget && beforeGadgetIsSeparator) {
        delete Remove(*beforeGadget);
    }
    else if ((beforeGadget == 0 || beforeGadgetIsSeparator) &&
      nextGadget && nextGadgetIsSeparator) {
        delete Remove(*nextGadget);
    }
  }
}

TGadget* TDockableControlBarEx::GetDragDropGadget(const TPoint& pt,
                          TPoint& p1, TPoint& p2, TPlacement& placement)
{
  bool horizontal = true;
  TDockingSlip* Slip = TYPESAFE_DOWNCAST(Parent, TDockingSlip);
  if (Slip) {
    if (Slip->GetLocation() == alLeft || Slip->GetLocation() == alRight)
      horizontal = false;
  }

  // Similar to GadgetFromPoint, but this get non visible gadgets too
  //
  TGadget* gadget = FirstGadget();
  TGadget* gadbefore = 0;
  TRect    gadbounds;
  int      refbottom = 0;
  int      maxright = 0;
  while(gadget) {
    gadbounds = gadget->GetBounds();
    refbottom = std::max(refbottom, (int)gadbounds.bottom);
    maxright  = std::max(maxright, (int)gadbounds.right);
    if (pt.y <= refbottom) {
      if (gadget->IsEndOfRow() && pt.x <= maxright)
        break;
      if (pt.x <= gadbounds.right)
        break;
    }
    gadbefore = gadget;
    gadget = gadget->NextGadget();
  }

  if (gadget == 0) {
    if (gadbefore && gadbefore->GetId() != 0)
      gadget = gadbefore;
  }

  THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
  if (gadget && harbor) {
    int midx = gadbounds.left+(gadbounds.right-gadbounds.left)/2;
    int midy = gadbounds.top+(gadbounds.bottom-gadbounds.top)/2;
    if (gadget->GetId() == 0) {
      TRect bounds;
      if (horizontal) {
        if (pt.x > midx && gadget->IsEndOfRow() == false) {
          if (gadget->NextGadget()) {
            placement = After;
            bounds = gadget->NextGadget()->GetBounds();
            p1.x = p2.x = bounds.left;
            p1.y = bounds.top;
            p2.y = bounds.bottom;
            return gadget;
          }
        }
        else {
          if (gadbefore) {
            placement = Before;
            bounds = gadbefore->GetBounds();
            p1.x = p2.x = bounds.right;
            p1.y = bounds.top;
            p2.y = bounds.bottom;
            return gadget;
          }
        }
      }
      else {
        if (pt.y > midy) {
          if (gadget->NextGadget()) {
            placement = After;
            bounds = gadget->NextGadget()->GetBounds();
            p1.y = p2.y = bounds.top;
            p1.x = bounds.left;
            p2.x = bounds.right;
            return gadget;
          }
        }
        else {
          if (gadbefore) {
            placement = Before;
            bounds = gadbefore->GetBounds();
            p1.y = p2.y = bounds.bottom;
            p1.x = bounds.left;
            p2.x = bounds.right;
            return gadget;
          }
        }
      }
    }
    else {
      if (horizontal) {
        if (pt.x > midx) {
          placement = After;
          p1.x = p2.x = gadbounds.right;
        }
        else {
          placement = Before;
          p1.x = p2.x = gadbounds.left;
        }
        p1.y = gadbounds.top;
        p2.y = gadbounds.bottom;
      }
      else {
        if (pt.y > midy) {
          placement = After;
          p1.y = p2.y = gadbounds.bottom;
        }
        else {
          placement = Before;
          p1.y = p2.y = gadbounds.top;
        }
        p1.x = gadbounds.left;
        p2.x = gadbounds.right;
      }
      return gadget;
    }
  }
  return 0;
}

#if (OWLInternalVersion >= 0x06000000L)
void TDockableControlBarEx::EvOwlWindowDocked(uint /*pos*/, const TDockingSlip& /*slip*/)
{
  // OWL60: Don't create FlatHandleGadget because, gripper is drawn in this class
}
#endif

void TDockableControlBarEx::EvLButtonDown(uint modKeys, const TPoint& point)
{
  THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
#ifdef CTXHELP
  TCtxHelpFileManager* ctxHelpM = TYPESAFE_DOWNCAST(GetApplication(),
    TCtxHelpFileManager);
  if (ctxHelpM && ctxHelpM->IsContextHelp()) {
    TGadget* gadget = Capture ? Capture : GadgetFromPoint(point);
    TGadgetDesc* desc = 0;
    if (gadget)
      desc = GadgetDescriptors->Find(gadget->GetId());
    if (desc && desc->Attr & GADG_HASHELP) {

      // click in a gadget: call help with gadget id > 0
      //
      int HelpId = gadget->GetId();
      if (HelpId > 0)
        ctxHelpM->WinHelp(HelpId);

    }
    else {
      // click in a seperator or the gripper: call help with Attr.Id != 0
      //
      if (HasHelp)
        ctxHelpM->WinHelp(Attr.Id);
    }
  }
  else
#endif
  {
    TPoint p(point);
    if (GetAndSetCustomizeGadget(p)) {
      harbor->GadgetDraggingBegin();
      return;
    }
    else
      TDockableControlBar::EvLButtonDown(modKeys, point);
  }
}

void TDockableControlBarEx::EvLButtonDblClk(uint modKeys, const TPoint& point)
{
  TPoint p(point);
  if (GetAndSetCustomizeGadget(p))
    return;

  // Check that the DBLClk position is in a visible enabled Gadget
  // an the Dockable flag is set true
  //
  if (ShouldBeginDrag(p)) {
    ToggleSlip();
    return;
  }
  TDockableControlBar::EvLButtonDblClk(modKeys, point);
}

void TDockableControlBarEx::EvRButtonDown(uint modKeys, const TPoint& point)
{
  THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
  TPoint p(point);
  if (GetAndSetCustomizeGadget(p)) {

    POINT pp;
    ::GetCursorPos(&pp);

    HMENU hMenu = harbor->GetGadgetMenu();
    if (hMenu && ::GetSubMenu(hMenu, 0)) {
      ::TrackPopupMenu(::GetSubMenu(hMenu, 0),
        TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
        pp.x, pp.y, 0, GetApplication()->GetMainWindow()->GetHandle(), 0);
      ::DestroyMenu(hMenu);
    }
  }
  TDockableControlBar::EvRButtonDown(modKeys, point);
}

void TDockableControlBarEx::StorePosAndLocation()
{
  // get the actual absolute location of the toolbar
  //
  TDockingSlip* dockingSlip = TYPESAFE_DOWNCAST(Parent, TDockingSlip);
  TAbsLocation actLocation = (dockingSlip ? dockingSlip->GetLocation() : alNone);

  // Location changed
  //
  if (actLocation != Location) {
    // if the actual location is equal with the saved toggled slip location
    //
    if ((actLocation == alNone && ToggleLocation == alNone) ||
      (actLocation != alNone && ToggleLocation != alNone)) {
        ToggleRect = Rect;
        ToggleLocation = Location;
    }
    Location = actLocation;
  }

  // get the rectangle of the toolbar
  //
  TFloatingSlipEx* floatingSlip = TYPESAFE_DOWNCAST(Parent, TFloatingSlipEx);
  if (floatingSlip)
    floatingSlip->GetWindowRect(Rect);
  else
    GetRect(Rect);
}

void TDockableControlBarEx::LoadSettings(THarborManagement& harborMan)
{
  int  bVisible, bDefault, iLocation, iGadgetCount, id;
  TRect rect;
  uint32 size = sizeof(int);
  TCHAR title[255];
  uint32 sizeTitle = sizeof(title);
  TCHAR idstr[80];

  wsprintf(idstr, TEXT("%s%d"), ToolbarSection, Attr.Id);
  owl::tstring toolbarstr = harborMan.GetRegistryName() + owl::tstring(_T("\\")) +
    ToolbarsSection + owl::tstring(_T("\\")) + idstr;
  TRegKey regKey(TRegKey::GetCurrentUser(), toolbarstr.c_str());

  if (regKey.QueryValue(NameSection, 0, (uint8*)(TCHAR*)title, &sizeTitle) !=
    ERROR_SUCCESS) return;
  if (regKey.QueryValue(VisibleSection, 0, (uint8*)&bVisible, &size) !=
    ERROR_SUCCESS) return;
  if (regKey.QueryValue(DefaultSection, 0, (uint8*)&bDefault, &size) !=
    ERROR_SUCCESS)  return;
  if (regKey.QueryValue(LocationSection, 0, (uint8*)&iLocation, &size) !=
    ERROR_SUCCESS)  return;
  if (regKey.QueryValue(LeftSection, 0, (uint8*)&rect.left, &size) !=
    ERROR_SUCCESS)  return;
  if (regKey.QueryValue(TopSection, 0, (uint8*)&rect.top, &size) !=
    ERROR_SUCCESS) return;
  if (regKey.QueryValue(RightSection, 0, (uint8*)&rect.right, &size) !=
    ERROR_SUCCESS) return;
  if (regKey.QueryValue(BottomSection, 0, (uint8*)&rect.bottom, &size) !=
    ERROR_SUCCESS) return;
  if (regKey.QueryValue(GadgetCountSection, 0, (uint8*)&iGadgetCount, &size) !=
    ERROR_SUCCESS) return;

  RemoveAllGadgets();
  for (int i=1; i<=iGadgetCount; i++) {
    TCHAR posstr[80];
    wsprintf(posstr, TEXT("%d"), i);

    TRegKey regKey(TRegKey::GetCurrentUser(), toolbarstr.c_str());
    if(regKey.QueryValue(posstr, 0, (uint8*)&id, &size) != ERROR_SUCCESS)
      return;

    TGadget* gadget = GadgetDescriptors->ConstructGadget(id);
    CHECK(gadget);
    if (gadget) {
      Insert(*gadget);

      // Load Button Appearance values
      //
      TButtonTextGadgetEx* btg = TYPESAFE_DOWNCAST(gadget, TButtonTextGadgetEx);
      if (btg && THarborManagement::GetInternVersion() >= 2) {
        bool foundBtnStr = false;
        owl::tstring tbBtnStr = toolbarstr+owl::tstring(_T("\\"))+ButtonSection+posstr;
        TRegKey tbBtnRegKey(TRegKey::GetCurrentUser(), tbBtnStr.c_str());
        int bmpId;
        if (tbBtnRegKey.QueryValue(
          BmpIdSection, 0, (uint8*)&bmpId, &size) == ERROR_SUCCESS) {
            btg->SetResId(bmpId);
            foundBtnStr = true;
        }
        int dispType;
        if (tbBtnRegKey.QueryValue(
          DisplayTypeSection, 0, (uint8*)&dispType, &size) == ERROR_SUCCESS) {
            btg->SetDisplayType((TDisplayType)dispType);
            foundBtnStr = true;
        }
        TCHAR text[255];
        uint32 sizeText = sizeof(text);
        if (tbBtnRegKey.QueryValue(
          TextSection, 0, (uint8*)(TCHAR*)text, &sizeText) == ERROR_SUCCESS) {
            btg->SetCommandText(owl::tstring(text));
            foundBtnStr = true;
        }
        if (foundBtnStr == false) {
          TRegKey::GetCurrentUser().NukeKey(tbBtnStr.c_str());
        }
      }
    }
  }
  CheckOnLastGadget();

  if ((bool)bDefault == false)
    SetCaption(title);

  THarbor* harbor = GetHarbor();
  if (harbor) {
    harbor->Remove(*this);
    TDockingSlip* dslip = harbor->Insert(
      *this, (TAbsLocation)iLocation, &rect.TopLeft());

    // If the inserted docking slip is floating one with location alNone
    // compute the real size. Thanks to Jo Parrello
    //
    TFloatingSlipEx* flslip = TYPESAFE_DOWNCAST(dslip, TFloatingSlipEx);
    TSize size(rect.Width(), rect.Height());
    if (flslip && ((TAbsLocation)iLocation == alNone)) {
      TSize newsize = flslip->ComputeSize(alNone, &size);
      flslip->Layout(alNone, &newsize);
    }
  }

  if (IsWindowVisible() != (bool)bVisible)
  {
    if ((bool)bVisible)
      Show();
    else
      Hide();
  }
}

void TDockableControlBarEx::SaveSettings(THarborManagement& harborMan)
{
  // If we are visible save the location and position
  //
  if (IsWindowVisible())
    StorePosAndLocation();

  TCHAR idstr[80];
  wsprintf(idstr, TEXT("%s%d"), ToolbarSection, Attr.Id);
  int len = GetWindowTextLength();
  TCHAR* title = new TCHAR[len+1]; // +1 for 0 terminator
  if (len > 0)
    GetWindowText(title, len+1);
  else
    *title=0;
  owl::tstring toolbarstr = harborMan.GetRegistryName() + owl::tstring(_T("\\")) +
    ToolbarsSection + owl::tstring(_T("\\")) + idstr;

  TRegKey toolbarRegKey(TRegKey::GetCurrentUser(), toolbarstr.c_str());
  toolbarRegKey.SetValue(NameSection, REG_SZ, (const uint8*)title, len);
  toolbarRegKey.SetValue(VisibleSection, (int)IsWindowVisible());
  toolbarRegKey.SetValue(DefaultSection, (int)Default);
  toolbarRegKey.SetValue(LocationSection, Location);
  toolbarRegKey.SetValue(LeftSection, Rect.left);
  toolbarRegKey.SetValue(TopSection, Rect.top);
  toolbarRegKey.SetValue(RightSection, Rect.right);
  toolbarRegKey.SetValue(BottomSection, Rect.bottom);
  toolbarRegKey.SetValue(GadgetCountSection, NumGadgets);
  delete[] title;

  // Save Gadgets
  int i = 1;
  TGadget* gadget = FirstGadget();
  while (gadget) {
    TCHAR posstr[80];
    wsprintf(posstr, TEXT("%d"), i);

    TRegKey toolbarRegKey(TRegKey::GetCurrentUser(), toolbarstr.c_str());
    toolbarRegKey.SetValue(posstr, gadget->GetId());

    // Save Button Appearance values
    //
    TButtonTextGadgetEx* btg = TYPESAFE_DOWNCAST(gadget, TButtonTextGadgetEx);
    if (btg && btg->HasDefaultValues() == false) {

      owl::tstring tbBtnStr = toolbarstr + owl::tstring(_T("\\")) + ButtonSection + posstr;
      TRegKey tbBtnRegKey(TRegKey::GetCurrentUser(), tbBtnStr.c_str());

      TGadgetDesc* desc = GadgetDescriptors->Find(gadget->GetId());
      TButtonTextGadgetDesc* btgd = TYPESAFE_DOWNCAST(desc,
        TButtonTextGadgetDesc);
      uint32 rid = btg->GetResId().GetInt();
      if (btgd && btgd->BmpResId != rid) {
        tbBtnRegKey.SetValue(BmpIdSection, rid);
      }
      if (btg->GetDisplayType() != btgd->DispType) {
        tbBtnRegKey.SetValue(DisplayTypeSection, btg->GetDisplayType());
      }
      if (btg->GetCommandText() != btgd->Text) {
        owl::tstring commandtext = btg->GetCommandText();
        tbBtnRegKey.SetValue(TextSection, REG_SZ,
          (const uint8*)commandtext.c_str(), static_cast<uint32>(commandtext.size()));
      }
    }
    i++;
    gadget = gadget->NextGadget();
  }
}

void TDockableControlBarEx::CmOk()
{
  ::SendMessage((HWND)GetFocus(), WM_KEYDOWN, VK_RETURN, MAKELPARAM(1, 0));
}

void TDockableControlBarEx::CmCancel()
{
  ::SendMessage((HWND)GetFocus(), WM_KEYDOWN, VK_ESCAPE, MAKELPARAM(1, 0));
}

void TDockableControlBarEx::DrawVerticalDividers(TDC& dc, TGadget* firstgadg,
                         TGadget* lastgadg, int miny, int maxy, bool skipEndOfRowCheck)
                         // Draw vertical dividers for SeparatorGadgetEx.
                         // Very special thanks to Jo Parrello
{
  TGadget* currentgadg = firstgadg;
  bool canCont = true;
  while (canCont) {
    if (currentgadg == lastgadg)
      canCont = false;
    if (currentgadg) {
      if (currentgadg->GetId() == 0 && currentgadg->NextGadget() &&
        (skipEndOfRowCheck || currentgadg->IsEndOfRow() == false)) {
          TRect boundrect = currentgadg->GetBounds();
          int midx = (boundrect.right - boundrect.left) / 2 + boundrect.left;
          TPen pen(TColor::Sys3dShadow);
          dc.SelectObject(pen);
          dc.MoveTo(midx, miny);
          dc.LineTo(midx, maxy);
          dc.RestorePen();
          TPen pen2(TColor::Sys3dHilight);
          dc.SelectObject(pen2);
          dc.MoveTo(midx + 1, miny);
          dc.LineTo(midx + 1, maxy);
          dc.RestorePen();
      }
      currentgadg = currentgadg->NextGadget();
    }
  }
}

void TDockableControlBarEx::DrawHorizontalDividers(TDC& dc, TGadget* firstgadg,
                           TGadget* lastgadg, int minx, int maxx)
                           // Draw horizontal dividers for SeparatorGadgetEx.
                           // Very special thanks to Jo Parello
{
  TGadget* currentgadg = firstgadg;
  bool canCont = true;
  while (canCont) {
    if (currentgadg == lastgadg)
      canCont = false;
    if (currentgadg) {
      if (currentgadg->GetId() == 0 && currentgadg->NextGadget()) {
        TRect boundrect = currentgadg->GetBounds();
        int midy = (boundrect.bottom - boundrect.top) / 2 + boundrect.top;
        TPen pen(TColor::Sys3dShadow);
        dc.SelectObject(pen);
        dc.MoveTo(minx, midy);
        dc.LineTo(maxx, midy);
        dc.RestorePen();
        TPen pen2(TColor::Sys3dHilight);
        dc.SelectObject(pen2);
        dc.MoveTo(minx, midy + 1);
        dc.LineTo(maxx, midy + 1);
        dc.RestorePen();
      }
      currentgadg = currentgadg->NextGadget();
    }
  }
}

void TDockableControlBarEx::DrawCustomizeRect(TDC& dc, TRect& rect)
{
  TPen pen (TColor::Black, 2);
  dc.SelectObject(pen);
  dc.SelectStockObject(NULL_BRUSH);
  dc.Rectangle(rect);
  dc.RestoreBrush();
  dc.RestorePen();
}

// ****************** TGridItem ***********************************************

TGridItem::TGridItem(TWindow* Window, TAbsLocation Location)
{
  // Initialise data
  Dockable = Window;
  TRect WindowRect = Dockable->GetWindowRect();
  // All the workings are to be in client coordinates of the edge slip
  ::MapWindowPoints(0, Window->GetParentH(), (TPoint*)&WindowRect, 2);

  if (Location == alTop || Location == alBottom) {
    LeftEdge = WindowRect.left;
    TopEdge  = WindowRect.top;      // Only needed to find grid line
    Width    = WindowRect.Width();
    Height   = WindowRect.Height();
  }
  else {
    // Transpose the coordinates for internal calculations
    LeftEdge = WindowRect.top;
    TopEdge  = WindowRect.left;     // Only needed to find grid line
    Width    = WindowRect.Height();
    Height   = WindowRect.Width();
  }

  GridLine = 0;
  NextItem = 0;
  PrevItem = 0;
}

void TGridItem::ArrangeLeft()
// Recursive function tries to push dockables which are overlapped to the left.
// It is not possible to push them beyond the left edge of the slip and this
// leads to packing against the left edge.
{
  // Can only be called where there is a following item
  // Push this item to the immediate left of the following item
  LeftEdge = NextItem->LeftEdge - Width + 2;
  if (PrevItem) {
    if(LeftEdge < PrevItem->LeftEdge + PrevItem->Width - 2) {
      // Where this grid item overlaps the previous one, TRY to push the
      // previous one leftwards
      PrevItem->ArrangeLeft(); // This function recurses
      if(LeftEdge < PrevItem->LeftEdge + PrevItem->Width - 2) {
        // Where the previous item was prevented from moving far enough,
        // push this item to the right of it
        LeftEdge = PrevItem->LeftEdge + PrevItem->Width - 2;
      }
    }
  }
  else if (LeftEdge < -2) {
    // Where this is the first grid item, ensure it does not stick off the left
    LeftEdge = -2;
  }
}

void TGridItem::ArrangeRight(bool Limit, int SlipWidth)
// Recursive function tries to push dockables which are overlapped to the right.
// Where Limit is true, it is not possible to push them beyond the right edge of
// the slip and this leads to packing against the right edge. If Limit is false,
// items are allowed to stick out past the right edge, or even be pushed wholly
// beyond it.
{
  // Can only be called where there is a preceding item
  // Push this item to the immediate left of the preceding item
  LeftEdge = PrevItem->LeftEdge + PrevItem->Width - 2;
  if (NextItem) {
    if(LeftEdge + Width - 2 > NextItem->LeftEdge) {
      // Where this grid item overlaps the next one, TRY to push the
      // next one rightwards
      NextItem->ArrangeRight(Limit, SlipWidth); // This function recurses
      if(LeftEdge + Width - 2 > NextItem->LeftEdge) {
        // Where the next item was prevented from moving far enough,
        // push this item to the left of it
        LeftEdge = NextItem->LeftEdge - Width + 2;
      }
    }
  }
  else if (LeftEdge + Width - 2 > SlipWidth && Limit)
  {
    // Where this is the last grid item, ensure it does not stick off the right
    LeftEdge = SlipWidth - Width + 2;
  }
}

void TGridItem::ArrangePrecedingItems()
// Function called to move overlapped dockabes to the left of an anchor (this).
// The anchor is moved to the right if necessary.
{
  if (PrevItem) {
    if(LeftEdge < PrevItem->LeftEdge + PrevItem->Width - 2) {
      // Where this grid item overlaps the previous one, TRY to push the
      // previous one leftwards
      PrevItem->ArrangeLeft(); // This function recurses
      if(LeftEdge < PrevItem->LeftEdge + PrevItem->Width - 2) {
        // Where the previous item was prevented from moving far enough,
        // push this item to the right of it
        LeftEdge = PrevItem->LeftEdge + PrevItem->Width - 2;
      }
    }
  }
  else if (LeftEdge < -2) {
    // Where this is the first grid item, ensure it does not stick off the left
    LeftEdge = -2;
  }
}

void TGridItem::ArrangeFollowingItems(int SlipWidth)
// Function called to move overlapped dockabes to the right of an anchor (this).
// The function first tries to ensure packing against the right edge, but
// tries again without packing if the anchor cannot move far enough leftwards.
// The anchor is moved to the left or right if necessary.
{
  if (NextItem) {
    if(LeftEdge + Width - 2 > NextItem->LeftEdge) {
      // Where this grid item overlaps the next one, TRY to push the
      // next one rightwards
      NextItem->ArrangeRight(true, SlipWidth); // This function recurses
      if(LeftEdge + Width - 2 > NextItem->LeftEdge) {
        // Where the next item was prevented from moving far enough,
        // TRY to push this item to the left of it
        ArrangeLeft();
        if (LeftEdge + Width - 2 > NextItem->LeftEdge) {
          // Where this grid item still overlaps the next one, push the next one
          // rightwards
          NextItem->ArrangeRight(false, SlipWidth); // This function recurses
          // Is this part superfluous/counterproductive?
        }
      }
    }
  }
  else if (LeftEdge + Width - 2 > SlipWidth) {
    // Where this is the last grid item, ensure it does not stick off the right
    LeftEdge = SlipWidth - Width + 2;
    // This may result in the item sticking off the right edge
    ArrangePrecedingItems();
  }
}

// ****************** TGridLine ***********************************************

TGridLine::TGridLine(uint32 Top)
{
  TopEdge = Top;
  Height = 0;
  FirstItem = 0;
  NextLine = 0;
  PrevLine = 0;
}

TGridLine::~TGridLine()
{
  // CodeGuard loves me!
  TGridItem* Temp;
  while (FirstItem)
  {
    Temp = FirstItem->NextItem;
    delete FirstItem;
    FirstItem = Temp;
  }
}

TGridItem* TGridLine::LayoutGridItems(TGridItem* Anchor, int SlipWidth)
{
  // We should check that Anchor is in this gridline
  if (!Anchor)
    Anchor = FirstItem;

  // Confirm that the item is in the list
  TGridItem* IterItem = FirstItem;
  while (IterItem && IterItem != Anchor) {
    IterItem = IterItem->NextItem;
  }

  // Only do something if the item is in the list
  if (IterItem) {
    // Layout the dockables on this gridline so there are no overlaps
    Anchor->ArrangePrecedingItems();
    Anchor->ArrangeFollowingItems(SlipWidth);

    // Return a pointer to the first grid item pushed wholly off the right edge
    // of the slip
    TGridItem* FirstExcess;
    TGridItem* Temp = FirstItem;
    while (Temp && Temp->LeftEdge < SlipWidth)
      Temp = Temp->NextItem;
    FirstExcess = Temp;
    return FirstExcess;
  }
  return 0;
}

void TGridLine::ComputeHeight()
{
  int TempHeight = 0;
  // Height of the tallest grid item
  TGridItem* TempItem = FirstItem;
  while (TempItem)
  {
    TempHeight = max(TempItem->Height, TempHeight);
    TempItem = TempItem->NextItem;
  }
  Height = TempHeight;
}

void TGridLine::InsertGridItem(TGridItem* NewItem)
{
  if (!FirstItem)
    FirstItem = NewItem;
  else {
    TGridItem* InsertAfter = 0;
    TGridItem* InsertBefore = 0;
    TGridItem* IterNext = FirstItem;
    // Loop to sort position by LeftEdge
    while (IterNext) {
      if (NewItem->LeftEdge > IterNext->LeftEdge)
        InsertAfter = IterNext;
      else if (!InsertBefore)
        InsertBefore = IterNext;
      IterNext = IterNext->NextItem;
    }

    if (InsertAfter) {
      NewItem->PrevItem = InsertAfter;
      InsertAfter->NextItem = NewItem;
    }
    else
      FirstItem = NewItem;

    if (InsertBefore) {
      NewItem->NextItem = InsertBefore;
      InsertBefore->PrevItem = NewItem;
    }
    else
      NewItem->NextItem = 0; // Make sure the list is terminated properly
  }

  NewItem->GridLine = this;
  NewItem->TopEdge  = TopEdge;
  ComputeHeight();  // Can't remember why I added this...
}

void TGridLine::RemoveGridItem(TGridItem* NewItem)
{
  // Confirm that the item is in the list
  TGridItem* IterItem = FirstItem;
  while (IterItem && IterItem != NewItem)
    IterItem = IterItem->NextItem;

  // Only do something if the item is in the list
  if (IterItem) {
    if (NewItem->PrevItem)
      NewItem->PrevItem->NextItem = NewItem->NextItem;
    else {
      // The first item is being removed
      // Omitting this line caused the item in
      // question to be deleted twice - Bang!
      FirstItem = NewItem->NextItem;
    }
    if (NewItem->NextItem)
      NewItem->NextItem->PrevItem = NewItem->PrevItem;
    NewItem->PrevItem = 0;
    //NewItem->NextItem = 0; I'll be needing this later
    NewItem->GridLine = 0;
  }
}

// ****************** TGridLayout *********************************************

TGridLayout::TGridLayout(TAbsLocation Loc)
{
  FirstLine = 0;
  Parent = 0;
  Dockable = 0;
  DockableItem = 0;
  Location = Loc;
}

void TGridLayout::CreateLists(TWindow* parent, TWindow* dockable)
{
  Parent = parent;     // The edge slip
  Dockable = dockable; // The just dropped dockable child of the edge slip

  TWindow* FirstChild = Parent->GetFirstChild();
  if (FirstChild) {
    TWindow* ChildWindow = FirstChild;
    do {
      TGridItem* NewItem = new TGridItem(ChildWindow, Location);
      if (ChildWindow == Dockable) {
        // Don't insert the current item into the linked lists yet
        DockableItem = NewItem;
      }
      else {
        // Insert the item into the linked lists
        InsertGridItem(NewItem);
      }
      ChildWindow = ChildWindow->Next();
    }
    while (ChildWindow != FirstChild); // Does Next() cycle?
  }
}

TGridLine* TGridLayout::InsertDockableItem()
{
  // Need some logic to determine which existing grid line it should be in
  // or whether a new grid line should be inserted

  if (!DockableItem)
    return 0;

  TGridLine* DockableLine;

  if (FirstLine) {
    // Find first grid line with TopEdge greater than that of the new item
    TGridLine* IterPrev = 0;
    TGridLine* IterNext = FirstLine;
    while (IterNext && IterNext->TopEdge < DockableItem->TopEdge) {
      IterPrev = IterNext;
      IterNext = IterNext->NextLine;
    }
    // IterNext points to the first line with TopEdge >= DockableItem->TopEdge

    // Find the comparator
    int CompareTop;
    if (IterNext)
      CompareTop = IterNext->TopEdge;
    else
      CompareTop = IterPrev->TopEdge + IterPrev->Height - 2;

    // Decide which grid line to use based on degree of overlap of the new
    // dockable with the existing gridlines.
    // I'm sure this could be done lots better, but it does the trick quite
    // well.
    if (CompareTop < DockableItem->TopEdge + (DockableItem->Height - 1) / 3) {
      // Insert it on the next line - this will force a new line if no NextLine
      DockableItem->TopEdge = CompareTop;
      DockableLine = InsertGridItem(DockableItem);
    }
    else if (CompareTop < DockableItem->TopEdge + (2 * (DockableItem->Height - 1)) / 3) {
      // Force a new line to be inserted below
      DockableItem->TopEdge = CompareTop - 1;
      DockableLine = InsertGridItem(DockableItem);
    }
    else if (IterPrev) {
      CompareTop = IterPrev->TopEdge + IterPrev->Height - 2;
      if (CompareTop > DockableItem->TopEdge + (2 * (DockableItem->Height - 1)) / 3) {
        // Pull onto previous line
        DockableItem->TopEdge = IterPrev->TopEdge;
        DockableLine = InsertGridItem(DockableItem);
      }
      else {
        // Force a new line to be inserted below previous line
        DockableItem->TopEdge = IterPrev->TopEdge + 1;
        DockableLine = InsertGridItem(DockableItem);
      }
    }
    else {
      // Force a new line to be inserted above
      DockableItem->TopEdge = CompareTop - 1;
      DockableLine = InsertGridItem(DockableItem);
    }
  }
  else {
    DockableItem->TopEdge = -2;
    DockableLine = InsertGridItem(DockableItem);
  }
  return DockableLine;
}

TGridLayout::~TGridLayout()
{
  // CodeGuard loves me!
  TGridLine* Temp;
  while (FirstLine) {
    Temp = FirstLine->NextLine;
    delete FirstLine;
    FirstLine = Temp;
  }
}

int TGridLayout::LayoutGridLines()
{
  // Vertical layout - sets top edge mainly
  int CumulativeTop = -2;
  TGridLine* IterLine = FirstLine;
  while (IterLine) {
    IterLine->TopEdge = CumulativeTop;
    IterLine->ComputeHeight();
    CumulativeTop = CumulativeTop + IterLine->Height - 2;
    IterLine = IterLine->NextLine;
  }

  // Return the required height/width of the slip, taking all the gridlines
  // into consideration. The sides are slightly different to the top and
  // bottom because the etched edge slip borders are oriented differently
  // relative to the gridlines. CumulativeTop should not be negative - it
  // screws up the 3D client edge of the main client window.
  if (Location == alTop || Location == alBottom || CumulativeTop == -2)
    CumulativeTop += 2;
  return CumulativeTop;
}

void TGridLayout::LayoutDockables()
// This function iterates through the grid items and implements the results
// of all our calculations.
{
  TPoint ItemPoint;
  TPoint DockPoint;

  // Iterate through the grid lines
  TGridLine* IterLine = FirstLine;
  while (IterLine) {
    // Iterate through the grid items in the current grid line
    TGridItem* IterItem = IterLine->FirstItem;
    while (IterItem) {
      // Do something to the dockables
      // Make a point from the left of the dockable and the top of its grid line
      if (Location == alTop || Location == alBottom) {
        ItemPoint = TPoint(IterItem->LeftEdge, IterLine->TopEdge);
      }
      else {
        // Swap the points back over for the real window
        ItemPoint = TPoint(IterLine->TopEdge, IterItem->LeftEdge);
      }

      // Find the current top left point of the dockable
      DockPoint = TPoint(IterItem->Dockable->GetWindowRect().TopLeft());
      ::MapWindowPoints(0, *Parent, &DockPoint, 1);
      // Compare with the current top left point of the dockable
      if (ItemPoint != DockPoint) {
        IterItem->Dockable->SetWindowPos(0, ItemPoint.x, ItemPoint.y, 0, 0,
          SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
      }

      IterItem = IterItem->NextItem;
    }
    IterLine = IterLine->NextLine;
  }
}

void TGridLayout::InsertGridLine(TGridLine* NewLine)
{
  if (!FirstLine)
    FirstLine = NewLine;
  else {
    TGridLine* InsertAfter = 0;
    TGridLine* InsertBefore = 0;
    TGridLine* IterNext = FirstLine;
    // Loop to sort position by LeftEdge
    while (IterNext) {
      if (NewLine->TopEdge > IterNext->TopEdge)
        InsertAfter = IterNext;
      else if (!InsertBefore)
        InsertBefore = IterNext;
      IterNext = IterNext->NextLine;
    }

    if (InsertAfter) {
      NewLine->PrevLine = InsertAfter;
      InsertAfter->NextLine = NewLine;
    }
    else
      FirstLine = NewLine;

    if (InsertBefore) {
      NewLine->NextLine = InsertBefore;
      InsertBefore->PrevLine = NewLine;
    }
  }
}

TGridLine* TGridLayout::InsertGridItem(TGridItem* NewItem)
{
  NewItem->GridLine = 0; // Just to make sure
  NewItem->NextItem = 0; // In case it was removed from another line

  // Find which gridline the item should be inserted into
  TGridLine* IterLine = FirstLine;
  while (IterLine) {
    if (NewItem->TopEdge == IterLine->TopEdge)
      IterLine->InsertGridItem(NewItem);
    IterLine = IterLine->NextLine;
  }

  if (!NewItem->GridLine) // No grid line in which to insert it
  {
    // Create a new gridline for the item
    TGridLine* NewLine = new TGridLine(NewItem->TopEdge);
    InsertGridLine(NewLine);
    NewLine->InsertGridItem(NewItem);
  }

  // Return the grid line into which the item was inserted
  return NewItem->GridLine;
}

// ***************************** TFloatingSlipEx ******************************

DEFINE_RESPONSE_TABLE1(TFloatingSlipEx, TFloatingSlip)
EV_WM_NCLBUTTONDOWN,
EV_WM_NCRBUTTONDOWN,
EV_WM_LBUTTONDOWN,
EV_WM_CLOSE,
EV_WM_NCLBUTTONDBLCLK,
END_RESPONSE_TABLE;

TFloatingSlipEx::TFloatingSlipEx(TWindow* parent, int x, int y, TWindow* clientWnd,
                 bool shrinkToClient, int captionHeight, bool popupPalette, TModule* module):
TFloatingSlip(parent, x, y, clientWnd, shrinkToClient, captionHeight,
        popupPalette, module)
{
}

TFloatingSlipEx::~TFloatingSlipEx()
{
}

void TFloatingSlipEx::DockableInsert(TDockable& dockable, const TPoint* topLeft,
                   TRelPosition position, TDockable* relDockable)
{
  TFloatingSlip::DockableInsert(dockable, topLeft, position, relDockable);

  TDockableControlBarEx* cb = TYPESAFE_DOWNCAST(dockable.GetWindow(),
    TDockableControlBarEx);
  if (cb) {
    TMargins m(TMargins::Pixels,
      FloatMarginsX, FloatMarginsX, FloatMarginsY, FloatMarginsY);
    cb->SetMargins(m);
  }
}

void TFloatingSlipEx::DockableRemoved(const TRect& orgRect)
// Overwrite to call the TDockableControlBar that it will be removed
{
  TDockableControlBarEx* cb = TYPESAFE_DOWNCAST(GetWindow(),
    TDockableControlBarEx);
  if (cb)
    cb->PreRemoved();
  TFloatingSlip::DockableRemoved(orgRect);
}

void TFloatingSlipEx::EvNCLButtonDown(uint hitTest, const TPoint& point)
{
#ifdef CTXHELP
  TCtxHelpFileManager* ctxHelpM = TYPESAFE_DOWNCAST(GetApplication(),
    TCtxHelpFileManager);
  if (ctxHelpM && ctxHelpM->IsContextHelp()) {
    // click in the titlebar of floating slip: call help with Attr.Id != 0
    //
    TDockableControlBarEx* cb = TYPESAFE_DOWNCAST(GetWindow(),
      TDockableControlBarEx);
    if (cb && cb->Attr.Id != 0)
      ctxHelpM->WinHelp(cb->Attr.Id);
  }
  else
#endif
    TFloatingSlip::EvNCLButtonDown(hitTest, point);
}

void TFloatingSlipEx::EvNCRButtonDown(uint hitTest, const TPoint& point)
{
  TFloatingSlip::EvNCRButtonDown(hitTest, point);

  // Show popup toolbars popup menu if the user right clicks into the title
  //
  if (GetApplication()) {
    THarborManagement* hm = TYPESAFE_DOWNCAST(GetApplication(),
      THarborManagement);
    if (hm && hm->GetHarbor() && GetApplication()->GetMainWindow()) {
      HMENU hMenu = hm->GetHarbor()->GetToolbarMenu(true);
      if (hMenu && ::GetSubMenu(hMenu, 0) &&
        GetApplication()->GetMainWindow()->GetHandle()) {
          TPoint pt;
          ::GetCursorPos(&pt);
          ::TrackPopupMenu(::GetSubMenu(hMenu, 0),
            TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
            pt.x, pt.y, 0, GetApplication()->GetMainWindow()->GetHandle(), 0);
          ::DestroyMenu(hMenu);
      }
    }
  }
}

void TFloatingSlipEx::EvLButtonDown(uint hitTest, const TPoint& point)
{
#ifdef CTXHELP
  TCtxHelpFileManager* ctxHelpM = TYPESAFE_DOWNCAST(GetApplication(),
    TCtxHelpFileManager);
  if (ctxHelpM && ctxHelpM->IsContextHelp()) {
    // click in the margings of floating slip: call help with Attr.Id != 0
    //
    TDockableControlBarEx* cb = TYPESAFE_DOWNCAST(GetWindow(),
      TDockableControlBarEx);
    if (cb && cb->Attr.Id != 0)
      ctxHelpM->WinHelp(cb->Attr.Id);
  }
  else
#endif
    TFloatingSlip::EvLButtonDown(hitTest, point);
}

void TFloatingSlipEx::EvClose()
// Update the customize dialog when the user click on x button in titlebar
{
  TFloatingSlip::EvClose();

  // Get x box rect. It's relative to to upper left of tiny caption
  //
  TPoint screenPt;
  GetCursorPos(screenPt);
  TRect rect = TTinyCaption::GetSysBoxRect();
  TPoint winPt = screenPt - TTinyCaption::GetWindowRect().TopLeft();
  rect.Offset(TTinyCaption::GetCaptionRect().Width()-rect.Width(), 0);
  if (((TTinyCaption::GetStyle() & WS_SYSMENU) || TTinyCaption::CloseBox)&&
    rect.Contains(winPt)) {
      THarborEx* harbor = TYPESAFE_DOWNCAST(GetHarbor(), THarborEx);
      if (harbor)
        harbor->UpdateShowHideCtlBar();
  }
}

void TFloatingSlipEx::EvNCLButtonDblClk(uint modKeys, const TPoint& point)
{
  TDockableControlBarEx* cb = TYPESAFE_DOWNCAST(GetWindow(),
    TDockableControlBarEx);
  if (cb)
    cb->ToggleSlip();
  TFloatingSlip::EvNCLButtonDblClk(modKeys, point);
}

// ****************** TEdgeSlipEx *********************************************

DEFINE_RESPONSE_TABLE1(TEdgeSlipEx, TWindow)
EV_WM_LBUTTONDOWN,
EV_WM_LBUTTONDBLCLK,
EV_WM_NCCALCSIZE,
EV_WM_NCPAINT,
EV_WM_ERASEBKGND,
EV_WM_PARENTNOTIFY,
EV_WM_WINDOWPOSCHANGING,
END_RESPONSE_TABLE;

TEdgeSlipEx::TEdgeSlipEx(TDecoratedFrame& parent, TAbsLocation location, TModule* module)
:
TWindow(&parent, _T("EdgeSlipEx"), module),
Location(location),
GridType(Location == alTop || Location == alBottom ? YCoord : XCoord)
{
  SetBkgndColor(TColor::Sys3dFace);
  Attr.Style = (WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
  InsertingDockable = false;
}

TEdgeSlipEx::~TEdgeSlipEx()
{
  Destroy(IDCANCEL);
}

void TEdgeSlipEx::SetGripperMargins(TDockableControlBarEx* cb)
// Set the margins for the gripper
{
  if (cb) {
    THarborEx* harbor = TYPESAFE_DOWNCAST(cb->GetHarbor(), THarborEx);
    if (harbor) {
      int gripperSize = 3*GripperDistSize;
      if (Location == alTop || Location == alBottom) {
        TMargins m(TMargins::Pixels,
          (harbor->IsDrawGripper()) ? gripperSize+EdgeMaginsX : EdgeMaginsX,
          EdgeMaginsX,
          EdgeMaginsY,
          EdgeMaginsY);
        cb->SetMargins(m);
      }
      else {
        TMargins m(TMargins::Pixels,
          EdgeMaginsX,
          EdgeMaginsX,
          (harbor->IsDrawGripper()) ? gripperSize+EdgeMaginsY : EdgeMaginsY,
          EdgeMaginsY);
        cb->SetMargins(m);
      }
    }
  }
}

void TEdgeSlipEx::DockableInsert(TDockable& dockable, const TPoint* topLeft,
                 TRelPosition position, TDockable* relDockable)
                 // Similar to TEdgeSlip
{
  // Get dockable's window & hide it in case we have to toss it around a bit
  // Reparent the window to the edge slip
  TWindow* dockableWindow = dockable.GetWindow();
  dockableWindow->ShowWindow(SW_HIDE);
  dockableWindow->SetParent(this);

  //D: Set the margins for the Gripper
  //
  TDockableControlBarEx* cb = TYPESAFE_DOWNCAST(dockableWindow,
    TDockableControlBarEx);
  SetGripperMargins(cb);

  // Let window know it was docked...
  if (dockableWindow->IsWindow())
    dockableWindow->SendMessage(WM_OWLWINDOWDOCKED, TParam1(position),
    TParam2((TDockingSlip*)this));

  // Make sure that the dockable is oriented the right way--horizontal layout
  // for Y-gridded slips
  InsertingDockable = true;  // Prevent EvParentNotify from doing anything
  dockable.Layout(GridType == YCoord ? alTop : alLeft);
  InsertingDockable = false;

  DockableMove(dockable, topLeft, position, relDockable);
  dockableWindow->ShowWindow(SW_SHOWNA);
}

void TEdgeSlipEx::DockableMove(TDockable& dockable, const TPoint* topLeft,
                 TRelPosition /*position*/, TDockable* /*relDockable*/)
{
  TWindow* dockableWindow = dockable.GetWindow();

  if (topLeft) {
    // Move the window into position relative to its parent (this)
    ::MapWindowPoints(0, *this, (TPoint*)topLeft, 1);
    dockableWindow->SetWindowPos(0, topLeft->x, topLeft->y, 0, 0,
      SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
  }
  else {
    // A stand-in until a better method of approximation is used
    dockableWindow->SetWindowPos(0, -2, -2, 0, 0,
      SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
  }

  // Now do the magic which shuffles all the dockables about
  ArrangeAllDockables(dockableWindow);
}

void TEdgeSlipEx::DockableRemoved(const TRect& /*orgRect*/)
// Overwrite to call the TDockableControlBar that it will be removed
{
  TDockableControlBarEx* cb = TYPESAFE_DOWNCAST(GetFirstChild(),
    TDockableControlBarEx);
  if (cb)
    cb->PreRemoved();

  // Do the magic which shuffles all the dockables about
  // Should just close up empty gridlines
  ArrangeAllDockables(0);
}

void TEdgeSlipEx::ArrangeAllDockables(TWindow* NewDockable)
{
  TGridLayout GridLayout(Location);
  GridLayout.CreateLists(this, NewDockable);

  TGridLine* DockableLine = GridLayout.InsertDockableItem();
  if (DockableLine) {
    int SlipWidth;
    if (Location == alTop || Location == alBottom) {
      // This gave zero if the slip was just created - first toolbar positioned
      // always at the far left, not the drop position.
      SlipWidth = GetWindowRect().Width();

      //SlipWidth = std::max(Parent->GetClientRect().Width(), 0);
    }
    else {
      // This gave zero if the slip was just created - first toolbar positioned
      // always at the top, not the drop position.
      SlipWidth = std::max(GetWindowRect().Height() - 4, 0); // Compensate for borders

      //TFrameWindow* Frame = TYPESAFE_DOWNCAST(Parent, TFrameWindow);
      // This makes no allowance for the possible increase if the new toolbar was
      // dragged from the top or bottom slips, or for the offset if the new
      // toolbar was dragged from the top slip.
      //SlipWidth = std::max(Frame->GetClientWindow()->GetClientRect().Height() - 4, 0);
    }

    TGridItem* OverItem = DockableLine->LayoutGridItems(GridLayout.DockableItem, SlipWidth);
    if (OverItem) {
      // This will deal with wrapping items pushed off the right edge.
      // Assumes we only need to wrap at most one new grid line.
      TGridItem* IterItem = OverItem;
      while (IterItem) {
        TGridItem* IterNext = IterItem->NextItem;

        DockableLine->RemoveGridItem(IterItem);
        IterItem->TopEdge += 1;  // Force a new line to be inserted
        IterItem->LeftEdge = -2; // Jam over to the left side
        GridLayout.InsertGridItem(IterItem);

        IterItem = IterNext;
      }
      TGridLine* ExtraLine = OverItem->GridLine;
      // Layout the new line
      ExtraLine->LayoutGridItems(OverItem, SlipWidth);
    }
  }
  int SlipHeight = GridLayout.LayoutGridLines();

  // Translate the calculations to actual window positions
  GridLayout.LayoutDockables();

  // Alter the size of the slip itself if necessary
  LayoutSlip(SlipHeight);
}

void TEdgeSlipEx::LayoutSlip(int Height)
{
  // This resizes the edge slip by modifying the layout metrics held by its
  // parent frame
  bool SizeChanged;
  if (Location == alTop || Location == alBottom) {
    SizeChanged = Height != Attr.H;
    Attr.H = Height;
  }
  else {
    SizeChanged = Height != Attr.W;
    Attr.W = Height;
  }

  if (SizeChanged) {
    TDecoratedFrame* Frame = TYPESAFE_DOWNCAST(Parent, TDecoratedFrame);
    TLayoutMetrics Metrics;
    Frame->GetChildLayoutMetrics(*this, Metrics);
    Frame->SetChildLayoutMetrics(*this, Metrics);
    Frame->Layout();
  }
}

TAbsLocation TEdgeSlipEx::GetLocation() const
// Copied more or less from TEdgeSlip implemenation
{
  return Location;
}

TResult TEdgeSlipEx::EvCommand(uint id, THandle hWndCtl, uint notifyCode)
// Copied more or less from TEdgeSlip implemenation
{
  if (notifyCode == 0 && Parent)
    return Parent->EvCommand(id, hWndCtl, notifyCode);

  return TWindow::EvCommand(id, hWndCtl, notifyCode);
}

void TEdgeSlipEx::EvCommandEnable(TCommandEnabler& commandEnabler)
// Copied more or less from TEdgeSlip implemenation
{
  if (Parent) {
    // Already being processed?
    if (!commandEnabler.IsReceiver(Parent->GetHandle())) {
      // No, so forward it up to our parent
      commandEnabler.SetReceiver(Parent->GetHandle());
      Parent->EvCommandEnable(commandEnabler);
    }
  }
}

void TEdgeSlipEx::SetupWindow()
{
  TWindow::SetupWindow();

  // Give an initial position to all the child windows
  // This part may not be required if user wants to set positions
  // directly via Attr
  TWindow* FirstChild = GetFirstChild();
  if (FirstChild) {
    TWindow* ChildWindow = FirstChild;
    do {
      // A stand-in until a better method of approximation is used
      ChildWindow->SetWindowPos(0, -2, -2, 0, 0,
        SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
      ChildWindow = ChildWindow->Next();
    }
    while (ChildWindow != FirstChild); // Does Next() cycle?
  }

  TGridLayout GridLayout(Location);
  GridLayout.CreateLists(this, 0); //No Anchor
  int SlipWidth;
  if (Location == alTop || Location == alBottom)
    SlipWidth = GetWindowRect().Width();
  else
    SlipWidth = std::max(GetWindowRect().Height() - 4, 0); // Compensate for borders

  // This spaces the grid lines vertically to ensure there is space to insert
  // new lines if necessary
  GridLayout.LayoutGridLines();

  TGridLine* DockableLine = GridLayout.FirstLine;
  // Cycle through the grid lines to lay them all out
  while (DockableLine) {
    TGridItem* OverItem = DockableLine->LayoutGridItems(0, SlipWidth);
    TGridLine* ExtraLine = DockableLine;
    while (OverItem) {
      // This will deal with wrapping items pushed off the right edge
      // Loops in case of multiple wraps
      TGridItem* IterItem = OverItem;
      while (IterItem) {
        TGridItem* IterNext = IterItem->NextItem;

        ExtraLine->RemoveGridItem(IterItem);
        IterItem->TopEdge += 1;  // Force a new line to be inserted
        IterItem->LeftEdge = -2; // Jam over to the left side
        GridLayout.InsertGridItem(IterItem);

        IterItem = IterNext;
      }
      // Change to the newly created line
      ExtraLine = OverItem->GridLine;
      // Layout the new line
      OverItem = ExtraLine->LayoutGridItems(OverItem, SlipWidth);
    }
    // Not the best scheme since newly inserted lines will be laid out twice.
    DockableLine = DockableLine->NextLine;
  }
  int SlipHeight = GridLayout.LayoutGridLines();

  // Translate the calculations to actual window positions
  GridLayout.LayoutDockables();

  // Alter the size of the slip itself if necessary
  LayoutSlip(SlipHeight);
}

void TEdgeSlipEx::EvLButtonDown(uint modKeys, const TPoint& point)
// Copied more or less from TEdgeSlip implemenation
{
  TWindow* cw = GetWindowPtr(ChildWindowFromPoint(point));

  // Only allow immediate children of the docking window to be clicked on.
  if (cw && cw->GetParentO() == this) {
    // Is the mouseDown in a area where we can move the docked window?
    TPoint childPt = point;
    MapWindowPoints(*cw, &childPt, 1);
    TDockable* d = TYPESAFE_DOWNCAST(cw, TDockable);
    if (d && d->ShouldBeginDrag(childPt)) {
      TPoint p(point);
      MapWindowPoints(0, &p, 1);
      if (Harbor && Harbor->DockDraggingBegin(*d, p, Location, this))
        return;  // Successfully started
    }
  }
  TWindow::EvLButtonDown(modKeys, point);
}

void TEdgeSlipEx::EvLButtonDblClk(uint /*modKeys*/, const TPoint& /*point*/)
// Copied more or less from TEdgeSlip implemenation
{
  Parent->HandleMessage(WM_OWLSLIPDBLCLK);
}

uint TEdgeSlipEx::EvNCCalcSize(bool calcValidRects, NCCALCSIZE_PARAMS & calcSize)
// Copied more or less from TEdgeSlip implemenation
{
  // Why is this function required?
  // Does it affect the interpretation of the client coordinates?
  // Yes - this appears to be the case.

  uint ret = TWindow::EvNCCalcSize(calcValidRects, calcSize);
  if (IsIconic())
    return ret;

  // Only add in space if this slip is not shrunk to nothing
  if (calcSize.rgrc[0].bottom - calcSize.rgrc[0].top > 0) {
    if (!(Attr.Style & WS_BORDER)) {
      if (Location != alBottom)
        calcSize.rgrc[0].top += 2;
      if (Location != alTop)
        calcSize.rgrc[0].bottom -= 2;
    }
  }
  return 0;
}

void TEdgeSlipEx::EvNCPaint(HRGN)
// Copied more or less from TEdgeSlip implemenation
{
  // Non-3d style
  if (Attr.Style & WS_BORDER) {
    DefaultProcessing();
  }
  // Use 3-d style
  else {
    // Paint etched line along the top for left, top & right slips, and along
    // the bottom for bottom, left & right slips to separate from the menubar,
    // statusbar & each other.
    TWindowDC dc(*this);
    int height(GetWindowRect().Height());
    if (Location != alBottom)
      TUIBorder(TRect(0,0,9999,2), TUIBorder::EdgeEtched, TUIBorder::Top).Paint(dc);
    if (Location != alTop)
      TUIBorder(TRect(0,height-2,9999,height), TUIBorder::EdgeEtched, TUIBorder::Bottom).Paint(dc);
  }
}

bool TEdgeSlipEx::EvEraseBkgnd(HDC hDC)
// Copied more or less from TEdgeSlip implemenation
{
  TWindow::EvEraseBkgnd(hDC);  // Let TWindow erase everything
  return true;
}

void TEdgeSlipEx::EvParentNotify(const TParentNotify& n)
// Copied more or less from TEdgeSlip implemenation
{
  if (n.Event == WM_SIZE && !InsertingDockable)
    ArrangeAllDockables(0); // Can cause double Layout of frame window
  else
    DefaultProcessing();
}

bool TEdgeSlipEx::EvWindowPosChanging(WINDOWPOS& windowPos)
// Copied more or less from TEdgeSlip implemenation
{
  return TWindow::EvWindowPosChanging(windowPos);
}

} // OwlExt namespace
//========================================================================