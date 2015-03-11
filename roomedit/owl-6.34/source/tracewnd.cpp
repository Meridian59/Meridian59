//----------------------------------------------------------------------------
//  ObjectWindows
//  Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
///   This source file implements the TTraceWindow class.
//----------------------------------------------------------------------------
#include <owl/pch.h>
//#if !defined(__GNUC__) //JJH added removal of pragma hdrstop for gcc
#pragma hdrstop
//#endif

//FMM
//#include <io.h>
//#include <stdio.h>

#include <owl/private/dir.h>
#include <owl/choosefo.h>
#include <owl/opensave.h>
#include <owl/inputdia.h>
#include <owl/panespli.h>
#include <owl/profile.h>
#include <owl/applicat.h>
#include <owl/edit.h>
#include <owl/listbox.h>
#include <owl/checklst.h>
#include <owl/validate.h>
#include <owl/filename.h>
#include <owl/tracewnd.h>

namespace owl {
OWL_DIAGINFO;

//
// struct TTraceWindowHook
//
void TTraceWindowHook::Output(TDiagBase* group, LPCTSTR str)
{
  if (TTraceWindow::sTraceWnd)
    TTraceWindow::sTraceWnd->OutputMessage(group, str);
}

//
// class TTraceEdit
// ~~~~~ ~~~~~~~~~~
//
class TTraceEdit : public TEdit {
  public:
    TTraceEdit(TWindow* parent)
               : TEdit(parent,500, _T(""), 0, 0, 0, 0, 0, true)
              {}
    ~TTraceEdit();

    void AppendText (LPCTSTR sz);
};

TTraceEdit::~TTraceEdit()
{
}

void TTraceEdit::AppendText (LPCTSTR sz)
{
  tstring s = GetText() + sz;
  SetText(s);
}

//
// class TGroupList
// ~~~~~ ~~~~~~~~~~
//
class TGroupList: public TCheckList{
  public:
    TGroupList(TWindow* parent, int Id,  
               int x, int y, int w, int h, 
               TModule* module = 0)  
      :TCheckList(parent,Id,x,y,w,h,0,0,module)
      {}
    ~TGroupList();

    void    AddItem(const tstring& text, bool checked, void * data);
    void*    GetData(int index) { return (void*)GetItem(index)->GetData(); }
    void    SetItemText(int index, const tstring& text);
    virtual void  ClearList();

  protected:
    void  EvLButtonDown(uint modKeys, const TPoint& point);
    void  EvChar(uint key, uint repeatCount, uint flags);
    void  SetGroupState (int index, bool enabled);

    TIPtrArray<TCheckListItem*> CheckArray;

