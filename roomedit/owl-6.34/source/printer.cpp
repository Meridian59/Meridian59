//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1997 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/applicat.h>
#include <owl/appdict.h> 
#include <owl/window.h>
#include <owl/framewin.h>
#include <owl/dc.h>
#include <owl/static.h>
#include <owl/printer.h>
#include <stdio.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;


//
// Template used to set and clear a bunch of flags at once
//
template <class T1, class T2>
inline void SetClear(T1& var, T2 bitsOn, T2 bitsOff) {
  var &= ~bitsOff;
  var |=  bitsOn;
}

// Define 'MANUAL_ABORT_CALL to enable the explicit abort proc call
//
// #define MANUAL_ABORT_CALL   


//----------------------------------------------------------------------------

//
// class TFormattedStatic
// ~~~~~ ~~~~~~~~~~~~~~~~
// Static control that uses its resource title as a printf format string to
// format one or two text strings provided in the constructor
//
class TFormattedStatic: public TStatic {
  public:
    TFormattedStatic(TWindow* parent, int resId, const tstring& text, const tstring& text2 = tstring());

  protected:
    void SetupWindow();

  private:
    tstring Text;    // Text to display
    tstring Text2;
};

//
// Construct the object.
// Copy the two strings passed.
// Second string is optional.
//
TFormattedStatic::TFormattedStatic(TWindow* parent, int resId, const tstring& text, const tstring& text2)
: TStatic(parent, resId, 0),
  Text(text),
  Text2(text2)
{}

//
// Override SetupWindow to set the text for the static control.
//
void
TFormattedStatic::SetupWindow()
{
  TStatic::SetupWindow();

  // Use the text retrieved from the resource as a printf template for
  // the one or two text strings, then update the control text with the result.
  //
  tstring c = GetText();
  int len = c.length() + Text.length() + Text2.length() + 5;
  TAPointer<tchar> buff(new tchar[len]);
  _stprintf(buff, c.c_str(), Text.c_str(), Text2.c_str());
  SetText(buff);
}

//
// class TNumericStatic
// ~~~~~ ~~~~~~~~~~~~~~
// Static control that uses its resource title as a printf format string to
// format one or two text strings provided in the constructor
//
class TNumericStatic: public TStatic 
{
  public:
    TNumericStatic(TWindow* parent, int resId, int number);

  protected:
    TResult EvSetNumber(TParam1, TParam2);

  private:
    int Number;         // Number to display

  DECLARE_RESPONSE_TABLE(TNumericStatic);
};

#define WM_SETNUMBER    WM_USER+100

DEFINE_RESPONSE_TABLE1(TNumericStatic, TStatic)
  EV_MESSAGE(WM_SETNUMBER, EvSetNumber),
END_RESPONSE_TABLE;

//
// Construct the object and remember the number to display.
//
TNumericStatic::TNumericStatic(TWindow* parent, int resId, int number)
: TStatic(parent, resId, 0),
  Number(number)
{}

//
// Handle our user defined message to set the number displayed in the %d part
// of the Title format string. If the number is <= 0, then hide this window
//
TResult
TNumericStatic::EvSetNumber(TParam1 param1, TParam2)
{
  Number = param1;
  if (Number > 0) {
    LPCTSTR c = GetCaption();
    CHECK(c);
    if (c)
    {
      int len = ::_tcslen(c) + sizeof(Number) + 5;
      TAPointer<tchar> buff(new tchar[len]);
      wsprintf(buff, c, Number);
      SetText(buff);
    }
  }
  THandle hWndDefault = GetParentO()->GetDlgItem(-1);
  if (Number > 0) 
  {
    const uint flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW;
    SetWindowPos(HWND_TOP, 0, 0, 0, 0, flags);
    if (hWndDefault)
      ::ShowWindow(hWndDefault, SW_HIDE);
    UpdateWindow();
  }
  return 0;
}


//----------------------------------------------------------------------------

DEFINE_RESPONSE_TABLE1(TPrinterAbortDlg, TDialog)
  EV_COMMAND(IDCANCEL, CmCancel),
END_RESPONSE_TABLE;


