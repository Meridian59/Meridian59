//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Default OwlMain to satisfy LibMain in DLLs that aren't components
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/module.h>
#include <owl/applicat.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlMain);

} // OWL namespace

using namespace owl;

/// Routine which initializes an OWL User DLL..
/// \note Must be explicitly invoked if your DLL provides it's own
///       LibMain, DllEntryPoint (or DllMain).
//
bool
OWLInitUserDLL(HINSTANCE hInstance, LPTSTR cmdLine)
{
  // If no global ::Module ptr exists yet, provide a default now
  //
  InitGlobalModule(hInstance);

  // Store away information about this instance
  //
  TApplication::SetWinMainParams(hInstance, 0, 
                cmdLine?cmdLine:_T(""), SW_SHOWNORMAL);

  try {
    int retVal = OwlMain(0, 0);
    TRACEX(OwlMain, 0, _T("DllEntryPoint() returns ") << retVal);
    return retVal == 0;
  }
  catch (TXEndSession&) {return true;} 
  catch (TXBase& x) {return owl::HandleGlobalException(x, 0) == 0;} 
}

/* ========================================================================== */

