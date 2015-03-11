//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class THelpFileManager & THelpContext
//----------------------------------------------------------------------------

#include <owl/pch.h>

#include <owl/hlpmanag.h>
#include <owl/applicat.h>
#include <owl/window.h>
#include <owl/framewin.h>
#include <owl/gadgetwi.h>
#include <owl/template.h>

#if !defined(OWL_HLPMANAG_RH)
# include <owl/hlpmanag.rh>
#endif

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;

#if !defined(NO_HTMLHELP)

static tchar HtmlHelpDllName[] = _T("HHCTRL.OCX");

THtmlHelpDll::THtmlHelpDll()
:
   TModule(HtmlHelpDllName, true, true, false), // shouldLoad, mustLoad and !addToList
  DllCanUnloadNow(*this, "DllCanUnloadNow"),
  DllGetClassObject(*this, "DllGetClassObject"),
#ifdef UNICODE
  HtmlHelp(*this, "HtmlHelpW")
#else
  HtmlHelp(*this, "HtmlHelpA")
#endif
{
}

#if defined(__BORLANDC__) && !defined(_WIN64)
// Cool new C++ technique to instantiate the template
template class /*_OWLCLASS*/ TDllLoader<THtmlHelpDll>;
#endif


THlpPopup::THlpPopup(uint id)
{
  cbStruct       = sizeof(HH_POPUP);
  hinst         = 0;
  idString       = id;
  pszText       = 0;
  pt            = TPoint(-1,-1);
  clrForeground = static_cast<COLORREF>(-1); //JJH added static casts
  clrBackground = static_cast<COLORREF>(-1);
  rcMargins     = TRect(-1,-1,-1,-1);
  pszFont       = 0;
}

void
THlpWinType::SetStyle(int style)
{
  fsValidMembers |= HHWIN_PARAM_STYLES;
  dwStyles |= style;
}

void
THlpWinType::SetProperties(int prop)
{
  fsValidMembers |= HHWIN_PARAM_PROPERTIES;
  fsWinProperties |= prop;
}

void
THlpWinType::SetToolBarFlags(int flags)
{
  fsValidMembers |= HHWIN_PARAM_TB_FLAGS;
  fsToolBarFlags  |= flags;
}

void
THlpWinType::SetWindowsPos(const TRect& rect)
{
  fsValidMembers |= HHWIN_PARAM_TB_FLAGS;
  rcWindowPos  = rect;
}

#endif


//
/// Constructor. Saves the name of the help file and creates the global context
/// table.
//
THelpFileManager::THelpFileManager(const tstring& helpFileName)
:
#if !defined(NO_HTMLHELP)
  UseHTMLHelp(false),
  WinToHTML(true),
#endif
  HelpCursor(0),
  ContextHelp(false),
  HelpState(false),
  LastParent(0)
{
  SetHelpFile(helpFileName);
  ContextTable = new TContextList;
}

//
/// Deletes the allocated context table.
//
THelpFileManager::~THelpFileManager()
{
  delete ContextTable;
  delete HelpCursor;
}

//
/// Called by EvHelp() to activate the help file with the help context ID.
//
void
THelpFileManager::ActivateHelp(TWindow* /*window*/, int helpFileContextId, uint helpCmd)
{
  if (helpCmd == HELP_INDEX || helpCmd == HELP_CONTENTS)
    helpCmd = HELP_FINDER;
  TApplication* app = TYPESAFE_DOWNCAST(this, TApplication);
#if !defined(NO_HTMLHELP)
  if(UseHTMLHelp){
    if (app)
      HelpState = ToBool(HtmlHelp(app->GetMainWindow(), GetHelpFile().c_str(),
                                  helpCmd, helpFileContextId) != 0);
    else
      HelpState = ToBool(HtmlHelp(0, GetHelpFile().c_str(),
                                  helpCmd, helpFileContextId) != 0);
  }
  else{
#endif
    if (app)
      HelpState = ToBool(app->GetMainWindow()->WinHelp(GetHelpFile().c_str(),
                         helpCmd, helpFileContextId));
    else
      HelpState = ToBool(::WinHelp(0, GetHelpFile().c_str(),
                         helpCmd, helpFileContextId));
#if !defined(NO_HTMLHELP)
  }
#endif
}

//
/// Deactivates the help.
//
void
THelpFileManager::DeactivateHelp()
{
  TApplication* app = TYPESAFE_DOWNCAST(this, TApplication);
#if !defined(NO_HTMLHELP)
  if(UseHTMLHelp){
    if (app)
      HtmlHelp(app->GetMainWindow(), GetHelpFile().c_str(), HELP_QUIT, 0);
    else
      HtmlHelp(0, GetHelpFile().c_str(), HELP_QUIT, 0);
  }
  else{
#endif
    if (app)
      app->GetMainWindow()->WinHelp(GetHelpFile().c_str(), HELP_QUIT, 0);
    else
      ::WinHelp(0, GetHelpFile().c_str(), HELP_QUIT, 0);
#if !defined(NO_HTMLHELP)
  }
#endif
}


