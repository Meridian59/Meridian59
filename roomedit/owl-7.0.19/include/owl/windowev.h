//
/// \file
/// Event response table macros for windows messages
//
// Part of OWLNext - the next generation Object Windows Library
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
// Copyright (c) 2013 Vidar Hasfjord
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//

#if !defined(OWL_WINDOWEV_H)
#define OWL_WINDOWEV_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/eventhan.h>

#if OWL_EV_SIGNATURE_CHECK

//
/// Response table entry for raw message handling
/// Uses a dispatcher that just forwards WPARAM and LPARAM.
///
/// Required method signature:
/// TResult method(TParam1, TParam2)
//
#define EV_MESSAGE(msgId, method)\
  OWL_ID_EV_GENERIC_(msgId, msgId, 0, method, ::owl::TDispatchRawArguments)

//
/// Resonse table entry for a registered message
///
/// Required method signature:
/// TResult method(TParam1, TParam2)
//
#define EV_REGISTERED(str, method)\
  OWL_ID_EV_GENERIC_(WM_NULL, ::RegisterWindowMessage(str), 0, method, ::owl::TDispatchRawArguments)

//
/// Response table entry for a child ID notification handled at the child's parent
///
/// Required method signature:
/// void method()
//
#define EV_CHILD_NOTIFY(id, notifyCode, method)\
  OWL_ID_EV_GENERIC_(WM_COMMAND, notifyCode, id, method, ::owl::TDispatchChildNotify)

//
/// Response table entry for a child ID notification handled at the child's parent
///
/// The notification code is passed in as an argument; this allows multiple notifications to be
/// handled by a single response function.
///
/// Required method signature:
/// void method(uint notifyCode)
//
#define EV_CHILD_NOTIFY_AND_CODE(id, notifyCode, method)\
  OWL_ID_EV_GENERIC_(WM_COMMAND, notifyCode, id, method, ::owl::TDispatchChildNotifyWithCode)

//
/// Generic handler for child ID notification: rather than specify the specific notification codes
/// you are interested in, ALL notifications from the child are passed to the response function.
///
/// Required method signature:
/// void method(uint notifyCode)
//
#define EV_CHILD_NOTIFY_ALL_CODES(id, method)\
  OWL_ID_EV_GENERIC_(WM_COMMAND, UINT_MAX, id, method, ::owl::TDispatchChildNotifyWithCode)

//
/// Response table entry for a child ID notification handled at the child
///
/// Required method signature:
/// void method()
//
#define EV_NOTIFY_AT_CHILD(notifyCode, method)\
  OWL_ID_EV_GENERIC_(WM_COMMAND, notifyCode, UINT_MAX, method, ::owl::TDispatchChildNotify)

//
/// Response table entry for a menu/accelerator/push button message
///
/// Required method signature:
/// void method()
//
#define EV_COMMAND(id, method)\
  OWL_ID_EV_GENERIC_(WM_COMMAND, 0, id, method, ::owl::TDispatchCommand)

//
/// Response table entry for a menu/accelerator/push button message
/// The menu id is passed in as an argument; this allows multiple commands to be handled by a
/// single response function.
///
/// Required method signature:
/// void method(uint commandId)
//
#define EV_COMMAND_AND_ID(id, method)\
  OWL_ID_EV_GENERIC_(WM_COMMAND, 0, id, method, ::owl::TDispatchCommandWithId)

#else

//
/// Response table entry for raw message handling
/// Uses a dispatcher that just forwards WPARAM and LPARAM.
///
/// Required method signature:
/// TResult method(TParam1, TParam2)
//
#define EV_MESSAGE(message, method)\
  {{static_cast<::owl::TMsgId>(message)}, 0, OWL_DISPATCH(::owl::DispatchRawArguments, method)}

//
/// Resonse table entry for a registered message
///
/// Required method signature:
/// TResult method(TParam1, TParam2)
//
#define EV_REGISTERED(str, method)\
  EV_MESSAGE(::RegisterWindowMessage(str), method)

//
/// Response table entry for a child ID notification handled at the child's parent
///
/// Required method signature:
/// void method()
//
#define EV_CHILD_NOTIFY(id, notifyCode, method)\
  {{static_cast<::owl::uint>(notifyCode)}, static_cast<::owl::uint>(id), OWL_DISPATCH(::owl::DispatchChildNotify, method)}

