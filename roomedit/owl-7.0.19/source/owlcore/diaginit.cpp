//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of diagnostic initialization functions. Settings are stored
/// in a profile (name is provided) in a section named "Diagnostics". Profile
/// format in an ascii .ini file looks like:
/// \code
///  [Diagnostics]
///  Enabled = <enabled>
///  <groupname1> = <enabled> <level>
///  <groupname2> = <enabled> <level>
///    ...
///  <groupnameN> = <enabled> <level>
/// \endcode
/// where:
/// \code
///  <enabled> = 0|1
///  <level>   = 0..255
/// \endcode
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/profile.h>
#include <owl/shellitm.h>
#include <owl/filename.h>
#include <sstream>

using namespace owl;
using namespace std;

namespace owl {

namespace
{

  const tchar DiagnosticsSection[] = _T("Diagnostics");

  struct TGroupSettings
  {
    int Enabled;
    int Level;

    // Parses the settings for the given group in the given profile.
    // The format of the setting is "<int Enabled> <int Level>", e.g. "1 3".
    // If no setting is found, or parsing fails, the given defaults are used.
    //
    TGroupSettings(TProfile& p, const tstring& group, int defEnabled, int defLevel = 0)
      : Enabled(defEnabled), Level(defLevel)
    {
      tistringstream is(p.GetString(group));
      is >> Enabled >> Level;
    }
  };

  TFileName GetRoamingAppDataFolderPath_()
  {
    PIDLIST_ABSOLUTE pidl = {};
    const HRESULT r = TShell::SHGetSpecialFolderLocation(nullptr, CSIDL_APPDATA, &pidl);
    CHECKX(SUCCEEDED(r), _T("SHGetSpecialFolderLocation failed")); InUse(r);
    tchar buf[_MAX_PATH];
    const bool ok = TShell::SHGetPathFromIDList(pidl, &buf[0]);
    CHECKX(ok, _T("SHGetPathFromIDList failed")); InUse(ok);
    const TFileName f(buf, true);
    CoTaskMemFree(pidl);
    return f;
  };

} // namespace

//
// Returns the full path for the given file.
// If the given file has no path, it is assumed to be located in "%appdata%\OWLNext".
// This is a private function. However, it is defined in the `owl` namespace, since it is reused
// internally by TTraceWindow in "tracewnd.cpp".
//
tstring GetDiagIniFullFileName_(LPCSTR filename_)
{
  _USES_CONVERSION;
  tstring filename(_A2W(filename_));
  TFileName f(filename);
  if (f.HasParts(TFileName::Server) || f.HasParts(TFileName::Device) || f.HasParts(TFileName::Path))
    return f.Canonical();

  TFileName d = GetRoamingAppDataFolderPath_().AddSubDir(_T("OWLNext"));
  if (!d.Exists())
    d.CreateDir();
  return d.MergeParts(TFileName::File | TFileName::Ext, f).Canonical();
}

_OWLFUNC(bool)
GetDiagEnabled(LPCSTR filename, LPCSTR diagGroup, bool defEnabled)
{
  _USES_CONVERSION;
  TProfile p(DiagnosticsSection, GetDiagIniFullFileName_(filename));

  bool globallyEnabled = p.GetInt(_T("Enabled"), 1) != 0;
  if (!globallyEnabled) return false;

  TGroupSettings s(p, _A2W(diagGroup), defEnabled ? 1 : 0);
  return s.Enabled != 0;
}

_OWLFUNC(int)
GetDiagLevel(LPCSTR filename, LPCSTR diagGroup, int defLevel)
{
  _USES_CONVERSION;
  TProfile p(DiagnosticsSection, GetDiagIniFullFileName_(filename));
  TGroupSettings s(p, _A2W(diagGroup), 0, defLevel);
  return s.Level;
}

} // OWL namespace

