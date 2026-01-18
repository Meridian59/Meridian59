//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
///  Definition of TOcControl class - Proxy object representing a control in
//                                   an OCF container.
//----------------------------------------------------------------------------

#if !defined(OCF_OCCTRL_H)
#define OCF_OCCTRL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/ocview.h>
#include <ocf/ocpart.h>
#include <ocf/automacr.h>




// Check build target!
//

interface _ICLASS ITypeInfo;

namespace ocf {

class _ICLASS TOcControlEvent;
class TAutoStack;

//
//
//
class _OCFCLASS TEventEntry {
  public:
    TEventEntry();
   ~TEventEntry();

    void      Init(int index, ITypeInfo* typeInfo);
    void      Cleanup();

  protected:
    DISPID    IDOfEvent;
    owl::uint      ParamCount;
    BSTR*     NameAndParams;
};

//
//
//
class _OCFCLASS TEventList {
  public:
    TEventList(ITypeInfo* typeInfo);
   ~TEventList();
    TEventEntry&  operator[](int index);

    owl::uint          GetCount() const    { return Count;    }
    const GUID&   GetEventIID() const { return EventIID; }
  protected:
    TEventEntry*  EventList;
    owl::uint          Count;
    GUID          EventIID;
};


//
/// \class TOcControl
// ~~~~~ ~~~~~~~~~~
//
/// Class representing an embedded control within a document
///
/// \todo Research this, as it is not true:
/// change from protected IBControlSite to public IBControlSite -> c++ syntax
/// in DECLARE_AUTOCLASS(TOcControl) exist conversion to IDispatch
class _ICLASS TOcControl : public TOcPart, protected IBControlSite {
  public:
    TOcControl(TOcDocument& document, int id=0, TOcControlEvent* pEv=nullptr);
    TOcControl(TOcDocument& document, LPCTSTR name);

    // Overriden virtual of TOcPart
    //
    bool        Init(TOcInitInfo * initInfo, owl::TRect pos);

    // Define AddRef/Release/QI to avoid ambiguity
    //
    TUNKNOWN_STANDARD_IUNKNOWN_METHODS;

    // Retrieve interface/object
    //
    IBControl*  GetBControlI() {return BCtrlI;}
    TOcView*    GetActiveControlView();

    ITypeInfo*  GetEventTypeInfo();
    IDispatch*  GetCtlDispatch();

    // Flag accessor functions
    //
    void        SetDefault(bool def)
                {if (def) Flags |= OcxDefault; else Flags &= ~OcxDefault;}
    void        SetCancel(bool cancel)
                {if (cancel) Flags |= OcxCancel; else Flags &= ~OcxCancel;}
    void        SetUserName(owl::TString& name);
    owl::uint        GetId() const { return CtlId; }
    void        SetId(owl::uint id){ CtlId = id;   }

    // Support for caching/looking of control's events
    //
    bool        FillEventList();
    TEventList* GetEventList() const;

    // Access to Extended property support
    //
    bool       IsCancel()  const        {return (Flags & OcxCancel) != 0;}
    bool       IsDefault() const        {return (Flags & OcxDefault)!= 0;}
    long       GetLeft()                { return Pos.x; }
    long       GetTop()                 { return Pos.y; }
    long       GetWidth()               { return Size.cx; }
    long       GetHeight()              { return Size.cy; }
    void       SetLeft(long Value);
    void       SetWidth(long Value);
    void       SetTop(long Value);
    void       SetHeight(long Value);
    owl::TString&   GetUserName();
    IDispatch* GetParent();

    // Standard methods
    //
    HRESULT     Refresh()               { return GetBControlI()->Refresh(); }
    HRESULT     DoClick()               { return GetBControlI()->DoClick(); }
    HRESULT     AboutBox()              { return GetBControlI()->AboutBox();}

  protected:
   ~TOcControl();

    // Overriden virtual of TOcPart
    //
    bool InitObj (TOcInitInfo * initInfo);

    // Helper to create served object (IDispatch*) of a class
    //
    TUnknown*   CreateAutoObject(const void* obj, TAutoClass& clsInfo);

    // TUnknown virtual overrides
    //
    HRESULT     QueryObject(const IID & iid, void * * iface);

    IBControl       *BCtrlI;
    owl::TString                 *pUserName;
    TServedObject   *pIExtended;
    TOcControlEvent *pEvents;
  bool            pEventsOwnership;
    owl::uint            CtlId;
    TEventList*     EventList;

