//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// Definition of OWL signatures for ObjectComponents messages
//----------------------------------------------------------------------------

#if !defined(OCF_OCFEVENT_H)
#define OCF_OCFEVENT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/eventhan.h>
#include <ocf/ocapp.h>
#include <ocf/ocview.h>
#include <ocf/occtrl.h>

#if defined(OWL5_COMPAT)

namespace ocf {

//----------------------------------------------------------------------------
// Signatures for ObjectComponents messages
//
DECLARE_SIGNATURE1(bool,B_OC_MD_Sig,TOcMenuDescr &)
DECLARE_SIGNATURE1(bool,B_OC_DD_Sig,TOcDragDrop &)
DECLARE_SIGNATURE1(bool,B_OC_VSCROLL_Sig,TOcScrollDir)
DECLARE_SIGNATURE1(bool,B_OC_PCI_Sig,TOcPartChangeInfo&)
DECLARE_SIGNATURE1(bool,B_OC_VP_Sig,TOcViewPaint &)
DECLARE_SIGNATURE1(bool,B_OC_VSP_Sig,TOcSaveLoad &)
DECLARE_SIGNATURE1(bool,B_OC_VCF_Sig,TOcFormatData &)
DECLARE_SIGNATURE1(bool,B_OC_VTB_Sig,TOcToolBarInfo &)
DECLARE_SIGNATURE1(bool,B_OC_VSS_Sig,TOcScaleFactor&)
DECLARE_SIGNATURE1(bool,B_OC_PS_Sig,TOcPartSize &)
DECLARE_SIGNATURE1(bool,B_OC_IT_Sig,TOcItemName&)
DECLARE_SIGNATURE1(bool,B_OC_SL_Sig,TOcLinkView&)
DECLARE_SIGNATURE1(bool,B_OC_PA_Sig,TOcPart&)
DECLARE_SIGNATURE1(bool,B_OC_PO_Sig,TOcInitInfo&)
DECLARE_SIGNATURE1(bool,B_OC_PI32_Sig,owl::int32*)
DECLARE_SIGNATURE1(bool,B_OC_PI16_Sig,owl::int16*)
DECLARE_SIGNATURE1(bool,B_OC_I16_Sig,owl::int16)
DECLARE_SIGNATURE1(bool,B_OC_PB_Sig,bool*)
DECLARE_SIGNATURE1(bool,B_OC_PPTS_Sig,owl::TString**)
DECLARE_SIGNATURE1(bool,B_OC_PPID_Sig,IDispatch **)
DECLARE_SIGNATURE1(bool,B_OC_PTS_Sig,owl::TString*)
DECLARE_SIGNATURE1(bool,B_OC_PID_Sig,IDispatch*)
DECLARE_SIGNATURE1(bool,B_U_Sig,owl::uint)

// Signature templates used to validate OCX-events handlers
//
DECLARE_SIGNATURE1(bool,B_OC_PE_Sig,TCtrlEvent*)
DECLARE_SIGNATURE1(bool,B_OC_PME_Sig,TCtrlMouseEvent*)
DECLARE_SIGNATURE1(bool,B_OC_PKE_Sig,TCtrlKeyEvent*)
DECLARE_SIGNATURE1(bool,B_OC_PFE_Sig,TCtrlFocusEvent*)
DECLARE_SIGNATURE1(bool,B_OC_PPE_Sig,TCtrlPropertyEvent*)
DECLARE_SIGNATURE1(bool,B_OC_PCE_Sig,TCtrlCustomEvent*)
DECLARE_SIGNATURE1(bool,B_OC_PEE_Sig,TCtrlErrorEvent*)

} // OCF namespace

#endif

//----------------------------------------------------------------------------
// Dispatchers for the WM_OCEVENT message and individual events
//

namespace owl
{

//
// Dispatch specialization for the WM_OCEVENT message
//
template <>
struct TDispatch<WM_OCEVENT>
{
  //
  // Raw message dispatch
  // Takes the event as a 'uint' and the event argument as a TParam2. Returns a TResult.
  //
  template <class F>
  static TResult Encode(F sendMessage, HWND wnd, uint eventId, TParam2 p2)
  {return sendMessage(wnd, WM_OCEVENT, eventId, p2);}

  template <class T, TResult (T::*M)(uint eventId, TParam2)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {return (static_cast<T*>(i)->*M)(static_cast<uint>(p1), p2);}

