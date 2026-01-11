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

#include <owl/defs.h>
#include <owl/geometry.h>
#include <owl/wsyscls.h>
#include <windowsx.h>
#include <dbt.h> // We need PDEV_BROADCAST_HDR for the WM_DEVICECHANGE message.

OWL_DISABLE_WARNING_PUSH
OWL_DISABLE_WARNING_ZERO_AS_NULL_POINTER

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
  &dispatch<TMyClass, &TMyClass::method>

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
{return F(&obj, p1, p2);}

#define OWL_DISPATCH(dispatch, method)\
  &owl::AnyDispatch<&dispatch<TMyClass, &TMyClass::method> >

#endif

//-------------------------------------------------------------------------------------------------

#if !defined(OWL_EV_SIGNATURE_CHECK)
#define OWL_EV_SIGNATURE_CHECK 1
#endif

#if OWL_EV_SIGNATURE_CHECK

// Specialised dispatch templates

template <TMsgId MsgId>
struct TDispatchRawArguments
{
  template <class T> struct THandler {typedef TResult (T::*type)(TParam1, TParam2);};

  template <class T, TResult (T::*M)(TParam1, TParam2)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (static_cast<T*>(i)->*M)(p1, p2);}
};

template <TMsgId MsgId>
struct TDispatchChildNotify;

template <> struct TDispatchChildNotify<WM_COMMAND>
{
  template <class T> struct THandler {typedef void (T::*type)();};

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (static_cast<T*>(i)->*M)(), 0;}
};

template <TMsgId MsgId>
struct TDispatchChildNotifyWithCode;
  
template <> struct TDispatchChildNotifyWithCode<WM_COMMAND>
{
  template <class T> struct THandler {typedef void (T::*type)(uint);};

  template <class T, void (T::*M)(uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (static_cast<T*>(i)->*M)(static_cast<uint>(p1)), 0;}
};

template <TMsgId MsgId>
struct TDispatchCommand;

template <> struct TDispatchCommand<WM_COMMAND>
{
  template <class T> struct THandler {typedef void (T::*type)();};

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (static_cast<T*>(i)->*M)(), 0;}
};

template <TMsgId MsgId>
struct TDispatchCommandWithId;

template <> struct TDispatchCommandWithId<WM_COMMAND>
{
  template <class T> struct THandler {typedef void (T::*type)(uint);};

  template <class T, void (T::*M)(uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (static_cast<T*>(i)->*M)(static_cast<uint>(p1)), 0;}
};

#else

// Obsolete free-standing dispatch functions.

template <class T, TResult (T::*M)(TParam1, TParam2)>
TResult DispatchRawArguments(void* i, TParam1 p1, TParam2 p2)
{return (static_cast<T*>(i)->*M)(p1, p2);}

template <class T, void (T::*M)()>
TResult DispatchChildNotify(void* i, TParam1, TParam2)
{return (static_cast<T*>(i)->*M)(), 0;}

template <class T, void (T::*M)(uint)>
TResult DispatchChildNotifyWithCode(void* i, TParam1 p1, TParam2)
{return (static_cast<T*>(i)->*M)(static_cast<uint>(p1)), 0;}

template <class T, void (T::*M)()>
TResult DispatchCommand(void* i, TParam1, TParam2)
{return (static_cast<T*>(i)->*M)(), 0;}

template <class T, void (T::*M)(uint)>
TResult DispatchCommandWithId(void* i, TParam1 p1, TParam2)
{return (static_cast<T*>(i)->*M)(static_cast<uint>(p1)), 0;}

#endif

#if OWL_EV_SIGNATURE_CHECK

//
/// Utility template for checking event handler signatures.
//
template <class T, TMsgId MsgId, template <TMsgId> class D>
constexpr auto CheckSignature(typename D<MsgId>::template THandler<T>::type) noexcept -> bool
{return true;}

//
/// Utility template for checking event handler signatures.
/// Overload for notifications.
//
template <class T, TMsgId MsgId, uint NotificationCode, template <TMsgId> class D>
constexpr auto CheckSignature(typename D<MsgId>::template TNotificationDispatch<NotificationCode>::template THandler<T>::type) noexcept -> bool
{return true;}

#endif

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

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint, bool, HWND);};

#endif

  template <class T, void (T::*M)(uint, bool, HWND)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, UINT active, HWND other, BOOL minimized)
      {(i_->*M)(active, minimized, other);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_ACTIVATE(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_ACTIVATEAPP>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(bool, DWORD);};

#endif

  template <class T, void (T::*M)(bool, DWORD)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL active, DWORD threadId)
      {(i_->*M)(active, threadId);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_ACTIVATEAPP(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_ASKCBFORMATNAME>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint bufferSize, LPTSTR formatNameBuffer);};

#endif

  template <class T, void (T::*M)(uint bufferSize, LPTSTR formatNameBuffer)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int cchMax, LPTSTR rgchName)
      {(i_->*M)(static_cast<uint>(cchMax), rgchName);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_ASKCBFORMATNAME(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CANCELMODE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CANCELMODE(nullptr, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_CAPTURECHANGED is missing from "windowsx.h".
//
template <> struct TDispatch<WM_CAPTURECHANGED>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND capture);};

#endif

  template <class T, void (T::*M)(HWND capture)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {return (static_cast<T*>(i)->*M)(reinterpret_cast<HWND>(p2)), 0;}
};

template <> struct TDispatch<WM_CHANGECBCHAIN>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND removed, HWND next);};

#endif

  template <class T, void (T::*M)(HWND removed, HWND next)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND hwndRemove, HWND hwndNext)
      {(i_->*M)(hwndRemove, hwndNext);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CHANGECBCHAIN(nullptr, p1, p2, forwarder);
  }
};

//
// TODO: Remove the useless 'flags' parameter, change 'repeatCount' to int, and change the 'key' parameter to 'ch'.
//
template <> struct TDispatch<WM_CHAR>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint ch, uint repeatCount, uint flags);};