//
/// Response table entry for a child ID notification handled at the child's parent
///
/// The notification code is passed in as an argument; this allows multiple notifications to be
/// handled by a single response function.
///
/// Required method signature:
/// void method(uint notifyCode)
//
#define EV_CHILD_NOTIFY_AND_CODE(id, notifyCode, method)\
  {{static_cast<::owl::uint>(notifyCode)}, static_cast<::owl::uint>(id), OWL_DISPATCH(::owl::DispatchChildNotifyWithCode, method)}

//
/// Generic handler for child ID notification: rather than specify the specific notification codes
/// you are interested in, ALL notifications from the child are passed to the response function.
///
/// Required method signature:
/// void method(uint notifyCode)
//
#define EV_CHILD_NOTIFY_ALL_CODES(id, method)\
  {{UINT_MAX}, static_cast<::owl::uint>(id), OWL_DISPATCH(::owl::DispatchChildNotifyWithCode, method)}

//
/// Response table entry for a child ID notification handled at the child
///
/// Required method signature:
/// void method()
//
#define EV_NOTIFY_AT_CHILD(notifyCode, method)\
  {{static_cast<::owl::uint>(notifyCode)}, UINT_MAX, OWL_DISPATCH(::owl::DispatchChildNotify, method)}

//
/// Response table entry for a menu/accelerator/push button message
///
/// Required method signature:
/// void method()
//
#define EV_COMMAND(id, method)\
  {{0}, static_cast<::owl::uint>(id), OWL_DISPATCH(::owl::DispatchCommand, method)}

//
/// Response table entry for a menu/accelerator/push button message
/// The menu id is passed in as an argument; this allows multiple commands to be handled by a
/// single response function.
///
/// Required method signature:
/// void method(uint commandId)
//
#define EV_COMMAND_AND_ID(id, method)\
  {{0}, static_cast<::owl::uint>(id), OWL_DISPATCH(::owl::DispatchCommandWithId, method)}

#endif

//
/// Response table entry for enabling a command
///
/// Required method signature:
/// void method(TCommandEnabler&)
//
#define EV_COMMAND_ENABLE(id, method)\
  OWL_ID_EV_(WM_COMMAND_ENABLE, id, method)

//
/// \name Document Manager Messages
/// These macros handle messages generated by the document manager:
/// @{

#define EV_OWLDOCUMENT(id, method) OWL_ID_EV_(WM_OWLDOCUMENT, id, method)
#define EV_OWLVIEW(id, method) OWL_ID_EV_(WM_OWLVIEW, id, method)
#define EV_OWLNOTIFY(id, method) OWL_ID_EV_(WM_OWLNOTIFY, id, method)
#define EV_VIEWNOTIFY(id, method) EV_OWLNOTIFY(id, method)

/// @}

#define EV_WM_CHILDINVALID OWL_EV_(WM_CHILDINVALID, EvChildInvalid)

//
/// Response table entry for the OWL frame sizing message
//
#define EV_OWLFRAMESIZE OWL_EV_(WM_OWLFRAMESIZE, EvOwlFrameSize)

//
/// Response table entry for the OWL Docking message
//
#define EV_WM_OWLWINDOWDOCKED OWL_EV_(WM_OWLWINDOWDOCKED, EvOwlWindowDocked)
#define EV_OWLWINDOWDOCKED EV_WM_OWLWINDOWDOCKED

//
/// Response table entry for the OWL Help manager message
//
#define EV_OWLHELPHIT OWL_EV_(WM_OWLHELPHIT, EvHelpHit)

/// \name Standard Windows Messages
/// These macros handle Windows messages. To determine the name of the Windows message that
/// corresponds to the macro, remove the EV_ prefix. For example, WM_ACTIVATE is the name of the
/// Windows message that the EV_WM_ACTIVATE macro handles. These macros take no arguments. They
/// all assume the event handler function has a corresponding predefined name. For example, the
/// predefined event handler function name for WM_ACTIVATE is EvActivate.
///
/// \note Messages WM_COMMAND and WM_NOTIFY are not dispatched directly. Instead, they are handled
/// in the virtual functions TWindow::EvCommand and TWindow::EvNotify, respectively. There the 
/// messages are partly decoded, identifying the notification code and sender, and then dispatched
/// accordingly, i.e. the response tables are searched for an entry that matches the notification
/// and/or sender. Response table entries for notification messages are created using separate sets
/// of macros, such as EV_COMMAND, EV_BN_CLICKED, EV_NM_CHAR, EV_EN_SELCHANGE. If you need to
/// handle WM_COMMAND or WM_NOTIFY directly, override TWindow::EvCommand or TWindow::EvNotify,
/// respectively.
/// @{