  //
  // Base class template for WM_OCEVENT events
  // Event handlers take a single parameter of the given type and return a boolean result.
  //
  template <owl::uint NotificationCode, typename TParamType>
  struct TNotificationDispatchBase
  {
    template <class F>
    static bool Encode(F sendMessage, HWND wnd, TParamType p)
    {return sendMessage(wnd, WM_OCEVENT, NotificationCode, static_cast<TParam2>(p)) == TRUE;}

    template <class T, bool (T::*M)(TParamType p)>
    static TResult Decode(void* i, TParam1, TParam2 p2)
    {return (static_cast<T*>(i)->*M)(static_cast<TParamType>(p2)) ? TRUE : FALSE;}
  };

  //
  // Specialization for pointer parameter
  //
  template <owl::uint NotificationCode, typename TParamType>
  struct TNotificationDispatchBase<NotificationCode, TParamType*>
  {
    template <class F>
    static bool Encode(F sendMessage, HWND wnd, TParamType* p)
    {return sendMessage(wnd, WM_OCEVENT, NotificationCode, reinterpret_cast<TParam2>(p)) == TRUE;}

    template <class T, bool (T::*M)(TParamType* p)>
    static TResult Decode(void* i, TParam1, TParam2 p2)
    {return (static_cast<T*>(i)->*M)(reinterpret_cast<TParamType*>(p2)) ? TRUE : FALSE;}
  };

  //
  // Specialization for reference parameter
  // Guards against creating null-references. If a null pointer is passed in `TParam2`, then the
  // decoder does not forward the message, but instead returns FALSE.
  //
  template <owl::uint NotificationCode, typename TParamType>
  struct TNotificationDispatchBase<NotificationCode, TParamType&>
  {
    template <class F>
    static bool Encode(F sendMessage, HWND wnd, TParamType& p)
    {return sendMessage(wnd, WM_OCEVENT, NotificationCode, reinterpret_cast<TParam2>(&p)) == TRUE;}

    template <class T, bool (T::*M)(TParamType& p)>
    static TResult Decode(void* i, TParam1, TParam2 p2)
    {
      PRECONDITION(p2);
      return (p2 != 0 && (static_cast<T*>(i)->*M)(*reinterpret_cast<TParamType*>(p2))) ? TRUE : FALSE;
    }
  };

  //
  // Specialization for no (void) parameter
  //
  template <owl::uint NotificationCode>
  struct TNotificationDispatchBase<NotificationCode, void>
  {
    template <class F>
    static bool Encode(F sendMessage, HWND wnd)
    {return sendMessage(wnd, WM_OCEVENT, NotificationCode, 0) == TRUE;}

    template <class T, bool (T::*M)()>
    static TResult Decode(void* i, TParam1, TParam2)
    {return (static_cast<T*>(i)->*M)() ? TRUE : FALSE;}
  };

  //
  // Dispatch template for WM_OCEVENT events
  //
  template <owl::uint NotificationCode>
  struct TNotificationDispatch;

};

template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_APPINSMENUS> : TNotificationDispatchBase<OC_APPINSMENUS, ocf::TOcMenuDescr&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_APPMENUS> : TNotificationDispatchBase<OC_APPMENUS, ocf::TOcMenuDescr&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_APPPROCESSMSG> : TNotificationDispatchBase<OC_APPPROCESSMSG, MSG*> {}; // B_MSG_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_APPFRAMERECT> : TNotificationDispatchBase<OC_APPMENUS, TRect*> {}; // B_RECT_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_APPBORDERSPACEREQ> : TNotificationDispatchBase<OC_APPMENUS, TRect*> {}; // B_RECT_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_APPBORDERSPACESET> : TNotificationDispatchBase<OC_APPMENUS, TRect*> {}; // B_RECT_Sig

template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_APPSTATUSTEXT> // v_CS_Sig
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, LPCTSTR statusText)
  {sendMessage(wnd, WM_OCEVENT, OC_APPSTATUSTEXT, reinterpret_cast<TParam2>(statusText));}

  template <class T, void (T::*M)(LPCTSTR statusText)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {return (static_cast<T*>(i)->*M)(reinterpret_cast<LPCTSTR>(p2)), 0;}
};

template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_APPRESTOREUI> // v_Sig
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd)
  {sendMessage(wnd, WM_OCEVENT, OC_APPRESTOREUI, 0);}

  template <class T, void (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return (static_cast<T*>(i)->*M)(), 0;}
};

template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_APPDIALOGHELP>
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, ocf::TOcDialogHelp& p)
  {sendMessage(wnd, WM_OCEVENT, OC_APPDIALOGHELP, reinterpret_cast<TParam2>(&p));}

  template <class T, void (T::*M)(ocf::TOcDialogHelp& p)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    PRECONDITION(p2);
    return p2 != 0 ? ((static_cast<T*>(i)->*M)(*reinterpret_cast<ocf::TOcDialogHelp*>(p2)), 0) : 0;
  }
};

