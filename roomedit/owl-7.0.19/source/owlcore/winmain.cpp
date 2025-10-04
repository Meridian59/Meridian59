//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of WinMain for user exes
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/applicat.h>
#include <owl/lclstrng.h>
#include <owl/gdiplus.h>

#if defined(BI_COMP_BORLANDC)
#include <dos.h>
#endif

#if defined(BI_COMP_GNUC) || defined(BI_COMP_CLANG)

// We need some headers for our command line parser.
//
#include <vector>
#include <algorithm>

#endif

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlMain);

} // OWL namespace

using namespace std;
using namespace owl;

#if defined(BI_COMP_GNUC) || defined(BI_COMP_CLANG)

namespace
{

  // Parse the command line into an argv-style string list.
  // Note: Does not handle escape characters.
  //
  auto ParseCmdLine_(const tstring& cmd) -> vector<tstring>
  {
    using TIt = tstring::const_iterator;
    const auto quote = _T('\"');

    auto parseQuotedString = [&](TIt& i, TIt end) -> tstring
    {
      PRECONDITION(i != end && *i == quote);
      const auto e = find(++i, end, quote);
      if (e == end) throw TXOwl{_T("Unpaired quote in command-line.")};
      const auto s = tstring{i, e};
      i = next(e);
      return s;
    };

    auto parseString = [](TIt& i, TIt end) -> tstring
    {
      PRECONDITION(i != end);
      const auto e = find_if(i, end, [](tchar c) { return _istspace(c); });
      const auto s = tstring{i, e};
      i = e;
      return s;
    };

    auto a = vector<tstring>{};
    for (auto i = cmd.begin(); i != cmd.end();)
    {
      auto c = *i;
      if (_istspace(c))
        ++i;
      else if (c == quote)
        a.push_back(parseQuotedString(i, cmd.end()));
      else
        a.push_back(parseString(i, cmd.end()));
    }
    return a;
  };

} // namespace

#endif

// MSC code in module.cpp
#if !defined(_BUILDOWLDLL) && !defined(_OWLDLL)
#if defined(BI_COMP_BORLANDC)
// NOTE: We use priority 31 to come just before/after ctr/dtr of global
//       objects (which are assigned a priorority of 32)
//
long TlsAddRefs();
long TlsRelease();

static void __initOWL()
{
  TlsAddRefs();
}
#pragma startup __initOWL 31

//
static void __termOWL()
{
  TlsRelease();
}
#pragma exit __termOWL 31
#endif // BI_COMP_BORLANDC
#endif // if !_BUILDOWLDLL

using owl::TModule;
using owl::TApplication;

#if defined(BI_COMP_BORLANDC) && defined(_UNICODE)

extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR cmdLine, int cmdShow);

#elif defined(BI_COMP_GNUC)

extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR cmdLine, int cmdShow);

#endif

//
// Default WinMain calls OwlMain, after setting params into TApplication
//
int WINAPI
_tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR cmdLine, int cmdShow)
{
  TRACEX(OwlMain, 0, _T("WinMain(")
    << hex << static_cast<void*>(hInstance) << _T(", ")
    << hex << static_cast<void*>(hPrevInstance) << _T(", \"")
    << TResId(cmdLine) << _T("\", ")
    << cmdShow << _T(") called"));

  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  InitGlobalModule(hInstance);

  owl::TApplication::SetWinMainParams(hInstance, hPrevInstance, cmdLine ? cmdLine : _T(""), cmdShow); //LB
  int retVal;
  try
  {

#if defined(BI_COMP_GNUC) || defined(BI_COMP_CLANG)

    // Create an argv-style array of string pointers to the command line arguments.
    //
    auto a = ParseCmdLine_(::GetCommandLine());
    auto argv = vector<LPTSTR>{a.size()};
    transform(a.begin(), a.end(), argv.begin(), [](tstring& s) {return &s[0];});
    retVal = OwlMain(argv.size(), &argv[0]);

#elif defined(BI_COMP_BORLANDC)

    retVal = OwlMain(_argc, _targv);

#else

    retVal = OwlMain(__argc, __targv);

#endif

  }
  catch (owl::TXEndSession&) {retVal = 0;}
  catch (owl::TXBase& x) {retVal = owl::HandleGlobalException(x, 0);}

  Gdiplus::GdiplusShutdown(gdiplusToken);
  TRACEX(OwlMain, 0, _T("WinMain() returns ") << retVal);
  return retVal;
}