#endif

  template <class T, void (T::*M)(uint ch, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, TCHAR ch, int cRepeat)
      {(i_->*M)(static_cast<uint>(ch), static_cast<uint>(cRepeat), 0);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CHAR(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CHARTOITEM>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef int (T::*type)(uint ch, HWND listBox, int caretIndex);};

#endif

  template <class T, int (T::*M)(uint ch, HWND listBox, int caretIndex)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      int operator ()(HWND, UINT ch, HWND hwndListbox, int iCaret)
      {return (i_->*M)(ch, hwndListbox, iCaret);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CHARTOITEM(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CHILDACTIVATE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CHILDACTIVATE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CLOSE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CLOSE(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_COMPACTING>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint compactRatio);};

#endif

  template <class T, void (T::*M)(uint compactRatio)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, UINT compactRatio)
      {(i_->*M)(compactRatio);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_COMPACTING(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_COMPAREITEM>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef int (T::*type)(uint, const COMPAREITEMSTRUCT&);};

#endif

  template <class T, int (T::*M)(uint, const COMPAREITEMSTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      int operator ()(HWND, const COMPAREITEMSTRUCT* c)
      {
        PRECONDITION(c);
        return c ? (i_->*M)(c->CtlID, *c) : 0;
      }
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_COMPAREITEM(nullptr, p1, p2, forwarder);
  }
};

//
// TODO: Coordinate parameters should be replaced by TPoint.
//
template <> struct TDispatch<WM_CONTEXTMENU>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND, int, int);};

#endif

  template <class T, void (T::*M)(HWND, int, int)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND w, int x, int y)
      {(i_->*M)(w, x, y);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CONTEXTMENU(nullptr, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_COPYDATA has a bug; it treats the handler as returning nothing (void), and the macro
// hence always returns 0. We therefore implement this specialization manually.
//
template <> struct TDispatch<WM_COPYDATA>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)(HWND sender, const COPYDATASTRUCT&);};

#endif

  template <class T, bool (T::*M)(HWND sender, const COPYDATASTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    PRECONDITION(p2 != 0);
    return (p2 != 0 && (static_cast<T*>(i)->*M)(reinterpret_cast<HWND>(p1), *reinterpret_cast<PCOPYDATASTRUCT>(p2))) ? TRUE : FALSE;
  }
};

template <> struct TDispatch<WM_CREATE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)(CREATESTRUCT&);};

#endif

  template <class T, bool (T::*M)(CREATESTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      BOOL operator ()(HWND, LPCREATESTRUCT c)
      {
        PRECONDITION(c);
        return (c && (i_->*M)(*c)) ? TRUE : FALSE;
      }
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CREATE(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CTLCOLORBTN>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef HBRUSH (T::*type)(HDC, HWND, uint);};

#endif

  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*M)(d, w, ctlType);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CTLCOLORBTN(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CTLCOLOREDIT>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef HBRUSH (T::*type)(HDC, HWND, uint);};

#endif

  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*M)(d, w, ctlType);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CTLCOLOREDIT(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CTLCOLORDLG>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef HBRUSH (T::*type)(HDC, HWND, uint);};

#endif

  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*M)(d, w, ctlType);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CTLCOLORDLG(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CTLCOLORLISTBOX>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef HBRUSH (T::*type)(HDC, HWND, uint);};

#endif

  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*M)(d, w, ctlType);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CTLCOLORLISTBOX(nullptr, p1, p2, forwarder);
  }
};

#if defined(OWL5_COMPAT)

//
/// This dispatch specialization is deprecated.
/// The WM_CTLCOLORMSGBOX message is obsolete. It is not even documented at MSDN.
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/ms644995.aspx#control_color
//
template <> struct TDispatch<WM_CTLCOLORMSGBOX>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef HBRUSH (T::*type)(HDC, HWND, uint);};

#endif

  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*M)(d, w, ctlType);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CTLCOLORMSGBOX(0, p1, p2, forwarder);
  }
};

#endif

template <> struct TDispatch<WM_CTLCOLORSCROLLBAR>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef HBRUSH (T::*type)(HDC, HWND, uint);};

#endif

  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*M)(d, w, ctlType);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CTLCOLORSCROLLBAR(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CTLCOLORSTATIC>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef HBRUSH (T::*type)(HDC, HWND, uint);};

#endif

  template <class T, HBRUSH (T::*M)(HDC, HWND, uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      HBRUSH operator ()(HWND, HDC d, HWND w, UINT ctlType)
      {return (i_->*M)(d, w, ctlType);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CTLCOLORSTATIC(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_CUT>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_CUT(0, p1, p2, forwarder);
  }
};

//
// TODO: Remove the useless 'flags' parameter, change 'repeatCount' to int, and change the 'deadKey' parameter to 'ch'.
//
template <> struct TDispatch<WM_DEADCHAR>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint deadKey, uint repeatCount, uint flags);};

#endif

  template <class T, void (T::*M)(uint deadKey, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, TCHAR ch, int cRepeat)
      {(i_->*M)(static_cast<uint>(ch), static_cast<uint>(cRepeat), 0);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_DEADCHAR(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DELETEITEM>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint, const DELETEITEMSTRUCT&);};

#endif

  template <class T, void (T::*M)(uint, const DELETEITEMSTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, const DELETEITEMSTRUCT* d)
      {
        PRECONDITION(d);
        if (d) (i_->*M)(d->CtlID, *d);
      }
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_DELETEITEM(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DESTROY>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_DESTROY(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DESTROYCLIPBOARD>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
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

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef TResult (T::*type)(uint event, PDEV_BROADCAST_HDR eventData);};

#endif

  template <class T, TResult (T::*M)(uint event, PDEV_BROADCAST_HDR eventData)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (static_cast<T*>(i)->*M)(static_cast<uint>(p1), reinterpret_cast<PDEV_BROADCAST_HDR>(p2));}
};

template <> struct TDispatch<WM_DEVMODECHANGE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(LPCTSTR deviceName);};

#endif

  template <class T, void (T::*M)(LPCTSTR deviceName)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, LPCTSTR lpszDeviceName)
      {(i_->*M)(lpszDeviceName);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_DEVMODECHANGE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DISPLAYCHANGE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint bitsPerPixel, uint screenResolutionWidth, uint screenResolutionHeight);};

#endif

  template <class T, void (T::*M)(uint bitsPerPixel, uint screenResolutionWidth, uint screenResolutionHeight)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, UINT bitsPerPixel, UINT cxScreen, UINT cyScreen)
      {(i_->*M)(bitsPerPixel, cxScreen, cyScreen);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_DISPLAYCHANGE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DRAWCLIPBOARD>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_DRAWCLIPBOARD(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DRAWITEM>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint, const DRAWITEMSTRUCT&);};

#endif

  template <class T, void (T::*M)(uint, const DRAWITEMSTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, const DRAWITEMSTRUCT* d)
      {
        PRECONDITION(d);
        if (d) (i_->*M)(d->CtlID, *d);
      }
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_DRAWITEM(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_DROPFILES>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(TDropInfo);};

#endif

  template <class T, void (T::*M)(TDropInfo)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HDROP hdrop)
      {(i_->*M)(TDropInfo(hdrop));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_DROPFILES(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_ENABLE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(bool);};

#endif

  template <class T, void (T::*M)(bool)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL fEnable)
      {(i_->*M)(fEnable);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
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


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(bool endSession, uint flags);};