    // IBControlSite implementation for BOle to use
    //
    HRESULT _IFUNC Init(UINT, IBControl*, UINT);
    HRESULT _IFUNC OnPropertyChanged(DISPID dispid);
    HRESULT _IFUNC OnPropertyRequestEdit(DISPID dispid);
    HRESULT _IFUNC OnControlFocus(BOOL fGotFocus);
    HRESULT _IFUNC TransformCoords(owl::TPointL * lpptlHimetric,
                            owl::TPointF * lpptfContainer, DWORD flags);

    // TOcPart virtual routed to correct base (don't want other Init to hide)
    //
    HRESULT _IFUNC Init(IBDataProvider * dp, IBPart * p, LPCOLESTR s, BOOL b)
                       {return TOcPart::Init(dp, p, s, b);}

    // Initialize the event IDispatch [Assumes pEvents is initialized]
    //
    void       SetEventDispatch();

  DECLARE_AUTOCLASS(TOcControl)
    //
    // Standard extended properties
    //
    AUTOPROP(Visible,     IsVisible,    SetVisible,     bool, public:)
    AUTOPROP(Cancel,      IsCancel,     SetCancel,      bool, public:)
    AUTOPROP(Default,     IsDefault,    SetDefault,     bool, public:)
    //AUTOPROP(Name,        GetUserName,  SetUserName,    owl::TString, public:)
#pragma warn -inl
        AUTOPROPBUG(Name,        GetUserName,  SetUserName, owl::TString, public:) // yura 05/26/98
#pragma warn .inl

    AUTOPROPRO(Parent,    GetParent,    IDispatch*,           public:)
    AUTOPROP(Left,        GetLeft,      SetLeft,        long, public:)
    AUTOPROP(Top,         GetTop,       SetTop,         long, public:)
    AUTOPROP(Width,       GetWidth,     SetWidth,       long, public:)
    AUTOPROP(Height,      GetHeight,    SetHeight,      long, public:)

  friend TOcControlEvent;
};

//
/// \class TOcControlEvent
// ~~~~~ ~~~~~~~~~~~~~~~
/// OC Control Event class for standard control events
//
class _ICLASS TOcControlEvent : public TUnknown {
  public:
    TOcControlEvent();

    // Accessor to associated control object
    //
    TOcControl*   GetControl() {return pCtrl;}

  protected:
   ~TOcControlEvent();

    virtual void  InitEventDispatch();
    TUnknown*     CreateAutoObject(const void* obj, TAutoClass& clsInfo);

    // Standard events support
    //
    long          Click();
    long          DblClick();
    long          MouseDown(short Button, short Shift, long X, long Y);
    long          MouseMove(short Button, short Shift, long X, long Y);
    long          MouseUp  (short Button, short Shift, long X, long Y);
    long          KeyDown  (short* KeyCode, short Shift);
    long          KeyUp    (short* KeyCode, short Shift);
    long          ErrorEvent(short number, TAutoString Description,
                             SCODE SCode, TAutoString Source,
                             TAutoString HelpFile,
                             long helpContext, bool * CancelDisplay);

    HRESULT       ForwardClickEvent(owl::uint msg, DISPID id);
    HRESULT       ForwardKeyEvent  (owl::uint msg, DISPID id, short *KeyCode, short Shift);
    HRESULT       ForwardMouseEvent(owl::uint msg, DISPID id, short Button, short Shift,
                                    long X, long Y);
    HRESULT       CustomEvent(long *args);

  protected:
    TOcControl    *pCtrl;
    TServedObject *pIEvents;

  DECLARE_AUTOCLASS(TOcControlEvent)

    // Automation symbol table declaring methods handling standard events
    //
    AUTOFUNC0R(Click,           Click,          long)
    AUTOFUNC0R(DblClick,    DblClick,       long)
    AUTOFUNC4(MouseDown,  MouseDown,  long,  short,  short,  long,  long, public:)
    AUTOFUNC4(MouseUp,    MouseUp,    long,  short,  short,  long,  long, public:)
    AUTOFUNC4(MouseMove,  MouseMove,  long,  short,  short,  long,  long, public:)
    AUTOFUNC2(KeyDown,    KeyDown,    long,  short*, short, public:)
    AUTOFUNC2(KeyUp,      KeyUp,      long,  short*, short, public:)
#pragma warn -inl
    AUTOFUNC7(ErrorEvent, ErrorEvent, long,
                                      short         /* number       */,
                                      TAutoString   /* Description  */,
                                      long          /* SCode        */,
                                      TAutoString   /* Source       */,
                                      TAutoString   /* HelpFile     */,
                                      long          /* helpContext  */,
                                      bool*         /* CancelDisplay */, public:)
#pragma warn .inl
    AUTOFUNCX(CustomEvent, CustomEvent, long, public:)