//
/// Constructs an Abort dialog box that contains a Cancel button and displays the
/// given title, device, and port.
//
TPrinterAbortDlg::TPrinterAbortDlg(TWindow* parent, TResId resId, const tstring& title, const tstring& device, const tstring& port, HDC prnDC)
:
  TDialog(parent, resId),
  PrnDC(prnDC)
{
  new TNumericStatic(this, ID_PAGE, 0);
  new TFormattedStatic(this, ID_TITLE, title);
  new TFormattedStatic(this, ID_DEVICE, device, port);
  new TNumericStatic(this, ID_TOPAGE, 0);
}

//
/// Overrides SetupWindow. This function disables the Close system menu option.
//
void
TPrinterAbortDlg::SetupWindow()
{
  // Ignore missing controls, resource may be different.
  //
  // While needlessly flexible, previous versions of OWL allowed a derived 
  // dialog to ommit the controls required by this base dialog class, and/or 
  // to redefine the control identifiers. 
  //
  // Note that doing so will cause the dialog to lack the corresponding
  // functionality of these controls, as the printing code relies on these
  // controls being present with the identifiers defined by the 
  // IDD_ABORTDIALOG dialog resource (i.e. ID_PAGE and ID_TOPAGE).
  // Otherwise no page number progress will be reported by TPrinter::Print.
  //
  // The updated code below still allows redefinition for backwards 
  // compatibility, but it logs a warning and uses more stringent exception 
  // handling to filter out exceptions caused by other issues.
  // The original code used a "catch all" handler that did nothing.
  //
  // Note that this code depends on our controls being last in the children
  // list. This can be assumed since TWindow::AddChild inserts children at
  // the front of the list, i.e. any additional controls defined by a subclass
  // will have already been constructed if and when our controls should fail to 
  // create due to a missing or redefined resource. Hence the state of the 
  // derived class is unaffected in this case, and we can safely ignore the
  // failed controls. It is also vital that no code relies on these controls
  // to function. TPrinter::Print currently passes messages to these controls
  // but doesn't rely on that succeeding.
  //
  try {
    TDialog::SetupWindow();
  }
  catch (const TXWindow& x) {
    const TWindow* w = x.GetWindow();
    int id = w ? w->GetId() : 0; 
    switch (id) {
      case ID_PAGE:
      case ID_TITLE:
      case ID_DEVICE:
      case ID_TOPAGE:
        WARN(true, _T("TPrinterAbortDlg control failure [id=") << id << _T("]: ") << x.what());
        break; // Do nothing. Let these controls fail silently.

      default:
        throw; // The cause is unknown. Rethrow the exception.
    }
  } 
  EnableMenuItem(GetSystemMenu(false), SC_CLOSE, MF_GRAYED);
}

//
/// Handles the print-cancel button by setting the user print abort flag in the
/// application.
//
void
TPrinterAbortDlg::CmCancel()
{
  TPrinter::SetUserAbort(PrnDC);
  THandle hBtn = GetDlgItem(IDCANCEL);
  if (hBtn)
    ::EnableWindow(hBtn, false);
}

//----------------------------------------------------------------------------

//
/// Constructs a TXPrinter object with a default IDS_PRINTERERROR message.
//
TXPrinter::TXPrinter(uint resId)
:
  TXOwl(resId)
{
}

//
/// Clone the exception object for safe throwing across stack frames.
//
TXPrinter*
TXPrinter::Clone() const
{
  return new TXPrinter(*this);
}

//
/// Throws the exception.
//
void
TXPrinter::Throw()
{
  throw *this;
}

//
/// Creates the exception object and throws it.
//
void
TXPrinter::Raise(uint resId)
{
  TXPrinter(resId).Throw();
}

//----------------------------------------------------------------------------

//
/// Constructs a TXPrinting object with a default IDS_PRINTERERROR message.
//
TXPrinting::TXPrinting(int error)
:
  TXOwl(IDS_PRINTERERROR), // TODO: Create specific string for this exception?
  Error(error)
{
}

//
/// Clones the exception object for safe throwing across stack frames.
//
TXPrinting*
TXPrinting::Clone() const
{
  return new TXPrinting(*this);
}

