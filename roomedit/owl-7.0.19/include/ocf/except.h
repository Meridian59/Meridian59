//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// OCF Exception classes
/// \note All OCF Exception classes are defined here except for TXAuto,
///           the exception used in automation failures. [See AUTODEFS.H
///           for TXAuto].
//----------------------------------------------------------------------------

#if !defined(OCF_EXCEPT_H)
#define OCF_EXCEPT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/defs.h>
#include <owl/exbase.h>

#include <ocf/defs.h>
#include <owl/private/ole2inc.h>


namespace ocf {

//
/// \class TXOle
// ~~~~~ ~~~~~
/// OLE API related exception object
//
class _OCFCLASS TXOle : public owl::TXBase {
  public:
    TXOle(const owl::tstring& msg, HRESULT stat);
    TXOle(const TXOle& src);

    ~TXOle();

    TXOle*  Clone();
    void    Throw();

    static void Check(HRESULT stat, LPCTSTR msg);
    static void Check(HRESULT stat);
    static void Throw(HRESULT stat, LPCTSTR msg);
    static void OleErrorFromCode(HRESULT stat, LPTSTR buffer, int size);

    const long  Stat;
};

//
//
//
#  if defined(BI_COMP_BORLANDC)
#    pragma warn -inl
#  endif
inline
TXOle::TXOle(const owl::tstring& msg, HRESULT stat)
      : owl::TXBase(msg), Stat((long)stat)
{}

//
//
//
inline TXOle::TXOle(const TXOle& src)
             : owl::TXBase(src), Stat(src.Stat)
{}
#  if defined(BI_COMP_BORLANDC)
#    pragma warn .inl
#  endif

//
// Macro to simply error checking of OLE calls
//
#if (__DEBUG > 0) || defined(__WARN)
# define OLECALL(func, msg) TXOle::Check(func, msg)
#else
# define OLECALL(func, msg) TXOle::Check(func)
#endif


//
/// \class TXObjComp
// ~~~~~ ~~~~~~~~~
/// Base OC exception class
//
#  if defined(BI_COMP_BORLANDC)
#    pragma warn -inl
#  endif
class _OCFCLASS TXObjComp : public TXOle {
  public:
    enum TError {

      // Application Errors
      //
      xNoError,
      xBOleLoadFail,
      xBOleVersFail,
      xBOleBindFail,
      xDocFactoryFail,

      // Doc & Part Errors
      //
      xMissingRootIStorage,
      xInternalPartError,
      xPartInitError,

      // Storage Errors
      //
      xRootStorageOpenError,
      xStorageOpenError,
      xStorageILockError,
      xStreamOpenError,
    };

    TXObjComp(TError err, const owl::tstring& msg, HRESULT stat = HR_FAIL)
      : TXOle(msg, stat), ErrorCode(err) {}
    TXObjComp(const TXObjComp& src)
      : TXOle(src), ErrorCode(src.ErrorCode) {}
     ~TXObjComp();

    TXObjComp*  Clone();
    void        Throw();

    static void Check(HRESULT stat, TError err, LPCTSTR msg = 0);
    static void Throw(TError err, HRESULT stat = HR_FAIL, LPCTSTR msg = 0);

    const TError ErrorCode;
};
#  if defined(BI_COMP_BORLANDC)
#    pragma warn .inl
#  endif

} // OCF namespace

#endif  //  OCF_EXCEPT_H
