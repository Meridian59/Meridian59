//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Declares THelpContext and THelpFileManager.
//----------------------------------------------------------------------------

#if !defined(OWL_HLPMANAG_H)
#define OWL_HLPMANAG_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/window.h>

#if !defined(NO_HTMLHELP)
# if !defined(__HTMLHELP_H__)
#  if defined(BI_NEED_HTMLHELP_H) || defined(WINELIB)
#   include <api_upd/htmlhelp.h>
#  else
#   include <htmlhelp.h>
#  endif
# endif
#endif


namespace owl {

class _OWLCLASS THelpContext;
class _OWLCLASS TWindow;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>


// moved from owl/signatur.h

struct THelpHitInfo
{
  THelpHitInfo(const TPoint& pt, TWindow* wnd):Point(pt),Window(wnd){}
  THelpHitInfo(int x,int y, TWindow* wnd):Point(x,y),Window(wnd){}

  TPoint     Point;
  TWindow*  Window;
};


#if !defined(NO_HTMLHELP)

class _OWLCLASS THlpNotify : public HHN_NOTIFY{
  public:
    THlpNotify(){}

    operator  NMHDR&() { return hdr; }
};

class _OWLCLASS THlpPopup : public  HH_POPUP {
  public:
    THlpPopup(uint id = 0);
};

class _OWLCLASS THlpAklink : public  HH_AKLINK {
  public:
    THlpAklink()
      {
        ZeroMemory(this,sizeof(HH_AKLINK));
        cbStruct = sizeof(HH_AKLINK);
       }
};

class _OWLCLASS THlpEnumIt : public  HH_ENUM_IT {
  public:
    THlpEnumIt()
      {
        ZeroMemory(this,sizeof(HH_ENUM_IT));
        cbStruct = sizeof(HH_ENUM_IT);
       }
};

class _OWLCLASS THlpSetInfoType : public  HH_SET_INFOTYPE {
  public:
    THlpSetInfoType()
      {
        ZeroMemory(this,sizeof(HH_SET_INFOTYPE));
        cbStruct = sizeof(HH_SET_INFOTYPE);
       }
};

class _OWLCLASS THlpFtsQuery : public  HH_FTS_QUERY {
  public:
    THlpFtsQuery()
      {
        ZeroMemory(this,sizeof(HH_FTS_QUERY));
        cbStruct = sizeof(HH_FTS_QUERY);
       }
};

class _OWLCLASS THlpWinType : public  HH_WINTYPE {
  public:
    THlpWinType()
      {
        ZeroMemory(this,sizeof(HH_WINTYPE));
        cbStruct = sizeof(HH_WINTYPE);
       }
    void SetStyle(int style);
    void SetProperties(int prop);
    void SetToolBarFlags(int flags);
    void SetWindowsPos(const TRect& rect);
};

class _OWLCLASS THlpTrack : public  HHNTRACK {
  public:
    THlpTrack(){}

    operator  NMHDR&() { return hdr; }
};


#endif
//
/// \class THelpContext
// ~~~~~ ~~~~~~~~~~~~
/// THelpContext is a class that maintains information about a menu item id and
/// a child control id with a help context id.
/// As particular windows get and lose focus, their context tables are removed
/// from a global context table.
//
class _OWLCLASS THelpContext {
  public:
    THelpContext();
    THelpContext(TWindow* window, int helpId, int menuId, int controlId);
    THelpContext(const THelpContext&);
   ~THelpContext();

    THelpContext& operator =(const THelpContext&);
    bool operator ==(const THelpContext&) const;
    // not implemented just to make happy STL
    bool operator <(const THelpContext&) const;

    int GetHelpFileContextId() const;
    int GetMenuContextId() const;
    int GetControlContextId() const;
    TWindow* GetWindow() const;
    void SetWindow(TWindow* window);

  private:
    TWindow* Window;
    int HelpFileContextId;
    int MenuContextId;
    int ControlContextId;
};

// Forward declarations
template <class T> class TBaseList;
template<class T> class TBaseListIterator;

//
/// \class THelpFileManager
// ~~~~~ ~~~~~~~~~~~~~~~~
/// THelpFileManager, which is designed to be a mix-in for TApplication, uses the
/// global context table. THelpFileManager looks for the WM_HELP message and calls
/// the help file with the associated context ID.
/// \todo HTMLHelp should be by default, and discourage using old WinHelp files 
/// as they are not well supported under Windows Vista and later
//
class _OWLCLASS THelpFileManager : virtual public TEventHandler {
  public:
DECLARE_CASTABLE;
    THelpFileManager(const tstring& helpFileName);
    virtual ~THelpFileManager();

    virtual void ActivateHelp(TWindow*, int helpFileContextId, uint hlpCmd = HELP_CONTEXT);
    virtual void DeactivateHelp();

    void         SetHelpFile(const tstring& helpFileName);
    tstring   GetHelpFile() const;

    bool     GetHelpContextFromControl(THelpContext&, int controlId, HWND ctrl) const;
    bool     GetHelpContextFromMenu(THelpContext&, int menuId) const;

    void     AddContextInfo(TWindow*, int helpId, int menuId, int controlId);
    void     RemoveContextInfo(TWindow*);

  protected:
    virtual bool  ProcessHelpMsg (MSG& msg);
    virtual void  SetHelpCursor();

    void   EvHelp(const HELPINFO&);
    void   EvHelpHit(const THelpHitInfo&);

