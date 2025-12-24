//
/// \file
/// Definition of classes for CommonControl encapsulation
//
// Part of OWLNext - the next generation Object Windows Library
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
// Copyright (c) 2013 Vidar Hasfjord
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//

#if !defined(OWL_COMMCTRL_H)
#define OWL_COMMCTRL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

//JJH
#if !defined(RC_INVOKED)

# include <owl/control.h>
# include <owl/module.h>

#if !defined(__RICHEDIT_H)
# include <richedit.h>
#endif

#endif /*RC_INVOKED*/

//
// Get the necessary system headers
//
#if !defined(_INC_COMMCTRL)
# include <commctrl.h>
#endif

#include <owl/shelwapi.h>

#pragma comment(lib, "comctl32")

OWL_DISABLE_WARNING_PUSH
OWL_DISABLE_WARNING_OLD_STYLE_CAST

namespace owl {

//
/// Wrapper for the Windows API function InitCommmonControlsEx.
/// The given argument must be one or a combination of flags indicating the controls to register.
/// Throws TXCommCtrl on failure.
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/bb775697.aspx
//
void InitializeCommonControls(uint controlFlags);

//
/// Returns the version number of the Common Control library (ComCtl32.dll).
/// The upper WORD of the returned value contains the major version number, and the lower WORD
/// contains the minor version number.
/// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/hh298349.aspx#DllGetVersion
//
auto GetCommCtrlVersion() -> DWORD;

//----------------------------------------------------------------------------
// Forward declaration of classes used in dispatch macros
//
class _OWLCLASS THdrItem;

#include <owl/preclass.h>

//
/// \class TXCommCtrl
// ~~~~~ ~~~~~~~~~~
/// Exception object thrown by the Common Control wrappers of ObjectWindows.
//
class _OWLCLASS TXCommCtrl : public TXOwl {
  public:
    TXCommCtrl();
    TXCommCtrl* Clone();
    void        Throw();
    static void Raise();
};

//
/// \class TNotify
// ~~~~~ ~~~~~~~
/// TNotify is a thin wrapper around the NMHDR structure. It's a
/// place-holder for future enhancements for handling notifications.
//
class _OWLCLASS TNotify : public NMHDR {
  public:
    TNotify();
    TNotify(HWND ctl, uint id, uint code);
};


//----------------------------------------------------------------------------
// Structures used for UpDown (Spin Ctrl) notifications
//----------------------------------------------------------------------------
//
/// \class TNmUpDown
// ~~~~~ ~~~~~~~~~
/// TNmUpDown is a wrapper of the NM_UPDOWN structure sent with
/// notifications from an 'UpDown' control.
//
class _OWLCLASS TNmUpDown : public NM_UPDOWN {
  public:
    TNmUpDown(HWND hwnd, uint id, uint code, int pos, int delta);

    // Allows the notification structure to be transparently treated an
    // an NMHDR structure thereby eliminating the need to explicitly refer
    // to the NMHDR data member [always the first member of notification
    // structures]
    //
    operator  NMHDR&() { return hdr; }
};



//----------------------------------------------------------------------------
// Structures used for Tab Control notifications
//----------------------------------------------------------------------------
//
/// \class TTabKeyDown
// ~~~~~ ~~~~~~~~~~~
/// Wrapper of the TC_KEYDOWN structure sent to the parent of a tabcontrol
/// to notify that a key has been pressed
//
class _OWLCLASS TTabKeyDown : public TC_KEYDOWN {
  public:
    TTabKeyDown();
    TTabKeyDown(HWND ctl, uint id, uint code, uint16 vk, uint flg);

/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return hdr; }
};



//----------------------------------------------------------------------------
// Structures used for tooltip notifications & API
//----------------------------------------------------------------------------
//
/// \class TTooltipText
// ~~~~~ ~~~~~~~~~~~~
/// TTooltipText identifies a tool for which text is to be displayed. It
/// is sent to the parent of the tooltip via a WM_NOTIFY/TTN_NEEDTEXT
/// notification. It receives the text to be displayed.
//
class _OWLCLASS TTooltipText : public TOOLTIPTEXT {
  public:
    operator  NMHDR&() { return hdr; }

    // Set the text to be displayed for the tool.
    //
    void        SetText(LPCTSTR buff);
    void        SetText(int resId, HINSTANCE hInstance);
    void        CopyText(LPCTSTR buff);
    void CopyText(const tstring& s) { CopyText(s.c_str()); }
};

//
//
//
class _OWLCLASS TTtCustomDraw : public NMTTCUSTOMDRAW {
  public:
    operator  NMCUSTOMDRAW&() { return nmcd; }
};

//
//
//
class _OWLCLASS TTooltipDispInfo : public NMTTDISPINFO {
  public:
    operator  NMHDR&() { return hdr; }
};

//
// Documented but not defined ????????????????
//
#if 0
class _OWLCLASS TTooltipCreated : public NMTOOLTIPSCREATED {
  public:
    operator  NMHDR&() { return hdr; }
};
#endif

//----------------------------------------------------------------------------
// Structures used for Column Header notifications
//----------------------------------------------------------------------------
//
/// \class THdrNotify
// ~~~~~ ~~~~~~~~~~
/// THdrNotify encapsulates the 'HD_NOTIFY' structure which is sent
/// with the WM_NOTIFY messages sent by the HeaderControl to its parent.
//
class _OWLCLASS THdrNotify : public HD_NOTIFY {
  public:
/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return hdr; }
};

//
// Version 4.0
//
class _OWLCLASS THdrDispInfo : public NMHDDISPINFO {
  public:
    operator  NMHDR&() { return hdr; }
};

//
// Version 5.8
//
class _OWLCLASS THdrFilterBtnClick : public NMHDFILTERBTNCLICK {
  public:
    operator  NMHDR&() { return hdr; }
};

//----------------------------------------------------------------------------
// TDateTimePicker structures
//
// Version 4.70
//
class _OWLCLASS TDateTimeChange : public NMDATETIMECHANGE {
  public:
    operator  NMHDR&() { return nmhdr; }
};

//
// Version 4.70
//
class _OWLCLASS TDateTimeFormat : public NMDATETIMEFORMAT {
  public:
    operator  NMHDR&() { return nmhdr; }
};

//
// Version 4.70
//
class _OWLCLASS TDateTimeFormatQuery : public NMDATETIMEFORMATQUERY {
  public:
    operator  NMHDR&() { return nmhdr; }
};

//
// Version 4.70
//
class _OWLCLASS TDateTimeString : public NMDATETIMESTRING {
  public:
    operator  NMHDR&() { return nmhdr; }
};

//
// Version 4.70
//
class _OWLCLASS TDateTimeKeyDown : public NMDATETIMEWMKEYDOWN {
  public:
    operator  NMHDR&() { return nmhdr; }
};

//----------------------------------------------------------------------------
// Structures used for RichEdit notifications and API
//----------------------------------------------------------------------------
//
/// \class TEnDropFiles
// ~~~~~ ~~~~~~~~~~~~
/// Structure sent with EN_DROPFILES notification.
//
class _OWLCLASS TEnDropFiles : public ENDROPFILES {
  public:
/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return nmhdr; }
};

//
/// \class TMsgFilter
// ~~~~~ ~~~~~~~~~~
/// Structure sent with EN_MSGFILTER notification
//
class _OWLCLASS TMsgFilter : public MSGFILTER {
  public:
/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return nmhdr; }
};

//
/// \class TEnProtected
// ~~~~~ ~~~~~~~~~~~~
/// Structure sent with EN_PROTECTED notification
//
class _OWLCLASS TEnProtected : public ENPROTECTED {
  public:
/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return nmhdr; }
};

//
/// \class TReqResize
// ~~~~~ ~~~~~~~~~~
/// Structure sent with EN_REQUESTRESIZE notification
//
class _OWLCLASS TReqResize : public REQRESIZE {
  public:
/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return nmhdr; }
};

//
/// \class TSelChange
// ~~~~~ ~~~~~~~~~~
/// Structure sent with EN_SELCHANGE notification
//
class _OWLCLASS TSelChange : public SELCHANGE {
  public:
/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return nmhdr; }
};


//
// Forward declarations of notifications
//