#if !defined(NO_HTMLHELP)
//
// Support for nwe HTML Help
//
HWND
THelpFileManager::HtmlHelp(TWindow* wnd, LPCTSTR lpszHelp, uint uCmd,  uint32 data)
{
  // translate uCommand into HTML command
  uint command = uCmd;
  if(WinToHTML){
    switch(uCmd){
      case HELP_FINDER:
        command = HH_HELP_FINDER;
        break;
      case HELP_CONTENTS:  //== HELP_INDEX
        command = HH_HELP_CONTEXT;    //
        break;
      case HELP_WM_HELP:
        command = HH_TP_HELP_WM_HELP;
        break;
      case HELP_CONTEXTMENU:
        command = HH_TP_HELP_CONTEXTMENU;
        break;
      case HELP_QUIT:
        command = HH_CLOSE_ALL;
        break;
      case HELP_CONTEXTPOPUP: {
//-----------------------
//Start Ole Änderung 02.04.2002
//          command = HH_TP_HELP_CONTEXTMENU;
               command = HH_DISPLAY_TEXT_POPUP;
          if(THtmlHelp::IsAvailable()){
                  if(LastHit == TPoint(-1, -1))
                    HtmlHelp(GetWindowPtr(GetDesktopWindow()), lpszHelp, HH_HELP_CONTEXT,data) ;
                  else
                  {
                    HH_POPUP hPop; // HTML Help popup structure
                    memset(&hPop, 0, sizeof(hPop));
                    hPop.cbStruct         = sizeof(hPop);
                    hPop.clrBackground    = -1;
                    hPop.clrForeground    = -1;
                    hPop.rcMargins.left   = -1;
                    hPop.rcMargins.bottom = -1;
                    hPop.rcMargins.right  = -1;
                    hPop.pt               = LastHit;
                    hPop.idString         = data;
                    hPop.pszText          = NULL;
                    hPop.pszFont          = NULL;

            if(wnd)
                      return THtmlHelp::Dll()->HtmlHelp(*wnd, lpszHelp, command, (DWORD)&hPop);
            else
                      return THtmlHelp::Dll()->HtmlHelp(0, lpszHelp, command, (DWORD)&hPop);
                  }

//            if(wnd)
//              return THtmlHelp::Dll()->HtmlHelp(*wnd,lpszHelp,command,data);
//            else
//              return THtmlHelp::Dll()->HtmlHelp(0, lpszHelp,command,data);
//Ende  Ole Änderung 02.04.2002
//-----------------------
          }
          return 0;
        }
    }
  }
  if(THtmlHelp::IsAvailable()){
    if(wnd)
      return THtmlHelp::Dll()->HtmlHelp(*wnd,lpszHelp,command,data);
    else
      return THtmlHelp::Dll()->HtmlHelp(0, lpszHelp,command,data);
  }
  return 0;
}
#endif

//
/// Changes the name of the help file.
//
void
THelpFileManager::SetHelpFile(const tstring& helpFileName)
{
  HelpFileName = helpFileName;
#if !defined(NO_HTMLHELP)
  UseHTMLHelp = false;
  LPCTSTR ptr = _tcsrchr(HelpFileName.c_str(),_T('.'));
  if(ptr && _tcsicmp(ptr, _T(".chm"))==0)
    UseHTMLHelp = true;
#endif
}

DEFINE_RESPONSE_TABLE1(THelpFileManager, TEventHandler)
  EV_COMMAND(CM_CONTEXTHELP,           CmContextHelp),
  EV_COMMAND_ENABLE(CM_CONTEXTHELP,   CeContextHelp),
  EV_COMMAND(CM_WHAT_IS_THIS,         CmWhatIsThis),
  EV_WM_HELP,
  EV_OWLHELPHIT,
END_RESPONSE_TABLE;

//
//
//
void
THelpFileManager::SetHelpCursor()
{
  if(!HelpCursor){
    TApplication* app = TYPESAFE_DOWNCAST(this, TApplication);
    if(app)
      HelpCursor = new TCursor(*app, TResId(IDC_HELPCURSOR));
  }
  ::SetCursor(*HelpCursor);
}

//
//
//
void
THelpFileManager::CmContextHelp ()
{
  ContextHelp = true;
   SetHelpCursor();
}

//
//
//
void
THelpFileManager::CeContextHelp (TCommandEnabler& ce)
{
  ce.Enable(true);
}

