//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Winsock for OWL subsystem.
/// Based on work by Paul Pedriana, 70541.3223@compuserve.com
//----------------------------------------------------------------------------

#if !defined(OWL_WSKHOSTM_H)
#define OWL_WSKHOSTM_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <owl/window.h>
#include <owl/winsock.h>


namespace owl {

//
// Forward ref.   
//
class _OWLCLASS TSocketAddress;

#define WINSOCK_NOERROR (int)0
#define WINSOCK_ERROR   (int)SOCKET_ERROR

//
// Supposedly in RFC 883.
//
#define N_MAX_HOST_NAME 128
#define MSG_HOST_INFO_NOTIFY ((UINT)(WM_USER+303))

class _OWLCLASS THostInfoManager;

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

//
/// \class THostInfoWindow
// ~~~~~ ~~~~~~~~~~~~~~~
/// A private class created by THostInfoManager to catch WinSock messages.
//
class _OWLCLASS THostInfoWindow : public TWindow {
  public:
    THostInfoWindow(THostInfoManager* hostInfoManagerParent);

    TResult DoNotification(TParam1, TParam2);

  public_data:
    /// Object to notify of Winsock events
    //
    THostInfoManager* HostInfoManagerParent;

  DECLARE_RESPONSE_TABLE(THostInfoWindow);
};

//
/// \class THostEntry
// ~~~~~ ~~~~~~~~~~
/// THostEntry encapsulates the attributes of a host (hostent).
//
class _OWLCLASS THostEntry : public hostent {
  public:
    THostEntry();

    int   GetAddressCount();
    ulong GetINetAddress();

    /// An internet addressing -specific function.
    //
    ulong GetNthINetAddress(int nIndex = 0);
};

//
/// \class THostInfoManager
// ~~~~~ ~~~~~~~~~~~~~~~~
/// The THostInfoManager class (and its friend class THostInfoWindow) encapsulate
/// the Winsock database functions gethostbyname(), gethostbyaddr(), and
/// gethostname(). These blocking (gethostby...) and non-blocking (gethostname)
/// functions return information about the host in a hostent structure.
//
class _OWLCLASS THostInfoManager {
  public:
    THostInfoManager();
    virtual ~THostInfoManager();

    int   GetLastError();
    int   GetHostName(char * name, int nameLength = N_MAX_HOST_NAME);
    int   GetHostAddress(char * szHostAddress, const char * szHostName);
    int   GetHostAddress(TSocketAddress& sAddress, const char * szHostName);
    int   GetHostInfo(THostEntry*& hEntry, const TSocketAddress& sAddress);
    int   GetHostInfo(THostEntry*& hEntry, const char * szName);
    int   GetHostInfoAsync(HANDLE& hTheHostRequest, TSocketAddress& sAddress);
    int   GetHostInfoAsync(HANDLE& hTheHostRequest, char * szName);
    int   GetHostInfoAsync(TWindow& wndNotify, HANDLE& hTheHostRequest,
                           TSocketAddress& sAddress, uint nMessage = MSG_HOST_INFO_NOTIFY,
                           char * chBuffer = 0);
    int   GetHostInfoAsync(TWindow& wndNotify, HANDLE& hTheHostRequest,
                           char * szName, uint nMessage=MSG_HOST_INFO_NOTIFY,
                           char * chBuffer = 0);
    int   CancelHostRequest(HANDLE hTheHostRequest = 0);
    short GetHostRequestCompleted();
    static int HostEntryToAddress(THostEntry* hEntry, char * szAddress);
    static int HostEntryToAddress(THostEntry* hEntry, TSocketAddress& sAddress);

// !CQ public?
    // Set to point to HostInfoBuffer.
    //
  public_data:
    THostEntry*     HostEntry;

  protected:
    bool            HostRequestCompleted;              ///< Flag if host completed last request
    HANDLE          HostRequest;                       ///< Handle of host to get info about
    int             LastError;                         ///< Last error code
    char            HostInfoBuffer[MAXGETHOSTSTRUCT];  ///< Used for calls to WSAAsync...()
    THostInfoWindow HostWindow;                        ///< Hidden window to catch notifications

    void  SetHostRequestCompleted(int error);

  friend class THostInfoWindow;
};

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns the Internet address of the host.
//
inline ulong
THostEntry::GetINetAddress()
{
  return GetNthINetAddress(0);
}

//
/// This function returns the last error code.
//
inline int
THostInfoManager::GetLastError()
{
  return LastError;
}

//
/// This function returns true if the host completed the last requested transaction.
/// \todo really return a short? HostRequestCompleted is bool
//
inline short
THostInfoManager::GetHostRequestCompleted()
{
  return HostRequestCompleted;
}

} // OWL namespace


#endif  // OWL_WSKHOSTM_H
