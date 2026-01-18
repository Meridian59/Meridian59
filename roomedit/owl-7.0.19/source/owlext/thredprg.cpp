//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// Provides the classes TGscThread, which is a progress-notifying thread class,
// and TThreadProgressDlg, which is a Progress dialog that handles thread
// creation and destruction.
//
// Works in both 16- and 32- bit environments
//
// Original code:
// Copyright (c) 1997 Rich Goldstein, MD
// goldstei@interport.net
// May be used without permission, if appropriate credit is given
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/thredprg.h>


using namespace owl;

namespace OwlExt {


TProgressThread::TProgressThread(TProgressDlg* target, uint id) :
  TThread(),
  target(target), id(id)
{ }



void TProgressThread::NotifyStart(int len)
{
  if (target)
    target->NotifyProgressStart(id, len);
}

void TProgressThread::NotifyProgress(int curr)
{
  if (target)
    target->NotifyProgress(id, curr);
}

void TProgressThread::NotifyEnd(int ret)
{
  if (target)
    target->NotifyProgressEnd(id, ret);
}

void TProgressThread::NotifyProgressMessage(const owl::tstring& str)
{
  if (target)
    target->NotifyProgressMessage(id, str);
}

void TProgressThread::NotifyProgressError(const owl::tstring& err)
{
  if (target)
    target->NotifyProgressError(id, err);
}


//
DEFINE_RESPONSE_TABLE1(TThreadProgressDlg, TProgressDlg)
EV_COMMAND(IDCANCEL, CmCancel),
END_RESPONSE_TABLE;

TThreadProgressDlg::TThreadProgressDlg(TWindow* parent, LPCTSTR text,
                     TResId id, TModule* mod)
                     : TProgressDlg(parent, text, id, mod)
                     , _othread(0), notifyEndCalled(false)
{}

TThreadProgressDlg::~TThreadProgressDlg()
{
  delete _othread;
}

void TThreadProgressDlg::SetupWindow()
{
  TProgressDlg::SetupWindow();
  _othread = BuildThread();
  if (_othread)
    _othread->Start();
  else
    CloseWindow(IDCANCEL);
}

void TThreadProgressDlg::CleanupWindow()
{
  if (_othread && _othread->GetStatus() != TThread::Finished)
    _othread->TerminateAndWait(TThread::NoLimit);
  TProgressDlg::CleanupWindow();
}

bool TThreadProgressDlg::CanClose()
{
  if (!notifyEndCalled && _othread && _othread->GetStatus() != TProgressThread::Finished)
    return false;
  else
    return true;
}

void TThreadProgressDlg::CmCancel()
{
  if (_othread && _othread->GetStatus() != TThread::Finished)
    _othread->TerminateAndWait(TThread::NoLimit);
  TProgressDlg::CmCancel();
}

void TThreadProgressDlg::EvProgressEnd(uint id, int returnValue)
{
  notifyEndCalled = true;
  TProgressDlg::EvProgressEnd(id, returnValue);
}

} // OwlExt namespace



//
// Test code follows
#if defined(TEST_THE_CODE)
#include <owl/statusba.h>
#include <owl/controlg.h>
#include "gsc/testprg.rh"
#include "gsc/gaugegad.h"

using namespace OwlExt;

class SillyThread : public TProgressThread
{
public:
  SillyThread(HWND hWnd, int count)
    : TProgressThread(0, hWnd), _count(count)
  {}

  int Run()
  {
    NotifyStart(_count);
    for (int i = 0; i < _count && !ShouldTerminate(); ++i)
    {
      Sleep(100);
      NotifyProgress(i);
    }
    NotifyEnd();
    return 0;
  }

protected:
  int   _count;
};

class SillyProgressDlg : public TThreadProgressDlg
{
public:
  SillyProgressDlg(TWindow* parent)
    : TThreadProgressDlg(parent, "Running Silly Loop")
  {}

  TProgressThread* BuildThread() { return new SillyThread(GetHandle(), 50); }
};



