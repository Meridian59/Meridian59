//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TPrintDialog, a Print and PrintSetup common Dialog
/// encapsulation
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/printdia.h>
#include <owl/dc.h>
#include <owl/framewin.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
# pragma option -w-rch // Disable "Unreachable code"
#endif

namespace owl {

OWL_DIAGINFO;


DEFINE_RESPONSE_TABLE1(TPrintDialog, TCommonDialog)
END_RESPONSE_TABLE;

namespace 
{

tstring ToString(LPCTSTR s)
{return s ? tstring(s) : tstring();}

} // namespace

//
/// Constructs a print or print setup dialog box with specified data from the
/// TPrintDialog::TData struct structure, parent window, window caption, print and
/// setup templates, and module.
//
TPrintDialog::TPrintDialog(TWindow* parent, TData& data, LPCTSTR printTemplateName, LPCTSTR setupTemplateName,
  LPCTSTR title, TModule* module)
:
  TCommonDialog(parent, title, module),
  Data(data),
  PrintTemplateName(ToString(printTemplateName)),
  SetupTemplateName(ToString(setupTemplateName))
{
  Init();
}

//
/// String-aware overload
//
TPrintDialog::TPrintDialog(TWindow* parent, TData& data, const tstring& printTemplateName, const tstring& setupTemplateName,
  const tstring& title, TModule* module)
:
  TCommonDialog(parent, title.c_str(), module),
  Data(data),
  PrintTemplateName(printTemplateName),
  SetupTemplateName(setupTemplateName)
{
  Init();
}

//
/// Common initialization of the contained data structures.
//
void
TPrintDialog::Init()
{
  memset(&Pd, 0, sizeof Pd);
  Pd.lStructSize = sizeof Pd;
  Pd.hwndOwner = GetParentO() ? GetParentO()->GetHandle() : 0;
  Pd.hInstance = *GetModule();
  Pd.Flags = PD_ENABLEPRINTHOOK | PD_ENABLESETUPHOOK | Data.Flags;
  Pd.Flags &= ~PD_RETURNDEFAULT;

  if (PrintTemplateName.length() > 0)
  {
    Pd.lpPrintTemplateName = PrintTemplateName.c_str();
    Pd.Flags |= PD_ENABLEPRINTTEMPLATE;
  }
  else
    Pd.Flags &= ~PD_ENABLEPRINTTEMPLATE;

  if (SetupTemplateName.length() > 0) {
    Pd.lpSetupTemplateName = SetupTemplateName.c_str();
    Pd.Flags |= PD_ENABLESETUPTEMPLATE;
  }
  else
    Pd.Flags &= ~PD_ENABLESETUPTEMPLATE;

  Pd.lpfnPrintHook = 0;
  Pd.lpfnSetupHook = 0;
  Pd.nFromPage = (uint16)Data.FromPage;
  Pd.nToPage = (uint16)Data.ToPage;
  Pd.nMinPage = (uint16)Data.MinPage;
  Pd.nMaxPage = (uint16)Data.MaxPage;
  Pd.nCopies = (uint16)Data.Copies;

  memset(&Psd, 0, sizeof Psd);
  Psd.lStructSize = sizeof Psd;
  Psd.hwndOwner = GetParentO() ? GetParentO()->GetHandle() : 0;
  Psd.hInstance = *GetModule();
  Psd.Flags = PSD_ENABLEPAGESETUPHOOK | PSD_ENABLEPAGEPAINTHOOK | Data.PageSetupFlags;

  if (SetupTemplateName.length() > 0)
  {
    Psd.lpPageSetupTemplateName = SetupTemplateName.c_str();
    Psd.Flags |= PSD_ENABLEPAGESETUPTEMPLATE;
  }
  else
    Psd.Flags &= ~PSD_ENABLEPAGESETUPTEMPLATE;

  Psd.lpfnPageSetupHook = 0;
  Psd.lpfnPagePaintHook = 0;
  Psd.ptPaperSize = Data.PaperSize;
  Psd.rtMinMargin = Data.MinMargin;
  Psd.rtMargin = Data.Margin;
  
#if WINVER >= 0x0500

  memset(&Pde, 0, sizeof Pde);
  Pde.lStructSize = sizeof Pde;

  // Note: PRINTDLGEX::hwndOwner cannot be NULL (see Windows API).
  // But, we need to maintain compatibility with older versions which allowed Parent == 0.
  // So, if no parent has been passed we'll try to assign the main window handle.
  // If this fails we'll handle the problem in DoExecute by reverting to the old dialog.

  TWindow* main_window = GetApplication() ? GetApplication()->GetMainWindow() : 0;
  Pde.hwndOwner = GetParentO() ? GetParentO()->GetHandle() : main_window ? main_window->GetHandle() : NULL; 
  Pde.hInstance = *GetModule();
  Pde.Flags = Data.Flags;  
  Pde.Flags &= ~PD_RETURNDEFAULT;
	Pde.Flags2 = 0;
  Pde.nStartPage = START_PAGE_GENERAL;   // Needed, won't work otherwise even for PD_RETURNDEFAULT

  if (PrintTemplateName.length() > 0)
  {
    Pde.lpPrintTemplateName = PrintTemplateName.c_str();
    Pde.Flags |= PD_ENABLEPRINTTEMPLATE;
    Pde.hInstance = GetModule()->GetHandle();
  }
  else
  {
    Pde.Flags &= ~PD_ENABLEPRINTTEMPLATE;
    Pde.hInstance = 0;
  }
    
  Pde.lpCallback = 0; 
  Pde.nMaxPageRanges = 1;
  Pde.lpPageRanges = &PdeRange;
  Pde.lpPageRanges[0].nFromPage = (uint16)Data.FromPage;
  Pde.lpPageRanges[0].nToPage = (uint16)Data.ToPage;
  Pde.nMinPage = (uint16)Data.MinPage;
  Pde.nMaxPage = (uint16)Data.MaxPage;
  Pde.nCopies = (uint16)Data.Copies;

#endif //#if WINVER >= 0x0500
}

//
/// Default processing.
/// Responds to the click of the setup button with an EV_COMMAND message.
//
void TPrintDialog::CmSetup() 
{
  DefaultProcessing();
}

//
/// Returns the PRINTDLG structure used by the dialog.
//
PRINTDLG& TPrintDialog::GetPD() 
{
  return pd;
}

//
/// Sets the PRINTDLG structure used by the dialog.
//
void TPrintDialog::SetPD(const PRINTDLG& _pd) 
{
  pd = _pd;
}

//
/// Returns the transfer data of the dialog.
//
TPrintDialog::TData& TPrintDialog::GetData() 
{
  return Data;
}

//
/// Returns true if a message is handled.
//
TDialog::TDialogProcReturnType
TPrintDialog::DialogFunction(TMsgId msg, TParam1 param1, TParam2 param2)
{
  return TCommonDialog::DialogFunction(msg, param1, param2);
}

//
/// If no error occurs, DoExecute copies flags and print specifications into the
/// data argument in the constructor. If an error occurs, DoExecute sets the error
/// number of data to an error code from TPrintDialog::TData::Error.
//
int
TPrintDialog::DoExecute()
{
  if (Data.DoPageSetup)
  {
    Psd.lpfnPageSetupHook = LPPAGESETUPHOOK(StdDlgProc);
    Psd.lpfnPagePaintHook = LPPAGEPAINTHOOK(StdDlgProc);

    Data.Unlock();

    Psd.hDevMode = Data.HDevMode;
    Psd.hDevNames = Data.HDevNames;

    int ret = TCommDlg::PageSetupDlg(&Psd);
    if (ret)
    {
      Data.PageSetupFlags = Psd.Flags;
      Data.Error = 0;
      Data.PaperSize = Psd.ptPaperSize;
      Data.MinMargin = Psd.rtMinMargin;
      Data.Margin = Psd.rtMargin;
    }
    else
    {
      Data.Error = TCommDlg::CommDlgExtendedError();
    }

    Data.HDevMode = Psd.hDevMode;
    Data.HDevNames = Psd.hDevNames;

    Data.Lock();

    return ret ? IDOK : IDCANCEL;
  }

  Pd.lpfnPrintHook = LPPRINTHOOKPROC(StdDlgProc);
  Pd.lpfnSetupHook = LPSETUPHOOKPROC(StdDlgProc);

  Data.Unlock();
  Pd.hDevMode = Data.HDevMode;
  Pd.hDevNames = Data.HDevNames;

  int result;  

#if WINVER >= 0x0500

	HWND hDummyWnd = NULL;

  if (!Data.UseOldDialog && TSystem::GetMajorVersion() > 4 && Pde.hwndOwner == NULL)
  {
  	//If no TWindow is provided, create a dummy window to pass to the new print dialog
    //
  	hDummyWnd = ::CreateWindow(_T("STATIC"), _T(""), 0,
       0, 0, 0, 0,
       NULL, NULL,
       GetApplication() ? GetApplication()->GetHandle() : NULL, NULL);

    Pde.hwndOwner = hDummyWnd;
  }

  if (!Data.UseOldDialog && TSystem::GetMajorVersion() > 4 && Pde.hwndOwner != NULL)
  {
  	Pde.hDevMode = Data.HDevMode;
  	Pde.hDevNames = Data.HDevNames;
    CHECK(Pde.hwndOwner); // cannot be NULL
  	HRESULT ret = TCommDlg::PrintDlgEx(&Pde);
    if (ret == S_OK)
	  {
      if (!(Pde.Flags & PD_RETURNDEFAULT) && Pde.dwResultAction == PD_RESULT_CANCEL)
      {
      	Data.Error = 0;
        result = IDCANCEL;
      }
      else
      {
        Data.Flags = Pde.Flags;
        Data.Error = 0;
        Data.HDc = Pde.hDC;
        Data.FromPage = Pde.lpPageRanges[0].nFromPage;
        Data.ToPage = Pde.lpPageRanges[0].nToPage;
        Data.Copies = Pde.nCopies;

        if ((Pde.Flags & PD_RETURNDEFAULT) || Pde.dwResultAction == PD_RESULT_PRINT)
        {
          result = IDOK;
        }
        else
        {
          result = IDCANCEL;
        }
      }
    }
  	else
  	{
    	Data.Error = TCommDlg::CommDlgExtendedError();
      result = IDCANCEL;
  	}
  	Data.HDevMode = Pde.hDevMode;
  	Data.HDevNames = Pde.hDevNames;

    if (hDummyWnd != NULL)
    {
    	::DestroyWindow(hDummyWnd);
    }
  }
  else

#endif //#if WINVER >= 0x0500

  {
	  int ret = TCommDlg::PrintDlg(&Pd);
    if (ret)
	  {
  	  Data.Flags = Pd.Flags;
    	Data.Error = 0;
    	Data.HDc = Pd.hDC;
    	Data.FromPage = Pd.nFromPage;
    	Data.ToPage = Pd.nToPage;
    	Data.Copies = Pd.nCopies;
      result = IDOK;
  	}
  	else
  	{
    	Data.Error = TCommDlg::CommDlgExtendedError();
      result = IDCANCEL;
  	}
  	Data.HDevMode = Pd.hDevMode;
  	Data.HDevNames = Pd.hDevNames;
  }

	Data.Lock();
  return result;
}

//
/// Without displaying a dialog box, GetDefaultPrinter gets the device mode and name
/// that are initialized for the system default printer.
//
bool
TPrintDialog::GetDefaultPrinter()
{
  Pd.Flags |= PD_RETURNDEFAULT;
#if(WINVER >= 0x0500)
  int flags = Pde.Flags;
  Pde.Flags = PD_RETURNDEFAULT;
#endif
  Data.ClearDevMode();
  Data.ClearDevNames();
  int result = DoExecute();
#if(WINVER >= 0x0500)
  Pde.Flags = flags;
#endif
  return result == IDOK;
}

//----------------------------------------------------------------------------

TPrintDialog::TData::TData()
:
  Flags(PD_ALLPAGES|PD_COLLATE),
  Error(0),
  FromPage(-1), ToPage(-1),
  MinPage(-1), MaxPage(-1),
  Copies(1),
  PageSetupFlags(PSD_DEFAULTMINMARGINS),
  DoPageSetup(false),
  UseOldDialog(false),
  HDevMode(0),
  HDevNames(0), HDc(0),
  DevMode(0), DevNames(0)
{
}

TPrintDialog::TData::~TData()
{
  if (HDevMode) 
  {
    ::GlobalUnlock(HDevMode);
    ::GlobalFree(HDevMode);
  }
  if (HDevNames) 
  {
    ::GlobalUnlock(HDevNames);
    ::GlobalFree(HDevNames);
  }
  if (HDc)
    ::DeleteDC(HDc);
}

//
/// Gets a pointer to a DEVMODE structure (a structure containing information
/// necessary to initialize the dialog controls).
//
const DEVMODE* TPrintDialog::TData::GetDevMode() const 
{
  return DevMode;
}

//
/// Gets a pointer to a non-const DEVMODE structure (a structure containing information
/// necessary to initialize the dialog controls).
//
DEVMODE* TPrintDialog::TData::GetDevMode() 
{
  return DevMode;
}

//
/// Gets a pointer to a DEVNAMES structure (a structure containing three strings
/// used to specify the driver name, the printer name, and the output port name).
//
const DEVNAMES * TPrintDialog::TData::GetDevNames() const 
{
  return DevNames;
}

//
/// Locks memory associated with the DEVMODE and DEVNAMES structures.
//
void
TPrintDialog::TData::Lock()
{
  if (HDevMode)
    DevMode = (DEVMODE *)::GlobalLock(HDevMode);
  else
    DevMode = 0;
  if (HDevNames)
    DevNames = (DEVNAMES *)::GlobalLock(HDevNames);
  else
    DevNames = 0;
}

//
/// Unlocks memory associated with the DEVMODE and DEVNAMES structures.
//
void
TPrintDialog::TData::Unlock()
{
  if (HDevMode) 
  {
    ::GlobalUnlock(HDevMode);
    DevMode = 0;
  }
  if (HDevNames) 
  {
    ::GlobalUnlock(HDevNames);
    DevNames = 0;
  }
  if (HDc) 
  {
    ::DeleteDC(HDc);
    HDc = 0;
  }
}

//
/// Clears device mode information (information necessary to initialize the dialog
/// controls).
//
void
TPrintDialog::TData::ClearDevMode()
{
  if (HDevMode) 
  {
    ::GlobalUnlock(HDevMode);
    ::GlobalFree(HDevMode);
    HDevMode = 0;
    DevMode = 0;
  }
}

//
/// Sets the values for the DEVMODE structure.
//
void
TPrintDialog::TData::SetDevMode(const DEVMODE* devMode)
{
  ClearDevMode();
  if (devMode) 
  {
    int size = devMode->dmSize + devMode->dmDriverExtra;
    HDevMode = ::GlobalAlloc(GHND, size);
    DevMode = (DEVMODE *)::GlobalLock(HDevMode);
    memcpy(DevMode, devMode, size);
  }
}

//
/// Clears the device name information (information that contains three strings used
/// to specify the driver name, the printer name, and the output port name).
//
void
TPrintDialog::TData::ClearDevNames()
{
  if (HDevNames) 
  {
    ::GlobalUnlock(HDevNames);
    ::GlobalFree(HDevNames);
    HDevNames = 0;
    DevNames = 0;
  }
}

//
/// Gets the name of the printer device driver.
//
LPCTSTR
TPrintDialog::TData::GetDriverName() const
{
  return DevNames ? reinterpret_cast<LPCTSTR>(DevNames) + DevNames->wDriverOffset : 0;
}

//
/// Gets the name of the output device.
//
LPCTSTR
TPrintDialog::TData::GetDeviceName() const
{
  return DevNames ? reinterpret_cast<LPCTSTR>(DevNames) + DevNames->wDeviceOffset : 0;
}

//
/// Gets the name of the physical output medium.
//
LPCTSTR
TPrintDialog::TData::GetOutputName() const
{
  return DevNames ? reinterpret_cast<LPCTSTR>(DevNames) + DevNames->wOutputOffset : 0;
}

//
/// Sets the values for the DEVNAMES structure.
//
void
TPrintDialog::TData::SetDevNames(const tstring& driver, const tstring& device, const tstring& output)
{
  ClearDevNames();

  // Calculate the required buffer size, as the number of characters incl. null-terminators,
  // and the resulting total size, in bytes, of the DEVNAMES structure including trailing strings.
  // Then allocate and lock the required amount of global memory.
  //
  const int n = driver.length() + 1 + device.length() + 1 + output.length() + 1; 
  const int size = sizeof(DEVNAMES) + (n * sizeof(tchar));
  HDevNames = ::GlobalAlloc(GHND, size); 
  DevNames = static_cast<DEVNAMES*>(::GlobalLock(HDevNames));
  DevNames->wDefault = false;

  // Calculate the offsets to the strings within DEVNAMES.
  // Then copy the given names into DEVNAMES (actually, behind the fixed part of DEVNAMES).
  //
  // NB! Offsets are in character counts. Here we assume the size of the fixed part of DEVNAMES is divisible 
  // by the character size. Otherwise the driver name will overwrite the last byte of the fixed part.
  // But DEVNAMES is divisible and forever set in stone, so we don't need to consider an odd struct size.
  //
  CHECK(sizeof(DEVNAMES) % sizeof(tchar) == 0); 
  const LPTSTR base = reinterpret_cast<LPTSTR>(DevNames);
  const LPTSTR pDriver = reinterpret_cast<LPTSTR>(DevNames + 1); // Jump past the the fixed part, assuming even struct size.
  const LPTSTR pDevice = pDriver + driver.length() + 1; // Jump past the preceding string, including null-terminator.
  const LPTSTR pOutput = pDevice + device.length() + 1;
  DevNames->wDriverOffset = static_cast<WORD>(pDriver - base);
  DevNames->wDeviceOffset = static_cast<WORD>(pDevice - base);
  DevNames->wOutputOffset = static_cast<WORD>(pOutput - base);
  _tcscpy(pDriver, driver.c_str());
  _tcscpy(pDevice, device.c_str());
  _tcscpy(pOutput, output.c_str());
}

//
/// Creates and returns a TPrintDC with the current settings.
/// Pass ownership of our hDC to the caller thru the new's TPrintDC object
//
TPrintDC*
TPrintDialog::TData::TransferDC()
{
  if (!HDc)
    return 0;
  HDC hdc = HDc;
  HDc = 0;
  return new TPrintDC(hdc, AutoDelete);
}


//
// Read the persistent object from the stream.
//
void*
TPrintDialog::TData::Read(ipstream& is, uint32 version)
{
  is >> Flags;
  is >> FromPage;
  is >> ToPage;
  is >> MinPage;
  is >> MaxPage;
  is >> Copies;
  char * driver = is.freadString();
  char * device = is.freadString();
  char * output = is.freadString();
  uint16 deflt;
  is >> deflt;
  _USES_CONVERSION;
  SetDevNames(_A2W(driver), _A2W(device), _A2W(output));
  if (DevNames)
    DevNames->wDefault = deflt;
  delete[] driver;
  delete[] device;
  delete[] output;

  int16 size;
  is >> size;
  if (size)
  {
    DEVMODE * devMode = (DEVMODE *)new  tchar[size];
    is.freadBytes(devMode, size);
    SetDevMode(devMode);
    delete[] devMode;
  }
  else
    ClearDevMode();

  if (version > 1)
  {
    is >> PageSetupFlags;
    is >> PaperSize;
    is >> MinMargin;
    is >> Margin;
  }

  return this;
}

//
/// Writes the object to a peristent stream.
//
void
TPrintDialog::TData::Write(opstream& os)
{
  _USES_CONVERSION;

  os << Flags;
  os << FromPage;
  os << ToPage;
  os << MinPage;
  os << MaxPage;
  os << Copies;
  os.fwriteString(_W2A(GetDriverName()));
  os.fwriteString(_W2A(GetDeviceName()));
  os.fwriteString(_W2A(GetOutputName()));
  os << (DevNames ? DevNames->wDefault : uint16(0));

  if (DevMode) 
  {
    int16 size = int16(DevMode->dmSize + DevMode->dmDriverExtra);
    os << size;
    os.fwriteBytes(DevMode, size); // NB! Problem with Unicode?
  }
  else
    os << int16(0);

  os << PageSetupFlags;
  os << PaperSize;
  os << MinMargin;
  os << Margin;
}


} // OWL namespace

