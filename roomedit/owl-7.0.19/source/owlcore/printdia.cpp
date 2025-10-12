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
#include <memory>

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

tstring ToString(TResId id)
{return id.IsString() ? tstring(id.GetString()) : tstring();}

} // namespace

//
/// Constructs a print or print setup dialog box with specified data from the
/// TPrintDialog::TData struct structure, parent window, window caption, print and
/// setup templates, and module.
//
TPrintDialog::TPrintDialog(TWindow* parent, TData& data, TResId printTemplate, TResId setupTemplate,
  LPCTSTR title, TModule* module)
:
  TCommonDialog(parent, title, module),
  Data(data),
  PrintTemplate(printTemplate),
  PrintTemplateName(ToString(printTemplate)),
  SetupTemplate(setupTemplate),
  SetupTemplateName(ToString(setupTemplate))
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
  PrintTemplate(),
  PrintTemplateName(printTemplateName),
  SetupTemplate(),
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
  // If our dialog template identifiers are given as strings, then redirect the resource
  // identifiers to our buffered copies (copied in the constructor). This prevents run-time errors
  // caused by dangling string pointers.
  //
  if (!PrintTemplateName.empty())
    PrintTemplate = TResId{PrintTemplateName.c_str()};

  if (!SetupTemplateName.empty())
    SetupTemplate = TResId{SetupTemplateName.c_str()};

  memset(&Pd, 0, sizeof Pd);
  Pd.lStructSize = sizeof Pd;
  Pd.hwndOwner = GetParentO() ? GetParentO()->GetHandle() : nullptr;
  Pd.hInstance = *GetModule();
  Pd.Flags = PD_ENABLEPRINTHOOK | PD_ENABLESETUPHOOK | Data.Flags;
  Pd.Flags &= ~PD_RETURNDEFAULT;

  if (PrintTemplate)
  {
    Pd.lpPrintTemplateName = PrintTemplate.GetPointerRepresentation();
    Pd.Flags |= PD_ENABLEPRINTTEMPLATE;
  }
  else
    Pd.Flags &= ~PD_ENABLEPRINTTEMPLATE;

  if (SetupTemplate)
  {
    Pd.lpSetupTemplateName = SetupTemplate.GetPointerRepresentation();
    Pd.Flags |= PD_ENABLESETUPTEMPLATE;
  }
  else
    Pd.Flags &= ~PD_ENABLESETUPTEMPLATE;

  Pd.lpfnPrintHook = nullptr;
  Pd.lpfnSetupHook = nullptr;
  Pd.nFromPage = static_cast<uint16>(Data.FromPage);
  Pd.nToPage = static_cast<uint16>(Data.ToPage);
  Pd.nMinPage = static_cast<uint16>(Data.MinPage);
  Pd.nMaxPage = static_cast<uint16>(Data.MaxPage);
  Pd.nCopies = static_cast<uint16>(Data.Copies);

  memset(&Psd, 0, sizeof Psd);
  Psd.lStructSize = sizeof Psd;
  Psd.hwndOwner = GetParentO() ? GetParentO()->GetHandle() : nullptr;
  Psd.hInstance = *GetModule();
  Psd.Flags = PSD_ENABLEPAGESETUPHOOK | PSD_ENABLEPAGEPAINTHOOK | Data.PageSetupFlags;

  if (SetupTemplate)
  {
    Psd.lpPageSetupTemplateName = SetupTemplate.GetPointerRepresentation();
    Psd.Flags |= PSD_ENABLEPAGESETUPTEMPLATE;
  }
  else
    Psd.Flags &= ~PSD_ENABLEPAGESETUPTEMPLATE;

  Psd.lpfnPageSetupHook = nullptr;
  Psd.lpfnPagePaintHook = nullptr;
  Psd.ptPaperSize = Data.PaperSize;
  Psd.rtMinMargin = Data.MinMargin;
  Psd.rtMargin = Data.Margin;

  memset(&Pde, 0, sizeof Pde);
  Pde.lStructSize = sizeof Pde;

  // Note: PRINTDLGEX::hwndOwner cannot be NULL (see Windows API).
  // But, we need to maintain compatibility with older versions which allowed Parent == 0.
  // So, if no parent has been passed we'll try to assign the main window handle.
  // If this fails we'll handle the problem in DoExecute by reverting to the old dialog.

  TWindow* main_window = GetApplication() ? GetApplication()->GetMainWindow() : nullptr;
  Pde.hwndOwner = GetParentO() ? GetParentO()->GetHandle() : main_window ? main_window->GetHandle() : nullptr;
  Pde.hInstance = *GetModule();
  Pde.Flags = Data.Flags;
  Pde.Flags &= ~PD_RETURNDEFAULT;
  Pde.Flags2 = 0;
  Pde.nStartPage = START_PAGE_GENERAL;   // Needed, won't work otherwise even for PD_RETURNDEFAULT

  if (PrintTemplate)
  {
    Pde.lpPrintTemplateName = PrintTemplate.GetPointerRepresentation();
    Pde.Flags |= PD_ENABLEPRINTTEMPLATE;
    Pde.hInstance = GetModule()->GetHandle();
  }
  else
  {
    Pde.Flags &= ~PD_ENABLEPRINTTEMPLATE;
    Pde.hInstance = nullptr;
  }

  Pde.lpCallback = nullptr;
  Pde.nMaxPageRanges = 1;
  Pde.lpPageRanges = &PdeRange;
  Pde.lpPageRanges[0].nFromPage = (uint16)Data.FromPage;
  Pde.lpPageRanges[0].nToPage = (uint16)Data.ToPage;
  Pde.nMinPage = static_cast<uint16>(Data.MinPage);
  Pde.nMaxPage = static_cast<uint16>(Data.MaxPage);
  Pde.nCopies = static_cast<uint16>(Data.Copies);
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
  return Pd;
}

