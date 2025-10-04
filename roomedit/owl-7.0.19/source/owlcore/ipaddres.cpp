//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Bidus Yura, All Rights Reserved
//
/// \file
/// Implementation of the TIPAddress class
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/ipaddres.h>


namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommCtrl);        // Common Controls diagnostic group

//
//
//
TIPAddress::TIPAddress(TWindow* parent, int id, int x, int y, int w, int h,
                       TModule* module)
:
  TControl(parent,id,_T(""),x,y,w,h,module)
{
  InitializeCommonControls(ICC_INTERNET_CLASSES);
}


//
//
//
TIPAddress::TIPAddress(TWindow* parent, int resourceId, TModule* module)
:
  TControl(parent, resourceId, module)
{
  InitializeCommonControls(ICC_INTERNET_CLASSES);
}

//
/// Constructs a IP address object to encapsulate (alias) an existing control.
//
TIPAddress::TIPAddress(THandle hWnd, TModule* module)
:
  TControl(hWnd, module)
{
  InitializeCommonControls(ICC_INTERNET_CLASSES);
}

//
//
//
TIPAddress::~TIPAddress()
{
}

auto TIPAddress::GetWindowClassName() -> TWindowClassName
{
  return TWindowClassName{WC_IPADDRESS};
}


//
//
//
uint
TIPAddress::Transfer(void* buffer, TTransferDirection direction)
{
  if (!buffer && direction != tdSizeData) return 0;
  TIPAddressBits* ipData = static_cast<TIPAddressBits*>(buffer);
  if (direction == tdGetData)
    GetAddress(*ipData);
  else if (direction == tdSetData)
    SetAddress(*ipData);

  return sizeof(TIPAddressBits);
}
} // OWL namespace
/* ========================================================================== */
