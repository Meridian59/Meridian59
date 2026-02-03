//
/// \file system.cpp
/// Implementation of TSystem class
//
// Part of OWLNext - the next generation Object Windows Library
// Copyright (c) 1995, 1997 by Borland International, All Rights Reserved.
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//

#include <owl/pch.h>
#include <owl/system.h>
#include <owl/module.h>
#include <owl/registry.h>
#include <map>

#if defined(__BORLANDC__)
# pragma warn -rvl // Turn off W8070 "Function should return a value" due to our assembler code.
#endif

using namespace owl;

//
// Local helpers
//
namespace
{

  //
  // Returns an initialized system info object.
  //
  static const SYSTEM_INFO& GetSystemInfo_()
  {
    struct TLocal
    {
      static SYSTEM_INFO Init()
      {
        SYSTEM_INFO s;
        ::GetNativeSystemInfo(&s);
        return s;
      }
    };
    static SYSTEM_INFO systemInfo = TLocal::Init();
    return systemInfo;
  }

} // namespace

namespace owl
{

//
/// Retrieves the product edition for the operating system on the local computer,
/// and maps the type to the product types supported by the specified operating system, e.g.
/// PRODUCT_HOME_BASIC, PRODUCT_HOME_PREMIUM, PRODUCT_PROFESSIONAL, PRODUCT_ENTERPRISE.
/// This function is only supported on Vista and later, and returns 0 (PRODUCT_UNDEFINED) on XP.
/// For more information see GetProductInfo in the Windows API documentation.
//
uint TSystem::GetProductInfo()
{
  static auto productType = []
  {
    auto p = DWORD{PRODUCT_UNDEFINED};
    const auto r = ::GetProductInfo(GetMajorVersion(), GetMinorVersion(), 0, 0, &p);
    CHECK(r); InUse(r);
    return p;
  }();
  return productType;
}

//
/// Returns the product version for "kernel32.dll".
//
auto TSystem::GetVersionInfo() -> TModuleVersionInfo&
{
  static TModuleVersionInfo systemVersion{_T("kernel32.dll")};
  return systemVersion;
}

uint TSystem::GetMajorVersion()
{
  return GetVersionInfo().GetProductVersionMS() >> 16;
}

uint TSystem::GetMinorVersion()
{
  return GetVersionInfo().GetProductVersionMS() & 0x0000FFFFu;
}

uint TSystem::GetBuildNumber()
{
  return GetVersionInfo().GetProductVersionLS() >> 16;
}

auto TSystem::GetVersionStr() -> tstring
{
  return GetVersionInfo().GetProductVersion();
}

//
/// Returns true if the OS is 64-bit.
//
bool TSystem::IsWin64Bit()
{

#ifdef _WIN64

  return true;

#else

  static const auto is64Bit = []
  {
    auto is64Bit = BOOL{FALSE};
    const auto r = IsWow64Process(::GetCurrentProcess(), &is64Bit);
    WARN(!r, _T("IsWow64Process failed. LastError: ") << GetLastError()); InUse(r);
    return is64Bit;
  }();
  return is64Bit != FALSE;

#endif

}

//
/// Returns the value of the SYSTEM_INFO::dwNumberOfProcessors field returned by ::GetSystemInfo.
/// Note that the value may specify the number of CPU cores, not necessarily the number of physical
/// processor packages/chips. See the Windows API for details.
//
int TSystem::GetNumberOfProcessors()
{
  return static_cast<int>(GetSystemInfo_().dwNumberOfProcessors);
}

//
/// Returns the value of the SYSTEM_INFO::wProcessorArchitecture field returned by ::GetSystemInfo.
/// See the Windows API for details of possible return values.
/// Use GetProcessorArchitectureName to get a description string.
//
uint TSystem::GetProcessorArchitecture()
{
  return GetSystemInfo_().wProcessorArchitecture;
}

//
/// Returns the description string for the given processor architecture.
/// If the given architecture ID is unknown, then an empty string is returned.
/// See GetProcessorArchitecture.
//
tstring TSystem::GetProcessorArchitectureName(uint architecture)
{
  typedef std::map<uint, tstring> TMap;
  typedef TMap::value_type T;
  const T i[] =
  {
    T(PROCESSOR_ARCHITECTURE_INTEL, _T("IA-32 (x86)")),
    T(PROCESSOR_ARCHITECTURE_MIPS, _T("MIPS")),
    T(PROCESSOR_ARCHITECTURE_ALPHA, _T("Alpha")),
    T(PROCESSOR_ARCHITECTURE_PPC, _T("PPC")),
    T(PROCESSOR_ARCHITECTURE_SHX, _T("SHX")),
    T(PROCESSOR_ARCHITECTURE_ARM, _T("ARM")),
    T(PROCESSOR_ARCHITECTURE_IA64, _T("IA-64")),
    T(PROCESSOR_ARCHITECTURE_ALPHA64, _T("Alpha64")),
    T(PROCESSOR_ARCHITECTURE_MSIL, _T("MSIL")),
    T(PROCESSOR_ARCHITECTURE_AMD64, _T("AMD64 (x86-64)")),
    T(PROCESSOR_ARCHITECTURE_IA32_ON_WIN64, _T("IA-32 on IA-64"))
  };
  static const TMap m(i, i + COUNTOF(i));
  TMap::const_iterator r = m.find(architecture);
  return r == m.end() ? tstring() : r->second;
}

//
/// Returns info for the current processor.
//
const TSystem::TProcessorInfo& TSystem::GetProcessorInfo()
{
  static TProcessorInfo processorInfo;
  return processorInfo;
}

} // namespace

