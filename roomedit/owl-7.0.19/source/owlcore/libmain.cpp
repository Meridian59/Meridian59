//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Default implementation of DLlEntryPoint()/LibMain() for user DLLs
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/applicat.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlMain);

} // OWL namespace

using namespace owl;

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

#if defined(BI_COMP_MSC)
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD reason, LPVOID)
#else
int WINAPI DllEntryPoint(HINSTANCE hInstance, uint32 reason, LPVOID)
#endif
{
  TRACEX(OwlMain, 0, "DllEntryPoint(" << hInstance << ", " << reason << ", " <<
         ") called");

  switch (reason) {
    case DLL_PROCESS_ATTACH:

      // If you provide your own version of DLL EntryPoint, make sure
      // you call OwlInitUserDLL to allow OWL to initialize '::Module'
      //
      return OWLInitUserDLL(hInstance, _T("")) ? 1 : 0;
  }
  return 1;
}

