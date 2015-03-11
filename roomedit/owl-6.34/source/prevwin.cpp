//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TPreviewWin
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/prevwin.h>
#include <owl/prevwin.rh>
#include <owl/editfile.rh>
#include <stdio.h>

namespace owl { 

OWL_DIAGINFO;

DEFINE_RESPONSE_TABLE1(TPreviewWin, TDecoratedFrame)
  EV_COMMAND(IDB_PREVIOUS, CmPrevious),
  EV_COMMAND_ENABLE(IDB_PREVIOUS, CePrevious),
  EV_COMMAND(IDB_NEXT, CmNext),
  EV_COMMAND_ENABLE(IDB_NEXT, CeNext),
  EV_COMMAND(IDB_ONEUP, CmOneUp),
  EV_COMMAND(IDB_TWOUP, CmTwoUp),
  EV_COMMAND_ENABLE(IDB_TWOUP, CeTwoUp),
  EV_COMMAND(IDB_DONE, CmDone),
END_RESPONSE_TABLE;


//
/// Destructor of Preview frame window. Performs general cleanup.
//
TPreviewWin::TPreviewWin(TWindow *parentWindow,   /* Parent window object */
                         TPrinter&  printer,      /* Printer object       */
                         TPrintout& printout,     /* Printout object      */
                         TWindow&   dataWindow,   /* Owner of preview data*/
                         LPCTSTR    title,        /* Title of preview win */
                         TLayoutWindow* client)   /* Client of preview win*/
:
  TDecoratedFrame(parentWindow, title, client),
  DataWindow(dataWindow),
  Printer(printer),
  Printout(printout),
  PrnDC(0),
  PreviewSpeedBar(0),
  Page1(0),
  Page2(0),
  Client(client)
{
  PRECONDITION(client);
  Init();
}

//
/// String-aware overload
//
TPreviewWin::TPreviewWin(
  TWindow *parentWindow,
  TPrinter&  printer,
  TPrintout& printout,
  TWindow& dataWindow,
  const tstring& title,
  TLayoutWindow* client)
:
  TDecoratedFrame(parentWindow, title, client),
  DataWindow(dataWindow),
  Printer(printer),
  Printout(printout),
  PrnDC(0),
  PreviewSpeedBar(0),
  Page1(0),
  Page2(0),
  Client(client)
{
  PRECONDITION(client);
  Init();
}

void TPreviewWin::Init()
{
  TPrintDialog::TData& data = Printer.GetSetup();

  // Retrieve device context of default printer
  //
  PrnDC = new TPrintDC(data.GetDriverName(),
                       data.GetDeviceName(),
                       data.GetOutputName(),
                       data.GetDevMode());

  // Let printer object update it page sizes
  //
  Printer.SetPageSizes(*PrnDC);

  // Make a local copy of the printer's page size
  //
  PrinterPageSize = Printer.GetPageSize();

  // Set background of preview window
  //
  SetBkgndColor(TColor::SysAppWorkspace);

  // Setup toolbar of preview window
  //
  SetupSpeedBar();

  // !BB Revisit the following restrictions...
  //

#if 0
  // We want a window that cannot be sized, maximized, or minimized.
  //
  Attr.Style = WS_VISIBLE | WS_POPUPWINDOW | WS_MINIMIZEBOX;

#else
  Attr.X = 0;
  Attr.Y = -1;
  Attr.W = GetParentO()->GetClientRect().Width();
  Attr.H = GetParentO()->GetClientRect().Height() + 1;
  Parent->MapWindowPoints(GetHandle(), (TPoint*)&Attr.X, 1);
#endif
}

//
// Destructor of Preview frame window - Performs general cleanup
//
TPreviewWin::~TPreviewWin()
{
  delete Page1;
  Page1 = 0;
  delete Page2;
  Page2 = 0;

  delete PrnDC;
  PrnDC = 0;
}

//
// Creates a control bar which is inserted along the top edge of the
// preview frame window.
// NOTE: The control bar is populated with buttongadgets to iterate
//       through the pages and print the document.
//
void
TPreviewWin::SetupSpeedBar()
{
  // Create default toolbar New and associate toolbar buttons with commands.
  //
  PreviewSpeedBar = new TControlBar(this);
  PreviewSpeedBar->Insert(*new TButtonGadget(IDB_PREVIOUS, IDB_PREVIOUS, TButtonGadget::Command, true));
  PreviewSpeedBar->Insert(*new TButtonGadget(IDB_NEXT, IDB_NEXT, TButtonGadget::Command, true));
  PreviewSpeedBar->Insert(*new TSeparatorGadget(6));
  PreviewSpeedBar->Insert(*new TButtonGadget(IDB_ONEUP, IDB_ONEUP, TButtonGadget::Exclusive, true, TButtonGadget::Down));
  PreviewSpeedBar->Insert(*new TButtonGadget(IDB_TWOUP, IDB_TWOUP, TButtonGadget::Exclusive, true));
  PreviewSpeedBar->Insert(*new TSeparatorGadget(12));
#if BI_MSG_LANGUAGE == 0x0411
  PreviewSpeedBar->Insert(*new TTextGadget(IDB_CURRPAGE, TGadget::Recessed, TTextGadget::Left, 10, "Íß°¼Þ 1"));
#else
  PreviewSpeedBar->Insert(*new TTextGadget(IDB_CURRPAGE, TGadget::Recessed, TTextGadget::Left, 10, _T("Page 1")));
#endif
  PreviewSpeedBar->Insert(*new TSeparatorGadget(20));
  PreviewSpeedBar->Insert(*new TButtonGadget(CM_FILEPRINT, CM_FILEPRINT, TButtonGadget::Command, true));
  PreviewSpeedBar->Insert(*new TSeparatorGadget(20));
  PreviewSpeedBar->Insert(*new TButtonGadget(IDB_DONE, IDB_DONE, TButtonGadget::Command, true));
  Insert(*PreviewSpeedBar, TDecoratedFrame::Top);
}

//
/// Overriden virtual of TWindow to allow preview frame to create the preview
/// page(s).
//
void
TPreviewWin::SetupWindow()
{
  TDecoratedFrame::SetupWindow();

  // !BB NOTE: The follow order of calls (i.e. calling
  //        SetPrintParams() before calling GetDialogInfo()
  //        is the opposite of the order used by TPrinter::Print
  //        (when we're really printing). However, the following
  //        seems more logical since the 'Printout' can accurately
  //        compute it page info. only after it has received the
  //        PrinterDC...
  //        Investigate and make update so that a 'Printout' can
  //        expect a consistent order of calls whether we're printing
  //        or simply previewing...
  //

  // Hand printout the target DC and page size
  //
  Printout.SetPrintParams(PrnDC, PrinterPageSize);

  // Allow printout to update page range information
  //
  TPrintDialog::TData& data = Printer.GetSetup();
  Printout.GetDialogInfo(data.MinPage, data.MaxPage,
                         data.FromPage, data.ToPage);

  // Create/Initialize Preview Pages
  //
  Page1 = GetNewPreviewPage(Client, Printout, *PrnDC, PrinterPageSize,
                            data.FromPage > 0 ? data.FromPage : 1);
  Page1->Create();
  Page2 = 0;

  LayoutPages();

/*
  data.FromPage = 1;
  data.ToPage = 1;
  data.MinPage = 1;
  data.MaxPage = 1;
*/
}

//
/// Returns a pointer to a TPreviewPage object.
/// \note This default version simply returns a 'true' TPreviewPage. However,
/// derived TPreviewWin classes can return a more sophisticated preview page (e.g.,
/// one that can handle zooming).
//
TPreviewPage*
TPreviewWin::GetNewPreviewPage(TWindow* parent, TPrintout& printout,
                               TPrintDC&  prndc, TSize& printExtent,
                               int pagenum)
{
  return new TPreviewPage(parent, printout, prndc, printExtent, pagenum);
}

//
/// Repositions the preview page(s) using the aspect ratio of the printer when
/// determining the dimensions of the pages.
//
void
TPreviewWin::LayoutPages()
{
  TLayoutMetrics metrics1;

  metrics1.X.Set(lmLeft, lmRightOf, lmParent, lmLeft, 15);
  metrics1.Y.Set(lmTop, lmBelow, lmParent, lmTop, 15);

  //
  // Determine major axis of preview page, have that follow parent size.
  // Make minor axis a percentage (aspect ratio) of the page's major axis
  //
  TRect r = Client->GetClientRect();
  long ratio;

  if (PrinterPageSize.cx > PrinterPageSize.cy)
    ratio = ((long)PrinterPageSize.cy * 100) / PrinterPageSize.cx;
  else
    ratio = ((long)PrinterPageSize.cx * 100) / PrinterPageSize.cy;

  bool xMajor = (((r.Width() * ratio) / 100) > r.Height());
  if (xMajor) {
    metrics1.Height.Set(lmBottom, lmAbove, lmParent, lmBottom, 15);
    metrics1.Width.PercentOf(Page1, (int)((long)PrinterPageSize.cx * 95 / PrinterPageSize.cy), lmHeight);
  }
  else {
    metrics1.Height.PercentOf(Page1, (int)((long)PrinterPageSize.cy * 95 / PrinterPageSize.cx), lmWidth);
    metrics1.Width.Set(lmRight, lmLeftOf, lmParent, lmRight, 15);
  }

  Client->SetChildLayoutMetrics(*Page1, metrics1);

  if (Page2) {
    TLayoutMetrics metrics2;

    metrics2.X.Set(lmLeft, lmRightOf, Page1, lmRight, 30);
    metrics2.Y.SameAs(Page1, lmTop);

    // Assume portrait
    //
    metrics2.Width.SameAs(Page1, lmWidth);
    metrics2.Height.SameAs(Page1, lmBottom);

    Client->SetChildLayoutMetrics(*Page2, metrics2);
  }

  Client->Layout();
}

//
// Update the information displayed on the control bar of the 
// preview frame window.
// NOTE: This method simply updates the page info. Derived classes
//       can display additional information.
//
void 
TPreviewWin::UpdateSpeedBar()
{
  // Update the page count.
  //
  TTextGadget* pgGadget = TYPESAFE_DOWNCAST(
                                 PreviewSpeedBar->GadgetWithId(IDB_CURRPAGE),
                                 TTextGadget);
  if (pgGadget) {
    tchar buffer[32];
#if BI_MSG_LANGUAGE == 0x0411
      sprintf(buffer, "Íß°¼Þ %d - %d", Page1->GetPageNumber(),
                                       Page2->GetPageNumber());
    else
      sprintf(buffer, "Íß°¼Þ %d", Page1->GetPageNumber());
#else
    if (Page2 && Page2->GetPageNumber())
      _stprintf(buffer, _T("Page %d - %d"), Page1->GetPageNumber(),
                                      Page2->GetPageNumber());
    else
      _stprintf(buffer, _T("Page %d"), Page1->GetPageNumber());
#endif
    pgGadget->SetText(buffer);
  }
}

//
// Command enabler of 'PREVIOUS' command
//
void
TPreviewWin::CePrevious(TCommandEnabler& ce)
{
  // Only have previous on if we're not at the first page.
  //
  ce.Enable(Page1->GetPageNumber() > 1);
}

//
// Command enabler of 'NEXT' command.
//
void
TPreviewWin::CeNext (TCommandEnabler& ce)
{
  // Only have next on if we're not at the last page.
  //
  TPrintDialog::TData &printerData = Printer.GetSetup();
  ce.Enable(printerData.ToPage < printerData.MaxPage);
}

//
// Handler of 'PREVIOUS' command
//
void
TPreviewWin::CmPrevious()
{
  TPrintDialog::TData &printerData = Printer.GetSetup();

  if (printerData.FromPage > printerData.MinPage) { 
    printerData.FromPage--;
    printerData.ToPage--;

    Page1->SetPageNumber(printerData.FromPage);
    if (Page2)
      Page2->SetPageNumber(printerData.ToPage);
  }

  UpdateSpeedBar();
}

//
// Handler of 'NEXT' command
//
void 
TPreviewWin::CmNext()
{
  TPrintDialog::TData &printerData = Printer.GetSetup();

  if (printerData.ToPage < printerData.MaxPage) {
    printerData.FromPage++;
    printerData.ToPage++;

    Page1->SetPageNumber(printerData.FromPage);
    if (Page2)
      Page2->SetPageNumber(printerData.ToPage);
  }
  UpdateSpeedBar();
}

//
// Handler of request for only one preview page.
//
void 
TPreviewWin::CmOneUp ()
{
  if (Page2) {
    Client->RemoveChildLayoutMetrics(*Page2);

    delete Page2;
    Page2 = 0;

    Client->Layout();

    TPrintDialog::TData &printerData = Printer.GetSetup();
    printerData.ToPage = printerData.FromPage;

    UpdateSpeedBar();
  }
}

//
// Command enabler of request for two preview pages.
//
void 
TPreviewWin::CeTwoUp(TCommandEnabler& ce)
{
  // Two up is only available for portrait mode.
  //
  ce.Enable(PrinterPageSize.cx <= PrinterPageSize.cy);
}

//
// Handler of request for two preview pages
//
void 
TPreviewWin::CmTwoUp()
{
  if (Page2 == 0) {
    Page2 = GetNewPreviewPage(Client, Printout, *PrnDC, 
                              PrinterPageSize, 
                              Page1->GetPageNumber() + 1);
    Page2->Create();

    TPrintDialog::TData &printerData = Printer.GetSetup();

    // Page 2 is the next page.  If the next page is outside of our range then
    // set the first page back one and the 2nd page is the current page.  If
    // the document is only 1 page long then the 2nd page is empty.
    //
    if (printerData.FromPage == printerData.MaxPage) {
      if (printerData.FromPage > 1) {
        printerData.FromPage--;
        printerData.ToPage = printerData.FromPage + 1;
        Page1->SetPageNumber(printerData.FromPage);
        Page2->SetPageNumber(printerData.ToPage);
      } 
      else {
        Page2->SetPageNumber(0);
      }
    } 
    else {
      printerData.ToPage = printerData.FromPage + 1;
      Page2->SetPageNumber(printerData.ToPage);
    }

    LayoutPages();
    UpdateSpeedBar();
  }
}

//
// Handler of request to terminate preview session
//
void 
TPreviewWin::CmDone()
{
  CloseWindow();
}


} // OWL namespace
/* ========================================================================== */

