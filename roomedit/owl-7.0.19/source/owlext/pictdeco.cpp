//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// TPictDecorator: Decorator-pattern-based class to add bitmap-backgrounds
//    to any OWL client window
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/pictdeco.h>

using namespace owl;

namespace OwlExt {

//
// Diagnostic group declarations/definitions
//
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlexPictDecor, 1, 0);
DIAG_DECLARE_GROUP(OwlMsg);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                    TPictDialog
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TPictDecorator::TPictDecorator(TDib* dib, TWindow& decorated)
:
TWindow(decorated.GetHandle()),
pDib(dib)
{
  TRACEX(OwlexPictDecor, 1, "TPictDecorator constructed @ " << (void*)this);
  TRACEX(OwlexPictDecor, 2, "dib = " << (void*)dib << ", decorated = " << (void*)&decorated);
}
TPictDecorator::~TPictDecorator()
{
  TRACEX(OwlexPictDecor, 1, "TPictDecorator @ " << (void*)this << " destructed");

  // Note that we assume *no* responsibility for the TDib* by this method; we
  // probably should, but that's another issue.
}


DEFINE_RESPONSE_TABLE1(TPictDecorator, TWindow)
EV_WM_ERASEBKGND,
EV_WM_SIZE,
END_RESPONSE_TABLE;

bool TPictDecorator::EvEraseBkgnd(HDC hdc)
{
  TRACEX(OwlexPictDecor, 2, "TPictDecorator::EvEraseBkgnd entered");

  // Call back up the chain, in case the DIB doesn't cover the entire window
  //
  DefaultProcessing();

  // create a TDC based off of the DC handed to us
  //
  TDC dc(hdc);

  // From here on, code copied almost verbatim from TPictWindow
  //

  // Check if the DC supports palettes, and if so, select the DIB's into it
  //
  TPointer<TPalette> palette(0);
  bool hasPalette = ToBool(dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE);
  TDib* dib = pDib();
  // We could just use the op() form of TProperty<> everywhere we reference
  // 'dib' in the code below, but for speed reasons (which are pretty slim), I
  // choose to capture the pointer stored in the TProperty into a local var and
  // use that, instead.

  if (dib){
    if (hasPalette){
      TRACEX(OwlexPictDecor, 3, "TPictDecorator::EvEraseBkgnd: DC supports palettes; "
        "selecting and realizing DIB palette");
      palette = new TPalette(*dib);
      dc.SelectObject(*palette);
      dc.RealizePalette();
    }

    // figure out upper left corner of the client area
    //
    TRect clientRect(GetClientRect());
    TPoint sourcePoint(0, 0);
    TRACEX(OwlexPictDecor, 3, "TPictDecorator::EvEraseBkgnd: clientRect = " << clientRect);

    // determine offsets
    //
    int offsetX = abs(dib->Width() - clientRect.Width()) / 2;
    if (dib->Width() > clientRect.Width())
      sourcePoint.x += offsetX;
    else
      clientRect.Offset(offsetX, 0);

    int offsetY = abs(dib->Height() - clientRect.Height()) / 2;
    if (dib->Height() > clientRect.Height())
      sourcePoint.y += offsetY;
    else
      clientRect.Offset(0, offsetY);

    // adjust the lower right corner
    //
    clientRect.bottom = clientRect.top + dib->Height();
    clientRect.right  = clientRect.left + dib->Width();

    // if the picture is larger than screen dimensions,
    // adjust the upper left corner.
    //
    clientRect.top   -= sourcePoint.y;
    clientRect.left  -= sourcePoint.x;

    // display the dib
    //
    TRACEX(OwlexPictDecor, 3, "TPictDecorator::EvEraseBkgnd: Displaying dib;"
      " clientRect = " << clientRect << ", sourcePoint = " << sourcePoint);
    dc.SetDIBitsToDevice(clientRect, sourcePoint, *dib);

    // restore the DC to what it was
    //
    dc.RestoreObjects();
  } // if (dib)

  TRACEX(OwlexPictDecor, 2, "TPictDecorator::EvEraseBkgnd exited");
  return true;
}
void TPictDecorator::EvSize(uint /*sizeType*/, const TSize& /*size*/)
{
  // Because Windows 4.0 sends WM_SIZE messages to the window *as* the window
  // is being resized, the background pict can be drawn in weird ways, esp. if
  // the pict doesn't cover the entire size of the window. To help with this, I
  // force the entire background to be redrawn on each WM_SIZE message. This has
  // the unfortunate effect of causing some flicker in the redrawing process, but
  // if you care not to see that flicker, just comment out this entire event-handler
  // (and its corresponding macro in the TPictDecorator event-table) and you'll
  // see no flicker. I only recommend this for non-Windows 4.0 apps.
  //
  Invalidate();

  // Call back to the decorated window for help
  //
  DefaultProcessing();
}


} // OwlExt namespace


#ifdef TEST_THE_CODE

// Core
//
#include <owl/owlcore.h>

using namespace OwlExt;

int OwlMain(int, char* [])
{
  // Kick off default app with default frame window
  //
  GetApplicationObject()->Start();  //??????????????????????????????????????????

  // Create a vanilla TWindow with a TPictDecorator wrapped around it
  //
  TWindow* window = new TWindow(GetApplicationObject()->GetMainWindow());
  window->Create();
  // TPictDecorator requires an HWND it can subclass, so Create() window before
  // handing it off to TPictDecorator.
  TPictDecorator* pictDecor = new TPictDecorator(new TDib("CLOUDS.BMP"), *window);
  // pictDecor will be delete'd in the normal course of things, by TApplication
  // when it shuts down, so pictDecor *must* be a heap-allocated object!

  // Set the pictDecor to be the client window
  //
  GetApplicationObject()->GetMainWindow()->SetClientWindow(window);

  // Let 'er rip
  //
  return GetApplicationObject()->Run();
}

#endif