//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
//
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/pictdlg.h>

using namespace owl;

namespace OwlExt {

//
// Diagnostic group declarations/definitions
//
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlexPictDlg, 1, 0);
DIAG_DECLARE_GROUP(OwlMsg);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                    TPictDialog
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TPictDialog::TPictDialog(TDib* dib, TWindow* parent, TResId resId, TModule* module)
:
TDialog(parent, resId, module),
m_pdib(dib)
{
  TRACEX(OwlexPictDlg, 1, "TPictDialog constructed @ " << (void*)this);
  TRACEX(OwlexPictDlg, 2, "dib = " << (void*)dib << ", "
    "parent = " << (void*)parent << ", "
    "resId = " << resId << ", "
    "module = " << (void*)module);
}
TPictDialog::~TPictDialog()
{
  TRACEX(OwlexPictDlg, 1, "TPictDialog @ " << (void*)this << " destructed");
}

TDib* TPictDialog::SetDib(TDib* newDib)
{
  TRACEX(OwlexPictDlg, 1, "TPictDialog::SetDib replacing " << (void*)m_pdib
    << " with " << (void*)newDib);
  TDib* oldDib = m_pdib;
  m_pdib = newDib;
  return oldDib;
}

DEFINE_RESPONSE_TABLE1(TPictDialog, TDialog)
EV_WM_ERASEBKGND,
END_RESPONSE_TABLE;

bool TPictDialog::EvEraseBkgnd(HDC hdc)
{
  TRACEX(OwlexPictDlg, 2, "TPictDialog::EvEraseBkgnd entered");

  // Call back up the chain, in case the DIB doesn't cover the entire dialog
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
  TDib* dib = GetDib();
  if (dib){
    if (hasPalette){
      TRACEX(OwlexPictDlg, 3, "TPictDialog::EvEraseBkgnd: DC supports palettes; "
        "selecting and realizing DIB palette");
      palette = new TPalette(*GetDib());
      dc.SelectObject(*palette);
      dc.RealizePalette();
    }

    // figure out upper left corner of the client area
    //
    TRect clientRect(GetClientRect());
    TPoint sourcePoint(0, 0);
    TRACEX(OwlexPictDlg, 3, "TPictDialog::EvEraseBkgnd: clientRect = " << clientRect);

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
    TRACEX(OwlexPictDlg, 3, "TPictDialog::EvEraseBkgnd: Displaying dib;"
      " clientRect = " << clientRect << ", sourcePoint = " << sourcePoint);
    dc.SetDIBitsToDevice(clientRect, sourcePoint, *dib);

    // restore the DC to what it was
    //
    dc.RestoreObjects();
  } // if (dib)
  TRACEX(OwlexPictDlg, 2, "TPictDialog::EvEraseBkgnd exited");
  return true;
}

} // OwlExt namespace

#ifdef TEST_THE_CODE

// Core
//
#include <owl/owlcore.h>

using namespace OwlExt;

int OwlMain(int, char* [])
{
  // Since TDialog::Execute() tells TApplication to take over in the message
  // loop processing, no explicit call to TApplication::Run() is required.
  //
  TPictDialog dlg(new TDib("BALL.BMP"), 0, 1);
  dlg.Execute();
}

#endif
