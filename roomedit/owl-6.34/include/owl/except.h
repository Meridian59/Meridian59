//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// ObjectWindows exception class & function definitions
//----------------------------------------------------------------------------

#if !defined(OWL_EXCEPT_H)
#define OWL_EXCEPT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/private/except.h>
#include <owl/exbase.h>
#include <owl/private/gmodule.h>
#include <owl/except.rh>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>


//
// Global function to handle exceptions in OWL as a last resort
//
_OWLFUNC(int) HandleGlobalException(owl::TXBase& x, LPCTSTR caption, LPCTSTR canResume=0);

/// \addtogroup except
/// @{
/// \class TXOwl
// ~~~~~ ~~~~~
/// TXOwl is root class of the ObjectWindows exception hierarchy.
//
class _OWLCLASS TXOwl : public TXBase {
  public:
    TXOwl(const tstring& msg, uint resId = 0);
    TXOwl(uint resId, TModule* module = &GetGlobalModule());
    virtual ~TXOwl() throw();

    // Override TXBase's virtuals
    //
    virtual TXOwl* Clone() const; // override
    void Throw();

    /// Per-exception class unhandled-handler, will default to the per-module
    /// unhandled-handler
    //
    virtual int Unhandled(TModule* appModule, uint promptResId);

    uint GetErrorCode() const;

    static tstring ResourceIdToString(bool* found, uint resId,
                                     TModule* module = &GetGlobalModule());
    static tstring MakeMessage(uint resId, const tstring& infoStr, TModule* module = &GetGlobalModule());
    static tstring MakeMessage(uint resId, LPCTSTR infoStr, TModule* module = &GetGlobalModule());
#if defined(UNICODE)
    static tstring MakeMessage(uint resId, LPCSTR infoStr, TModule* module = &GetGlobalModule());
#endif
    static tstring MakeMessage(uint resId, uint infoNum, TModule* module = &GetGlobalModule());

    static void Raise(const tstring& msg, uint resId = 0);
    static void Raise(uint resId, TModule* module = &GetGlobalModule());

  public:
/// Resource ID for a TXOwl object.
    uint ResId;
};

//
/// \class TXOutOfMemory
// ~~~~~ ~~~~~~~~~~~~~
/// Describes an exception that results from running out of memory.
//
class _OWLCLASS TXOutOfMemory : public TXOwl {
  public:
    TXOutOfMemory();

    virtual TXOutOfMemory* Clone() const; // override
    void   Throw();

    static void Raise();
};

//
/// \class TXNotSupportedCall
// ~~~~~ ~~~~~~~~~~~~~~~~~~
class _OWLCLASS TXNotSupportedCall : public TXOwl {
  public:
    TXNotSupportedCall();

    virtual TXNotSupportedCall* Clone() const; // override
    void   Throw();

    static void Raise();
};


//
/// \class TXEndSession
// ~~~~~ ~~~~~~~~~~~~
/// TXEndSession is thrown from TWindow's handler for WM_ENDSESSION.
/// Since it is not derived from TXOwl, it does not fall into the same
/// catch handler.
/// It is used exclusively for shutting down the application.
//

class _OWLCLASS_RTL TXEndSession {
  public:
    TXEndSession() {}
};



// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//
/// Returns the resource ID.
//
inline uint TXOwl::GetErrorCode() const {
  return ResId;
}

} // OWL namespace


#endif  // OWL_EXCEPT_H
