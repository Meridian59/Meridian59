//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// This class is used to encapsulate a task-specific or thread-specific
// mouse hook, depending on 16 vs. 32-bit compile.  The way to use
// TMouseHook is to derive from it, and override the virtual OnMouseEvent()
// method.  The hook can be activated via the ActivateMouseHook() method.
// If all TMouseHook objects (Win32: in a thread) are deactivated, the
// Windows hook is not set.
//
// Implementation notes:
//
// All instances of TMouseHook-derived objects (Win32: per thread) are
// kept in a chain.  When one or more objects are "active", we will set
// a Windows hook.  MouseProc() is our hook callback function.  In response
// to a hook callback (with code==HC_ACTION), we walk over the object list
// calling OnMouseEvent() for any activate objects.
//
// Original code by Don Griffin; used with permission.
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/mousehk.h>

using namespace owl;


namespace OwlExt {



//  The list and hook are maintained per thread in Win32:
//
__declspec( thread ) static TMouseHook*  sFirstHook;
__declspec( thread ) static HHOOK       sHook;



TMouseHook::TMouseHook ()
{
  // Add the new object to the list:
  mNext       = sFirstHook;
  sFirstHook  = this;

  // Objects are initially inactive:
  mActive     = false;
}


TMouseHook::~TMouseHook ()
{
  // Deactivate this object:
  ActivateMouseHook (false);

  // Remove ourselves from the object list:
  if (sFirstHook == this)
    sFirstHook = mNext;
  else
  {
    TMouseHook * prev = sFirstHook;

    while (prev->mNext != this)
    {
      prev = prev->mNext;

      const bool ok = VALIDPTR (prev);
      WARN (!ok, _T ("TKeyHook::~TKeyHook: Terminating due to failed precondition."));
      if (!ok) std::terminate ();
    }

    prev->mNext = mNext;
  }
}


////////////////////////////////////////////////////////////////////////////
//  This method will activate or deactivate this object.  If we are the
//  first to become active, we call SetWindowsHookEx to install our hook
//  function (MouseProc).  If we are the last object to become inactive,
//  we call UnhookWindowsHookEx to remove our hook.
//
void TMouseHook::ActivateMouseHook (bool activate)
{
  if (mActive == activate)
    return;

  uint count = ActiveCount();  // count before we change state...

  mActive = activate;

  if (mActive && count==0)
  {
    HMODULE hModule;

    hModule = 0;

    PRECONDITION (! sHook);

    sHook = SetWindowsHookEx (WH_MOUSE, MouseProc, hModule,
      GetCurrentThreadId());
    ++count;
  }
  else if (!mActive && count==1)
  {
    PRECONDITION (sHook);
    UnhookWindowsHookEx (sHook);
    sHook = 0;
    --count;
  }

  PRECONDITION (sHook || count==0); InUse(count);
}


////////////////////////////////////////////////////////////////////////////
//  This static method will return the number of active objects in the
//  list.
//
uint TMouseHook::ActiveCount ()
{
  uint count = 0;
  TMouseHook *hook = sFirstHook;

  while (hook)
  {
    count += hook->mActive;
    hook = hook->mNext;
  }

  return count;
}


////////////////////////////////////////////////////////////////////////////
//  This method is called for each HC_ACTION hook code we get.
//
bool TMouseHook::OnMouseEvent (uint msg, MOUSEHOOKSTRUCT &mouseHook)
{
  TMouseHook  * nextActive = mNext;

  while (nextActive && ! nextActive->mActive)
    nextActive = nextActive->mNext;

  if (nextActive)
    return nextActive->OnMouseEvent (msg, mouseHook);

  return sHook
    ? CallNextHookEx (sHook, HC_ACTION, msg,
    reinterpret_cast<LPARAM> (&mouseHook))
    : false;
}


////////////////////////////////////////////////////////////////////////////
//  This is our mouse hook callback.  This is where we walk the object
//  list in response to an event.
//
LRESULT CALLBACK
TMouseHook::MouseProc (int code, WPARAM wParam, LPARAM lParam)
{
  if (code == HC_ACTION && sFirstHook)
  {
    TMouseHook * firstActive = sFirstHook;

    while (firstActive && ! firstActive->mActive)
      firstActive = firstActive->mNext;

    if (firstActive)
    {
      MOUSEHOOKSTRUCT   * mouseHook;

      mouseHook = reinterpret_cast <MOUSEHOOKSTRUCT *> (lParam);

      return firstActive->OnMouseEvent (uint(wParam), *mouseHook);
    }
  }

  return sHook ? CallNextHookEx (sHook, code, wParam, lParam) : false;
}


} // OwlExt namespace
//==============================================================================
