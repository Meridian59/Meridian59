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
#include <owl/module.h>
#include <owl/winsock.h>
#include <owl/winsock.rh>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;

//
/// This function constructs the object with the error code and the size of the
/// buffer to allocate.
//
TSocketError::TSocketError(int error, unsigned sizeToAllocate)
:
  Error(error),
  SizeToAllocate(sizeToAllocate),
  String(0)
{
  GetErrorString();
}

//
/// This function destroys the allocated string.
//
TSocketError::~TSocketError()
{
  delete[] String;
}

//
/// Copies the error code and string.
//
TSocketError::TSocketError(const TSocketError& src)
:
  Error (src.Error),
  String (strnewdup(src.String)),
  SizeToAllocate (src.SizeToAllocate)
{
}

//
/// This function copies the error code and string.
//
TSocketError& TSocketError::operator =(const TSocketError& src)
{
  Error = src.Error;
  delete[] String;
  String = strnewdup(src.String);
  SizeToAllocate = src.SizeToAllocate;

  return *this;
}

//
/// This function tests for equality between two TSocketError objects. The important
/// criteria for determining equality is the error value. The string is unimportant.
//
bool operator ==(const TSocketError& socketError1,
                 const TSocketError& socketError2)
{
  return socketError1.Error == socketError2.Error;
}

//
/// This function initializes the error code.
//
void TSocketError::Init(int error)
{
  Error = error;
  GetErrorString();
}

//
/// This function returns the error code.
//
int TSocketError::GetReasonValue() const
{
  return Error;
}

//
/// This function hands the pointer to the string to the caller. The caller
/// shouldn't alter this string (because it doesn't belong to the caller).
//
const tchar* TSocketError::GetReasonString() const
{
  return String;
}

//
/// This function appends the error string to whatever is in the string
/// 'stringToAppendErrorTo' and put the result in 'destination'.  You may
/// want to put something specific about the error in the string and then use
/// AppendError() to add the Winsock error code and description to it.
///
/// For example, you could say:
/// \code
//   MessageBox(TSocketError(WSAENOTCONN).AppendError("Unable to send your mail"),
//                                                   "Error", MB_OK);
/// \endcode
/// And AppendError() will put "\n\nWinsock Error 10057: Socket is not presently connected"
/// after the "Unable to send you mail" string.  Quite convenient.
///
/// If destination is valid, destination provides the space for the result.
/// \note It is assumed that desination has enough space.
/// If destination is 0, our internal string buffer is used for the result.
///
/// \note Using the internal string changes the result of subsequent calls to GetReasonString.
/// Use Init to reset the error string.
//
tchar* TSocketError::AppendError(const tchar* stringToAppendErrorTo, tchar* destination)
{
  // Create the error string.
  //
  tstring s = stringToAppendErrorTo;
  s += _T("\n\n");
  s += GetReasonString();

  if (!destination) {
    //
    // Expand the internal string buffer if needed.
    //
    size_t n = s.length() + 1;
    if (n > SizeToAllocate) {
      delete [] String;
      String = new tchar[SizeToAllocate = n];
      }
    destination = String;
    }

  // Copy the string to the destination.

  _tcscpy(destination, s.c_str());
  return destination;
}

//
/// This function is similar to AppendError(char*, char*), but the pre-string comes
/// from a string resource and szStringToAppendErrorTo will be overwritten with what
/// is in the string resource and appended to the Windows Sockets Error description.
/// stringToAppendErrorTo must be able to hold at least 128 characters.
//
tchar* TSocketError::AppendError(int stringResourceToAppendErrorTo, tchar* destination)
{
  tstring s = GetModule().LoadString(stringResourceToAppendErrorTo);
  return AppendError (s.c_str(), destination);
}

//
/// This function gets a string, suitable for display, based on the nError value.
/// The previous string is deleted if necessary. Note that the string allocated must
/// be at least 128 characters long. Even though the error strings you see don't
/// have error numbers associated with them, the function prepends the error number
/// to the szString before returning.
///
/// If you are writing string resources for the error strings, don't put error
/// numbers in the string, because that will be done for you later.
//
void TSocketError::GetErrorString()
{
  int resIdLookup[] = {
    WSAEINTR,
    WSAEBADF,
    WSAEACCES,
    WSAEFAULT,
    WSAEINVAL,
    WSAEMFILE,
    WSAEWOULDBLOCK,
    WSAEINPROGRESS,
    WSAEALREADY,
    WSAENOTSOCK,
    WSAEDESTADDRREQ,
    WSAEMSGSIZE,
    WSAEPROTOTYPE,
    WSAENOPROTOOPT,
    WSAEPROTONOSUPPORT,
    WSAESOCKTNOSUPPORT,
    WSAEOPNOTSUPP,
    WSAEPFNOSUPPORT,
    WSAEAFNOSUPPORT,
    WSAEADDRINUSE,
    WSAEADDRNOTAVAIL,
    WSAENETDOWN,
    WSAENETUNREACH,
    WSAENETRESET,
    WSAECONNABORTED,
    WSAECONNRESET,
    WSAENOBUFS,
    WSAEISCONN,
    WSAENOTCONN,
    WSAESHUTDOWN,
    WSAETOOMANYREFS,
    WSAETIMEDOUT,
    WSAECONNREFUSED,
    WSAELOOP,
    WSAENAMETOOLONG,
    WSAEHOSTDOWN,
    WSAEHOSTUNREACH,
    WSAENOTEMPTY,
    WSAEPROCLIM,
    WSAEUSERS,
    WSAEDQUOT,
    WSAESTALE,
    WSAEREMOTE,
    WSAEDISCON,
    WSASYSNOTREADY,
    WSAVERNOTSUPPORTED,
    WSANOTINITIALISED,
    WSAHOST_NOT_FOUND,
    WSATRY_AGAIN,
    WSANO_RECOVERY,
    WSANO_DATA,
  };
  unsigned id = 0;
  while (id < COUNTOF(resIdLookup) && resIdLookup[id] != Error)
    id++;

  tstring s = GetModule().LoadString(IDS_WINSOCK_BASE+id+1);
  size_t n = s.length() + 26; // Add space for prefix "Winsock Error 123456789: ".
  if (SizeToAllocate < n) SizeToAllocate = n;
  delete[] String;
  String = new tchar[SizeToAllocate];
  String[0] = 0;
  
  _sntprintf(String, SizeToAllocate, _T("Winsock Error %d: %s"), Error, (tchar*) s.c_str());
}

//  
/// Return the current module.  Used when loading strings.  
/// Important when linking to OWL dynamically because ::LoadString
/// would try to load from the EXE, but the WinSock resource 
/// strings are in the DLL.
//
TModule& 
TSocketError::GetModule()         // this is a static function
{
  return GetGlobalModule();
}

} // OWL namespace

/* ========================================================================== */

