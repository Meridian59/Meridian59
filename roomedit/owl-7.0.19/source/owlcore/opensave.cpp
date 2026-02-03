//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of OpenSave abstract, FileOpen, FileSave Common Dialog
/// classes
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/opensave.h>
#include <owl/commctrl.h>

namespace owl {

OWL_DIAGINFO;


//
/// Constructs a TOpenSaveDialog::TData structure.
//
TOpenSaveDialog::TData::TData(
  uint32 flags,
  LPCTSTR filter,
  LPTSTR, // customFilter, // Dummy for backward compatibility.
  LPCTSTR initialDir,
  LPCTSTR defExt,
  int maxPath,
  int filterIndex,
  uint32 flagsEx
  )
  : Flags(flags),
  FlagsEx(flagsEx),
  Error(0),
  FileName(nullptr),
  Filter(nullptr),
  FilterIndex(filterIndex),
  InitialDir(initialDir),
  DefExt(defExt),
  MaxPath(maxPath ? maxPath : _MAX_PATH)
{
  FileName = new tchar[MaxPath];
  *FileName = 0;
  SetFilter(filter);

  // OFN_EXPLORER implies long names, but we include OFN_LONGNAMES in case OFN_EXPLORER is turned
  // off by user code later.
  //
  Flags |= OFN_EXPLORER | OFN_LONGNAMES;
}

//
/// String-aware overload
/// TODO: Add string support for all parameters.
//
TOpenSaveDialog::TData::TData(
  uint32 flags,
  const tstring& filter,
  LPTSTR, // customFilter, // Dummy for backward compatibility.
  LPCTSTR initialDir,
  LPCTSTR defExt,
  int maxPath,
  int filterIndex,
  uint32 flagsEx
  )
  : Flags(flags),
  FlagsEx(flagsEx),
  Error(0),
  FileName(nullptr),
  Filter(nullptr),
  FilterIndex(filterIndex),
  InitialDir(initialDir),
  DefExt(defExt),
  MaxPath(maxPath ? maxPath : _MAX_PATH)
{
  FileName = new tchar[MaxPath];
  *FileName = 0;
  SetFilter(filter);
  Flags |= OFN_EXPLORER | OFN_LONGNAMES;
}

//
//
//
TOpenSaveDialog::TData::TData(const TData& src)
  : Flags(src.Flags),
  FlagsEx(src.FlagsEx),
  Error(src.Error),
  FileName(nullptr),
  Filter(nullptr),
  FilterIndex(src.FilterIndex),
  InitialDir(src.InitialDir),
  DefExt(src.DefExt),
  MaxPath(src.MaxPath)
{
  FileName = strnewdup(src.FileName, MaxPath);
  SetFilter(src.Filter);
}

//
/// Destructs a TOpenSaveDialog::TData structure.
//
TOpenSaveDialog::TData::~TData()
{
  delete[] FileName;
  delete[] Filter;
}

//
//
//
TOpenSaveDialog::TData&
TOpenSaveDialog::TData::operator =(const TData& src)
{
  Flags = src.Flags;
  FlagsEx = src.FlagsEx;
  Error = src.Error;
  FilterIndex = src.FilterIndex;
  InitialDir = src.InitialDir;
  DefExt = src.DefExt;
  MaxPath = src.MaxPath;

  delete[] FileName;
  FileName = strnewdup(src.FileName, MaxPath);

  SetFilter(src.Filter);

  return *this;
}

//
/// Makes a copy of the filter list used to display the file names.
//
/// Set the file list box filter strings. Translates '|'s into 0s so that the
/// string can be kept as a resource with imbeded '|'s like:
/// \code
/// "Text Files(*.txt)|*.TXT|All Files(*.*)|*.*|"
/// \endcode
/// Can also handle already processed filter strings.
//
void
TOpenSaveDialog::TData::SetFilter(LPCTSTR filter)
{
  // Copy filter string
  //
  if (filter) {
    delete[] Filter;
    if (_tcschr(filter, _T('|'))) {
      uint len = static_cast<uint>(::_tcslen(filter)) + 2; // one for each terminating 0
      Filter = ::_tcscpy(new tchar[len], filter);
      Filter[len-1] = 0;             // in case trailing '|' is missing
    }
    else {
      LPCTSTR p = filter;
      while (*p)
        p += ::_tcslen(p) + 1;             // scan for 00 at end
      uint len = uint(p - filter) + 1;  // one more for last 0
      Filter = new tchar[len];
      memcpy(Filter, filter, sizeof(tchar) * len);
    }
  }
  // Stomp |s with 0s
  //
  if (Filter) {
    for (TCharIterator<tchar> i(Filter); ; i++) {
      if (!*i.Current())
        break;
      if (*i.Current() == _T('|'))
        *i.Current() = 0;
    }
  }
}


//----------------------------------------------------------------------------

DEFINE_RESPONSE_TABLE1(TOpenSaveDialog, TCommonDialog)
END_RESPONSE_TABLE;

const uint TOpenSaveDialog::ShareViMsgId = ::RegisterWindowMessage(SHAREVISTRING);


//
/// Initializes a TOpenSaveDialog object with the current resource ID.
//
void
TOpenSaveDialog::Init(TResId templateId)
{
  memset(&ofn, 0, sizeof(OPENFILENAME));
#if defined(OPENFILENAME_SIZE_VERSION_400)
  ofn.lStructSize = TSystem::GetMajorVersion() < 5 ?
                    OPENFILENAME_SIZE_VERSION_400 : sizeof(OPENFILENAME);
#else
  ofn.lStructSize = sizeof(OPENFILENAME);
#endif
  ofn.hwndOwner = GetParentO() ? GetParentO()->GetHandle() : nullptr;
  ofn.hInstance = *GetModule();
  ofn.Flags = OFN_ENABLEHOOK | Data.Flags;
  if (templateId) {
    ofn.lpTemplateName = templateId;
    ofn.Flags |= OFN_ENABLETEMPLATE;
  }
  else
    ofn.Flags &= ~OFN_ENABLETEMPLATE;
#if WINVER >= 0x500 && !defined(WINELIB)
  ofn.FlagsEx = Data.FlagsEx;
#endif
  ofn.lpfnHook = nullptr;

  ofn.lpstrFilter = Data.Filter;
  ofn.nFilterIndex = Data.FilterIndex;
  ofn.lpstrFile = Data.FileName;
  ofn.nMaxFile = Data.MaxPath;
  ofn.lpstrInitialDir = Data.InitialDir;
  ofn.lpstrDefExt = Data.DefExt;
}

//
/// Constructs an open save dialog box object with the supplied parent window, data,
/// resource ID, title, and current module object.
//
TOpenSaveDialog::TOpenSaveDialog(TWindow* parent, TData& data, TModule*   module)
:
  TCommonDialog(parent, nullptr, module),
  Data(data)
{
}

//
//
//
TOpenSaveDialog::TOpenSaveDialog(TWindow*        parent,
                                 TData&          data,
                                 TResId          templateId,
                                 LPCTSTR         title,
                                 TModule*        module)
:
  TCommonDialog(parent, nullptr, module),
  Data(data),
  Title(title ? title : _T(""))
{
  Init(templateId);
  ofn.lpstrTitle = title ? &Title[0] : nullptr;
}

//
/// String-aware overload
//
TOpenSaveDialog::TOpenSaveDialog(
  TWindow* parent,
  TData& data,
  TResId templateId,
  const tstring& title,
  TModule* module)
:
  TCommonDialog(parent, nullptr, module),
  Data(data),
  Title(title)
{
  Init(templateId);
  ofn.lpstrTitle = &Title[0];
}

//
/// Handles CDN_SHAREVIOLATION and the older SHAREVISTRING registered message.
/// Calls the base implementation to handle all other messages.
//
INT_PTR
TOpenSaveDialog::DialogFunction(TMsgId msg, TParam1 param1, TParam2 param2)
{
  const bool explorerStyle = ofn.Flags & OFN_EXPLORER;

  // First check for the new message.
  //
  TNotify* n = (msg == WM_NOTIFY) ? reinterpret_cast<TNotify*>(param2) : nullptr;
  if (n && n->code == CDN_SHAREVIOLATION)
  {
    WARN(!explorerStyle, _T("CDN_SHAREVIOLATION was sent to old-style dialog."));
    int r = ShareViolation();

    // The documentation for CDN_SHAREVIOLATION states that 0 should be
    // returned to get the standard warning message for a sharing violation.
    // It does not list OFN_SHAREWARN as a valid DWL_MSGRESULT value.
    // Note that this causes the old SHAREVISTRING to be sent; see below.
    //
    if (r == OFN_SHAREWARN)
      return 0;

    CHECK(!OWL_STRICT || r == OFN_SHAREFALLTHROUGH || r == OFN_SHARENOWARN);
    WARN(r != OFN_SHAREFALLTHROUGH && r != OFN_SHARENOWARN, _T("ShareViolation returned undefined value: ") << r);
    return SetMsgResult(r);
  }

  // Now check for the old message.
  //
  else if (msg == TOpenSaveDialog::ShareViMsgId)
  {
    int r = ShareViolation();

    // Do not handle SHAREVISTRING for Explorer-style dialogs.
    // The message is a result of the default handling for CDN_SHAREVIOLATION, i.e.
    // when CDN_SHAREVIOLATION returns 0 then SHAREVISTRING is sent.
    //
    if (explorerStyle)
    {
      WARN(r != OFN_SHAREWARN, _T("ShareViolation returned unexpected value in Explorer-style dialog:") << r);
      return 0;
    }

    CHECK(!OWL_STRICT || r == OFN_SHAREFALLTHROUGH || r == OFN_SHARENOWARN || r == OFN_SHAREWARN);
    WARN(r != OFN_SHAREFALLTHROUGH && r != OFN_SHARENOWARN && r != OFN_SHAREWARN,
      _T("ShareViolation returned undefined value: ") << r);

    // The documentation for SHAREVISTRING states that the result should be
    // returned directly, rather than through DWL_MSGRESULT.
    //
    return r;
  }

  // If we get here, pass the message on to the base.
  //
  return TCommonDialog::DialogFunction(msg, param1, param2);
}

//
/// If a sharing violation occurs when a file is opened or saved, ShareViolation is
/// called to obtain a response. The default return value is OFN_SHAREWARN. Other
/// sharing violation responses are listed in the following table.
/// - \c \b  OFN_SHAREFALLTHROUGH Specifies that the file name can be used and that the
/// dialog box should return it to the application.
/// - \c \b  OFN_SHARENOWARN Instructs the dialog box to perform no further action with
/// the file name and not to warn the user of the situation.
/// - \c \b  OFN_SHAREWARN This is the default response that is defined as 0. Instructs the
/// dialog box to display a standard warning message.
//
int
TOpenSaveDialog::ShareViolation()
{
  return OFN_SHAREWARN;
}

//----------------------------------------------------------------------------

//
/// Constructs and initializes the TFileOpen object based on information in the
/// TOpenSaveDialog::TData data structure. The parent argument points to the dialog
/// box's parent window. data is a reference to the TData object. templateID is the
/// ID for a custom template. title is an optional title. module points to the
/// module instance.
//
TFileOpenDialog::TFileOpenDialog(TWindow*        parent,
                                 TData&          data,
                                 TResId          templateId,
                                 LPCTSTR         title,
                                 TModule*        module)
:
  TOpenSaveDialog(parent, data, templateId, title, module)
{
}

//
/// Creates the TFileOpenDialog object.
//
int
TFileOpenDialog::DoExecute()
{
  ofn.lpfnHook = LPOFNHOOKPROC(StdDlgProc);
  int ret = GetOpenFileName(&ofn);
  Data.Error = CommDlgExtendedError();
  if (ret) {
    Data.Flags = ofn.Flags;
    Data.FilterIndex = static_cast<int>(ofn.nFilterIndex);
  }
  return ret ? IDOK : IDCANCEL;
}


//----------------------------------------------------------------------------

//
/// Constructs and initializes the TFileOpen object based on the
/// TOpenSaveDialog::TData  structure, which contains information about the file
/// name, file directory, and file name search filers.
//
TFileSaveDialog::TFileSaveDialog(TWindow*        parent,
                                 TData&          data,
                                 TResId          templateId,
                                 LPCTSTR         title,
                                 TModule*        module)
:
  TOpenSaveDialog(parent, data, templateId, title, module)
{
}

//
/// Creates the TFileSaveDialog object.
//
int
TFileSaveDialog::DoExecute()
{
  ofn.lpfnHook = LPOFNHOOKPROC(StdDlgProc);
  int ret = GetSaveFileName(&ofn);
  Data.Error = CommDlgExtendedError();
  if (ret) {
    Data.Flags = ofn.Flags;
    Data.FilterIndex = ofn.nFilterIndex;
  }
  return ret ? IDOK : IDCANCEL;
}


//
//
//
void
TOpenSaveDialog::TData::Read(ipstream& is)
{
  is >> Flags;
#if defined(UNICODE)
  _USES_CONVERSION;

  char* fileName      = is.readString();
  char* filter        = is.freadString();
  char* customFilter  = is.freadString(); // Dummy for backward compatibility.
  if (customFilter) delete [] customFilter; // Discard dummy.

  FileName = strnewdup(_A2W(fileName));
  Filter = strnewdup(_A2W(filter));

  delete[] fileName;
  delete[] filter;

  is >> FilterIndex;

  char* initDir = is.freadString();
  char* defExt  = is.freadString();

  InitialDir = strnewdup(_A2W(initDir));
  DefExt = strnewdup(_A2W(defExt));

  delete[] initDir;
  delete[] defExt;

#else
  FileName = is.readString();
  Filter = is.freadString();
  const auto customFilter = is.freadString(); // Dummy for backward compatibility.
  if (customFilter) delete[] customFilter; // Discard dummy.
  is >> FilterIndex;
  InitialDir = is.freadString();
  DefExt = is.freadString();
#endif
  is >> FlagsEx;
}

//
//
//
void
TOpenSaveDialog::TData::Write(opstream& os)
{
  _USES_CONVERSION;

  os << Flags;
  os.writeString(_W2A(FileName));
  os.fwriteString(_W2A(Filter));
  const auto customFilter = nullptr; // Dummy for backward compatibility.
  os.fwriteString(customFilter);
  os << FilterIndex;
  os.fwriteString(_W2A(InitialDir));
  os.fwriteString(_W2A(DefExt));
  os << FlagsEx;
}

//
/// Returns a vector containing the full paths of every file entry in member FileName.
/// This function is only useful in conjunction with a multi-selection TFileOpenDialog using the
/// Explorer style.
///
/// \note It is assumed that the buffer pointed to by member FileName contains a null-separated
/// list of file names terminated by two null characters, as will be the case if filled in by a
/// multi-selection Explorer-style TFileOpenDialog. Note that this function does not work unless
/// the flags OFN_EXPLORER and OFN_ALLOWMULTISELECT were both set!
///
/// Usage:
///
/// \code
/// void TBmpViewWindow::CmFileOpen()
/// {
///   TOpenSaveDialog::TData data
///   {
///     OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST, // flags
///     "Bitmap Files (*.bmp)|*.bmp", // filter
///     nullptr, // customFilter
///     nullptr, // initialDir
///     nullptr, // defExt
///     65536 // maxPath (ample buffer space for multiple selections)
///   };
///   TFileOpenDialog dlg{this, data};
///   const auto r = dlg.Execute();
///   if (CommDlgExtendedError() == FNERR_BUFFERTOOSMALL)
///   {
///     MessageBox("Buffer too small!", "File Open Error", MB_OK | MB_ICONEXCLAMATION);
///     return;
///   }
///   if (r == IDOK)
///     for (const auto& f : data.GetFileNames())
///       OpenFile(f);
/// }
/// \endcode
///
/// \sa http://docs.microsoft.com/en-us/windows/desktop/api/Commdlg/ns-commdlg-tagofna
//
auto TOpenSaveDialog::TData::GetFileNames() const -> std::vector<tstring>
{
  PRECONDITION((Flags & OFN_EXPLORER) && (Flags & OFN_ALLOWMULTISELECT));

  auto v = std::vector<tstring>{};
  auto p = FileName;
  const auto d = tstring{p};
  p += d.size() + 1;
  CHECK(static_cast<int>(p - FileName) < MaxPath);

  // NOTE: We assume double null-termination here, even for a single file!
  //
  if (*p == _T('\0'))
  {
    // We have a single entry, being the full path to the file.
    //
    v.push_back(d);
  }
  else
  {
    // We have multiple entries following the directory path, each being the bare file name without a path.
    // Compose the full path to each file by prepending the directory path.
    //
    while (*p != _T('\0'))
    {
      const auto n = tstring{p};
      const auto f = d + _T('\\') + n;
      v.push_back(f);
      p += n.size() + 1;
      CHECK(static_cast<int>(p - FileName) < MaxPath);
    }
  }
  return v;
}

} // OWL namespace
/* ========================================================================== */

