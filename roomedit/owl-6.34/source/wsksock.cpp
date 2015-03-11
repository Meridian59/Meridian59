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
/// Default constructor for a socket. The individual members of the TSocket can be
/// set later.
//
TSocket::TSocket()
:
  Handle(INVALID_SOCKET),
  Bound(false),
  Family(PF_UNSPEC),
  Type(0),
  Protocol(0),
  LastError(0),
  MaxReadBufferSize(N_DEF_MAX_READ_BUFFFER_SIZE),
  SaveSocket(0),
  Window(this)
{
  Init();
}

//
/// TSocket(SOCKET&) is a constructor based on a Winsock SOCKET descriptor.
//
TSocket::TSocket(SOCKET& newS)
:
  Handle(newS),
  Bound(false),
  LastError(0),
  MaxReadBufferSize(N_DEF_MAX_READ_BUFFFER_SIZE),
  SaveSocket(0),
  Window(this)
{
  int temp;
  GetMyAddress(SocketAddress, temp, Handle); // Fills in our address information.
  Family = SocketAddress.sa_family;
  GetPeerAddress(SocketAddress, temp, Handle); // Fills in out Peer's address info if it exists.
  Init();
}

//
/// This is the standard constructor for a TSocket. It doesn't call socket() or
/// bind(). These must be done independently.
//
TSocket::TSocket(TSocketAddress& newSocketAddress, int newFamily, int newType, int newProtocol)
:
  Handle(INVALID_SOCKET),
  Bound(false),
  SocketAddress(newSocketAddress),
  Family(newFamily),
  Type(newType),
  Protocol(newProtocol),
  LastError(0),
  MaxReadBufferSize(N_DEF_MAX_READ_BUFFFER_SIZE),
  SaveSocket(0),
  Window(this)
{
  Init();
}

//
/// This TSocket destructor will close the socket it if has not be closed already.
/// It will also delete the friend notification window.
//
TSocket::~TSocket()
{
  if (Handle != static_cast<SOCKET>(INVALID_SOCKET) && !SaveSocket) {
    SetLingerOption(0, 60); //Don't wait till data gets sent.  Just kill the socket now.
    CloseSocket();
  }
}

//
/// This function is an intitialization function called by the TSocket constructors.
/// It simply creates the friend window that the TSocket needs for Winsock
/// notifications.
//
void TSocket::Init()
{
  // This is for initialization steps that are common to all constructors.
  //
  try {
     Window.Create();
  }
  catch (...) {
    //::MessageBox();
  }
}