#define EV_WM_ACTIVATE OWL_EV_(WM_ACTIVATE, EvActivate)
#define EV_WM_ACTIVATEAPP OWL_EV_(WM_ACTIVATEAPP, EvActivateApp)
#define EV_WM_ASKCBFORMATNAME OWL_EV_(WM_ASKCBFORMATNAME, EvAskCBFormatName)
#define EV_WM_CANCELMODE OWL_EV_(WM_CANCELMODE, EvCancelMode)
#define EV_WM_CAPTURECHANGED OWL_EV_(WM_CAPTURECHANGED, EvCaptureChanged)
#define EV_WM_CHANGECBCHAIN OWL_EV_(WM_CHANGECBCHAIN, EvChangeCBChain)
#define EV_WM_CHAR OWL_EV_(WM_CHAR, EvChar)
#define EV_WM_CHARTOITEM OWL_EV_(WM_CHARTOITEM, EvCharToItem)
#define EV_WM_CHILDACTIVATE OWL_EV_(WM_CHILDACTIVATE, EvChildActivate)
#define EV_WM_CLOSE OWL_EV_(WM_CLOSE, EvClose)
#define EV_WM_COMPACTING OWL_EV_(WM_COMPACTING, EvCompacting)
#define EV_WM_COMPAREITEM OWL_EV_(WM_COMPAREITEM, EvCompareItem)
#define EV_WM_CONTEXTMENU OWL_EV_(WM_CONTEXTMENU, EvContextMenu)
#define EV_WM_COPYDATA OWL_EV_(WM_COPYDATA, EvCopyData)
#define EV_WM_CREATE OWL_EV_(WM_CREATE, EvCreate)
#define EV_WM_CTLCOLORBTN(method) OWL_EV_(WM_CTLCOLORBTN, method)
#define EV_WM_CTLCOLOREDIT(method) OWL_EV_(WM_CTLCOLOREDIT, method)
#define EV_WM_CTLCOLORDLG(method) OWL_EV_(WM_CTLCOLORDLG, method)
#define EV_WM_CTLCOLORLISTBOX(method) OWL_EV_(WM_CTLCOLORLISTBOX, method)

#if defined(OWL5_COMPAT)

//
/// This macro is deprecated.
/// \sa TDispatch<WM_CTLCOLORMSGBOX>.
//
#define EV_WM_CTLCOLORMSGBOX(method) OWL_EV_(WM_CTLCOLORMSGBOX, method)

#endif