//
//
//
void
THelpFileManager::CmWhatIsThis()
{
  PRECONDITION(LastParent);
  if(!LastParent)
    return;
  LastParent->ScreenToClient(LastHit); // TWindow
  HWND hWnd = LastParent->ChildWindowFromPoint(LastHit);
  HELPINFO Info;
  Info.cbSize       = sizeof(Info);
  Info.MousePos     = LastHit;

  if(hWnd){
    TGadgetWindow* gwin = TYPESAFE_DOWNCAST(GetWindowPtr(hWnd), TGadgetWindow);
    if(gwin){
      TGadget* gd = gwin->GadgetFromPoint(LastHit);
      if(gd){
        Info.iContextType = HELPINFO_MENUITEM;
        Info.iCtrlId      = gd->GetId();
        EvHelp(Info);
      }
      return;
    }
  }
  Info.iContextType = HELPINFO_WINDOW;
  Info.dwContextId  = 0;

  // If the click wasn't on one of the controls, open Help for dialog.
  if (hWnd == 0 || hWnd == LastParent->GetHandle()){
    Info.iContextType = HELPINFO_MENUITEM;
    Info.iCtrlId      = IDHELP;
    Info.hItemHandle  = LastParent->GetHandle();
  }
  else{
    Info.iCtrlId      = ::GetDlgCtrlID(hWnd);
    Info.hItemHandle  = hWnd;
  }
  EvHelp(Info);
}

//
/// Event handler for the WM_HELP message.
//
void
THelpFileManager::EvHelp(const HELPINFO& hi)
{
  THelpContext context;
  bool success = false;
  uint hlpCmd = HELP_CONTEXTPOPUP;

  if (hi.iContextType == HELPINFO_MENUITEM){
    success = GetHelpContextFromMenu(context, hi.iCtrlId);
    hlpCmd = HELP_CONTEXT;
  }
  else if (hi.iContextType == HELPINFO_WINDOW)
    success = GetHelpContextFromControl(context, hi.iCtrlId, (HWND)hi.hItemHandle);

  LastHit = hi.MousePos;
  if (::GetKeyState(VK_F1) < 0 && ::GetKeyState(VK_SHIFT) < 0)
  {
    TWindow* wnd = context.GetWindow();
    TWindow* child = wnd->ChildWithId(hi.iCtrlId);
    LastHit = TPoint(wnd->GetWindowAttr().X + child->GetWindowAttr().X, wnd->GetWindowAttr().Y + child->GetWindowAttr().Y);
  }
  else if(::GetKeyState(VK_F1) < 0)
    LastHit = TPoint(-1, -1);

  if (success)
    ActivateHelp(context.GetWindow(), context.GetHelpFileContextId(), hlpCmd);
}

//
//
//
bool
THelpFileManager::ProcessHelpMsg (MSG& msg)
{
  if (msg.message == WM_COMMAND) {
    // help command from menu or from gadget from "what this?" for gadget
    if (ContextHelp || (::GetKeyState(VK_F1) < 0)) {
      ContextHelp = false;
      HELPINFO Info;
      Info.cbSize       = sizeof(Info);
      Info.iContextType = HELPINFO_MENUITEM;
      Info.iCtrlId      = static_cast<int>(msg.wParam);
      Info.dwContextId  = 0;
      EvHelp(Info);
      return true;
    }
  }
  else{
    switch (msg.message) {
      case WM_KEYDOWN :
        if (msg.wParam == VK_F1) {
          // If the Shift/F1 then set the help cursor and turn on the modal help state.
          if (::GetKeyState(VK_SHIFT) < 0 && !ContextHelp) {
            CmContextHelp();
            return true;        // Gobble up the message.
          }
        }
        else {
          if (ContextHelp && (msg.wParam == VK_ESCAPE)) {
            ContextHelp = false;
            TApplication* app = TYPESAFE_DOWNCAST(this, TApplication);
            if (app)
              app->GetMainWindow()->SetCursor(0, IDC_ARROW); //?????????????????
            return true;    // Gobble up the message.
          }
        }
        break;

      case WM_MOUSEMOVE :
      case WM_NCMOUSEMOVE :
        if(ContextHelp){
          SetHelpCursor();
          return true;        // Gobble up the message.
        }
        break;

      case WM_INITMENU :
        if(ContextHelp) {
           SetHelpCursor();
          return true;        // Gobble up the message.
        }
        break;

      case WM_ENTERIDLE :
        if(msg.wParam == MSGF_MENU)
          if (GetKeyState(VK_F1) < 0) {
            ContextHelp = true;
            TApplication* app = TYPESAFE_DOWNCAST(this, TApplication);
            if (app)
              app->GetMainWindow()->PostMessage(WM_KEYDOWN, VK_RETURN, 0L);
            return true;       // Gobble up the message.
          }
        break;

      default :
        ;
    }  // End of switch
  }
  // Continue normal processing.
  return false;
}

