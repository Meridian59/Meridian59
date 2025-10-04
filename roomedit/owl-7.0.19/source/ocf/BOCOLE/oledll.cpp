//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
// $Revision: 1.2 $
//
// OleDLL.cpp -- DLL entry/exit points for BOLE.DLL
//----------------------------------------------------------------------------
#include "bole32.h"
#include <windows.h>
#include "ole2ui.h"

HINSTANCE boleInst;


OLEDBGDATA_MAIN ("BOLE")

extern "C" int FAR PASCAL LibMain( HINSTANCE hInst, WORD, WORD, LPSTR )
{
  boleInst = hInst;
  OleUIInitialize (boleInst, NULL);
  return 1;
}

extern "C" int FAR PASCAL WEP(int /*systemExit*/)
{
  OleUIUninitialize ();
  return 1;
}

extern "C" BOOL WINAPI DllEntryPoint( HINSTANCE hInst, DWORD dwReason, LPVOID )
{
  switch  (dwReason) {
    case DLL_PROCESS_ATTACH:
      LibMain(hInst, NULL, NULL, NULL);
      break;
    case DLL_PROCESS_DETACH:
      WEP(NULL);
      break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
  }

  return 1;
}