//
/// Checks the return error value from a sockets call, caching the last error if one
/// occured (i.e., error is non-zero). Returns a Winsock error/noerror code.
//
int TSocket::SocketsCallCheck(int error)
{
  if (error) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
// Does a deep copy of the TSocket, as much as possible.
//
TSocket& TSocket::operator =(TSocket& newSocket)
{
  Family            = newSocket.Family;
  Type              = newSocket.Type;
  Protocol          = newSocket.Protocol;
  LastError         = newSocket.LastError;
  Handle            = newSocket.Handle;
  SocketAddress     = newSocket.SocketAddress;
  PeerSocketAddress = newSocket.PeerSocketAddress;
  SaveSocket        = newSocket.SaveSocket;

  // Copy the friend (helper) window.  Note that there is an operator= defined
  //  for the class TSocketWindow.  However, the TSocketWindow::operator= will
  //  set its 'SocketParent' member to be the newSocket SocketWindow's previous
  //  TSocket parent, which may not be us.  Thus, after setting 'Window =
  //  newSocket.Window;', we set 'Window.SocketParent = this;'.
  //
  Window              = newSocket.Window;
  Window.SetSocketParent(this);

  return *this;
}

//
/// While it's possible that two sockets could refer to the same SOCKET (though this
/// would likely create a mess if not governed with care), it's defined as not
/// possible that two Sockets could have the same window member. This is because the
/// window is created uniquely on construction for each TSocket.
//
bool operator ==(const TSocket& socket1, const TSocket& socket2)
{
  return socket1.Handle == socket2.Handle && socket1.Window == socket2.Window;
}

//
/// This function says to listen only to FD_ACCEPT messages. Note that a socket set
/// up to be a listening socket will never be a connected socket, and a connected
/// socket will never receive FD_ACCEPT messages. Thus all stream sockets are
/// implicitly either connected sockets or listening sockets. Since the accepted
/// socket needs a different notification window from the listening socket, and the
/// sockets specification says that an accepted socket inherits the notification
/// properties of the listening socket, the listening socket must not be set to
/// receive FD_READ, etc, notifications. This is because it's possible that between
/// the accept() call for the new socket and the WSAAsyncSelect() call for the new
/// socket, data may be received for the new socket. Thus the listening socket may
/// get sent the message and it would never get routed to the new socket. Calling
/// this function is saying that this SocketWindow is for listening for connections.
/// 
/// The return value is WINSOCK_ERROR or WINSOCK_NOERROR. You can then examine
/// GetLastError().
//
int TSocket::StartAcceptNotification()
{
  int error = Window.StartAcceptNotification();
  if (error == WINSOCK_ERROR) {
    LastError = Window.GetLastError();
  }
  return error;
}

//
/// This function turns on all Winsock notifications except FD_ACCEPT. Calling this
/// function is saying that this SocketWindow is for connections rather than for
/// listening. Since a Winsock socket cannot be a listening socket and a connected
/// socket at the same time, the notification functions are separated from each
/// other: StartAcceptNotification() and StartRegularNotification().
/// 
/// The return value is WINSOCK_ERROR or WINSOCK_NOERROR. You can then examine
/// GetLastError().
//
int TSocket::StartRegularNotification()
{
  int error = Window.StartRegularNotification();
  if (error == WINSOCK_ERROR) {
    LastError = Window.GetLastError();
  }
  return error;
}

//
/// The return value is WINSOCK_ERROR or WINSOCK_NOERROR. You can then examine
/// GetLastError().
//
int TSocket::StartCustomNotification(int selectionOptions)
{
  int error = Window.StartCustomNotification(selectionOptions);
  if (error == WINSOCK_ERROR) {
    LastError = Window.GetLastError();
  }
  return error;
}

//
/// CancelNotification() turns off the notification to this window. This also
/// changes the socket to be blocking.
/// The return value is WINSOCK_ERROR or WINSOCK_NOERROR. You can then examine
/// GetLastError().
//
int TSocket::CancelNotification()
{
  int error = Window.CancelNotification();
  if (error == WINSOCK_ERROR) {
    LastError = Window.GetLastError();
  }
  return error;
}

//
/// The SetSocketStyle function can be used to set or change some TSocket member
/// data. Note that the newFamily is also represented in the TSocketAddress member,
/// and so they should match.
//
void TSocket::SetSocketStyle(int newFamily, int newType, int newProtocol)
{
  Family   = newFamily;
  Type     = newType;
  Protocol = newProtocol;
}

//
/// Converts a string protocol to integer value. Makes assumptions about the
/// protocol string. Only "tcp" and udp return valid values.
//
int TSocket::ConvertProtocol(char* protocol)
{
  if (!protocol)
    return IPPROTO_TCP;
  if (strcmp("tcp", protocol) == 0)
    return IPPROTO_TCP;
  if(strcmp("udp", protocol) == 0)
    return IPPROTO_UDP;
  return IPPROTO_IP;
}

//
/// The CreateSocket function is much like the Winsock socket() function. This
/// function assumes that nFamily, nType, and nProtocol are already set properly.
/// Note also that since the return of socket() is assigned to 's', that 's' must
/// not already be used. This is another way of saying that there can only be one
/// SOCKET for each TSocket object.
//
int TSocket::CreateSocket()
{
  Handle = TWinSockDll::socket(Family, Type, Protocol);
  return SocketsCallCheck(Handle == static_cast<SOCKET>(INVALID_SOCKET));
}

//
/// The CloseSocket() function is much like the Winsock closesocket() function.
//
int TSocket::CloseSocket()
{
  if (Handle != static_cast<SOCKET>(INVALID_SOCKET)) {
    int error = TWinSockDll::closesocket(Handle);
    if (error) {
      LastError = TWinSockDll::WSAGetLastError();
      return WINSOCK_ERROR;
    }
  }
  Handle = INVALID_SOCKET;  // It's invalid now.
  Bound = false;
  return WINSOCK_NOERROR;
}

//
/// The ShutDownSocket() function is much like the Winsock shutdown() function. Note
/// that shutting down a socket essentially means that you can't un-shut it down.
/// It's a graceful way of preparing to end a session, somewhat like a yellow
/// stoplight. Use this function to close your socket, while still allowing data be
/// received from the network. This is as opposed to CloseSocket(), which kills all
/// transfers in both directions. shutMode is one of the enumerations:
/// ShutModeNoRecv, ShutModeNoSend, or ShutModeNoRecvSend.
//
int TSocket::ShutDownSocket(TShutMode shutMode)
{
  if (Handle != static_cast<SOCKET>(INVALID_SOCKET)) {
    int error = TWinSockDll::shutdown(Handle, shutMode);
    if (error) {
      LastError = TWinSockDll::WSAGetLastError();
      return WINSOCK_ERROR;
    }
    Handle = INVALID_SOCKET;  // It's invalid now.
    Bound = false;
  }
  return WINSOCK_NOERROR;
}

//
/// BindSocket is much like the Winsock bind() function. Regardless of what
/// mySocketAddress may have been previously, a call to 'bind()' immediately makes
/// the socket's address the one put into the bind() call. Thus, mySocketAddress is
/// always assigned to be boundSocketAddress. The address argument must be in
/// network byte ordering. On the other hand, the SocketAddress class always keeps
/// its addresses in network byte ordering.
//
int TSocket::BindSocket(const TSocketAddress& addressToBindTo)
{
  SocketAddress = addressToBindTo;

  // bind() ideally returns 0.
  //
  if (TWinSockDll::bind(Handle, &SocketAddress, sizeof(sockaddr))) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  Bound = true;
  return WINSOCK_NOERROR;
}

//
/// This BindSocket simply binds with the previously defined member data socket
/// address.
//
int TSocket::BindSocket()
{
  return BindSocket(SocketAddress);
}

//
/// This function stores the address into the reference argument 'socketAddress'.
/// 'addressLength' will hold the length of the address. 'socket' refers to the
/// socket whose address will be examined.
//
int TSocket::GetMyAddress(TSocketAddress& socketAddress, int& addressLength, SOCKET& socket)
{
  return SocketsCallCheck(!TWinSockDll::getsockname(socket, &socketAddress, &addressLength));
}

//
/// This function stores the address into the reference argument 'socketAddress'.
/// 'addressLength' will hold the length of the address. Uses the SOCKET in my
/// member data as the socket to get the address of.
//
int TSocket::GetMyAddress(TSocketAddress& socketAddress, int& addressLength)
{
  return GetMyAddress(socketAddress, addressLength, Handle);
}

//
/// The GetPeerAddress() function is much like the Winsock getpeername() function.
/// The Winsock getpeername() function is misnamed; it should be getpeeraddress().
/// socketAddress will be changed to have the right addressing info, and
/// nAddressLength will be set to be the address length.
/// Note that this function can be used to get the address for any socket
/// descriptor, not just our own socket descriptor.
//
int TSocket::GetPeerAddress(TSocketAddress& socketAddress, int& addressLength, SOCKET& socket)
{
  // This code only works because SOCKET is defined as a uint and not a struct
  //
  if (!socket)
    socket = Handle;

  return SocketsCallCheck(!TWinSockDll::getpeername(socket, &socketAddress, &addressLength));
}

//
/// This version of GetPeerAddress() works on our own socket descriptor.
//
int TSocket::GetPeerAddress(TSocketAddress& socketAddress, int& addressLength)
{
  return GetPeerAddress(socketAddress, addressLength, Handle);
}

//
/// This may be useful for changing the address or setting the address before
/// binding. It's no good to change this after binding, as a binding is a permanent
/// association between a socket descriptor and a full address (for IP, this is a
/// ushort port and ulong address).
//
void TSocket::SetMyAddress(TSocketAddress& newSocketAddress)
{
  SocketAddress = newSocketAddress;
}

//
/// The 'myPeerSocketAddress' member variable is useful for Datagram sockets because
/// it allows them to specify a default destination to send datagrams to. With a
/// default destination, a datagram socket that always or often sends to one address
/// can simply call the Write or Send functions with no address arguments and
/// the data will send to the default address. This function can also be used by a
/// stream socket to set the address for a peer that it wants to connect to.
//
void TSocket::SetPeerSocketAddress(TSocketAddress& newPeerSocketAddress)
{
  PeerSocketAddress = newPeerSocketAddress;
}

#if !defined(__GNUC__) //JJH removal of pragma warn for GCC
#pragma warn -cln
#endif

//
/// GetDriverWaitingSize() is much like calling ioctlsocket(s, FIONREAD,...) in
/// Winsock. It returns the number of bytes waiting to be read on the socket. For
/// datagrams, it is the size of the next datagram. For streams, it should be the
/// total waiting bytes.
//
ulong TSocket::GetDriverWaitingSize()
{
  ulong charsWaiting;
  if (TWinSockDll::ioctlsocket(Handle, FIONREAD, &charsWaiting)) {
    return 0;
  }
  return charsWaiting;
}
#if !defined(__GNUC__) //JJH removal of pragma warn for GCC
#pragma warn .cln
#endif

//
/// Returns the total number of bytes waiting to be read.
//
ulong TSocket::GetTotalWaitingSize()
{
  return GetDriverWaitingSize();
}

//
/// This function gets called whenever the socket gets a read notification. This
/// means that data on the port is ready to be read. Thus this function must be
/// subclassed by a DatagramSocket and StreamSocket.
//
int TSocket::DoReadNotification(const SOCKET& /*socket*/, int /*nError*/)
{
  return 0;
}

//
/// The generic socket doesn't know how many bytes it can send, since this limit is
/// dependent on whether the socket is a stream or datagram socket. Thus this
/// function must be subclassed by a DatagramSocket and StreamSocket.
//
int TSocket::DoWriteNotification(const SOCKET& /*s*/, int /*nError*/)
{
  return 0;
}

//
/// This isn't responded to in the generic TSocket class.
//
int TSocket::DoOOBNotification(const SOCKET& /*s*/, int /*nError*/)
{
  return 0;
}

//
/// This isn't responded to in the generic TSocket class.
//
int TSocket::DoAcceptNotification(const SOCKET& /*s*/, int /*nError*/)
{
  return 0;
}

//
/// This isn't responded to in the generic TSocket class.
//
int TSocket::DoConnectNotification(const SOCKET& /*s*/, int /*nError*/)
{
  return 0;
}

//
/// This isn't responded to in the generic TSocket class.
//
int TSocket::DoCloseNotification(const SOCKET& /*s*/, int /*nError*/)
{
  return 0;
}

//
/// This should be called by someone who knows what the correct value is.
//
void TSocket::SetMaxReadBufferSize(int maxReadBufferSize)
{
  if(maxReadBufferSize > 0)
    MaxReadBufferSize = maxReadBufferSize;
}

//
/// Allows transmission of broadcast messages.
//
int TSocket::SetBroadcastOption(bool broadcast)
{
  // Must pass an int, not a bool, to setsockopt
  //
  BOOL bcast = broadcast;
  return SocketsCallCheck(TWinSockDll::setsockopt(Handle, SOL_SOCKET, SO_BROADCAST, (char*)&bcast, sizeof bcast));
}

//
/// Records debugging info.
//
int TSocket::SetDebugOption(bool debug)
{
  // Must pass an BOOL, not a bool, to setsockopt
  //
  BOOL bDebug = debug;
  return SocketsCallCheck(TWinSockDll::setsockopt(Handle, SOL_SOCKET, SO_DEBUG, (char*)&bDebug, sizeof bDebug));
}

//
/// If you set 'linger' to true, then that means to linger for 'lingerTime' seconds.
/// Examples:
/// - linger=true, lingerTime=0  Hard immediate close. All queued data for sending
/// gets canned immediately.
/// - linger=true, lingerTime=2.  Graceful close. Waits 2 seconds to try to send any
/// pending data.
/// - linger=false, lingerTime=\<any\>.  "Graceful" immediate close. Causes data to be
/// still in queue to send when ready.
//
int TSocket::SetLingerOption(bool linger, ushort lingerTime)
{
  LINGER lingerOptions;

  lingerOptions.l_onoff = linger;  //Note that bLinger is a bool and LINGER.l_onoff is a u_short.
  lingerOptions.l_linger = lingerTime;

  return SocketsCallCheck(TWinSockDll::setsockopt(Handle, SOL_SOCKET, SO_LINGER, (char*)&lingerOptions, sizeof lingerOptions));
}

//
/// A false argument means don't route.
//
int TSocket::SetRouteOption(bool route)
{
  BOOL bRout = !route;
  return SocketsCallCheck(TWinSockDll::setsockopt(Handle, SOL_SOCKET, SO_DONTROUTE, (char*)&bRout, sizeof bRout));
}

//
/// Sends keepAlive messages.
//
int TSocket::SetKeepAliveOption(bool keepAlive)
{
  BOOL bKeepAlive = keepAlive;
  return SocketsCallCheck(TWinSockDll::setsockopt(Handle, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof bKeepAlive));
}

//
/// Receives out-of-band (OOB) data in the normal data stream.
//
int TSocket::SetOOBOption(bool sendOOBDataInline)
{
  BOOL bSendOOBDataInline = sendOOBDataInline;
  return SocketsCallCheck(TWinSockDll::setsockopt(Handle, SOL_SOCKET, SO_OOBINLINE, (char*)&bSendOOBDataInline, sizeof bSendOOBDataInline));
}

//
/// Sets the buffer size for receiving messages.
//
int TSocket::SetReceiveBufferOption(int receiveBufferSize)
{
  return SocketsCallCheck(TWinSockDll::setsockopt(Handle, SOL_SOCKET, SO_RCVBUF, (char*)&receiveBufferSize, sizeof receiveBufferSize));
}

//
/// Sets the buffer size for sending messages.
//
int TSocket::SetSendBufferOption(int sendBufferSize)
{
  return SocketsCallCheck(TWinSockDll::setsockopt(Handle, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, sizeof sendBufferSize));
}

//
/// Allows the socket to bind to an already-bound address.
//
int TSocket::SetReuseAddressOption(bool allowReuseAddress)
{
  BOOL bAllowReuseAddress = allowReuseAddress;
  return SocketsCallCheck(TWinSockDll::setsockopt(Handle, SOL_SOCKET, SO_REUSEADDR, (char*)&bAllowReuseAddress, sizeof bAllowReuseAddress));
}

//
/// Retrieves the current broadcast option.
//
int TSocket::GetBroadcastOption(bool& broadcast)
{
  BOOL bBroadcast; 
  int size = sizeof bBroadcast;
  int retval = SocketsCallCheck(TWinSockDll::getsockopt(Handle, SOL_SOCKET, SO_BROADCAST, (char*)&bBroadcast, &size));
  broadcast = bBroadcast != FALSE;
  return retval;
}

//
/// Retrieves the current debugging option.
//
int TSocket::GetDebugOption(bool& debug)
{
  BOOL bDebug; 
  int size = sizeof bDebug;
  int retval = SocketsCallCheck(TWinSockDll::getsockopt(Handle, SOL_SOCKET, SO_DEBUG, (char*)&bDebug, &size));
  debug = bDebug != FALSE;
  return retval;
}

//
/// Retreives the current linger option.
//
int TSocket::GetLingerOption(bool& linger, ushort& lingerTime)
{
  LINGER lingerOptions;
  int size = sizeof lingerOptions;

  if (SocketsCallCheck(TWinSockDll::getsockopt(Handle, SOL_SOCKET, SO_LINGER, (char*)&lingerOptions, &size)) == WINSOCK_NOERROR) {
    linger = lingerOptions.l_onoff;
    lingerTime = lingerOptions.l_linger;
    return WINSOCK_NOERROR;
  }
  return WINSOCK_ERROR;
}

//
/// Retrieves the routing option.
//
int TSocket::GetRouteOption(bool& route)
{
  BOOL bRoute;
  int size = sizeof bRoute;
  if (SocketsCallCheck(TWinSockDll::getsockopt(Handle, SOL_SOCKET, SO_DONTROUTE, (char*)&bRoute, &size)) == WINSOCK_NOERROR) {
    // route value of true means "don't route."  So we change it to mean what
    // you'd think.
    //
    route = bRoute == FALSE;
    return WINSOCK_NOERROR;
  }
  return WINSOCK_ERROR;
}

//
/// Retrieves the keepAlive option.
//
int TSocket::GetKeepAliveOption(bool& keepAlive)
{
  BOOL bKeepAlive;
  int size = sizeof bKeepAlive;
  int retval = SocketsCallCheck(TWinSockDll::getsockopt(Handle, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, &size));
  keepAlive = bKeepAlive != FALSE;
  return retval;
}

//
/// Retrieves the out-of-band (OOB) option.
//
int TSocket::GetOOBOption(bool& sendOOBDataInline)
{
  BOOL bSendOOBDataInline;
  int size = sizeof bSendOOBDataInline;
  int retval = SocketsCallCheck(TWinSockDll::getsockopt(Handle, SOL_SOCKET, SO_OOBINLINE, (char*)&bSendOOBDataInline, &size));
  sendOOBDataInline = bSendOOBDataInline != FALSE;
  return retval;
}

//
/// Retrieves the current receiving buffer size.
//
int TSocket::GetReceiveBufferOption(int& receiveBufferSize)
{
  int size = sizeof receiveBufferSize;
  return SocketsCallCheck(TWinSockDll::getsockopt(Handle, SOL_SOCKET, SO_RCVBUF, (char*)&receiveBufferSize, &size));
}

//
/// Retrieves the current sending buffer size.
//
int TSocket::GetSendBufferOption(int& sendBufferSize)
{
  int size = sizeof sendBufferSize;
  return SocketsCallCheck(TWinSockDll::getsockopt(Handle, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &size));
}

//
/// Retrieves the reusable address option.
//
int TSocket::GetReuseAddressOption(bool& allowReuseAddress)
{
  BOOL bAllowReuseAddress;
  int size = sizeof bAllowReuseAddress;
  int retval = SocketsCallCheck(TWinSockDll::getsockopt(Handle, SOL_SOCKET, SO_REUSEADDR, (char*)&bAllowReuseAddress, &size));
  allowReuseAddress = bAllowReuseAddress != FALSE;
  return retval;
}

//----------------------------------------------------------------------------

//
//
//
DEFINE_RESPONSE_TABLE1(TSocketWindow, TWindow)
  EV_MESSAGE(MSG_SOCKET_NOTIFY, DoNotification),
END_RESPONSE_TABLE;

//
// Default this to be our standard MSG_SOCKET_NOTIFY
//
uint TSocketWindow::MsgSocketNotification = MSG_SOCKET_NOTIFY;

TSocketWindow::TSocketWindow(TSocket* socketParent)
:
  TWindow(0, _T("TSocketWindow")),
  SocketParent(socketParent),
  SelectOptions(0),
  WindowNotification(0),
  NotificationSet(NotifyAll),
  LastError(0)
{
  Attr.Style = 0;  // Turn off WS_CHILD (the default) style.
}

//
/// From the user's standpoint, the only thing that is required to make two SocketWindows
///  act as equal is to make their parents and selection options the same.  The actual
///  window handle identities are unimportant.  Thus, we keep our original Window handle,
///  even if the newSocketWindow had an empty window handle.
///
/// However, it may be impossible to assign the parent correctly if this operator is being called
///  in the parent's operator=().  The new parent SHOULD be the original parent, yet the
///  newSocketWindow has a new parent.  We cannot know in this function the conditions
///  under which this assignment is called, so we blindly copy the new parent.
///  The parent will have to override this assignment if the old parent is to
///  remain as it was.
///
/// This function does the best it can to make this window act just like newSocketWindow:
//
TSocketWindow& TSocketWindow::operator =(TSocketWindow& src)
{
  SelectOptions      = src.SelectOptions;
  SocketParent       = src.SocketParent;
  WindowNotification = src.WindowNotification;
  NotificationSet    = src.NotificationSet;
  LastError           = src.LastError;

  if (SocketParent->Handle) {
    // Note that if SelectOptions were empty (0), then we are saying to turn
    //  off notifications and make the socket blocking.
    //
    TWinSockDll::WSAAsyncSelect(SocketParent->Handle, *this, MSG_SOCKET_NOTIFY, SelectOptions);
  }
  return *this;
}

//
/// This function says to only listen to FD_ACCEPT messages.  It is important to note that a
///  socket set up to be a listening socket will never be a connected socket, and a connected
///  socket will never receive FD_ACCEPT messages.  Thus all stream sockets are implicitly either
///  connected sockets or listening sockets.
///
/// Since the accepted socket will want to have a different notification window than the
///  listening socket, and the sockets specification says that an accepted socket
///  inherits the notification properties of the listening socket, it is imperative
///  that the listening socket not be set to receive FD_READ, etc notifications.  This
///  is because it is possible that between the accept() call for the new socket and
///  the WSAAsyncSelect() call for the new socket, data may be received for the new socket.
///  Thus the listening socket may get sent the message and it would never get routed to
///  the new socket.
///
/// Calling this function is saying that this SocketWindow is for listening for connections.
///
/// The return value is WINSOCK_ERROR or WINSOCK_NOERROR.  You can then examine GetLastError().
//
int TSocketWindow::StartAcceptNotification()
{
  SelectOptions = FD_ACCEPT;
  return StartCustomNotification(SelectOptions);
}

//
/// This function turns on all Winsock notifications except FD_ACCEPT.
///
/// Calling this function is saying that this SocketWindow is for connections rather
/// than for listening.  Since a Winsock socket cannot be a listening socket and
/// a connected socket at the same time, we have serarate the notification functions
/// from each other: StartAcceptNotification() and StartRegularNotification().
///
/// The return value is WINSOCK_ERROR or WINSOCK_NOERROR.  You can then examine
/// GetLastError().
//
int TSocketWindow::StartRegularNotification()
{
  SelectOptions = FD_ALL & ~FD_ACCEPT;
  return StartCustomNotification(SelectOptions);
}

//
/// The return value is WINSOCK_ERROR or WINSOCK_NOERROR.  You can then examine
/// GetLastError().
//
int TSocketWindow::StartCustomNotification(int selectOptions)
{
  SelectOptions = selectOptions;

  int error = TWinSockDll::WSAAsyncSelect(*SocketParent, *this, MSG_SOCKET_NOTIFY, SelectOptions);
  if (error) {
    LastError = TWinSockDll::WSAGetLastError();
    return WINSOCK_ERROR;
  }
  return WINSOCK_NOERROR;
}

//
/// CancelNotification() turns off the notification to this window. This also changes the
///  socket to be blocking.
/// The return value is WINSOCK_ERROR or WINSOCK_NOERROR.  You can then examine GetLastError().
//
int TSocketWindow::CancelNotification()
{
  SelectOptions = 0;
  return StartCustomNotification(SelectOptions);
}

//
/// DoNotification() is the SocketWindow's protected internal notification system.
//
TResult TSocketWindow::DoNotification(TParam1 param1, TParam2 param2)
{
  SOCKET socket = param1;
  int     error   = (int)WSAGETSELECTERROR(param2);

  if (socket != SocketParent->Handle)
    return 0;

  switch (WSAGETSELECTEVENT(param2)) {
    case FD_READ:
      if (WindowNotification && (NotificationSet & NotifyRead)) {
        return WindowNotification->SendMessage(MsgSocketNotification, param1, param2);
      }
      SocketParent->DoReadNotification(socket, error);
      break;
    case FD_WRITE:
      if (WindowNotification && (NotificationSet & NotifyWrite)) {
        return WindowNotification->SendMessage(MsgSocketNotification, param1, param2);
      }
      SocketParent->DoWriteNotification(socket, error);
      break;
    case FD_OOB:
      if (WindowNotification && (NotificationSet & NotifyOOB)) {
        return WindowNotification->SendMessage(MsgSocketNotification, param1, param2);
      }
      SocketParent->DoOOBNotification(socket, error);
      break;
    case FD_ACCEPT:
      if (WindowNotification && (NotificationSet & NotifyAccept)) {
        return WindowNotification->SendMessage(MsgSocketNotification, param1, param2);
      }
      SocketParent->DoAcceptNotification(socket, error);
      break;
    case FD_CONNECT:
      if (WindowNotification && (NotificationSet & NotifyConnect)) {
        return WindowNotification->SendMessage(MsgSocketNotification, param1, param2);
      }
      SocketParent->DoConnectNotification(socket, error);
      break;
    case FD_CLOSE:
      if (WindowNotification && (NotificationSet & NotifyClose)) {
        return WindowNotification->SendMessage(MsgSocketNotification, param1, param2);
      }
      SocketParent->DoCloseNotification(socket, error);
  }
  return 1;
}

} // OWL namespace
/* ========================================================================== */