#endif

  template <class T, void (T::*M)(bool endSession, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (static_cast<T*>(i)->*M)(static_cast<bool>(p1), static_cast<uint>(p2)), 0;}
};

template <> struct TDispatch<WM_ENTERIDLE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint source, HWND);};

#endif

  template <class T, void (T::*M)(uint source, HWND)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, UINT source, HWND hWndDlg)
      {(i_->*M)(source, hWndDlg);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_ENTERIDLE(nullptr, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_ENTERMENULOOP is missing from "windowsx.h".
//
template <> struct TDispatch<WM_ENTERMENULOOP>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(bool isTrackPopupMenu);};

#endif

  template <class T, void (T::*M)(bool isTrackPopupMenu)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (static_cast<T*>(i)->*M)(static_cast<BOOL>(p1) == TRUE), 0;}
};

template <> struct TDispatch<WM_ENTERSIZEMOVE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_ENTERSIZEMOVE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_ERASEBKGND>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)(HDC);};

#endif

  template <class T, bool (T::*M)(HDC)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      BOOL operator ()(HWND, HDC h)
      {return (i_->*M)(h) ? TRUE : FALSE;}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_ERASEBKGND(nullptr, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_EXITMENULOOP is missing from "windowsx.h".
//
template <> struct TDispatch<WM_EXITMENULOOP>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(bool isTrackPopupMenu);};

#endif

  template <class T, void (T::*M)(bool isTrackPopupMenu)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (static_cast<T*>(i)->*M)(static_cast<BOOL>(p1) == TRUE), 0;}
};

//
// HANDLE_WM_EXITSIZEMOVE is missing from "windowsx.h".
//
template <> struct TDispatch<WM_EXITSIZEMOVE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (static_cast<T*>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_FONTCHANGE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_FONTCHANGE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_GETDLGCODE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef uint (T::*type)(const MSG*);};

#endif

  template <class T, uint (T::*M)(const MSG*)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      uint operator ()(HWND, LPMSG msg)
      {return (i_->*M)(msg);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_GETDLGCODE(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_GETFONT>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef HFONT (T::*type)();};

#endif

  template <class T, HFONT (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      HFONT operator ()(HWND)
      {return (i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_GETFONT(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_GETICON>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef HICON (T::*type)(uint iconType);};

#endif

  template <class T, HICON (T::*M)(uint iconType)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      HICON operator ()(HWND, UINT iconType)
      {return (i_->*M)(iconType);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_GETICON(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_GETMINMAXINFO>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(MINMAXINFO&);};

#endif

  template <class T, void (T::*M)(MINMAXINFO&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, LPMINMAXINFO lpMinMaxInfo)
      {
        PRECONDITION(lpMinMaxInfo);
        if (lpMinMaxInfo) (i_->*M)(*lpMinMaxInfo);
      }
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_GETMINMAXINFO(nullptr, p1, p2, forwarder);
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
    (static_cast<T*>(i)->*M)(static_cast<uint>(textBufferSize), reinterpret_cast<LPTSTR>(p2));
    int length = static_cast<int>(static_cast<T*>(i)->HandleMessage(WM_GETTEXTLENGTH));
    CHECK(length >= 0);
    return static_cast<TResult>(length >= textBufferSize ? textBufferSize - 1 : length);
  }
};

#else

template <> struct TDispatch<WM_GETTEXT>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef int (T::*type)(int textBufferSize, LPTSTR textBuffer);};

#endif

  template <class T, int (T::*M)(int textBufferSize, LPTSTR textBuffer)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      int operator ()(HWND, int cchTextMax, LPTSTR lpszText)
      {return (i_->*M)(cchTextMax, lpszText);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
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

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef uint (T::*type)();};

#endif

  template <class T, uint (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      int operator ()(HWND)
      {return static_cast<int>((i_->*M)());}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_GETTEXTLENGTH(0, p1, p2, forwarder);
  }
};

#else

template <> struct TDispatch<WM_GETTEXTLENGTH>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef int (T::*type)();};

#endif

  template <class T, int (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      int operator ()(HWND)
      {return (i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
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

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(const HELPINFO&);};

#endif

  template <class T, void (T::*M)(const HELPINFO&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 != 0 ? ((static_cast<T*>(i)->*M)(*reinterpret_cast<HELPINFO*>(p2)), 0) : 0;
  }
};

#if defined(OWL5_COMPAT)

template <> struct TDispatch<WM_HOTKEY>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(int idHotKey);};

#endif

  template <class T, void (T::*M)(int idHotKey)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int idHotKey, UINT /*fuModifiers*/, UINT /*vk*/)
      {(i_->*M)(idHotKey);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_HOTKEY(0, p1, p2, forwarder);
  }
};

#else

template <> struct TDispatch<WM_HOTKEY>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(int idHotKey, uint modifiers, uint vk);};

#endif

  template <class T, void (T::*M)(int idHotKey, uint modifiers, uint vk)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int idHotKey, UINT fuModifiers, UINT vk)
      {(i_->*M)(idHotKey, fuModifiers, vk);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_HOTKEY(0, p1, p2, forwarder);
  }
};

#endif

template <> struct TDispatch<WM_HSCROLL>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint scrollCode, uint pos, HWND hCtl);};

#endif

  template <class T, void (T::*M)(uint scrollCode, uint pos, HWND hCtl)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND hwndCtl, UINT code, int pos)
      {(i_->*M)(code, static_cast<uint>(pos), hwndCtl);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_HSCROLL(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_HSCROLLCLIPBOARD>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND cliboardViewer, uint scrollCode, uint pos);};

#endif

  template <class T, void (T::*M)(HWND cliboardViewer, uint scrollCode, uint pos)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND hwndCBViewer, UINT code, int pos)
      {(i_->*M)(hwndCBViewer, code, static_cast<uint>(pos));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_HSCROLLCLIPBOARD(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_INITMENU>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HMENU);};

#endif

  template <class T, void (T::*M)(HMENU)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HMENU hMenu)
      {(i_->*M)(hMenu);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_INITMENU(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_INITMENUPOPUP>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HMENU, uint index, bool isSysMenu);};

#endif

  template <class T, void (T::*M)(HMENU, uint index, bool isSysMenu)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HMENU hMenu, UINT item, BOOL fSystemMenu)
      {(i_->*M)(hMenu, item, fSystemMenu);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_INITMENUPOPUP(nullptr, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_IMPUTLANGCHANGE is missing from "windowsx.h".
//
template <> struct TDispatch<WM_INPUTLANGCHANGE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint charSet, uint localeId);};

