//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Winsock for OWL subsystem.
/// Based on work by Paul Pedriana, 70541.3223@compuserve.com
//----------------------------------------------------------------------------

#if !defined(OWL_WSKERR_H)
#define OWL_WSKERR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

//
/// \class TSocketError
// ~~~~~ ~~~~~~~~~~~~
/// TSocketError converts Windows Sockets errors to string messages.
//
class _OWLCLASS TSocketError {
  public:
    TSocketError(int error = 0, unsigned sizeToAllocate = 128);
    virtual ~TSocketError();

    TSocketError(const TSocketError& src);
    TSocketError& operator =(const TSocketError& src);
    friend bool operator ==(const TSocketError& socketError1,
                            const TSocketError& socketError2);

    void  Init(int error);
    int   GetReasonValue() const;
    const tchar* GetReasonString() const;

  // !CQ this seems like too much general policy. Maybe in TXOwl? Or remove.
    // Default to our own string (NULL argument).
    tchar* AppendError(const tchar* stringToAppendErrorTo, tchar* destination = 0);
    tchar* AppendError(int stringResourceToAppendErrorTo, tchar* destination = 0);

  protected:
    int   Error;            ///< Error code
    size_t SizeToAllocate;   ///< Size to allocate for String.
                            ///< Usually bigger than String so we can append to it.
    tchar* String;          ///< Error code converted to string
    void  GetErrorString(); ///< Convert the error code to string

  private:
    static TModule& GetModule();    ///< used for LoadString()
};

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_WSKERR_H