#define EV_WM_CTLCOLORSCROLLBAR(method) OWL_EV_(WM_CTLCOLORSCROLLBAR, method)
#define EV_WM_CTLCOLORSTATIC(method) OWL_EV_(WM_CTLCOLORSTATIC, method)
#define EV_WM_CUT OWL_EV_(WM_CUT, EvCut)
#define EV_WM_DEADCHAR OWL_EV_(WM_DEADCHAR, EvDeadChar)
#define EV_WM_DELETEITEM OWL_EV_(WM_DELETEITEM, EvDeleteItem)
#define EV_WM_DESTROY OWL_EV_(WM_DESTROY, EvDestroy)
#define EV_WM_DESTROYCLIPBOARD OWL_EV_(WM_DESTROYCLIPBOARD, EvDestroyClipboard)
#define EV_WM_DEVICECHANGE OWL_EV_(WM_DEVICECHANGE, EvDeviceChange)
#define EV_WM_DEVMODECHANGE OWL_EV_(WM_DEVMODECHANGE, EvDevModeChange)
#define EV_WM_DISPLAYCHANGE OWL_EV_(WM_DISPLAYCHANGE, EvDisplayChange)
#define EV_WM_DRAWCLIPBOARD OWL_EV_(WM_DRAWCLIPBOARD, EvDrawClipboard)
#define EV_WM_DRAWITEM OWL_EV_(WM_DRAWITEM, EvDrawItem)
#define EV_WM_DROPFILES OWL_EV_(WM_DROPFILES, EvDropFiles)
#define EV_WM_ENABLE OWL_EV_(WM_ENABLE, EvEnable)
#define EV_WM_ENDSESSION OWL_EV_(WM_ENDSESSION, EvEndSession)
#define EV_WM_ENTERIDLE OWL_EV_(WM_ENTERIDLE, EvEnterIdle)
#define EV_WM_ENTERMENULOOP OWL_EV_(WM_ENTERMENULOOP, EvEnterMenuLoop)
#define EV_WM_ENTERSIZEMOVE OWL_EV_(WM_ENTERSIZEMOVE, EvEnterSizeMove)
#define EV_WM_ERASEBKGND OWL_EV_(WM_ERASEBKGND, EvEraseBkgnd)
#define EV_WM_EXITMENULOOP OWL_EV_(WM_EXITMENULOOP, EvExitMenuLoop)
#define EV_WM_EXITSIZEMOVE OWL_EV_(WM_EXITSIZEMOVE, EvExitSizeMove)
#define EV_WM_FONTCHANGE OWL_EV_(WM_FONTCHANGE, EvFontChange)
#define EV_WM_GETDLGCODE OWL_EV_(WM_GETDLGCODE, EvGetDlgCode)
#define EV_WM_GETFONT OWL_EV_(WM_GETFONT, EvGetFont)
#define EV_WM_GETICON OWL_EV_(WM_GETICON, EvGetIcon)
#define EV_WM_GETMINMAXINFO OWL_EV_(WM_GETMINMAXINFO, EvGetMinMaxInfo)
#define EV_WM_GETTEXT OWL_EV_(WM_GETTEXT, EvGetText)
#define EV_WM_GETTEXTLENGTH OWL_EV_(WM_GETTEXTLENGTH, EvGetTextLength)
#define EV_WM_HELP OWL_EV_(WM_HELP, EvHelp)
#define EV_WM_HOTKEY OWL_EV_(WM_HOTKEY, EvHotKey)
#define EV_WM_HSCROLL OWL_EV_(WM_HSCROLL, EvHScroll)
#define EV_WM_HSCROLLCLIPBOARD OWL_EV_(WM_HSCROLLCLIPBOARD, EvHScrollClipboard)
#define EV_WM_INITMENU OWL_EV_(WM_INITMENU, EvInitMenu)
#define EV_WM_INITMENUPOPUP OWL_EV_(WM_INITMENUPOPUP, EvInitMenuPopup)
#define EV_WM_INPUTLANGCHANGE OWL_EV_(WM_INPUTLANGCHANGE, EvInputLangChange)
#define EV_WM_INPUTLANGCHANGEREQUEST OWL_EV_(WM_INPUTLANGCHANGEREQUEST, EvInputLangChangeRequest)
#define EV_WM_KEYDOWN OWL_EV_(WM_KEYDOWN, EvKeyDown)
#define EV_WM_KEYUP OWL_EV_(WM_KEYUP, EvKeyUp)
#define EV_WM_KILLFOCUS OWL_EV_(WM_KILLFOCUS, EvKillFocus)
#define EV_WM_LBUTTONDBLCLK OWL_EV_(WM_LBUTTONDBLCLK, EvLButtonDblClk)
#define EV_WM_LBUTTONDOWN OWL_EV_(WM_LBUTTONDOWN, EvLButtonDown)
#define EV_WM_LBUTTONUP OWL_EV_(WM_LBUTTONUP, EvLButtonUp)
#define EV_WM_MBUTTONDBLCLK OWL_EV_(WM_MBUTTONDBLCLK, EvMButtonDblClk)
#define EV_WM_MBUTTONDOWN OWL_EV_(WM_MBUTTONDOWN, EvMButtonDown)
#define EV_WM_MBUTTONUP OWL_EV_(WM_MBUTTONUP, EvMButtonUp)
#define EV_WM_MDIACTIVATE OWL_EV_(WM_MDIACTIVATE, EvMDIActivate)
#define EV_WM_MDICASCADE OWL_EV_(WM_MDICASCADE, EvMDICascade)
#define EV_WM_MDICREATE OWL_EV_(WM_MDICREATE, EvMDICreate)
#define EV_WM_MDIDESTROY OWL_EV_(WM_MDIDESTROY, EvMDIDestroy)
#define EV_WM_MEASUREITEM OWL_EV_(WM_MEASUREITEM, EvMeasureItem)
#define EV_WM_MENUCHAR OWL_EV_(WM_MENUCHAR, EvMenuChar)
#define EV_WM_MENUSELECT OWL_EV_(WM_MENUSELECT, EvMenuSelect)
#define EV_WM_MOUSEACTIVATE OWL_EV_(WM_MOUSEACTIVATE, EvMouseActivate)
#define EV_WM_MOUSEHOVER OWL_EV_(WM_MOUSEHOVER, EvMouseHover)
#define EV_WM_MOUSEHWHEEL OWL_EV_(WM_MOUSEHWHEEL, EvMouseHWheel)
#define EV_WM_MOUSELEAVE OWL_EV_(WM_MOUSELEAVE, EvMouseLeave)
#define EV_WM_MOUSEMOVE OWL_EV_(WM_MOUSEMOVE, EvMouseMove)
#define EV_WM_MOUSEWHEEL OWL_EV_(WM_MOUSEWHEEL, EvMouseWheel)
#define EV_WM_MOVE OWL_EV_(WM_MOVE, EvMove)
#define EV_WM_MOVING OWL_EV_(WM_MOVING, EvMoving)
#define EV_WM_NCACTIVATE OWL_EV_(WM_NCACTIVATE, EvNCActivate)
#define EV_WM_NCCALCSIZE OWL_EV_(WM_NCCALCSIZE, EvNCCalcSize)
#define EV_WM_NCCREATE OWL_EV_(WM_NCCREATE, EvNCCreate)
#define EV_WM_NCDESTROY OWL_EV_(WM_NCDESTROY, EvNCDestroy)
#define EV_WM_NCHITTEST OWL_EV_(WM_NCHITTEST, EvNCHitTest)
#define EV_WM_NCLBUTTONDBLCLK OWL_EV_(WM_NCLBUTTONDBLCLK, EvNCLButtonDblClk)
#define EV_WM_NCLBUTTONDOWN OWL_EV_(WM_NCLBUTTONDOWN, EvNCLButtonDown)
#define EV_WM_NCLBUTTONUP OWL_EV_(WM_NCLBUTTONUP, EvNCLButtonUp)
#define EV_WM_NCMBUTTONDBLCLK OWL_EV_(WM_NCMBUTTONDBLCLK, EvNCMButtonDblClk)
#define EV_WM_NCMBUTTONDOWN OWL_EV_(WM_NCMBUTTONDOWN, EvNCMButtonDown)
#define EV_WM_NCMBUTTONUP OWL_EV_(WM_NCMBUTTONUP, EvNCMButtonUp)
#define EV_WM_NCMOUSEHOVER OWL_EV_(WM_NCMOUSEHOVER, EvNCMouseHover)
#define EV_WM_NCMOUSELEAVE OWL_EV_(WM_NCMOUSELEAVE, EvNCMouseLeave)
#define EV_WM_NCMOUSEMOVE OWL_EV_(WM_NCMOUSEMOVE, EvNCMouseMove)
#define EV_WM_NCPAINT OWL_EV_(WM_NCPAINT, EvNCPaint)
#define EV_WM_NCRBUTTONDBLCLK OWL_EV_(WM_NCRBUTTONDBLCLK, EvNCRButtonDblClk)
#define EV_WM_NCRBUTTONDOWN OWL_EV_(WM_NCRBUTTONDOWN, EvNCRButtonDown)
#define EV_WM_NCRBUTTONUP OWL_EV_(WM_NCRBUTTONUP, EvNCRButtonUp)
#define EV_WM_NEXTDLGCTL OWL_EV_(WM_NEXTDLGCTL, EvNextDlgCtl)
#define EV_WM_NEXTMENU OWL_EV_(WM_NEXTMENU, EvNextMenu)
#define EV_WM_PAINT OWL_EV_(WM_PAINT, EvPaint)
#define EV_WM_PAINTCLIPBOARD OWL_EV_(WM_PAINTCLIPBOARD, EvPaintClipboard)
#define EV_WM_PALETTECHANGED OWL_EV_(WM_PALETTECHANGED, EvPaletteChanged)
#define EV_WM_PALETTEISCHANGING OWL_EV_(WM_PALETTEISCHANGING, EvPaletteIsChanging)
#define EV_WM_PARENTNOTIFY OWL_EV_(WM_PARENTNOTIFY, EvParentNotify)
#define EV_WM_PASTE OWL_EV_(WM_PASTE, EvPaste)
#define EV_WM_POWERBROADCAST OWL_EV_(WM_POWERBROADCAST, EvPowerBroadCast)
#define EV_WM_PRINT OWL_EV_(WM_PRINT, EvPrint)
#define EV_WM_PRINTCLIENT OWL_EV_(WM_PRINTCLIENT, EvPrintClient)
#define EV_WM_QUERYDRAGICON OWL_EV_(WM_QUERYDRAGICON, EvQueryDragIcon)
#define EV_WM_QUERYENDSESSION OWL_EV_(WM_QUERYENDSESSION, EvQueryEndSession)
#define EV_WM_QUERYNEWPALETTE OWL_EV_(WM_QUERYNEWPALETTE, EvQueryNewPalette)
#define EV_WM_QUERYOPEN OWL_EV_(WM_QUERYOPEN, EvQueryOpen)
#define EV_WM_QUEUESYNC OWL_EV_(WM_QUEUESYNC, EvQueueSync)
#define EV_WM_RBUTTONDBLCLK OWL_EV_(WM_RBUTTONDBLCLK, EvRButtonDblClk)
#define EV_WM_RBUTTONDOWN OWL_EV_(WM_RBUTTONDOWN, EvRButtonDown)
#define EV_WM_RBUTTONUP OWL_EV_(WM_RBUTTONUP, EvRButtonUp)
#define EV_WM_RENDERALLFORMATS OWL_EV_(WM_RENDERALLFORMATS, EvRenderAllFormats)
#define EV_WM_RENDERFORMAT OWL_EV_(WM_RENDERFORMAT, EvRenderFormat)
#define EV_WM_SETCURSOR OWL_EV_(WM_SETCURSOR, EvSetCursor)
#define EV_WM_SETICON OWL_EV_(WM_SETICON, EvSetIcon)
#define EV_WM_SETFOCUS OWL_EV_(WM_SETFOCUS, EvSetFocus)
#define EV_WM_SETFONT OWL_EV_(WM_SETFONT, EvSetFont)
#define EV_WM_SETREDRAW OWL_EV_(WM_SETREDRAW, EvSetRedraw)
#define EV_WM_SETTEXT OWL_EV_(WM_SETTEXT, EvSetText)
#define EV_WM_SETTINGCHANGE OWL_EV_(WM_SETTINGCHANGE, EvSettingChange)
#define EV_WM_SHOWWINDOW OWL_EV_(WM_SHOWWINDOW, EvShowWindow)
#define EV_WM_SIZE OWL_EV_(WM_SIZE, EvSize)
#define EV_WM_SIZECLIPBOARD OWL_EV_(WM_SIZECLIPBOARD, EvSizeClipboard)
#define EV_WM_SIZING OWL_EV_(WM_SIZING, EvSizing)
#define EV_WM_SPOOLERSTATUS OWL_EV_(WM_SPOOLERSTATUS, EvSpoolerStatus)
#define EV_WM_STYLECHANGED OWL_EV_(WM_STYLECHANGED, EvStyleChanged)
#define EV_WM_STYLECHANGING OWL_EV_(WM_STYLECHANGING, EvStyleChanging)
#define EV_WM_SYSCHAR OWL_EV_(WM_SYSCHAR, EvSysChar)
#define EV_WM_SYSCOLORCHANGE OWL_EV_(WM_SYSCOLORCHANGE, EvSysColorChange)
#define EV_WM_SYSCOMMAND OWL_EV_(WM_SYSCOMMAND, EvSysCommand)
#define EV_WM_SYSDEADCHAR OWL_EV_(WM_SYSDEADCHAR, EvSysDeadChar)
#define EV_WM_SYSKEYDOWN OWL_EV_(WM_SYSKEYDOWN, EvSysKeyDown)
#define EV_WM_SYSKEYUP OWL_EV_(WM_SYSKEYUP, EvSysKeyUp)
#define EV_WM_TCARD OWL_EV_(WM_TCARD, EvTCard)
#define EV_WM_THEMECHANGED OWL_EV_(WM_THEMECHANGED, EvThemeChanged)
#define EV_WM_TIMECHANGE OWL_EV_(WM_TIMECHANGE, EvTimeChange)
#define EV_WM_TIMER OWL_EV_(WM_TIMER, EvTimer)
#define EV_WM_UNDO OWL_EV_(WM_UNDO, EvUndo)
#define EV_WM_USERCHANGED OWL_EV_(WM_USERCHANGED, EvUserChanged)
#define EV_WM_VKEYTOITEM OWL_EV_(WM_VKEYTOITEM, EvVKeyToItem)
#define EV_WM_VSCROLL OWL_EV_(WM_VSCROLL, EvVScroll)
#define EV_WM_VSCROLLCLIPBOARD OWL_EV_(WM_VSCROLLCLIPBOARD, EvVScrollClipboard)
#define EV_WM_WINDOWPOSCHANGED OWL_EV_(WM_WINDOWPOSCHANGED, EvWindowPosChanged)
#define EV_WM_WINDOWPOSCHANGING OWL_EV_(WM_WINDOWPOSCHANGING, EvWindowPosChanging)

