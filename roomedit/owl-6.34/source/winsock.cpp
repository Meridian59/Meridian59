//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TWinSockDll, a WinSock dll loader & wrapper
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/winsock.h>
#include <owl/module.h>

namespace owl {

OWL_DIAGINFO;

  static tchar WinSockDllName[] = _T("WSock32.dll");

//
// Construct wrapper object and load DLL
//

TWinSockDll::TWinSockDll()
:
  TModule(WinSockDllName, true, true, false) // shouldLoad, mustLoad and !addToList
{}

//
//  Standard socket functions
//

SOCKET
TWinSockDll::accept(SOCKET s, struct sockaddr * addr, int * addrLen)
{
  static TModuleProc3<SOCKET,SOCKET,struct sockaddr *, int *>
    accept(WinSockModule(), "accept");
  return accept(s, addr, addrLen);
}

int
TWinSockDll::bind(SOCKET s, struct sockaddr * addr, int nameLen)
{
  static TModuleProc3<int,SOCKET,struct sockaddr *,int>
    bind(WinSockModule(), "bind");
  return bind(s, addr, nameLen);
}

int
TWinSockDll::closesocket(SOCKET s)
{
  static TModuleProc1<int,SOCKET> closesocket(WinSockModule(), "closesocket");
  return closesocket(s);
}

int
TWinSockDll::connect(SOCKET s, struct sockaddr * name, int nameLen)
{
  static TModuleProc3<int,SOCKET,struct sockaddr *,int>
    connect(WinSockModule(), "connect");
  return connect(s, name, nameLen);
}

int
TWinSockDll::getpeername(SOCKET s, struct sockaddr * name, int * nameLen)
{
  static TModuleProc3<int,SOCKET,struct sockaddr *,int *>
    getpeername(WinSockModule(), "getpeername");
  return getpeername(s, name, nameLen);
}

int
TWinSockDll::getsockname(SOCKET s, struct sockaddr * name, int * nameLen)
{
  static TModuleProc3<int,SOCKET,struct sockaddr *,int *>
    getsockname(WinSockModule(), "getsockname");
  return getsockname(s, name, nameLen);
}

int
TWinSockDll::getsockopt(SOCKET s, int level, int optName, char * optVal, int * optLen)
{
  static TModuleProc5<int,SOCKET,int,int,char *,int *>
    getsockopt(WinSockModule(), "getsockopt");
  return getsockopt(s, level, optName, optVal, optLen);
}

ulong
TWinSockDll::_htonl(ulong hostLong)
{
  static TModuleProc1<ulong,ulong> _htonl(WinSockModule(), "htonl");
  return _htonl(hostLong);
}

ushort
TWinSockDll::_htons(ushort hostShort)
{
  static TModuleProc1<ushort,ushort> _htons(WinSockModule(), "htons");
  return _htons(hostShort);
}

ulong
TWinSockDll::inet_addr(const char * cp)
{
  static TModuleProc1<ulong,LPCSTR> inet_addr(WinSockModule(), "inet_addr");
  return inet_addr(cp);
}

char *
TWinSockDll::inet_ntoa(struct in_addr in)
{
  static TModuleProc1<char *,struct in_addr> inet_ntoa(WinSockModule(), "inet_ntoa");
  return inet_ntoa(in);
}

int
TWinSockDll::ioctlsocket(SOCKET s, long cmd, ulong * argp)
{
  static TModuleProc3<int,SOCKET,long,ulong *>
    ioctlsocket(WinSockModule(), "ioctlsocket");
  return ioctlsocket(s, cmd, argp);
}

int
TWinSockDll::listen(SOCKET s, int backlog)
{
  static TModuleProc2<int,SOCKET,int> listen(WinSockModule(), "listen");
  return listen(s, backlog);
}

ulong
TWinSockDll::_ntohl(ulong netLong)
{
  static TModuleProc1<ulong,ulong> _ntohl(WinSockModule(), "ntohl");
  return _ntohl(netLong);
}

ushort
TWinSockDll::_ntohs(ushort netShort)
{
  static TModuleProc1<ushort,ushort> _ntohs(WinSockModule(), "ntohs");
  return _ntohs(netShort);
}

int
TWinSockDll::recv(SOCKET s, char * buf, int len, int flags)
{
  static TModuleProc4<int,SOCKET,char *,int,int> recv(WinSockModule(), "recv");
  return recv(s, buf, len, flags);
}

int
TWinSockDll::recvfrom(SOCKET s, char * buf, int len, int flags,
  struct sockaddr * from, int* fromLen)
{
  static TModuleProc6<int,SOCKET,char *,int,int,struct sockaddr *,int*>
    recvfrom(WinSockModule(), "recvfrom");
  return recvfrom(s, buf, len, flags, from, fromLen);
}

int
TWinSockDll::select(int nfds, struct fd_set * readfds,
  struct fd_set * writefds, struct fd_set * exceptfds,
  const struct timeval * timeout)
{
  static TModuleProc5<int,int,struct fd_set *,struct fd_set *,
    struct fd_set *,const struct timeval *> select(WinSockModule(), "select");
  return select(nfds, readfds, writefds, exceptfds, timeout);
}

int
TWinSockDll::send(SOCKET s, LPCSTR buf, int len, int flags)
{
  static TModuleProc4<int,SOCKET,LPCSTR,int,int> send(WinSockModule(), "send");
  return send(s, buf, len, flags);
}

int
TWinSockDll::sendto(SOCKET s, LPCSTR buf, int len, int flags,
  const struct sockaddr * to, int toLen)
{
  static TModuleProc6<int,SOCKET,LPCSTR,int,int,
    const struct sockaddr *,int> sendto(WinSockModule(), "sendto");
  return sendto(s, buf, len, flags, to, toLen);
}

int
TWinSockDll::setsockopt(SOCKET s, int level, int optName,
  LPCSTR optVal, int optLen)
{
  static TModuleProc5<int,SOCKET,int,int,LPCSTR,int>
    setsockopt(WinSockModule(), "setsockopt");
  return setsockopt(s, level, optName, optVal, optLen);
}

int
TWinSockDll::shutdown(SOCKET s, int how)
{
  static TModuleProc2<int,SOCKET,int> shutdown(WinSockModule(), "shutdown");
  return shutdown(s, how);
}

SOCKET
TWinSockDll::socket(int af, int type, int protocol)
{
  static TModuleProc3<SOCKET,int,int,int> socket(WinSockModule(), "socket");
  return socket(af, type, protocol);
}

//
// Database functions
//

struct hostent *
TWinSockDll::gethostbyaddr(LPCSTR addr, int len, int type)
{
  static TModuleProc3<struct hostent *,LPCSTR,int,int>
    gethostbyaddr(WinSockModule(), "gethostbyaddr");
  return gethostbyaddr(addr, len, type);
}

struct hostent *
TWinSockDll::gethostbyname(LPCSTR name)
{
  static TModuleProc1<struct hostent *,LPCSTR>
    gethostbyname(WinSockModule(), "gethostbyname");
  return gethostbyname(name);
}

int
TWinSockDll::gethostname(char * name, int nameLen)
{
  static TModuleProc2<int,char *,int> gethostname(WinSockModule(), "gethostname");
  return gethostname(name, nameLen);
}

struct servent *
TWinSockDll::getservbyname(LPCSTR name, LPCSTR proto)
{
  static TModuleProc2<struct servent *,LPCSTR, LPCSTR>
    getservbyname(WinSockModule(), "getservbyname");
  return getservbyname(name, proto);
}

struct servent *
TWinSockDll::getservbyport(int port, LPCSTR proto)
{
  static TModuleProc2<struct servent *,int,LPCSTR>
    getservbyport(WinSockModule(), "getservbyport");
  return getservbyport(port, proto);
}

struct protoent *
TWinSockDll::getprotobyname(LPCSTR name)
{
  static TModuleProc1<struct protoent *, LPCSTR>
    getprotobyname(WinSockModule(), "getprotobyname");
  return getprotobyname(name);
}

struct protoent *
TWinSockDll::getprotobynumber(int proto)
{
  static TModuleProc1<struct protoent *, int>
    getprotobynumber(WinSockModule(), "getprotobynumber");
  return getprotobynumber(proto);
}

//
// Microsoft Windows Extension functions
//

int
TWinSockDll::WSAStartup(uint16 versionRequested, LPWSADATA WSAData)
{
  static TModuleProc2<int,uint16,LPWSADATA> WSAStartup(WinSockModule(), "WSAStartup");
  return WSAStartup(versionRequested, WSAData);
}

int
TWinSockDll::WSACleanup(void)
{
  static TModuleProc0<int> WSACleanup(WinSockModule(), "WSACleanup");
  return WSACleanup();
}

void
TWinSockDll::WSASetLastError(int error)
{
  static TModuleProcV1<int> WSASetLastError(WinSockModule(), "WSASetLastError");
  WSASetLastError(error);
}

int
TWinSockDll::WSAGetLastError(void)
{
  static TModuleProc0<int> WSAGetLastError(WinSockModule(), "WSAGetLastError");
  return WSAGetLastError();
}

BOOL
TWinSockDll::WSAIsBlocking(void)
{
  static TModuleProc0<int> WSAIsBlocking(WinSockModule(), "WSAIsBlocking");
  return WSAIsBlocking();
}

int
TWinSockDll::WSAUnhookBlockingHook(void)
{
  static TModuleProc0<int> WSAUnhookBlockingHook(WinSockModule(), "WSAUnhookBlockingHook");
  return WSAUnhookBlockingHook();
}

FARPROC
TWinSockDll::WSASetBlockingHook(FARPROC blockFunc)
{
  static TModuleProc1<FARPROC,FARPROC>
    WSASetBlockingHook(WinSockModule(), "WSASetBlockingHook");
  return WSASetBlockingHook(blockFunc);
}

int
TWinSockDll::WSACancelBlockingCall(void)
{
  static TModuleProc0<int> WSACancelBlockingCall(WinSockModule(), "WSACancelBlockingCall");
  return WSACancelBlockingCall();
}

HANDLE
TWinSockDll::WSAAsyncGetServByName(HWND hWnd, uint msg, LPCSTR name,
  LPCSTR proto, char * buf, int bufLen)
{
  static TModuleProc6<HANDLE,HWND,uint,LPCSTR,LPCSTR,
    char *,int> WSAAsyncGetServByName(WinSockModule(), "WSAAsyncGetServByName");
  return WSAAsyncGetServByName(hWnd, msg, name, proto, buf, bufLen);
}

HANDLE
TWinSockDll::WSAAsyncGetServByPort(HWND hWnd, uint msg, int port,
  LPCSTR proto, char * buf, int bufLen)
{
  static TModuleProc6<HANDLE,HWND,uint,int,LPCSTR,char *,int>
    WSAAsyncGetServByPort(WinSockModule(), "WSAAsyncGetServByPort");
  return WSAAsyncGetServByPort(hWnd, msg, port, proto, buf, bufLen);
}

HANDLE
TWinSockDll::WSAAsyncGetProtoByName(HWND hWnd, uint msg, LPCSTR name,
  char * buf, int bufLen)
{
  static TModuleProc5<HANDLE,HWND,uint,LPCSTR,char *,int>
    WSAAsyncGetProtoByName(WinSockModule(), "WSAAsyncGetProtoByName");
  return WSAAsyncGetProtoByName(hWnd, msg, name, buf, bufLen);
}

HANDLE
TWinSockDll::WSAAsyncGetProtoByNumber(HWND hWnd, uint msg, int number,
  char * buf, int bufLen)
{
  static TModuleProc5<HANDLE,HWND,uint,int,char *,int>
    WSAAsyncGetProtoByNumber(WinSockModule(), "WSAAsyncGetProtoByNumber");
  return WSAAsyncGetProtoByNumber(hWnd, msg, number, buf, bufLen);
}

HANDLE
TWinSockDll::WSAAsyncGetHostByName(HWND hWnd, uint msg, LPCSTR name,
  char * buf, int bufLen)
{
  static TModuleProc5<HANDLE,HWND,uint,LPCSTR,char *,int>
    WSAAsyncGetHostByName(WinSockModule(), "WSAAsyncGetHostByName");
  return WSAAsyncGetHostByName(hWnd, msg, name, buf, bufLen);
}

HANDLE
TWinSockDll::WSAAsyncGetHostByAddr(HWND hWnd, uint msg, LPCSTR addr,
  int len, int type, char * buf, int bufLen)
{
  static TModuleProc7<HANDLE,HWND,uint,LPCSTR,int,int,char *,int>
    WSAAsyncGetHostByAddr(WinSockModule(), "WSAAsyncGetHostByAddr");
  return WSAAsyncGetHostByAddr(hWnd, msg, addr, len, type, buf, bufLen);
}

int
TWinSockDll::WSACancelAsyncRequest(HANDLE hTaskHandle)
{
  static TModuleProc1<int,HANDLE>
    WSACancelAsyncRequest(WinSockModule(), "WSACancelAsyncRequest");
  return WSACancelAsyncRequest(hTaskHandle);
}

int
TWinSockDll::WSAAsyncSelect(SOCKET s, HWND hWnd, uint msg, long event)
{
  static TModuleProc4<int,SOCKET,HWND,uint,long>
    WSAAsyncSelect(WinSockModule(), "WSAAsyncSelect");
  return WSAAsyncSelect(s, hWnd, msg, event);
}

int
TWinSockDll::WSARecvEx(SOCKET s, char * buf, int len, int * flags)
{
  TModuleProc4<int,SOCKET,char *,int,int *> WSARecvEx(WinSockModule(), "WSARecvEx");
  return WSARecvEx(s, buf, len, flags);
}

TModule& 
TWinSockDll::WinSockModule()
{
  static TModule winSockModule(WinSockDllName, true, true);
  return winSockModule;
}

bool 
TWinSockDll::IsAvailable()
{
  return WinSockModule().IsLoaded();
}

/// Returns true if the DLL implementing WinSock is available and loaded, or false
/// otherwise.
bool       
TWinSock::IsAvailable()
{
  return TWinSockDll::IsAvailable();
}

TWinSockDll* 
TWinSock::Dll()
{
  static TWinSockDll winSockDll;
  return &winSockDll;
}


} // OWL namespace
/* ========================================================================== */

