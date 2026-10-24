/*
 * cc_kernel.cpp:  Kernel functions comctl32 needs: critical sections, events,
 *   RegOpenKey.  Weak, for the kernel's owner to replace.
 */

#include "core/kernel/kernel.h"

#include <atomic>
#include <cstring>
#include <mutex>

using namespace w32;

#define CC_WEAK __attribute__((weak))

namespace {

// The section's lock, made on first use: sections may be zero-initialized
// statics never passed to InitializeCriticalSection.  Kept in LockSemaphore.
std::recursive_mutex *Lock(CRITICAL_SECTION *cs)
{
   auto *slot = reinterpret_cast<std::atomic<std::recursive_mutex *> *>(&cs->LockSemaphore);
   std::recursive_mutex *m = slot->load(std::memory_order_acquire);
   if (m != nullptr)
      return m;
   auto *made = new std::recursive_mutex;
   if (slot->compare_exchange_strong(m, made, std::memory_order_acq_rel))
      return made;
   delete made;
   return m;
}

class Event : public Waitable
{
public:
   Event(bool manual, bool signaled) : Waitable(KernelKind::Event), manual_(manual), signaled_(signaled) {}
   bool Signaled() const override { return signaled_; }
   void Acquire() override
   {
      if (!manual_)
         signaled_ = false;
   }
   void Set(bool signaled) { signaled_ = signaled; }

private:
   bool manual_;
   bool signaled_;
};

}  // namespace

extern "C" {

CC_WEAK void WINAPI InitializeCriticalSection(CRITICAL_SECTION *cs)
{
   memset(cs, 0, sizeof(*cs));
   cs->LockCount = -1;
   // Debug info, as Windows allocates it: Wine's code names its sections there.
   auto *debug = new RTL_CRITICAL_SECTION_DEBUG();
   debug->CriticalSection = cs;
   debug->Spare[1] = 0x4D3539;   // made here
   cs->DebugInfo = debug;
   Lock(cs);
}

CC_WEAK BOOL WINAPI InitializeCriticalSectionEx(CRITICAL_SECTION *cs, DWORD, DWORD)
{
   InitializeCriticalSection(cs);
   return TRUE;
}

CC_WEAK BOOL WINAPI InitializeCriticalSectionAndSpinCount(CRITICAL_SECTION *cs, DWORD)
{
   InitializeCriticalSection(cs);
   return TRUE;
}

CC_WEAK void WINAPI DeleteCriticalSection(CRITICAL_SECTION *cs)
{
   delete reinterpret_cast<std::recursive_mutex *>(cs->LockSemaphore);
   cs->LockSemaphore = nullptr;
   // Static sections' debug info isn't ours (Wine's points at a static).
   if (cs->DebugInfo && cs->DebugInfo->CriticalSection == cs && cs->DebugInfo->Spare[1] == 0x4D3539)
      delete cs->DebugInfo;
   cs->DebugInfo = nullptr;
}

CC_WEAK void WINAPI EnterCriticalSection(CRITICAL_SECTION *cs)
{
   Lock(cs)->lock();
   cs->RecursionCount++;
   cs->OwningThread = (HANDLE) (ULONG_PTR) GetCurrentThreadId();
}

CC_WEAK BOOL WINAPI TryEnterCriticalSection(CRITICAL_SECTION *cs)
{
   if (!Lock(cs)->try_lock())
      return FALSE;
   cs->RecursionCount++;
   cs->OwningThread = (HANDLE) (ULONG_PTR) GetCurrentThreadId();
   return TRUE;
}

CC_WEAK void WINAPI LeaveCriticalSection(CRITICAL_SECTION *cs)
{
   if (--cs->RecursionCount == 0)
      cs->OwningThread = nullptr;
   Lock(cs)->unlock();
}

CC_WEAK HANDLE WINAPI CreateEventA(LPSECURITY_ATTRIBUTES, BOOL manual, BOOL initial, LPCSTR)
{
   // Named events would be shared between processes; nothing here names one.
   return AddKernelObject(new Event(manual != FALSE, initial != FALSE));
}

CC_WEAK HANDLE WINAPI CreateEventW(LPSECURITY_ATTRIBUTES sa, BOOL manual, BOOL initial, LPCWSTR)
{
   return CreateEventA(sa, manual, initial, nullptr);
}

static BOOL SetEventState(HANDLE h, bool signaled)
{
   KernelRef<Event> event(RefKernelObject(h, KernelKind::Event));
   if (!event)
      return FALSE;
   {
      std::lock_guard<std::mutex> lock(WaitLock());
      event->Set(signaled);
   }
   if (signaled)
      WakeWaiters();
   return TRUE;
}

CC_WEAK BOOL WINAPI SetEvent(HANDLE h)
{
   return SetEventState(h, true);
}

CC_WEAK BOOL WINAPI ResetEvent(HANDLE h)
{
   return SetEventState(h, false);
}

CC_WEAK LSTATUS WINAPI RegOpenKeyA(HKEY key, LPCSTR subkey, PHKEY result)
{
   return RegOpenKeyExA(key, subkey, 0, KEY_ALL_ACCESS, result);
}

CC_WEAK LSTATUS WINAPI RegOpenKeyW(HKEY key, LPCWSTR subkey, PHKEY result)
{
   return RegOpenKeyExW(key, subkey, 0, KEY_ALL_ACCESS, result);
}

}  // extern "C"
