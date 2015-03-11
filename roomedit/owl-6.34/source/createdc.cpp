//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of classes TCreatedDC, and TIC
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/dc.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlGDI);

//
/// Creates a device context (DC) for the given device. DC objects can be
/// constructed either by borrowing an existing HDC handle or by supplying device
/// and driver information.
//
TCreatedDC::TCreatedDC()
:
  TDC()
{
  TRACEX(OwlGDI, OWL_CDLEVEL, _T("TCreatedDC constructed @") << (void*)this);
}

//
/// Creates a device context (DC) object for the device specified by driver
/// (driver name), device (device name), and output (the name of the file or device
/// [port] for the physical output medium). The optional initData argument provides
/// a DEVMODE structure containing device-specific initialization data for this DC.
/// initData must be 0 (the default) if the device is to use any default
/// initializations specified by the user.
//
TCreatedDC::TCreatedDC(LPCTSTR driver, LPCTSTR device,
                       LPCTSTR output, const DEVMODE * initData)
:
  TDC()
{
  Handle = ::CreateDC(driver, device, output, initData);
  CheckValid();
  TRACEX(OwlGDI, OWL_CDLEVEL, _T("TCreatedDC constructed @") << (void*)this <<
    _T(" with driver ") << tstring(driver).c_str() );
}

//
/// String-aware overload
//
TCreatedDC::TCreatedDC(const tstring& driver, const tstring& device, const tstring& output, const DEVMODE* initData)
:
  TDC()
{
  Handle = ::CreateDC(
    driver.empty() ? 0 : driver.c_str(), 
    device.empty() ? 0 : device.c_str(),
    output.empty() ? 0 : output.c_str(), 
    initData);
  CheckValid();
  TRACEX(OwlGDI, OWL_CDLEVEL, _T("TCreatedDC constructed @") << (void*)this <<
    _T(" with driver ") << tstring(driver).c_str() );
}

//
/// Creates a DC object using an existing DC.
//
TCreatedDC::TCreatedDC(HDC handle, TAutoDelete autoDelete)
:
  TDC(handle, autoDelete)
{
  TRACEX(OwlGDI, OWL_CDLEVEL, _T("TCreatedDC constructed @") << (void*)this <<
    _T(" with handle ") << uint(handle));
}

//
/// Destructor
/// Calls RestoreObjects clears any nonzero OrgXXX data members. If ShouldDelete is
/// true the destructor deletes this DC.
//
TCreatedDC::~TCreatedDC()
{
  RestoreObjects();
  if (ShouldDelete && Handle)
    ::DeleteDC(STATIC_CAST(HDC,Handle));
  TRACEX(OwlGDI, OWL_CDLEVEL, _T("TCreatedDC destructed @") << (void*)this);
}

//
/// Creates a DC object with the given driver, device, and port names and
/// initialization values.
//
TIC::TIC(LPCTSTR driver, LPCTSTR device,
         LPCTSTR output, const DEVMODE * initData)
:
  TCreatedDC()
{
  Handle = STATIC_CAST(HANDLE,::CreateIC(driver, device, output, initData));
  CheckValid();
  TRACEX(OwlGDI, OWL_CDLEVEL, _T("TIC constructed @") << (void*)this <<
    " with driver " << tstring(driver).c_str());
}

//
/// String-aware overload
//
TIC::TIC(const tstring& driver, const tstring& device, const tstring& output, const DEVMODE* initData)
:
  TCreatedDC()
{
  HDC h = ::CreateIC(
    driver.empty() ? 0 : driver.c_str(), 
    device.empty() ? 0 : device.c_str(),
    output.empty() ? 0 : output.c_str(), 
    initData);
  Handle = STATIC_CAST(HANDLE, h);
  CheckValid();
  TRACEX(OwlGDI, OWL_CDLEVEL, _T("TIC constructed @") << (void*)this <<
    " with driver " << tstring(driver).c_str());
}

//
//
//
TIC::~TIC()
{
  TRACEX(OwlGDI, OWL_CDLEVEL, _T("TIC destructed @") << (void*)this);
}

} // OWL namespace
/* ========================================================================== */