#if defined(OWL5_COMPAT)

//
/// The WM_CTLCOLOR is an obsolete Win16 message, superceded by the more specific messages
/// WM_CTLCOLORBTN etc. But for backwards compatibility we simulate an entry for it here;
/// implemented by directing all the new messages to the same handler.
//
#define EV_WM_CTLCOLOR\
  EV_WM_CTLCOLORBTN(EvCtlColor),\
  EV_WM_CTLCOLOREDIT(EvCtlColor),\
  EV_WM_CTLCOLORDLG(EvCtlColor),\
  EV_WM_CTLCOLORLISTBOX(EvCtlColor),\
  EV_WM_CTLCOLORMSGBOX(EvCtlColor),\
  EV_WM_CTLCOLORSCROLLBAR(EvCtlColor),\
  EV_WM_CTLCOLORSTATIC(EvCtlColor)

//
// WM_POWER is an obsolete Win16 message. Applications should use the WM_POWERBROADCAST message.
//
#define EV_WM_POWER OWL_EV_(WM_POWER, EvPower)

//
// WM_SETTINGCHANGE supercedes WM_WININICHANGE.
// See Decode_WM_WININICHANGE.
//
#define EV_WM_WININICHANGE\
   {{WM_WININICHANGE}, 0, OWL_DISPATCH(::owl::Decode_WM_WININICHANGE, EvWinIniChange)}