//
/// Sets the PRINTDLG structure used by the dialog.
//
void TPrintDialog::SetPD(const PRINTDLG& pd)
{
  Pd = pd;
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
INT_PTR
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

    int ret = PageSetupDlg(&Psd);
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
      Data.Error = CommDlgExtendedError();
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

  HWND hDummyWnd = nullptr;

  if (!Data.UseOldDialog && TSystem::GetMajorVersion() > 4 && Pde.hwndOwner == nullptr)
  {
    //If no TWindow is provided, create a dummy window to pass to the new print dialog
    //
    hDummyWnd = ::CreateWindow(_T("STATIC"), _T(""), 0,
       0, 0, 0, 0,
       nullptr, nullptr,
       GetApplication() ? GetApplication()->GetHandle() : nullptr, nullptr);

    Pde.hwndOwner = hDummyWnd;
  }

  if (!Data.UseOldDialog && TSystem::GetMajorVersion() > 4 && Pde.hwndOwner != nullptr)
  {
    Pde.hDevMode = Data.HDevMode;
    Pde.hDevNames = Data.HDevNames;
    CHECK(Pde.hwndOwner); // cannot be NULL
    HRESULT ret = PrintDlgEx(&Pde);
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
      Data.Error = CommDlgExtendedError();
      result = IDCANCEL;
    }
    Data.HDevMode = Pde.hDevMode;
    Data.HDevNames = Pde.hDevNames;

    if (hDummyWnd != nullptr)
    {
      ::DestroyWindow(hDummyWnd);
    }
  }
  else
  {
    int ret = PrintDlg(&Pd);
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
      Data.Error = CommDlgExtendedError();
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
  int flags = Pde.Flags;
  Pde.Flags = PD_RETURNDEFAULT;
  Data.ClearDevMode();
  Data.ClearDevNames();
  int result = DoExecute();
  Pde.Flags = flags;
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
  HDevMode(nullptr),
  HDevNames(nullptr), HDc(nullptr),
  DevMode(nullptr), DevNames(nullptr)
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
    DevMode = nullptr;
  if (HDevNames)
    DevNames = (DEVNAMES *)::GlobalLock(HDevNames);
  else
    DevNames = nullptr;
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
    DevMode = nullptr;
  }
  if (HDevNames)
  {
    ::GlobalUnlock(HDevNames);
    DevNames = nullptr;
  }
  if (HDc)
  {
    ::DeleteDC(HDc);
    HDc = nullptr;
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
    HDevMode = nullptr;
    DevMode = nullptr;
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
    HDevNames = nullptr;
    DevNames = nullptr;
  }
}

//
/// Gets the name of the printer device driver.
//
LPCTSTR
TPrintDialog::TData::GetDriverName() const
{
  return DevNames ? reinterpret_cast<LPCTSTR>(DevNames) + DevNames->wDriverOffset : nullptr;
}

//
/// Gets the name of the output device.
//
LPCTSTR
TPrintDialog::TData::GetDeviceName() const
{
  return DevNames ? reinterpret_cast<LPCTSTR>(DevNames) + DevNames->wDeviceOffset : nullptr;
}

//
/// Gets the name of the physical output medium.
//
LPCTSTR
TPrintDialog::TData::GetOutputName() const
{
  return DevNames ? reinterpret_cast<LPCTSTR>(DevNames) + DevNames->wOutputOffset : nullptr;
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
  const int n = static_cast<int>(driver.length() + 1 + device.length() + 1 + output.length() + 1);
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
    return nullptr;
  HDC hdc = HDc;
  HDc = nullptr;
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
  const auto driverNarrow = std::unique_ptr<char[]>(is.freadString());
  const auto deviceNarrow = std::unique_ptr<char[]>(is.freadString());
  const auto outputNarrow = std::unique_ptr<char[]>(is.freadString());
  uint16 deflt;
  is >> deflt;
  _USES_CONVERSION;
  const auto driver = _A2W(driverNarrow.get());
  const auto device = _A2W(deviceNarrow.get());
  const auto output = _A2W(outputNarrow.get());
  SetDevNames(driver, device, output);
  if (DevNames)
    DevNames->wDefault = deflt;

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

