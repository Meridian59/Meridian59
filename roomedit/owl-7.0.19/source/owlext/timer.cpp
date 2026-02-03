//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// TTimer
//
// This file implements TTimer, a class that enapsulates SetTimer API.
// The reason for using a hidden window, as opposed to a timer proc, is
// that in Win16, timer procs can be called from other application's
// context.  Since this is usually a very bad thing, we use a window to
// get WM_TIMER messages.  Either would probably work in Win32, but we
// want to work in both if possible.
//
// Original code by Don Griffin; used with permission.
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owl/appdict.h>

#include <owlext/timer.h>

using namespace owl;

namespace OwlExt {

//--------------------------------------------------------------------------

#define  IDT_TIMER      100

class TTimerWindow : public TWindow
{
  DECLARE_RESPONSE_TABLE (TTimerWindow);

public:
  TTimerWindow ();
  ~TTimerWindow ();

  TTimer    * mFirst;

private:
  void SetupWindow () override;
  void CleanupWindow () override;
  void            EvTimer (uint);
};

DEFINE_RESPONSE_TABLE1 (TTimerWindow, TWindow)
EV_WM_TIMER,
END_RESPONSE_TABLE;

TTimerWindow::TTimerWindow ()
:
TWindow (GetApplicationObject()->GetMainWindow(), _T(""))
{
  ModifyStyle(WS_VISIBLE|WS_CHILD,WS_POPUP);
}

TTimerWindow::~TTimerWindow ()
{
  Destroy ();
}


void TTimerWindow::SetupWindow ()
{
  TWindow::SetupWindow ();

  SetTimer (IDT_TIMER, 50);  // 50ms is about the best resolution possible
}

void TTimerWindow::CleanupWindow ()
{
  KillTimer (IDT_TIMER);

  TWindow::CleanupWindow ();
}

void TTimerWindow::EvTimer (uint)
{
  TTimer::DoTicks ();
}


//--------------------------------------------------------------------------

static TTimerWindow   * sTimerWindow;
TTimer                * TTimer::sFirst;


TTimer::TTimer (uint duration)
:
mDuration (duration),
mLastTick (::GetTickCount())
{
  if (! sTimerWindow) {
    sTimerWindow = new TTimerWindow;
    PRECONDITION (sTimerWindow);  // new should throw and we never get here...
    sTimerWindow->Create ();
  }

  mProcessing = false;

  //  Add ourselves to the timer chain:
  mNext = sFirst;
  sFirst = this;
}


TTimer::~TTimer ()
{
  const bool ok = sTimerWindow != 0;
  WARN (!ok, _T ("TTimer::~TTimer: Terminating due to failed precondition."));
  if (!ok) std::terminate ();

  TTimer    * prev = 0;
  TTimer    * rover = sFirst;

  while (rover){
    if (rover == this){
      if (prev)
        prev->mNext = mNext;
      else
        sFirst = mNext;

      break;
    }

    prev = rover;
    rover = rover->mNext;
  }
  const bool found = rover != 0; // We walked the list and should find ourselves!
  WARN (!found, _T ("TTimer::~TTimer: Terminating due to failed precondition."));
  if (!found) std::terminate ();

  //  See if we were the last TTimer object:
  if (! sFirst){
    delete sTimerWindow; // don't need this guy anymore...
    sTimerWindow = 0;
  }
}

void TTimer::DoTick (uint32 tickCount)
{
  if (mLastTick + mDuration < tickCount)
  {
    mLastTick = tickCount;

    //  If the Tick method yields time, we may be called again.
    //  We prevent re-entrancy here:
    if (! mProcessing)
    {
      mProcessing = true;

      Tick ();

      if (IsTimer())  // we may have been deleted!
        mProcessing = false;
    }
  }
}

void TTimer::DoTicks ()
{
  TTimer    * timer = sFirst;
  TTimer    * nextTimer;
  uint32      tickCount  = ::GetTickCount();


  PRECONDITION (timer);

  while (timer)
  {
    nextTimer = timer->mNext;
    timer->DoTick (tickCount);
    timer = nextTimer;
  }
}

bool TTimer::IsTimer (TTimer *timer)
{
  TTimer    * rover = sFirst;

  while (rover)
  {
    if (rover == timer)
      return true;
    rover = rover->mNext;
  }

  return false;
}

} // OwlExt namespace