template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_APPSHUTDOWN> // B_Sig
: TNotificationDispatchBase<OC_APPSHUTDOWN, void>
{};

template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWTITLE> // CS_Sig
{
  template <class F>
  static LPCTSTR Encode(F sendMessage, HWND wnd)
  {return reinterpret_cast<LPCTSTR>(sendMessage(wnd, WM_OCEVENT, OC_VIEWTITLE, 0));}

  template <class T, LPCTSTR (T::*M)()>
  static TResult Decode(void* i, TParam1, TParam2)
  {return reinterpret_cast<TResult>((static_cast<T*>(i)->*M)());}
};

template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWSETTITLE> // v_CS_Sig
{
  template <class F>
  static void Encode(F sendMessage, HWND wnd, LPCTSTR title)
  {sendMessage(wnd, WM_OCEVENT, OC_VIEWSETTITLE, reinterpret_cast<TParam2>(title));}

  template <class T, void (T::*M)(LPCTSTR title)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {return (static_cast<T*>(i)->*M)(reinterpret_cast<LPCTSTR>(p2)), 0;}
};

template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWBORDERSPACEREQ> : TNotificationDispatchBase<OC_VIEWBORDERSPACEREQ, TRect*> {}; // B_RECT_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWBORDERSPACESET> : TNotificationDispatchBase<OC_VIEWBORDERSPACESET, TRect*> {}; // B_RECT_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWDROP> : TNotificationDispatchBase<OC_VIEWDROP, ocf::TOcDragDrop&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWDRAG> : TNotificationDispatchBase<OC_VIEWDRAG, ocf::TOcDragDrop&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWSCROLL> : TNotificationDispatchBase<OC_VIEWSCROLL, ocf::TOcScrollDir> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWPARTINVALID> : TNotificationDispatchBase<OC_VIEWPARTINVALID, ocf::TOcPartChangeInfo&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWPAINT> : TNotificationDispatchBase<OC_VIEWPAINT, ocf::TOcViewPaint&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWLOADPART> : TNotificationDispatchBase<OC_VIEWLOADPART, ocf::TOcSaveLoad&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWSAVEPART> : TNotificationDispatchBase<OC_VIEWSAVEPART, ocf::TOcSaveLoad&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWCLOSE> : TNotificationDispatchBase<OC_VIEWCLOSE, void> {}; // B_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWINSMENUS> : TNotificationDispatchBase<OC_VIEWINSMENUS, ocf::TOcMenuDescr&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWSHOWTOOLS> : TNotificationDispatchBase<OC_VIEWSHOWTOOLS, ocf::TOcToolBarInfo&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWGETPALETTE> : TNotificationDispatchBase<OC_VIEWGETPALETTE, LOGPALETTE**> {}; // B_PPALETTE_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWCLIPDATA> : TNotificationDispatchBase<OC_VIEWCLIPDATA, ocf::TOcFormatData&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWSETDATA> : TNotificationDispatchBase<OC_VIEWSETDATA, ocf::TOcFormatData&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWPARTSIZE> : TNotificationDispatchBase<OC_VIEWPARTSIZE, ocf::TOcPartSize&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWOPENDOC> : TNotificationDispatchBase<OC_VIEWOPENDOC, LPCTSTR> {}; // B_CS_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWATTACHWINDOW> : TNotificationDispatchBase<OC_VIEWATTACHWINDOW, bool> {}; // B_B_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWSETSCALE> : TNotificationDispatchBase<OC_VIEWSETSCALE, ocf::TOcScaleFactor&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWGETSCALE> : TNotificationDispatchBase<OC_VIEWGETSCALE, ocf::TOcScaleFactor&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWGETSITERECT> : TNotificationDispatchBase<OC_VIEWGETSITERECT, TRect*> {}; // B_RECT_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWSETSITERECT> : TNotificationDispatchBase<OC_VIEWSETSITERECT, TRect*> {}; // B_RECT_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWGETITEMNAME> : TNotificationDispatchBase<OC_VIEWGETITEMNAME, ocf::TOcItemName&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWSETLINK> : TNotificationDispatchBase<OC_VIEWSETLINK, ocf::TOcLinkView&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWBREAKLINK> : TNotificationDispatchBase<OC_VIEWBREAKLINK, ocf::TOcLinkView&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWPARTACTIVATE> : TNotificationDispatchBase<OC_VIEWPARTACTIVATE, ocf::TOcPart&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWPASTEOBJECT> : TNotificationDispatchBase<OC_VIEWPASTEOBJECT, ocf::TOcInitInfo&> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_VIEWDOVERB> : TNotificationDispatchBase<OC_VIEWDOVERB, uint> {}; // B_U_Sig

