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

using namespace std;

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
  int len = static_cast<int>(c.length() + Text.length() + Text2.length() + 5);
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
  Number = static_cast<int>(param1);
  if (Number > 0) {
    LPCTSTR c = GetCaption();
    CHECK(c);
    if (c)
    {
      int len = static_cast<int>(::_tcslen(c) + sizeof(Number) + 5);
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
TXPrinter::Clone()
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

namespace {

auto GetDevMode_(const tstring& device) -> vector<char>
{
  // Get the size required for this device's DEVMODE structure.
  //
  const auto n = const_cast<LPTSTR>(device.c_str());
  const auto s = DocumentProperties(nullptr, nullptr, n, nullptr, nullptr, 0);
  if (s <= 0) throw TXPrinter{};

  // Now, create a buffer of the required size and get the device data.
  //
  auto buffer = vector<char>(s);
  const auto p = reinterpret_cast<PDEVMODE>(buffer.data());
  const auto r = DocumentProperties(nullptr, nullptr, n, p, nullptr, DM_OUT_BUFFER);
  if (r != IDOK) throw TXPrinter{};
  return buffer;
}

} // namespace

//
/// Retrieves the DEVMODE device data for the given device.
//
TPrinter::TDevMode::TDevMode(const tstring& device)
  : Buffer{GetDevMode_(device)}
{}

//----------------------------------------------------------------------------

//
/// Set by printing dialog if user cancels.
//
HDC TPrinter::UserAbortDC = nullptr;

//
/// Retrieves the printer name of the default printer for the current user on the local computer.
/// Wrapper for the Windows API function GetDefaultPrinter.
/// http://msdn.microsoft.com/en-us/library/windows/desktop/dd144876.aspx
/// Throws TXPrinter if there is no default printer, or if an unexpected error occurs.
//
auto TPrinter::GetDefaultPrinter() -> tstring
{
  auto b = tstring(15, _T('\0')); // Intentionally small to allow Small String Optimization.
  b.resize(b.capacity()); // Ensures we use the full size of our buffer.
  auto n = static_cast<DWORD>(b.size()) + 1; // The buffer includes the null-terminator as well.

  // We retry calls to ::GetDefaultPrinter in a loop since the initial buffer may be insufficient.
  // Also, although unlikely, the printer may change between calls to ::GetDefaultPrinter, asking
  // for a larger and larger buffer for each iteration around the loop.
  //
  while (!::GetDefaultPrinter(&b[0], &n))
    switch (GetLastError())
    {
    case ERROR_INSUFFICIENT_BUFFER:
      CHECK(n > b.size() + 1);
      b.resize(n - 1); // String size excludes the null-terminator.
      break;

    case ERROR_FILE_NOT_FOUND: // There is no default printer.
    default:
      throw TXPrinter{};
    }
  CHECK(n > 0); // We should at least have a null-terminator.
  b.resize(n - 1);
  return b;
}

//
/// Associates this TPrinter object with the given printer device.
/// If no name is given, the name of the current default printer is used.
//
TPrinter::TPrinter(const tstring& device)
  : Error{0}, Data{nullptr}, PageSize{}, PageSizeInch{}, OutputFile{}
{
  SetDevice(device);
}

//
/// Move-constructor; enables auto-declaration style.
//
TPrinter::TPrinter(TPrinter&& other)
  : Error{other.Error}, Data{other.Data},
  PageSize{other.PageSize}, PageSizeInch{other.PageSizeInch}, 
  OutputFile{move(other.OutputFile)}
{
  other.Data = nullptr;
}

//
/// Frees the resources allocated to TPrinter.
//
TPrinter::~TPrinter()
{
  delete Data;
}

//
/// Assigns the device to be represented by this TPrinter object.
//
void TPrinter::SetDevice(const tstring& device)
{
  auto d = make_unique<TPrintDialog::TData>();
  d->SetDevNames(_T(""), device, _T(""));
  d->SetDevMode(TDevMode{device}.GetData());
  SetData(d.get());
  d.release(); // We took ownership.
}

//
/// Returns a reference to the TPrintDialog data structure.
//
TPrintDialog::TData& TPrinter::GetSetup()
{
  return *Data;
}

//
/// Returns the device context already obtained in our setup data, or if none, creates a new
/// one based on the device name and device initialization data (DEVMODE).
//
auto TPrinter::GetDC() -> unique_ptr<TPrintDC>
{
  const auto d = GetData(); CHECK(d);
  const auto dc = d->TransferDC();
  return dc ? unique_ptr<TPrintDC>(dc) :
    make_unique<TPrintDC>(d->GetDeviceName(), d->GetDevMode());
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

namespace {

//
// Common implementation of GetCapability for capabilities that return a vector of strings.
//
auto GetCapabilityStrings_(const TPrinter& p, TPrinter::TCapability c, int stringBufferSize) -> vector<tstring>
{
  PRECONDITION(c == TPrinter::dcBinNames || c == TPrinter::dcFileDependencies || c == TPrinter::dcMediaReady ||
    c == TPrinter::dcMediaTypeNames || c == TPrinter::dcPaperNames || c == TPrinter::dcPersonality);
  auto r = vector<tstring>{};
  const auto n = p.GetCapability(c, nullptr);
  if (n == static_cast<DWORD>(-1)) return r;
  auto b = vector<TCHAR>(n * stringBufferSize);
  const auto m = p.GetCapability(c, reinterpret_cast<LPTSTR>(b.data()));
  if (m == static_cast<DWORD>(-1)) return r;
  CHECK(m <= n); // If there are more elements since the last call, we had a buffer overflow!
  for (auto i = 0; i != min(m, n); ++i)
  {
    // The string buffers may not be null-terminated, hence the two-step construction.
    //
    const auto p = &b[i * stringBufferSize];
    r.emplace_back(tstring{p, static_cast<size_t>(stringBufferSize)}.c_str());
  }
  return r;
}

} // namespace

//
/// Retrieves the names of the printer's paper bins.
/// If the function fails, an empty vector is returned.
/// \sa TCapability::dcBinNames
//
auto TPrinter::GetBinNames() const -> TBinNames
{
  return GetCapabilityStrings_(*this, dcBinNames, 24);
}

//
/// Retrieves a list of available paper bins.
/// Each element of the returned vector is a bin identifier, e.g. DMBIN_AUTO.
/// See the description of the dmDefaultSource member of the DEVMODE structure.
/// If the function fails, an empty vector is returned.
/// \sa TCapability::dcBins
//
auto TPrinter::GetBins() const -> TBins
{
  auto f = [&](LPTSTR s) { return GetCapability(dcBins, s); };
  const auto n = f(nullptr);
  if (n == static_cast<DWORD>(-1)) return TBins{};
  auto r = TBins(n);
  const auto m = f(reinterpret_cast<LPTSTR>(r.data()));
  if (m == static_cast<DWORD>(-1)) return TBins{};
  CHECK(m <= n); // If there are more elements since the last call, we had a buffer overflow!
  return r;
}

//
/// Returns whether the printer supports collating.
/// If the function fails, `false` is returned.
/// \sa TCapability::dcCollate
//
auto TPrinter::CanCollate() const -> bool
{
  return GetCapability(dcCollate, nullptr) == 1;
}

//
/// Returns whether the printer supports color printing.
/// If the function fails, `false` is returned.
/// \sa TCapability::dcColorDevice
//
auto TPrinter::IsColorDevice() const -> bool
{
  return GetCapability(dcColorDevice, nullptr) == 1;
}

//
/// Return the maximum number of copies the printer can print.
/// If the function fails, 0 is returned.
/// \sa TCapability::dcCopies
//
auto TPrinter::GetMaxCopies() const -> int
{
  const auto n = GetCapability(dcCopies, nullptr);
  if (n == static_cast<DWORD>(-1)) return 0;
  return static_cast<int>(n);
}

//
/// Returns the version number of the printer driver.
/// If the function fails, 0xFFFFFFFF is returned.
/// \sa TCapability::dcDriver
//
auto TPrinter::GetDriverVersion() const -> DWORD
{
  return GetCapability(dcDriver, nullptr);
}

//
/// Returns whether the printer supports duplex printing.
/// If the function fails, `false` is returned.
/// \sa TCapability::dcDuplex
//
auto TPrinter::HasDuplexSupport() const -> bool
{
  return GetCapability(dcDuplex, nullptr) == 1;
}

//
/// Retrieves a list of the resolutions supported by the printer.
/// If the function fails, an empty vector is returned.
/// \sa TCapability::dcEnumResolutions
//
auto TPrinter::GetResolutions() const -> TResolutions
{
  auto r = TResolutions{};
  auto f = [&](LPTSTR s) { return GetCapability(dcEnumResolutions, s); };
  const auto n = f(nullptr);
  if (n == static_cast<DWORD>(-1)) return r;
  auto b = vector<LONG>(2 * n);
  const auto m = f(reinterpret_cast<LPTSTR>(b.data()));
  if (m == static_cast<DWORD>(-1)) return r;
  CHECK(m <= n); // If there are more elements since the last call, we had a buffer overflow!
  for (auto i = 0; i != 2 * min(m, n); i += 2)
    r.emplace_back(b[i], b[i + 1]);
  return r;
}

//
/// Returns the number of bytes required for the device-specific portion of the DEVMODE
/// structure for the printer driver.
/// If the function fails, 0 is returned.
/// \sa TCapability::dcExtra
//
auto TPrinter::GetDevModeExtra() const -> int
{
  const auto n = GetCapability(dcExtra, nullptr);
  if (n == static_cast<DWORD>(-1)) return 0;
  return static_cast<int>(n);
}

//
/// Returns the `dmFields` member of the printer driver's DEVMODE structure.
/// The `dmFields` member indicates which members in the device-independent portion of the
/// structure are supported by the printer driver.
/// \sa TCapability::dcFields
//
auto TPrinter::GetDevModeFields() const -> DWORD
{
  return GetCapability(dcFields, nullptr);
}

//
/// Returns the names of any additional files that need to be loaded for the printer driver.
/// If the function fails, an empty vector is returned.
/// \sa TCapability::dcFileDependencies
//
auto TPrinter::GetDriverDependencies() const -> TFileDependencies
{
  return GetCapabilityStrings_(*this, dcFileDependencies, 64);
}

//
/// Returns the maximum paper size that the printer supports.
/// If the function fails, TSize{0, 0} is returned.
/// \sa TCapability::dcMaxExtent, TCapability::dcMinExtent
//
auto TPrinter::GetMaxExtent() const -> TSize
{
  const auto m = GetCapability(dcMaxExtent, nullptr);
  if (m == static_cast<DWORD>(-1)) return TSize{0, 0};
  return TSize{LoInt16(m), HiInt16(m)};
}

//
/// Returns the names of the paper forms that are currently available for use.
/// If the function fails, an empty vector is returned.
/// \sa TCapability::dcMediaReady
//
auto TPrinter::GetReadyMedia() const -> TPaperForms
{
  return GetCapabilityStrings_(*this, dcMediaReady, 64);
}

//
/// Returns the names of the supported media types.
/// If the function fails, an empty vector is returned.
/// \sa TCapability::dcMediaTypeNames
//
auto TPrinter::GetMediaTypeNames() const -> TMediaTypeNames
{
  return GetCapabilityStrings_(*this, dcMediaTypeNames, 64);
}

//
/// Returns a list of supported media types.
/// For a list of possible media type values, see the description of the `dmMediaType` member
/// of the DEVMODE structure.
/// If the function fails, an empty vector is returned.
/// \sa TCapability::dcMediaTypes
//
auto TPrinter::GetMediaTypes() const -> TMediaTypes
{
  auto f = [&](LPTSTR s) { return GetCapability(dcMediaTypes, s); };
  const auto n = f(nullptr);
  if (n == static_cast<DWORD>(-1)) return TMediaTypes{};
  auto r = TMediaTypes(n);
  const auto m = f(reinterpret_cast<LPTSTR>(r.data()));
  if (m == static_cast<DWORD>(-1)) return TMediaTypes{};
  CHECK(m <= n); // If there are more elements since the last call, we had a buffer overflow!
  return r;
}

//
/// Returns the minimum paper size that the printer supports.
/// If the function fails, TSize{0, 0} is returned.
/// \sa TCapability::dcMinExtent, TCapability::dcMaxExtent
//
auto TPrinter::GetMinExtent() const -> TSize
{
  const auto m = GetCapability(dcMinExtent, nullptr);
  if (m == static_cast<DWORD>(-1)) return TSize{0, 0};
  return TSize{LoInt16(m), HiInt16(m)};
}

//
/// Returns the angle of rotation between portrait and landscape orientation.
/// The result is given in terms of the number of degrees that portrait orientation is rotated
/// counter-clockwise to produce landscape orientation. If landscape orientation is not
/// supported, then 0 is returned.
/// If the function fails, 0 is returned.
/// \sa TCapability::dcOrientation
//
auto TPrinter::GetLandscapeOrientation() const -> int
{
  const auto r = GetCapability(dcOrientation, nullptr);
  if (r == static_cast<DWORD>(-1)) return 0;
  return static_cast<int>(r);
}

//
/// Retrieves a list of integers that indicate the printers ability to print multiple document
/// pages per printed page. Each element of the list represent a supported configuration, given
/// by the number of document pages per printed page.
/// If the function fails, an empty vector is returned.
/// \sa TCapability::dcNup
//
auto TPrinter::GetNupConfigurations() const -> TNupConfigurations
{
  auto f = [&](LPTSTR s) { return GetCapability(dcNup, s); };
  const auto n = f(nullptr);
  if (n == static_cast<DWORD>(-1)) return TNupConfigurations{};
  auto r = TNupConfigurations(n);
  CHECK(sizeof(TNupConfigurations::value_type) == sizeof(DWORD));
  const auto m = f(reinterpret_cast<LPTSTR>(r.data()));
  if (m == static_cast<DWORD>(-1)) return TNupConfigurations{};
  CHECK(m <= n); // If there are more elements since the last call, we had a buffer overflow!
  return r;
}

//
/// Returns a list of supported paper names.
/// For example, paper names could be "Letter" or "Legal".
/// If the function fails, an empty vector is returned.
/// \sa TCapability::dcPaperNames
//
auto TPrinter::GetPaperNames() const -> TPaperNames
{
  return GetCapabilityStrings_(*this, dcPaperNames, 64);
}

//
/// Returns a list of supported paper sizes.
/// For a list of the possible element values, see the description of the `dmPaperSize` member
/// of the DEVMODE structure.
/// If the function fails, an empty vector is returned.
/// \sa TCapability::dcPapers
//
auto TPrinter::GetPapers() const -> TPapers
{
  auto f = [&](LPTSTR s) { return GetCapability(dcPapers, s); };
  const auto n = f(nullptr);
  if (n == static_cast<DWORD>(-1)) return TPapers{};
  auto r = TPapers(n);
  const auto m = f(reinterpret_cast<LPTSTR>(r.data()));
  if (m == static_cast<DWORD>(-1)) return TPapers{};
  CHECK(m <= n); // If there are more elements since the last call, we had a buffer overflow!
  return r;
}

//
/// Retrieves the dimensions of each supported paper size.
/// The unit is tenths of a millimeter (LOMETRIC).
/// If the function fails, an empty vector is returned.
/// \sa TCapability::dcPaperSize
//
auto TPrinter::GetPaperSizes() const -> TPaperSizes
{
  auto r = TPaperSizes{};
  auto f = [&](LPTSTR s) { return GetCapability(dcPaperSize, s); };
  const auto n = f(nullptr);
  if (n == static_cast<DWORD>(-1)) return r;
  auto b = vector<POINT>(n);
  const auto m = f(reinterpret_cast<LPTSTR>(b.data()));
  if (m == static_cast<DWORD>(-1)) return r;
  CHECK(m <= n); // If there are more elements since the last call, we had a buffer overflow!
  for (auto i = 0; i != min(m, n); ++i)
    r.emplace_back(b[i].x, b[i].y);
  return r;
}

//
/// Returns a list of printer description languages supported by the printer.
/// If the function fails, an empty vector is returned.
/// \sa TCapability::dcPersonality
//
auto TPrinter::GetDescriptionLanguages() const -> TDescriptionLanguages
{
  return GetCapabilityStrings_(*this, dcPersonality, 32);
}

//
/// Returns the amount of available printer memory, in kilobytes.
/// If the function fails, 0 is returned.
/// \sa TCapability::dcPrinterMem
//
auto TPrinter::GetMemoryCapacity() const -> int
{
  const auto r = GetCapability(dcPrinterMem, nullptr);
  if (r == static_cast<int>(-1)) return 0;
  return static_cast<int>(r);
}

//
/// Returns the printer's print rate.
/// Call GetCapability<TPrinter::dcPrintRateUnit> to determine the units of the returned value.
/// If the function fails, 0 is returned.
/// \sa TCapability::dcPrintRate, TCapability::dcPrintRateUnit, TPrintRateUnit
//
auto TPrinter::GetPrintRate() const -> int
{
  const auto r = GetCapability(dcPrintRate, nullptr);
  if (r == static_cast<int>(-1)) return 0;
  return static_cast<int>(r);
}

//
/// Returns the printer's print rate in pages per minute.
/// If the function fails, 0 is returned.
/// \sa TCapability::dcPrintRatePpm
//
auto TPrinter::GetPrintRatePpm() const -> int
{
  const auto r = GetCapability(dcPrintRatePpm, nullptr);
  if (r == static_cast<int>(-1)) return 0;
  return static_cast<int>(r);
}

//
/// Returns the printer's print rate units.
/// The returned value determines the units for the value returned by GetCapability<TPrinter::dcPrintRate>.
/// If the function fails, `pruUnknown` is returned.
/// \sa TCapability::dcPrintRateUnit, TCapability::dcPrintRate, TPrintRateUnit
//
auto TPrinter::GetPrintRateUnit() const -> TPrintRateUnit
{
  const auto r = GetCapability(dcPrintRateUnit, nullptr);
  switch (r)
  {
  case pruPagesPerMinute:
  case pruCharactersPerSecond:
  case pruLinesPerMinute:
  case pruInchesPerMinute:
    return static_cast<TPrintRateUnit>(r);

  default:
    return pruUnknown;
  }
}

//
/// Returns the `dmSize` member of the printer driver's DEVMODE structure.
/// The `dmSize` member indicates the size (and hence version) of the DEVMODE structure.
/// The size excludes the private driver-data that follows the structure. See the `dmDriverExtra` member.
/// If the function fails, 0 is returned.
/// \sa TCapability::dcSize
//
auto TPrinter::GetDevModeSize() const -> int
{
  const auto n = GetCapability(dcSize, nullptr);
  if (n == static_cast<DWORD>(-1)) return 0;
  return static_cast<int>(n);
}

//
/// Returns whether the printer supports stapling.
/// If the function fails, `false` is returned.
/// \sa TCapability::dcStaple
//
auto TPrinter::CanStaple() const -> bool
{
  return GetCapability(dcStaple, nullptr) == 1;
}

//
/// Retrieves the abilities of the printer to use TrueType fonts.
/// The returned value can be a combination of flags DCTT_BITMAP, DCTT_DOWNLOAD and DCTT_SUBDEV.
/// If the function fails, 0 is returned.
/// \sa TCapability::dcTrueType
//
auto TPrinter::GetTrueTypeCapabilities() const -> DWORD
{
  const auto r = GetCapability(dcTrueType, nullptr);
  if (r == static_cast<DWORD>(-1)) return 0;
  return r;
}

//
/// Returns the specification version to which the printer driver conforms.
/// If the function fails, 0xFFFFFFFF is returned.
/// \sa TCapability::dcVersion
//
auto TPrinter::GetDriverSpecificationVersion() const -> DWORD
{
  return GetCapability(dcVersion, nullptr);
}

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
    TPrinter::SetUserAbort(nullptr);
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
  TPointer<TPrintDC> prnDC (nullptr);  // Pointer to printer DC created by Printer Object
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
    prnDC = new TPrintDC{Data->GetDeviceName(), Data->GetDevMode()};
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
  if ((prompt && (Data->Flags & PD_SELECTION)) || selFromPage)
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
  LPCTSTR out = OutputFile.length() == 0 ? nullptr : OutputFile.c_str();

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
    SetUserAbort(nullptr);
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
  const auto errorCaption = parent->LoadString(IDS_PRNERRORCAPTION);
  const auto errorStr = TXPrinting(Error).GetErrorMessage(parent->GetModule());
  const auto formatStr = parent->LoadString(IDS_PRNERRORTEMPLATE);
  parent->FormatMessageBox(formatStr, errorCaption, MB_OK | MB_ICONSTOP, printout.GetTitle(), errorStr.c_str());
}

//----------------------------------------------------------------------------


IMPLEMENT_STREAMABLE(TPrinter);

#if OWL_PERSISTENT_STREAMS

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

#endif



} // OWL namespace
/* ========================================================================== */

