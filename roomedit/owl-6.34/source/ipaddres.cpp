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
DIAG_DECLARE_GROUP(OwlCommCtrl);        // CommonCtrl Diagnostic group

//
//
//
TIPAddress::TIPAddress(TWindow* parent, int id, int x, int y, int w, int h,
                       TModule* module)
:
  TControl(parent,id,_T(""),x,y,w,h,module)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionIE3)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_INTERNET_CLASSES );
}


//
//
//
TIPAddress::TIPAddress(TWindow* parent, int resourceId, TModule* module)
:
  TControl(parent, resourceId, module)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionIE3)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_INTERNET_CLASSES );
}

//
/// Constructs a IP address object to encapsulate (alias) an existing control.
//
TIPAddress::TIPAddress(THandle hWnd, TModule* module)
:
  TControl(hWnd, module)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionIE3)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_INTERNET_CLASSES );
}

//
//
//
TIPAddress::~TIPAddress()
{
}


// Return the proper class name.
// Windows class: WC_PAGESCROLLER is defined in commctrl.h
TWindow::TGetClassNameReturnType
TIPAddress::GetClassName()
{
  return WC_IPADDRESS;
}


//
//
//
uint
TIPAddress::Transfer(void* buffer, TTransferDirection direction)
{

   TIPAddressBits* ipData = (TIPAddressBits*)buffer;
  if (direction == tdGetData)
    GetAddress(*ipData);
  else if (direction == tdSetData)
    SetAddress(*ipData);

  return sizeof(TIPAddressBits);
}
} // OWL namespace
/* ========================================================================== */