//
/// Throws the exception.
//
void
TXPrinting::Throw()
{
  throw *this;
}

//
/// Creates the exception object and throws it.
//
void
TXPrinting::Raise(int error)
{
  TXPrinting(error).Throw();
}

//
/// Returns the error message for the current error code.
/// If the given module is null, the message is loaded from the global module.
//
tstring
TXPrinting::GetErrorMessage(TModule* m) const
{
  TModule& module = m ? *m : GetGlobalModule();
  uint errorMsgId;
  switch (Error) 
  {
    case SP_APPABORT:
      errorMsgId = IDS_PRNCANCEL;
      break;
    case SP_USERABORT:
      errorMsgId = IDS_PRNMGRABORT;
      break;
    case SP_OUTOFDISK:
      errorMsgId = IDS_PRNOUTOFDISK;
      break;
    case SP_OUTOFMEMORY:
      errorMsgId = IDS_PRNOUTOFMEMORY;
      break;
    case SP_ERROR:
    default:
      errorMsgId = IDS_PRNGENERROR;
      break;
  }
  return module.LoadString(errorMsgId);
}

//----------------------------------------------------------------------------

//
/// Set by printing dialog if user cancels.
//
HDC TPrinter::UserAbortDC = 0;

//
/// Constructs an instance of TPrinter associated with the default printer. To
/// change the printer, call SetDevice after the object has been initialized or call
/// Setup to let the user select the new device through a dialog box.
//
TPrinter::TPrinter(TApplication* app)
: Error(0),
  Data(new TPrintDialog::TData),
  Application(app ? app : OWLGetAppDictionary().GetApplication(0)),
  OutputFile()
{
  CHECK(Application);

  // Initialize with user's default printer.
  // NOTE: GetDefaultPrinter is virtual. However, we're currently
  //       in the constructor. Hence, TPrinter's version will be called
  //       in this case even for derived classes which override that
  //       method
  //
  GetDefaultPrinter();
}

//
/// Frees the resources allocated to TPrinter.
//
TPrinter::~TPrinter()
{
  delete Data;
}

//
/// Returns a reference to the TPrintDialog data structure.
//
TPrintDialog::TData& TPrinter::GetSetup() 
{
  return *Data;
}

//
/// Sets the user abort DC for the printer.
//
void TPrinter::SetUserAbort(HDC abortDC)
{
  UserAbortDC = abortDC;
}

//
/// Returns the abort DC.
//
HDC TPrinter::GetUserAbort()
{
  return UserAbortDC;
}

//
/// Returns the error code from the printer.
//
int TPrinter::GetError()
{
  return Error;
}

//
/// Returns the common dialog data associated with the printer.
//
TPrintDialog::TData* TPrinter::GetData()
{
  return Data;
}

//
/// Sets the common dialog data; takes ownership.
//
void TPrinter::SetData(TPrintDialog::TData* data)
{
  if (data == Data) return;
  if (Data) delete Data;
  Data = data;
}

//
/// Returns the application pointer.
//
TApplication* TPrinter::GetApplication()
{
  return Application;
}

//
/// Sets the application pointer.
//
void TPrinter::SetApplication(TApplication* app)
{
  Application = app;
}

//
/// Returns the size of the page.
//
TSize TPrinter::GetPageSize() const
{
  return PageSize;
}

//
/// Sets the page's size.
//
void TPrinter::SetPageSize(const TSize& pagesize)
{
  PageSize = pagesize;
}

//
/// Returns the size of an inch the page.
//
TSize TPrinter::GetPageSizeInch() const
{
  return PageSizeInch;
}

//
/// Sets the size of an inch on the page.
//
void TPrinter::SetPageSizeInch(const TSize& pageSizeInch)
{
  PageSizeInch = pageSizeInch;
}

//
/// Called by the Destructor, ClearDevice disassociates the device
/// with the current printer. ClearDevice changes the current status of the printer
/// to PF_UNASSOCIATED, which causes the object to ignore all calls to Print until
/// the object is reassociated with a printer.
//
void TPrinter::ClearDevice()
{
  Data->ClearDevMode();
  Data->ClearDevNames();
}