#endif

/// @}
/// \name Dialog messages
/// @{

#define EV_DM_GETDEFID OWL_EV_(DM_GETDEFID, EvGetDefId)
#define EV_DM_SETDEFID OWL_EV_(DM_SETDEFID, EvSetDefId)
#define EV_DM_REPOSITION OWL_EV_(DM_REPOSITION, EvReposition)

/// @}
/// \name Button Notification Messages
/// @{

#define EV_BN_CLICKED(id, method) EV_CHILD_NOTIFY(id, BN_CLICKED, method)
#define EV_BN_DISABLE(id, method) EV_CHILD_NOTIFY(id, BN_DISABLE, method)
#define EV_BN_DOUBLECLICKED(id, method) EV_CHILD_NOTIFY(id, BN_DOUBLECLICKED, method)
#define EV_BN_HILITE(id, method) EV_CHILD_NOTIFY(id, BN_HILITE, method)
#define EV_BN_PAINT(id, method) EV_CHILD_NOTIFY(id, BN_PAINT, method)
#define EV_BN_UNHILITE(id, method) EV_CHILD_NOTIFY(id, BN_UNHILITE, method)
#define EV_BN_KILLFOCUS(id, method) EV_CHILD_NOTIFY(id, BN_KILLFOCUS, method)
#define EV_BN_SETFOCUS(id, method) EV_CHILD_NOTIFY(id, BN_SETFOCUS, method)

