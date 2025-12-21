//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// Implementation of the private TTraceWindow class.
//----------------------------------------------------------------------------

#include <owl/pch.h>
#pragma hdrstop

#include "tracewnd.h"
#include <owl/opensave.h>
#include <owl/inputdia.h>
#include <owl/panespli.h>
#include <owl/profile.h>
#include <owl/edit.h>
#include <owl/listbox.h>
#include <owl/checklst.h>
#include <owl/validate.h>

using namespace std;
using namespace std::filesystem;

namespace owl {
OWL_DIAGINFO;

auto GetDiagIniFullFileName_(LPCSTR filename) -> tstring; // Private function defined in "diaginit.cpp".

//-------------------------------------------------------------------------------------------------

class TTraceWindow::TGroupList
  : public TCheckList
{
public:

  TGroupList(TWindow* parent, int id)
    : TCheckList(parent, id, 0, 0, 0, 0)
  {}

  void AddItem(const tstring& text, bool checked, TDiagBase* group)
  {
    const auto i = new TCheckListItem(text, checked ? BF_CHECKED : BF_UNCHECKED);
    i->SetData(reinterpret_cast<UINT_PTR>(group));
    TCheckList::AddItem(i);
  }

  auto GetData(int index) -> TDiagBase*
  { 
    PRECONDITION(GetItem(index));
    return reinterpret_cast<TDiagBase*>(GetItem(index)->GetData());
  }

  void SetItemText(int index, const tstring& text)
  {
    PRECONDITION(GetItem(index));
    GetItem(index)->SetText(text);
  }

private:

  void EvLButtonDown(uint modKeys, const TPoint& point)
  {
    TCheckList::EvLButtonDown(modKeys, point);
    const auto index = GetCaretIndex();
    const auto i = GetItem(index);
    if (i && i->IsEnabled() && point.x < CheckList_BoxWidth)
      SetGroupState(index, i->IsChecked());
  }

  void EvChar(uint key, uint repeatCount, uint flags)
  {
    TCheckList::EvChar(key, repeatCount, flags);
    const auto index = GetCaretIndex();
    const auto i = GetItem(index);
    if (i && i->IsEnabled() && key == _T(' '))
      SetGroupState(index, i->IsChecked());
  }

  void SetGroupState(int index, bool enabled)
  {
    const auto group = GetData(index);
    if (!group) return;
    group->Enable(enabled);

    // Save level into "OWL.INI".
    //
    auto s = tostringstream{};
    s << (group->IsEnabled() ? 1 : 0) << _T(' ') << group->GetLevel();
    TProfile ini(_T("Diagnostics"), GetDiagIniFullFileName_(OWL_INI));
    ini.WriteString(to_tstring(group->GetName()), s.str());
  }

  DECLARE_RESPONSE_TABLE(TGroupList);
};

DEFINE_RESPONSE_TABLE1(TTraceWindow::TGroupList, TCheckList)
  EV_WM_LBUTTONDOWN,
  EV_WM_CHAR,
END_RESPONSE_TABLE;

//-------------------------------------------------------------------------------------------------

namespace {

enum
{
  Cm_SaveTrace = 100,
  Cm_Top,
  Cm_EditClear,
  Cm_EditSelectAll,
  Cm_SetLevel,
  Cm_NextPane,
  Cm_PreviousPane
};

enum
{
  Idc_TraceEdit = 200,
  Idc_ModuleListBox,
  Idc_GroupListBox
};

} // namespace

TTraceWindow* TTraceWindow::Instance = nullptr;

DEFINE_RESPONSE_TABLE1(TTraceWindow, TFrameWindow)
  EV_COMMAND(Cm_SaveTrace, CmSave),
  EV_COMMAND(Cm_Top, CmTop),
  EV_COMMAND_ENABLE(Cm_Top, CeTop),
  EV_COMMAND(Cm_EditClear, CmEditClear),
  EV_COMMAND(Cm_EditSelectAll, CmEditSelectAll),
  EV_COMMAND(Cm_SetLevel, CmSetLevel),
  EV_COMMAND_ENABLE(Cm_SetLevel, CeSetLevel),
  EV_COMMAND(Cm_NextPane, CmNextPane),
  EV_COMMAND(Cm_PreviousPane, CmPreviousPane),
  EV_LBN_SELCHANGE(Idc_ModuleListBox, LbnSelChangeModule),
  EV_LBN_DBLCLK(Idc_GroupListBox, CmSetLevel),
  EV_WM_ACTIVATE,
END_RESPONSE_TABLE;

auto TTraceWindow::GetInstance(bool shouldCreateIfNeccessary) -> TTraceWindow*
{
  if (shouldCreateIfNeccessary && !Instance)
    Instance = new TTraceWindow{};
  return Instance;
}

void TTraceWindow::DestroyInstance()
{
  if (Instance)
    delete Instance;
}
  
void TTraceWindow::Output(TDiagBase* group, LPCTSTR msg)
{
  PRECONDITION(group);

  // Don't process trace messages from the Diagnostic Window itself.
  //
  if (Active) return;

  TraceText += group->GetDiagModule()->GetName();
  TraceText += _T(": ");
  TraceText += msg;
  TraceDirty = true;
}

auto TTraceWindow::IdleAction(long idleCount) -> bool
{
  const auto more = TFrameWindow::IdleAction(idleCount);
  if (TraceDirty)
    UpdateTraceText();
  return more;
}

void TTraceWindow::SetupWindow()
{
  TFrameWindow::SetupWindow();

  const auto title = tstring{_T("Diagnostic Window - ")} +
    GetApplication()->GetMainWindow()->GetCaption();
  SetWindowText(title);

  //
  //  Build the menu in code to not force a linked in RC file:
  //
  TMenu menu{NoAutoDelete};
  TPopupMenu traceMenu{NoAutoDelete};
  menu.AppendMenu(MF_POPUP, traceMenu, _T("&Trace"));
  traceMenu.AppendMenu(MF_STRING, Cm_SaveTrace, _T("&Save messages...\tCtrl+S"));
  traceMenu.AppendMenu(MF_SEPARATOR);
  traceMenu.AppendMenu(MF_STRING, Cm_Top, _T("Always On &Top"));

  TPopupMenu editMenu{NoAutoDelete};
  menu.AppendMenu(MF_POPUP, editMenu, _T("&Edit"));
  editMenu.AppendMenu(MF_STRING, CM_EDITCOPY, _T("&Copy\tCtrl+C"));
  editMenu.AppendMenu(MF_STRING, Cm_EditClear, _T("C&lear"));
  editMenu.AppendMenu(MF_SEPARATOR);
  editMenu.AppendMenu(MF_STRING, Cm_EditSelectAll, _T("Select &all\tCtrl+A"));

  TPopupMenu traceGroupMenu{NoAutoDelete};
  menu.AppendMenu(MF_POPUP, traceGroupMenu, _T("&Group"));
  traceGroupMenu.AppendMenu(MF_STRING, Cm_SetLevel, _T("Set level...\tEnter"));

  SetMenu(menu);
  DrawMenuBar();

  const ACCEL accelerators[6]
  {
    {FVIRTKEY, VK_F11, Cm_SaveTrace},
    {FVIRTKEY | FCONTROL, 'S', Cm_SaveTrace},
    {FVIRTKEY | FCONTROL, 'A', Cm_EditSelectAll},
    {FVIRTKEY, VK_RETURN, Cm_SetLevel},
    {FVIRTKEY, VK_F6, Cm_NextPane},
    {FVIRTKEY | FSHIFT, VK_F6, Cm_PreviousPane}
  };
  SethAccel(CreateAcceleratorTable(const_cast<LPACCEL>(accelerators), static_cast<int>(COUNTOF(accelerators))));

  TraceMessages.LimitText(0);

  Panels.SplitPane(&TraceMessages, nullptr, psNone);
  Panels.SplitPane(&TraceMessages, &TraceModules, psHorizontal, 0.50f);
  Panels.SplitPane(&TraceModules, &TraceGroups, psVertical, 0.50f);

  const auto font = TDefaultGuiFont{TDefaultGuiFont::sfiMessage};
  TraceMessages.SetWindowFont(font, false);
  TraceModules.SetWindowFont(font, false);
  TraceGroups.SetWindowFont(font, false);

  AddModules(TModule::NextModule(nullptr));
  OldHook = TDiagBase::SetGlobalHook(this);
}

void TTraceWindow::CleanupWindow()
{
  TDiagBase::SetGlobalHook(OldHook);
  SaveWindowState();
  TFrameWindow::CleanupWindow();
}

TTraceWindow::TTraceWindow()
  : TFrameWindow(nullptr, _T("Diagnostic Window")),
  OldHook{},
  Panels{*new TPaneSplitter{this}},
  TraceMessages{*new TEdit{this, Idc_TraceEdit, _T(""), 0, 0, 0, 0, 0, true}},
  TraceGroups{*new TGroupList(this, Idc_GroupListBox)},
  TraceModules{*new TListBox(this, Idc_ModuleListBox, 0, 0, 0, 0)},
  TraceDirty{false},
  Active{true},
  TracePath{to_tstring(current_path())},
  TraceText{}
{
  Instance = this;
  SetClientWindow(&Panels);

  TraceMessages.ModifyStyle(WS_POPUP | WS_BORDER, ES_READONLY | WS_CHILD);
  TraceModules.ModifyStyle(WS_POPUP | WS_BORDER | LBS_SORT, WS_CHILD | LBS_NOINTEGRALHEIGHT);
  TraceModules.ModifyExStyle(0, WS_EX_CLIENTEDGE);
  TraceGroups.ModifyStyle(WS_POPUP | WS_BORDER, WS_CHILD | LBS_SORT);

  RestoreWindowState();
}

TTraceWindow::~TTraceWindow()
{
  Destroy();
  Instance = nullptr;
}

void TTraceWindow::CmSave()
{
  auto data = TFileSaveDialog::TData{};
  data.SetFilter(_T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*|"));
  data.Flags |= OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
  data.InitialDir = const_cast<tchar*>(TracePath.c_str());

  auto f = to_tstring(path{TracePath} / _T("Trace-000.txt"));
  const auto n = f.size();
  auto i = size_t{0};
  while (exists(f))
  {
    ++i;
    if (i > 999)
      break;
    const auto makeDigit = [](size_t i) { return static_cast<tchar>('0' + (i % 10)); };
    f[n - 5] = makeDigit(i / 1);
    f[n - 6] = makeDigit(i / 10);
    f[n - 7] = makeDigit(i / 100);
  }
  _tcscpy_s(data.FileName, data.MaxPath, to_tstring(path{f}.filename()).c_str());

  TFileSaveDialog dlg{this, data};
  if (dlg.Execute() == IDOK)
  {
    const auto& s = TraceMessages.GetText();
    tofstream{data.FileName} << s;
    TracePath = to_tstring(path{data.FileName}.parent_path());
  }
}

void TTraceWindow::CmTop()
{
  Attr.ExStyle ^= WS_EX_TOPMOST;  // toggle the bit
  const auto topMost = (Attr.ExStyle & WS_EX_TOPMOST) != 0;
  SetWindowPos(topMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  auto ini = TProfile{_T("TraceWindow"), GetDiagIniFullFileName_(OWL_INI)};
  ini.WriteInt(_T("TopMost"), topMost ? 1 : 0);
}

void TTraceWindow::CeTop(TCommandEnabler& tce)
{
  tce.SetCheck((Attr.ExStyle & WS_EX_TOPMOST) != 0);
}

void TTraceWindow::CmEditClear()
{
  TraceMessages.SetWindowText(_T(""));
}

void TTraceWindow::CmEditSelectAll()
{
  TraceMessages.SetSelection(0, -1);
  TraceMessages.SetFocus();
}

void TTraceWindow::CmSetLevel()
{
  const auto i = TraceGroups.GetSelIndex();
  if (i == LB_ERR) return;
  const auto group = reinterpret_cast<TDiagBase*>(TraceGroups.GetData(i));
  auto s = tostringstream{};
  s << group->GetLevel();
  const auto levelString = s.str();

  auto prompt = tstring{_T("Set level for ")};
  prompt += FormatLink(group->GetDiagModule());
  prompt += _T("::");
  prompt += to_tstring(group->GetName());

  const auto m = TModule::FindResModule(IDD_INPUTDIALOG, TResId{RT_DIALOG});
  TInputDialog dlg{&TraceGroups, _T("Diagnostic Window"), prompt, levelString, m, new TRangeValidator{INT_MIN, INT_MAX}};
  if (dlg.Execute())
  {
    auto level = 0;
    tistringstream{dlg.GetBuffer()} >> level;
    group->SetLevel(level);
    TraceGroups.SetItemText(i, FormatGroup(group).c_str());

    // Save the state to the configuration file.
    //
    auto ini = TProfile{_T("Diagnostics"), GetDiagIniFullFileName_(OWL_INI)};
    auto s = tostringstream{};
    s << (group->IsEnabled() ? 1 : 0) << _T(' ') << level;
    ini.WriteString(to_tstring(group->GetName()), s.str());
  }
}

void  TTraceWindow::CeSetLevel(TCommandEnabler& tce)
{
  tce.Enable(TraceGroups.GetSelIndex() != LB_ERR);
}

void TTraceWindow::CmNextPane()
{
  const auto w = GetFocus();
  if (w == TraceMessages.GetHandle())
    TraceModules.SetFocus();
  else if (w == TraceModules.GetHandle())
    TraceGroups.SetFocus();
  else
    TraceMessages.SetFocus();
}

void TTraceWindow::CmPreviousPane()
{
  const auto w = GetFocus();
  if (w == TraceMessages.GetHandle())
    TraceGroups.SetFocus();
  else if (w == TraceModules.GetHandle())
    TraceMessages.SetFocus();
  else
    TraceModules.SetFocus();
}

void TTraceWindow::EvActivate(uint active, bool minimized, HWND hWndOther)
{
  TWindow::EvActivate(active, minimized, hWndOther);
  Active = active;
}

void TTraceWindow::LbnSelChangeModule()
{
  const auto i = TraceModules.GetSelIndex();
  if (i == LB_ERR) return;
  const auto module = reinterpret_cast<TModule*>(TraceModules.GetItemData(i));
  TraceGroups.SetRedraw(false);
  TraceGroups.ClearList();
  for (auto group = TDiagBase::GetDiagGroup(); group; group = TDiagBase::GetDiagGroup(group))
    if (group->GetDiagModule() == module)
      TraceGroups.AddItem(FormatGroup(group), group->IsEnabled(), group);
  TraceGroups.SetRedraw(true);
  TraceGroups.Invalidate();
}

void TTraceWindow::SaveWindowState()
{
  auto ini = TProfile{_T("TraceWindow"), GetDiagIniFullFileName_(OWL_INI)};
  auto s = tostringstream{};
  const auto ws = _T(' ');
  s << Attr.X << ws << Attr.Y << ws << (Attr.X + Attr.W) << ws << (Attr.Y + Attr.H);
  ini.WriteString(_T("Position"), s.str());
  ini.WriteString(_T("Path"), TracePath.c_str());
}

void TTraceWindow::RestoreWindowState()
{
  auto ini = TProfile{_T("TraceWindow"), GetDiagIniFullFileName_(OWL_INI)};
  auto is = tistringstream{ini.GetString(_T("Position"), _T("-1 -1 -1 -1"))};
  auto r = TRect{};
  is >> r.left >> r.top >> r.right >> r.bottom;
  if (!r.IsEmpty())
  {
    Attr.X = r.left;
    Attr.Y = r.top;
    Attr.W = r.Width();
    Attr.H = r.Height();
  }
  bool topMost = ini.GetInt(_T("TopMost"), 0) == 1;
  if (topMost)
    ModifyExStyle(0, WS_EX_TOPMOST);
  TracePath = ini.GetString(_T("Path"), TracePath);
}

void TTraceWindow::AddModules(TModule* module)
{
  for (auto m = module; m; m = TModule::NextModule(m))
  {
    const auto i = TraceModules.AddString(FormatLink(m));
    TraceModules.SetItemData(i, reinterpret_cast<LPARAM>(m));
  }
  for (auto g = TDiagBase::GetDiagGroup(nullptr); g; g = TDiagBase::GetDiagGroup(g))
    if (g->GetDiagModule() == module)
      TraceGroups.AddItem(FormatGroup(g), g->IsEnabled(), g);
}

void TTraceWindow::UpdateTraceText()
{
  if (!IsWindow()) return;
  const auto tmp = TraceText;
  TraceText = _T("");
  TraceDirty = false;
  const auto n = TraceMessages.GetWindowTextLength();
  TraceMessages.SetSelection(n, -1);
  TraceMessages.Insert(tmp);
}

auto TTraceWindow::FormatGroup(TDiagBase* group) -> tstring
{
  const auto n = to_tstring(group->GetName());
  auto ini = TProfile{_T("Diagnostics Descriptions"), GetDiagIniFullFileName_(OWL_INI)};
  const auto d = ini.GetString(n, n); // Replace name by description, if present.
  auto s = tostringstream{};
  s << d << _T(" (") << group->GetLevel() << _T(')');
  return s.str();
}

auto TTraceWindow::FormatLink(TModule* module) -> tstring
{
  return module->GetName();
}

} // OWL namespace