template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETBACKCOLOR> : TNotificationDispatchBase<OC_AMBIENT_GETBACKCOLOR, int32*> {}; // B_OC_PI32_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETFORECOLOR> : TNotificationDispatchBase<OC_AMBIENT_GETFORECOLOR, int32*> {}; // B_OC_PI32_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETLOCALEID> : TNotificationDispatchBase<OC_AMBIENT_GETLOCALEID, int32*> {}; // B_OC_PI32_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETTEXTALIGN> : TNotificationDispatchBase<OC_AMBIENT_GETTEXTALIGN, int16*> {}; // B_OC_PI16_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETMESSAGEREFLECT> : TNotificationDispatchBase<OC_AMBIENT_GETMESSAGEREFLECT, bool*> {}; // B_OC_PB_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETUSERMODE> : TNotificationDispatchBase<OC_AMBIENT_GETUSERMODE, bool*> {}; // B_OC_PB_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETUIDEAD> : TNotificationDispatchBase<OC_AMBIENT_GETUIDEAD, bool*> {}; // B_OC_PB_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETSHOWGRABHANDLES> : TNotificationDispatchBase<OC_AMBIENT_GETSHOWGRABHANDLES, bool*> {}; // B_OC_PB_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETSHOWHATCHING> : TNotificationDispatchBase<OC_AMBIENT_GETSHOWHATCHING, bool*> {}; // B_OC_PB_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETSUPPORTSMNEMONICS> : TNotificationDispatchBase<OC_AMBIENT_GETSUPPORTSMNEMONICS, bool*> {}; // B_OC_PB_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETDISPLAYASDEFAULT> : TNotificationDispatchBase<OC_AMBIENT_GETDISPLAYASDEFAULT, bool*> {}; // B_OC_PB_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETDISPLAYNAME> : TNotificationDispatchBase<OC_AMBIENT_GETDISPLAYNAME, TString**> {}; // B_OC_PPTS_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETSCALEUNITS> : TNotificationDispatchBase<OC_AMBIENT_GETSCALEUNITS, TString**> {}; // B_OC_PPTS_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_GETFONT> : TNotificationDispatchBase<OC_AMBIENT_GETFONT, IDispatch**> {}; // B_OC_PPID_Sig

template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETBACKCOLOR> : TNotificationDispatchBase<OC_AMBIENT_SETBACKCOLOR, int32> {}; // B_I32_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETFORECOLOR> : TNotificationDispatchBase<OC_AMBIENT_SETFORECOLOR, int32> {}; // B_I32_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETLOCALEID> : TNotificationDispatchBase<OC_AMBIENT_SETLOCALEID, int32> {}; // B_I32_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETTEXTALIGN> : TNotificationDispatchBase<OC_AMBIENT_SETTEXTALIGN, int16> {}; // B_OC_I16_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETMESSAGEREFLECT> : TNotificationDispatchBase<OC_AMBIENT_SETMESSAGEREFLECT, bool> {}; // B_B_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETUSERMODE> : TNotificationDispatchBase<OC_AMBIENT_SETUSERMODE, bool> {}; // B_B_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETUIDEAD> : TNotificationDispatchBase<OC_AMBIENT_SETUIDEAD, bool> {}; // B_B_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETSHOWGRABHANDLES> : TNotificationDispatchBase<OC_AMBIENT_SETSHOWGRABHANDLES, bool> {}; // B_B_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETSHOWHATCHING> : TNotificationDispatchBase<OC_AMBIENT_SETSHOWHATCHING, bool> {}; // B_B_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETDISPLAYASDEFAULT> : TNotificationDispatchBase<OC_AMBIENT_SETDISPLAYASDEFAULT, bool> {}; // B_B_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETSUPPORTSMNEMONICS> : TNotificationDispatchBase<OC_AMBIENT_SETSUPPORTSMNEMONICS, bool> {}; // B_B_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETDISPLAYNAME> : TNotificationDispatchBase<OC_AMBIENT_SETDISPLAYNAME, TString*> {}; // B_OC_PTS_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETSCALEUNITS> : TNotificationDispatchBase<OC_AMBIENT_SETSCALEUNITS, TString*> {}; // B_OC_PTS_Sig
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_AMBIENT_SETFONT> : TNotificationDispatchBase<OC_AMBIENT_SETFONT, IDispatch*> {}; // B_OC_PID_Sig