//
/// Updates the printer structure with information about the user's default printer.
//
void
TPrinter::GetDefaultPrinter()
{
  //
  // Have a printDialog go get the default printer information for us
  // into DevMode and DevNames. We never actually Execute() the dialog.
  //
  SetClear(Data->Flags, 0L, long(PD_RETURNDC));
  TPrintDialog printDialog(Application->GetMainWindow(), *Data);
  if (!printDialog.GetDefaultPrinter()) {
    Data->Error = PDERR_NODEFAULTPRN;
  }
///TH    TXOwl::Raise(IDS_PRNNODEFAULT, GetApplication());
///TH    TXGdi::Raise();
}

//
/// SetPrinter changes the printer device association. Setup calls SetPrinter to
/// change the association interactively. The valid parameters to this method can be
/// found in the [devices] section of the WIN.INI file.
/// Entries in the [devices] section have the following format:
/// \code
/// <device name>=<driver>, <port> { , <port>}
/// \endcode
void
TPrinter::SetPrinter(const tstring& driver, const tstring& device, const tstring& output)
{
  Data->SetDevNames(driver.c_str(), device.c_str(), output.c_str());
}

//
// Abort procedure used during printing, called by windows. Returns true to
// continue the print job, false to cancel.
//
int CALLBACK
TPrinterAbortProc(HDC hDC, int code)
{
  TApplication* appl = OWLGetAppDictionary().GetApplication(0);
  if(appl)
    appl->PumpWaitingMessages();

  // UserAbortDC will have been set by the AbortDialog
  //
  if (TPrinter::GetUserAbort() == hDC || TPrinter::GetUserAbort() == HDC(-1)) {
    TPrinter::SetUserAbort(0);
    return false;
  }
  return code == 0 || code == SP_OUTOFDISK;
}

//
/// Virtual called from within Print() to construct and execute a print dialog
/// before actual printing occurs. Return true to continue printing, false to
/// cancel
//
bool
TPrinter::ExecPrintDialog(TWindow* parent)
{
  return TPrintDialog(parent, *Data).Execute() == IDOK;
}

//
// Page setup dialog for Win95 support.
//
#if !defined(__GNUC__) //JJH added removal of pragma warn for gcc
#pragma warn -par
#endif

/// Page setup dialog for Win95 support.
bool
TPrinter::ExecPageSetupDialog(TWindow* parent)
{
  Data->DoPageSetup = true;
  bool ret = TPrintDialog(parent, *Data).Execute() == IDOK;
  Data->DoPageSetup = false;
  return ret;
}

#if !defined(__GNUC__) //JJH added removal of pragma warn for gcc
#pragma warn .par
#endif


//
/// Virtual called from withing Print() just before the main printing loop to
/// construct and create the printing status, or abort window. This window
/// should use the control IDs specified in printer.rh
//
TWindow*
TPrinter::CreateAbortWindow(TWindow* parent, TPrintout& printout)
{
  TDC* dc = printout.GetPrintDC();
  TWindow* win = new TPrinterAbortDlg(parent, IDD_ABORTDIALOG,
                                      printout.GetTitle(),
                                      Data->GetDeviceName(),
                                      Data->GetOutputName(),
                                      dc ? HDC(*dc) : HDC(-1));
  win->Create();
  return win;
}

//
/// Returns the filename for output redirection.
//
LPCTSTR
TPrinter::GetOutputFile() const 
{
  return OutputFile.c_str();
}

// 
/// Sets the filename for output redirection.
//
void 
TPrinter::SetOutputFile(const tstring& outputFile) 
{
  OutputFile = outputFile;
}

//
/// Updates the PageSize variables by querying the device capabilities of the
/// specified device context.
//
void
TPrinter::SetPageSizes(const TPrintDC& prnDC)
{
  // !BB Should we try PHYSICALWIDTH and PHYSICALHEIGHT first and then
  // !BB fallback on HORZRES and VERTRES
  PageSize.cx = prnDC.GetDeviceCaps(HORZRES);
  PageSize.cy = prnDC.GetDeviceCaps(VERTRES);
  PageSizeInch.cx = prnDC.GetDeviceCaps(LOGPIXELSX);
  PageSizeInch.cy = prnDC.GetDeviceCaps(LOGPIXELSY);
}

