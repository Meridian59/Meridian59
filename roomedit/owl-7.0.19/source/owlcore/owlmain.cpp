//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Default OwlMain to satisfy LibMain in DLLs that aren't components
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/private/checks.h>

OWL_DIAGINFO;

using namespace owl;

int
OwlMain(int argc, _TCHAR* argv[])
{
  // This OwlMain should be called only when the user builds a DLL.
  // If it's called from an EXE, that means the user did not provide
  // an OwlMain with the proper prototype.  Issue a warning.
  //
  // When LibMain/DllEntryPoint calls this OwlMain, both arguments
  // are 0.  If either argument is non-zero, an EXE is running.
  //
  PRECONDITION(argc == 0);
  PRECONDITION(argv == 0);

  if (argc != 0 && argv != 0)
    ::MessageBox(0, _T("You have accidently used the dummy version of OwlMain."), _T("Error"), MB_ICONHAND | MB_OK);
  return 0;
}