template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_CTRLEVENT_CLICK> : TNotificationDispatchBase<OC_CTRLEVENT_CLICK, ocf::TCtrlEvent*> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_CTRLEVENT_DBLCLICK> : TNotificationDispatchBase<OC_CTRLEVENT_DBLCLICK, ocf::TCtrlEvent*> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_CTRLEVENT_MOUSEDOWN> : TNotificationDispatchBase<OC_CTRLEVENT_MOUSEDOWN, ocf::TCtrlMouseEvent*> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_CTRLEVENT_MOUSEUP> : TNotificationDispatchBase<OC_CTRLEVENT_MOUSEUP, ocf::TCtrlMouseEvent*> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_CTRLEVENT_MOUSEMOVE> : TNotificationDispatchBase<OC_CTRLEVENT_MOUSEMOVE, ocf::TCtrlMouseEvent*> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_CTRLEVENT_KEYDOWN> : TNotificationDispatchBase<OC_CTRLEVENT_KEYDOWN, ocf::TCtrlKeyEvent*> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_CTRLEVENT_KEYUP> : TNotificationDispatchBase<OC_CTRLEVENT_KEYUP, ocf::TCtrlKeyEvent*> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_CTRLEVENT_ERROREVENT> : TNotificationDispatchBase<OC_CTRLEVENT_ERROREVENT, ocf::TCtrlErrorEvent*> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_CTRLEVENT_FOCUS> : TNotificationDispatchBase<OC_CTRLEVENT_FOCUS, ocf::TCtrlFocusEvent*> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_CTRLEVENT_PROPERTYCHANGE> : TNotificationDispatchBase<OC_CTRLEVENT_PROPERTYCHANGE, ocf::TCtrlPropertyEvent*> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_CTRLEVENT_PROPERTYREQUESTEDIT> : TNotificationDispatchBase<OC_CTRLEVENT_PROPERTYREQUESTEDIT, ocf::TCtrlPropertyEvent*> {};
template <> struct TDispatch<WM_OCEVENT>::TNotificationDispatch<OC_CTRLEVENT_CUSTOMEVENT> : TNotificationDispatchBase<OC_CTRLEVENT_CUSTOMEVENT, ocf::TCtrlCustomEvent*> {};

} // namespace

//----------------------------------------------------------------------------
// Event response table macros for ObjectConnections events
//

//
// Internal response table entry macro for OCF events
// Looks up the dispatcher (decoder) in owl::TDispatch<WM_OCEVENT>::TNotificationDispatch.
//
#define OCF_EV_(notificationCode, method)\
  {{WM_OCEVENT}, notificationCode,\
    OWL_DISPATCH(::owl::TDispatch<WM_OCEVENT>::TNotificationDispatch<notificationCode>::Decode, method)}

//
// Raw OC compound document family event
//
#define EV_OCEVENT(id, method) OWL_ID_EV_(WM_OCEVENT, id, method)

//
// App and app frame window events
//
#define EV_OC_APPINSMENUS OCF_EV_(OC_APPINSMENUS, EvOcAppInsMenus) // B_OC_MD_Sig
#define EV_OC_APPMENUS OCF_EV_(OC_APPMENUS, EvOcAppMenus) // B_OC_MD_Sig
#define EV_OC_APPPROCESSMSG OCF_EV_(OC_APPPROCESSMSG, EvOcAppProcessMsg) // B_MSG_Sig
#define EV_OC_APPFRAMERECT OCF_EV_(OC_APPFRAMERECT, EvOcAppFrameRect) // B_RECT_Sig
#define EV_OC_APPBORDERSPACEREQ OCF_EV_(OC_APPBORDERSPACEREQ, EvOcAppBorderSpaceReq) // B_RECT_Sig
#define EV_OC_APPBORDERSPACESET OCF_EV_(OC_APPBORDERSPACESET, EvOcAppBorderSpaceSet) // B_RECT_Sig
#define EV_OC_APPSTATUSTEXT OCF_EV_(OC_APPSTATUSTEXT, EvOcAppStatusText) // v_CS_Sig
#define EV_OC_APPRESTOREUI OCF_EV_(OC_APPRESTOREUI, EvOcAppRestoreUI) // v_Sig
#define EV_OC_APPDIALOGHELP OCF_EV_(OC_APPDIALOGHELP, EvOcAppDialogHelp) // v_OC_AH_Sig
#define EV_OC_APPSHUTDOWN OCF_EV_(OC_APPSHUTDOWN, EvOcAppShutdown) // B_Sig

