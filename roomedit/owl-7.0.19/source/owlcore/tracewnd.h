//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// Definition of private class TTraceWindow.
//----------------------------------------------------------------------------

#ifndef OWL_TRACEWND_H
#define OWL_TRACEWND_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/defs.h>
#include <owl/framewin.h>

namespace owl {

#include <owl/preclass.h>

class _OWLCLASS TPaneSplitter;
class _OWLCLASS TImageList;
class _OWLCLASS TEdit;
class _OWLCLASS TListBox;

class TTraceWindow
  : public TFrameWindow, public TDiagBaseHook
{
public:

  static auto GetInstance(bool shouldCreateIfNeccessary = false) -> TTraceWindow*;
  static void DestroyInstance();
  void Output(TDiagBase* group, LPCTSTR str) override;
  auto IdleAction(long) -> bool override;

protected:

  void SetupWindow() override;
  void CleanupWindow() override;

private:

  static TTraceWindow* Instance;
  TDiagBaseHook* OldHook;
  TPaneSplitter& Panels;
  TEdit& TraceMessages;
  class TGroupList;
  TGroupList& TraceGroups;
  TListBox& TraceModules;
  bool TraceDirty;
  bool Active;
  tstring TracePath;
  tstring TraceText;

  TTraceWindow();
  ~TTraceWindow() override;

  void CmSave();
  void CmTop();
  void CeTop(TCommandEnabler &);
  void CmEditClear();
  void CmEditSelectAll();
  void CmSetLevel();
  void CeSetLevel(TCommandEnabler &);
  void CmNextPane();
  void CmPreviousPane();

  void LbnSelChangeModule();
  void EvActivate(uint active, bool minimized, HWND hWndOther);

  void SaveWindowState();
  void RestoreWindowState();
  void AddModules(TModule*  module);
  void UpdateTraceText();
  auto FormatGroup(TDiagBase* group) -> tstring;
  auto FormatLink(TModule* group) -> tstring;

  friend struct TTraceWindowHook;

  DECLARE_RESPONSE_TABLE(TTraceWindow);
};

#include <owl/posclass.h>

} // OWL namespace

#endif  // __OWL_TRACEWND_H