//
/// Return true if the control id is the same as the context entry's control id.
//
static bool
MatchControlId(const THelpContext& context, int controlId, HWND hwnd)
{
  if (context.GetControlContextId() == controlId &&
      context.GetWindow() && context.GetWindow()->IsChild(hwnd)) {
    return true;
  }
  return false;
}

//
/// Returns true if a match for the control ID was found.
//
bool
THelpFileManager::GetHelpContextFromControl(THelpContext& context,
                                            int controlId, HWND hwnd) const
{
  // for each item in the context table
  //
  for(TContextListIterator iterator(*ContextTable); iterator; iterator++){
    context = iterator.Current();

    // try to match the control id
    //
    if (IsIndirectHelpContext(context)) {
      THelpContext* indirect = (THelpContext*)context.GetWindow();
      while (!IsLastIndirectContext(*indirect)) {
        if (MatchControlId(*indirect, controlId, hwnd)) {
          context = *indirect;
          return true;
        }
        indirect++;
      }
    }
    else if (MatchControlId(context, controlId, hwnd))
      return true;
  }
  return false;
}

//
/// Return true if the menu item identifier matches a context entry's menu
/// item identifier.
//
static bool
MatchMenuId(const THelpContext& context, int menuId)
{
  if (context.GetMenuContextId() == menuId) {
    return true;
  }
  return false;
}

//
/// Returns true if a match for the menu item ID was found.
//
bool
THelpFileManager::GetHelpContextFromMenu(THelpContext& context, int menuId) const
{
  // for each item in the context table
  //
  for (TContextListIterator iterator(*ContextTable); iterator; iterator++) {
    context = iterator.Current();


    // try to match the menu item identifier
    //
    if (IsIndirectHelpContext(context)) {
      THelpContext* indirect = (THelpContext*)context.GetWindow();
      while (!IsLastIndirectContext(*indirect)) {
        if (MatchMenuId(*indirect, menuId)) {
          context = *indirect;
          return true;
        }
        indirect++;
      }
    }
    else if (MatchMenuId(context, menuId))
      return true;
  }
  return false;
}

//
/// Adds an entry into the global context table.
//
void
THelpFileManager::AddContextInfo(TWindow* window, int helpId, int menuId, int controlId)
{
  ContextTable->Add(THelpContext(window, helpId, menuId, controlId));
}

static bool
WindowsEqual(const THelpContext& context, void* ptr)
{
  TWindow* targetWindow = (TWindow*)ptr;
  if (context.GetWindow() == targetWindow)
    return true;
  return false;
}

//
/// Removes all the entries in the context table for the window.
//
void
THelpFileManager::RemoveContextInfo(TWindow* window)
{
  THelpContext* context = ContextTable->FirstThat(WindowsEqual, window);
  while (context) {
    ContextTable->DetachItem(*context);
    context = ContextTable->FirstThat(WindowsEqual, window);
  }
  if (ContextTable->Empty())
    DeactivateHelp();
}

//
/// Default constructor that initializes every data member to zero.
//
THelpContext::THelpContext()
:
  Window(0),
  HelpFileContextId(0),
  MenuContextId(0),
  ControlContextId(0)
{
}

//
/// Convenient constructor to initialize the context entry with the
/// proper ids.
//
THelpContext::THelpContext(TWindow* window, int helpId, int menuId, int controlId)
:
  Window(window),
  HelpFileContextId(helpId),
  MenuContextId(menuId),
  ControlContextId(controlId)
{
}

//
/// Makes a copy of the context entry.
//
THelpContext::THelpContext(const THelpContext& other)
{
  *this = other;
}

//
// Do nothing.
//
THelpContext::~THelpContext()
{
}

//
/// Makes a copy of the context entry.
//
THelpContext&
THelpContext::operator =(const THelpContext& other)
{
  HelpFileContextId = other.GetHelpFileContextId();
  MenuContextId     = other.GetMenuContextId();
  ControlContextId  = other.GetControlContextId();
  Window            = other.GetWindow();
  return *this;
}

//
/// Returns true if the context entries match.
//
bool
THelpContext::operator ==(const THelpContext& other) const
{
  return Window == other.GetWindow() &&
         HelpFileContextId == other.GetHelpFileContextId() &&
         MenuContextId == other.GetMenuContextId() &&
         ControlContextId == other.GetControlContextId();
}
//
//
//
bool
THelpContext::operator <(const THelpContext& other) const
{
  return HelpFileContextId < other.GetHelpFileContextId();
}

} // OWL namespace
/* ========================================================================== */