//
// View events
//
#define EV_OC_VIEWTITLE OCF_EV_(OC_VIEWTITLE, EvOcViewTitle) // CS_Sig
#define EV_OC_VIEWSETTITLE OCF_EV_(OC_VIEWSETTITLE, EvOcViewSetTitle) // v_CS_Sig
#define EV_OC_VIEWBORDERSPACEREQ OCF_EV_(OC_VIEWBORDERSPACEREQ, EvOcViewBorderSpaceReq) // B_RECT_Sig
#define EV_OC_VIEWBORDERSPACESET OCF_EV_(OC_VIEWBORDERSPACESET, EvOcViewBorderSpaceSet) // B_RECT_Sig
#define EV_OC_VIEWDROP OCF_EV_(OC_VIEWDROP, EvOcViewDrop) // B_OC_DD_Sig
#define EV_OC_VIEWDRAG OCF_EV_(OC_VIEWDRAG, EvOcViewDrag) // B_OC_DD_Sig
#define EV_OC_VIEWSCROLL OCF_EV_(OC_VIEWSCROLL, EvOcViewScroll) // B_OC_VSCROLL_Sig
#define EV_OC_VIEWPARTINVALID OCF_EV_(OC_VIEWPARTINVALID, EvOcViewPartInvalid) // B_OC_PCI_Sig
#define EV_OC_VIEWPAINT OCF_EV_(OC_VIEWPAINT, EvOcViewPaint) // B_OC_VP_Sig
#define EV_OC_VIEWLOADPART OCF_EV_(OC_VIEWLOADPART, EvOcViewLoadPart) // B_OC_VSP_Sig
#define EV_OC_VIEWSAVEPART OCF_EV_(OC_VIEWSAVEPART, EvOcViewSavePart) // B_OC_VSP_Sig
#define EV_OC_VIEWCLOSE OCF_EV_(OC_VIEWCLOSE, EvOcViewClose) // B_Sig
#define EV_OC_VIEWINSMENUS OCF_EV_(OC_VIEWINSMENUS, EvOcViewInsMenus) // B_OC_MD_Sig
#define EV_OC_VIEWSHOWTOOLS OCF_EV_(OC_VIEWSHOWTOOLS, EvOcViewShowTools) // B_OC_VTB_Sig
#define EV_OC_VIEWGETPALETTE OCF_EV_(OC_VIEWGETPALETTE, EvOcViewGetPalette) // B_PPALETTE_Sig
#define EV_OC_VIEWCLIPDATA OCF_EV_(OC_VIEWCLIPDATA, EvOcViewClipData) // B_OC_VCF_Sig
#define EV_OC_VIEWSETDATA OCF_EV_(OC_VIEWSETDATA, EvOcViewSetData) // B_OC_VCF_Sig
#define EV_OC_VIEWPARTSIZE OCF_EV_(OC_VIEWPARTSIZE, EvOcViewPartSize) // B_OC_PS_Sig
#define EV_OC_VIEWOPENDOC OCF_EV_(OC_VIEWOPENDOC, EvOcViewOpenDoc) // B_CS_Sig
#define EV_OC_VIEWATTACHWINDOW OCF_EV_(OC_VIEWATTACHWINDOW, EvOcViewAttachWindow) // B_B_Sig
#define EV_OC_VIEWSETSCALE OCF_EV_(OC_VIEWSETSCALE, EvOcViewSetScale) // B_OC_VSS_Sig
#define EV_OC_VIEWGETSCALE OCF_EV_(OC_VIEWGETSCALE, EvOcViewGetScale) // B_OC_VSS_Sig
#define EV_OC_VIEWGETSITERECT OCF_EV_(OC_VIEWGETSITERECT, EvOcViewGetSiteRect) // B_RECT_Sig
#define EV_OC_VIEWSETSITERECT OCF_EV_(OC_VIEWSETSITERECT, EvOcViewSetSiteRect) // B_RECT_Sig
#define EV_OC_VIEWGETITEMNAME OCF_EV_(OC_VIEWGETITEMNAME, EvOcViewGetItemName) // B_OC_IT_Sig
#define EV_OC_VIEWSETLINK OCF_EV_(OC_VIEWSETLINK, EvOcViewSetLink) // B_OC_SL_Sig
#define EV_OC_VIEWBREAKLINK OCF_EV_(OC_VIEWBREAKLINK, EvOcViewBreakLink) // B_OC_SL_Sig
#define EV_OC_VIEWPARTACTIVATE OCF_EV_(OC_VIEWPARTACTIVATE, EvOcViewPartActivate) // B_OC_PA_Sig
#define EV_OC_VIEWPASTEOBJECT OCF_EV_(OC_VIEWPASTEOBJECT, EvOcViewPasteObject) // B_OC_PO_Sig
#define EV_OC_VIEWDOVERB OCF_EV_(OC_VIEWDOVERB, EvOcViewDoVerb) // B_U_Sig