//----------------------------------------------------------------------------
// Structures used for ListView control (TListViewCtrl) notifications
//----------------------------------------------------------------------------
//
/// \class TLvNotify
// ~~~~~ ~~~~~~~~~
/// Basic TListViewCtrl notification
//
class _OWLCLASS TLvNotify : public NMLISTVIEW {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TLvNotify TLwNotify; // Old alternative name - deprecated.

//
/// \class TLvDispInfoNotify
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// A TListViewCtrl notification to repaint an item.
//
class _OWLCLASS TLvDispInfoNotify : public NMLVDISPINFO {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TLvDispInfoNotify TLwDispInfoNotify; // Old alternative name - deprecated.

//
/// \class TLvKeyDownNotify
// ~~~~~ ~~~~~~~~~~~~~~~~
/// A TListViewCtrl notification that a key has been pressed.
//
class _OWLCLASS TLvKeyDownNotify : public NMLVKEYDOWN {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TLvKeyDownNotify TLwKeyDownNotify; // Old alternative name - deprecated.

//
/// \class TLvBkImage
// ~~~~~ ~~~~~~~~~~
// Version 4.70
class _OWLCLASS TLvBkImage : public LVBKIMAGE{
  public:
    TLvBkImage(){}
};
typedef TLvBkImage TLwBkImage; // Old alternative name - deprecated.

//
/// \class TLvCacheHint
// ~~~~~ ~~~~~~~~~~
// Version 4.70
class _OWLCLASS TLvCacheHint : public NMLVCACHEHINT{
  public:
    TLvCacheHint(){}
    operator  NMHDR&() { return hdr; }
};
typedef TLvCacheHint TLwCacheHint; // Old alternative name - deprecated.

//
/// \class TLvCustomDraw
// ~~~~~ ~~~~~~~~~~~~~
// Version 4.70/4.71
class _OWLCLASS TLvCustomDraw : public NMLVCUSTOMDRAW{
  public:
    TLvCustomDraw(){}

    operator  NMCUSTOMDRAW&() { return nmcd; }

    TColor   GetTextColor()   const { return TColor(clrText);   }
    TColor   GetBkColor()     const { return TColor(clrTextBk); }
    int     GetSubItem()    const { return iSubItem;          } // Version 4.71
};
typedef TLvCustomDraw TLwCustomDraw; // Old alternative name - deprecated.

//
/// \class TLvFindItem
// ~~~~~ ~~~~~~~~~~~
// Version 4.70
class _OWLCLASS TLvFindItem : public NMLVFINDITEM{
  public:
    TLvFindItem(){}

    operator  NMHDR&() { return hdr; }
};
typedef TLvFindItem TLwFindItem; // Old alternative name - deprecated.

//
/// \class TLvOdStateChanged
// ~~~~~ ~~~~~~~~~~~~~~~~~
// Version 4.70
class _OWLCLASS TLvOdStateChanged : public NMLVODSTATECHANGE{
  public:
    TLvOdStateChanged(){}

    operator  NMHDR&() { return hdr; }
};
typedef TLvOdStateChanged TLwOdStateChanged; // Old alternative name - deprecated.

//
/// \class TLvGetInfoTip
// ~~~~~ ~~~~~~~~~~~~~
// Version 4.71
class _OWLCLASS TLvGetInfoTip : public NMLVGETINFOTIP{
  public:
    TLvGetInfoTip(){}

    operator  NMHDR&() { return hdr; }
};
typedef TLvGetInfoTip TLwGetInfoTip; // Old alternative name - deprecated.

//
/// \class TLvItemActivate
// ~~~~~ ~~~~~~~~~~~~~~~
// Version 4.71
class _OWLCLASS TLvItemActivate : public NMITEMACTIVATE{
  public:
    TLvItemActivate(){}

    operator  NMHDR&() { return hdr; }
};
typedef TLvItemActivate TLwItemActivate; // Old alternative name - deprecated.



//----------------------------------------------------------------------------
// Structures used for TreeView control (TTreeViewCtrl) notifications
//----------------------------------------------------------------------------
//
/// \class TTvNotify
// ~~~~~ ~~~~~~~~~
/// Basic TTreeViewCtrl notification
//
class _OWLCLASS TTvNotify : public NMTREEVIEW {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TTvNotify TTwNotify; // Old alternative name - deprecated.

//
/// \class TTvDispInfoNotify
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// A TTreeViewCtrl notification to repaint an item.
//
class _OWLCLASS TTvDispInfoNotify : public NMTVDISPINFO {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TTvDispInfoNotify TTwDispInfoNotify; // Old alternative name - deprecated.

//
/// \class TTvKeyDownNotify
// ~~~~~ ~~~~~~~~~~~~~~~~
/// A TTreeViewCtrl notification that a key has been pressed.
//
class _OWLCLASS TTvKeyDownNotify : public NMTVKEYDOWN {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TTvKeyDownNotify TTwKeyDownNotify; // Old alternative name - deprecated.

//
/// \class TTvHitTestInfo
// ~~~~~ ~~~~~~~~~~~~~~
/// A TTreeViewCtrl notification for hit-testing.
//
class _OWLCLASS TTvHitTestInfo : public TVHITTESTINFO {
  public:
};
typedef TTvHitTestInfo TTwHitTestInfo; // Old alternative name - deprecated.

//
/// \class TTvCustomDraw
// ~~~~~ ~~~~~~~~~~~~~
// Version 4.70/4.71
class _OWLCLASS TTvCustomDraw : public NMTVCUSTOMDRAW{
  public:
    operator NMHDR&() {return nmcd.hdr;}
    operator  NMCUSTOMDRAW&() { return   nmcd; }

    TColor   GetTextColor()   const { return TColor(clrText);   }
    TColor   GetBkColor()     const { return TColor(clrTextBk); }
    int     GetLevel()      const { return iLevel;            } // Version 4.71
};
typedef TTvCustomDraw TTwCustomDraw; // Old alternative name - deprecated.

//
// class TTvGetInfoTip
// ~~~~~ ~~~~~~~~~~~~~
// Version 4.71
class _OWLCLASS TTvGetInfoTip : public NMTVGETINFOTIP {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TTvGetInfoTip TTwGetInfoTip; // Old alternative name - deprecated.

////////////////////////////////////////////////////////////////////////////////
//
/// \class TNmMouse
// ~~~~~ ~~~~~~~~
//
class _OWLCLASS TNmMouse : public NMMOUSE {
  public:
    operator  NMHDR&() { return hdr; }
};

//
/// \class TNmObjectNotify
// ~~~~~ ~~~~~~~~~~~~~~~
//
class _OWLCLASS TNmObjectNotify : public NMOBJECTNOTIFY {
  public:
    operator  NMHDR&() { return hdr; }
};

//
/// \class TNmKey
// ~~~~~ ~~~~~~
//
class _OWLCLASS TNmKey : public NMKEY {
  public:
    operator  NMHDR&() { return hdr; }
};


//
/// \class TNmChar
// ~~~~~ ~~~~~~~
//
class _OWLCLASS TNmChar : public NMCHAR {
  public:
    operator  NMHDR&() { return hdr; }
};

//
/// \class TNmCustomDraw
// ~~~~~ ~~~~~~~~~~~~~
//
class _OWLCLASS TNmCustomDraw : public NMCUSTOMDRAW {
  public:
    TNmCustomDraw(){}
};

//
/// \class TRebarInfo
// ~~~~~ ~~~~~~~~~~
//
class _OWLCLASS TRebarInfo : public REBARINFO {
  public:
    TRebarInfo() { cbSize = sizeof(REBARINFO); }

    // Would prefer to use TImageLists...
    void SetImageList(HIMAGELIST Images);
    HIMAGELIST GetImageList();
};

//
/// \class TColorScheme
// ~~~~~ ~~~~~~~~~~~~
//
class _OWLCLASS TColorScheme : public COLORSCHEME {
  public:
    TColorScheme()
      {
        dwSize=sizeof(COLORSCHEME);
        clrBtnHighlight=clrBtnShadow=TColor::None;
      }
    TColorScheme(const TColor& btn, const TColor& shad)
      {
        dwSize = sizeof(COLORSCHEME);
        clrBtnHighlight=btn;
        clrBtnShadow=shad;
      }

