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

#ifdef _WIN64
#ifdef BI_COMP_MSC
# include <intrin.h>
#endif
#endif

#if !defined(VER_SUITE_WH_SERVER)
#define VER_SUITE_WH_SERVER 0x00008000
#endif

#if defined(_MSC_VER)
# pragma warning(disable: 4996) // Turn off deprecation warnings (triggered by GetVersion/GetVersionEx).
#endif

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
  // Return an initialized version info object.
  //
  static const OSVERSIONINFOEX& GetVersionInfo_()
  {
    struct TLocal
    {
      static OSVERSIONINFOEX Init() 
      {
        OSVERSIONINFOEX v;
        v.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        ::GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(&v));
        return v;
      }
    };
    static OSVERSIONINFOEX versionInfo = TLocal::Init();
    return versionInfo;
  }

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
/// Returns the system platform as given by OSVERSIONINFOEX::dwPlatformId.
/// See the Windows API for details of possible return values.
//
uint TSystem::GetPlatformId()
{
  return GetVersionInfo_().dwPlatformId;
}

//
/// Retrieves the product edition for the operating system on the local computer,
/// and maps the type to the product types supported by the specified operating system, e.g.
/// PRODUCT_HOME_BASIC, PRODUCT_HOME_PREMIUM, PRODUCT_PROFESSIONAL, PRODUCT_ENTERPRISE.
/// This function is only supported on Vista and later, and returns 0 (PRODUCT_UNDEFINED) on XP.
/// For more information see GetProductInfo in the Windows API documentation.
//
uint TSystem::GetProductInfo()
{
  struct TLocal
  {
    //
    // Dynamically links and calls GetProductInfo.
    // Windows XP does not have GetProductInfo function implemented and returns PRODUCT_UNDEFINED.
    // _WIN32_WINNT < 0x0600
    //
    static DWORD GetProductInfo(DWORD major, DWORD minor) 
    {
      DWORD productType = PRODUCT_UNDEFINED;
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
  static uint productType = TLocal::GetProductInfo(GetMajorVersion(), GetMinorVersion());
  return productType;
}

//
/// Gets the windows version number in a 16-bit unsigned int, with the bytes in
/// the correct order: major in high byte, minor in low byte. Uses older
/// GetVersion() API call.
//
uint TSystem::GetVersion() 
{
  DWORD v = ::GetVersion();
  static uint version = ((v & 0x00FF) << 8) | ((v & 0xFF00) >> 8);
  return version;
}

//
/// Gets the windows version flags number in a 16-bit unsigned int. This is the
/// high 16 bits of the older GetVersion() API call.
//
uint TSystem::GetVersionFlags()
{
  static uint versionFlags = ::GetVersion() >> 16;
  return versionFlags;
}

uint TSystem::GetMajorVersion()
{
  return GetVersionInfo_().dwMajorVersion;
}

uint TSystem::GetMinorVersion()
{
  return GetVersionInfo_().dwMinorVersion;
}

uint TSystem::GetBuildNumber()
{
  return GetVersionInfo_().dwBuildNumber;
}

LPCTSTR TSystem::GetVersionStr()
{
  return GetVersionInfo_().szCSDVersion;
}

uint TSystem::GetSuiteMask()
{
  return GetVersionInfo_().wSuiteMask;
}

uint TSystem::GetProductType()
{
  return GetVersionInfo_().wProductType;
}

uint TSystem::GetServicePackMajor()
{
  return GetVersionInfo_().wServicePackMajor;
}

uint TSystem::GetServicePackMinor()
{
  return GetVersionInfo_().wServicePackMinor;
}

bool TSystem::IsXP()
{
  return (GetMajorVersion() == 5) && (GetMinorVersion() == 1) ? true : false;
}

bool TSystem::IsXP64()
{
  return (GetMajorVersion() == 5) && (GetMinorVersion() == 2) &&
    (GetProductType() == VER_NT_WORKSTATION) && (GetProcessorArchitecture() == PROCESSOR_ARCHITECTURE_AMD64) ?
    true : false;
}

bool TSystem::IsWin2003()
{
  return (GetMajorVersion() == 5) && (GetMinorVersion() == 2) &&
    (::GetSystemMetrics(SM_SERVERR2) == 0) ? true : false;
}

bool TSystem::IsHomeSrv()
{
  return (GetMajorVersion() == 5) && (GetMinorVersion() == 2) &&
    (GetSuiteMask() == VER_SUITE_WH_SERVER) ? true : false;
}

bool TSystem::IsWin2003R2()
{
  return (GetMajorVersion() == 5) && (GetMinorVersion() == 2) &&
    (::GetSystemMetrics(SM_SERVERR2) != 0) ? true : false;
}

bool TSystem::IsVista()
{
  return (GetMajorVersion() == 6) && (GetMinorVersion() == 0) &&
    (GetProductType() == VER_NT_WORKSTATION) ? true : false;
}

bool TSystem::IsWin2008()
{
  return (GetMajorVersion() == 6) && (GetMinorVersion() == 0) &&
    (GetProductType() != VER_NT_WORKSTATION) ? true : false;
}

bool TSystem::IsWin2008R2()
{
  return (GetMajorVersion() == 6) && (GetMinorVersion() == 1) &&
    (GetProductType() != VER_NT_WORKSTATION) ? true : false;
}

bool TSystem::IsWin7()
{
  return (GetMajorVersion() == 6) && (GetMinorVersion() == 1) &&
    (GetProductType() == VER_NT_WORKSTATION) ? true : false;
}

bool TSystem::IsWin2012()
{
  return (GetMajorVersion() == 6) && (GetMinorVersion() == 2) &&
    (GetProductType() != VER_NT_WORKSTATION) ? true : false;
}

bool TSystem::IsWin8()
{
  return (GetMajorVersion() == 6) && (GetMinorVersion() == 2) &&
    (GetProductType() == VER_NT_WORKSTATION) ? true : false;
}

//
/// Returns true if the OS is 64bit.
//
bool TSystem::IsWin64Bit()
{

#ifdef _WIN64

  return true;

#else

  struct TLocal
  {
    //
    // Returns TRUE if the current process is running on WOW64.
    // Dynamically links and calls IsWow64Process.
    // Windows XP does not have IsWow64Process function implemented and will return FALSE.
    // _WIN32_WINNT < 0x0600
    //
    static BOOL IsWow64Process() 
    {
      BOOL is64Bit = FALSE;
      try
      {
        TModule m(_T("kernel32.dll"), true, true, false);
        TModuleProc2<BOOL, HANDLE, PBOOL> f(m, "IsWow64Process");
        f(::GetCurrentProcess(), &is64Bit);
      }
      catch (const TXOwl &) {}
      return is64Bit;
    }
  };
  static bool is64Bit = ToBool(TLocal::IsWow64Process());
  return is64Bit;

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
// Local helpers
//
namespace 
{

  //
  // Returns 1 if CPUID instruction supported on this processor, zero otherwise.
  // This isn't necessary on 64-bit processors because all x86-64 processors support CPUID.
  //
  bool IsCpuIdSupported_() 
  {

#ifdef _WIN64

    return true;

#elif defined(BI_COMP_MSC) || (defined(BI_COMP_BORLANDC) && !defined(BI_COMP_CLANG)) // MSC or BCC32

    __asm
    {
      push ecx            // save ecx
      pushfd              // push original EFLAGS
      pop eax             // get original EFLAGS
      mov ecx, eax        // save original EFLAGS
      xor eax, 200000h    // flip bit 21 in EFLAGS
      push eax            // save new EFLAGS value on stack
      popfd               // replace current EFLAGS value
      pushfd              // get new EFLAGS
      pop eax             // store new EFLAGS in EAX
      xor eax, ecx        // Bit 21 of flags at 200000h will be 1 if CPUID exists
      shr eax, 21         // Shift bit 21 to bit 0 and return it
      push ecx
      popfd               // restore bit 21 in EFLAGS first
      pop ecx             // restore ecx
    }

#else

#pragma message("IsCpuIdSupported_ is not implemented for this compiler.")

    return false;

#endif

  }

  //
  // Class that encapsulates the CPUID result
  //
  struct TCpuIdResult_
  {
    int Reg[4];

    int& operator [](size_t i)
    {return Reg[i];}

    const int& operator [](size_t i) const
    {return Reg[i];}
  };

  //
  // Returns CPU information using the CPUID instruction.
  //
  TCpuIdResult_ CpuId_(int func)
  {
    if (!IsCpuIdSupported_()) return TCpuIdResult_();
    TCpuIdResult_ r;

#if defined(BI_COMP_MSC)

    __cpuid(r.Reg, func);

#elif defined(BI_COMP_BORLANDC) && !defined(BI_COMP_CLANG) // BCC32

    __asm
    {
      mov eax, func
      cpuid
      mov dword ptr [r] + 0, eax
      mov dword ptr [r] + 4, ebx
      mov dword ptr [r] + 8, ecx
      mov dword ptr [r] + 12, edx
    }

#else

#pragma message("CpuId_ is not implemented for this compiler.")

#endif

    return r;
  }

  //
  // Returns the current value of the CPU's high precision counter using the RDTSC instruction.
  //
  uint64 Rdtsc_() 
  {

#if defined(BI_COMP_MSC)

    return __rdtsc();

#elif defined(BI_COMP_BORLANDC) && !defined(BI_COMP_CLANG) // BCC32

    uint64 r;
    __asm
    {
      rdtsc
      mov dword ptr [r], eax
      mov dword ptr [r] + 4, edx
    }
    return r;

#else 

#pragma message("Rdtsc_ is not implemented for this compiler.")

    return 0;

#endif

  }

  tstring MakeString_(int packedString)
  {
    union TPackedString
    {
      int data;
      char str[5];
    } p = {packedString};
    p.str[4] = '\0';
    _USES_CONVERSION;
    return _A2W(&p.str[0]);
  }

  tstring MakeString_(const TCpuIdResult_& packedString)
  {
    tostringstream r;
    r << MakeString_(packedString[0]) <<
      MakeString_(packedString[1]) <<
      MakeString_(packedString[2]) <<
      MakeString_(packedString[3]);
    return r.str();
  }

  tstring GetCpuVendorId_()
  {
    TCpuIdResult_ id = CpuId_(0x00000000);
    tostringstream r;
    r << MakeString_(id[1]) << MakeString_(id[3]) << MakeString_(id[2]);
    return r.str();
  }

  //
  // Returns the Highest Extended Function Supported for the CPUID instruction.
  // Note that the result is unsigned, i.e. extended function argument values are considered larger 
  // than standard function argument values (e.g. 0x80000000u > 0x00000000u).
  //
  uint GetCpuIdMaxArg_()
  {
    return static_cast<uint>(CpuId_(0x80000000)[0]);
  }

  tstring GetCpuBrandString_()
  {
    if (GetCpuIdMaxArg_() < 0x80000004u) return tstring();
    TCpuIdResult_ part1 = CpuId_(0x80000002);
    TCpuIdResult_ part2 = CpuId_(0x80000003);
    TCpuIdResult_ part3 = CpuId_(0x80000004);
    tostringstream r;
    r << MakeString_(part1) << MakeString_(part2) << MakeString_(part3);
    return r.str();
  }

  //
  // Returns the approximate frequency of the given CPU/core.
  //
  int GetCpuFrequency_(int cpuIndex = 0)
  {
    try
    {
      tostringstream keyName;
      keyName << _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\") << cpuIndex;
      TRegKey k(HKEY_LOCAL_MACHINE, keyName.str(), KEY_READ, TRegKey::NoCreate);
      TRegValue v(k, _T("~MHz"));
      return static_cast<int>(static_cast<uint32>(v)); // Hz
    }
    catch (const TXRegistry&)
    {
      return 0;
    }
  }

} // namespace

namespace owl 
{

struct TSystem::TProcessorInfo::TImpl
{
  tstring Name;
  tstring VendorId;
  TCpuIdResult_ Info;
  TCpuIdResult_ ExtInfo;
  int Frequency;

  TImpl()
    : Name(GetCpuBrandString_()),
    VendorId(GetCpuVendorId_()),
    Info(CpuId_(0x00000001)),
    ExtInfo((GetCpuIdMaxArg_() >= 0x80000001u) ? CpuId_(0x80000001) : TCpuIdResult_()),
    Frequency(GetCpuFrequency_())
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
/// Returns the Processor Brand String, if supported by the CPU.
/// Otherwise an empty string is returned.
//
tstring TSystem::TProcessorInfo::GetName() const
{
  return Pimpl->Name;
}

tstring TSystem::TProcessorInfo::GetVendorId() const
{
  return Pimpl->VendorId;
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

uint TSystem::TProcessorInfo::GetModel() const
{
  return (Pimpl->Info[0] >> 4) & 0xF;
}

uint TSystem::TProcessorInfo::GetExtModel() const
{
  return (Pimpl->Info[0] >> 16) & 0xF;
}

uint TSystem::TProcessorInfo::GetFamily() const
{
  return (Pimpl->Info[0] >> 8) & 0xF;
}

uint TSystem::TProcessorInfo::GetExtFamily() const
{
  return (Pimpl->Info[0] >> 20) & 0xFF;
}

uint TSystem::TProcessorInfo::GetType() const
{
  return (Pimpl->Info[0] >> 12) & 0x3;
}

uint TSystem::TProcessorInfo::GetStepping() const
{
  return Pimpl->Info[0] & 0xF;
}

//
/// Returns the frequency of CPU 0 as reported by the Windows Registry ("CentralProcessor/0/~MHz").
/// The returned frequency is specified in MHz.
//
int TSystem::TProcessorInfo::GetNominalFrequency() const
{
  return Pimpl->Frequency;
}

//
/// Measures and returns the current frequency of the current CPU core.
/// The given measurement period should be specified in milliseconds. A longer period will increase 
/// precision. It should not be shorter than the resolution of the GetTickCount (5ms on WinXP).
/// Note that modern CPUs run at various frequencies, depending on the power and turbo mode state.
/// The returned frequency is specified in MHz.
//
int TSystem::TProcessorInfo::GetCurrentFrequency(int measurementPeriod) const
{
  LARGE_INTEGER counterBegin;
  ::QueryPerformanceCounter(&counterBegin);
  uint64 cyclesStart = Rdtsc_();

  ::Sleep(measurementPeriod);

  uint64 cyclesEnd = Rdtsc_();
  LARGE_INTEGER counterEnd;
  ::QueryPerformanceCounter(&counterEnd);

  LARGE_INTEGER f;
  ::QueryPerformanceFrequency(&f);
  return static_cast<int>(
    static_cast<uint64>(cyclesEnd - cyclesStart) * f.QuadPart / 
    (counterEnd.QuadPart - counterBegin.QuadPart) /
    1000000);
}

bool TSystem::TProcessorInfo::HasMmx() const
{
  return ToBool((Pimpl->Info[3] >> 23) & 0x1);
}

bool TSystem::TProcessorInfo::HasMmxExt() const
{
  return ToBool((Pimpl->ExtInfo[3] >> 22) & 0x1);
}

bool TSystem::TProcessorInfo::Has3dNow() const
{
  return ToBool((Pimpl->ExtInfo[3] >> 31) & 0x1);
}

bool TSystem::TProcessorInfo::Has3dNowExt() const
{
  return ToBool((Pimpl->ExtInfo[3] >> 30) & 0x1);
}

bool TSystem::TProcessorInfo::HasSse() const
{
  return ToBool((Pimpl->Info[3] >> 25) & 0x1);
}

bool TSystem::TProcessorInfo::HasSse2() const
{
  return ToBool((Pimpl->Info[3] >> 26) & 0x1);
}

bool TSystem::TProcessorInfo::HasSse3() const
{
  return ToBool(Pimpl->Info[2] & 0x1);
}

bool TSystem::TProcessorInfo::HasHtt() const
{
  return ToBool((Pimpl->Info[3] >> 28) & 0x1);
}

} // OWL namespace