//-------------------------------------------------------------------------------------------------

//
// Local helpers for looking up processor information in the Windows Registry.
//
namespace
{

  template <class TReturn>
  auto GetCpuInfo_(int cpuIndex, LPCTSTR value) -> TReturn
  {
    const auto keyName = _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\") + to_tstring(cpuIndex);
    const auto k = TRegKey::GetLocalMachine().GetSubkey(keyName);
    return k ? k->GetValueOrDefault(value, TReturn{}) : TReturn{};
  }

  int GetCpuFrequency_(int cpuIndex)
  {
    return static_cast<int>(GetCpuInfo_<uint32>(cpuIndex, _T("~MHz")));
  }

  auto GetCpuNameString_(int cpuIndex) -> tstring
  {
    return GetCpuInfo_<tstring>(cpuIndex, _T("ProcessorNameString"));
  }

  auto GetCpuVendorIdentifier_(int cpuIndex) -> tstring
  {
    return GetCpuInfo_<tstring>(cpuIndex, _T("VendorIdentifier"));
  }

  auto GetCpuIdentifier_(int cpuIndex) -> tstring
  {
    return GetCpuInfo_<tstring>(cpuIndex, _T("Identifier"));
  }

} // namespace

namespace owl
{

struct TSystem::TProcessorInfo::TImpl
{
  tstring Name;
  tstring VendorId;
  tstring Identifier;
  int Frequency;

  TImpl(int cpuIndex = 0)
    : Name(GetCpuNameString_(cpuIndex)),
    VendorId(GetCpuVendorIdentifier_(cpuIndex)),
    Identifier(GetCpuIdentifier_(cpuIndex)),
    Frequency(GetCpuFrequency_(cpuIndex))
  {}
};

//
/// Queries CPU info and initializes members.
//
TSystem::TProcessorInfo::TProcessorInfo()
  : Pimpl(new TImpl)
{}

TSystem::TProcessorInfo::~TProcessorInfo()
{
  delete Pimpl;
}

//
tstring TSystem::TProcessorInfo::GetName() const
{
  return Pimpl->Name;
}

tstring TSystem::TProcessorInfo::GetVendorId() const
{
  return Pimpl->VendorId;
}

auto TSystem::TProcessorInfo::GetIdentifier() const -> tstring
{
  return Pimpl->Identifier;
}

//
/// Returns the name of the CPU vendor.
/// If the vendor is unknown to this function, an empty string is returned.
//
tstring TSystem::TProcessorInfo::GetVendorName(const tstring& vendorId)
{
  typedef std::map<tstring, tstring> TMap;
  typedef TMap::value_type T;
  const T i[] =
  {
    T(_T("AuthenticAMD"), _T("AMD")),
    T(_T("GenuineIntel"), _T("Intel")),
    T(_T("CyrixInstead"), _T("Cyrix")),
    T(_T("CentaurHauls"), _T("Centaur")),
    T(_T("RiseRiseRise"), _T("Rise")),
    T(_T("GenuineTMx86"), _T("Transmeta")),
    T(_T("UMC UMC UMC "), _T("UMC"))
  };
  static const TMap m(i, i + COUNTOF(i));
  TMap::const_iterator r = m.find(vendorId);
  return r == m.end() ? tstring() : r->second;
}

//
/// Returns the frequency of CPU 0 as reported by the Windows Registry ("CentralProcessor/0/~MHz").
/// The returned frequency is specified in MHz.
//
int TSystem::TProcessorInfo::GetNominalFrequency() const
{
  return Pimpl->Frequency;
}

bool TSystem::TProcessorInfo::HasMmx()
{
  return IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE) != FALSE;
}

bool TSystem::TProcessorInfo::Has3dNow()
{
  return IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE) != FALSE;
}

bool TSystem::TProcessorInfo::HasSse()
{
  return IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) != FALSE;
}

bool TSystem::TProcessorInfo::HasSse2()
{
  return IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE;
}

bool TSystem::TProcessorInfo::HasSse3()
{
  return IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE) != FALSE;
}

} // OWL namespace