namespace
{

  struct TPrintErrorHandler
  {
    friend int operator %(int r, const TPrintErrorHandler&)
    {
      if (r <= 0) TXPrinting::Raise(r);
      return r;
    }
  };

  //
  // Calculate flags based on band info from the driver.
  // If a driver does not support BANDINFO the Microsoft recommended way
  // of determining text only bands is if the first band is the full page,
  // all others are graphics only or both.
  //
  uint CalculateBandFlags(TPrintout& printout, const TRect& bandRect)
  {
    PRECONDITION(printout.GetPrintDC());
    TPrintErrorHandler eh;

    TPrintDC& dc = *printout.GetPrintDC();
    if (!ToBool(dc.QueryEscSupport(BANDINFO))) 
    {
      const TRect pageRect(TPoint(0, 0), printout.GetPageSize());
      return (bandRect == pageRect) ? pfText : pfGraphics;
    }
    TBandInfo bandInfo;
    dc.BandInfo(bandInfo) %eh;
    return (bandInfo.HasGraphics ? pfGraphics : 0) | 
        (bandInfo.HasText ? pfText : 0);
  }

  //
  // Prints a page using the deprecated banding mechanism.
  // TODO: Review the necessity of this code.
  //
  void PrintBandPage(TPrintout& printout, int pageNum)
  {
    PRECONDITION(printout.GetPrintDC());
    TPrintErrorHandler eh;
    TPrintDC& dc = *printout.GetPrintDC();
    TRect bandRect(TPoint(0, 0), printout.GetPageSize());
    while (!bandRect.IsEmpty())
    { 
      dc.NextBand(bandRect) %eh;
      uint bandFlags = CalculateBandFlags(printout, bandRect);
      if (printout.WantForceAllBands() && (bandFlags & pfBoth) == pfGraphics)
        dc.SetPixel(TPoint(0, 0), TColor::Black); // Some old drivers need this.
      dc.DPtoLP(bandRect, 2);
      printout.PrintPage(pageNum, bandRect, bandFlags);
    }
  }

  // 
  // Sets up the device context and forwards the call to the printout.
  //
  void PrintPlainPage(TPrintout& printout, int pageNum)
  {
    PRECONDITION(printout.GetPrintDC());
    TPrintErrorHandler eh;
    TPrintDC& dc = *printout.GetPrintDC();
    dc.StartPage() %eh;
    TRect pageRect(TPoint(0, 0), printout.GetPageSize());
    printout.PrintPage(pageNum, pageRect, pfBoth);
    dc.EndPage() %eh;
  }

  //
  // Function type for the inner body of the print loop
  //
  typedef void (*TPageFunc)(TPrintout&, int pagenum);

  // 
  // Exception-safe begin and end code for a print job
  //
  struct TPrintingScope
  {
    TWindow& parent;
    TPrintout& printout;

    TPrintingScope(TWindow& p, TPrintout& po) : parent(p), printout(po) 
    {
      parent.EnableWindow(false);
      printout.BeginPrinting();
    }

    ~TPrintingScope() 
    {
      printout.EndPrinting();
      parent.EnableWindow(true);
    }
  };

  // 
  // Exception-safe begin and end code for printing a document
  //
  struct TDocumentScope
  {
    TPrintout& printout;

    TDocumentScope(TPrintout& p, int fromPage, int toPage) : printout(p) 
    {
      printout.BeginDocument(fromPage, toPage, pfBoth); 
    }

    ~TDocumentScope() 
    {
      printout.EndDocument();
    }
  };

