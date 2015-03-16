//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Winsock for OWL subsystem.
/// Based on work by Paul Pedriana, 70541.3223@compuserve.com
//----------------------------------------------------------------------------

#if !defined(OWL_WSKSOCK_H)
#define OWL_WSKSOCK_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/window.h>
#include <owl/wskaddr.h>


namespace owl {

#define WINSOCK_NOERROR (int)0
#define WINSOCK_ERROR   (int)SOCKET_ERROR

#define N_DEF_MAX_READ_BUFFFER_SIZE 8192

//
//
//
#define MSG_SOCKET_NOTIFY ((UINT)(WM_USER+301))		///< User-defined message used for socked notifications
#define FD_ALL            (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT|FD_CLOSE)
#define FD_NONE           (0)

//
/// Enumeration describing the type of event notifications you want to
/// receive for a given socket.
//
enum TNotificationSet {
  NotifyNone    = 0x00,   ///< No notifications
  NotifyRead    = 0x01,   ///< Notification of readiness for reading
  NotifyWrite   = 0x02,   ///< Notification of readiness for writing
  NotifyOOB     = 0x04,   ///< Notification of the arrival of out-of-band data
  NotifyAccept  = 0x08,   ///< Notification of incoming connections
  NotifyConnect = 0x10,   ///< Notification of completed connection
  NotifyClose   = 0x20,   ///< Notification of socket closure
  NotifyAll     = 0x3F    ///< All notifications
};

//
// Forward ref. 
//
class _OWLCLASS TSocket;

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

//
/// \class TSocketWindow
// ~~~~~ ~~~~~~~~~~~~~
/// Derived from TWindow, a private window used to catch notification messages.
//
class _OWLCLASS TSocketWindow : public TWindow {
  public:
    TSocketWindow(TSocket* socketParent);
    TSocketWindow& operator =(TSocketWindow& src); // !CQ const?

    void SetNotificationSet(int notificationSet);
    void SetNotificationWindow(TWindow* windowNotification);

    int  GetLastError();
    int  StartAcceptNotification();
    int  StartRegularNotification();
    int  StartCustomNotification(int selectOptions);
    int  CancelNotification();

    TSocket* GetSocketParent() const;
    void SetSocketParent(TSocket* socket);

  public_data:
    static uint  MsgSocketNotification; ///< Message used to notify hwndNotification, if hwndNotification is used.
    TSocket*     SocketParent;          ///< Exported so Parent can have easy access to it.

  protected:
    int           SelectOptions;      ///< We need to keep our own copy of this so we can do an assignment operator.
    TWindow*      WindowNotification; ///< A second window that can be notified instead of the Socket.
    int           NotificationSet;    ///< Types of notification to respond to
    int           LastError;          ///< Last error

    TResult DoNotification(TParam1, TParam2);

  DECLARE_RESPONSE_TABLE(TSocketWindow);
};

//
/// \class TSocket
// ~~~~~ ~~~~~~~
/// TSocket encapsulates the basic attributes of a socket. A socket is an endpoint
/// of communication to which a name may be bound. Each socket in use has a type and
/// an associated process.
//
class _OWLCLASS TSocket {
  public:
    /// How to shutdown the socket.
    //
    enum TShutMode {
      ShutModeNoRecv = 0,        ///< No more receives on the socket.
      ShutModeNoSend = 1,        ///< No more sends on the socket.
      ShutModeNoRecvSend = 2     ///< No more sends or receives.
    };

    TSocket();
    TSocket(SOCKET& newS);
    TSocket(TSocketAddress& newSocketAddress, int nNewFamily = PF_INET,
            int nNewType = SOCK_STREAM, int nNewProtocol = 0);
    virtual ~TSocket();

    operator SOCKET() const;
    virtual int CloseSocket();
    virtual int ShutDownSocket(TShutMode shutMode = ShutModeNoRecvSend);

    TSocket& operator =(TSocket& newSocket);
    friend bool operator ==(const TSocket& socket1, const TSocket& socket2);

    // Commands
    //
    virtual void SetNotificationSet(int notificationSet);
    virtual void SetNotificationWindow(TWindow* windowNotification);
    virtual int  CreateSocket();
    /// binds to a given address
    //
    virtual int  BindSocket(const TSocketAddress& addressToBindTo);

    /// binds to our address
    //
    virtual int  BindSocket();
    virtual void SetMyAddress(TSocketAddress& newSocketAddress);
    virtual void SetPeerSocketAddress(TSocketAddress& newPeerSocketAddress);
    virtual void SetSocketStyle(int nNewFamily = PF_INET,
                                int nNewType = SOCK_STREAM,
                                int nNewProtocol = 0);
    virtual int StartAcceptNotification();
    virtual int StartRegularNotification();
    virtual int StartCustomNotification(int nSelectionOptions);
    virtual int CancelNotification();

    // Tells class Sockete not to close the socket on deletion.
    bool    SetSaveSocketOnDelete(bool saveSocket = true);

