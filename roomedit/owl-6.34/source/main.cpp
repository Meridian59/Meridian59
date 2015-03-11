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

// link owl libraries
//#include <owl/private/owllink.h>

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

  HINSTANCE hInstance = ::GetModuleHandle(NULL);
  InitGlobalModule(hInstance);

  LPCTSTR cmdLine = GetCommandLine();
  owl::TApplication::SetWinMainParams(hInstance, 0, 
                        cmdLine?cmdLine:_T(""), SW_SHOWNORMAL);

  try {
    int retVal = OwlMain(argc, argv);
    TRACEX(OwlMain, 0, _T("WinMain() returns ") << retVal);
    return retVal;
  }
  catch (owl::TXEndSession&) {return 0;} 
  catch (owl::TXBase& x) {return owl::HandleGlobalException(x, 0);} 
}

/* ========================================================================== */

