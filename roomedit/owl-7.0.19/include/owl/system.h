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

#include <owl/wsysinc.h>
#include <owl/defs.h>

namespace owl {

class TModuleVersionInfo;

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

  static uint GetProductInfo();
  static auto GetVersionInfo() -> TModuleVersionInfo&;
  static uint GetMajorVersion();
  static uint GetMinorVersion();
  static uint GetBuildNumber();
  static auto GetVersionStr() -> tstring;
  static bool IsWin64Bit();

  /// @}

  //
  /// Encapsulates information about a processor core.
  /// See TSystem::GetProcessorInfo.
  //
  class _OWLCLASS TProcessorInfo // final
  {
  public:

    TProcessorInfo();
    ~TProcessorInfo();

    /// \name CPU attributes
    /// @{

    tstring GetName() const;
    tstring GetVendorId() const;
    static tstring GetVendorName(const tstring& vendorId);
    auto GetIdentifier() const -> tstring;
    int GetNominalFrequency() const;

    /// @}
    /// \name x86 CPU feature predicates
    /// @{

    static bool HasMmx();
    static bool Has3dNow();
    static bool HasSse();
    static bool HasSse2();
    static bool HasSse3();

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
