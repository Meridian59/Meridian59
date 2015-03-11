//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
// 
/// \file
/// Implementation of support classes for Print/PrintPreview of TRichEdits
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/window.h>
#include <owl/control.h>
#include <owl/edit.h>
#include <owl/editsear.h>
#include <owl/editfile.h>
#include <owl/richedit.h>
#include <owl/richedpr.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;

//
/// Constructs a Printout object which represent a RICHEDIT's document.
//
TRichEditPrintout::TRichEditPrintout(TPrinter&  printer,
                                     TRichEdit& richEdit,
                                     LPCTSTR    title)
:
  TPrintout(title),
 Printer(printer),
  RichEdit(richEdit),
  SizePhysPage(0),
  SizePhysInch(0),
  Margins(0),
  FlushCache(false),
  TextLen(0),
  PageCount(0),
  PageIndices(5)
{
  PRECONDITION(HWND(RichEdit));

  // Initialize data members
  //
  PageIndices.Add(0);
}

//
/// String-aware overload
//
TRichEditPrintout::TRichEditPrintout(TPrinter&  printer, TRichEdit& richEdit, const tstring& title)
:
  TPrintout(title),
  Printer(printer),
  RichEdit(richEdit),
  SizePhysPage(0),
  SizePhysInch(0),
  Margins(0),
  FlushCache(false),
  TextLen(0),
  PageCount(0),
  PageIndices(5)
{
  PRECONDITION(HWND(RichEdit));

  // Initialize data members
  //
  PageIndices.Add(0);
}

//
// Destructor of 'RichEdit PrintOut' - Flushes any cached formatting
// information
//
TRichEditPrintout::~TRichEditPrintout()
{
  // Flush cached format information
  //
  if (FlushCache) {
    CHECK(HWND(RichEdit));
    RichEdit.FormatRange();
  }
}

//
/// This method is invoked by the printer or print-preview classes to allow the
/// printout to update the page range information.
//
void
TRichEditPrintout::GetDialogInfo(int& minPage, int& maxPage,
                                 int& selFromPage, int& selToPage)
{
  if (PageCount && TextLen) {
    minPage = 1;
    maxPage = PageCount;

    CHECK(HWND(RichEdit));
    int startSel, endSel;
    RichEdit.GetSelection(startSel, endSel);

    // !BB Update to support multiple preview pages
    //
    selFromPage = startSel != endSel ? PageOfOffset(startSel) : 1;
    selToPage = PageCount > selFromPage ? selFromPage+1 : 0;
  }
  else {
    minPage = maxPage = 0;
    selFromPage = selToPage = 0;
  }
}

//
/// This method is invoked by the printer or print-preview objects to hand the
/// printout object the target device context and the size of its pages.
//
void
TRichEditPrintout::SetPrintParams(TPrintDC* dc, TSize pageSize)
{
  PRECONDITION(HWND(RichEdit));

  // Chain call base class' version
  //
  TPrintout::SetPrintParams(dc, pageSize);

  // Update size info about printer DC
  //
  SizePhysInch = TSize(dc->GetDeviceCaps(LOGPIXELSX),
                       dc->GetDeviceCaps(LOGPIXELSY));
  SizePhysPage = TSize(dc->GetDeviceCaps(PHYSICALWIDTH),
                       dc->GetDeviceCaps(PHYSICALHEIGHT));
  if (!SizePhysPage.cx)
    SizePhysPage.cx = dc->GetDeviceCaps(HORZRES);
  if (!SizePhysPage.cy)
    SizePhysPage.cy = dc->GetDeviceCaps(VERTRES);


  // Set up the target and render DCs
  // NOTE: In the case of PrintPreview, the two HDCs differ.
  //       When we're really printing, the attribute DC and
  //       GetHDC() return the same HDC.
  //
  FmtRange.SetTargetDC(dc->GetAttributeHDC()); // Format Device
  FmtRange.SetRenderDC(dc->GetHDC());          // Render Device

  // Set the page and render rectangles in twips 
  // Handle margins, if requested.
  //
  TRect rect(0, 0, MulDiv(SizePhysPage.cx, 1440, SizePhysInch.cx),
                   MulDiv(SizePhysPage.cy, 1440, SizePhysInch.cy));
  if (Margins.cx) {
    ;
  }
  if (Margins.cy) {
    ;
  }
  FmtRange.SetPageRect(rect);
  FmtRange.SetRenderRect(rect);

  // Retrieve length of document and set range
  //
  TextLen = RichEdit.GetTextLength();

  // Compute number of pages and cache their offsets
  //
  if (TextLen > 0) {
    PageCount = 1;                  // Pages are not zero-based
    FmtRange.SetRange(0, TextLen);  // Format entire range.....
    int endOfRange;
    do {
      endOfRange = RichEdit.FormatRange(FmtRange, false);

      if(PageCount >= (int)PageIndices.Size())
        PageIndices.Add(endOfRange);
      else
        PageIndices[PageCount++] = endOfRange;

      FmtRange.chrg.cpMin = endOfRange;
      FmtRange.SetRenderRect(rect); // Ctl seems to corrupt the 'rc' member!
    } while (FmtRange.chrg.cpMin < TextLen);

    PageCount--;

    // Free cached formatting information
    //
    RichEdit.FormatRange();
  } 
  else {
    PageCount = 0;
  }
}

//
/// This routine is invoked to inform the printout that a printing operation has started.
//
void 
TRichEditPrintout::BeginPrinting()
{
}