#endif

  template <class T, void (T::*M)(uint charSet, uint localeId)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (static_cast<T*>(i)->*M)(static_cast<uint>(p1), static_cast<uint>(p2)), 0;}
};

//
// HANDLE_WM_IMPUTLANGCHANGEREQUEST is missing from "windowsx.h".
//
template <> struct TDispatch<WM_INPUTLANGCHANGEREQUEST>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint flags, uint localeId);};

#endif

  template <class T, void (T::*M)(uint flags, uint localeId)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (static_cast<T*>(i)->*M)(static_cast<uint>(p1), static_cast<uint>(p2)), 0;}
};

template <> struct TDispatch<WM_KEYDOWN>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint key, uint repeatCount, uint flags);};

#endif

  template <class T, void (T::*M)(uint key, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, UINT vk, BOOL, int cRepeat, UINT flags)
      {(i_->*M)(vk, static_cast<uint>(cRepeat), flags);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_KEYDOWN(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_KEYUP>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint key, uint repeatCount, uint flags);};

#endif

  template <class T, void (T::*M)(uint key, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, UINT vk, BOOL, int cRepeat, UINT flags)
      {(i_->*M)(vk, static_cast<uint>(cRepeat), flags);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_KEYUP(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_KILLFOCUS>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND);};