/// @}
/// \name Combo Box Notification Messages
/// @{

#define EV_CBN_CLOSEUP(id, method) EV_CHILD_NOTIFY(id, CBN_CLOSEUP, method)
#define EV_CBN_DBLCLK(id, method) EV_CHILD_NOTIFY(id, CBN_DBLCLK, method)
#define EV_CBN_DROPDOWN(id, method) EV_CHILD_NOTIFY(id, CBN_DROPDOWN, method)
#define EV_CBN_EDITCHANGE(id, method) EV_CHILD_NOTIFY(id, CBN_EDITCHANGE, method)
#define EV_CBN_EDITUPDATE(id, method) EV_CHILD_NOTIFY(id, CBN_EDITUPDATE, method)
#define EV_CBN_ERRSPACE(id, method) EV_CHILD_NOTIFY(id, CBN_ERRSPACE, method)
#define EV_CBN_KILLFOCUS(id, method) EV_CHILD_NOTIFY(id, CBN_KILLFOCUS, method)
#define EV_CBN_SELCHANGE(id, method) EV_CHILD_NOTIFY(id, CBN_SELCHANGE, method)
#define EV_CBN_SELENDCANCEL(id, method) EV_CHILD_NOTIFY(id, CBN_SELENDCANCEL, method)
#define EV_CBN_SELENDOK(id, method) EV_CHILD_NOTIFY(id, CBN_SELENDOK, method)
#define EV_CBN_SETFOCUS(id, method) EV_CHILD_NOTIFY(id, CBN_SETFOCUS, method)

