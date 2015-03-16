//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1997 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TSystem class.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/system.h>

#if !defined(PRODUCT_UNDEFINED)
#define PRODUCT_UNDEFINED                           0x00000000
#endif

using namespace owl; //Jogy - without this BCC5.5 cannot properly parse the TModule used in the local struct definition in GetWinProductType() 

namespace owl {

#if defined(OWL5_COMPAT)
//
// Does this OS support tiny caption (via the WS_EX_TOOLWINDOW style)
//
bool TSystem::HasSmallCaption()
{
  return Has3dUI();
}

//
// Does this OS support small window icons
//
bool TSystem::HasSmallIcon()
{
  return Has3dUI();
}

//
bool TSystem::Has3dUI()
{
  return true;
}

//
bool TSystem::IsWin32s() {
  return GetPlatformId() == Win32s;
}

//
bool TSystem::IsNT() {
  return GetPlatformId() == Win32NT;
}

//
bool TSystem::IsWin95() {
  return GetPlatformId() == Win32Windows;
}
#endif  // defined(OWL5_COMPAT)

//
uint TSystem::GetPlatformId() {
  return GetVersionInfo().GetPlatformId();
}

//
TSystem::TProcessor TSystem::GetProcessorType() {
  return GetSystemInfo().GetProcessorType();
}

//
uint TSystem::GetProcessorArchitecture() {
  return GetSystemInfo().GetProcessorArchitecture();
}

//
uint TSystem::GetMajorVersion() {
  return GetVersionInfo().GetMajorVersion();
}

//
uint TSystem::GetMinorVersion() {
  return GetVersionInfo().GetMinorVersion();
}

//
uint TSystem::GetBuildNumber() {
  return GetVersionInfo().GetBuildNumber();
}

//
const tchar* TSystem::GetVersionStr() {
  return GetVersionInfo().GetVersionStr();
}

//
uint TSystem::GetSuiteMask() {
  return GetVersionInfo().GetSuiteMask();
}

//
uint TSystem::GetProductType() {
  return GetVersionInfo().GetProductType();
}

//
bool TSystem::IsXP() {
  return (GetMajorVersion() == 5) && (GetMinorVersion() == 1) ? true : false;
}

//
bool TSystem::IsXP64() {
  return (GetMajorVersion() == 5) && (GetMinorVersion() == 2) &&
    (GetProductType() == VER_NT_WORKSTATION) && (GetProcessorArchitecture() == PROCESSOR_ARCHITECTURE_AMD64) ?
      true : false;
}

//
bool TSystem::IsWin2003() {
  return (GetMajorVersion() == 5) && (GetMinorVersion() == 2) &&
    (::GetSystemMetrics(SM_SERVERR2) == 0) ? true : false;
}

//
bool TSystem::IsHomeSrv() {
  return (GetMajorVersion() == 5) && (GetMinorVersion() == 2) &&
    (GetSuiteMask() == VER_SUITE_WH_SERVER) ? true : false;
}

//
bool TSystem::IsWin2003R2() {
  return (GetMajorVersion() == 5) && (GetMinorVersion() == 2) &&
    (::GetSystemMetrics(SM_SERVERR2) != 0) ? true : false;
}

//
bool TSystem::IsVista() {
  return (GetMajorVersion() == 6) && (GetMinorVersion() == 0) &&
    (GetProductType() == VER_NT_WORKSTATION) ? true : false;
}

//
bool TSystem::IsWin2008() {
  return (GetMajorVersion() == 6) && (GetMinorVersion() == 0) &&
    (GetProductType() != VER_NT_WORKSTATION) ? true : false;
}

//
bool TSystem::IsWin2008R2() {
  return (GetMajorVersion() == 6) && (GetMinorVersion() == 1) &&
    (GetProductType() != VER_NT_WORKSTATION) ? true : false;
}

//
bool TSystem::IsWin7() {
  return (GetMajorVersion() == 6) && (GetMinorVersion() == 1) &&
    (GetProductType() == VER_NT_WORKSTATION) ? true : false;
}

//
bool TSystem::IsWin2012() {
  return (GetMajorVersion() == 6) && (GetMinorVersion() == 2) &&
    (GetProductType() != VER_NT_WORKSTATION) ? true : false;
}

//
bool TSystem::IsWin8() {
  return (GetMajorVersion() == 6) && (GetMinorVersion() == 2) &&
    (GetProductType() == VER_NT_WORKSTATION) ? true : false;
}

//
TSystem::TSystemInfo::TSystemInfo() {
  // no struct size to fill in...
  ::GetSystemInfo(this);
}

//
TSystem::TProcessor TSystem::TSystemInfo::GetProcessorType() const {
  return (TProcessor)dwProcessorType;
}

//
uint TSystem::TSystemInfo::GetProcessorArchitecture() const {
  return wProcessorArchitecture;
}

//
TSystem::TVersionInfo::TVersionInfo() {
  dwOSVersionInfoSize = sizeof *this;
  ::GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(this));
}

