//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Winsock for OWL subsystem.
/// Based on work by Paul Pedriana, 70541.3223@compuserve.com
//----------------------------------------------------------------------------

#if !defined(OWL_WSKSOCKD_H)
#define OWL_WSKSOCKD_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/wsksock.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

//
/// \class TDatagramSocket
// ~~~~~ ~~~~~~~~~~~~~~~
/// The TDatagramSocket encapsulates a Winsock datagram (non-reliable, packet-based)
/// socket. While Winsock version 1.1 is for TCP/IP, Winsock 2.0 and later will
/// likely support other protocols. The TDatagramSocket Windows Socket Class class
/// is designed to be as forward compatible with Winsock 2.0 as is currently
/// possible; it contains no TCP/UDP-specific calls, and uses TSocketAddresses and
/// not TINetSocketAddresses.
/// \note Unlike stream sockets, datagram sockets are unreliable - i.e. the 
///       bi-directional flow of data is not guaranteed to be sequenced and/or 
///       unduplicated.
//
class _OWLCLASS TDatagramSocket : public TSocket {
  public:
    TDatagramSocket();
    TDatagramSocket(SOCKET& newS);
    TDatagramSocket(TSocketAddress& newSocketAddress, int addressFormat = PF_INET,
                    int type = SOCK_DGRAM, int protocol = 0);
    TDatagramSocket& operator =(TDatagramSocket& newDatagramSocket1);

    void SetMaxPacketSendSize(int size);
    int Read(char* data, int& charsToRead, TSocketAddress& sAddress);
    int Write(char* data, int& charsToWrite, TSocketAddress& outSocketAddress,
              bool becomeOwnerOfData = true, bool copyData = true);
    int Write(char* data, int& charsToWrite, bool becomeOwnerOfData = true,
              bool copyData = true);

  protected:
    /// Defined by the WSAStartup() call return information (WSAData).
    //
    static int MaxPacketSendSize;

    int DoReadNotification(const SOCKET& s, int error);
    int DoWriteNotification(const SOCKET& s, int error);
};

#define N_DEF_MAX_QUEUED_CONNECTIONS 5

//
/// \class TStreamSocket
// ~~~~~ ~~~~~~~~~~~~~
/// The TStreamSocket encapsulates a Winsock stream socket. While Winsock version
/// 1.1 is for TCP/IP, Winsock 2.0 and later will likely support other protocols.
/// The TStreamSocket Windows Socket Class class is designed to be as
/// forward-compatible with Winsock 2.0 as is currently possible; it contains no
/// TCP/UDP-specific calls, and uses TSocketAddress and not TINetSocketAddress (IP
/// addresses).
/// In addition to supporting standard data reading and writing, the TStreamSocket
/// also supports out-of-band reading and writing. Winsock calls such as accept()
/// and listen() are implemented.
//
class _OWLCLASS TStreamSocket : public TSocket {
  public:
    /// Current status of this stream socket
    //
    enum TConnectStatus {
      NotConnected,       ///< This socket is not used
      ConnectPending,     ///< Connection is pending
      Connected,          ///< Currently connected
      Listening           ///< Waiting for a connection
    } ConnectStatus;

    TStreamSocket();
    TStreamSocket(SOCKET& newS);
    TStreamSocket(TSocketAddress& socketAddress, int addressFormat = PF_INET,
                  int Type = SOCK_STREAM, int protocol = 0);
    TStreamSocket& operator =(TStreamSocket& src);

    int Listen(int nMaxQueuedConnections = N_DEF_MAX_QUEUED_CONNECTIONS);
    int Connect();
    int Connect(TSocketAddress& addressToConnectTo);
    int Accept(TStreamSocket& socket);
    int Accept(SOCKET& socket, sockaddr& sAddress);

    int Read(char* data, int& charsToRead);
    int Write(char* data, int& charsToWrite, int flags = 0,
              bool becomeOwnerOfData = true, bool copyData = true);
              //Is the same as send func
    int ReadOOB(char* data, int& charsToRead);
    int WriteOOB(char* data, int& charsToWrite, int nFlags = MSG_OOB,
                 bool becomeOwnerOfData = true, bool copyData = true);
                 //ORs nFlags w MSG_OOB.

  protected:
    int DoReadNotification(const SOCKET& s, int nError);
    int DoWriteNotification(const SOCKET& s, int nError);
    int DoOOBNotification(const SOCKET& s, int nError);
    int DoAcceptNotification(const SOCKET& s, int nError);
    int DoConnectNotification(const SOCKET& s, int nError);
    int DoCloseNotification(const SOCKET& s, int nError);
};

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Copies the datagram socket connection information.
//
inline TDatagramSocket&
TDatagramSocket::operator =(TDatagramSocket& newDatagramSocket1)
{
  TSocket::operator =(newDatagramSocket1);
  return *this;
}

//
/// This function sets the maximum size of the send packet buffer.
//
inline void
TDatagramSocket::SetMaxPacketSendSize(int size)
{
  MaxPacketSendSize = size;
}


} // OWL namespace



#endif  // OWL_WSKSOCKD_H
