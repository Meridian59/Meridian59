//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TSystem, a system information provider class
//----------------------------------------------------------------------------

#if !defined(OWL_SYSTEM_H)
#define OWL_SYSTEM_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/wsysinc.h>
#include <owl/defs.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
// Define Win95 platform constant if not defined in system headers
//
#if !defined(VER_PLATFORM_WIN32_WINDOWS)
# define VER_PLATFORM_WIN32_WINDOWS 1
#endif

//
// The build number if the system is Windows Server 2003 R2;
// otherwise, 0.
//
#if !defined(SM_SERVERR2)
# define SM_SERVERR2 89
#endif

#if !defined(PROCESSOR_ARCHITECTURE_AMD64)
#define PROCESSOR_ARCHITECTURE_AMD64            9
#endif

#if !defined(VER_SUITE_WH_SERVER)
#define VER_SUITE_WH_SERVER                 0x00008000
#endif

//
/// \class TSystem
// ~~~~~ ~~~~~~~
class _OWLCLASS TSystem {
  public:

#if defined(OWL5_COMPAT)
    // High level capability support queries
    //
    static bool SupportsExceptions();
    static bool SupportsThreads();
    static bool SupportsInstanceData();

    static bool Has3dUI();
    static bool HasSmallCaption();
    static bool HasSmallIcon();

    static bool IsWin32s(); // Deprecated. Win32 is no longer supported.
    static bool IsWin95();  // Deprecated.

    // More specific platform type queries
    //
    static bool IsNT();     // Deprecated. Always true under supported systems
#endif

    enum TPlatformId {
      Win32s       = VER_PLATFORM_WIN32s,            // Not supported anymore
      Win32Windows = VER_PLATFORM_WIN32_WINDOWS,     // Win95/Win98/WinME?
      Win32NT      = VER_PLATFORM_WIN32_NT,
      Win16        = -1,                             // Not supported anymore
    };
    static uint GetPlatformId();

    // Version information
    //
    static uint GetVersion();
    static uint GetVersionFlags();

    static uint GetMajorVersion();
    static uint GetMinorVersion();
    static uint GetBuildNumber();

    static const tchar* GetVersionStr();
	static uint GetSuiteMask();
    static uint GetProductType();

    // Windows version
    //
    static bool IsXP();
    static bool IsXP64();
    static bool IsWin2003();
    static bool IsHomeSrv();
    static bool IsWin2003R2();
    static bool IsVista();
    static bool IsWin2008();
    static bool IsWin2008R2();
    static bool IsWin7();
    static bool IsWin2012();
    static bool IsWin8();
    // Windows product type
    // e.g. PRODUCT_HOME_BASIC, PRODUCT_HOME_PREMIUM, PRODUCT_PROFESSIONAL, PRODUCT_ENTERPRISE
    //
    static uint32 GetWinProductType();
    // Win OS 64bit
    //
    static bool IsWin64Bit();

    // Hardware information
    //
    enum TProcessor
    {
      Intel386     = PROCESSOR_INTEL_386,
      Intel486     = PROCESSOR_INTEL_486,
      IntelPentium = PROCESSOR_INTEL_PENTIUM,
      MipsR4000    = PROCESSOR_MIPS_R4000,
      Alpha21064   = PROCESSOR_ALPHA_21064,
      Ppc601       = -1,
      Ppc603       = -1,
      Ppc604       = -1,
      Ppc620       = -1,
    };
    static TProcessor GetProcessorType();
    static uint GetProcessorArchitecture();

  protected:

    // Class that encapsulates the OSVERSIONINFOEX struct and the GetVersionEx()
    // Win32 call.
    //
    class TVersionInfo : private OSVERSIONINFOEX
    {
      public:
        TVersionInfo();
        uint GetMajorVersion() const;
        uint GetMinorVersion() const;
        uint GetBuildNumber() const;
        uint GetPlatformId() const;
        const tchar* GetVersionStr() const;
        uint GetSuiteMask() const;
        uint GetProductType() const;
    };

    // Return an initialized version info object
    //
    static TVersionInfo& GetVersionInfo();

    // Class that encapsulates the _SYSTEM_INFO struct and the GetSystemInfo()
    // Win32 call.
    //
    class TSystemInfo : private _SYSTEM_INFO
    {
      public:
        TSystemInfo();
        TProcessor GetProcessorType() const;
		uint GetProcessorArchitecture() const;
    };

    // Return an initialized system info object
    //
    static TSystemInfo& GetSystemInfo();
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace

#endif  // OWL_SYSTEM_H
