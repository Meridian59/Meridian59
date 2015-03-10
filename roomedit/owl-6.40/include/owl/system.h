//
/// \file system.h
/// Definition of TSystem, a system information provider class
//
// Part of OWLNext - the next generation Object Windows Library 
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved.
//
// For more information, including license details, see 
// http://owlnext.sourceforge.net
//

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

#include <owl/preclass.h>

//
/// \class TSystem
/// Provides information on OS relevant information.
/// Note that this class is only a namespace for the contained static member functions,
/// and hence it should not (and can not) be instantiated or inherited.
//
class _OWLCLASS TSystem // final
{
public:

  /// \name Accessors for OS info
  /// @{

  static uint GetPlatformId();
  static uint GetProductInfo();
  static uint GetVersion();
  static uint GetVersionFlags();
  static uint GetMajorVersion();
  static uint GetMinorVersion();
  static uint GetBuildNumber();
  static LPCTSTR GetVersionStr();
  static uint GetSuiteMask();
  static uint GetProductType();
  static uint GetServicePackMajor();
  static uint GetServicePackMinor();

  /// @}
  /// \name Member functions for checking the specific OS edition
  /// @{

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
  static bool IsWin64Bit();

  /// @}

  //
  /// Encapsulates information about a processor core.
  /// See TSystem::GetProcessorInfo.
  //
  class TProcessorInfo // final
  {
  public:

    TProcessorInfo();
    ~TProcessorInfo();

    /// \name CPU attributes
    /// @{

    tstring GetName() const;
    tstring GetVendorId() const;
    static tstring GetVendorName(const tstring& vendorId);
    uint GetModel() const;
    uint GetExtModel() const;
    uint GetFamily() const;
    uint GetExtFamily() const;
    uint GetType() const;
    uint GetStepping() const;
    int GetNominalFrequency() const;
    int GetCurrentFrequency(int measurementPeriod = 200) const;

    /// @}
    /// \name x86 CPU feature predicates
    /// @{

    bool HasMmx() const;
    bool HasMmxExt() const;
    bool Has3dNow() const;
    bool Has3dNowExt() const;
    bool HasSse() const;
    bool HasSse2() const;
    bool HasSse3() const;
    bool HasHtt() const;

    /// @}

  private:

    struct TImpl;
    TImpl* Pimpl;

    // Copy prevention

    TProcessorInfo(const TProcessorInfo&); // = delete
    const TProcessorInfo& operator =(const TProcessorInfo&); // = delete
  };

  /// \name Accessors for processor info
  /// @{

  static int GetNumberOfProcessors();
  static uint GetProcessorArchitecture();
  static tstring GetProcessorArchitectureName(uint architecture);
  static const TProcessorInfo& GetProcessorInfo();

  /// @}

private:

  // Instantiation and inheritance prevention

  TSystem(); // = delete
  TSystem(const TSystem&); // = delete
  TSystem& operator =(const TSystem&); // = delete

};

#include <owl/posclass.h>

} // OWL namespace

#endif  // OWL_SYSTEM_H
