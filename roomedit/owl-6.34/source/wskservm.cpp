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
DEFINE_RESPONSE_TABLE1(TServiceWindow, TWindow)
   EV_MESSAGE(MSG_SERVICE_NOTIFY, DoNotification),
END_RESPONSE_TABLE;

//
/// A TServiceWindow requires a TServiceManager parent.
//
TServiceWindow::TServiceWindow(TServiceManager* serviceManagerParent)
:
  TWindow(0, _T("WinsockServiceWindow")),
  ServiceManagerParent(serviceManagerParent)
{
  Attr.Style = 0;  // Turn off WS_CHILD (the default) style).
}

//
/// This function merely calls the parent notification function. Since Windows is
/// designed to receive only system messages, there is a dummy window that does
/// nothing but simulate the concept of Objects receiving messages (in this case,
/// the ServceManager object). The TServiceWindow hides this from the user
/// (programmer).
//
TResult TServiceWindow::DoNotification(TParam1, TParam2 param2)
{
  ServiceManagerParent->ServiceCompleted(WSAGETASYNCERROR(param2));
  return 1;
}

//----------------------------------------------------------------------------

//
/// Initializes all data members of servent to 0.
//
TServiceEntry::TServiceEntry()
{
  s_name    = 0;
  s_aliases = 0;
  s_port    = 0;
  s_proto   = 0;
}

//----------------------------------------------------------------------------

//
/// This function creates the hidden window and initializes data members.
//
TServiceManager::TServiceManager()
:
  Window(this)
{
  ServiceEntry = (TServiceEntry*)&ServiceBuffer;
  LastServiceRequest          = 0;
  LastError                   = 0;
  LastServiceRequestCompleted = 0;
  OutstandingServiceRequests  = 0;
  try {
    Window.Create();
  }
  catch (...) {
    //Invalid Window exception.
  }
}

//
/// If there are any outstanding requests, this cancels them.
//
TServiceManager::~TServiceManager()
{
  // If there is an outstanding request...
  //
  if (LastServiceRequest && !LastServiceRequestCompleted)
    CancelServiceRequest();
}