    TColor  BtnHighlight() { return TColor(clrBtnHighlight);    }
    void      BtnHighlight(const TColor& clr){clrBtnHighlight=clr;}
    TColor   BtnShadow()    { return TColor(clrBtnShadow);        }
    void      BtnShadow(const TColor& clr){ clrBtnShadow  = clr;  }
};

//
/// \class TRbHitTestInfo
// ~~~~~ ~~~~~~~~~~
//
class _OWLCLASS TRbHitTestInfo : public RBHITTESTINFO {
  public:
    TRbHitTestInfo(const TPoint& point){pt.x = point.x; pt.y=point.y;}
};

//
// ComboBoxEx structures
// ~~~~~~~~~~ ~~~~~~~~~~
//
class _OWLCLASS TCBExItemInfo : public COMBOBOXEXITEM {
  public:
};

class _OWLCLASS TCBExEditInfo : public NMCBEENDEDIT {
  public:
};

class _OWLCLASS TCBExDragInfo : public NMCBEDRAGBEGIN {
  public:
};

class _OWLCLASS TCBExNotifyInfo : public NMCOMBOBOXEX {
  public:
};

//
/// MonthCalendar Control notifications
//

class _OWLCLASS TMCHitTestInfo:  public MCHITTESTINFO{
  public:
    TMCHitTestInfo() { cbSize = sizeof(MCHITTESTINFO);}
};

//
/// \class TNmDayState
// ~~~~~ ~~~~~~~~~~~
//
class _OWLCLASS TNmDayState : public NMDAYSTATE {
  public:
    operator  NMHDR&() { return nmhdr; }
};

//
/// \class TNmSelChange
// ~~~~~ ~~~~~~~~~~~~
//
class _OWLCLASS TNmSelChange : public NMSELCHANGE {
  public:
    operator  NMHDR&() { return nmhdr; }
};


//
/// Pager Control notifications
//

class _OWLCLASS TNmPGCalcSize:  public NMPGCALCSIZE{
  public:
    operator  NMHDR&() { return hdr; }
};

class _OWLCLASS TNmPGScroll:  public NMPGSCROLL{
  public:
    operator  NMHDR&() { return hdr; }
};

//
/// IP Address control notifications
//

class _OWLCLASS TNmIPAddress:  public NMIPADDRESS{
  public:
    operator  NMHDR&() { return hdr; }
};

// Property Sheet notifications

class _OWLCLASS TPshNotify : public PSHNOTIFY {
  public:
    TPshNotify(HWND hwnd, uint id, uint code, LPARAM lp);
    operator  NMHDR&() { return hdr; }
};

#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Control notification message dispatch
// TODO: Change handler parameters to const where applicable.
//

template <> struct TDispatch<WM_NOTIFY>
{
  static const TMsgId MessageId = WM_NOTIFY;
  typedef TResult THandlerResult;

  //
  // General encoder - used when the notification code is not known at compile-time.
  //
  template <class F>
  static THandlerResult Encode(F sendMessage, HWND wnd, NMHDR& nmhdr)
  {return FORWARD_WM_NOTIFY(wnd, nmhdr.idFrom, &nmhdr, sendMessage);}

#if OWL_EV_SIGNATURE_CHECK

  template <class T> struct THandler {typedef THandlerResult (T::*type)(NMHDR&);};

#endif

  //
  // General decoder - used when the notification code is not known at compile-time.
  //
  template <class T, THandlerResult (T::*M)(NMHDR&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    struct TForwarder
    {
      T* i_;
      LRESULT operator ()(HWND, int, LPNMHDR pnmhdr)
      {
        PRECONDITION(pnmhdr);
        return pnmhdr ? (i_->*M)(*pnmhdr) : 0;
      };
    }
    forwarder = {static_cast<T*>(i)};
    return HANDLE_WM_NOTIFY(0, p1, p2, forwarder);
  }

  //
  // Base class for specialized notification dispatch
  // Defines NotificationCode and THandlerResult.
  //
  template <uint NotificationCode, class THandlerResult, class TParam>
  struct TNotificationDispatchBase;

  //
  // Specialization for non-void return and reference parameter
  //
  template <uint NotificationCode_, class THandlerResult_, class TNmParam>
  struct TNotificationDispatchBase<NotificationCode_, THandlerResult_, TNmParam&>
  {
    enum {NotificationCode = NotificationCode_};
    typedef THandlerResult_ THandlerResult; // Hides TDispatch<WM_NOTIFY>::THandlerResult.

    template <class F>
    static THandlerResult Encode(F sendMessage, HWND wnd, TNmParam& n)
    {return static_cast<THandlerResult>(sendMessage(wnd, MessageId, static_cast<TParam1>(n.idFrom), reinterpret_cast<TParam2>(&n)));}


#if OWL_EV_SIGNATURE_CHECK

    template <class T> struct THandler {typedef THandlerResult (T::*type)(TNmParam&);};

#endif

    template <class T, THandlerResult (T::*M)(TNmParam&)>
    static TResult Decode(void* i, TParam1, TParam2 p2)
    {
      PRECONDITION(p2);
      return p2 != 0 ? (TResult)( ((static_cast<T*>(i)->*M)(*reinterpret_cast<TNmParam*>(p2)))) : 0;
    }
  };

  //
  // Specialization for void return and reference parameter
  //
  template <uint NotificationCode_, class TNmParam>
  struct TNotificationDispatchBase<NotificationCode_, void, TNmParam&>
  {
    enum {NotificationCode = NotificationCode_};
    typedef void THandlerResult; // Hides TDispatch<WM_NOTIFY>::THandlerResult.

    template <class F>
    static THandlerResult Encode(F sendMessage, HWND wnd, TNmParam& n)
    {sendMessage(wnd, MessageId, static_cast<TParam1>(n.idFrom), reinterpret_cast<TParam2>(&n));}


#if OWL_EV_SIGNATURE_CHECK

    template <class T> struct THandler {typedef THandlerResult (T::*type)(TNmParam&);};

#endif

    template <class T, THandlerResult (T::*M)(TNmParam&)>
    static TResult Decode(void* i, TParam1, TParam2 p2)
    {
      PRECONDITION(p2);
      return p2 != 0 ? ((static_cast<T*>(i)->*M)(*reinterpret_cast<TNmParam*>(p2)), 0) : 0;
    }
  };

  //
  // The next two specializations allow a message to be handled without passing any arguments.
  // Note: Since notification info is required to send a notification message, no encoder can be provided.
  // TODO: Ideally, all notification handlers should use at least a notification info parameter.
  //
  template <uint NotificationCode_, class THandlerResult_>
  struct TNotificationDispatchBase<NotificationCode_, THandlerResult_, void>
  {
    enum {NotificationCode = NotificationCode_};
    typedef THandlerResult_ THandlerResult; // Hides TDispatch<WM_NOTIFY>::THandlerResult.

#if OWL_EV_SIGNATURE_CHECK

    template <class T> struct THandler {typedef THandlerResult (T::*type)();};

#endif

    template <class T, THandlerResult (T::*M)()>
    static TResult Decode(void* i, TParam1, TParam2)
    {return static_cast<TResult>((static_cast<T*>(i)->*M)());}
  };

  template <uint NotificationCode_>
  struct TNotificationDispatchBase<NotificationCode_, void, void>
  {
    enum {NotificationCode = NotificationCode_};
    typedef void THandlerResult; // Hides TDispatch<WM_NOTIFY>::THandlerResult.

#if OWL_EV_SIGNATURE_CHECK

    template <class T> struct THandler {typedef THandlerResult (T::*type)();};

#endif

    template <class T, THandlerResult (T::*M)()>
    static TResult Decode(void* i, TParam1, TParam2)
    {return (static_cast<T*>(i)->*M)(), 0;}
  };

  //
  // Dispatch template for Common Controls notifications passed by WM_NOTIFY
  // Note that notification codes are unique among all Common Controls, so they all share this dispatch template.
  // This default implementation is looked up if a notification does not provide a full specialization.
  // Passes the general NMHDR& and returns TResult.
  //
  template <uint NotificationCode>
  struct TNotificationDispatch : TNotificationDispatchBase<NotificationCode, TResult, NMHDR&>
  {};

};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_CHAR> : TNotificationDispatchBase<NM_CHAR, void, TNmChar&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_CLICK> : TNotificationDispatchBase<NM_CLICK, void, void> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_CUSTOMDRAW> : TNotificationDispatchBase<NM_CUSTOMDRAW, int, TNmCustomDraw&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_DBLCLK> : TNotificationDispatchBase<NM_DBLCLK, void, void> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_HOVER> : TNotificationDispatchBase<NM_HOVER, bool, TNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_KEYDOWN> : TNotificationDispatchBase<NM_KEYDOWN, bool, TNmKey&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_KILLFOCUS> : TNotificationDispatchBase<NM_KILLFOCUS, void, void> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_NCHITTEST> : TNotificationDispatchBase<NM_NCHITTEST, int, TNmMouse&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_OUTOFMEMORY> : TNotificationDispatchBase<NM_OUTOFMEMORY, void, void> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_RCLICK> : TNotificationDispatchBase<NM_RCLICK, void, void> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_RDBLCLK> : TNotificationDispatchBase<NM_RDBLCLK, void, void> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_RELEASEDCAPTURE> : TNotificationDispatchBase<NM_RELEASEDCAPTURE, void, void> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_RETURN> : TNotificationDispatchBase<NM_RETURN, void, void> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_SETCURSOR> : TNotificationDispatchBase<NM_SETCURSOR, int, TNmMouse&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_SETFOCUS> : TNotificationDispatchBase<NM_SETFOCUS, void, void> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<ACN_START> : TNotificationDispatchBase<ACN_START, void, void> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<ACN_STOP> : TNotificationDispatchBase<ACN_STOP, void, void> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_BEGINDRAG> : TNotificationDispatchBase<HDN_BEGINDRAG, bool, THdrNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_BEGINTRACK> : TNotificationDispatchBase<HDN_BEGINTRACK, bool, THdrNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_DIVIDERDBLCLICK> : TNotificationDispatchBase<HDN_DIVIDERDBLCLICK, void, THdrNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_ENDDRAG> : TNotificationDispatchBase<HDN_ENDDRAG, bool, THdrNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_ENDTRACK> : TNotificationDispatchBase<HDN_ENDTRACK, void, THdrNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_FILTERBTNCLICK> : TNotificationDispatchBase<HDN_FILTERBTNCLICK, bool, THdrFilterBtnClick&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_FILTERCHANGE> : TNotificationDispatchBase<HDN_FILTERCHANGE, void, THdrNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_GETDISPINFO> : TNotificationDispatchBase<HDN_GETDISPINFO, void, THdrNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_ITEMCHANGED> : TNotificationDispatchBase<HDN_ITEMCHANGED, void, THdrNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_ITEMCHANGING> : TNotificationDispatchBase<HDN_ITEMCHANGING, void, THdrNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_ITEMCLICK> : TNotificationDispatchBase<HDN_ITEMCLICK, void, THdrNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_ITEMDBLCLICK> : TNotificationDispatchBase<HDN_ITEMDBLCLICK, void, THdrNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<HDN_TRACK> : TNotificationDispatchBase<HDN_TRACK, bool, THdrNotify&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<DTN_CLOSEUP> : TNotificationDispatchBase<DTN_CLOSEUP, void, void> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<DTN_DATETIMECHANGE> : TNotificationDispatchBase<DTN_DATETIMECHANGE, int, TDateTimeChange&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<DTN_DROPDOWN> : TNotificationDispatchBase<DTN_DROPDOWN, void, void> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<DTN_FORMAT> : TNotificationDispatchBase<DTN_FORMAT, int, TDateTimeFormat&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<DTN_FORMATQUERY> : TNotificationDispatchBase<DTN_FORMATQUERY, int, TDateTimeFormat&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<DTN_USERSTRING> : TNotificationDispatchBase<DTN_USERSTRING, int, TDateTimeString&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<DTN_WMKEYDOWN> : TNotificationDispatchBase<DTN_WMKEYDOWN, int, TDateTimeKeyDown&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TCN_GETOBJECT> : TNotificationDispatchBase<TCN_GETOBJECT, void, TNmObjectNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TCN_KEYDOWN> : TNotificationDispatchBase<TCN_KEYDOWN, void, TTabKeyDown&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TCN_SELCHANGE> : TNotificationDispatchBase<TCN_SELCHANGE, void, TNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TCN_SELCHANGING> : TNotificationDispatchBase<TCN_SELCHANGING, bool, TNotify&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<EN_DROPFILES> : TNotificationDispatchBase<EN_DROPFILES, bool, TEnDropFiles&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<EN_MSGFILTER> : TNotificationDispatchBase<EN_MSGFILTER, bool, TMsgFilter&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<EN_PROTECTED> : TNotificationDispatchBase<EN_PROTECTED, bool, TEnProtected&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<EN_REQUESTRESIZE> : TNotificationDispatchBase<EN_REQUESTRESIZE, void, TReqResize&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<EN_SELCHANGE> : TNotificationDispatchBase<EN_SELCHANGE, bool, TSelChange&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<UDN_DELTAPOS> : TNotificationDispatchBase<UDN_DELTAPOS, bool, TNmUpDown&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TTN_GETDISPINFO> : TNotificationDispatchBase<TTN_GETDISPINFO, void, TTooltipDispInfo&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TTN_POP> : TNotificationDispatchBase<TTN_POP, void, TNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TTN_SHOW> : TNotificationDispatchBase<TTN_SHOW, void, TNotify&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_BEGINDRAG> : TNotificationDispatchBase<TVN_BEGINDRAG, void, TTvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_BEGINLABELEDIT> : TNotificationDispatchBase<TVN_BEGINLABELEDIT, bool, TTvDispInfoNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_BEGINRDRAG> : TNotificationDispatchBase<TVN_BEGINRDRAG, void, TTvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_DELETEITEM> : TNotificationDispatchBase<TVN_DELETEITEM, void, TTvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_ENDLABELEDIT> : TNotificationDispatchBase<TVN_ENDLABELEDIT, void, TTvDispInfoNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_GETDISPINFO> : TNotificationDispatchBase<TVN_GETDISPINFO, void, TTvDispInfoNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_GETINFOTIP> : TNotificationDispatchBase<TVN_GETINFOTIP, void, TTvGetInfoTip&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_ITEMEXPANDED> : TNotificationDispatchBase<TVN_ITEMEXPANDED, void, TTvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_ITEMEXPANDING> : TNotificationDispatchBase<TVN_ITEMEXPANDING, bool, TTvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_KEYDOWN> : TNotificationDispatchBase<TVN_KEYDOWN, void, TTvKeyDownNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_SELCHANGED> : TNotificationDispatchBase<TVN_SELCHANGED, void, TTvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_SELCHANGING> : TNotificationDispatchBase<TVN_SELCHANGING, bool, TTvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_SETDISPINFO> : TNotificationDispatchBase<TVN_SETDISPINFO, void, TTvDispInfoNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<TVN_SINGLEEXPAND> : TNotificationDispatchBase<TVN_SINGLEEXPAND, void, TTvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<NM_TVSTATEIMAGECHANGING> : TNotificationDispatchBase<NM_TVSTATEIMAGECHANGING, void, NMTVSTATEIMAGECHANGING&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_BEGINDRAG> : TNotificationDispatchBase<LVN_BEGINDRAG, void, TLvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_BEGINLABELEDIT> : TNotificationDispatchBase<LVN_BEGINLABELEDIT, bool, TLvDispInfoNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_BEGINRDRAG> : TNotificationDispatchBase<LVN_BEGINRDRAG, void, TLvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_COLUMNCLICK> : TNotificationDispatchBase<LVN_COLUMNCLICK, void, TLvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_DELETEALLITEMS> : TNotificationDispatchBase<LVN_DELETEALLITEMS, void, TLvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_DELETEITEM> : TNotificationDispatchBase<LVN_DELETEITEM, void, TLvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_ENDLABELEDIT> : TNotificationDispatchBase<LVN_ENDLABELEDIT, bool, TLvDispInfoNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_GETDISPINFO> : TNotificationDispatchBase<LVN_GETDISPINFO, void, TLvDispInfoNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_GETINFOTIP> : TNotificationDispatchBase<LVN_GETINFOTIP, void, TLvGetInfoTip&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_HOTTRACK> : TNotificationDispatchBase<LVN_HOTTRACK, bool, TLvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_INSERTITEM> : TNotificationDispatchBase<LVN_INSERTITEM, void, TLvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_ITEMACTIVATE> : TNotificationDispatchBase<LVN_ITEMACTIVATE, void, TLvItemActivate&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_ITEMCHANGED> : TNotificationDispatchBase<LVN_ITEMCHANGED, void, TLvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_ITEMCHANGING> : TNotificationDispatchBase<LVN_ITEMCHANGING, bool, TLvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_KEYDOWN> : TNotificationDispatchBase<LVN_KEYDOWN, void, TLvKeyDownNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_MARQUEEBEGIN> : TNotificationDispatchBase<LVN_MARQUEEBEGIN, bool, TLvNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_ODCACHEHINT> : TNotificationDispatchBase<LVN_ODCACHEHINT, bool, TLvCacheHint&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_ODFINDITEM> : TNotificationDispatchBase<LVN_ODFINDITEM, int, TLvFindItem&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_ODSTATECHANGED> : TNotificationDispatchBase<LVN_ODSTATECHANGED, bool, TLvOdStateChanged&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<LVN_SETDISPINFO> : TNotificationDispatchBase<LVN_SETDISPINFO, void, TLvDispInfoNotify&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<RBN_AUTOSIZE> : TNotificationDispatchBase<RBN_AUTOSIZE, void, TNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<RBN_BEGINDRAG> : TNotificationDispatchBase<RBN_BEGINDRAG, bool, TNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<RBN_CHILDSIZE> : TNotificationDispatchBase<RBN_CHILDSIZE, void, TNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<RBN_DELETEDBAND> : TNotificationDispatchBase<RBN_DELETEDBAND, void, TNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<RBN_DELETINGBAND> : TNotificationDispatchBase<RBN_DELETINGBAND, void, TNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<RBN_ENDDRAG> : TNotificationDispatchBase<RBN_ENDDRAG, void, TNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<RBN_GETOBJECT> : TNotificationDispatchBase<RBN_GETOBJECT, bool, TNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<RBN_HEIGHTCHANGE> : TNotificationDispatchBase<RBN_HEIGHTCHANGE, void, TNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<RBN_LAYOUTCHANGED> : TNotificationDispatchBase<RBN_LAYOUTCHANGED, void, TNotify&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<CBEN_BEGINEDIT> : TNotificationDispatchBase<CBEN_BEGINEDIT, void, TNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<CBEN_DELETEITEM> : TNotificationDispatchBase<CBEN_DELETEITEM, bool, TCBExItemInfo&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<CBEN_DRAGBEGIN> : TNotificationDispatchBase<CBEN_DRAGBEGIN, void, TCBExDragInfo&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<CBEN_ENDEDIT> : TNotificationDispatchBase<CBEN_ENDEDIT, bool, TCBExEditInfo&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<CBEN_GETDISPINFO> : TNotificationDispatchBase<CBEN_GETDISPINFO, bool, TCBExItemInfo&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<CBEN_INSERTITEM> : TNotificationDispatchBase<CBEN_INSERTITEM, bool, TCBExItemInfo&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<MCN_GETDAYSTATE> : TNotificationDispatchBase<MCN_GETDAYSTATE, void, TNmDayState&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<MCN_SELCHANGE> : TNotificationDispatchBase<MCN_SELCHANGE, void, TNmSelChange&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<MCN_SELECT> : TNotificationDispatchBase<MCN_SELECT, void, TNmSelChange&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PGN_CALCSIZE> : TNotificationDispatchBase<PGN_CALCSIZE, void, TNmPGCalcSize&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PGN_SCROLL> : TNotificationDispatchBase<PGN_SCROLL, void, TNmPGScroll&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<IPN_FIELDCHANGED> : TNotificationDispatchBase<IPN_FIELDCHANGED, void, TNmIPAddress&> {};

template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PSN_APPLY> : TNotificationDispatchBase<PSN_APPLY, int, TPshNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PSN_GETOBJECT> : TNotificationDispatchBase<PSN_GETOBJECT, void, TNmObjectNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PSN_HELP> : TNotificationDispatchBase<PSN_HELP, void, TPshNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PSN_KILLACTIVE> : TNotificationDispatchBase<PSN_KILLACTIVE, bool, TPshNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PSN_QUERYCANCEL> : TNotificationDispatchBase<PSN_QUERYCANCEL, bool, TPshNotify&> {};
// TODO: template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PSN_QUERYINITIALFOCUS> {...};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PSN_RESET> : TNotificationDispatchBase<PSN_RESET, void, TPshNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PSN_SETACTIVE> : TNotificationDispatchBase<PSN_SETACTIVE, int, TPshNotify&> {};
// TODO: template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PSN_TRANSLATEACCELERATOR> {...};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PSN_WIZBACK> : TNotificationDispatchBase<PSN_WIZBACK, int, TPshNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PSN_WIZFINISH> : TNotificationDispatchBase<PSN_WIZFINISH, bool, TPshNotify&> {};
template <> struct TDispatch<WM_NOTIFY>::TNotificationDispatch<PSN_WIZNEXT> : TNotificationDispatchBase<PSN_WIZNEXT, int, TPshNotify&> {};

//----------------------------------------------------------------------------

#if defined(OWL5_COMPAT)

DECLARE_SIGNATURE1(void,v_NMHDR_Sig,NMHDR &)
DECLARE_SIGNATURE1(bool,b_NMHDR_Sig,NMHDR &)
DECLARE_SIGNATURE1(void,v_NOTIFY_Sig,TNotify &)
DECLARE_SIGNATURE1(bool,b_NOTIFY_Sig,TNotify &)
DECLARE_SIGNATURE1(int,i_NOTIFY_Sig,TNotify &)
DECLARE_SIGNATURE1(int,i_CUSTOMDRAW_Sig,TNmCustomDraw &)
DECLARE_SIGNATURE1(int,i_NMMOUSE_Sig,TNmMouse &)
DECLARE_SIGNATURE1(void,v_NMCHAR_Sig,TNmChar &)
DECLARE_SIGNATURE1(bool,b_NMOBJECTNOTIFY_Sig,TNmObjectNotify &)
DECLARE_SIGNATURE1(bool,b_NMKEY_Sig,TNmKey &)
DECLARE_SIGNATURE2(void,v_i_NOTIFY_Sig,int,TNotify &)
DECLARE_SIGNATURE2(bool,b_i_NOTIFY_Sig,int,TNotify &)
DECLARE_SIGNATURE1(void,v_HEADERNOTIFY_Sig,THdrNotify &)
DECLARE_SIGNATURE1(bool,b_HEADERNOTIFY_Sig,THdrNotify &)
DECLARE_SIGNATURE1(void,v_HDN_DISPINFO_NOTIFY_Sig,THdrDispInfo &)
DECLARE_SIGNATURE1(void,b_HDN_DISPINFO_NOTIFY_Sig,THdrDispInfo &)
DECLARE_SIGNATURE1(bool,b_HDN_FILTERBTNCLICK_NOTIFY_Sig,THdrFilterBtnClick &)
DECLARE_SIGNATURE1(bool,b_NMUPDOWN_NOTIFY_Sig,TNmUpDown &)
DECLARE_SIGNATURE1(void,v_NEEDTEXT_Sig,TTooltipText &)
DECLARE_SIGNATURE1(int,i_NMTTCUSTOMDRAW_Sig,TTtCustomDraw &)
DECLARE_SIGNATURE1(void,v_NMTTDISPINFO_Sig,TTooltipDispInfo &)
DECLARE_SIGNATURE1(void,v_TREEWIND_NOTIFY_Sig,TTvNotify &)
DECLARE_SIGNATURE1(bool,b_TREEWIND_NOTIFY_Sig,TTvNotify &)
DECLARE_SIGNATURE1(void,v_TV_KEYDOWN_NOTIFY_Sig,TTvKeyDownNotify &)
DECLARE_SIGNATURE1(bool,b_TV_DISPINFO_NOTIFY_Sig,TTvDispInfoNotify &)
DECLARE_SIGNATURE1(void,v_TV_DISPINFO_NOTIFY_Sig,TTvDispInfoNotify &)
DECLARE_SIGNATURE1(int,i_TV_CUSTOMDRAW_Sig,TTvCustomDraw &) // Version 4.70/4.71
DECLARE_SIGNATURE1(void,v_TV_GETINFOTIP_Sig,TTvGetInfoTip &) // Version 4.70/4.71
DECLARE_SIGNATURE1(void,v_LISTWIND_NOTIFY_Sig,TLvNotify &)
DECLARE_SIGNATURE1(bool,b_LISTWIND_NOTIFY_Sig,TLvNotify &)
DECLARE_SIGNATURE1(void, v_LISTWIND_ITEMACTIVATE_Sig, TLwItemActivate far&)
DECLARE_SIGNATURE1(bool, b_LISTWIND_ITEMACTIVATE_Sig, TLwItemActivate far&)
DECLARE_SIGNATURE1(void,v_LV_KEYDOWN_NOTIFY_Sig,TLvKeyDownNotify &)
DECLARE_SIGNATURE1(bool,b_LV_DISPINFO_NOTIFY_Sig,TLvDispInfoNotify &)
DECLARE_SIGNATURE1(void,v_LV_DISPINFO_NOTIFY_Sig,TLvDispInfoNotify &)
DECLARE_SIGNATURE1(void,v_LV_GETINFOTIP_Sig,TLvGetInfoTip &) // Version 4.71
DECLARE_SIGNATURE1(bool,b_NMLVCACHEHINT_Sig,TLvCacheHint &)  // Version 4.70
DECLARE_SIGNATURE1(int,i_NMLVFINDITEM_Sig,TLvFindItem &)     // Version 4.70
DECLARE_SIGNATURE1(bool,b_NMLVODSTATECHANGE_Sig,TLvOdStateChanged &) // Version 4.70
DECLARE_SIGNATURE1(int,i_NMLVCUSTOMDRAW_Sig,TLvCustomDraw &) // Version 4.70/4.71
DECLARE_SIGNATURE1(bool,b_CBEN_ITEMINFO_NOTIFY_Sig,TCBExItemInfo &)
DECLARE_SIGNATURE1(void,v_CBEN_DRAGBEGIN_NOTIFY_Sig,TCBExDragInfo &)
DECLARE_SIGNATURE1(bool,b_CBEN_ENDEDIT_NOTIFY_Sig,TCBExEditInfo &)
DECLARE_SIGNATURE1(void,v_NMDAYSTATE_Sig,TNmDayState &)
DECLARE_SIGNATURE1(void,v_NMSELCHANGE_Sig,TNmSelChange &)
DECLARE_SIGNATURE1(void,v_NMPGCALCSIZE_Sig,TNmPGCalcSize &)
DECLARE_SIGNATURE1(void,v_NMPGSCROLL_Sig,TNmPGScroll &)
DECLARE_SIGNATURE1(void,v_NMIPADDRESS_Sig,TNmIPAddress &)
DECLARE_SIGNATURE1(int,i_DATETIMECHANGE_Sig,TDateTimeChange &)
DECLARE_SIGNATURE1(int,i_NMDATETIMEFORMAT_Sig,TDateTimeFormat &)
DECLARE_SIGNATURE1(int,i_NMDATETIMEFORMATQUERY_Sig,TDateTimeFormatQuery &)
DECLARE_SIGNATURE1(int,i_NMDATETIMESTRING_Sig,TDateTimeString &)
DECLARE_SIGNATURE1(int,i_NMDATETIMEWMKEYDOWN_Sig,TDateTimeKeyDown &)
DECLARE_SIGNATURE1(void,v_KEYDOWNNOTIFY_Sig,TTabKeyDown &)
DECLARE_SIGNATURE1(bool,b_ENDROPFILES_Sig,TEnDropFiles &)
DECLARE_SIGNATURE1(bool,b_MSGFILTER_Sig,TMsgFilter &)
DECLARE_SIGNATURE1(bool,b_PROTECTED_Sig,TEnProtected &)
DECLARE_SIGNATURE1(void,v_REQRESIZE_Sig,TReqResize &)
DECLARE_SIGNATURE1(bool,b_SELCHANGE_Sig,TSelChange &)

#endif

//----------------------------------------------------------------------------
/// \name Common control notification macros
/// @{

#if OWL_EV_SIGNATURE_CHECK

//
/// Response table entry macro for WM_NOTIFY messages.
/// Looks up the correct dispatcher for the given \p notificationCode.
//
#define OWL_EV_NOTIFICATION(notificationCode, sourceId, method)\
  {{static_cast<::owl::uint>(notificationCode)}, static_cast<::owl::uint>(sourceId),\
  (::owl::CheckSignature<TMyClass, WM_NOTIFY, static_cast<::owl::uint>(notificationCode), ::owl::TDispatch>(&TMyClass::method),\
  OWL_DISPATCH(::owl::TDispatch<WM_NOTIFY>::TNotificationDispatch<static_cast<::owl::uint>(notificationCode)>::Decode, method))}

#else

//
/// Response table entry macro for WM_NOTIFY messages.
/// Looks up the correct dispatcher for the given \p notificationCode.
//
#define OWL_EV_NOTIFICATION(notificationCode, sourceId, method)\
  {{static_cast<::owl::uint>(notificationCode)}, static_cast<::owl::uint>(sourceId),\
  OWL_DISPATCH(::owl::TDispatch<WM_NOTIFY>::TNotificationDispatch<static_cast<::owl::uint>(notificationCode)>::Decode, method)}

#endif

//
/// Response table entry macro for WM_NOTIFY messages handled at the child.
/// Looks up the correct dispatcher for the given \p notificationCode.
//
#define OWL_EV_NOTIFICATION_AT_CHILD(notificationCode, method)\
  OWL_EV_NOTIFICATION(notificationCode, UINT_MAX, method)

//
/// void method(TNmChar&)
//
#define EV_NM_CHAR(id, method) OWL_EV_NOTIFICATION(NM_CHAR, id, method)

//
/// void method()
//
#define EV_NM_CLICK(id, method) OWL_EV_NOTIFICATION(NM_CLICK, id, method)

//
/// int method(TNmCustomDraw&)
//
#define EV_NM_CUSTOMDRAW(id, method) OWL_EV_NOTIFICATION(NM_CUSTOMDRAW, id, method)

//
/// void method()
//
#define EV_NM_DBLCLK(id, method) OWL_EV_NOTIFICATION(NM_DBLCLK, id, method)

//
/// bool method(TNotify&)
//
#define EV_NM_HOVER(id, method) OWL_EV_NOTIFICATION(NM_HOVER, id, method)

//
/// bool method(TNmKey&)
//
#define EV_NM_KEYDOWN(id, method) OWL_EV_NOTIFICATION(NM_KEYDOWN, id, method)

//
/// void method()
//
#define EV_NM_KILLFOCUS(id, method) OWL_EV_NOTIFICATION(NM_KILLFOCUS, id, method)

//
/// int method(TNmMouse&)
//
#define EV_NM_NCHITTEST(id, method) OWL_EV_NOTIFICATION(NM_NCHITTEST, id, method)

//
/// void method()
//
#define EV_NM_OUTOFMEMORY(id, method) OWL_EV_NOTIFICATION(NM_OUTOFMEMORY, id, method)

//
/// void method()
//
#define EV_NM_RCLICK(id, method) OWL_EV_NOTIFICATION(NM_RCLICK, id, method)

//
/// void method()
//
#define EV_NM_RDBLCLK(id, method) OWL_EV_NOTIFICATION(NM_RDBLCLK, id, method)

//
/// void method()
//
#define EV_NM_RELEASEDCAPTURE(id, method) OWL_EV_NOTIFICATION(NM_RELEASEDCAPTURE, id, method)

//
/// void method()
//
#define EV_NM_RETURN(id, method) OWL_EV_NOTIFICATION(NM_RETURN, id, method)

//
/// int method(TNmMouse&)
//
#define EV_NM_SETCURSOR(id, method) OWL_EV_NOTIFICATION(NM_SETCURSOR, id, method)

//
/// void method()
//
#define EV_NM_SETFOCUS(id, method) OWL_EV_NOTIFICATION(NM_SETFOCUS, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name Animation control notifications
/// @{

//
/// void method()
//
#define EV_ACN_START(id, method) OWL_EV_NOTIFICATION(ACN_START, id, method)

//
/// void method()
//
#define EV_ACN_STOP(id, method) OWL_EV_NOTIFICATION(ACN_STOP, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name Header control notifications
/// @{

//
/// bool method(THdrNotify&)
//
#define EV_HDN_BEGINDRAG(id, method) OWL_EV_NOTIFICATION(HDN_BEGINDRAG, id, method)

//
/// bool method(THdrNotify&)
//
#define EV_HDN_BEGINTRACK(id, method) OWL_EV_NOTIFICATION(HDN_BEGINTRACK, id, method)

//
/// void method(THdrNotify&)
//
#define EV_HDN_DIVIDERDBLCLICK(id, method) OWL_EV_NOTIFICATION(HDN_DIVIDERDBLCLICK, id, method)

//
/// bool method(THdrNotify&)
//
#define EV_HDN_ENDDRAG(id, method) OWL_EV_NOTIFICATION(HDN_ENDDRAG, id, method)

//
/// void method(THdrNotify&)
//
#define EV_HDN_ENDTRACK(id, method) OWL_EV_NOTIFICATION(HDN_ENDTRACK, id, method)


//
/// bool method(THdrFilterBtnClick&)
//
#define EV_HDN_FILTERBTNCLICK(id, method) OWL_EV_NOTIFICATION(HDN_FILTERBTNCLICK, id, method)

//
/// void method(THdrNotify&)
//
#define EV_HDN_FILTERCHANGE(id, method) OWL_EV_NOTIFICATION(HDN_FILTERCHANGE, id, method)



//
/// void method(THdrNotify&)
//
#define EV_HDN_GETDISPINFO(id, method) OWL_EV_NOTIFICATION(HDN_GETDISPINFO, id, method)

//
/// void method(THdrNotify&)
//
#define EV_HDN_ITEMCHANGED(id, method) OWL_EV_NOTIFICATION(HDN_ITEMCHANGED, id, method)

//
/// void method(THdrNotify&)
//
#define EV_HDN_ITEMCHANGING(id, method) OWL_EV_NOTIFICATION(HDN_ITEMCHANGING, id, method)

//
/// void method(THdrNotify&)
//
#define EV_HDN_ITEMCLICK(id, method) OWL_EV_NOTIFICATION(HDN_ITEMCLICK, id, method)

//
/// void method(THdrNotify&)
//
#define EV_HDN_ITEMDBLCLICK(id, method) OWL_EV_NOTIFICATION(HDN_ITEMDBLCLICK, id, method)

//
/// bool method(THdrNotify&)
//
#define EV_HDN_TRACK(id, method) OWL_EV_NOTIFICATION(HDN_TRACK, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name TDateTimepicker control notifications
/// @{

//
/// void method()
//
#define EV_DTN_CLOSEUP(id, method) OWL_EV_NOTIFICATION(DTN_CLOSEUP, id, method)

//
/// int method(TDateTimeChange&)
/// The owner of the control must return zero.
//
#define EV_DTN_DATETIMECHANGE(id, method) OWL_EV_NOTIFICATION(DTN_DATETIMECHANGE, id, method)

//
/// void method()
//
#define EV_DTN_DROPDOWN(id, method) OWL_EV_NOTIFICATION(DTN_DROPDOWN, id, method)

//
/// int method(TDateTimeFormat&)
/// The owner of the control must return zero.
//
#define EV_DTN_FORMAT(id, method) OWL_EV_NOTIFICATION(DTN_FORMAT, id, method)

//
/// int method(TDateTimeFormat&)
/// The owner of the control must return zero.
//
#define EV_DTN_FORMATQUERY(id, method) OWL_EV_NOTIFICATION(DTN_FORMATQUERY, id, method)

//
/// int method(TDateTimeString&)
/// The owner of the control must return zero.
//
#define EV_DTN_USERSTRING(id, method) OWL_EV_NOTIFICATION(DTN_USERSTRING, id, method)

//
/// int method(TDateTimeKeyDown&)
/// The owner of the control must return zero.
//
#define EV_DTN_WMKEYDOWN(id, method) OWL_EV_NOTIFICATION(DTN_WMKEYDOWN, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name Tab control notifications
/// @{

//
/// void method(TNmObjectNotify&)
//
#define EV_TCN_GETOBJECT(id, method) OWL_EV_NOTIFICATION(TCN_GETOBJECT, id, method)

//
/// void method(TTabKeyDown&)
//
#define EV_TCN_KEYDOWN(id, method) OWL_EV_NOTIFICATION(TCN_KEYDOWN, id, method)

//
/// void method(TNotify&)
//
#define EV_TCN_SELCHANGE(id, method) OWL_EV_NOTIFICATION(TCN_SELCHANGE, id, method)

//
/// bool method(TNotify&)
//
#define EV_TCN_SELCHANGING(id, method) OWL_EV_NOTIFICATION(TCN_SELCHANGING, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name Rich Edit control notifications
/// @{

//
/// bool method(TEnDropFiles&)
//
#define EV_EN_DROPFILES(id, method) OWL_EV_NOTIFICATION(EN_DROPFILES, id, method)

//
/// bool method(TMsgFilter&)
//
#define EV_EN_MSGFILTER(id, method) OWL_EV_NOTIFICATION(EN_MSGFILTER, id, method)

//
/// bool method(TEnProtected&)
//
#define EV_EN_PROTECTED(id, method) OWL_EV_NOTIFICATION(EN_PROTECTED, id, method)

//
/// void method(TReqResize&)
//
#define EV_EN_REQRESIZE(id, method) OWL_EV_NOTIFICATION(EN_REQUESTRESIZE, id, method)

//
/// bool method(TSelChange&)
//
#define EV_EN_SELCHANGE(id, method) OWL_EV_NOTIFICATION(EN_SELCHANGE, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name UpDown control notifications
/// @{

//
/// bool method(TNmUpDown&)
//
#define EV_UDN_DELTAPOS(id, method) OWL_EV_NOTIFICATION(UDN_DELTAPOS, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name Tool Tip control notifications
/// @{

//
/// void method(TTooltipDispInfo&)
//
#define EV_TTN_GETDISPINFO(id, method) OWL_EV_NOTIFICATION(TTN_GETDISPINFO, id, method)

//
/// void method(TTooltipDispInfo&)
/// Note: This notification code is identical to TTN_GETDISPINFO.
//
#define EV_TTN_NEEDTEXT(id, method) EV_TTN_GETDISPINFO(id, method)

//
/// void method(TNotify&)
//
#define EV_TTN_POP(id, method) OWL_EV_NOTIFICATION(TTN_POP, id, method)

//
/// void method(TNotify&)
//
#define EV_TTN_SHOW(id, method) OWL_EV_NOTIFICATION(TTN_SHOW, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name Tree View control notifications
/// @{

//
/// void method(TTvNotify&)
//
#define EV_TVN_BEGINDRAG(id, method) OWL_EV_NOTIFICATION(TVN_BEGINDRAG, id, method)

//
/// bool method(TTvDispInfoNotify&)
//
#define EV_TVN_BEGINLABELEDIT(id, method) OWL_EV_NOTIFICATION(TVN_BEGINLABELEDIT, id, method)

//
/// void method(TTvNotify&)
//
#define EV_TVN_BEGINRDRAG(id, method) OWL_EV_NOTIFICATION(TVN_BEGINRDRAG, id, method)

//
/// void method(TTvNotify&)
//
#define EV_TVN_DELETEITEM(id, method) OWL_EV_NOTIFICATION(TVN_DELETEITEM, id, method)

//
/// void method(TTvDispInfoNotify&)
//
#define EV_TVN_ENDLABELEDIT(id, method) OWL_EV_NOTIFICATION(TVN_ENDLABELEDIT, id, method)

//
/// void method(TTvDispInfoNotify&)
//
#define EV_TVN_GETDISPINFO(id, method) OWL_EV_NOTIFICATION(TVN_GETDISPINFO, id, method)

//
/// void method(TTvGetInfoTip&)
//
#define EV_TVN_GETINFOTIP(id, method) OWL_EV_NOTIFICATION(TVN_GETINFOTIP, id, method)

//
/// void method(TTvNotify&)
//
#define EV_TVN_ITEMEXPANDED(id, method) OWL_EV_NOTIFICATION(TVN_ITEMEXPANDED, id, method)

//
/// bool method(TTvNotify&)
//
#define EV_TVN_ITEMEXPANDING(id, method) OWL_EV_NOTIFICATION(TVN_ITEMEXPANDING, id, method)

//
/// void method(TTvKeyDownNotify&)
//
#define EV_TVN_KEYDOWN(id, method) OWL_EV_NOTIFICATION(TVN_KEYDOWN, id, method)

//
/// void method(TTvNotify&)
//
#define EV_TVN_SELCHANGED(id, method) OWL_EV_NOTIFICATION(TVN_SELCHANGED, id, method)

//
/// bool method(TTvNotify&)
//
#define EV_TVN_SELCHANGING(id, method) OWL_EV_NOTIFICATION(TVN_SELCHANGING, id, method)

//
/// void method(TTvDispInfoNotify&)
//
#define EV_TVN_SETDISPINFO(id, method) OWL_EV_NOTIFICATION(TVN_SETDISPINFO, id, method)

//
/// void method(TTvNotify&)
//
#define EV_TVN_SINGLEEXPAND(id, method) OWL_EV_NOTIFICATION(TVN_SINGLEEXPAND, id, method)

//
/// void method(NMTVSTATEIMAGECHANGING&)
//
#define EV_NM_TVSTATEIMAGECHANGING(id, method) OWL_EV_NOTIFICATION(NM_TVSTATEIMAGECHANGING, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name List View control notifications
/// @{

//
/// void method(TLvNotify&)
//
#define EV_LVN_BEGINDRAG(id, method) OWL_EV_NOTIFICATION(LVN_BEGINDRAG, id, method)

//
/// bool method(TLvDispInfoNotify&)
//
#define EV_LVN_BEGINLABELEDIT(id, method) OWL_EV_NOTIFICATION(LVN_BEGINLABELEDIT, id, method)

//
/// void method(TLvNotify&)
//
#define EV_LVN_BEGINRDRAG(id, method) OWL_EV_NOTIFICATION(LVN_BEGINRDRAG, id, method)

//
/// void method(TLvNotify&)
//
#define EV_LVN_COLUMNCLICK(id, method) OWL_EV_NOTIFICATION(LVN_COLUMNCLICK, id, method)

//
/// void method()
//
#define EV_LVN_DBLCLK(id, method) EV_NM_DBLCLK(id, method)


//
/// void method(TLvNotify&)
//
#define EV_LVN_DELETEALLITEMS(id, method) OWL_EV_NOTIFICATION(LVN_DELETEALLITEMS, id, method)

//
/// void method(TLvNotify&)
//
#define EV_LVN_DELETEITEM(id, method) OWL_EV_NOTIFICATION(LVN_DELETEITEM, id, method)

//
/// bool method(TLvDispInfoNotify&)
//
#define EV_LVN_ENDLABELEDIT(id, method) OWL_EV_NOTIFICATION(LVN_ENDLABELEDIT, id, method)

//
/// void method(TLvDispInfoNotify&)
//
#define EV_LVN_GETDISPINFO(id, method) OWL_EV_NOTIFICATION(LVN_GETDISPINFO, id, method)

//
/// void method(TLvGetInfoTip&)
//
#define EV_LVN_GETINFOTIP(id, method) OWL_EV_NOTIFICATION(LVN_GETINFOTIP, id, method)

//
/// bool method(TLvNotify&)
//
#define EV_LVN_HOTTRACK(id, method) OWL_EV_NOTIFICATION(LVN_HOTTRACK, id, method)

//
/// void method(TLvNotify&)
//
#define EV_LVN_INSERTITEM(id, method) OWL_EV_NOTIFICATION(LVN_INSERTITEM, id, method)

//
/// void method(TLvItemActivate&)
//
#define EV_LVN_ITEMACTIVATE(id, method) OWL_EV_NOTIFICATION(LVN_ITEMACTIVATE, id, method)

//
/// void method(TLvNotify&)
//
#define EV_LVN_ITEMCHANGED(id, method) OWL_EV_NOTIFICATION(LVN_ITEMCHANGED, id, method)

//
/// bool method(TLvNotify&)
//
#define EV_LVN_ITEMCHANGING(id, method) OWL_EV_NOTIFICATION(LVN_ITEMCHANGING, id, method)

//
/// void method(TLvKeyDownNotify&)
//
#define EV_LVN_KEYDOWN(id, method) OWL_EV_NOTIFICATION(LVN_KEYDOWN, id, method)

//
/// bool method(TLvNotify&)
//
#define EV_LVN_MARQUEEBEGIN(id, method) OWL_EV_NOTIFICATION(LVN_MARQUEEBEGIN, id, method)

//
/// bool method(TLvCacheHint&)
//
#define EV_LVN_ODCACHEHINT(id, method) OWL_EV_NOTIFICATION(LVN_ODCACHEHINT, id, method)

//
/// int method(TLvFindItem&)
//
#define EV_LVN_ODFINDITEM(id, method) OWL_EV_NOTIFICATION(LVN_ODFINDITEM, id, method)

//
/// bool method(TLvOdStateChanged&)
//
#define EV_LVN_ODSTATECHANGED(id, method) OWL_EV_NOTIFICATION(LVN_ODSTATECHANGED, id, method)

//
/// void method(TLvDispInfoNotify&)
//
#define EV_LVN_SETDISPINFO(id, method) OWL_EV_NOTIFICATION(LVN_SETDISPINFO, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name Rebar control notifications
/// @{

//
/// void method(TNotify&)
//
#define EV_RBN_AUTOSIZE(id, method) OWL_EV_NOTIFICATION(RBN_AUTOSIZE, id, method)

//
/// bool method(TNotify&)
//
#define EV_RBN_BEGINDRAG(id, method) OWL_EV_NOTIFICATION(RBN_BEGINDRAG, id, method)

//
/// void method(TNotify&)
//
#define EV_RBN_CHILDSIZE(id, method) OWL_EV_NOTIFICATION(RBN_CHILDSIZE, id, method)

//
/// void method(TNotify&)
//
#define EV_RBN_DELETEDBAND(id, method) OWL_EV_NOTIFICATION(RBN_DELETEDBAND, id, method)

//
/// void method(TNotify&)
//
#define EV_RBN_DELETINGBAND(id, method) OWL_EV_NOTIFICATION(RBN_DELETINGBAND, id, method)

//
/// void method(TNotify&)
//
#define EV_RBN_ENDDRAG(id, method) OWL_EV_NOTIFICATION(RBN_ENDDRAG, id, method)

//
/// bool method(TNotify&)
/// Must return true.
//
#define EV_RBN_GETOBJECT(id, method) OWL_EV_NOTIFICATION(RBN_GETOBJECT, id, method)

//
/// void method(TNotify&)
//
#define EV_RBN_HEIGHTCHANGE(id, method) OWL_EV_NOTIFICATION(RBN_HEIGHTCHANGE, id, method)

//
/// void method(TNotify&)
//
#define EV_RBN_LAYOUTCHANGED(id, method) OWL_EV_NOTIFICATION(RBN_LAYOUTCHANGED, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name ComboBoxEx control notifications
/// @{

//
/// void method(TNotify&)
//
#define EV_CBEN_BEGINEDIT(id, method) OWL_EV_NOTIFICATION(CBEN_BEGINEDIT, id, method)

//
/// bool method(TCBExItemInfo&)
//
#define EV_CBEN_DELETEITEM(id, method) OWL_EV_NOTIFICATION(CBEN_DELETEITEM, id, method)

//
/// void method(TCBExDragInfo&)
//
#define EV_CBEN_DRAGBEGIN(id, method) OWL_EV_NOTIFICATION(CBEN_DRAGBEGIN, id, method)

//
/// bool method(TCBExEditInfo&)
//
#define EV_CBEN_ENDEDIT(id, method) OWL_EV_NOTIFICATION(CBEN_ENDEDIT, id, method)

//
/// bool method(TCBExItemInfo&)
//
#define EV_CBEN_GETDISPINFO(id, method) OWL_EV_NOTIFICATION(CBEN_GETDISPINFO, id, method)

//
/// bool method(TCBExItemInfo&)
//
#define EV_CBEN_INSERTITEM(id, method) OWL_EV_NOTIFICATION(CBEN_INSERTITEM, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name Month Calendar control notifications
/// @{

//
/// void method(TNmDayState&)
//
#define EV_MCN_GETDAYSTATE(id, method) OWL_EV_NOTIFICATION(MCN_GETDAYSTATE, id, method)

//
/// void method(TNmSelChange&)
//
#define EV_MCN_SELCHANGE(id, method) OWL_EV_NOTIFICATION(MCN_SELCHANGE, id, method)

//
/// void method(TNmSelChange&)
//
#define EV_MCN_SELECT(id, method) OWL_EV_NOTIFICATION(MCN_SELECT, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name Pager control notifications
/// @{

//
/// void method(TNmPGCalcSize&)
//
#define EV_PGN_CALCSIZE(id, method) OWL_EV_NOTIFICATION(PGN_CALCSIZE, id, method)

//
/// void method(TNmPGScroll&)
//
#define EV_PGN_SCROLL(id, method) OWL_EV_NOTIFICATION(PGN_SCROLL, id, method)

/// @}

//----------------------------------------------------------------------------
/// \name IP Address control notifications
/// @{

//
/// void method(TNmIPAddress&)
//
#define EV_IPN_FIELDCHANGED(id, method) OWL_EV_NOTIFICATION(IPN_FIELDCHANGED, id, method)

/// @}

//----------------------------------------------------------------------------
// Inline implemenations
//

//
/// Constructor to create a TNotify object (NMHDR wrapper) from the specified
/// window handle, control id and notification code.
//
inline
TNotify::TNotify(HWND ctl, uint id, uint ctlCode) {
  hwndFrom = ctl;
  idFrom = id;
  code = ctlCode;
}

//
/// Constructor to create a TNotify object whose members are initialized to
/// zero.
//
inline
TNotify::TNotify() {
  hwndFrom = nullptr;
  idFrom = 0;
  code = 0;
}

inline
TNmUpDown::TNmUpDown(HWND hwnd, uint id, uint code, int pos, int delta)
{
  iPos = pos;
  iDelta = delta;
  hdr.hwndFrom = hwnd;
  hdr.idFrom = id;
  hdr.code = code;
}

//
/// TTabKeyDown contains information about a key press in a tab control.
/// This constructor initializes the structure NUL value for the
/// virtual wVKey and flags members.
//
inline
TTabKeyDown::TTabKeyDown() {
  wVKey = 0;
  flags = 0;
}

//
/// TTabKeyDown contains information about a key press in a tab control.
/// This constructor initializes the members to the specified parameters.
//
inline
TTabKeyDown::TTabKeyDown(HWND ctl, uint id, uint code, uint16 vk, uint flg)
{
  hdr.hwndFrom = ctl;
  hdr.idFrom = id;
  hdr.code = code;
  wVKey = vk;
  flags = flg;
}


inline void
TRebarInfo::SetImageList(HIMAGELIST Images)
{
  fMask |= RBIM_IMAGELIST;
  himl = Images;
}

inline HIMAGELIST
TRebarInfo::GetImageList()
{
  return himl;
}

} // OWL namespace

OWL_DISABLE_WARNING_POP

#endif  // OWL_COMMCTRL_H

