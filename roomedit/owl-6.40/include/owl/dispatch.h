//
/// \file
/// Dispatch functions (crackers) to crack a Windows message and pass control
/// to a member function via a pointer (Handler).
//
// Part of OWLNext - the next generation Object Windows Library 
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
// Copyright (c) 2013 Vidar Hasfjord 
//
// For more information, including license details, see 
// http://owlnext.sourceforge.net
//

#if !defined(OWL_DISPATCH_H)
#define OWL_DISPATCH_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include "windowsx.h"
#include <dbt.h> // We need PDEV_BROADCAST_HDR for the WM_DEVICECHANGE message.

#if defined(BI_COMP_MSC)
# pragma pointers_to_members(full_generality, virtual_inheritance)
#endif

namespace owl {

//
// Forward declared types used for pointers and references in the dispatchers
//

class _OWLCLASS TCommandEnabler;
class _OWLCLASS TDocument;
class _OWLCLASS TView;
class _OWLCLASS TDockingSlip;
class _OWLCLASS TWindow;
class _OWLCLASS TPoint;
class _OWLCLASS TDropInfo;
class _OWLCLASS TSize;
class _OWLCLASS TRect;
struct _OWLCLASS THelpHitInfo;

//
/// \name Aliases for Windows message parameters
/// @{
//
typedef LRESULT TResult; ///< Result type
typedef UINT TMsgId; ///< Message ID type
typedef WPARAM TParam1; ///< First parameter type
typedef LPARAM TParam2; ///< Second parameter type

/// @}

//
/// \name Parameter factories
/// @{
//

template <class T1, class T2>
inline TParam2 MkParam2(const T1& lo, const T2& hi) {
  return (uint32(hi) << 16) | uint16(lo);
}
inline TParam1 MkParam1(uint lo, uint hi) {
  return (uint32(hi) << 16) | uint16(lo);
}

/// @}

#if !defined(OWL5_COMPAT)
#define OWL_NEW_RESPONSE_TABLE 1
#endif

#if OWL_NEW_RESPONSE_TABLE // New response table implementation:

typedef void TGeneric; // For backward compatibility.

//
/// The general signature of a dispatcher function
/// Takes a pointer to an object as well as the raw message arguments.
/// The dispatcher should crack the message arguments and forward the message to the given object.
/// This means that the dispatcher needs to have static knowledge of the particular message 
/// handled, the type of object passed, and the particular member function to forward to.
//
typedef _OWLFUNC(TResult) (*TDispatchFunction)(void* i, TParam1, TParam2);

//
// Simple dispatch using the new implementation of the machinery
//
#define OWL_DISPATCH(dispatch, method)\
  &dispatch<TMyClass, &TMyClass::method > // Space needed to avoid ">>" if method is a template instantiation.

#else // Old response table implementation:

//
// Generic class for casting pointer to objects and pointer to member functions
// Class is not actually defined or implemented anywhere
//
class TGeneric;

//
// Generic pointer to member function
//
typedef void (TGeneric::*TAnyPMF)();

//
// All message dispatcher functions take four parameters:
//
// - reference to an object
// - pointer to member function (signature varies according to the cracking
//   that the function performs)
// - \c \b  wParam
// - \c \b  lParam
//
typedef _OWLFUNC(TResult) (*TAnyDispatcher)(TGeneric&, TAnyPMF, TParam1, TParam2);

//
// This template function is used as an adaptor to the new dispatchers.
// It has the old TAnyDispatcher signature, so can be plugged into the old machinery.
// It simply drops the unused TAnyPMF argument and forwards the call to the given dispatcher.
// If the machinery is modernised in the future, the use of this function can be removed.
//
template <TResult (*F)(void*, TParam1, TParam2)>
TResult AnyDispatch(TGeneric& obj, TAnyPMF, TParam1 p1, TParam2 p2)
{
	if ((void*)F) return F(&obj, p1, p2);
	else return 0;
}

#define OWL_DISPATCH(dispatch, method)\
  &owl::AnyDispatch<&dispatch<TMyClass, &TMyClass::method > > // Space needed to avoid ">>" if method is a template instantiation.

#endif

//-------------------------------------------------------------------------------------------------
// Utility functions

//
/// Casts the given void-pointer to a pointer to the specified type.
/// Used to circumvent a bug in the BCC32 compiler, where a simple static_cast, in certain 
/// template contexts (e.g. DispatchRawArguments), will produce a compilation error.
//
template <class T>
T* ConvertVoidPtr(void* v)
{return static_cast<T*>(v);}

//-------------------------------------------------------------------------------------------------
// New general dispatchers

template <class T, TResult (T::*M)(TParam1, TParam2)>
TResult DispatchRawArguments(void* i, TParam1 p1, TParam2 p2)
{return (ConvertVoidPtr<T>(i)->*M)(p1, p2);}

template <class T, void (T::*M)()>
TResult DispatchChildNotify(void* i, TParam1, TParam2)
{return (ConvertVoidPtr<T>(i)->*M)(), 0;}

template <class T, void (T::*M)(uint)>
TResult DispatchChildNotifyWithCode(void* i, TParam1 p1, TParam2)
{return (ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(p1)), 0;}

template <class T, void (T::*M)()>
TResult DispatchCommand(void* i, TParam1, TParam2)
{return (ConvertVoidPtr<T>(i)->*M)(), 0;}

template <class T, void (T::*M)(uint)>
TResult DispatchCommandWithId(void* i, TParam1 p1, TParam2)
{return (ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(p1)), 0;}

//-------------------------------------------------------------------------------------------------
// Windows message-specific dispatchers

//
/// Undefined default template for dispatchers
/// Template specialization is used to allow the compiler to look up the dispatcher for a 
/// particular message at compile-time.
//
template <uint MessageID>
struct TDispatch;

template <> struct TDispatch<WM_ACTIVATE>
{
  template <class T, void (T::*M)(uint, bool, HWND)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, bool, HWND);
      THandler m;
      void operator ()(HWND, UINT active, HWND other, BOOL minimized)
      {(i_->*m)(active, minimized, other);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_ACTIVATE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_ACTIVATEAPP>
{
  template <class T, void (T::*M)(bool, DWORD)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(bool, DWORD);
      THandler m;
      void operator ()(HWND, BOOL active, DWORD threadId)
      {(i_->*m)(active, threadId);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_ACTIVATEAPP(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_ASKCBFORMATNAME>
{
  template <class T, void (T::*M)(uint bufferSize, LPTSTR formatNameBuffer)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, LPTSTR);
      THandler m;
      void operator ()(HWND, int cchMax, LPTSTR rgchName)
      {(i_->*m)(static_cast<unsigned int>(cchMax), rgchName);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_ASKCBFORMATNAME(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CANCELMODE>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CANCELMODE(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_CAPTURECHANGED is missing from "windowsx.h".
//
template <> struct TDispatch<WM_CAPTURECHANGED>
{
  template <class T, void (T::*M)(HWND capture)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {return (ConvertVoidPtr<T>(i)->*M)(reinterpret_cast<HWND>(p2)), 0;}
};

template <> struct TDispatch<WM_CHANGECBCHAIN>
{
  template <class T, void (T::*M)(HWND removed, HWND next)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HWND, HWND);
      THandler m;
      void operator ()(HWND, HWND hwndRemove, HWND hwndNext)
      {(i_->*m)(hwndRemove, hwndNext);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CHANGECBCHAIN(0, p1, p2, forwarder);
  }
};

//
// TODO: Remove the useless 'flags' parameter, change 'repeatCount' to int, and change the 'key' parameter to 'ch'. 
//
template <> struct TDispatch<WM_CHAR>
{
  template <class T, void (T::*M)(uint ch, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, uint, uint);
      THandler m;
      void operator ()(HWND, TCHAR ch, int cRepeat)
      {(i_->*m)(static_cast<unsigned int>(ch), static_cast<unsigned int>(cRepeat), 0);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CHAR(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CHARTOITEM>
{
  template <class T, int (T::*M)(uint ch, HWND listBox, uint caretIndex)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef int (T::*THandler)(uint, HWND, uint);
      THandler m;
      int operator ()(HWND, UINT ch, HWND hwndListbox, int iCaret)
      {return (i_->*m)(ch, hwndListbox, static_cast<unsigned int>(iCaret));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CHARTOITEM(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CHILDACTIVATE>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CHILDACTIVATE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CLOSE>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CLOSE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_COMPACTING>
{
  template <class T, void (T::*M)(uint compactRatio)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint);
      THandler m;
      void operator ()(HWND, UINT compactRatio)
      {(i_->*m)(compactRatio);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_COMPACTING(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_COMPAREITEM>
{
  template <class T, int (T::*M)(uint, const COMPAREITEMSTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef int (T::*THandler)(uint, const COMPAREITEMSTRUCT&);
      THandler m;
      int operator ()(HWND, const COMPAREITEMSTRUCT* c)
      {
        PRECONDITION(c);
        return c ? (i_->*m)(c->CtlID, *c) : 0;
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_COMPAREITEM(0, p1, p2, forwarder);
  }
};

//
// TODO: Coordinate parameters should be replaced by TPoint.
//
template <> struct TDispatch<WM_CONTEXTMENU>
{
  template <class T, void (T::*M)(HWND, int, int)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HWND, int, int);
      THandler m;
      void operator ()(HWND, HWND w, int x, int y)
      {(i_->*m)(w, x, y);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CONTEXTMENU(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_COPYDATA has a bug; it treats the handler as returning nothing (void), and the macro
// hence always returns 0. We therefore implement this specialization manually.
//
template <> struct TDispatch<WM_COPYDATA>
{
  template <class T, bool (T::*M)(HWND sender, const COPYDATASTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    PRECONDITION(p2 != 0);
    return (p2 != 0 && (ConvertVoidPtr<T>(i)->*M)(reinterpret_cast<HWND>(p1), *reinterpret_cast<PCOPYDATASTRUCT>(p2))) ? TRUE : FALSE;
  }
};

template <> struct TDispatch<WM_CREATE>
{
  template <class T, bool (T::*M)(CREATESTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef bool (T::*THandler)(CREATESTRUCT&);
      THandler m;
      BOOL operator ()(HWND, LPCREATESTRUCT c)
      {
        PRECONDITION(c);
        return (c && (i_->*m)(*c)) ? TRUE : FALSE;
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CREATE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CTLCOLORBTN>
{
  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef HBRUSH (T::*THandler)(HDC, HWND, uint);
      THandler m;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*m)(d, w, ctlType);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CTLCOLORBTN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CTLCOLOREDIT>
{
  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef HBRUSH (T::*THandler)(HDC, HWND, uint);
      THandler m;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*m)(d, w, ctlType);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CTLCOLOREDIT(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CTLCOLORDLG>
{
  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef HBRUSH (T::*THandler)(HDC, HWND, uint);
      THandler m;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*m)(d, w, ctlType);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CTLCOLORDLG(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CTLCOLORLISTBOX>
{
  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef HBRUSH (T::*THandler)(HDC, HWND, uint);
      THandler m;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*m)(d, w, ctlType);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CTLCOLORLISTBOX(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CTLCOLORMSGBOX>
{
  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef HBRUSH (T::*THandler)(HDC, HWND, uint);
      THandler m;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*m)(d, w, ctlType);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CTLCOLORMSGBOX(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CTLCOLORSCROLLBAR>
{
  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef HBRUSH (T::*THandler)(HDC, HWND, uint);
      THandler m;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*m)(d, w, ctlType);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CTLCOLORSCROLLBAR(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CTLCOLORSTATIC>
{
  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef HBRUSH (T::*THandler)(HDC, HWND, uint);
      THandler m;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*m)(d, w, ctlType);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CTLCOLORSTATIC(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CUT>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_CUT(0, p1, p2, forwarder);
  }
};

//
// TODO: Remove the useless 'flags' parameter, change 'repeatCount' to int, and change the 'deadKey' parameter to 'ch'. 
//
template <> struct TDispatch<WM_DEADCHAR>
{
  template <class T, void (T::*M)(uint deadKey, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, uint, uint);
      THandler m;
      void operator ()(HWND, TCHAR ch, int cRepeat)
      {(i_->*m)(static_cast<unsigned int>(ch), static_cast<unsigned int>(cRepeat), 0);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_DEADCHAR(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DELETEITEM>
{
  template <class T, void (T::*M)(uint, const DELETEITEMSTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const DELETEITEMSTRUCT&);
      THandler m;
      void operator ()(HWND, const DELETEITEMSTRUCT* d)
      {
        PRECONDITION(d);
        if (d) (i_->*m)(d->CtlID, *d);
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_DELETEITEM(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DESTROY>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_DESTROY(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DESTROYCLIPBOARD>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_DESTROYCLIPBOARD(0, p1, p2, forwarder);
  }
};

//
// Note that the OWLNext 6.32 handler signature was non-compliant (bool (T::*)(uint, uint)).
// For this reason we do not provide a compatibility implementation here.
// Also note that the HANDLE_WM_DEVICECHANGE macro in "windowsx.h" has a bug.
// The event data pointer passed in the LPARAM argument is truncated to DWORD.
// Also, the result type should allow for the BROADCAST_QUERY_DENY return value specified for some 
// of the events. We use TResult here to signal that the return value type is the union of possible 
// return value types from all the current (and possible future) events. See the Windows API 
// documentation for details.
//
template <> struct TDispatch<WM_DEVICECHANGE>
{
  template <class T, TResult (T::*M)(uint event, PDEV_BROADCAST_HDR eventData)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(p1), reinterpret_cast<PDEV_BROADCAST_HDR>(pEventData));}
};

template <> struct TDispatch<WM_DEVMODECHANGE>
{
  template <class T, void (T::*M)(LPCTSTR deviceName)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(LPCTSTR);
      THandler m;
      void operator ()(HWND, LPCTSTR lpszDeviceName)
      {(i_->*m)(lpszDeviceName);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_DEVMODECHANGE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DISPLAYCHANGE>
{
  template <class T, void (T::*M)(uint bitsPerPixel, uint screenResolutionWidth, uint screenResolutionHeight)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, uint, uint);
      THandler m;
      void operator ()(HWND, UINT bitsPerPixel, UINT cxScreen, UINT cyScreen)
      {(i_->*m)(bitsPerPixel, cxScreen, cyScreen);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_DISPLAYCHANGE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DRAWCLIPBOARD>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_DRAWCLIPBOARD(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DRAWITEM>
{
  template <class T, void (T::*M)(uint, const DRAWITEMSTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const DRAWITEMSTRUCT&);
      THandler m;
      void operator ()(HWND, const DRAWITEMSTRUCT* d)
      {
        PRECONDITION(d);
        if (d) (i_->*m)(d->CtlID, *d);
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_DRAWITEM(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DROPFILES>
{
  template <class T, void (T::*M)(TDropInfo)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(TDropInfo);
      THandler m;
      void operator ()(HWND, HDROP hdrop)
      {(i_->*m)(::owl::TDropInfo(hdrop));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_DROPFILES(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_ENABLE>
{
  template <class T, void (T::*M)(bool)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(bool);
      THandler m;
      void operator ()(HWND, BOOL fEnable)
      {(i_->*m)(fEnable);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_ENABLE(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_ENDSESSION does not decode and pass the flags.
//
template <> struct TDispatch<WM_ENDSESSION>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, bool endSession, uint flags)
  {sendMessage(wnd, WM_ENDSESSION, endSession ? TRUE : FALSE, static_cast<TParam2>(flags));}

  template <class T, void (T::*M)(bool endSession, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (ConvertVoidPtr<T>(i)->*M)(static_cast<bool>(p1), static_cast<uint>(p2)), 0;}
};

template <> struct TDispatch<WM_ENTERIDLE>
{
  template <class T, void (T::*M)(uint source, HWND)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, HWND);
      THandler m;
      void operator ()(HWND, UINT source, HWND hWndDlg)
      {(i_->*m)(source, hWndDlg);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_ENTERIDLE(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_ENTERMENULOOP is missing from "windowsx.h".
//
template <> struct TDispatch<WM_ENTERMENULOOP>
{
  template <class T, void (T::*M)(bool isTrackPopupMenu)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (ConvertVoidPtr<T>(i)->*M)(static_cast<BOOL>(p1) == TRUE), 0;}
};

template <> struct TDispatch<WM_ENTERSIZEMOVE>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_ENTERSIZEMOVE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_ERASEBKGND>
{
  template <class T, bool (T::*M)(HDC)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef bool (T::*THandler)(HDC);
      THandler m;
      BOOL operator ()(HWND, HDC h)
      {return (i_->*m)(h) ? TRUE : FALSE;}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_ERASEBKGND(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_EXITMENULOOP is missing from "windowsx.h".
//
template <> struct TDispatch<WM_EXITMENULOOP>
{
  template <class T, void (T::*M)(bool isTrackPopupMenu)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (ConvertVoidPtr<T>(i)->*M)(static_cast<BOOL>(p1) == TRUE), 0;}
};

//
// HANDLE_WM_EXITSIZEMOVE is missing from "windowsx.h".
//
template <> struct TDispatch<WM_EXITSIZEMOVE>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (ConvertVoidPtr<T>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_FONTCHANGE>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_FONTCHANGE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_GETDLGCODE>
{
  template <class T, uint (T::*M)(const MSG*)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef uint (T::*THandler)(const MSG*);
      THandler m;
      uint operator ()(HWND, LPMSG msg)
      {return (i_->*m)(msg);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_GETDLGCODE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_GETFONT>
{
  template <class T, HFONT (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef HFONT (T::*THandler)();
      THandler m;
      HFONT operator ()(HWND)
      {return (i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_GETFONT(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_GETICON>
{
  template <class T, HICON (T::*M)(uint iconType)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef HICON (T::*THandler)(uint);
      THandler m;
      HICON operator ()(HWND, UINT iconType)
      {return (i_->*m)(iconType);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_GETICON(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_GETMINMAXINFO>
{
  template <class T, void (T::*M)(MINMAXINFO&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(MINMAXINFO&);
      THandler m;
      void operator ()(HWND, LPMINMAXINFO lpMinMaxInfo)
      {
        PRECONDITION(lpMinMaxInfo);
        if (lpMinMaxInfo) (i_->*m)(*lpMinMaxInfo);
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_GETMINMAXINFO(0, p1, p2, forwarder);
  }
};

#if defined(OWL5_COMPAT)

// 
// The old "void EvGetText(uint, LPTSTR)" signature is non-compliant. 
// The handler must return the number of characters copied, not including the terminating 
// null-character. Here we assume that the WM_GETTEXTLENGHT message sent to the same window
// will return that correct number, except when the buffer is too small, in which case we assume 
// that the buffer was filled to capacity (truncated), including a terminating null-character.
//
template <> struct TDispatch<WM_GETTEXT>
{
  template<class T, void (T::*M)(uint textBufferSize, LPTSTR textBuffer)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    int textBufferSize = static_cast<int>(p1);
    CHECK(textBufferSize > 0);
    (ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(textBufferSize), reinterpret_cast<LPTSTR>(p2));
    int length = static_cast<int>(ConvertVoidPtr<T>(i)->HandleMessage(WM_GETTEXTLENGTH));
    CHECK(length >= 0);
    return static_cast<TResult>(length >= textBufferSize ? textBufferSize - 1 : length);
  }
};

#else

template <> struct TDispatch<WM_GETTEXT>
{
  template <class T, int (T::*M)(int textBufferSize, LPTSTR textBuffer)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef int (T::*THandler)(int, LPTSTR);
      THandler m;
      int operator ()(HWND, int cchTextMax, LPTSTR lpszText)
      {return (i_->*m)(cchTextMax, lpszText);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_GETTEXT(0, p1, p2, forwarder);
  }
};

#endif

#if defined(OWL5_COMPAT)

//
// The old EvGetTextLength returns uint.
//
template <> struct TDispatch<WM_GETTEXTLENGTH>
{
  template <class T, uint (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef uint (T::*THandler)();
      THandler m;
      int operator ()(HWND)
      {return static_cast<int>((i_->*m)());}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_GETTEXTLENGTH(0, p1, p2, forwarder);
  }
};

#else

template <> struct TDispatch<WM_GETTEXTLENGTH>
{
  template <class T, int (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef int (T::*THandler)();
      THandler m;
      int operator ()(HWND)
      {return (i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_GETTEXTLENGTH(0, p1, p2, forwarder);
  }
};

#endif

//
// HANDLE_WM_HELP is missing from "windowsx.h".
//
template <> struct TDispatch<WM_HELP>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, const HELPINFO& i)
  {
    HELPINFO iCopy = i;
    sendMessage(wnd, WM_HELP, 0, reinterpret_cast<TParam2>(&iCopy));
  }

  template <class T, void (T::*M)(const HELPINFO&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 != 0 ? ((ConvertVoidPtr<T>(i)->*M)(*reinterpret_cast<HELPINFO*>(p2)), 0) : 0;
  }
};

#if defined(OWL5_COMPAT)

template <> struct TDispatch<WM_HOTKEY>
{
  template <class T, void (T::*M)(int idHotKey)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(int);
      THandler m;
      void operator ()(HWND, int idHotKey, UINT /*fuModifiers*/, UINT /*vk*/)
      {(i_->*m)(idHotKey);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_HOTKEY(0, p1, p2, forwarder);
  }
};

#else

template <> struct TDispatch<WM_HOTKEY>
{
  template <class T, void (T::*M)(int idHotKey, uint modifiers, uint vk)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(int, uint, uint);
      THandler m;
      void operator ()(HWND, int idHotKey, UINT fuModifiers, UINT vk)
      {(i_->*m)(idHotKey, fuModifiers, vk);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_HOTKEY(0, p1, p2, forwarder);
  }
};

#endif

template <> struct TDispatch<WM_HSCROLL>
{
  template <class T, void (T::*M)(uint scrollCode, uint thumbPos, HWND ctrl)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, uint, HWND);
      THandler m;
      void operator ()(HWND, HWND hwndCtl, UINT code, int pos)
      {(i_->*m)(code, static_cast<unsigned int>(pos), hwndCtl);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_HSCROLL(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_HSCROLLCLIPBOARD>
{
  template <class T, void (T::*M)(HWND cliboardViewer, uint scrollCode, uint thumbPos)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HWND, uint, uint);
      THandler m;
      void operator ()(HWND, HWND hwndCBViewer, UINT code, int pos)
      {(i_->*m)(hwndCBViewer, code, static_cast<unsigned int>(pos));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_HSCROLLCLIPBOARD(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_INITMENU>
{
  template <class T, void (T::*M)(HMENU)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HMENU);
      THandler m;
      void operator ()(HWND, HMENU hMenu)
      {(i_->*m)(hMenu);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_INITMENU(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_INITMENUPOPUP>
{
  template <class T, void (T::*M)(HMENU, uint index, bool isSysMenu)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HMENU, uint, bool);
      THandler m;
      void operator ()(HWND, HMENU hMenu, UINT item, BOOL fSystemMenu)
      {(i_->*m)(hMenu, item, fSystemMenu);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_INITMENUPOPUP(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_IMPUTLANGCHANGE is missing from "windowsx.h".
//
template <> struct TDispatch<WM_INPUTLANGCHANGE>
{
  template <class T, void (T::*M)(uint charSet, uint localeId)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(p1), static_cast<uint>(p2)), 0;}
};

//
// HANDLE_WM_IMPUTLANGCHANGEREQUEST is missing from "windowsx.h".
//
template <> struct TDispatch<WM_INPUTLANGCHANGEREQUEST>
{
  template <class T, void (T::*M)(uint flags, uint localeId)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(p1), static_cast<uint>(p2)), 0;}
};

template <> struct TDispatch<WM_KEYDOWN>
{
  template <class T, void (T::*M)(uint key, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, uint, uint);
      THandler m;
      void operator ()(HWND, UINT vk, BOOL, int cRepeat, UINT flags)
      {(i_->*m)(vk, static_cast<unsigned int>(cRepeat), flags);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_KEYDOWN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_KEYUP>
{
  template <class T, void (T::*M)(uint key, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, uint, uint);
      THandler m;
      void operator ()(HWND, UINT vk, BOOL, int cRepeat, UINT flags)
      {(i_->*m)(vk, static_cast<unsigned int>(cRepeat), flags);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_KEYUP(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_KILLFOCUS>
{
  template <class T, void (T::*M)(HWND)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HWND);
      THandler m;
      void operator ()(HWND, HWND hwndNewFocus)
      {(i_->*m)(hwndNewFocus);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_KILLFOCUS(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_LBUTTONDBLCLK>
{
  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, BOOL, int x, int y, UINT modKeys)
      {(i_->*m)(modKeys, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_LBUTTONDBLCLK(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_LBUTTONDOWN>
{
  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, BOOL, int x, int y, UINT modKeys)
      {(i_->*m)(modKeys, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_LBUTTONDOWN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_LBUTTONUP>
{
  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, int x, int y, UINT modKeys)
      {(i_->*m)(modKeys, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_LBUTTONUP(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MBUTTONDBLCLK>
{
  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, BOOL, int x, int y, UINT modKeys)
      {(i_->*m)(modKeys, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_MBUTTONDBLCLK(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MBUTTONDOWN>
{
  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, BOOL, int x, int y, UINT modKeys)
      {(i_->*m)(modKeys, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_MBUTTONDOWN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MBUTTONUP>
{
  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, int x, int y, UINT modKeys)
      {(i_->*m)(modKeys, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_MBUTTONUP(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MDIACTIVATE>
{
  template <class T, void (T::*M)(HWND activated, HWND deactivated)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HWND, HWND);
      THandler m;
      void operator ()(HWND, BOOL, HWND activated, HWND deactivated)
      {(i_->*m)(activated, deactivated);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_MDIACTIVATE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MDICASCADE>
{
  template <class T, bool (T::*M)(uint cmd)>
  static TResult Decode(T* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef bool (T::*THandler)(uint);
      THandler m;
      BOOL operator ()(HWND, UINT cmd)
      {return (i_->*m)(cmd) ? TRUE : FALSE;}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_MDICASCADE(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_MDICREATE casts the HWND result value through UINT. Seems like a bug, so manually crack this one.
//
template <> struct TDispatch<WM_MDICREATE>
{
  template <class T, HWND (T::*M)(MDICREATESTRUCT&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 != 0 ? reinterpret_cast<TResult>((ConvertVoidPtr<T>(i)->*M)(*reinterpret_cast<LPMDICREATESTRUCT>(p2))) : 0;
  }
}; 

template <> struct TDispatch<WM_MDIDESTROY>
{
  template <class T, void (T::*M)(HWND)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HWND);
      THandler m;
      void operator ()(HWND, HWND hwndDestroy)
      {(i_->*m)(hwndDestroy);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_MDIDESTROY(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MEASUREITEM>
{
  template <class T, void (T::*M)(uint, MEASUREITEMSTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, MEASUREITEMSTRUCT&);
      THandler m;
      void operator ()(HWND, MEASUREITEMSTRUCT* ms)
      {
        PRECONDITION(m);
        if (ms) (i_->*m)(ms->CtlID, *ms);
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_MEASUREITEM(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MENUCHAR>
{
  template <class T, int32 (T::*M)(uint, uint, HMENU)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef int32 (T::*THandler)(uint, uint, HMENU);
      THandler m;
      int32 operator ()(HWND, UINT nChar, UINT menuType, HMENU hmenu)
      {return (i_->*m)(nChar, menuType, hmenu);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_MENUCHAR(0, p1, p2, forwarder);
  }
};

//
// Note that we do not use the HANDLE_WM_MENUSELECT macro here.
// The handler signature demanded by the macro is incompatible with OWL 5.
// It also does more than just decoding the arguments; it creates new ones by calling ::GetSubMenu 
// for popup menus, and in this case, discards information (the menu item index of the popup) on
// which existing code depends (e.g. hints for menu items; see TDecoratedFrame::EvMenuSelect).
// For these reasons we use the OWL 5 signature and perform manual decoding.
// See the Windows API documentation.
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms646352.aspx
//
template <> struct TDispatch<WM_MENUSELECT>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, uint item, uint flags, HMENU hMenu)
  {sendMessage(wnd, WM_MENUSELECT, MkParam1(item, flags), reinterpret_cast<TParam2>(hMenu));}

  template <class T, void (T::*M)(uint item, uint flags, HMENU)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (ConvertVoidPtr<T>(i)->*M)(LOWORD(p1), HIWORD(p1), reinterpret_cast<HMENU>(p2)), 0;}
};

template <> struct TDispatch<WM_MOUSEACTIVATE>
{
  template <class T, uint (T::*M)(HWND topLevel, uint codeHitTest, TMsgId)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef uint (T::*THandler)(HWND, uint, TMsgId);
      THandler m;
      uint operator ()(HWND, HWND hwndTopLevel, UINT codeHitTest, UINT msg)
      {return (i_->*m)(hwndTopLevel, codeHitTest, msg);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_MOUSEACTIVATE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MOUSEHOVER>
{
  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    TPoint p(GET_X_LPARAM(p2), GET_Y_LPARAM(p2));
    return (ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(p1), p), 0;
  }
};

template <> struct TDispatch<WM_MOUSELEAVE>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (ConvertVoidPtr<T>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_MOUSEMOVE>
{
  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, int x, int y, UINT modKeys)
      {(i_->*m)(modKeys, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_MOUSEMOVE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MOUSEWHEEL>
{
  template <class T, bool (T::*M)(uint modKeys, int zDelta, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef bool (T::*THandler)(uint, int, const TPoint&);
      THandler m;
      BOOL operator ()(HWND, int x, int y, int zDelta, UINT modKeys)
      {return (i_->*m)(modKeys, zDelta, ::owl::TPoint(x, y)) ? TRUE : FALSE;}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_MOUSEWHEEL(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MOVE>
{
  template <class T, void (T::*M)(const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(const TPoint&);
      THandler m;
      void operator ()(HWND hwnd, int x, int y)
      {(i_->*m)(::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_MOVE(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_MOVING is missing from "windowsx.h".
//
template <> struct TDispatch<WM_MOVING>
{
  template <class T, void (T::*M)(TRect&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    LPRECT rect = reinterpret_cast<LPRECT>(p2);
    CHECK(rect);
    if (!rect) return FALSE; // Not processed.
    TRect clone(*rect);
    (ConvertVoidPtr<T>(i)->*M)(clone);
    *rect = clone;
    return TRUE; // Processed.
  }
};

template <> struct TDispatch<WM_NCACTIVATE>
{
  template <class T, bool (T::*M)(bool)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef bool (T::*THandler)(bool);
      THandler m;
      BOOL operator ()(HWND, BOOL fActive, HWND, BOOL)
      {return (i_->*m)(fActive) ? TRUE : FALSE;}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCACTIVATE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCCALCSIZE>
{
  template <class T, uint (T::*M)(bool calcValidRects, NCCALCSIZE_PARAMS&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef uint (T::*THandler)(bool, NCCALCSIZE_PARAMS&);
      THandler m;
      uint operator ()(HWND, BOOL fCalcValidRects, NCCALCSIZE_PARAMS* lpcsp)
      {
        PRECONDITION(lpcsp);
        return lpcsp ? (i_->*m)(fCalcValidRects, *lpcsp) : 0;
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCCALCSIZE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCCREATE>
{
  template <class T, bool (T::*M)(CREATESTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef bool (T::*THandler)(CREATESTRUCT&);
      THandler m;
      BOOL operator ()(HWND, LPCREATESTRUCT lpCreateStruct)
      {
        PRECONDITION(lpCreateStruct);
        return (lpCreateStruct && (i_->*m)(*lpCreateStruct)) ? TRUE : FALSE;
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCCREATE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCDESTROY>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCDESTROY(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCHITTEST>
{
  template <class T, uint (T::*M)(const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef uint (T::*THandler)(const TPoint&);
      THandler m;
      uint operator ()(HWND, int x, int y)
      {return (i_->*m)(::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCHITTEST(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCLBUTTONDBLCLK>
{
  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, BOOL, int x, int y, UINT codeHitTest)
      {(i_->*m)(codeHitTest, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCLBUTTONDBLCLK(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCLBUTTONDOWN>
{
  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, BOOL, int x, int y, UINT codeHitTest)
      {(i_->*m)(codeHitTest, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCLBUTTONDOWN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCLBUTTONUP>
{
  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, int x, int y, UINT codeHitTest)
      {(i_->*m)(codeHitTest, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCLBUTTONUP(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCMBUTTONDBLCLK>
{
  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, BOOL, int x, int y, UINT codeHitTest)
      {(i_->*m)(codeHitTest, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCMBUTTONDBLCLK(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCMBUTTONDOWN>
{
  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, BOOL, int x, int y, UINT codeHitTest)
      {(i_->*m)(codeHitTest, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCMBUTTONDOWN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCMBUTTONUP>
{
  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, int x, int y, UINT codeHitTest)
      {(i_->*m)(codeHitTest, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCMBUTTONUP(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_NCMOUSEHOVER is missing from "windowsx.h".
//
template <> struct TDispatch<WM_NCMOUSEHOVER>
{
  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    TPoint p(GET_X_LPARAM(p2), GET_Y_LPARAM(p2));
    return (ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(p1), p), 0;
  }
};

//
// HANDLE_WM_NCMOUSELEAVE is missing from "windowsx.h".
//
template <> struct TDispatch<WM_NCMOUSELEAVE>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (ConvertVoidPtr<T>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_NCMOUSEMOVE>
{
  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, int x, int y, UINT codeHitTest)
      {(i_->*m)(codeHitTest, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCMOUSEMOVE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCPAINT>
{
  template <class T, void (T::*M)(HRGN)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HRGN);
      THandler m;
      void operator ()(HWND, HRGN hrgn)
      {(i_->*m)(hrgn);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCPAINT(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCRBUTTONDBLCLK>
{
  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, BOOL, int x, int y, UINT codeHitTest)
      {(i_->*m)(codeHitTest, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCRBUTTONDBLCLK(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCRBUTTONDOWN>
{
  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, BOOL, int x, int y, UINT codeHitTest)
      {(i_->*m)(codeHitTest, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCRBUTTONDOWN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCRBUTTONUP>
{
  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, int x, int y, UINT codeHitTest)
      {(i_->*m)(codeHitTest, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NCRBUTTONUP(0, p1, p2, forwarder);
  }
};

//
// Note that the OWLNext 6.32 handler signature was non-compliant (void (T::*)(uint, uint)).
// For this reason we do not provide a compatibility implementation here.
// Also, note that the HANDLE_WM_NEXTDLGCTL macro in "windowsx.h" has a bug.
// It specifies HWND return type for the handler, not compliant with the documentation.
// See the Windows API documentation for details.
//
template <> struct TDispatch<WM_NEXTDLGCTL>
{
  template <class T, void (T::*M)(TParam1 handleOrDirectionFlag, bool isHandle)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(TParam1, bool);
      THandler m;
      HWND operator ()(HWND, HWND hwndSetFocus, BOOL fNext)
      {
        // Note: The spaces within the angle brackets on the next line are needed to avoid digraph '<:'.
        // According to the C++98 standard the digraph '<:' is interpreted as '['.
        //
        return (i_->*m)(reinterpret_cast< ::owl::TParam1 >(hwndSetFocus), !!fNext), 0;
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_NEXTDLGCTL(0, p1, p2, forwarder);
  }
};

//
// WM_NEXTMENU is missing from "windowsx.h".
// Note that the OWLNext 6.32 handler signature was non-compliant (void (T::*)()).
// For this reason we do not provide a compatibility implementation here.
// See the Windows API documentation for details.
//
template <> struct TDispatch<WM_NEXTMENU>
{
  template <class T, void (T::*M)(uint key, MDINEXTMENU&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    uint nKey = static_cast<uint>(p1);
    LPMDINEXTMENU lpMdiNextMenu = reinterpret_cast<LPMDINEXTMENU>(p2);
    CHECK(lpMdiNextMenu);
    if (lpMdiNextMenu) (ConvertVoidPtr<T>(i)->*M)(nKey, *lpMdiNextMenu);
  }
};

template <> struct TDispatch<WM_PAINT>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_PAINT(0, p1, p2, forwarder);
  }
};

//
// Note that the OWLNext 6.32 handler signature was non-compliant (void (T::*)(HWND, HANDLE)).
// For this reason we do not provide a compatibility implementation here.
// See the Windows API documentation for details.
//
template <> struct TDispatch<WM_PAINTCLIPBOARD>
{
  template <class T, void (T::*M)(HWND clipboardViewer, const PAINTSTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HWND, const PAINTSTRUCT&);
      THandler m;
      void operator ()(HWND, HWND hwndCBViewer, const PAINTSTRUCT* lpPaintStruct)
      {
        PRECONDITION(lpPaintStruct);
        if (lpPaintStruct) (i_->*m)(hwndCBViewer, *lpPaintStruct);
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_PAINTCLIPBOARD(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_PALETTECHANGED>
{
  template <class T, void (T::*M)(HWND paletteChange)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HWND);
      THandler m;
      void operator ()(HWND, HWND hWndPalChg)
      {(i_->*m)(hWndPalChg);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_PALETTECHANGED(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_PALETTEISCHANGING>
{
  template <class T, void (T::*M)(HWND paletteChange)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HWND);
      THandler m;
      void operator ()(HWND, HWND hwndPaletteChange)
      {(i_->*m)(hwndPaletteChange);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_PALETTEISCHANGING(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_PARENTNOTIFY>
{
  //
  /// Base class for the parameter to the WM_PARENTNOTIFY handler.
  //
  struct TArgs
  {
    UINT Event;

    TArgs(UINT e) : Event(e) {}
  };

  //
  /// Parameter package for the WM_PARENTNOTIFY handler.
  /// Used for events that are not mouse-related.
  //
  struct TChildInfoArgs : TArgs
  {
    HWND Child;
    UINT ChildID;

    TChildInfoArgs(UINT e, HWND c, UINT i) : TArgs(e), Child(c), ChildID(i) {}
  };

  //
  /// Parameter package for the WM_PARENTNOTIFY handler.
  /// Used for mouse-related events.
  //
  struct TMouseInfoArgs : TArgs
  {
    TPoint Coordinate;
    UINT Button; ///< Only valid for WM_XBUTTONDOWN.

    TMouseInfoArgs(UINT e, TPoint c, UINT b) : TArgs(e), Coordinate(c), Button(b) {}
  };

  //
  /// The handler should static_cast the TParentNotify argument to the specific derived type, which
  /// depends on the event handled. For mouse-related events the dynamic type of the argument is 
  /// TParentNotifyMouseInfo, and for other events it is TParentNotifyChildInfo.
  ///
  /// Se TFrameWindow::EvParentNotify for an example.
  //
  template <class T, void (T::*M)(const TArgs&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    UINT event = LOWORD(p1);
    if (event >= WM_MOUSEFIRST && event <= WM_MOUSELAST)
      (ConvertVoidPtr<T>(i)->*M)(TMouseInfoArgs(event, TPoint(GET_X_LPARAM(p2), GET_Y_LPARAM(p2)), HIWORD(p1)));
    else
      (ConvertVoidPtr<T>(i)->*M)(TChildInfoArgs(event, reinterpret_cast<HWND>(p2), HIWORD(p1)));
    return 0;
  }

};

typedef TDispatch<WM_PARENTNOTIFY>::TArgs TParentNotify; ///< Alias for convenience
typedef TDispatch<WM_PARENTNOTIFY>::TChildInfoArgs TParentNotifyChildInfo; ///< Alias for convenience
typedef TDispatch<WM_PARENTNOTIFY>::TMouseInfoArgs TParentNotifyMouseInfo; ///< Alias for convenience

template <> struct TDispatch<WM_PASTE>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_PASTE(0, p1, p2, forwarder);
  }
};

#if defined(OWL5_COMPAT)

//
// WM_POWER is an obsolete Win16 message. Applications should use the WM_POWERBROADCAST message.
// Note that HANDLE_WM_POWER uses a non-compliant signature for the handler (void (int)), which
// does not cater for the return value (PWR_FAIL | PWR_OK) for the event code PWR_SUSPENDREQUEST.
// Therefore, we crack the arguments manually here.
//
template <> struct TDispatch<WM_POWER>
{
  template <class T, int (T::*M)(uint event)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(p1));}
};

#endif

#if defined(OWL5_COMPAT)

//
// HANDLE_WM_POWERBROADCAST is missing from "windowsx.h".
// Note that the old handler return type (bool) is non-compliant.
// To compensate, we translate `true` to TRUE and `false` to BROADCAST_QUERY_DENY.
// See the Windows API documentation.
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa373247.aspx
//
template <> struct TDispatch<WM_POWERBROADCAST>
{
  template <class T, bool (T::*M)(uint event, TParam2)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    uint event = static_cast<uint>(p1);
    return (ConvertVoidPtr<T>(i)->*M)(event, p2) ? TRUE : BROADCAST_QUERY_DENY;
  }
};

#else

//
// HANDLE_WM_POWERBROADCAST is missing from "windowsx.h".
//
template <> struct TDispatch<WM_POWERBROADCAST>
{
  template <class T, TResult (T::*M)(uint event, TParam2)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    uint event = static_cast<uint>(p1);
    return (ConvertVoidPtr<T>(i)->*M)(event, p2);
  }
};

#endif 

//
// HANDLE_WM_PRINT is missing from "windowsx.h".
//
template <> struct TDispatch<WM_PRINT>
{
  template <class T, void (T::*M)(HDC dc, uint options)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    HDC dc = reinterpret_cast<HDC>(p1);
    uint options = static_cast<uint>(p2);
    return (ConvertVoidPtr<T>(i)->*M)(dc, options), 0;
  }
};

//
// HANDLE_WM_PRINTCLIENT is missing from "windowsx.h".
//
template <> struct TDispatch<WM_PRINTCLIENT>
{
  template <class T, void (T::*M)(HDC dc, uint options)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    HDC dc = reinterpret_cast<HDC>(p1);
    uint options = static_cast<uint>(p2);
    return (ConvertVoidPtr<T>(i)->*M)(dc, options), 0;
  }
};

template <> struct TDispatch<WM_QUERYDRAGICON>
{
  template <class T, HANDLE (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef HANDLE (T::*THandler)();
      THandler m;
      HANDLE operator ()(HWND)
      {return (i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_QUERYDRAGICON(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_QUERYENDSESSION does not pass the flags.
//
template <> struct TDispatch<WM_QUERYENDSESSION>
{
  template <class F>
  static bool Encode(F sendMessage, HWND wnd, uint flags)
  {return sendMessage(wnd, WM_QUERYENDSESSION, 0, static_cast<TParam2>(flags)) == TRUE;}

  template <class T, bool (T::*M)(uint flags)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {return (ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(p2)) ? TRUE : FALSE;}
};

template <> struct TDispatch<WM_QUERYNEWPALETTE>
{
  template <class T, bool (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef bool (T::*THandler)();
      THandler m;
      BOOL operator ()(HWND)
      {return (i_->*m)() ? TRUE : FALSE;}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_QUERYNEWPALETTE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_QUERYOPEN>
{
  template <class T, bool (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef bool (T::*THandler)();
      THandler m;
      BOOL operator ()(HWND)
      {return (i_->*m)() ? TRUE : FALSE;}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_QUERYOPEN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_QUEUESYNC>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_QUEUESYNC(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_RBUTTONDBLCLK>
{
  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, BOOL, int x, int y, UINT modKeys)
      {(i_->*m)(modKeys, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_RBUTTONDBLCLK(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_RBUTTONDOWN>
{
  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, BOOL, int x, int y, UINT modKeys)
      {(i_->*m)(modKeys, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_RBUTTONDOWN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_RBUTTONUP>
{
  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, int x, int y, UINT modKeys)
      {(i_->*m)(modKeys, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_RBUTTONUP(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_RENDERALLFORMATS>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_RENDERALLFORMATS(0, p1, p2, forwarder);
  }
};

//
// The HANDLE_WM_RENDERFORMAT macro in "windowsx.h" has a bug.
// It specifies HANDLE return type for the handler, not compliant with the documentation.
// Se the Windows API documentation for details.
//
template <> struct TDispatch<WM_RENDERFORMAT>
{
  template <class T, void (T::*M)(uint format)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {
    uint format = static_cast<UINT>(p1);
    return (ConvertVoidPtr<T>(i)->*M)(format), 0;
  }
};

template <> struct TDispatch<WM_SETCURSOR>
{
  template <class T, bool (T::*M)(HWND hWndCursor, uint codeHitTest, TMsgId mouseMsg)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef bool (T::*THandler)(HWND, uint, TMsgId);
      THandler m;
      BOOL operator ()(HWND, HWND hwndCursor, UINT codeHitTest, UINT msg)
      {return (i_->*m)(hwndCursor, codeHitTest, msg) ? TRUE : FALSE;}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SETCURSOR(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_SETICON is missing from "windowsx.h".
//
template <> struct TDispatch<WM_SETICON>
{
  template <class T, HICON (T::*M)(bool isBigIcon, HICON)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    HICON r = (ConvertVoidPtr<T>(i)->*M)(p1 == ICON_BIG, reinterpret_cast<HICON>(p2));
    return reinterpret_cast<TResult>(r);
  }
};

template <> struct TDispatch<WM_SETFOCUS>
{
  template <class T, void (T::*M)(HWND hwndLostFocus)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HWND);
      THandler m;
      void operator ()(HWND, HWND hWndLostFocus)
      {(i_->*m)(hWndLostFocus);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SETFOCUS(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SETFONT>
{
  template <class T, void (T::*M)(HFONT, bool redraw)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HFONT, bool);
      THandler m;
      void operator ()(HWND, HFONT hfont, BOOL fRedraw)
      {(i_->*m)(hfont, fRedraw);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SETFONT(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SETREDRAW>
{
  template <class T, void (T::*M)(bool redraw)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(bool);
      THandler m;
      void operator ()(HWND, BOOL fRedraw)
      {(i_->*m)(!!fRedraw);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SETREDRAW(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SETTEXT>
{
  template <class T, void (T::*M)(LPCTSTR)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(LPCTSTR);
      THandler m;
      void operator ()(HWND, LPCTSTR lpszText)
      {(i_->*m)(lpszText);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SETTEXT(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_SETTINGCHANGE is missing from "windowsx.h".
//
template <> struct TDispatch<WM_SETTINGCHANGE>
{
  template <class T, void (T::*M)(uint flags, LPCTSTR section)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    (ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(p1), reinterpret_cast<LPCTSTR>(p2));
    return 0;
  }
};

template <> struct TDispatch<WM_SHOWWINDOW>
{
  template <class T, void (T::*M)(bool, uint status)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(bool, uint);
      THandler m;
      void operator ()(HWND, BOOL fShow, UINT status)
      {(i_->*m)(fShow, status);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SHOWWINDOW(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SIZE>
{
  template <class T, void (T::*M)(uint, const TSize&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TSize&);
      THandler m;
      void operator ()(HWND, UINT sizeType, int cx, int cy)
      {(i_->*m)(sizeType, ::owl::TSize(cx, cy));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SIZE(0, p1, p2, forwarder);
  }
};

//
// Note that the OWLNext 6.32 handler signature was non-compliant (void (T::*)(HWND, HANDLE)).
// For this reason we do not provide a compatibility implementation here.
// See the Windows API documentation for details.
//
template <> struct TDispatch<WM_SIZECLIPBOARD>
{
  template <class T, void (T::*M)(HWND clipboardViewer, const TRect& client)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HWND, const TRect&);
      THandler m;
      void operator ()(HWND, HWND hwndCBViewer, const LPRECT lprc)
      {
        PRECONDITION(lprc);
        if (lprc) (i_->*m)(hwndCBViewer, ::owl::TRect(*lprc));
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SIZECLIPBOARD(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_SIZING is missing from "windowsx.h".
//
template <> struct TDispatch<WM_SIZING>
{
  template <class F>
  static bool Encode(F sendMessage, HWND wnd, uint side, TRect& r)
  {return static_cast<bool>(sendMessage(wnd, WM_SIZING, static_cast<TParam1>(side), reinterpret_cast<TParam2>(&r)));}

  template <class T, bool (T::*M)(uint side, TRect&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    PRECONDITION(p2);
    LPRECT r = reinterpret_cast<LPRECT>(p2);
    if (!r) return 0;
    TRect clone(*r);
    bool result = (ConvertVoidPtr<T>(i)->*M)(static_cast<unsigned int>(p1), clone);
    *r = clone;
    return result ? TRUE : FALSE;
  }
};

template <> struct TDispatch<WM_SPOOLERSTATUS> 
{
  template <class T, void (T::*M)(uint status, int jobsLeft)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, int);
      THandler m;
      void operator ()(HWND, UINT status, int cJobInQueue)
      {(i_->*m)(status, cJobInQueue);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SPOOLERSTATUS(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_STYLECHANGED is missing from "windowsx.h".
//
template <> struct TDispatch<WM_STYLECHANGED>
{
  template <class T, void (T::*M)(int styleType, const STYLESTRUCT& style)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    int styleType = static_cast<int>(p1);
    const STYLESTRUCT* lpStyleStruct = reinterpret_cast<LPSTYLESTRUCT>(p2);
    CHECK(lpStyleStruct);
    if (lpStyleStruct) (ConvertVoidPtr<T>(i)->*M)(styleType, *lpStyleStruct);
    return 0;
  }
};

//
// HANDLE_WM_STYLECHANGING is missing from "windowsx.h".
//
template <> struct TDispatch<WM_STYLECHANGING>
{
  template <class T, void (T::*M)(int styleType, STYLESTRUCT& style)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    int styleType = static_cast<int>(p1);
    LPSTYLESTRUCT lpStyleStruct = reinterpret_cast<LPSTYLESTRUCT>(p2);
    CHECK(lpStyleStruct);
    if (lpStyleStruct) (ConvertVoidPtr<T>(i)->*M)(styleType, *lpStyleStruct);
    return 0;
  }
};

//
// TODO: Remove the useless 'flags' parameter, change 'repeatCount' to int, and change the 'key' parameter to 'ch'. 
//
template <> struct TDispatch<WM_SYSCHAR>
{
  template <class T, void (T::*M)(uint key, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, uint, uint);
      THandler m;
      void operator ()(HWND, TCHAR ch, int cRepeat)
      {(i_->*m)(static_cast<unsigned int>(ch), static_cast<unsigned int>(cRepeat), 0);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SYSCHAR(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SYSCOLORCHANGE>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SYSCOLORCHANGE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SYSCOMMAND>
{
  template <class T, void (T::*M)(uint cmd, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, const TPoint&);
      THandler m;
      void operator ()(HWND, UINT cmd, int x, int y)
      {(i_->*m)(cmd, ::owl::TPoint(x, y));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SYSCOMMAND(0, p1, p2, forwarder);
  }
};

//
// TODO: Remove the useless 'flags' parameter, change 'repeatCount' to int, and change the 'key' parameter to 'ch'. 
//
template <> struct TDispatch<WM_SYSDEADCHAR>
{
  template <class T, void (T::*M)(uint ch, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, uint, uint);
      THandler m;
      void operator ()(HWND, TCHAR ch, int cRepeat)
      {(i_->*m)(static_cast<unsigned int>(ch), static_cast<unsigned int>(cRepeat), 0);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SYSDEADCHAR(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SYSKEYDOWN>
{
  template <class T, void (T::*M)(uint key, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, uint, uint);
      THandler m;
      void operator ()(HWND, UINT vk, BOOL, int cRepeat, UINT flags)
      {(i_->*m)(vk, static_cast<unsigned int>(cRepeat), flags);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SYSKEYDOWN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SYSKEYUP>
{
  template <class T, void (T::*M)(uint key, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, uint, uint);
      THandler m;
      void operator ()(HWND, UINT vk, BOOL, int cRepeat, UINT flags)
      {(i_->*m)(vk, static_cast<unsigned int>(cRepeat), flags);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_SYSKEYUP(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_TCARD is missing from "windowsx.h".
// Note that the OWLNext 6.32 handler signature was non-compliant (void (T::*)(int, int32)).
// For this reason we do not provide a compatibility implementation here.
// See the Windows API documentation for details.
//
template <> struct TDispatch<WM_TCARD>
{
  template <class T, void (T::*M)(uint idAction, ULONG_PTR data)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    uint idAction = static_cast<UINT>(p1);
    ULONG_PTR data = static_cast<ULONG_PTR>(p2);
    return (ConvertVoidPtr<T>(i)->*M)(idAction, data), 0;
  }
};

//
// HANDLE_WM_THEMECHANGED is missing from "windowsx.h".
//
template <> struct TDispatch<WM_THEMECHANGED>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (ConvertVoidPtr<T>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_TIMECHANGE>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_TIMECHANGE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_TIMER>
{
  template <class T, void (T::*M)(uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint);
      THandler m;
      void operator ()(HWND, UINT id)
      {(i_->*m)(id);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_TIMER(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_UNDO>
{
  template <class T, bool (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef bool (T::*THandler)();
      THandler m;
      BOOL operator ()(HWND)
      {return (i_->*m)() ? TRUE : FALSE;}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_UNDO(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_USERCHANGED>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)();
      THandler m;
      void operator ()(HWND)
      {(i_->*m)();}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_USERCHANGED(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_VKEYTOITEM>
{
  template <class T, int (T::*M)(uint vk, HWND listBox, int caretIndex)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef int (T::*THandler)(uint, HWND, int);
      THandler m;
      int operator ()(HWND, UINT vk, HWND hwndListbox, int iCaret)
      {return (i_->*m)(vk, hwndListbox, iCaret);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_VKEYTOITEM(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_VSCROLL>
{
  template <class T, void (T::*M)(uint scrollCode, uint thumbPos, HWND ctrl)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(uint, uint, HWND);
      THandler m;
      void operator ()(HWND, HWND hwndCtl, UINT code, int pos)
      {(i_->*m)(code, static_cast<unsigned int>(pos), hwndCtl);}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_VSCROLL(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_VSCROLLCLIPBOARD>
{
  template <class T, void (T::*M)(HWND cliboardViewer, uint scrollCode, uint thumbPos)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(HWND, uint, uint);
      THandler m;
      void operator ()(HWND, HWND hwndCBViewer, UINT code, int pos)
      {(i_->*m)(hwndCBViewer, code, static_cast<unsigned int>(pos));}
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_VSCROLLCLIPBOARD(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_WINDOWPOSCHANGED>
{
  template <class T, void (T::*M)(const WINDOWPOS&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef void (T::*THandler)(const WINDOWPOS&);
      THandler m;
      void operator ()(HWND, const LPWINDOWPOS lpwpos)
      {
        PRECONDITION(lpwpos);
        if (lpwpos) (i_->*m)(*lpwpos);
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_WINDOWPOSCHANGED(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_WINDOWPOSCHANGING>
{
  template <class T, bool (T::*M)(WINDOWPOS&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      typedef bool (T::*THandler)(WINDOWPOS&);
      THandler m;
      BOOL operator ()(HWND, LPWINDOWPOS lpwpos)
      {
        PRECONDITION(lpwpos);
        return (lpwpos && (i_->*m)(*lpwpos)) ? TRUE : FALSE;
      }
    }
    forwarder = {ConvertVoidPtr<T>(i), M};
    return HANDLE_WM_WINDOWPOSCHANGING(0, p1, p2, forwarder);
  }
};

#if defined(OWL5_COMPAT)

//
// Note: WM_WININICHANGE has been superceded by WM_SETTINGCHANGE.
// See the Windows API documentation for details.
// The new message has the same code, so we need to add a special dispatch function to provide
// backwards compatibility. 
//
template <class T, void (T::*M)(LPCTSTR section)>
static TResult Decode_WM_WININICHANGE(void* i, TParam1 p1, TParam2 p2)
{
  struct TForwarder
  {
    T* i_;
    typedef void (T::*THandler)(LPCTSTR);
    THandler m;
    void operator ()(HWND, LPCTSTR lpszSectionName)
    {(i_->*m)(lpszSectionName);}
  }
  forwarder = {ConvertVoidPtr<T>(i), M};
  return HANDLE_WM_WININICHANGE(0, p1, p2, forwarder);
}

#endif

//-------------------------------------------------------------------------------------------------
// Dialog message-specific dispatchers

template <> struct TDispatch<DM_GETDEFID>
{
  template <class T, uint (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {
    uint r = (ConvertVoidPtr<T>(i)->*M)();
    return r ? static_cast<TResult>(MkUint32(r, DC_HASDEFID)) : 0; // Ensure flag is set if ID is returned.
  }
};

template <> struct TDispatch<DM_REPOSITION>
{
  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (ConvertVoidPtr<T>(i)->*M)(), 0;}
};

template <> struct TDispatch<DM_SETDEFID>
{
  template <class T, bool (T::*M)(int)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (ConvertVoidPtr<T>(i)->*M)(static_cast<int>(p1)) ? TRUE : FALSE;}
};

//-------------------------------------------------------------------------------------------------
// OWLNext-specific messages and dispatchers

//
// Messages defined for OWL use - the top of the user range of ids is reserved
//
#define WM_OWLLAST          0x7FFF
#define WM_OWLFIRST        (WM_OWLLAST - 0x03FF)

#define WM_COMMAND_ENABLE  (WM_OWLLAST - 0)
#define WM_CHILDINVALID    (WM_OWLLAST - 1)
#define WM_OWLDOCUMENT     (WM_OWLLAST - 2)
#define WM_OWLVIEW         (WM_OWLLAST - 3)
#define WM_OWLNOTIFY       (WM_OWLLAST - 4)
#define WM_OWLPREPROCMENU  (WM_OWLLAST - 5)
#define WM_OWLCANCLOSE     (WM_OWLLAST - 6)
#define WM_VBXINITFORM     (WM_OWLLAST - 7)
#define WM_VBXNAME         (WM_OWLLAST - 8)
#define WM_VBXBASE         (WM_OWLLAST - 8 - 256)
#define WM_OWLWAKEUP       (WM_VBXBASE - 1)
#define WM_OWLFRAMESIZE    (WM_VBXBASE - 2) ///< Notify children of frame resizing
#define WM_OWLSLIPDBLCLK   (WM_VBXBASE - 3) ///< Notify parent of user dblclick of edge slip
#define WM_OWLWINDOWDOCKED (WM_VBXBASE - 4) ///< Notify window it was [un]docked/reparented
#define WM_OWLCREATETTIP   (WM_VBXBASE - 5) ///< Notify gadget window to create tooltips
#define WM_OWLHELPHIT      (WM_VBXBASE - 6) ///< Notify Help manager with hit point

template <> struct TDispatch<WM_CHILDINVALID>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, HWND invalidChild)
  {sendMessage(wnd, WM_CHILDINVALID, reinterpret_cast<TParam1>(invalidChild), 0);}

  template <class T, void (T::*M)(HWND)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (ConvertVoidPtr<T>(i)->*M)(reinterpret_cast<HWND>(p1)), 0;}
};

template <> struct TDispatch<WM_COMMAND_ENABLE>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, TCommandEnabler& enabler)
  {sendMessage(wnd, WM_COMMAND_ENABLE, 0, reinterpret_cast<TParam2>(&enabler));}

  template <class T, void (T::*M)(TCommandEnabler&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 ? ((ConvertVoidPtr<T>(i)->*M)(*reinterpret_cast<TCommandEnabler*>(p2)), 0) : 0;
  }
};

template <> struct TDispatch<WM_OWLCANCLOSE>
{
  template <class F>
  static bool Encode(F sendMessage, HWND wnd)
  {return sendMessage(wnd, WM_OWLCANCLOSE, 0, 0) == TRUE;}

  template <class T, bool (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (ConvertVoidPtr<T>(i)->*M)() ? TRUE : FALSE;}
};

template <> struct TDispatch<WM_OWLCREATETTIP>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd)
  {sendMessage(wnd, WM_OWLCREATETTIP, 0, 0);}

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (ConvertVoidPtr<T>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_OWLFRAMESIZE>
{
  template <class F>
  static void Encode(F sendMessage, uint sizeType, const TSize& size)
  {sendMessage(wnd, WM_OWLFRAMESIZE, static_cast<TParam1>(p1), reinterpret_cast<TParam2>(&p2));}

  template <class T, void (T::*M)(uint, const TSize&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 ? ((ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(p1), *reinterpret_cast<TSize*>(p2)), 0) : 0;
  }
};

template <> struct TDispatch<WM_OWLHELPHIT>
{
  template <class F>
  static void Encode(F sendMessage, const THelpHitInfo& i)
  {sendMessage(wnd, WM_OWLHELPHIT, 0, reinterpret_cast<TParam2>(&i));}

  template <class T, void (T::*M)(const THelpHitInfo&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 ? ((ConvertVoidPtr<T>(i)->*M)(*reinterpret_cast<THelpHitInfo*>(p2)), 0) : 0;
  }
};

template <> struct TDispatch<WM_OWLPREPROCMENU>
{
  template <class F>
  static void Encode(F sendMessage, HMENU menu)
  {sendMessage(wnd, WM_OWLPREPROCMENU, reinterpret_cast<TParam1>(menu), 0);}

  template <class T, void (T::*M)(HMENU)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (ConvertVoidPtr<T>(i)->*M)(reinterpret_cast<HMENU>(p1)), 0;}
};

template <> struct TDispatch<WM_OWLWAKEUP>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd)
  {sendMessage(wnd, WM_OWLWAKEUP, 0, 0);}

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (ConvertVoidPtr<T>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_OWLWINDOWDOCKED>
{
  template <class F>
  static void Encode(F sendMessage, uint pos, const TDockingSlip& d)
  {sendMessage(wnd, WM_OWLWINDOWDOCKED, static_cast<TParam1>(pos), reinterpret_cast<TParam2>(&d));}

  template <class T, void (T::*M)(uint, const TDockingSlip&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 ? ((ConvertVoidPtr<T>(i)->*M)(static_cast<uint>(p1), *reinterpret_cast<const TDockingSlip*>(p2)), 0) : 0;
  }
};

} // OWL namespace

#endif