//
/// Implements the blocking getservbyport(). sEntry is a pointer to a ServiceEntry*.
/// The caller does not allocate a ServiceEntry; it will be set by the system. Do
/// not change any data in the returned structure. port is passed to this function
/// in network byte ordering. szProtocol is the protocol name, but may be passed as
/// 0 to mean default or first found. The returned entry has the port in network
/// byte order.
//
int TServiceManager::GetService(TServiceEntry*& entry, uint16 port, const char* protocol)
{
  entry = (TServiceEntry*)TWinSockDll::getservbyport(port, protocol);
  if (!entry) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
/// This function implements the blocking getservbyname(). sEntry is a pointer to a
/// ServiceEntry*. If the caller does not allocate a ServiceEntry; it will be set by
/// the system. Do not change any data in the returned structure. The szname
/// argument points to the string representing the service name, such as ftp. It is
/// generally case-sensitive. szProtocol is the protocol name, but may be passed as
/// 0 to mean default or first found. The returned entry has the port in network
/// byte order.
//
int TServiceManager::GetService(TServiceEntry*& entry, const char* name, const char* protocol)
{
  entry = (TServiceEntry*)TWinSockDll::getservbyname(name, protocol);
  if (!entry) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
/// This function is blocking. Given an input service in szName, this function fills
/// nPort with the port (in network order). The port is a reference to an int. If
/// the call returns OK, this will be the port in network ordering. szProtocol is
/// the protocol name, but may be passed as 0 to mean default or first found.
//
int TServiceManager::GetServicePort(char* name, uint16& port, const char* protocol)
{
  TServiceEntry* tempServiceEntry;

  int returnValue = GetService(tempServiceEntry, name, protocol);
  port = tempServiceEntry->s_port; // Note that port is already in host byte order.
  return returnValue;
}

//
/// This function is blocking. The szName needs to be big enough to hold the
/// service's name (N_MAX_SERVICE_NAME). The port must be passed in network byte
/// ordering. szProtocol is the protocol name, but may be passed as 0 to mean
/// default or first found.
//
int TServiceManager::GetServiceName(uint16 port, char* name, const char* protocol)
{
  // Note that because we are calling our own member function, we don't convert
  //  the address from local to network format.
  //
  TServiceEntry* tempServiceEntry;
  int returnValue = GetService(tempServiceEntry, port, protocol);
  if (returnValue == WINSOCK_NOERROR)
    strcpy(name, tempServiceEntry->s_name);
  return returnValue;
}

//
/// This function cancels a pending service that equals the hService. Note that if
/// the service is 0 or unspecified, this function uses its own current service.
//
int TServiceManager::CancelServiceRequest(HANDLE hService)
{
  if (!hService)
    hService = LastServiceRequest;
  if (!hService) {
    LastError = WSAEINVAL;
    return WINSOCK_ERROR;
  }

  int tempValue = TWinSockDll::WSACancelAsyncRequest(hService);
  if (tempValue == SOCKET_ERROR) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  OutstandingServiceRequests--;
  return WINSOCK_NOERROR;
}

//
/// This call is non-blocking. It sets up a callback to its own member window.
/// hService is a HANDLE reference. If the call is successful, it will hold the
/// handle for the asynchrous call. Note that the caller can save the handle, but
/// also that this class stores the handle for itself as well. port is passed into
/// this function in network byte ordering.
/// Note that due to the design of this class, you cannot have two outstanding
/// Service requests that get notified directly to this class. You may use the
/// hwnd-specific notification version of this function to manage multiple requests
/// at a time. You may also simply create more than one instance of this class. The
/// service is complete when bServiceRequestCompleted is TRUE. You must look at
/// nLastError in this case to see if there was an error.
//
int TServiceManager::GetServiceAsync(HANDLE& hService, uint16 port, const char* protocol)
{
  LastServiceRequestCompleted = 0;
  hService = LastServiceRequest = TWinSockDll::WSAAsyncGetServByPort(Window, MSG_SERVICE_NOTIFY,
                                         port, protocol, ServiceBuffer, MAXGETHOSTSTRUCT);
  if (!hService) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }

  OutstandingServiceRequests++;
  return WINSOCK_NOERROR;
}

//
/// This call is nonblocking. It sets up a callback to its own member window.
/// hService is a HANDLE reference. If the call is successful, it will hold the
/// handle for the asynchrous call. Note that the caller can save the handle, but
/// also that this class stores the handle for itself as well. port is passed into
/// this function in network byte ordering.
/// Note that due to the design of this class, you cannot have two outstanding
/// Service requests that get notified directly to this class. You may use the
/// hwnd-specific notification version of this function to manage multiple requests
/// at a time. You may also simply create more than one instance of this class. The
/// service is complete when bServiceRequestCompleted is TRUE. You must look at
/// nLastError in this case to see if there was an error.
//
int TServiceManager::GetServiceAsync(HANDLE& hService, char* name, const char* protocol)
{
  LastServiceRequestCompleted=0;
  hService = LastServiceRequest = TWinSockDll::WSAAsyncGetServByName(Window, MSG_SERVICE_NOTIFY,
                                    name, protocol, ServiceBuffer, MAXGETHOSTSTRUCT);
  if (!hService) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  OutstandingServiceRequests++;
  return WINSOCK_NOERROR;
}

//
/// This function notifies the given wndNotify about the completion of the request.
/// nMessage is the message that the hwndNotify will receive. It defaults to
/// MSG_SERVICE_NOTIFY, which is defined in the TServiceManager's header file. The
/// hService will hold a handle that the caller can use to reference the request on
/// call-back. nPort should be passed in network byte ordering. The chBuffer is a
/// pointer to buffer that will be filled in with a SERVENT. It needs to be at least
/// MAXGETHOSTSTRUCT bytes.
/// If chBuffer is 0 (or not specified), the TServiceManager's internal buffer will
/// be used. szProtocol is the protocol name, but may be passed as 0 to mean default
/// or first found. This class will NOT save the hServcice for itself. wParam will
/// be equal to the hService returned. WSAGETSYNCERROR(lParam) holds an error, if
/// any (0 is OK). WSAGETSYNCBUFLEN(lParam) holds actual length of the buffer.
//
int TServiceManager::GetServiceAsync(TWindow& wndNotify, HANDLE& hService,
                                     uint16 port, const char* protocol,
                                     uint message, char* buffer)
{
  char* bufferToUse = buffer ? buffer : ServiceBuffer;

  LastServiceRequestCompleted=0;
  hService = TWinSockDll::WSAAsyncGetServByPort(wndNotify, message, port,
                                    protocol, bufferToUse, MAXGETHOSTSTRUCT);
  if (!hService) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
/// This function notifies the given wndNotify about the completion of the request.
/// nMessage is the message that the hwndNotify will receive. It defaults to
/// MSG_SERVICE_NOTIFY, which is defined in the TServiceManager's header file. The
/// hService will hold a handle that the caller can use to reference the request on
/// call-back. The chBuffer is a pointer to a buffer that will be filled in with a
/// SERVENT. It needs to be at least MAXGETHOSTSTRUCT bytes.
/// If chBuffer is 0 (or not specified), the TServiceManager's internal buffer will
/// be used. szName is a pointer to a c string service name, such as ftp. szProtocol
/// is the protocol name, but may be passed as 0 to mean default or first found.
/// This class will NOT save the hService for itself. wParam will be equal to the
/// hService returned. WSAGETSYNCERROR(lParam) holds an error, if any (0 is OK).
/// WSAGETSYNCBUFLEN(lParam) holds actual length of the buffer.
//
int TServiceManager::GetServiceAsync(TWindow& wndNotify, HANDLE& hService,
                                     char* name, const char* protocol,
                                     uint message, char* buffer)
{
  char* bufferToUse = buffer ? buffer : ServiceBuffer;

  LastServiceRequestCompleted = 0;
  hService = TWinSockDll::WSAAsyncGetServByName(wndNotify, message, name,
                                     protocol, bufferToUse, MAXGETHOSTSTRUCT);
  if (!hService) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
/// This function is called whenever an internal window callback is used for an
/// asynchronous call. When this function is called, the ServiceRequestCompleted
/// member is true (it got set to true right before this function was called). You
/// may want to override this function. If you do, you must call the base version.
//
void TServiceManager::ServiceCompleted(int error)
{
  if (error == SOCKET_ERROR)
    LastError = error;
  OutstandingServiceRequests--;
  LastServiceRequestCompleted = 1;
}

} // OWL namespace
/* ========================================================================== */

