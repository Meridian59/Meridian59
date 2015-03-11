//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Winsock for OWL subsystem.
/// Based on work by Paul Pedriana, 70541.3223@compuserve.com
//----------------------------------------------------------------------------

#if !defined(OWL_WSKADDR_H)
#define OWL_WSKADDR_H

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
/// \class TSocketAddress
// ~~~~~ ~~~~~~~~~~~~~~
/// The TSocketAddress class stores a Winsock socket address.  This class is much
/// like the sockaddr class; in fact, it is a subclass of sockaddr.  Note that the
/// TSocketAddress class does not store Internet (IP) addresses specifically.  It
/// stores generic socket addresses.  The TINetSocketAddress class is used to store
/// Internet addresses. 
///
/// TSocketAddress normally stores its data in network byte ordering, as opposed to host byte ordering.
//
class _OWLCLASS TSocketAddress : public sockaddr {
  public:
    TSocketAddress();
    TSocketAddress(const sockaddr& src);
    TSocketAddress(const TSocketAddress& src);
    TSocketAddress(ushort family, char* data, short dataLength);

    TSocketAddress& operator =(const sockaddr& src);
    friend bool operator ==(const TSocketAddress& address1, const TSocketAddress& address2);

    ushort GetFamily() const;
    void SetFamily(ushort family);
    void SetAddress(ushort family, const char* data, short dataLength);
};

//
/// \class TINetSocketAddress
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// The TINetSocketAddress class encapsulates a Winsock Internet address.  This
/// class stores a complete Internet address, which includes an address family, a
/// port, and a 32 bit (4 byte) IP address.  The Internet address-specific
/// information is stored in the sa_data field of the sockaddr. 
///
/// \todo What about IPv6?
//
class _OWLCLASS TINetSocketAddress : public TSocketAddress {
  public:
  	
		/// TINetClass is an enumeration that specifies which network class it belongs to.
		/// There are three network classes: class A, class B, and class C. The classes
		/// determine how many possible hosts there are on the network. Note that these
		/// classes are the same when programming Windows Sockets.
		///
    /// An address resolves to a 32-bit value. Each class has a different encoding scheme.
    //
    enum TINetClass {
      ClassA,         ///< A class net addressing
      ClassB,         ///< B class net addressing
      ClassC,         ///< C class net addressing
      ClassUnknown    ///< Unknown class net addressing
    };

    TINetSocketAddress();
    TINetSocketAddress(const sockaddr& src);
    TINetSocketAddress(ushort newPort, ulong newAddress = INADDR_ANY,
                       ushort newFamily = AF_INET);
    TINetSocketAddress(ushort newPort, const char* newAddress,
                       ushort newFamily = AF_INET);

    TINetSocketAddress& operator =(const sockaddr& src);
    operator sockaddr_in() const;

    TINetClass GetClass() const;
    ushort GetPort() const;
    ulong GetNetworkAddress() const;
    ulong GetNetwork() const;
    ulong GetNetwork(ulong subnet) const;
    ulong GetNode() const;
    ulong GetNode(ulong subnet) const;
    void SetAddress(ushort newFamily, ushort newPort, ulong newAddress);
    void SetPort(ushort port);
    void SetNetworkAddress(ulong address);
    void SetNetworkAddress(const char* addressDottedDecimal);
    void SetFiller();

    static ulong ConvertAddress(const char * address);
    static char * ConvertAddress(ulong address);
    static short IsAddressDottedDecimal(const char * address);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns the family of addressing this address belongs to.
//
inline ushort
TSocketAddress::GetFamily() const
{
  return sa_family;
}

//
/// Sets the family of addressing this address belongs to.
//
inline void
TSocketAddress::SetFamily(ushort family)
{
  sa_family = family;
}

//
/// This function converts this address to the sockaddr_in structure.
//
inline
TINetSocketAddress::operator sockaddr_in() const
{
  return *((sockaddr_in*)this);
}

//
/// This function returns the network ID masked with the subnet.
//
inline ulong
TINetSocketAddress::GetNetwork(ulong subnet) const
{
  return GetNetworkAddress() & subnet;
}

//
/// This function returns the node of this address.
//
inline ulong
TINetSocketAddress::GetNode(ulong subnet) const
{
  return GetNetworkAddress() & ~subnet;
}

} // OWL namespace


#endif  // OWL_WSKADDR_H