//
/// This routine is invoked to inform the printout that the printint operation has ended.
//
void
TRichEditPrintout::EndPrinting()
{
}

//
/// This routine is invoked to request the printout object to print a page.
//
void 
TRichEditPrintout::PrintPage (int page, TRect& bandRect, uint)
{
  PRECONDITION(page > 0);
  PRECONDITION(page <= PageCount);

  // Check control's handle is !0
  //
  PRECONDITION(HWND(RichEdit));

  // These should have been set via a prior call to 'SetPrintParams' 
  //
  PRECONDITION(FmtRange.hdcTarget !=0);
  PRECONDITION(FmtRange.hdc != 0);

  // Check whether we're in 'real' printing mode or just print preview.
  // In print preview mode the target does not match the render DC.....
  //
  if (FmtRange.hdc != FmtRange.hdcTarget) {

    int saveId = SaveDC(FmtRange.hdc);
    if (FmtRange.hdc) {
      TDC dc(FmtRange.hdc);

      // Make window DC match target's logical size
      //
      dc.SetMapMode(MM_ANISOTROPIC);
      dc.SetWindowExt(TSize(MulDiv(SizePhysPage.cx,
                                  dc.GetDeviceCaps(LOGPIXELSX), 
                                  SizePhysInch.cx),
                            MulDiv(SizePhysPage.cy, 
                                  dc.GetDeviceCaps(LOGPIXELSY), 
                                  SizePhysInch.cy)));
      dc.SetViewportExt(TSize(bandRect.Width(), bandRect.Height()));

      // Update character range for specified page
      //
      FmtRange.SetRange(PageIndices[page-1], PageIndices[page]);

      // Have Edit control draw into window
      //
      RichEdit.FormatRange(FmtRange);

      // Flag that we need to reset control
      //
      FlushCache = true;
    }
    // Restore the DC
    //
    RestoreDC(FmtRange.hdc, saveId);
  }
  else {
    // We're really printing! Update character range for specified page
    //
    FmtRange.SetRange(PageIndices[page-1], PageIndices[page]);

    // Have Edit control format data
    //
    RichEdit.FormatRange(FmtRange, false);

    // Flag that we need to reset control
    //
    FlushCache = true;

    // Have Edit control display to render rectangle of device
    //
    RichEdit.DisplayBand(*((TRect*)&FmtRange.rc));
  }
}

//
/// This routine is invoked to asked the printout object whether it has necessary
/// information to print the specified page.
//
bool 
TRichEditPrintout::HasPage (int pageNumber)
{
  return pageNumber > 0 && pageNumber <= PageCount;
}

//
/// Returns the index of the page at the particular offset within the buffer of an
/// edit control.
//
int
TRichEditPrintout::PageOfOffset(int offset)
{
  PRECONDITION(offset <= TextLen);
  
  int page = 0;
  if (PageCount > 0) {
    for (int i=1; i<=PageCount; i++) {
      if (PageIndices[i] >= offset) {
        if (PageIndices[i-1] <= offset) {
          page = i;
          break;
        }
      }    
    }    
  }
  return page;
}

//
/// Constructor of RichEdit PagePreview object.
//
TRichEditPagePreview::TRichEditPagePreview(TWindow* parent, 
                                           TPrintout& printout,
                                           TPrintDC& prndc,
                                           TSize& printExtent,
                                           int pagenum)
:
  TPreviewPage(parent, printout, prndc, printExtent, pagenum)
{
}

//
/// WM_PAINT handler of RichEdit PagePreview window. Displays a preview of the page
/// if the printout can handle it. Otherwise, simply fills the window with a white
/// background.
//
void
TRichEditPagePreview::Paint(TDC& dc, bool, TRect& /*clip*/) 
{
  TRect client;
  GetClientRect(client);

  TPreviewDCBase pdc(dc, PrintDC);
  Printout.SetPrintParams(&pdc, PrintExtent);

  if (Printout.HasPage(PageNum)) {
    Printout.BeginPrinting();
    Printout.BeginDocument(PageNum, PageNum, pfBoth);
    Printout.PrintPage(PageNum, client, pfBoth);
    Printout.EndDocument();
    Printout.EndPrinting();
  }
  else
    dc.PatBlt(client, WHITENESS);
}

//
/// Constructor of a RichEdit Preview Frame.
//
TRichEditPreviewFrame::TRichEditPreviewFrame(TWindow* parentWindow, 
                                             TPrinter& printer, 
                                             TPrintout& printout, 
                                             TRichEdit& richEdit, 
                                             LPCTSTR title, 
                                             TLayoutWindow* client)
:
  TPreviewWin(parentWindow, printer, printout, richEdit, title, client)
{
}

//
/// String-aware overload
//
TRichEditPreviewFrame::TRichEditPreviewFrame(
  TWindow* parentWindow, 
  TPrinter& printer, 
  TPrintout& printout, 
  TRichEdit& richEdit, 
  const tstring& title, 
  TLayoutWindow* client
  )
  : TPreviewWin(parentWindow, printer, printout, richEdit, title, client)
{}

//
/// Returns pointer to a preview page object.
//
TPreviewPage* 
TRichEditPreviewFrame::GetNewPreviewPage(TWindow* parent, 
                                         TPrintout& printout,
                                         TPrintDC&  prndc,
                                         TSize&     printExtent,
                                         int        pagenum)
{
  return new TRichEditPagePreview(parent, printout, prndc, 
                                  printExtent, pagenum);
}


} // OWL namespace
/* ========================================================================== */