#endif

  template <class T, void (T::*M)(HWND)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND hwndNewFocus)
      {(i_->*M)(hwndNewFocus);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_KILLFOCUS(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_LBUTTONDBLCLK>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, int x, int y, UINT modKeys)
      {(i_->*M)(modKeys, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_LBUTTONDBLCLK(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_LBUTTONDOWN>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, int x, int y, UINT modKeys)
      {(i_->*M)(modKeys, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_LBUTTONDOWN(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_LBUTTONUP>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int x, int y, UINT modKeys)
      {(i_->*M)(modKeys, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_LBUTTONUP(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MBUTTONDBLCLK>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, int x, int y, UINT modKeys)
      {(i_->*M)(modKeys, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_MBUTTONDBLCLK(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MBUTTONDOWN>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, int x, int y, UINT modKeys)
      {(i_->*M)(modKeys, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_MBUTTONDOWN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MBUTTONUP>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int x, int y, UINT modKeys)
      {(i_->*M)(modKeys, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_MBUTTONUP(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MDIACTIVATE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND activated, HWND deactivated);};

#endif

  template <class T, void (T::*M)(HWND activated, HWND deactivated)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, HWND activated, HWND deactivated)
      {(i_->*M)(activated, deactivated);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_MDIACTIVATE(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MDICASCADE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)(uint cmd);};

#endif

  template <class T, bool (T::*M)(uint cmd)>
  static TResult Decode(T* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      BOOL operator ()(HWND, UINT cmd)
      {return (i_->*M)(cmd) ? TRUE : FALSE;}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_MDICASCADE(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_MDICREATE casts the HWND result value through UINT. Seems like a bug, so manually crack this one.
//
template <> struct TDispatch<WM_MDICREATE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef HWND (T::*type)(MDICREATESTRUCT&);};

#endif

  template <class T, HWND (T::*M)(MDICREATESTRUCT&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 != 0 ? reinterpret_cast<TResult>((static_cast<T*>(i)->*M)(*reinterpret_cast<LPMDICREATESTRUCT>(p2))) : 0;
  }
};

template <> struct TDispatch<WM_MDIDESTROY>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND);};

#endif

  template <class T, void (T::*M)(HWND)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND hwndDestroy)
      {(i_->*M)(hwndDestroy);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_MDIDESTROY(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MEASUREITEM>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint, MEASUREITEMSTRUCT&);};

#endif

  template <class T, void (T::*M)(uint, MEASUREITEMSTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, MEASUREITEMSTRUCT* m)
      {
        PRECONDITION(m);
        if (m) (i_->*M)(m->CtlID, *m);
      }
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_MEASUREITEM(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MENUCHAR>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef int32 (T::*type)(uint, uint, HMENU);};

#endif

  template <class T, int32 (T::*M)(uint, uint, HMENU)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      int32 operator ()(HWND, UINT nChar, UINT menuType, HMENU hmenu)
      {return (i_->*M)(nChar, menuType, hmenu);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
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


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint item, uint flags, HMENU);};

#endif

  template <class T, void (T::*M)(uint item, uint flags, HMENU)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (static_cast<T*>(i)->*M)(LOWORD(p1), HIWORD(p1), reinterpret_cast<HMENU>(p2)), 0;}
};

template <> struct TDispatch<WM_MOUSEACTIVATE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef uint (T::*type)(HWND topLevel, uint codeHitTest, TMsgId);};

#endif

  template <class T, uint (T::*M)(HWND topLevel, uint codeHitTest, TMsgId)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      uint operator ()(HWND, HWND hwndTopLevel, UINT codeHitTest, UINT msg)
      {return (i_->*M)(hwndTopLevel, codeHitTest, static_cast<TMsgId>(msg));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_MOUSEACTIVATE(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MOUSEHOVER>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    TPoint p(GET_X_LPARAM(p2), GET_Y_LPARAM(p2));
    return (static_cast<T*>(i)->*M)(static_cast<uint>(p1), p), 0;
  }
};

template <> struct TDispatch<WM_MOUSEHWHEEL>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, int zDelta, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, int zDelta, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int x, int y, int zDelta, UINT modKeys)
      {(i_->*M)(modKeys, zDelta, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_MOUSEWHEEL(nullptr, p1, p2, forwarder);   // There is no HANDLE_WM_MOUSEHWHEEL in Windows SDK, so reuse HANDLE_WM_MOUSEWHEEL
  }
};


template <> struct TDispatch<WM_MOUSELEAVE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (static_cast<T*>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_MOUSEMOVE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int x, int y, UINT modKeys)
      {(i_->*M)(modKeys, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_MOUSEMOVE(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MOUSEWHEEL>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, int zDelta, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, int zDelta, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int x, int y, int zDelta, UINT modKeys)
      {(i_->*M)(modKeys, zDelta, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_MOUSEWHEEL(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_MOVE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(const TPoint&);};

#endif

  template <class T, void (T::*M)(const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int x, int y)
      {(i_->*M)(TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_MOVE(nullptr, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_MOVING is missing from "windowsx.h".
//
template <> struct TDispatch<WM_MOVING>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(TRect&);};

#endif

  template <class T, void (T::*M)(TRect&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    LPRECT rect = reinterpret_cast<LPRECT>(p2);
    CHECK(rect);
    if (!rect) return FALSE; // Not processed.
    TRect clone(*rect);
    (static_cast<T*>(i)->*M)(clone);
    *rect = clone;
    return TRUE; // Processed.
  }
};

template <> struct TDispatch<WM_NCACTIVATE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)(bool);};

#endif

  template <class T, bool (T::*M)(bool)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      BOOL operator ()(HWND, BOOL fActive, HWND, BOOL)
      {return (i_->*M)(fActive) ? TRUE : FALSE;}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCACTIVATE(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCCALCSIZE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef uint (T::*type)(bool calcValidRects, NCCALCSIZE_PARAMS&);};

#endif

  template <class T, uint (T::*M)(bool calcValidRects, NCCALCSIZE_PARAMS&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      uint operator ()(HWND, BOOL fCalcValidRects, NCCALCSIZE_PARAMS* lpcsp)
      {
        PRECONDITION(lpcsp);
        return lpcsp ? (i_->*M)(fCalcValidRects, *lpcsp) : 0;
      }
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCCALCSIZE(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCCREATE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)(CREATESTRUCT&);};

#endif

  template <class T, bool (T::*M)(CREATESTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      BOOL operator ()(HWND, LPCREATESTRUCT lpCreateStruct)
      {
        PRECONDITION(lpCreateStruct);
        return (lpCreateStruct && (i_->*M)(*lpCreateStruct)) ? TRUE : FALSE;
      }
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCCREATE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCDESTROY>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCDESTROY(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCHITTEST>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef uint (T::*type)(const TPoint&);};

#endif

  template <class T, uint (T::*M)(const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      uint operator ()(HWND, int x, int y)
      {return (i_->*M)(TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCHITTEST(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCLBUTTONDBLCLK>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint codeHitTest, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, int x, int y, UINT codeHitTest)
      {(i_->*M)(codeHitTest, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCLBUTTONDBLCLK(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCLBUTTONDOWN>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint codeHitTest, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, int x, int y, UINT codeHitTest)
      {(i_->*M)(codeHitTest, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCLBUTTONDOWN(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCLBUTTONUP>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint codeHitTest, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int x, int y, UINT codeHitTest)
      {(i_->*M)(codeHitTest, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCLBUTTONUP(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCMBUTTONDBLCLK>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint codeHitTest, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, int x, int y, UINT codeHitTest)
      {(i_->*M)(codeHitTest, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCMBUTTONDBLCLK(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCMBUTTONDOWN>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint codeHitTest, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, int x, int y, UINT codeHitTest)
      {(i_->*M)(codeHitTest, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCMBUTTONDOWN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCMBUTTONUP>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint codeHitTest, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int x, int y, UINT codeHitTest)
      {(i_->*M)(codeHitTest, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCMBUTTONUP(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_NCMOUSEHOVER is missing from "windowsx.h".
//
template <> struct TDispatch<WM_NCMOUSEHOVER>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint codeHitTest, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    TPoint p(GET_X_LPARAM(p2), GET_Y_LPARAM(p2));
    return (static_cast<T*>(i)->*M)(static_cast<uint>(p1), p), 0;
  }
};

//
// HANDLE_WM_NCMOUSELEAVE is missing from "windowsx.h".
//
template <> struct TDispatch<WM_NCMOUSELEAVE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (static_cast<T*>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_NCMOUSEMOVE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint codeHitTest, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int x, int y, UINT codeHitTest)
      {(i_->*M)(codeHitTest, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCMOUSEMOVE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCPAINT>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HRGN);};

#endif

  template <class T, void (T::*M)(HRGN)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HRGN hrgn)
      {(i_->*M)(hrgn);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCPAINT(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCRBUTTONDBLCLK>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint codeHitTest, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, int x, int y, UINT codeHitTest)
      {(i_->*M)(codeHitTest, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCRBUTTONDBLCLK(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCRBUTTONDOWN>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint codeHitTest, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, int x, int y, UINT codeHitTest)
      {(i_->*M)(codeHitTest, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_NCRBUTTONDOWN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_NCRBUTTONUP>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint codeHitTest, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint codeHitTest, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int x, int y, UINT codeHitTest)
      {(i_->*M)(codeHitTest, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
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

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(TParam1 handleOrDirectionFlag, bool isHandle);};

#endif

  template <class T, void (T::*M)(TParam1 handleOrDirectionFlag, bool isHandle)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      HWND operator ()(HWND, HWND hwndSetFocus, BOOL fNext)
      {return (i_->*M)(reinterpret_cast<TParam1>(hwndSetFocus), !!fNext), 0;}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
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

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint key, MDINEXTMENU&);};

#endif

  template <class T, void (T::*M)(uint key, MDINEXTMENU&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    uint nKey = static_cast<uint>(p1);
    LPMDINEXTMENU lpMdiNextMenu = reinterpret_cast<LPMDINEXTMENU>(p2);
    CHECK(lpMdiNextMenu);
    if (lpMdiNextMenu) (static_cast<T*>(i)->*M)(nKey, *lpMdiNextMenu);
  }
};

template <> struct TDispatch<WM_PAINT>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_PAINT(nullptr, p1, p2, forwarder);
  }
};

//
// Note that the OWLNext 6.32 handler signature was non-compliant (void (T::*)(HWND, HANDLE)).
// For this reason we do not provide a compatibility implementation here.
// See the Windows API documentation for details.
//
template <> struct TDispatch<WM_PAINTCLIPBOARD>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND clipboardViewer, const PAINTSTRUCT&);};

#endif

  template <class T, void (T::*M)(HWND clipboardViewer, const PAINTSTRUCT&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND hwndCBViewer, const PAINTSTRUCT* lpPaintStruct)
      {
        PRECONDITION(lpPaintStruct);
        if (lpPaintStruct) (i_->*M)(hwndCBViewer, *lpPaintStruct);
      }
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_PAINTCLIPBOARD(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_PALETTECHANGED>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND paletteChange);};

#endif

  template <class T, void (T::*M)(HWND paletteChange)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND hWndPalChg)
      {(i_->*M)(hWndPalChg);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_PALETTECHANGED(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_PALETTEISCHANGING>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND paletteChange);};

#endif

  template <class T, void (T::*M)(HWND paletteChange)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND hwndPaletteChange)
      {(i_->*M)(hwndPaletteChange);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
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

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(const TArgs&);};

#endif

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
      (static_cast<T*>(i)->*M)(TMouseInfoArgs(event, TPoint(GET_X_LPARAM(p2), GET_Y_LPARAM(p2)), HIWORD(p1)));
    else
      (static_cast<T*>(i)->*M)(TChildInfoArgs(event, reinterpret_cast<HWND>(p2), HIWORD(p1)));
    return 0;
  }

};

typedef TDispatch<WM_PARENTNOTIFY>::TArgs TParentNotify; ///< Alias for convenience
typedef TDispatch<WM_PARENTNOTIFY>::TChildInfoArgs TParentNotifyChildInfo; ///< Alias for convenience
typedef TDispatch<WM_PARENTNOTIFY>::TMouseInfoArgs TParentNotifyMouseInfo; ///< Alias for convenience

template <> struct TDispatch<WM_PASTE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
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

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef int (T::*type)(uint event);};

#endif

  template <class T, int (T::*M)(uint event)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (static_cast<T*>(i)->*M)(static_cast<uint>(p1));}
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

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)(uint event, TParam2);};

#endif

  template <class T, bool (T::*M)(uint event, TParam2)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    uint event = static_cast<uint>(p1);
    return (static_cast<T*>(i)->*M)(event, p2) ? TRUE : BROADCAST_QUERY_DENY;
  }
};

#else

//
// HANDLE_WM_POWERBROADCAST is missing from "windowsx.h".
//
template <> struct TDispatch<WM_POWERBROADCAST>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef TResult (T::*type)(uint event, TParam2);};

#endif

  template <class T, TResult (T::*M)(uint event, TParam2)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    uint event = static_cast<uint>(p1);
    return (static_cast<T*>(i)->*M)(event, p2);
  }
};

#endif

//
// HANDLE_WM_PRINT is missing from "windowsx.h".
//
template <> struct TDispatch<WM_PRINT>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HDC dc, uint options);};