  friend TOcControl;
};


//
/// \class TOcxView
// ~~~~~ ~~~~~~~~
/// The TOcxView partner is a container (viewer) of a given (server/client)
/// document.
//
class _ICLASS TOcxView : public TOcView {
  public:
    TOcxView(TOcDocument& doc, owl::TRegList* regList=0, IUnknown* outer=0);
   ~TOcxView();

    HRESULT     QueryObject(const IID & iid, void * * iface);

    // Ambient property support
    //
    void        SetBackColor(long Color);
    void        SetForeColor(long Color);
    void        SetLocaleID(long LocaleId);
    void        SetMessageReflect(bool MsgRef);
    void        SetTextAlign(short Align);
    void        SetUserMode(bool Mode);
    void        SetUIDead(bool Dead);
    void        SetShowGrabHandles(bool Handles);
    void        SetSupportsMnemonics(bool Mnem);
    void        SetShowHatching(bool Hatch);
    void        SetDisplayAsDefault(bool Disp);
    void        SetDisplayName(owl::TString& Name);
    void        SetScaleUnits(owl::TString& ScaleUnits);
    void        SetFont(IDispatch *pFontDisp);

  protected:
    long        GetBackColor();
    long        GetForeColor();
    long        GetLocaleID();
    bool        GetMessageReflect();
    short       GetTextAlign();
    bool        GetUserMode();
    bool        GetUIDead();
    bool        GetShowGrabHandles();
    bool        GetSupportsMnemonics();
    bool        GetShowHatching();
    bool        GetDisplayAsDefault();
    owl::TString&    GetDisplayName();
    owl::TString&    GetScaleUnits();
    IDispatch*  GetFont();  // returns IFontDispatch

    void        SetAmbBackColor(long Color);
    void        SetAmbForeColor(long Color);
    void        SetAmbLocaleID(long LocaleId);
    void        SetAmbMessageReflect(bool MsgRef);
    void        SetAmbTextAlign(short Align);
    void        SetAmbUserMode(bool Mode);
    void        SetAmbUIDead(bool Dead);
    void        SetAmbShowGrabHandles(bool Handles);
    void        SetAmbSupportsMnemonics(bool Mnem);
    void        SetAmbShowHatching(bool Hatch);
    void        SetAmbDisplayAsDefault(bool Disp);
    void        SetAmbDisplayName(owl::TString& Name);
    void        SetAmbScaleUnits(owl::TString& ScaleUnits);
    void        SetAmbFont(IDispatch *pFontDisp);

    void        AmbientChanged(DISPID dispid);
    long        GetAmbientValue(long AmbientMsg, long Default);

  protected:
    TServedObject *pIAmbients;
    owl::TString *pBlankString;

