//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of WinMain for user exes
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/applicat.h>
#include <owl/lclstrng.h>
#include <owl/gdiplus.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlMain);

} // OWL namespace

using namespace owl;

//
// Default WinMain calls OwlMain, after setting params into TApplication
//
int
_tmain(int argc, tchar* argv[])
{
  TRACEX(OwlMain, 0, _T("main() called"));

  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  HINSTANCE hInstance = ::GetModuleHandle(NULL);
  InitGlobalModule(hInstance);

  LPCTSTR cmdLine = GetCommandLine();
  owl::TApplication::SetWinMainParams(hInstance, 0, cmdLine ? cmdLine : _T(""), SW_SHOWNORMAL);
  int retVal;
  try {
    retVal = OwlMain(argc, argv);
    TRACEX(OwlMain, 0, _T("WinMain() returns ") << retVal);
  }
  catch (owl::TXEndSession&) {return retVal = 0;}
  catch (owl::TXBase& x) {return retVal = owl::HandleGlobalException(x, 0);}

  Gdiplus::GdiplusShutdown(gdiplusToken);
  return retVal;
}