//
uint TSystem::TVersionInfo::GetMajorVersion() const {
  return dwMajorVersion;
}

//
uint TSystem::TVersionInfo::GetMinorVersion() const {
  return dwMinorVersion;
}

//
uint TSystem::TVersionInfo::GetBuildNumber() const {
  return dwBuildNumber;
}

//
uint TSystem::TVersionInfo::GetPlatformId() const {
  return dwPlatformId;
}

//
const tchar* TSystem::TVersionInfo::GetVersionStr() const {
  return szCSDVersion;
}

//
uint TSystem::TVersionInfo::GetSuiteMask() const {
  return wSuiteMask;
}

//
uint TSystem::TVersionInfo::GetProductType() const {
  return wProductType;
}

//
// Return an initialized version info object
//
TSystem::TVersionInfo& TSystem::GetVersionInfo()
{
  static TVersionInfo versionInfo;
  return versionInfo;
}

//
// Return an initialized system info object
//
TSystem::TSystemInfo& TSystem::GetSystemInfo()
{
  static TSystemInfo systemInfo;
  return systemInfo;
}

#if defined(OWL5_COMPAT)
//
// Return true if the system has exception suport built in and the runtime
// library uses it for stack unwinding support.
//
bool TSystem::SupportsExceptions()
{
  static bool exceptionOK = GetPlatformId() != Win32s;
  return exceptionOK;
}

//
// Return true if the system has thread suport built in and the runtime
// library uses it.
//
bool TSystem::SupportsThreads()
{
  static bool threadOK = GetPlatformId() != Win32s;
  return threadOK;
}

//
// Return true if the system has suport for per-instance data built in and the
// runtime library uses it.
//
bool TSystem::SupportsInstanceData()
{
  static bool instanceDataOK = GetPlatformId() != Win32s;
  return instanceDataOK;
}
#endif // #if defined(OWL5_COMPAT)


//
// Get the windows version number in a 16bit unsigned int, with the bytes in
// the correct order: major in high byte, minor in low byte. Uses older
// GetVersion() API call.
//
uint TSystem::GetVersion() {
  static uint version = uint( ((::GetVersion()&0xFF) << 8) |
                              ((::GetVersion()&0xFF00) >> 8) );
  return version;
}

//
// Get the windows version flags number in a 16bit unsigned int. This is the
// high 16bits of the older GetVersion() API call.
//
uint TSystem::GetVersionFlags() {
  static uint versionFlags = uint(::GetVersion() >> 16);
  return versionFlags;
}

//
// Retrieves the product type for the operating system on the local computer,
// and maps the type to the product types supported by the specified operating system.
// e.g. PRODUCT_HOME_BASIC, PRODUCT_HOME_PREMIUM, PRODUCT_PROFESSIONAL, PRODUCT_ENTERPRISE
//
uint32 TSystem::GetWinProductType() {
  struct TLocal
  {
    //
    // Windows XP does not have GetProductInfo function implemented and will trap.
	// _WIN32_WINNT < 0x0600
    //
    static uint32 GetProductInfo(uint32 major, uint minor)
    {
      DWORD productType = PRODUCT_UNDEFINED;

      //
      // Quick check.  Must be greater than 6 to retrieve correct product type.
      //
      if (major < 6)
		  return productType;

	  try
      {
        TModule m(_T("kernel32.dll"), true, true, false);
        TModuleProc5<BOOL, DWORD, DWORD, DWORD, DWORD, PDWORD> f(m, "GetProductInfo");
        f(major, minor, 0, 0, &productType);
      }
      catch (const TXOwl &) {}

      return productType;
    }
  };
  static uint32 productType = TLocal::GetProductInfo(GetMajorVersion(), GetMinorVersion());
  return productType;
}

//
// Check if the Win OS is 64bit.
//
bool TSystem::IsWin64Bit() {
#ifdef _WIN64
  static bool Is64Bit = true;
#else
  struct TLocal
  {
    //
    // Windows XP does not have IsWow64Process function implemented and will trap.
	// _WIN32_WINNT < 0x0600
    //
    static bool IsWow64Process()
    {
      BOOL Is64Bit = FALSE;

	  try
      {
        TModule m(_T("kernel32.dll"), true, true, false);
        TModuleProc2<BOOL, HANDLE, PBOOL> f(m, "IsWow64Process");
        f(::GetCurrentProcess(), &Is64Bit);
      }
      catch (const TXOwl &) {}

      return Is64Bit;
    }
  };
  static bool Is64Bit = TLocal::IsWow64Process();
#endif
  return Is64Bit;
}

} // OWL namespace
//===============================================================================
