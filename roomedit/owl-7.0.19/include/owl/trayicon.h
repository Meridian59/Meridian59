//------------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 bBidus Yura, All Rights Reserved
//
//------------------------------------------------------------------------------

#if !defined(OWL_TRAYICON_H)
#define OWL_TRAYICON_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/defs.h>
#include <owl/wsyscls.h>
#include <owl/gdiobjec.h>
#include <owl/eventhan.h>

#include <shellapi.h>

namespace owl {

class _OWLCLASS TModule;

#include <owl/preclass.h>

//
/// \class TTrayIcon
// ~~~~~ ~~~~~~~~~
//
class _OWLCLASS TTrayIcon: protected NOTIFYICONDATA {
  public:
    TTrayIcon(TModule* module = &GetGlobalModule());
    TTrayIcon(TWindow* parent, TMsgId, const tstring& tip, TIcon& icon, uint id, TModule* module = &GetGlobalModule());
    TTrayIcon(TWindow* parent, TMsgId, const tstring& tip, TResId icon, uint id, TModule* module = &GetGlobalModule());
    virtual void Init(TWindow* parent, TMsgId, const tstring& tip, TIcon* icon=0, uint id=0);
    virtual ~TTrayIcon();

  // Operations
  public:
    bool Enabled() const;
    bool Visible() const;

    /// Create the tray icon => must have valid hWnd => call in SetupWindow();
    bool Create();

    //Change or retrieve the Tooltip text
    bool        SetText(const tstring& text);
    bool        SetText(TResId resId);
    tstring  GetText() const;

    //Change or retrieve the window to send notification messages to
    bool      SetParent(TWindow* wnd);
    TWindow*   GetParent() const;

    //Change or retrieve the icon displayed
    bool    SetIcon(TIcon& icon, TAutoDelete = NoAutoDelete);
    bool    SetIcon(TResId resId);
    TIcon*  GetIcon() const;


    void  HideIcon();
    void  ShowIcon();
    void  RemoveIcon();
    void  MoveToRight();

    /// Default handler for tray notification message
    TResult EvTrayNotification(TParam1, TParam2);

  // Implementation
  protected:
    bool       bEnabled;  ///< does O/S support tray icon?
    bool      bHidden;  ///< Has the icon been hidden?
    TWindow*   Parent;
    TModule*  Module;
    TIcon*    Icon;
    bool      ShouldDelete;

  //DECLARE_DYNAMIC(TTrayIcon)
};

#include <owl/posclass.h>

inline bool TTrayIcon::Enabled() const {
  return bEnabled;
}
inline bool TTrayIcon::Visible() const {
  return !bHidden;
}
inline TWindow* TTrayIcon::GetParent() const{
  return Parent;
}
inline bool TTrayIcon::SetText(TResId resId){
  return SetText(Module->LoadString(resId.GetInt()));
}

inline bool TTrayIcon::SetIcon(TResId resId){
  return SetIcon(*new TIcon(Module? (HINSTANCE)*Module : HINSTANCE(nullptr), resId), AutoDelete);
}

} // OWL namespace


#endif // #define OWL_TRAYICON_H

