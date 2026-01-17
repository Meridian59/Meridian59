//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TCommCtrlDll, a wrapper for the common control dll
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/commctrl.h>

namespace owl {

// Common Controls diagnostic group
OWL_DIAGINFO;

DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlCommCtrl, 1, 0);

void InitializeCommonControls(uint controlFlags)
{
  const auto a = INITCOMMONCONTROLSEX{sizeof(INITCOMMONCONTROLSEX), controlFlags};
  if (!InitCommonControlsEx(&a))
    throw TXCommCtrl{};
}

auto GetCommCtrlVersion() -> DWORD
{
  TModule m{_T("comctl32.dll"), true, true, false}; // shouldLoad, mustLoad, addToList
  auto getVersion = TModuleProc1<HRESULT, DLLVERSIONINFO*>{m, "DllGetVersion"};
  auto i = DLLVERSIONINFO{sizeof(DLLVERSIONINFO)};
  const auto r = getVersion(&i);
  CHECK(SUCCEEDED(r)); InUse(r);
  return MAKELONG(i.dwMinorVersion, i.dwMajorVersion);
}

//----------------------------------------------------------------------------

//
/// Constructor of the Object thrown by the CommonControl wrappers of ObjectWindows.
//
TXCommCtrl::TXCommCtrl()
:
  TXOwl(IDS_COMMCTRL)
{
}

//
/// Route to copy an exception object. The 'Clone' method is invoked when
/// an exception must be suspended. The cloned copy can latter be used to
/// resume the exception.
//
TXCommCtrl*
TXCommCtrl::Clone()
{
  return new TXCommCtrl(*this);
}


//
/// Method to localize the actual call to 'throw' an TXCommCtrl object.
/// \note Localizing the call to 'throw' is a size optimization: the
///       overhead of a 'throw' statement is generated only once.
//
void
TXCommCtrl::Throw()
{
  throw *this;
}

//
/// Constructs a TXCommCtrl exception from scratch, and throws it.
//
void
TXCommCtrl::Raise()
{
  TXCommCtrl().Throw();
}

} // OWL namespace