  DECLARE_RESPONSE_TABLE(TGroupList);
};

DEFINE_RESPONSE_TABLE1(TGroupList, TCheckList)
  EV_WM_LBUTTONDOWN,
  EV_WM_CHAR,
END_RESPONSE_TABLE;

//
TGroupList::~TGroupList()
{
}
//
void
TGroupList::SetGroupState(int index, bool enabled)
{
  TDiagBase* group = reinterpret_cast<TDiagBase*>(GetData(index));
  if(!group)
    return;

  group->Enable(uint8(enabled ? 1 : 0));

  // save level into owl.ini
  TProfile iniFile(_T("Diagnostics"), _T(OWL_INI));
  tchar buffer[20];
  wsprintf(buffer,_T("%d %d"),(int)group->IsEnabled(),(int)group->GetLevel());
  _USES_CONVERSION;
  iniFile.WriteString(_A2W(group->GetName()), buffer);
}

//
void
TGroupList::EvChar(uint key, uint repeatCount, uint flags)
{
  TCheckList::EvChar(key, repeatCount, flags);

  int index = GetCaretIndex();
  TCheckListItem* item = GetItem(index);
  if(item && item->IsEnabled() && key == _T(' '))
    SetGroupState (index, item->IsChecked());
}

//
void 
TGroupList::EvLButtonDown(uint modKeys, const TPoint& point)
{
  TCheckList::EvLButtonDown(modKeys, point);

  int index = GetCaretIndex();
  TCheckListItem* item = GetItem(index);
  if(item && item->IsEnabled() && point.x < CheckList_BoxWidth)
    SetGroupState (index, item->IsChecked());
}

//
void 
TGroupList::AddItem(const tstring& text, bool checked, void* data)
{
  TCheckListItem* item = new TCheckListItem(text, checked ? BF_CHECKED : BF_UNCHECKED);
  item->SetData((uint32)data);
  
  CheckArray.Add(item);

  TCheckList::AddItem(item);
}

//
void  
TGroupList::SetItemText(int index, const tstring& text)
{
  PRECONDITION(GetItem(index));
  GetItem(index)->SetText(text);
}

//
void  
TGroupList::ClearList()
{
  TCheckList::ClearList();
  CheckArray.Flush(true);
}

//
// class TModuleListBox
// ~~~~~ ~~~~~~~~~~~~~~
//
class TModuleListBox: public TListBox{
  public:
    TModuleListBox(TWindow* parent, int Id, 
                int x, int y, int w, int h, 
                TModule* module = 0)  
                :TListBox(parent,Id,x,y,w,h,module)
                {}
    ~TModuleListBox();
};
TModuleListBox::~TModuleListBox()
{
//
}

//====================================================================================
//
// class TTraceWindow; 
// 

TTraceWindow* TTraceWindow::sTraceWnd = 0;

#define  CM_SAVETRACE       101
#define  CM_TOP              102
#define  CM_SETLEVEL        103
#define  IDC_MODULELISTBOX  200
#define  IDC_GROUPLISTBOX   201

DEFINE_RESPONSE_TABLE1 (TTraceWindow, TFrameWindow)
    EV_COMMAND         (CM_SAVETRACE,     CmSave),
    EV_COMMAND         (CM_TOP,            CmTop),
    EV_COMMAND_ENABLE  (CM_TOP,            CeTop),
    EV_COMMAND         (CM_SETLEVEL,      CmSetLevel),
    EV_COMMAND_ENABLE  (CM_SETLEVEL,      CeSetLevel),
    EV_LBN_SELCHANGE   (IDC_MODULELISTBOX,EvModuleChange),
    EV_LBN_DBLCLK      (IDC_GROUPLISTBOX, CmSetLevel),
    EV_WM_ACTIVATE,
END_RESPONSE_TABLE;


TTraceWindow::TTraceWindow ()
: 
  TFrameWindow (0, _T("Diagnostic Window"), Panels = new TPaneSplitter(0)),
  Active(true)
{
  Edit = new TTraceEdit(this);

  TraceModules  = new TModuleListBox(this, IDC_MODULELISTBOX, 0, 0, 0, 0);
  TraceGroups    = new TGroupList(this, IDC_GROUPLISTBOX, 0, 0, 0, 0);

  Edit->ModifyStyle(WS_POPUP|WS_BORDER,WS_CHILD);
  Edit->ModifyExStyle(0,WS_EX_CLIENTEDGE);

  TraceModules->ModifyStyle(WS_POPUP|WS_BORDER|LBS_SORT, WS_CHILD);
  TraceModules->ModifyExStyle(0,WS_EX_CLIENTEDGE);
  TraceGroups->ModifyStyle(WS_POPUP|WS_BORDER,WS_CHILD|LBS_SORT);
  TraceGroups->ModifyExStyle(0,WS_EX_CLIENTEDGE);

  sTraceWnd = this;
  TraceDirty = false;

  TProfile iniFile(_T("TraceWindow"), _T("OWL.INI"));
  tchar buffer[40];
  if(iniFile.GetString(_T("Position"), buffer, 40, _T("-1 -1 -1 -1"))){
    TRect rect(-1,-1,-1,-1);
    _stscanf(buffer, _T("%ld %ld %ld %ld"),&rect.left,&rect.top,&rect.right,&rect.bottom);
    Attr.X = rect.left;
    Attr.Y = rect.top;
    Attr.W = rect.right == -1 ? -1 : rect.Width();
    Attr.H = rect.bottom == -1 ? -1 : rect.Height();
  }

  bool topMost = iniFile.GetInt(_T("TopMost"), 0) == 1;
  if (topMost)
    ModifyExStyle(0,WS_EX_TOPMOST);

  if(TraceDir.length() < 3 || TraceDir[1] != _T(':') || TraceDir[2] != _T('\\')){
    tchar path[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, path);
    TraceDir = path;
  }
}

//
//
//
TTraceWindow::~TTraceWindow ()
{
  if (GetHandle())
    SaveWindowState();

  Destroy ();
  sTraceWnd = 0;

  // ???????
  delete Edit;
  delete TraceModules;
  delete TraceGroups;

}
//
//
//
void
TTraceWindow::EvActivate(uint active, bool minimized, HWND hWndOther)
{
 TWindow::EvActivate(active, minimized, hWndOther);
 Active = active;
}

//
//
//
bool
TTraceWindow::CanClose ()
{
  bool retval = TWindow::CanClose ();
  if(retval)
    SaveWindowState();
  return retval;
}

//
//
//
bool 
TTraceWindow::IdleAction (long idleCount)
{
  bool more = TFrameWindow::IdleAction (idleCount);

  if (TraceDirty)
    UpdateTraceText();

  return more;
}

//--------------------------------------------------------------------------
//  Methods that fill in parts of the trace window (the splitters or list
//  boxes).
//
void
TTraceWindow::SetupWindow ()
{
  TFrameWindow::SetupWindow ();

  tstring title = _T("Diagnostic Window - ");

  title += GetApplication()->GetMainWindow()->GetCaption();

  SetWindowText(title.c_str());

  //
  //  Build the menu in code to not force a linked in RC file:
  //
  TMenu       menu (NoAutoDelete);
  TPopupMenu  traceMenu (NoAutoDelete);

  menu.AppendMenu (MF_POPUP, traceMenu, _T("&Trace"));
  traceMenu.AppendMenu (MF_STRING, CM_SAVETRACE, _T("&Save messages..."));
  traceMenu.AppendMenu (MF_SEPARATOR);
  traceMenu.AppendMenu (MF_STRING, CM_TOP,_T("Always On &Top"));

  TPopupMenu  editMenu (NoAutoDelete);

  menu.AppendMenu (MF_POPUP, editMenu, _T("&Edit"));
  editMenu.AppendMenu (MF_STRING, CM_EDITCUT,     _T("Cu&t"));
  editMenu.AppendMenu (MF_STRING, CM_EDITCOPY,    _T("&Copy"));
  editMenu.AppendMenu (MF_STRING, CM_EDITPASTE,   _T("&Paste"));
  editMenu.AppendMenu (MF_STRING, CM_EDITDELETE,  _T("&Delete"));
  editMenu.AppendMenu (MF_STRING, CM_EDITCLEAR,   _T("C&lear"));
  editMenu.AppendMenu (MF_SEPARATOR);
  editMenu.AppendMenu (MF_STRING, CM_EDITUNDO,    _T("&Undo"));

  TPopupMenu  traceGroupMenu(NoAutoDelete);
  menu.AppendMenu(MF_POPUP, traceGroupMenu, _T("&Group"));
  traceGroupMenu.AppendMenu (MF_STRING, CM_SETLEVEL,     _T("Set level..."));

  SetMenu(menu);
  DrawMenuBar();

  Edit->LimitText (0);

  Panels->SplitPane(Edit, 0, psNone);
  Panels->SplitPane(Edit, TraceModules, psHorizontal, (float)0.80);
  Panels->SplitPane(TraceModules, TraceGroups, psVertical, (float)0.50);

  AddModules(TModule::NextModule(0));

  TSystemMenu sysMenu(GetApplication()->GetMainWindow()->GetHandle());
  sysMenu.CheckMenuItem (SCM_TRACE, MF_BYCOMMAND | MF_CHECKED);

  // Last will be diagnostic hook 
  OldHook = TDiagBase::SetGlobalHook(&TraceHook);
}

//
void
TTraceWindow::CleanupWindow()
{
  TDiagBase::SetGlobalHook(OldHook);
  
// prevent crash during BCB5 diagnostic static debug library shutdown
// SIR June 18th 2007
   TApplication *appPtr = GetApplication();
  if (appPtr) 
  {
    TWindow *mainWindowPtr = appPtr->GetMainWindow();
     if (mainWindowPtr && mainWindowPtr->GetHandle()) 
     {
       TSystemMenu sysMenu(mainWindowPtr->GetHandle());
  sysMenu.CheckMenuItem (SCM_TRACE, MF_BYCOMMAND | MF_UNCHECKED);
     }
  }   

  TFrameWindow::CleanupWindow();
}

//
tstring
TTraceWindow::FormatGroup (TDiagBase* group)
{
  tchar  level [10];
  _USES_CONVERSION;
  tstring s = _A2W(group->GetName());
  
  TProfile iniFile(_T("Diagnostics Descriptions"), _T("OWL.INI"));
  tchar buffer[80];
  if(iniFile.GetString(s.c_str(), buffer, 80, _T("")))
    s = buffer;

#if defined __GNUC__
  _tprintf(level, _T("%d"), group->GetLevel());
#else
  _itot(group->GetLevel(), level, 10);
#endif

  s += _T("    (level = ");
  s += level;
  s += _T(")");

  return s;
}

//
tstring
TTraceWindow::FormatLink (TModule* module)
{
  tstring  s = module->GetName();
  return s;
}

//
void 
TTraceWindow::SaveWindowState()
{
  TProfile iniFile(_T("TraceWindow"), _T("OWL.INI"));
  tchar buffer[40];
  wsprintf(buffer, _T("%d %d %d %d"),Attr.X,Attr.Y,Attr.Y + Attr.W,Attr.X + Attr.H);
  iniFile.WriteString(_T("Position"), buffer);
  iniFile.WriteString(_T("Path"), TraceDir.c_str());
}


//--------------------------------------------------------------------------
//  Methods that respond to menu commands
//
void 
TTraceWindow::CmSave ()
{
  TFileSaveDialog::TData  data;

  data.SetFilter(_T("Trace Files (*.t\?\?)|*.t\?\?|All Files (*.*)|*.*|"));
  data.Flags |= OFN_HIDEREADONLY | OFN_NOCHANGEDIR
                  | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
  data.InitialDir = const_cast<tchar*> (TraceDir.c_str());

  tstring filename = TraceDir + _T("\\Trace.t00");
  size_t n = 0;

  while(TFileName(filename).Exists()){  // while (file exists)
    ++n;
    if (n > 999)
      break;

    filename [filename.length() - 1] = tchar (_T('0') + (n % 10));
    filename [filename.length() - 2] = tchar (_T('0') + ((n / 10) % 10));

    if (n > 99)
      filename [filename.length() - 3] = tchar (_T('0') + ((n / 100) % 10));
  }

  ::_tcscpy (data.FileName, _tcsrchr (filename.c_str(), _T('\\')) + 1);

  TFileSaveDialog dlg (this, data);
  if (IDOK == dlg.Execute())
  {
    tstring s = Edit->GetText();
    FILE* fp = _tfopen(data.FileName, _T("wt"));
    if (fp){
      _fputts (s.c_str(), fp);
      fclose (fp);
    }
    TraceDir = data.FileName;
    n = TraceDir.length() - 1;
    while (TraceDir[n] != _T('\\'))
      --n;
    TraceDir.resize (n);
  }
}

//
void TTraceWindow::CmSetLevel ()
{
  TDiagBase*  group;
  tchar      level [10];
  _USES_CONVERSION;

  int          cursel = TraceGroups->GetSelIndex();


  if (cursel == LB_ERR)
    return;
  group = reinterpret_cast<TDiagBase*>(TraceGroups->GetData(cursel));
#if defined __GNUC__
  _tprintf(level, _T("%d"), group->GetLevel());
#else
  _itot(group->GetLevel(), level, 10);
#endif

  tstring  prompt = _T("Set level for "); 
  prompt += FormatLink(group->GetDiagModule());
  prompt += _T("::");
  prompt += _A2W(group->GetName());
  
  TModule* module = TModule::FindResModule(IDD_INPUTDIALOG, (LPCTSTR)RT_DIALOG);
  TInputDialog dlg(this, _T("Diagnostic Window"), prompt.c_str(),
                   level, COUNTOF(level), module, new TRangeValidator(0, 10));

  if (dlg.Execute()){
    uint8 l = (uint8)_ttoi(level);
    group->SetLevel(l);
    TraceGroups->SetItemText(cursel, FormatGroup(group).c_str());

    // save level into owl.ini
    TProfile iniFile(_T("Diagnostics"), _T("OWL.INI"));
    tchar buffer[20];
    wsprintf(buffer,_T("%d %d"),group->IsEnabled(),l);
    iniFile.WriteString(_A2W(group->GetName()), buffer);
  }
}

//
void  TTraceWindow::CeSetLevel(TCommandEnabler& tce)
{
  tce.Enable(TraceGroups->GetSelIndex() != LB_ERR);
}

//
void TTraceWindow::CmTop ()
{
  THandle     hWndAfter;
  Attr.ExStyle ^= WS_EX_TOPMOST;  // toggle the bit
  bool topMost = (Attr.ExStyle & WS_EX_TOPMOST) != 0;

  hWndAfter = topMost ? HWND_TOPMOST : HWND_NOTOPMOST;

  SetWindowPos (hWndAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  TProfile(_T("TraceWindow"), _T("OWL.INI")).WriteInt(_T("TopMost"), topMost ? 1 : 0);
}

//
void TTraceWindow::CeTop (TCommandEnabler& tce)
{
   tce.SetCheck ((Attr.ExStyle & WS_EX_TOPMOST) != 0);
}

//
void TTraceWindow::AddModules(TModule* _module)
{
  TModule* module = _module;
  while(module){
    int index = TraceModules->AddString(FormatLink(module));
    TraceModules->SetItemData(index, reinterpret_cast<uint32>(module));
    module = TModule::NextModule(module);
  }
  for(TDiagBase* group = TDiagBase::GetDiagGroup(0); group; group = TDiagBase::GetDiagGroup(group)){
    if(group->GetDiagModule() == _module)
      TraceGroups->AddItem(FormatGroup(group).c_str(), group->IsEnabled(), group);
  }
}

//
void TTraceWindow::EvModuleChange ()
{
  TModule* module;
  int  cursel = TraceModules->GetSelIndex();

  TraceGroups->SetRedraw (false);
  TraceGroups->ClearList();

  if (cursel != LB_ERR){
    module = reinterpret_cast<TModule*>(TraceModules->GetItemData(cursel));

    for(TDiagBase* group = TDiagBase::GetDiagGroup(0); group; group = TDiagBase::GetDiagGroup(group)){
      if(group->GetDiagModule() == module)
        TraceGroups->AddItem(FormatGroup(group).c_str(), group->IsEnabled(), group);
    }
  }

  TraceGroups->SetRedraw (true);
  TraceGroups->Invalidate ();
}

//--------------------------------------------------------------------------
//  Methods that deal with trace messages
void TTraceWindow::OutputMessage(TDiagBase* group, LPCTSTR msg)
{
  if((TraceText.length() + ::_tcslen(msg) > 32000) || Active)
    return;

  if (group){
    TraceText += group->GetDiagModule()->GetName();
    TraceText += _T(":\t");
  }

  TraceText += msg;
  TraceDirty = true;
}

//
void TTraceWindow::UpdateTraceText()
{
  tstring  temp = TraceText;
  TraceText = _T("");
  TraceDirty = false;

  Edit->AppendText(temp.c_str());
}

} // OWL namespace
//======================================================================================