class TProgressFrame : public TDecoratedFrame
{
public:
  TProgressFrame(TWindow* parent,
    const char * title,
    TWindow* clientWnd,
    bool trackMenuSelection = false,
    TModule* module = 0);

protected:
  void CmBeginThread();
  void CmEndThread();
  void CmDialogThread();

  void CleanupWindow();

  LRESULT EvProgressStart(TParam1, TParam2 range)
  { _gauge->SetRange(0, (int)range); return 0; }
  LRESULT EvProgress(TParam1, TParam2 value)
  { _gauge->SetValue((int)value); return 0; }
  LRESULT EvProgressEnd(TParam1, TParam2);

  TStatusBar* _status;
  TGaugeGadget*   _gauge;
  // TGauge*     _gauge;

  TProgressThread* _othread;

  DECLARE_RESPONSE_TABLE(TProgressFrame);
};

DEFINE_RESPONSE_TABLE1(TProgressFrame, TDecoratedFrame)
EV_MESSAGE(WM_GSC_PROGRESS_START, EvProgressStart),
EV_MESSAGE(WM_GSC_PROGRESS, EvProgress),
EV_MESSAGE(WM_GSC_PROGRESS_END, EvProgressEnd),
EV_COMMAND(CM_BEGINTHREAD, CmBeginThread),
EV_COMMAND(CM_ENDTHREAD, CmEndThread),
EV_COMMAND(CM_DIALOGTHREAD, CmDialogThread),
END_RESPONSE_TABLE;

TProgressFrame::TProgressFrame(TWindow* parent,
                 const char * title,
                 TWindow* clientWnd,
                 bool trackMenuSelection,
                 TModule* module)
                 : TDecoratedFrame(parent, title, clientWnd, trackMenuSelection, module)
                 , _othread(0)
{
  _status = new TStatusBar(0, TGadget::Recessed,
    TStatusBar::CapsLock |
    TStatusBar::NumLock |
    TStatusBar::Overtype);
  // Create a gauge control
  _gauge = new TGaugeGadget(1, "%d%%", 96);
  _status->Insert(*_gauge);
  Insert(*_status, TDecoratedFrame::Bottom);
}

void TProgressFrame::CleanupWindow()
{
  if (_othread)
  {
    if (_othread->GetStatus() != TProgressThread::Finished)
      _othread->TerminateAndWait(TProgressThread::NoLimit);
    delete _othread;
  }
  TDecoratedFrame::CleanupWindow();
}


LRESULT TProgressFrame::EvProgressEnd(WPARAM, LPARAM)
{
  int l, h;
  _gauge->GetRange(l, h);
  _gauge->SetValue(l);

  // Wait for thread to end, then delete the thread object
  if (_othread)
  {
    if (_othread->GetStatus() != TProgressThread::Finished)
      _othread->TerminateAndWait(TProgressThread::NoLimit);
    delete _othread;
    // Prevent re-entrancy in 16 bit versions, and multiple
    // instances of the thread in 32 bit versions
    _othread = 0;
  }

  return 0;
}

void TProgressFrame::CmDialogThread()
{
  SillyProgressDlg(this).Execute();
}

void TProgressFrame::CmEndThread()
{
  if (_othread && _othread->GetStatus() != TProgressThread::Finished)
    _othread->Terminate();
}

void TProgressFrame::CmBeginThread()
{
  if (_othread)
  {
    MessageBox("A thread is already running.");
    return;
  }
  _othread = new SillyThread(GetHandle(), 100);

  // In 16 bit version, be careful to avoid re-entrancy!
  if (_othread)
    _othread->Start();
}

class App : public TApplication
{
public:
  App()
    : TApplication("Thread Progress Demo")
  { }

  ~App()
  { }


  void InitMainWindow()
  {
    TFrameWindow* frame = new TProgressFrame(0,
      GetName(),
      new TWindow(0, ""),
      false);
    frame->AssignMenu(MAINMENU);
    SetMainWindow(frame);

  }
};

int OwlMain (int , char* [])
{
  try {
    App app;
    return app.Run();
  }
  catch (xmsg& x) {
    ::MessageBox(0, x.why().c_str(), "Exception", MB_OK);
  }
  return -1;
}
#endif // TEST_THE_CODE
//