/// @}
/// \name Edit Control Notification  Messages
/// @{

#define EV_EN_CHANGE(id, method) EV_CHILD_NOTIFY(id, EN_CHANGE, method)
#define EV_EN_ERRSPACE(id, method) EV_CHILD_NOTIFY(id, EN_ERRSPACE, method)
#define EV_EN_HSCROLL(id, method) EV_CHILD_NOTIFY(id, EN_HSCROLL, method)
#define EV_EN_KILLFOCUS(id, method) EV_CHILD_NOTIFY(id, EN_KILLFOCUS, method)
#define EV_EN_MAXTEXT(id, method) EV_CHILD_NOTIFY(id, EN_MAXTEXT, method)
#define EV_EN_SETFOCUS(id, method) EV_CHILD_NOTIFY(id, EN_SETFOCUS, method)
#define EV_EN_UPDATE(id, method) EV_CHILD_NOTIFY(id, EN_UPDATE, method)
#define EV_EN_VSCROLL(id, method) EV_CHILD_NOTIFY(id, EN_VSCROLL, method)

/// @}
/// \name List Box Notification  Messages
/// @{

#define EV_LBN_DBLCLK(id, method) EV_CHILD_NOTIFY(id, LBN_DBLCLK, method)
#define EV_LBN_ERRSPACE(id, method) EV_CHILD_NOTIFY(id, LBN_ERRSPACE, method)
#define EV_LBN_KILLFOCUS(id, method) EV_CHILD_NOTIFY(id, LBN_KILLFOCUS, method)
#define EV_LBN_SELCANCEL(id, method) EV_CHILD_NOTIFY(id, LBN_SELCANCEL, method)
#define EV_LBN_SELCHANGE(id, method) EV_CHILD_NOTIFY(id, LBN_SELCHANGE, method)
#define EV_LBN_SETFOCUS(id, method) EV_CHILD_NOTIFY(id, LBN_SETFOCUS, method)

/// @}
/// \name Scroll Bar Notification Messages
/// @{

#define EV_SB_LINEDOWN(id, method) EV_CHILD_NOTIFY(id, SB_LINEDOWN, method)
#define EV_SB_LINEUP(id, method) EV_CHILD_NOTIFY(id, SB_LINEUP, method)
#define EV_SB_PAGEDOWN(id, method) EV_CHILD_NOTIFY(id, SB_PAGEDOWN, method)
#define EV_SB_PAGEUP(id, method) EV_CHILD_NOTIFY(id, SB_PAGEUP, method)
#define EV_SB_TOP(id, method) EV_CHILD_NOTIFY(id, SB_TOP, method)
#define EV_SB_BOTTOM(id, method) EV_CHILD_NOTIFY(id, SB_BOTTOM, method)
#define EV_SB_THUMBPOSITION(id, method) EV_CHILD_NOTIFY(id, SB_THUMBPOSITION, method)
#define EV_SB_ENDSCROLL(id, method) EV_CHILD_NOTIFY(id, SB_ENDSCROLL, method)
#define EV_SB_BEGINTRACK(id, method) EV_CHILD_NOTIFY(id, 9, method)

/// @}
/// \name Static Control Notification Messages
/// \note These notifications are only sent if the control has the SS_NOTIFY style.
/// @{

#define EV_STN_DBLCLK(id, method) EV_CHILD_NOTIFY(id, STN_DBLCLK, method)
#define EV_STN_CLICKED(id, method) EV_CHILD_NOTIFY(id, STN_CLICKED, method)
#define EV_STN_DISABLE(id, method) EV_CHILD_NOTIFY(id, STN_DISABLE, method)
#define EV_STN_ENABLE(id, method) EV_CHILD_NOTIFY(id, STN_ENABLE, method)

/// @}

#endif
