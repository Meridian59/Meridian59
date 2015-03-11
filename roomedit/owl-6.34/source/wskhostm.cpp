//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Winsock for OWL subsystem.
/// Based on work by Paul Pedriana, 70541.3223@compuserve.com
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/winsock.h>

namespace owl {


OWL_DIAGINFO;

//
//
//
DEFINE_RESPONSE_TABLE1(THostInfoWindow, TWindow)
   EV_MESSAGE(MSG_HOST_INFO_NOTIFY, DoNotification),
END_RESPONSE_TABLE;

//
/// The HostInfoWindow requires a HostInfoManager, so it can relay messages to it.
//
THostInfoWindow::THostInfoWindow(THostInfoManager* hostInfoManagerParent)
:
  TWindow(0, _T("HostInfo Window")),
  HostInfoManagerParent(hostInfoManagerParent)
{
  Attr.Style = 0;  // Turn off WS_CHILD (the default) style).
}

//
/// This is a relay function.
//
TResult THostInfoWindow::DoNotification(TParam1, TParam2 param2)
{
  HostInfoManagerParent->SetHostRequestCompleted(WSAGETASYNCERROR(param2));
  return 1;
}

//----------------------------------------------------------------------------

//
/// Constructor of THostEntry object. Initializes members describing host to 0.
//
THostEntry::THostEntry()
{
  h_name      = 0;
  h_aliases   = 0;
  h_addrtype  = 0;
  h_length    = 0;
  h_addr_list = 0;
}

//
/// Returns the number of pointers to addresses in the hostent (parent class of
/// THostEntry).
//
int THostEntry::GetAddressCount()
{
  int i = 0;
  while (h_addr_list[i])
    i++;

  return i;
}

//
/// This function returns the indexed internet address in unsigned long form. The
/// index must be between 0 and the number of address counts (see
/// GetAddressCount()).
///
/// \note This function is FLAWED in that it is internet addressing-specific
///  (AF_INET/PF_INET).
/// The proper way to implement this function would be to look at h_addrtype
///  and h_length to determine the nature of the address type and return
///  something useful or make a derived class that knows about each address
///  family. (...)
//
ulong THostEntry::GetNthINetAddress(int index)
{
  int addressCount = GetAddressCount();

  if (index >= addressCount)
    return 0;  //The caller asked for an address index that is out of range.

  return *((ulong*)h_addr_list[index]);
}

//----------------------------------------------------------------------------
// THostInfoManager
//

//
/// This function initializes the hidden window.
//
THostInfoManager::THostInfoManager()
:
  HostWindow(this)
{
  HostEntry            = (THostEntry*)&HostInfoBuffer;
  HostRequest          = 0;
  LastError            = 0;
  HostRequestCompleted = false;
  try {
    HostWindow.Create();
  }
  catch (...) {
    //::MessageBeep(10);  // !CQ Do something real here?
 }
}

//
/// With this destructor, you need to clear any pending requests before the
/// deletion.
//
THostInfoManager::~THostInfoManager()
{
  if (HostRequest && !HostRequestCompleted) // If there is an outstanding request...
    CancelHostRequest();
}

//
/// This function returns the name of the computer on which this program is running.
/// The name parameter is set to the name. The return value is either WINSOCK_ERROR
/// or WINSOCK_NOERROR. You can call THostInfoManager::GetLastError() to get the
/// actual error value. name is a pointer to a preallocated buffer of minimum size
/// of nameLength. nameLength should be at least N_MAX_HOST_NAME.
//
int THostInfoManager::GetHostName(char * name, int nameLength)
{
  if (TWinSockDll::gethostname(name, nameLength) == SOCKET_ERROR) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
/// This function effectively converts szHostName to szHostAddress. If you have a
/// name such as "joe_schmoe@borland.com" and you want the dotted-decimal IP address
/// for it, you can call this function. This function assumes that there is enough
/// space in szHostAddress for the address. This function, like most of Windows
/// Sockets, currently works only with IP addresses. Thus, the szHostAddress is
/// always going to be dotted-decimal format in Windows Sockets.
/// Note that when using the inet_ntoa() function, the char* string returned resides
/// in Windows Sockets memory space, the szHostAddress returned is allocated and
/// owned by the caller of this function and can be manipulated any way the caller
/// wants.
/// This function returns an error value of WINSOCK_ERROR or WINSOCK_NOERROR.
//
int THostInfoManager::GetHostAddress(char * hostAddress, const char * hostName)
{
  THostEntry* tempHostEntry;

  int error = GetHostInfo(tempHostEntry, hostName);
  if (error == WINSOCK_ERROR)
     return error;
  return HostEntryToAddress(tempHostEntry, hostAddress);
}

//
/// This function effectively converts szHostName to a socket address. If you have a
/// name such as "joe_schmoe@borland.com," and you want the TSocketAddress for it,
/// you can call this function. This function, like most of Windows Sockets,
/// currently works only with IP addresses. Thus, the szAddress is always going to
/// be dotted-decimal format in Windows Sockets. The szHostName is a string
/// parameter that specifies the host of which to get the address.
/// This function returns an error value of WINSOCK_ERROR or WINSOCK_NOERROR.
//
int THostInfoManager::GetHostAddress(TSocketAddress& address, const char * hostName)
{
   THostEntry* tempHostEntry;

   int error = GetHostInfo(tempHostEntry, hostName);
   if (error == WINSOCK_ERROR)
      return error;
   return HostEntryToAddress(tempHostEntry, address);
}

//
/// The caller of this function supplies a pointer to be assigned by this function.
/// The caller need not allocate space for any THostEntry structure. Because of
/// this, the data needs to be read immediately or copied for later use. hEntry is a
/// pointer passed by reference. sAddress is a preallocated SocketAddress reference.
/// Due to the design of the socket API, the call to gethostbyaddr currently
/// requires a pointer to the Internet address, rather than a sockaddr or even a
/// sockaddr_in. Because passing a ulong pointer would most likely not work (for
/// example, if the socket API were to support something other than IP), this issue
/// is fixed by making a sockaddr interface to this API. The address is in network
/// byte ordering.
//
int THostInfoManager::GetHostInfo(THostEntry*& entry, const TSocketAddress& address)
{
  sockaddr_in* tempSockAddrIn = (sockaddr_in*)&address;
  tempSockAddrIn->sin_addr.s_addr = tempSockAddrIn->sin_addr.s_addr;

  entry = (THostEntry*)TWinSockDll::gethostbyaddr(
                                   (const char *)&tempSockAddrIn->sin_addr.s_addr,
                                   sizeof(ulong), PF_INET);
  if (!entry) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
/// Windows Sockets can block a call until the other end finishes the transaction.
/// hEntry is a pointer passed by reference. The system will change that pointer to
/// point to an internal Windows Sockets data structure. The contents must not be
/// modified. szName is a preallocated string that holds a string. The address of
/// the host can be in string format or in binary format.
///
/// The caller of this function passes a pointer to a THostEntry struct, for
/// example:
/// \code
/// THostEntry* tempTHostEntry;
/// GetHostInfo(tempTHostEntry, "JoeShmoe@anywhere.com");
/// printf("%s", tempTHostEntry->h_name); //h_name should be "joeSchmoe@anywhere.com"
/// \endcode
int THostInfoManager::GetHostInfo(THostEntry*& entry, const char * name)
{
  entry = (THostEntry*)TWinSockDll::gethostbyname(name);
  if (!entry) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
/// The caller can use this call to cancel the last pending request.
//
int THostInfoManager::CancelHostRequest(HANDLE hostRequest)
{
  if (!hostRequest)
    hostRequest = HostRequest;

  if (!hostRequest) {
    LastError = WSAEINVAL;  // There is no handle to use.
    return WINSOCK_ERROR;
  }

  if (TWinSockDll::WSACancelAsyncRequest(hostRequest) == SOCKET_ERROR) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }

  return WINSOCK_NOERROR;
}

//
/// The TheHostRequest parameter is returned to the caller with the asynchrous
/// request handle. sAddress needs to be in network byte ordering. Note that due to
/// the design of this class, you cannot have two outstanding service requests that
/// get notified directly to this class. You can use the hwnd-specific notification
/// version of this function to manage multiple requests. You can also create more
/// than one instance of this class. The service is complete when
/// HostRequestCompleted is true. Look at LastError in this case to see if there was
/// an error.
///
/// Do not issue any asynchronous calls that post to this class hwnd until the
/// previous request is completed. The alternative is to create multiple
/// THostInfoManagers or manage the call-backs yourself. See the comments about the
/// non-asynchronous version of this call (THostInfoManager::GetHostInfo) for more
/// information.
//
int THostInfoManager::GetHostInfoAsync(HANDLE& hostRequest, TSocketAddress& address)
{
  sockaddr_in* tempSockAddrIn = (sockaddr_in*)&address;
  tempSockAddrIn->sin_addr.s_addr = tempSockAddrIn->sin_addr.s_addr;

  HostRequestCompleted = false;
  HostRequest = hostRequest =
    TWinSockDll::WSAAsyncGetHostByAddr(HostWindow, MSG_HOST_INFO_NOTIFY,
                                    (char *)&tempSockAddrIn->sin_addr.s_addr,
                                    sizeof(ulong), PF_INET, HostInfoBuffer,
                                    MAXGETHOSTSTRUCT);
  if (!hostRequest) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
/// Returns the same information as the other versions of GetHostInfoAsync. The
/// difference is that the host name can be a string, rather than a TSocketAddress.
//
int THostInfoManager::GetHostInfoAsync(HANDLE& theHostRequest, char * name)
{
  HostRequestCompleted = false;
  HostRequest = theHostRequest =
      TWinSockDll::WSAAsyncGetHostByName(HostWindow, MSG_HOST_INFO_NOTIFY, name,
                                      HostInfoBuffer, MAXGETHOSTSTRUCT);

  if (!HostRequest) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
/// This function notifies the given window that a request has completed. nMessage
/// is the message that the wndNotify will receive. It defaults to
/// MSG_HOST_INFO_NOTIFY, which is defined in the THostInfoManager's header file.
/// The chBuffer is a pointer to the buffer that will be filled in with a hostent.
/// It needs to be at least MAXGETHOSTSTRUCT bytes. If chBuffer is 0 (or not
/// specified), the THostInfoManager's internal buffer is used. The hTheHostRequest
/// will hold a handle that the caller can use to reference the request on
/// call-back. wParam will be equal to the hService returned.
/// WSAGETSYNCERROR (lParam) holds an error, if any (0 is OK). WSAGETSYNCBUFLEN
/// (lParam) holds actual length of the buffer. When this function returns,
/// myTHostEntry holds the appropriate information. Since this information belongs
/// to this object, you can delay reading it as long as you want. Note that while
/// the sAddress should be passed in network byte ordering, the output on callback
/// is also in network ordering.
//
int THostInfoManager::GetHostInfoAsync(TWindow& wndNotify, HANDLE& hostRequest,
                                       TSocketAddress& address, uint message,
                                       char * buffer)
{
  sockaddr_in* tempSockAddrIn = (sockaddr_in*)&address;
  tempSockAddrIn->sin_addr.s_addr = tempSockAddrIn->sin_addr.s_addr;

  char * bufferToUse = buffer ? buffer : HostInfoBuffer;

  HostRequestCompleted = false;
  HostRequest = hostRequest =
    TWinSockDll::WSAAsyncGetHostByAddr(wndNotify, message,
                                    (char *)&tempSockAddrIn->sin_addr.s_addr,
                                    sizeof(ulong), PF_INET, bufferToUse,
                                    MAXGETHOSTSTRUCT);

  if (!hostRequest) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
/// This function notifies the given window that a request has completed, wndNotify
/// is the window that will get the message that the request has completed. nMessage
/// is the message that the wndNotify will receive. It defaults to
/// MSG_HOST_INFO_NOTIFY, which is defined in the THostInfoManager's header file.
/// hTheHostRequest is the asynchrous request handle that will be a reference to the
/// request. szName is the name of the host, as in "coyote@acme.com." The chBuffer
/// is a pointer to buffer that will be filled in with a hostent. It needs to be at
/// least MAXGETHOSTSTRUCT bytes. If chBuffer is 0 (or not specified), the
/// THostInfoManager's internal buffer will be used. The returned address is in
/// network byte ordering.
//
int THostInfoManager::GetHostInfoAsync(TWindow& wndNotify, HANDLE& hostRequest, char * name,
                                       uint message, char * buffer)
{
  char * bufferToUse = buffer ? buffer : HostInfoBuffer;

  HostRequestCompleted = false;
  HostRequest = hostRequest =
     TWinSockDll::WSAAsyncGetHostByName(wndNotify, message, name, bufferToUse,
                                     MAXGETHOSTSTRUCT);

  if (!hostRequest) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
/// Given a THostEntry*, this function converts it to a dotted-decimal szAddress.
/// Because Windows Sockets supports only IP addressing, this function uses IP
/// addressing and the address is always dotted-decimal. The return value is
/// WINSOCK_ERROR or WINSOCK_NOERROR.
//
int THostInfoManager::HostEntryToAddress(THostEntry* entry, char * address)
{
  in_addr    tempInAddr;
  tempInAddr.s_addr = *((ulong*)entry->h_addr);
            //else: = *((ulong*)hEntry->h_addr_list[0])

  char * tempAddress = TWinSockDll::inet_ntoa(tempInAddr);
  if (!tempAddress)
    return WINSOCK_ERROR;
  strcpy(address, tempAddress);
  return WINSOCK_NOERROR;
}

//
/// Given a THostEntry*, this function converts it to a socket address. Because
/// Windows Sockets supports only IP addressing, this function uses IP addressing
/// and the address is an INetSocketAddress. The return value is WINSOCK_ERROR or
/// WINSOCK_NOERROR.
//
int THostInfoManager::HostEntryToAddress(THostEntry* entry, TSocketAddress& address)
{
  TINetSocketAddress *addr = STATIC_CAST(TINetSocketAddress*, &address);
  addr->SetNetworkAddress(*((ulong*)entry->h_addr));
  return WINSOCK_NOERROR;
}

//
/// This function is called whenever an asynchronous request is completed. You may
/// want to override this function in your THostInfoManager-derived class. If you
/// do, you must call the base version.
//
void THostInfoManager::SetHostRequestCompleted(int result)
{
  int error = WSAGETASYNCERROR(result);
  if (error != 0)
    LastError = error;
  HostRequestCompleted = true;  // Of course, there may have been an error.
}

} // OWL namespace

/* ========================================================================== */