//
// OCX container
//

//
// Standard Ambient properties - query
//
#define EV_OC_AMBIENT_GETBACKCOLOR OCF_EV_(OC_AMBIENT_GETBACKCOLOR, EvOcAmbientGetBackColor) // B_OC_PI32_Sig
#define EV_OC_AMBIENT_GETFORECOLOR OCF_EV_(OC_AMBIENT_GETFORECOLOR, EvOcAmbientGetForeColor) // B_OC_PI32_Sig
#define EV_OC_AMBIENT_GETLOCALEID OCF_EV_(OC_AMBIENT_GETLOCALEID, EvOcAmbientGetLocaleID) // B_OC_PI32_Sig
#define EV_OC_AMBIENT_GETTEXTALIGN OCF_EV_(OC_AMBIENT_GETTEXTALIGN, EvOcAmbientGetTextAlign) // B_OC_PI16_Sig
#define EV_OC_AMBIENT_GETMESSAGEREFLECT OCF_EV_(OC_AMBIENT_GETMESSAGEREFLECT, EvOcAmbientGetMessageReflect) // B_OC_PB_Sig
#define EV_OC_AMBIENT_GETUSERMODE OCF_EV_(OC_AMBIENT_GETUSERMODE, EvOcAmbientGetUserMode) // B_OC_PB_Sig
#define EV_OC_AMBIENT_GETUIDEAD OCF_EV_(OC_AMBIENT_GETUIDEAD, EvOcAmbientGetUIDead) // B_OC_PB_Sig
#define EV_OC_AMBIENT_GETSHOWGRABHANDLES OCF_EV_(OC_AMBIENT_GETSHOWGRABHANDLES, EvOcAmbientGetShowGrabHandles) // B_OC_PB_Sig
#define EV_OC_AMBIENT_GETSHOWHATCHING OCF_EV_(OC_AMBIENT_GETSHOWHATCHING, EvOcAmbientGetShowHatching) // B_OC_PB_Sig
#define EV_OC_AMBIENT_GETSUPPORTSMNEMONICS OCF_EV_(OC_AMBIENT_GETSUPPORTSMNEMONICS, EvOcAmbientGetSupportsMnemonics) // B_OC_PB_Sig
#define EV_OC_AMBIENT_GETDISPLAYASDEFAULT OCF_EV_(OC_AMBIENT_GETDISPLAYASDEFAULT, EvOcAmbientGetDisplayAsDefault) // B_OC_PB_Sig
#define EV_OC_AMBIENT_GETDISPLAYNAME OCF_EV_(OC_AMBIENT_GETDISPLAYNAME, EvOcAmbientGetDisplayName) // B_OC_PPTS_Sig
#define EV_OC_AMBIENT_GETSCALEUNITS OCF_EV_(OC_AMBIENT_GETSCALEUNITS, EvOcAmbientGetScaleUnits) // B_OC_PPTS_Sig
#define EV_OC_AMBIENT_GETFONT OCF_EV_(OC_AMBIENT_GETFONT, EvOcAmbientGetFont) // B_OC_PPID_Sig