    void  CmContextHelp ();
    void   CeContextHelp (TCommandEnabler& ce);
    void   CmWhatIsThis();

#if !defined(NO_HTMLHELP)
    //  not finished wil be changed ???????????????
  public:
    HWND   HtmlHelp(TWindow*, LPCTSTR lpszHelp, uint hlpCmd, uint32 data);

    HWND HtmlHelp(TWindow* w, const tstring& helpFile, uint hlpCmd, uint32 data)
    {return HtmlHelp(w, helpFile.c_str(), hlpCmd, data);}

    void  TranslateWinToHTML(bool translate) { WinToHTML = translate;}

  protected:
    bool   UseHTMLHelp;
    bool   WinToHTML;
#endif

  protected:
    typedef TBaseList<THelpContext>          TContextList;
    typedef TBaseListIterator<THelpContext> TContextListIterator;
    TContextList* ContextTable;

    tstring   HelpFileName;
    TCursor*     HelpCursor;

     bool         ContextHelp;
    bool         HelpState;

    TPoint      LastHit;
    TWindow*    LastParent;

  DECLARE_RESPONSE_TABLE(THelpFileManager);
};

#if !defined(NO_HTMLHELP)

/// \class THtmlHelpDll
// ~~~~~ ~~~~~~~~~~~~
/// Wrapper for the HHCTRL.OCX itself
class _OWLCLASS THtmlHelpDll : public TModule {
  public:
    THtmlHelpDll();

    TModuleProc0<HRESULT> DllCanUnloadNow;
    TModuleProc3<HRESULT,REFCLSID,REFIID,void**>   DllGetClassObject;
    TModuleProc4<HWND,HWND,LPCTSTR,uint,uint32>   HtmlHelp;
};

//
/// Loader for the HHCTRL.OCX
typedef _OWLCLASS TDllLoader<THtmlHelpDll> THtmlHelp;

#if defined(_OWLDLL) || defined(BI_APP_DLL)
  //
  /// Export template of TDllLoader<THtmlHelpDll> when building ObjectWindows
  /// DLL and provide import declaration of DLL instance for users of the class.
  //
  template class _OWLCLASS TDllLoader<THtmlHelpDll>;
#endif

#endif

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Macros to simplify usage of THelpContext
//

#define DECLARE_HELPCONTEXT(cls)               \
    static THelpContext __hcEntries[]

#define DEFINE_HELPCONTEXT(cls)\
  THelpContext cls::__hcEntries[] = {

#define END_HELPCONTEXT        \
    THelpContext(0, 0, 0, 0)   \
  }

#define HCENTRY_MENU(hcId, menuId)                    \
  THelpContext(0, hcId, menuId, 0)

#define HCENTRY_CONTROL(hcId, ctlId)                  \
  THelpContext(0, hcId, 0, ctlId)

#define HCENTRY_MENU_AND_CONTROL(hcId, menuId, ctlId) \
  THelpContext(0, hcId, menuId, ctlId)

#define SETUP_HELPCONTEXT(appCls, cls)                                  \
  {                                                                     \
    appCls* app = TYPESAFE_DOWNCAST(GetApplication(), appCls);          \
    if (app) {                                                          \
      for (THelpContext* hc = &__hcEntries[0]; !IsLastIndirectContext(*hc); hc++) { \
        app->AddContextInfo(this,                                       \
                            hc->GetHelpFileContextId(),                 \
                            hc->GetMenuContextId(),                     \
                            hc->GetControlContextId());                 \
      }                                                                 \
    }                                                                   \
  }

#define CLEANUP_HELPCONTEXT(appCls, cls)                          \
  {                                                               \
    appCls* app = TYPESAFE_DOWNCAST(GetApplication(), appCls);    \
    if (app)                                                      \
      app->RemoveContextInfo(this);                               \
  }

const int TablePtr = -1;

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Return true if the context entry is a pointer to another table.
//
inline bool
IsIndirectHelpContext(const THelpContext& context)
{
  if (context.GetMenuContextId() == TablePtr &&
      context.GetHelpFileContextId() == TablePtr &&
      context.GetControlContextId() == TablePtr)
    return true;
  return false;
}

//
/// Return true if this entry is the last entry.
//
inline bool
IsLastIndirectContext(const THelpContext& context)
{
  if (context.GetMenuContextId() == 0 &&
      context.GetHelpFileContextId() == 0 &&
      context.GetControlContextId() == 0)
    return true;
  return false;
}

//
/// Returns the name of the help file.
//
inline tstring
THelpFileManager::GetHelpFile() const
{
  return HelpFileName;
}

//
//
//
inline void
THelpFileManager::EvHelpHit(const THelpHitInfo& hit)
{
  LastHit      = hit.Point;
  LastParent  = hit.Window;
}

//
/// Returns the help file context id for the context entry.
//
inline int
THelpContext::GetHelpFileContextId() const
{
  return HelpFileContextId;
}

//
/// Returns the menu id for this context entry.
//
inline int
THelpContext::GetMenuContextId() const
{
  return MenuContextId;
}

//
/// Returns the child control id for this context entry.
//
inline int
THelpContext::GetControlContextId() const
{
  return ControlContextId;
}

//
/// Returns the window this entry is associated with.
//
inline TWindow*
THelpContext::GetWindow() const
{
  return Window;
}

//
/// Sets the window for this context entry.
//
inline void
THelpContext::SetWindow(TWindow* window)
{
  Window = window;
}


} // OWL namespace


#endif  // OWL_HLPMANAG_H