    virtual int  ConvertProtocol(char* protocol);

    // Info
    //
    int GetLastError();
    virtual ulong GetDriverWaitingSize();
    virtual ulong GetTotalWaitingSize();
    virtual int GetMyAddress(  TSocketAddress& socketAddress,
                              int& nAddressLength, SOCKET& socket);
    virtual int GetMyAddress(  TSocketAddress& socketAddress,
                              int& nAddressLength);
    virtual int GetPeerAddress(  TSocketAddress& socketAddress,
                                int& nAddressLength, SOCKET& socket);
    virtual int GetPeerAddress(  TSocketAddress& socketAddress,
                                int& nAddressLength);

    // Read/Write (Send/Receive)
    //
    virtual void SetMaxReadBufferSize(int nNewMaxReadBufferSize);

    // Options
    //
    int SetBroadcastOption(bool bBroadcast);
    int SetDebugOption(bool bDebug);
    int SetLingerOption(bool bLinger, ushort nLingerTime=0);
    int SetRouteOption(bool bRoute);
    int SetKeepAliveOption(bool bKeepAlive);
    int SetOOBOption(bool bSendOOBDataInline);
    int SetReceiveBufferOption(int nReceiveBufferSize);
    int SetSendBufferOption(int nSendBufferSize);
    int SetReuseAddressOption(bool bAllowReuseAddress);

    int GetBroadcastOption(bool& bBroadcast);
    int GetDebugOption(bool& bDebug);
    int GetLingerOption(bool& bLinger, ushort& nLingerTime);
    int GetRouteOption(bool& bRoute);
    int GetKeepAliveOption(bool& bKeepAlive);
    int GetOOBOption(bool& bSendOOBDataInline);
    int GetReceiveBufferOption(int& nReceiveBufferSize);
    int GetSendBufferOption(int& nSendBufferSize);
    int GetReuseAddressOption(bool& bAllowReuseAddress);

  public_data:
    SOCKET         Handle;             ///< The Socket handle
    bool           Bound;              ///< A flag that we can use to tell if
                                       ///< socket is bound or not.  It gets set
                                       ///< on automatically, but doesn't get
                                       ///< set off automatically.
    TSocketAddress SocketAddress;      ///< My address.
    TSocketAddress PeerSocketAddress;  ///< Address of Peer to communicate with.
                                       ///< Used for datagrams and connections.

  protected:
    int            Family;             ///< PF_INET, etc. (this is the protocol family)
    int            Type;               ///< SOCK_STREAM, etc.
    int            Protocol;           ///< IPPROTO_TCP, etc.
    int            LastError;          ///< Last Error.
    int            MaxReadBufferSize;  ///< Maximum buffer size
    short          SaveSocket;         ///< Save the socket on deletion?
    TSocketWindow  Window;             ///< Will receive internal notifications and pass them to this class.
    friend class   TSocketWindow;

    // Protected initialization
    //
    void Init();      // Sets up the friend window.

    int  SocketsCallCheck(int error);

    // Notification
    //
    virtual int DoReadNotification(const SOCKET& s, int nError);
    virtual int DoWriteNotification(const SOCKET& s, int nError);
    virtual int DoOOBNotification(const SOCKET& s, int nError);
    virtual int DoAcceptNotification(const SOCKET& s, int nError);
    virtual int DoConnectNotification(const SOCKET& s, int nError);
    virtual int DoCloseNotification(const SOCKET& s, int nError);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Return the last error on the socket.
//
inline int
TSocketWindow::GetLastError()
{
  return LastError;
}

//
/// Return the set of notifications socket will catch.
//
inline void
TSocketWindow::SetNotificationSet(int notificationSet)
{
  NotificationSet = notificationSet;
}

//
/// Set the new notification window.
//
inline void
TSocketWindow::SetNotificationWindow(TWindow* windowNotification)
{
  WindowNotification = windowNotification;
}

inline TSocket* TSocketWindow::GetSocketParent() const
{
	return SocketParent;
}

inline void TSocketWindow::SetSocketParent(TSocket* socket)
{
	SocketParent = socket;
}


//
/// Assigns new set of notifications socket will catch.
//
inline void
TSocket::SetNotificationSet(int notificationSet)
{
  Window.SetNotificationSet(notificationSet);
}

//
/// Sets the new notification window.
//
inline void
TSocket::SetNotificationWindow(TWindow* windowNotification)
{
  Window.SetNotificationWindow(windowNotification);
}

//
/// Returns the handle of the socket.
//
inline
TSocket::operator SOCKET() const
{
  return Handle;
}

//
/// Saves the socket on deletion.
//
inline bool
TSocket::SetSaveSocketOnDelete(bool saveSocket)
{
  return (SaveSocket = saveSocket);
}

//
/// Returns the last error of the socket.
//
inline int
TSocket::GetLastError()
{
  return LastError;
}

} // OWL namespace


#endif  // OWL_WSKSOCK_H
