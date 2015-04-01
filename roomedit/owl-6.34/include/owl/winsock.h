//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Main header of the Winsock OWL subsystem.
//----------------------------------------------------------------------------

#if !defined(OWL_WINSOCK_H)
#define OWL_WINSOCK_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/module.h>
#include <winsock.h>
#include <owl/wskaddr.h>
#include <owl/wsksock.h>
#include <owl/wskerr.h>
#include <owl/wskhostm.h>
#include <owl/wsksockd.h>
#include <owl/wsksockm.h>
#include <owl/wskservm.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

//
/// \class TWinSockDll
// ~~~~~ ~~~~~~~~~~~
/// The TWinSockDll class encapsulates the WinSock DLL (WINSOCK.DLL). It provides an
/// easy method to dynamically test for the availability of the DLL and bind to its
/// exported functions at runtime. By using the TWinSockDll class instead of direct
/// calls to the WinSock DLL, ObjectWindows applications can provide the appropriate
/// behaviour when running in an environment where the DLL is not available.
/// 
/// Each data member of the TWinSockDll class corresponds to the API with a similar
/// name exposed by the WinSock DLL. For example, TWinSockDll::WSAStartup
/// corresponds to the WSAStartup API exported by the WinSock DLL.
///
/// For more information about the members, consult
/// the documentation about the corresponding API exposed by the WinSock DLL.
//
class _OWLCLASS TWinSockDll : public TModule {
  public:
    TWinSockDll();

    // Socket functions
    //
    static SOCKET      accept(SOCKET s, struct sockaddr * addr,
                        int * addrLen);
    static int         bind(SOCKET s, struct sockaddr * addr, int nameLen);
    static int         closesocket(SOCKET s);
    static int         connect(SOCKET s, struct sockaddr * name, int nameLen);
    static int         getpeername(SOCKET s, struct sockaddr * name,
                        int * nameLen);
    static int         getsockname(SOCKET s, struct sockaddr * name,
                        int * nameLen);
    static int         getsockopt(SOCKET s, int level, int optName, char * optVal,
                        int * optLen);
    static ulong       _htonl(ulong hostLong);
    static ushort      _htons(ushort hostShort);
    static ulong       inet_addr(const char * cp);
    static char *   inet_ntoa(struct in_addr in);
    static int         ioctlsocket(SOCKET s, long cmd, ulong * argp);
    static int         listen(SOCKET s, int backlog);
    static ulong       _ntohl(ulong netLong);
    static ushort      _ntohs(ushort netShort);
    static int         recv(SOCKET s, char * buf, int len, int flags);
    static int         recvfrom(SOCKET s, char * buf, int len, int flags,
                        struct sockaddr * from, int* fromLen);
    static int         select(int nfds, struct fd_set * readfds,
                        struct fd_set * writefds,
                        struct fd_set * exceptfds,
                        const struct timeval * timeout);
    static int         send(SOCKET s, LPCSTR buf, int len, int flags);
    static int         sendto(SOCKET s, LPCSTR buf, int len, int flags,
                        const struct sockaddr * to, int toLen);
    static int         setsockopt(SOCKET s, int level, int optName,
                        LPCSTR optVal, int optLen);
    static int         shutdown(SOCKET s, int how);
    static SOCKET      socket(int af, int type, int protocol);

    // Database functions
    //
    static struct hostent  *  gethostbyaddr(LPCSTR addr, int len, int type);
    static struct hostent  *  gethostbyname(LPCSTR name);
    static int                   gethostname(char * name, int nameLen);
    static struct servent  *  getservbyname(LPCSTR name, LPCSTR proto);
    static struct servent  *  getservbyport(int port, LPCSTR proto);
    static struct protoent *  getprotobyname(LPCSTR name);
    static struct protoent *  getprotobynumber(int proto);

    // Microsoft Windows Extension functions
    //
    static int     WSAStartup(uint16 versionRequested, LPWSADATA WSAData);
    static int     WSACleanup(void);
    static void    WSASetLastError(int error);
    static int     WSAGetLastError(void);
    static BOOL    WSAIsBlocking(void);
    static int     WSAUnhookBlockingHook(void);
    static FARPROC WSASetBlockingHook(FARPROC blockFunc);
    static int     WSACancelBlockingCall(void);
    static HANDLE  WSAAsyncGetServByName(HWND hWnd, uint msg, LPCSTR name,
                    LPCSTR proto, char * buf, int bufLen);
    static HANDLE  WSAAsyncGetServByPort(HWND hWnd, uint msg, int port,
                    LPCSTR proto, char * buf, int bufLen);
    static HANDLE  WSAAsyncGetProtoByName(HWND hWnd, uint msg, LPCSTR name,
                    char * buf, int bufLen);
    static HANDLE  WSAAsyncGetProtoByNumber(HWND hWnd, uint msg, int number,
                    char * buf, int bufLen);
    static HANDLE  WSAAsyncGetHostByName(HWND hWnd, uint msg, LPCSTR name,
                    char * buf, int bufLen);
    static HANDLE  WSAAsyncGetHostByAddr(HWND hWnd, uint msg, LPCSTR addr,
                    int len, int type, char * buf, int bufLen);
    static int     WSACancelAsyncRequest(HANDLE hTaskHandle);
    static int     WSAAsyncSelect(SOCKET s, HWND hWnd, uint msg, long event);
    static int     WSARecvEx(SOCKET s, char * buf, int len, int * flags);

    // Check for presence of Winsock DLL
    //
    static TModule& WinSockModule();
    static bool IsAvailable();
};

//
/// \class TWinSock
// ~~~~~ ~~~~~~~~
/// TWinSock is an alias for an instance of the TDllLoader template which ensures
/// the underlying DLL is loaded and available. The 'IsAvailable' method (defined by
/// the TDllLoader template) can be used to load the DLL. For example,
/// \code
///         if (TWinSock::IsAvailable()) { 
///             // DLL is loaded - Proceed with WinSock calls
///         } else { 
///            // Error - Underlying DLL is not available.
///         }
/// \endcode
//

class _OWLCLASS TWinSock
{
  public:
    static bool         IsAvailable(void);
    static TWinSockDll* Dll(void);
};

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_WSKDLL_H