  //
  // Prints all the pages in the given document.
  // If copies are requested, then repeatedly prints the document (for collated copies) or 
  // pages (for non-collated copies) for the number of copies specified.
  //
  // TODO: Review the calling sequence in case of exceptions. The code currently calls TPrintout::EndDocument and 
  // TPrintout::EndPrinting even if printing fails (exceptions). On the other hand, TPrintDC::EndPage and 
  // TPrintDC::EndDoc are not called if printing fails (exceptions). This is consistent with the original code, 
  // but the logic of this should be reviewed.
  //
  void PrintLoop(TPrintout& printout, int fromPage, int toPage, int copies, bool collated, LPCTSTR out,
    TWindow& abortWin, TPageFunc printPage)
  {
    PRECONDITION(printout.GetPrintDC());
    PRECONDITION(abortWin.GetParentO());
    PRECONDITION(fromPage <= toPage);
    PRECONDITION(copies >= 0);
    TPrintErrorHandler eh;
    TPrintDC& dc = *printout.GetPrintDC();
    TPrintingScope printingScope(*abortWin.GetParentO(), printout);

    const int documentCopyCount = collated ? copies : 1;
    for (int documentCopyIndex = 0; documentCopyIndex != documentCopyCount; ++documentCopyIndex)
    {
      TDocumentScope documentScope(printout, fromPage, toPage);

      dc.StartDoc(printout.GetTitle(), out) %eh;
      abortWin.SendDlgItemMessage(ID_TOPAGE, WM_SETNUMBER, toPage);
      const int pageCopyCount = collated ? 1 : copies;
      for (int pageNum = fromPage; pageNum <= toPage && printout.HasPage(pageNum); ++pageNum) 
      {
        abortWin.SendDlgItemMessage(ID_PAGE, WM_SETNUMBER, pageNum);
        for (int pageCopyIndex = 0; pageCopyIndex != pageCopyCount; ++pageCopyIndex)
          printPage(printout, pageNum);
      }
      dc.EndDoc() %eh;
    }
  }

} // namespace

//
/// Print renders the given printout object on the associated printer device and
/// displays an Abort dialog box while printing. It displays any errors encountered
/// during printing. Prompt allows you to show the user a commdlg print dialog.
///
/// Note: The calling sequence here is somewhat of a catch-22 for the printout.
/// The printout cannot compute how many pages the document really has until the page
/// format is known. The printout doesn't get this information until SetPrintParams is 
/// called, which cannot happen until the dialog has returned a device context. 
/// Unfortunately, the page range information must be provided by GetDialogInfo which 
/// has to be called first to set up the dialog.
///
/// The problem is that the printer/page options are in the same dialog as the page 
/// range selection. The only way to handle this is to implement a custom interactive 
/// dialog box that updates the page range in real-time depending on the selected
/// printer/page format. Or deactivate the page range altogether.
//
bool
TPrinter::Print(TWindow* parent, TPrintout& printout, bool prompt)
{
  PRECONDITION(parent);

  //
  // Get page range & selection range (if any) from the document.
  //
  int selFromPage = 0;
  int selToPage = 0;
  printout.GetDialogInfo(Data->MinPage, Data->MaxPage, selFromPage, selToPage);
  if (selFromPage != 0) 
  {
    Data->Flags &= ~PD_NOSELECTION;
    Data->FromPage = selFromPage;
    Data->ToPage = selToPage;
  }
  else {
    Data->Flags |= PD_NOSELECTION;
    Data->FromPage = 0;
    Data->ToPage = 999;
  }
  if (Data->MinPage != 0) 
  {
    Data->Flags &= ~PD_NOPAGENUMS;
    if (Data->FromPage < Data->MinPage)
      Data->FromPage = Data->MinPage;
    else if (Data->FromPage > Data->MaxPage)
      Data->FromPage = Data->MaxPage;
    if (Data->ToPage < Data->MinPage)
      Data->ToPage = Data->MinPage;
    else if (Data->ToPage > Data->MaxPage)
      Data->ToPage = Data->MaxPage;
  }
  else
    Data->Flags |= PD_NOPAGENUMS;

  //
  // Create & execute a TPrintDialog (or derived) and have it return a usable
  // DC if prompt is enabled. If the dialog fails because the default printer
  // changed, clear our device information & try again.
  //
  TPointer<TPrintDC> prnDC (0);  // Pointer to printer DC created by Printer Object
  if (prompt) 
  {
    SetClear(Data->Flags, PD_RETURNDC, PD_RETURNDEFAULT|PD_PRINTSETUP);
    bool ok = ExecPrintDialog(parent);
    if (!ok && Data->Error == PDERR_DEFAULTDIFFERENT) 
    {
      ClearDevice();
      ok = ExecPrintDialog(parent);
    }
    if (!ok)
      return false;
    prnDC = Data->TransferDC();   // We now own the DC, let prnDC manage it
    if (!prnDC)
      TXPrinter::Raise();
  }
  else 
  {
    // Construct the DC directly if prompting was not enabled.
    //
    prnDC = new TPrintDC(Data->GetDriverName(), Data->GetDeviceName(),
      Data->GetOutputName(), Data->GetDevMode());
  }

  // Update the device page format and forward the DC and page size to the printout.
  //
  SetPageSizes(*prnDC);
  printout.SetPrintParams(prnDC, GetPageSize());

  // Figure out which page range to use: Selection, Dialog's from/to, whole document
  // range or all possible pages.
  //
  int fromPage;
  int toPage;
  if (prompt && (Data->Flags & PD_SELECTION) || selFromPage) 
  {
    fromPage = selFromPage;
    toPage = selToPage;
  }
  else if (prompt && (Data->Flags & PD_PAGENUMS)) 
  {
    fromPage = Data->FromPage;
    toPage = Data->ToPage;
  }
  else if (Data->MinPage) 
  {
    fromPage = Data->MinPage;
    toPage = Data->MaxPage;
  }
  else 
  {
    fromPage = 1;
    toPage = INT_MAX;
  }

  // Redirect output if requested.
  //
  LPCTSTR out = OutputFile.length() == 0 ? 0 : OutputFile.c_str();

  // Only band if the user requests banding and the printer supports banding.
  // TODO: Banding is obsolete. Review the necessity of this code.
  //
  bool banding = printout.WantBanding() && (prnDC->GetDeviceCaps(RASTERCAPS) & RC_BANDING);

  // Create modeless abort dialog and start printing.
  //
  try
  {
    TPointer<TWindow> abortWin(CreateAbortWindow(parent, printout));
    prnDC->SetAbortProc(TPrinterAbortProc);
    SetUserAbort(0);
    PrintLoop(printout, fromPage, toPage, Data->Copies, Data->Flags & PD_COLLATE, out, *abortWin, 
      banding ? PrintBandPage : PrintPlainPage);
  }
  catch (const TXPrinting& x)
  {
    // Report error if not already reported.
    //
    Error = x.Error;
    if (x.Error & SP_NOTREPORTED)
      ReportError(parent, printout);
    return false;
  }
  return true;
}