#endif

  template <class T, void (T::*M)(HDC dc, uint options)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    HDC dc = reinterpret_cast<HDC>(p1);
    uint options = static_cast<uint>(p2);
    return (static_cast<T*>(i)->*M)(dc, options), 0;
  }
};

//
// HANDLE_WM_PRINTCLIENT is missing from "windowsx.h".
//
template <> struct TDispatch<WM_PRINTCLIENT>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HDC dc, uint options);};

#endif

  template <class T, void (T::*M)(HDC dc, uint options)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    HDC dc = reinterpret_cast<HDC>(p1);
    uint options = static_cast<uint>(p2);
    return (static_cast<T*>(i)->*M)(dc, options), 0;
  }
};

//
// The HANDLE_WM_QUERYDRAGICON macro in "windowsx.h" has a bug; the HICON result is cast through UINT.
//
template <> struct TDispatch<WM_QUERYDRAGICON>
{
  template <class F>
  static HICON Encode(F sendMessage, HWND wnd)
  {return reinterpret_cast<HICON>(sendMessage(wnd, WM_QUERYDRAGICON, 0, 0));}


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef HICON (T::*type)();};

#endif

  template <class T, HICON (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return reinterpret_cast<TResult>((static_cast<T*>(i)->*M)());}
};

//
// HANDLE_WM_QUERYENDSESSION does not pass the flags.
//
template <> struct TDispatch<WM_QUERYENDSESSION>
{
  template <class F>
  static bool Encode(F sendMessage, HWND wnd, uint flags)
  {return sendMessage(wnd, WM_QUERYENDSESSION, 0, static_cast<TParam2>(flags)) == TRUE;}


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)(uint flags);};

#endif

  template <class T, bool (T::*M)(uint flags)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {return (static_cast<T*>(i)->*M)(static_cast<uint>(p2)) ? TRUE : FALSE;}
};

template <> struct TDispatch<WM_QUERYNEWPALETTE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)();};

#endif

  template <class T, bool (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      BOOL operator ()(HWND)
      {return (i_->*M)() ? TRUE : FALSE;}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_QUERYNEWPALETTE(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_QUERYOPEN>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)();};

#endif

  template <class T, bool (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      BOOL operator ()(HWND)
      {return (i_->*M)() ? TRUE : FALSE;}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_QUERYOPEN(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_QUEUESYNC>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_QUEUESYNC(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_RBUTTONDBLCLK>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, int x, int y, UINT modKeys)
      {(i_->*M)(modKeys, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_RBUTTONDBLCLK(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_RBUTTONDOWN>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL, int x, int y, UINT modKeys)
      {(i_->*M)(modKeys, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_RBUTTONDOWN(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_RBUTTONUP>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint modKeys, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint modKeys, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, int x, int y, UINT modKeys)
      {(i_->*M)(modKeys, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_RBUTTONUP(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_RENDERALLFORMATS>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
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

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint format);};

#endif

  template <class T, void (T::*M)(uint format)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {
    uint format = static_cast<UINT>(p1);
    return (static_cast<T*>(i)->*M)(format), 0;
  }
};

template <> struct TDispatch<WM_SETCURSOR>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)(HWND hWndCursor, uint codeHitTest, TMsgId mouseMsg);};

#endif

  template <class T, bool (T::*M)(HWND hWndCursor, uint codeHitTest, TMsgId mouseMsg)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      BOOL operator ()(HWND, HWND hwndCursor, UINT codeHitTest, UINT msg)
      {return (i_->*M)(hwndCursor, codeHitTest, static_cast<TMsgId>(msg)) ? TRUE : FALSE;}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SETCURSOR(nullptr, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_SETICON is missing from "windowsx.h".
//
template <> struct TDispatch<WM_SETICON>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef HICON (T::*type)(bool isBigIcon, HICON);};

#endif

  template <class T, HICON (T::*M)(bool isBigIcon, HICON)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    HICON r = (static_cast<T*>(i)->*M)(p1 == ICON_BIG, reinterpret_cast<HICON>(p2));
    return reinterpret_cast<TResult>(r);
  }
};

template <> struct TDispatch<WM_SETFOCUS>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND hwndLostFocus);};

#endif

  template <class T, void (T::*M)(HWND hwndLostFocus)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND hWndLostFocus)
      {(i_->*M)(hWndLostFocus);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SETFOCUS(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SETFONT>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HFONT, bool redraw);};

#endif

  template <class T, void (T::*M)(HFONT, bool redraw)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HFONT hfont, BOOL fRedraw)
      {(i_->*M)(hfont, fRedraw);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SETFONT(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SETREDRAW>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(bool redraw);};

#endif

  template <class T, void (T::*M)(bool redraw)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL fRedraw)
      {(i_->*M)(!!fRedraw);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SETREDRAW(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SETTEXT>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(LPCTSTR);};

#endif

  template <class T, void (T::*M)(LPCTSTR)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, LPCTSTR lpszText)
      {(i_->*M)(lpszText);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SETTEXT(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_SETTINGCHANGE is missing from "windowsx.h".
//
template <> struct TDispatch<WM_SETTINGCHANGE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint flags, LPCTSTR section);};

