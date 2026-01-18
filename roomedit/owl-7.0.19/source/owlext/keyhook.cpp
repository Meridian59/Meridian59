//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// This class is used to encapsulate a task-specific or thread-specific
// keyboard hook, depending on 16 vs. 32-bit compile.  The way to use
// TKeyHook is to derive from it, and override the virtual OnKeystroke()
// method.  The hook can be activated via the ActivateKeyHook() method.
// If all TKeyHook objects (Win32: in a thread) are deactivated, the
// Windows hook is not set.
//
// Implementation notes:
//
// All instances of TKeyHook-derived objects (Win32: per thread) are
// kept in a chain.  When one or more objects are "active", we will set
// a Windows hook.  KeyboardProc() is our hook callback function.  In
// response to a hook callback (with code==HC_ACTION), we walk over the
// object list calling OnKeystroke() for any activate objects.
//
// Original code by Don Griffin; used with permission.
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/keyhook.h>

using namespace owl;

namespace OwlExt {

//  The list and hook are maintained per thread in Win32:
//
__declspec( thread ) static TKeyHook * sFirstHook;
__declspec( thread ) static HHOOK      sHook;


TKeyHook::TKeyHook ()
{
  // Add the new object to the list:
  mNext       = sFirstHook;
  sFirstHook  = this;

  // Objects are initially inactive:
  mActive     = false;
}


TKeyHook::~TKeyHook ()
{
  // Deactivate this object:
  ActivateKeyHook (false);

  // Remove ourselves from the object list:
  if (sFirstHook == this)
    sFirstHook = mNext;
  else{
    TKeyHook * prev = sFirstHook;

    while (prev->mNext != this){
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
//  function (KeyboardProc).  If we are the las object to become inactive,
//  we call UnhookWindowsHookEx to remove our hook.
//
void TKeyHook::ActivateKeyHook (bool activate)
{
  if (mActive == activate)
    return;

  uint count = ActiveCount();  // count before we change state...

  mActive = activate;

  if (mActive && count==0){
    HMODULE hModule;
    hModule = 0;

    PRECONDITION (! sHook);

    sHook = SetWindowsHookEx (WH_KEYBOARD, KeyboardProc, hModule,
      GetCurrentThreadId());
    count++;
  }
  else if (!mActive && count==1){
    PRECONDITION (sHook); InUse(sHook);
    UnhookWindowsHookEx (sHook);
    sHook = 0;
    count--;
  }

  PRECONDITION (sHook || count==0); InUse(count);
}


////////////////////////////////////////////////////////////////////////////
//  This static method will return the number of active objects in the
//  list.
//
uint
TKeyHook::ActiveCount ()
{
  uint count = 0;
  TKeyHook *hook = sFirstHook;

  while (hook){
    count += hook->mActive;
    hook = hook->mNext;
  }

  return count;
}


////////////////////////////////////////////////////////////////////////////
//  This method is called for each HC_ACTION hook code we get.
//
bool
TKeyHook::OnKeystroke (uint vkey, uint32 flags)
{
  TKeyHook  * nextActive = mNext;

  while (nextActive && ! nextActive->mActive)
    nextActive = nextActive->mNext;

  if (nextActive)
    return nextActive->OnKeystroke (vkey, flags);

  return sHook ? CallNextHookEx (sHook, HC_ACTION, vkey, flags) : false;
}


////////////////////////////////////////////////////////////////////////////
//  This is our keyboard hook callback.  This is where we walk the object
//  list in response to a keyboard event.
//
LRESULT CALLBACK
TKeyHook::KeyboardProc (int code, WPARAM wParam, LPARAM lParam)
{
  if (code == HC_ACTION && sFirstHook){
    TKeyHook  * firstActive = sFirstHook;

    while (firstActive && ! firstActive->mActive)
      firstActive = firstActive->mNext;

    if (firstActive)
      return firstActive->OnKeystroke (uint(wParam), uint32(lParam));
  }

  return sHook ? CallNextHookEx (sHook, code, wParam, lParam) : false;
}

} // OwlExt namespace
//==============================================================================