//
/// Setup lets the user select and/or configure the currently associated printer.
/// Setup  opens a dialog box as a child of the given window. The user then selects
/// one of the buttons in the dialog box to select or configure the printer. The
/// form of the dialog box is based on  TPrintDialog, the common dialog printer
/// class.
/// The options button allows the user acces to the specific driver's options.
//
void TPrinter::Setup(TWindow* parent)
{
  ExecPageSetupDialog(parent);
}

//
/// Reports the current error by bringing up a system message box with an error message
/// composed of the error description and document title.
/// This function can be overridden to show a custom error dialog box.
//
void
TPrinter::ReportError(TWindow* parent, TPrintout& printout)
{
  PRECONDITION(parent);
  tstring errorCaption = parent->LoadString(IDS_PRNERRORCAPTION);
  tstring errorStr = TXPrinting(Error).GetErrorMessage(parent->GetModule());
  parent->FormatMessageBox(IDS_PRNERRORTEMPLATE, errorCaption, MB_OK | MB_ICONSTOP, printout.GetTitle(), errorStr.c_str());
}

//----------------------------------------------------------------------------


IMPLEMENT_STREAMABLE(TPrinter);

#if !defined(BI_NO_OBJ_STREAMING)

//
/// Restores the printer object from the persistent stream.
//
void*
TPrinter::Streamer::Read(ipstream& is, uint32 version) const
{
  GetObject()->Data->Read(is, version);
  return GetObject();
}

//
/// Saves the object into the persistent stream.
//
void
TPrinter::Streamer::Write(opstream& os) const
{
  GetObject()->Data->Write(os);
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)



} // OWL namespace
/* ========================================================================== */