#endif

  template <class T, void (T::*M)(uint flags, LPCTSTR section)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    (static_cast<T*>(i)->*M)(static_cast<uint>(p1), reinterpret_cast<LPCTSTR>(p2));
    return 0;
  }
};

template <> struct TDispatch<WM_SHOWWINDOW>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(bool, uint status);};

#endif

  template <class T, void (T::*M)(bool, uint status)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, BOOL fShow, UINT status)
      {(i_->*M)(fShow, status);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SHOWWINDOW(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SIZE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint, const TSize&);};

#endif

  template <class T, void (T::*M)(uint, const TSize&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, UINT sizeType, int cx, int cy)
      {(i_->*M)(sizeType, TSize(cx, cy));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SIZE(nullptr, p1, p2, forwarder);
  }
};

//
// Note that the OWLNext 6.32 handler signature was non-compliant (void (T::*)(HWND, HANDLE)).
// For this reason we do not provide a compatibility implementation here.
// See the Windows API documentation for details.
//
template <> struct TDispatch<WM_SIZECLIPBOARD>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND clipboardViewer, const TRect& client);};

#endif

  template <class T, void (T::*M)(HWND clipboardViewer, const TRect& client)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND hwndCBViewer, const LPRECT lprc)
      {
        PRECONDITION(lprc);
        if (lprc) (i_->*M)(hwndCBViewer, TRect(*lprc));
      }
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
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


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)(uint side, TRect&);};

#endif

  template <class T, bool (T::*M)(uint side, TRect&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    PRECONDITION(p2);
    LPRECT r = reinterpret_cast<LPRECT>(p2);
    if (!r) return 0;
    TRect clone(*r);
    bool result = (static_cast<T*>(i)->*M)(static_cast<uint>(p1), clone);
    *r = clone;
    return result ? TRUE : FALSE;
  }
};

template <> struct TDispatch<WM_SPOOLERSTATUS>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint status, int jobsLeft);};

#endif

  template <class T, void (T::*M)(uint status, int jobsLeft)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, UINT status, int cJobInQueue)
      {(i_->*M)(status, cJobInQueue);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SPOOLERSTATUS(0, p1, p2, forwarder);
  }
};

//
// HANDLE_WM_STYLECHANGED is missing from "windowsx.h".
//
template <> struct TDispatch<WM_STYLECHANGED>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(int styleType, const STYLESTRUCT& style);};

#endif

  template <class T, void (T::*M)(int styleType, const STYLESTRUCT& style)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    int styleType = static_cast<int>(p1);
    const STYLESTRUCT* lpStyleStruct = reinterpret_cast<LPSTYLESTRUCT>(p2);
    CHECK(lpStyleStruct);
    if (lpStyleStruct) (static_cast<T*>(i)->*M)(styleType, *lpStyleStruct);
    return 0;
  }
};

//
// HANDLE_WM_STYLECHANGING is missing from "windowsx.h".
//
template <> struct TDispatch<WM_STYLECHANGING>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(int styleType, STYLESTRUCT& style);};

#endif

  template <class T, void (T::*M)(int styleType, STYLESTRUCT& style)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    int styleType = static_cast<int>(p1);
    LPSTYLESTRUCT lpStyleStruct = reinterpret_cast<LPSTYLESTRUCT>(p2);
    CHECK(lpStyleStruct);
    if (lpStyleStruct) (static_cast<T*>(i)->*M)(styleType, *lpStyleStruct);
    return 0;
  }
};

//
// TODO: Remove the useless 'flags' parameter, change 'repeatCount' to int, and change the 'key' parameter to 'ch'.
//
template <> struct TDispatch<WM_SYSCHAR>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint key, uint repeatCount, uint flags);};

#endif

  template <class T, void (T::*M)(uint key, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, TCHAR ch, int cRepeat)
      {(i_->*M)(static_cast<uint>(ch), static_cast<uint>(cRepeat), 0);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SYSCHAR(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SYSCOLORCHANGE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SYSCOLORCHANGE(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SYSCOMMAND>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint cmd, const TPoint&);};

#endif

  template <class T, void (T::*M)(uint cmd, const TPoint&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, UINT cmd, int x, int y)
      {(i_->*M)(cmd, TPoint(x, y));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SYSCOMMAND(nullptr, p1, p2, forwarder);
  }
};

//
// TODO: Remove the useless 'flags' parameter, change 'repeatCount' to int, and change the 'key' parameter to 'ch'.
//
template <> struct TDispatch<WM_SYSDEADCHAR>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint ch, uint repeatCount, uint flags);};

#endif

  template <class T, void (T::*M)(uint ch, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, TCHAR ch, int cRepeat)
      {(i_->*M)(static_cast<uint>(ch), static_cast<uint>(cRepeat), 0);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SYSDEADCHAR(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SYSKEYDOWN>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint key, uint repeatCount, uint flags);};

#endif

  template <class T, void (T::*M)(uint key, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, UINT vk, BOOL, int cRepeat, UINT flags)
      {(i_->*M)(vk, static_cast<uint>(cRepeat), flags);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_SYSKEYDOWN(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_SYSKEYUP>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint key, uint repeatCount, uint flags);};

#endif

  template <class T, void (T::*M)(uint key, uint repeatCount, uint flags)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, UINT vk, BOOL, int cRepeat, UINT flags)
      {(i_->*M)(vk, static_cast<uint>(cRepeat), flags);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
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

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint idAction, ULONG_PTR data);};

#endif

  template <class T, void (T::*M)(uint idAction, ULONG_PTR data)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    uint idAction = static_cast<UINT>(p1);
    ULONG_PTR data = static_cast<ULONG_PTR>(p2);
    return (static_cast<T*>(i)->*M)(idAction, data), 0;
  }
};

//
// HANDLE_WM_THEMECHANGED is missing from "windowsx.h".
//
template <> struct TDispatch<WM_THEMECHANGED>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (static_cast<T*>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_TIMECHANGE>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_TIMECHANGE(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_TIMER>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint);};

#endif

  template <class T, void (T::*M)(uint)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, UINT id)
      {(i_->*M)(id);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_TIMER(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_UNDO>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)();};