//
// Standard Ambient properties - mutation
//
#define EV_OC_AMBIENT_SETBACKCOLOR OCF_EV_(OC_AMBIENT_SETBACKCOLOR, EvOcAmbientSetBackColor) // B_I32_Sig
#define EV_OC_AMBIENT_SETFORECOLOR OCF_EV_(OC_AMBIENT_SETFORECOLOR, EvOcAmbientSetForeColor) // B_I32_Sig
#define EV_OC_AMBIENT_SETLOCALEID OCF_EV_(OC_AMBIENT_SETLOCALEID, EvOcAmbientSetLocaleID) // B_I32_Sig
#define EV_OC_AMBIENT_SETTEXTALIGN OCF_EV_(OC_AMBIENT_SETTEXTALIGN, EvOcAmbientSetTextAlign) // B_OC_I16_Sig
#define EV_OC_AMBIENT_SETMESSAGEREFLECT OCF_EV_(OC_AMBIENT_SETMESSAGEREFLECT, EvOcAmbientSetMessageReflect) // B_B_Sig
#define EV_OC_AMBIENT_SETUSERMODE OCF_EV_(OC_AMBIENT_SETUSERMODE, EvOcAmbientSetUserMode) // B_B_Sig
#define EV_OC_AMBIENT_SETUIDEAD OCF_EV_(OC_AMBIENT_SETUIDEAD, EvOcAmbientSetUIDead) // B_B_Sig
#define EV_OC_AMBIENT_SETSHOWGRABHANDLES OCF_EV_(OC_AMBIENT_SETSHOWGRABHANDLES, EvOcAmbientSetShowGrabHandles) // B_B_Sig
#define EV_OC_AMBIENT_SETSHOWHATCHING OCF_EV_(OC_AMBIENT_SETSHOWHATCHING, EvOcAmbientSetShowHatching) // B_B_Sig
#define EV_OC_AMBIENT_SETDISPLAYASDEFAULT OCF_EV_(OC_AMBIENT_SETDISPLAYASDEFAULT, EvOcAmbientSetDisplayAsDefault) // B_B_Sig
#define EV_OC_AMBIENT_SETSUPPORTSMNEMONICS OCF_EV_(OC_AMBIENT_SETSUPPORTSMNEMONICS, EvOcAmbientSetSupportsMnemonics) // B_B_Sig
#define EV_OC_AMBIENT_SETDISPLAYNAME OCF_EV_(OC_AMBIENT_SETDISPLAYNAME, EvOcAmbientSetDisplayName) // B_OC_PTS_Sig
#define EV_OC_AMBIENT_SETSCALEUNITS OCF_EV_(OC_AMBIENT_SETSCALEUNITS, EvOcAmbientSetScaleUnits) // B_OC_PTS_Sig
#define EV_OC_AMBIENT_SETFONT OCF_EV_(OC_AMBIENT_SETFONT, EvOcAmbientSetFont) // B_OC_PID_Sig

//
// Standard control events
//
#define EV_OC_CTRLEVENT_CLICK OCF_EV_(OC_CTRLEVENT_CLICK, EvOcCtrlClick) // B_OC_PE_Sig
#define EV_OC_CTRLEVENT_DBLCLICK OCF_EV_(OC_CTRLEVENT_DBLCLICK, EvOcCtrlDblClick) // B_OC_PE_Sig
#define EV_OC_CTRLEVENT_MOUSEDOWN OCF_EV_(OC_CTRLEVENT_MOUSEDOWN, EvOcCtrlMouseDown) // B_OC_PME_Sig
#define EV_OC_CTRLEVENT_MOUSEUP OCF_EV_(OC_CTRLEVENT_MOUSEUP, EvOcCtrlMouseUp) // B_OC_PME_Sig
#define EV_OC_CTRLEVENT_MOUSEMOVE OCF_EV_(OC_CTRLEVENT_MOUSEMOVE, EvOcCtrlMouseMove) // B_OC_PME_Sig
#define EV_OC_CTRLEVENT_KEYDOWN OCF_EV_(OC_CTRLEVENT_KEYDOWN, EvOcCtrlKeyDown) // B_OC_PKE_Sig
#define EV_OC_CTRLEVENT_KEYUP OCF_EV_(OC_CTRLEVENT_KEYUP, EvOcCtrlKeyUp) // B_OC_PKE_Sig
#define EV_OC_CTRLEVENT_ERROREVENT OCF_EV_(OC_CTRLEVENT_ERROREVENT, EvOcCtrlErrorEvent) // B_OC_PEE_Sig
#define EV_OC_CTRLEVENT_FOCUS OCF_EV_(OC_CTRLEVENT_FOCUS, EvOcCtrlFocus) // B_OC_PFE_Sig
#define EV_OC_CTRLEVENT_PROPERTYCHANGE OCF_EV_(OC_CTRLEVENT_PROPERTYCHANGE, EvOcCtrlPropertyChange) // B_OC_PPE_Sig
#define EV_OC_CTRLEVENT_PROPERTYREQUESTEDIT OCF_EV_(OC_CTRLEVENT_PROPERTYREQUESTEDIT, EvOcCtrlPropertyRequestEdit) // B_OC_PPE_Sig
#define EV_OC_CTRLEVENT_CUSTOMEVENT OCF_EV_(OC_CTRLEVENT_CUSTOMEVENT, EvOcCtrlCustomEvent) // B_OC_PCE_Sig

#endif    //  OCF_OCFEVENT_H