  // Ambient properties
  //
  DECLARE_AUTOCLASS(TOcxView)
    AUTOPROP(BackColor,        GetBackColor,      SetAmbBackColor,      long, public:)
    AUTOPROP(ForeColor,        GetForeColor,      SetAmbForeColor,      long, public:)
    AUTOPROP(LocaleID,         GetLocaleID,       SetAmbLocaleID,       long, public:)
    AUTOPROP(MessageReflect,   GetMessageReflect, SetAmbMessageReflect, bool, public:)
    AUTOPROP(TextAlign,        GetTextAlign,      SetAmbTextAlign,      short, public:)
    AUTOPROP(UserMode,         GetUserMode,       SetAmbUserMode,       bool, public:)
    AUTOPROP(UIDead,           GetUIDead,         SetAmbUIDead,         bool, public:)
    AUTOPROP(ShowGrabHandles,  GetShowGrabHandles,SetAmbShowGrabHandles,bool, public:)
    AUTOPROP(ShowHatching,     GetShowHatching,   SetAmbShowHatching,   bool, public:)
    //AUTOPROP(DisplayName,      GetDisplayName,    SetAmbDisplayName,    owl::TString, public:)
#pragma warn -inl
    AUTOPROPBUG(DisplayName,   GetDisplayName,    SetAmbDisplayName, owl::TString, public:) // yura 05/26/98
#pragma warn .inl
    //AUTOPROP(ScaleUnits,       GetScaleUnits,     SetAmbScaleUnits,     owl::TString, public:)
#pragma warn -inl
    AUTOPROPBUG(ScaleUnits,    GetScaleUnits,     SetAmbScaleUnits,  owl::TString, public:) // yura 05/26/98
#pragma warn .inl
    AUTOPROP(Font,             GetFont,           SetAmbFont,           IDispatch*, public:)
    AUTOPROP(DisplayAsDefault, GetDisplayAsDefault, SetDisplayAsDefault,     bool, public:)
    AUTOPROP(SupportsMnemonics,GetSupportsMnemonics,SetAmbSupportsMnemonics, bool, public:)
};


//
/// \struct TCtrlEvent
//
/// Base struct for all control event messages
//
struct TCtrlEvent {
  TCtrlEvent(TOcControl* ctl, DISPID id) : Ctrl(ctl), DispId(id) {}

  TOcControl* Ctrl;       // Proxy object representing control
  DISPID      DispId;
};

struct TCtrlMouseEvent : public TCtrlEvent {
  TCtrlMouseEvent(TOcControl* ctl, DISPID id, short butt, short shft, long x, long y) :
                  TCtrlEvent(ctl, id), Button(butt), Shift(shft), X(x), Y(y)
                  {}

  short Button;
  short Shift;
  long  X;
  long  Y;
};

struct TCtrlKeyEvent : public TCtrlEvent {
  TCtrlKeyEvent(TOcControl* ctl, DISPID id, short kcode, short shft) : TCtrlEvent(ctl, id),
                KeyCode(kcode), Shift(shft)
                {}

  short KeyCode;
  short Shift;
};

struct TCtrlFocusEvent : public TCtrlEvent {
  TCtrlFocusEvent(TOcControl* ctl, bool gotFocus)
                : TCtrlEvent(ctl, 0/*IId of Focus Event??*/),
                  GotFocus(gotFocus){}

  bool GotFocus;
};

struct TCtrlPropertyEvent : public TCtrlEvent {
  TCtrlPropertyEvent(TOcControl* ctl, DISPID dispid, bool acpt = true) :
          TCtrlEvent(ctl, dispid), accept(acpt)
          {}

  bool    accept;
};

struct TCtrlErrorEvent : public TCtrlEvent {
#pragma warn -inl

  TCtrlErrorEvent(TOcControl* ctl, short num, TAutoString desc,
                  SCODE scode, TAutoString src, TAutoString hlpfile,
                  long hlpCtx, bool xceldisplay) :
                  TCtrlEvent(ctl, DISPID_ERROREVENT),
                  Number(num), Description(desc), SCode(scode), Source(src),
                  HelpFile(hlpfile), HelpContext(hlpCtx),
                  CancelDisplay(xceldisplay)
                  {}
#pragma warn .inl

  short       Number;
  TAutoString Description;
  SCODE       SCode;
  TAutoString Source;
  TAutoString HelpFile;
  long        HelpContext;
  bool        CancelDisplay;
};

struct TCtrlCustomEvent : public TCtrlEvent {
  TCtrlCustomEvent(TOcControl* ctl, DISPID id, TAutoStack* args) : TCtrlEvent(ctl, id),
                    Args(args)
                    {}
  TAutoStack *Args;
};

struct TCtrlTransformCoords : public TCtrlEvent {

  TCtrlTransformCoords(TOcControl* ctl, owl::TPointL* him, owl::TPointF* pcont,
                       DWORD flgs) :
                       TCtrlEvent(ctl, 0/*IID for this event??*/),
                       lpptHimetric(him), lpptContainer(pcont), flags(flgs)
                       {}

  owl::TPointL *lpptHimetric;
  owl::TPointF *lpptContainer;
  DWORD     flags;
};


//
// Global functions to allow easy registration/unregistration of OCXes
//
_OCFFUNC(HRESULT) OcRegisterControl(LPTSTR ocxPath);
_OCFFUNC(HRESULT) OcUnregisterControl(CLSID ClassId);

} // OCF namespace

#endif