#endif

  template <class T, bool (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      BOOL operator ()(HWND)
      {return (i_->*M)() ? TRUE : FALSE;}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_UNDO(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_USERCHANGED>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND)
      {(i_->*M)();}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_USERCHANGED(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_VKEYTOITEM>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef int (T::*type)(uint vk, HWND listBox, int caretIndex);};

#endif

  template <class T, int (T::*M)(uint vk, HWND listBox, int caretIndex)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      int operator ()(HWND, UINT vk, HWND hwndListbox, int iCaret)
      {return (i_->*M)(vk, hwndListbox, iCaret);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_VKEYTOITEM(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_VSCROLL>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint scrollCode, uint pos, HWND);};

#endif

  template <class T, void (T::*M)(uint scrollCode, uint pos, HWND)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND hwndCtl, UINT code, int pos)
      {(i_->*M)(code, static_cast<uint>(pos), hwndCtl);}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_VSCROLL(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_VSCROLLCLIPBOARD>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND cliboardViewer, uint scrollCode, uint pos);};

#endif

  template <class T, void (T::*M)(HWND cliboardViewer, uint scrollCode, uint pos)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, HWND hwndCBViewer, UINT code, int pos)
      {(i_->*M)(hwndCBViewer, code, static_cast<uint>(pos));}
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_VSCROLLCLIPBOARD(0, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_WINDOWPOSCHANGED>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(const WINDOWPOS&);};

#endif

  template <class T, void (T::*M)(const WINDOWPOS&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      void operator ()(HWND, const LPWINDOWPOS lpwpos)
      {
        PRECONDITION(lpwpos);
        if (lpwpos) (i_->*M)(*lpwpos);
      }
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_WINDOWPOSCHANGED(nullptr, p1, p2, forwarder);
  }
};

template <> struct TDispatch<WM_WINDOWPOSCHANGING>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)(WINDOWPOS&);};

#endif

  template <class T, bool (T::*M)(WINDOWPOS&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      BOOL operator ()(HWND, LPWINDOWPOS lpwpos)
      {
        PRECONDITION(lpwpos);
        return (lpwpos && (i_->*M)(*lpwpos)) ? TRUE : FALSE;
      }
    }
    forwarder = {static_cast<T*>(i)};
    InUse(p1); InUse(p2);
    return HANDLE_WM_WINDOWPOSCHANGING(nullptr, p1, p2, forwarder);
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
    void operator ()(HWND, LPCTSTR lpszSectionName)
    {(i_->*M)(lpszSectionName);}
  }
  forwarder = {static_cast<T*>(i)};
  InUse(p1); InUse(p2);
  return HANDLE_WM_WININICHANGE(0, p1, p2, forwarder);
}

#endif

//-------------------------------------------------------------------------------------------------
// Dialog message-specific dispatchers

template <> struct TDispatch<DM_GETDEFID>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef uint (T::*type)();};

#endif

  template <class T, uint (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {
    uint r = (static_cast<T*>(i)->*M)();
    return r ? static_cast<TResult>(MkUint32(r, DC_HASDEFID)) : 0; // Ensure flag is set if ID is returned.
  }
};

template <> struct TDispatch<DM_REPOSITION>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (static_cast<T*>(i)->*M)(), 0;}
};

template <> struct TDispatch<DM_SETDEFID>
{

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)(int);};

#endif

  template <class T, bool (T::*M)(int)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (static_cast<T*>(i)->*M)(static_cast<int>(p1)) ? TRUE : FALSE;}
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


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HWND);};

#endif

  template <class T, void (T::*M)(HWND)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (static_cast<T*>(i)->*M)(reinterpret_cast<HWND>(p1)), 0;}
};

template <> struct TDispatch<WM_COMMAND_ENABLE>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, TCommandEnabler& enabler)
  {sendMessage(wnd, WM_COMMAND_ENABLE, 0, reinterpret_cast<TParam2>(&enabler));}


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(TCommandEnabler&);};

#endif

  template <class T, void (T::*M)(TCommandEnabler&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 ? ((static_cast<T*>(i)->*M)(*reinterpret_cast<TCommandEnabler*>(p2)), 0) : 0;
  }
};

template <> struct TDispatch<WM_OWLCANCLOSE>
{
  template <class F>
  static bool Encode(F sendMessage, HWND wnd)
  {return sendMessage(wnd, WM_OWLCANCLOSE, 0, 0) == TRUE;}


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef bool (T::*type)();};

#endif

  template <class T, bool (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (static_cast<T*>(i)->*M)() ? TRUE : FALSE;}
};

template <> struct TDispatch<WM_OWLCREATETTIP>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd)
  {sendMessage(wnd, WM_OWLCREATETTIP, 0, 0);}


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (static_cast<T*>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_OWLFRAMESIZE>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, uint sizeType, const TSize& size)
  {sendMessage(wnd, WM_OWLFRAMESIZE, static_cast<TParam1>(sizeType), reinterpret_cast<TParam2>(&size));}


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint sizeType, const TSize&);};

#endif

  template <class T, void (T::*M)(uint sizeType, const TSize&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 ? ((static_cast<T*>(i)->*M)(static_cast<uint>(p1), *reinterpret_cast<TSize*>(p2)), 0) : 0;
  }
};

template <> struct TDispatch<WM_OWLHELPHIT>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, const THelpHitInfo& i)
  {sendMessage(wnd, WM_OWLHELPHIT, 0, reinterpret_cast<TParam2>(&i));}


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(const THelpHitInfo&);};

#endif

  template <class T, void (T::*M)(const THelpHitInfo&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 ? ((static_cast<T*>(i)->*M)(*reinterpret_cast<THelpHitInfo*>(p2)), 0) : 0;
  }
};

template <> struct TDispatch<WM_OWLPREPROCMENU>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, HMENU menu)
  {sendMessage(wnd, WM_OWLPREPROCMENU, reinterpret_cast<TParam1>(menu), 0);}


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(HMENU);};

#endif

  template <class T, void (T::*M)(HMENU)>
  static TResult Decode(void* i, TParam1 p1, TParam2)
  {return (static_cast<T*>(i)->*M)(reinterpret_cast<HMENU>(p1)), 0;}
};

template <> struct TDispatch<WM_OWLWAKEUP>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd)
  {sendMessage(wnd, WM_OWLWAKEUP, 0, 0);}


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)();};

#endif

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (static_cast<T*>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_OWLWINDOWDOCKED>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, uint pos, const TDockingSlip& d)
  {sendMessage(wnd, WM_OWLWINDOWDOCKED, static_cast<TParam1>(pos), reinterpret_cast<TParam2>(&d));}


#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef void (T::*type)(uint pos, const TDockingSlip&);};

#endif

  template <class T, void (T::*M)(uint pos, const TDockingSlip&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 ? ((static_cast<T*>(i)->*M)(static_cast<uint>(p1), *reinterpret_cast<const TDockingSlip*>(p2)), 0) : 0;
  }
};

} // OWL namespace

OWL_DISABLE_WARNING_POP

#endif
