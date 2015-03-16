//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Winsock for OWL subsystem.
/// Based on work by Paul Pedriana, 70541.3223@compuserve.com
//----------------------------------------------------------------------------

#if !defined(OWL_WSKSOCKM_H)
#define OWL_WSKSOCKM_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <winsock.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

//
/// \class TSocketInfo
// ~~~~~ ~~~~~~~~~~~
/// TSocketInfo encapsulates the structure that contains details of the
/// Windows Socket implementation. For example, it contains the version
/// of the Windows Socket specification implemented, the maximum number of
/// sockets that a single process can open etc. etc.
//
class _OWLCLASS TSocketInfo : public WSAData {
  public:
    TSocketInfo();
};

//
/// \class TSocketManager
// ~~~~~ ~~~~~~~~~~~~~~
/// A class that starts up WinSock and provides information about the system's WinSock.
//
class _OWLCLASS TSocketManager {
  public:
    TSocketManager(short versionMajor = 1, short versionMinor = 1,
                   bool autoStartup = true);
    virtual ~TSocketManager();

    void Init(short versionMajor = 1, short versionMinor = 1);

    int Startup();
    int ShutDown();

    int       IsAvailable();
    int       GetMajorVersion();
    int       GetMinorVersion();
    char*     GetDescription();
    char*     GetSystemStatus();
    ushort    GetMaxUdpDgAvailable();
    ushort    GetMaxSocketsAvailable();
    char * GetVendorInfo();
    int       GetLastError();
    void      Information(TSocketInfo& socketInfo);

  protected:
    int          LastError;      ///< Last error code
    short        Available;      ///< Flag for Winsock availability
    short        StartupCount;   ///< Make sure to not overflow number of connects
    TSocketInfo  SocketInfo;     ///< Information about this WinSocket implementation
    short        VersionMajor;   ///< Major verion number
    short        VersionMinor;   ///< Minor verion number
};

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns true if WinSock is available.
//
inline int
TSocketManager::IsAvailable()
{
  return Available;
}

//
/// Returns the major version of WinSock support.
//
inline int
TSocketManager::GetMajorVersion()
{
  return VersionMajor;
}

//
/// Returns the minor version of WinSock support.
//
inline int
TSocketManager::GetMinorVersion()
{
  return VersionMinor;
}

//
/// Returns the system's description of WinSock.
//
inline char*
TSocketManager::GetDescription()
{
  return SocketInfo.szDescription;
}

//
/// Returns the status of WinSock.
//
inline char*
TSocketManager::GetSystemStatus()
{
  return SocketInfo.szSystemStatus;
}

//
/// Returns maximum number of bytes each UDP packet can be.
//
inline ushort
TSocketManager::GetMaxUdpDgAvailable()
{
  return SocketInfo.iMaxUdpDg;
}

//
/// Returns maximum number of WinSock connections avaialble.
//
inline ushort
TSocketManager::GetMaxSocketsAvailable()
{
  return SocketInfo.iMaxSockets;
}

//
/// Returns this WinSocket's vendor's information.
//
inline char *
TSocketManager::GetVendorInfo()
{
  return SocketInfo.lpVendorInfo;
}

//
/// Returns the last error code.
//
inline int
TSocketManager::GetLastError()
{
  return LastError;
}

//
/// Sets the information about this WinSocket.
//
inline void
TSocketManager::Information(TSocketInfo& socketInfo)
{
  socketInfo = SocketInfo;
}

} // OWL namespace

#endif  // OWL_SOCKMGR_H
