//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
// Macros and utility stuff for implementing COM support (i.e. IUnknown) in
// Bolero apps.
//----------------------------------------------------------------------------
#ifndef _BCOM_H
#define _BCOM_H

/*
 *                      Include COM definitions
 */

#include <string.h>                     // for fmemcmp

#ifndef _OBJBASE_H_
  #include <objbase.h>                    // include COM definitions
#endif
#ifndef _OLEAUTO_H_
  #include <oleauto.h>
#endif


#include "portdefs.h"           // temporarily. portdefs is in transition. (PHP)

#include "pshpack8.h"

#ifdef __BORLANDC__
    #undef NOERROR
    #define NOERROR ((HRESULT) 0L)
    #undef ResultFromScode(sc)
    inline HRESULT ResultFromScode(SCODE sc)
        {return (HRESULT) (sc & 0x800FFFFFL);}
    #undef GetScode(hr)
    inline SCODE GetScode(HRESULT hr)
        {return   ((SCODE)(hr) & 0x800FFFFFL); }
#endif

// force this and vtable pointers far on interfaces
#ifdef __BORLANDC__
    #define _ICLASS  __HUGE
#else
    #define _ICLASS  FAR
#endif

#define _IFUNC STDMETHODCALLTYPE

#ifndef NULLP
        #define NULLP 0L
#endif

#define PREDECLARE_INTERFACE( I )               \
  class _ICLASS I;                                                      \
  typedef const I FAR * PC##I;                  \
  typedef const I FAR & RC##I;                  \
  typedef I FAR * P##I;                                         \
  typedef I FAR & R##I;                                         \

typedef IUnknown FAR *  PIUnknown;
PREDECLARE_INTERFACE( IBUnknownMain )



/*
 *              Aggregation help while using multiple inheritance of interfaces.
 */

// IUnknownMain (IUnknown replica forces separate implementation in MI)
//
// QueryInterfaceMain handles IID discrimination and delegation
// to "inner" helper objects.
// AddRefMain and ReleaseMain update a reference count data member.
// And delete the object when the reference count reaches zero.
//
// All other IUnknown's (inherited by other interfaces) simply delegate to
// this IUnknownMain, (or that of an aggregator if one exists).
//
// The layout of IUnknownMain is identical to that of IUnknown and can be
// casted to IUnknown safely.  The names are different to prevent the compiler
// from overriding the implementation when multiple inheritance is used.
//      This is the best way to support aggregation and multiple inheritance
// of interfaces, and requires less indirection than the "nested class"
// approach used by Microsoft's OLE2 samples.
//


class _ICLASS IBUnknownMain
{
public:
    virtual HRESULT _IFUNC QueryInterfaceMain(REFIID, LPVOID FAR *) = 0;
    virtual ULONG   _IFUNC AddRefMain() = 0;
    virtual ULONG   _IFUNC ReleaseMain() = 0;
};



#define DEFINE_IUNKNOWN(pUnkOuter)                                              \
        HRESULT _IFUNC QueryInterface(REFIID iid, void FAR* FAR* pif)   \
                {return pUnkOuter->QueryInterfaceMain(iid, pif);}               \
        ULONG _IFUNC AddRef() {return pUnkOuter->AddRefMain();}                 \
        ULONG _IFUNC Release(){return pUnkOuter->ReleaseMain();}


/*
 *      Macros for defining inline QueryInterface implementation that can be
 * called statically from derived classes' QueryInterface
 *
 *      For each interface a global inline function with the interface name
 * prepended (followed by an underscore) is defined to allow
 * each interface to compare its own id and QueryInterface its parents'.
 * This simplifies writing QueryInterface.
 */

inline HRESULT _IFUNC IUnknown_QueryInterface(IUnknown FAR *, REFIID iid, LPVOID FAR * pif)
{
        // To avoid handing out different IUnknown pointers for
        // the same Windows Object don't match here.
        // Only match in the main IUnknown in the outermost aggregator.

        return ResultFromScode(E_NOINTERFACE);
}

#define DEFINE_INLINE_QI(I, P) \
inline HRESULT _IFUNC I##_QueryInterface(I FAR * This, REFIID iid, LPVOID FAR *pif) \
{                                                                                                                                                                       \
        return (iid == IID_##I) ?                                                                                               \
                (*pif = This), This->AddRef(), NOERROR :                                                                \
                P##_QueryInterface(This, iid, pif);                                                             \
}

#define DEFINE_INLINE_QI2(I, P1, P2) \
inline HRESULT _IFUNC I##_QueryInterface(I FAR* This, REFIID iid, LPVOID FAR* pif) \
{                                            \
  return (iid == IID_##I) ?                  \
    (*pif = This), ((P1*)This)->AddRef(), NOERROR : \
    SUCCEEDED(P1##_QueryInterface(This, iid, pif))? NOERROR :\
    P2##_QueryInterface(This, iid, pif);      \
}


inline HRESULT _IFUNC IBUnknownMain::QueryInterfaceMain(REFIID iid, LPVOID FAR *pif)
{
        return (iid == IID_IUnknown) ?
                (*pif = this), AddRefMain(), NOERROR :
                ResultFromScode(E_NOINTERFACE);
}

inline HRESULT _IFUNC IUnknown::QueryInterface(REFIID iid, LPVOID FAR *pif)
{
        return (iid == IID_IUnknown) ?
                (*pif = this), AddRef(), NOERROR :
                ResultFromScode(E_NOINTERFACE);
}

inline PIBUnknownMain AsPIUnknownMain(PIUnknown pUnk) {return (PIBUnknownMain)pUnk;}
inline PIUnknown AsPIUnknown(PIBUnknownMain pUnk) {return (PIUnknown)pUnk;}


#include "poppack.h"

#define  IDS_SAVEOBJ  32900

#endif
