//----------------------------------------------------------------------------
// Borland WinSys Library
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Base exception support for framework exceptions
//----------------------------------------------------------------------------

#if !defined(OWL_EXBASE_H)
#define OWL_EXBASE_H
#define WINSYS_EXBASE_H  //this file replaces old <winsys/exbase.h>

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/defs.h>
#include <owl/private/except.h>

namespace owl {

#include <owl/preclass.h>

//----------------------------------------------------------------------------
/// \addtogroup except
/// @{

//
/// Derived from xmsg, TXBase is the base class for ObjectWindows and
/// ObjectComponents exception-handling classes. The ObjectWindows classes that
/// handle specific kinds of exceptions, such as out-of-memory or invalid window
/// exceptions, are derived from TXOwl, which is in turn derived from TXBase. The
/// ObjectComponents classes TXOle and TXAuto are derived directly from TXBase.
///
/// TXBase contains the function Throw(), which is overridden in all
/// derived classes, as well as two constructors.
///
/// \note The function Clone() is deprecated, as it was needed only in 16-bit Windows
//
class _OWLCLASS TXBase : public std::exception {
  public:
    TXBase(const tstring& msg);
    TXBase(const TXBase& src);
    virtual ~TXBase();

    virtual TXBase* Clone();
    virtual void Throw();

    static void Raise(const tstring& msg);

  public:
    const char* what() const noexcept;
    TXBase& _RTLENTRY operator=(const TXBase &src);

    tstring why() const;

  protected:
    std::string str;
};

/// @}

#include <owl/posclass.h>

} // OWL namespace


#endif  // OWL_EXBASE_H
