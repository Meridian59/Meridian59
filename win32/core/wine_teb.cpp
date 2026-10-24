/*
 * wine_teb.cpp:  The thread environment block Wine's sources read.  Built with
 *   __WINESRC__, Wine's headers inline GetLastError, SetLastError,
 *   GetCurrentThreadId, GetCurrentProcessId and GetProcessHeap as reads of
 *   NtCurrentTeb() (%gs:0x30 on x86_64, which isn't a TEB here); wine_prelude.h
 *   sends NtCurrentTeb() to m59_NtCurrentTeb, a per-thread block laid out where
 *   those inlines look.  The layer's last error lives in it, so Wine's code and
 *   the layer's SetLastError / GetLastError see the same value.
 */

#include "core/win32.h"

#include <unistd.h>

namespace {

// The fields' places, in pointers: [8] process id, [9] thread id, [12] the PEB
// (its [6] the process heap), [13] the last error (a DWORD).
struct FakeTeb
{
   void *slots[64] = {};
};

struct FakePeb
{
   void *slots[16] = {};
};

FakePeb *Peb()
{
   static FakePeb *peb = [] {
      auto *p = new FakePeb;
      p->slots[6] = (void *) GetProcessHeap();
      return p;
   }();
   return peb;
}

FakeTeb *Teb()
{
   // Never freed: Wine's code may run on the thread until it ends.
   thread_local FakeTeb *teb = nullptr;
   if (teb == nullptr)
   {
      teb = new FakeTeb;
      teb->slots[12] = Peb();
      teb->slots[8] = (void *) (ULONG_PTR) getpid();
      teb->slots[9] = (void *) (ULONG_PTR) GetCurrentThreadId();
   }
   return teb;
}

}  // namespace

namespace w32 {

DWORD *LastErrorSlot()
{
   return (DWORD *) &Teb()->slots[13];
}

}  // namespace w32

extern "C" W32_EXPORT struct _TEB *m59_NtCurrentTeb(void)
{
   return (struct _TEB *) Teb();
}
