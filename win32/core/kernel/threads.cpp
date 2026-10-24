/*
 * threads.cpp:  Threads (CreateThread, _beginthread) and semaphores on pthreads.
 */

#include "core/kernel/kernel.h"

#include <process.h>

#include <algorithm>
#include <cerrno>
#include <climits>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>

namespace w32 {

namespace {

std::atomic<DWORD> next_thread_id{0x104};
thread_local DWORD this_thread_id = 0;

class Thread : public Waitable
{
public:
   Thread() : Waitable(KernelKind::Thread) {}
   bool Signaled() const override { return done; }

   LPTHREAD_START_ROUTINE start = nullptr;   // CreateThread's
   void (*crt_start)(void *) = nullptr;      // _beginthread's
   void *param = nullptr;
   DWORD id = 0;
   HANDLE handle = nullptr;
   bool close_on_exit = false;   // _beginthread: the thread owns its handle

   // Under WaitLock().
   bool done = false;
   DWORD exit_code = STILL_ACTIVE;
};

void *ThreadMain(void *arg)
{
   auto *thread = static_cast<Thread *>(arg);
   this_thread_id = thread->id;
   DWORD code = 0;
   if (thread->start != nullptr)
      code = thread->start(thread->param);
   else
      thread->crt_start(thread->param);
   {
      std::lock_guard<std::mutex> lock(WaitLock());
      // After TerminateThread the thread already counts as ended, with its code.
      if (!thread->done)
      {
         thread->done = true;
         thread->exit_code = code;
      }
   }
   WakeWaiters();
   if (thread->close_on_exit)
      CloseHandle(thread->handle);
   thread->Release();
   return nullptr;
}

// Starts thread (with its handle set); false with errno on failure.
bool Start(Thread *thread, SIZE_T stack_size)
{
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
   if (stack_size != 0)
   {
      long page = sysconf(_SC_PAGESIZE);
      size_t size = std::max<size_t>(stack_size, PTHREAD_STACK_MIN);
      size = (size + page - 1) / page * page;
      pthread_attr_setstacksize(&attr, size);
   }
   thread->AddRef();   // the running thread's
   pthread_t id;
   int error = pthread_create(&id, &attr, ThreadMain, thread);
   pthread_attr_destroy(&attr);
   if (error != 0)
   {
      thread->Release();
      errno = error;
      return false;
   }
   return true;
}

class Semaphore : public Waitable
{
public:
   Semaphore(LONG count, LONG max) : Waitable(KernelKind::Semaphore), count(count), max(max) {}
   bool Signaled() const override { return count > 0; }
   void Acquire() override { count--; }

   LONG count, max;   // under WaitLock()
};

}  // namespace

}  // namespace w32

using namespace w32;

extern "C" {

HANDLE WINAPI CreateThread(LPSECURITY_ATTRIBUTES, SIZE_T stack_size, LPTHREAD_START_ROUTINE start,
                           LPVOID param, DWORD flags, LPDWORD thread_id)
{
   // CREATE_SUSPENDED is not supported (nothing here resumes threads); the
   // thread starts at once.
   (void) flags;
   if (start == nullptr)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return nullptr;
   }
   auto *thread = new Thread;
   thread->start = start;
   thread->param = param;
   thread->id = next_thread_id.fetch_add(4);
   thread->handle = AddKernelObject(thread);
   HANDLE handle = thread->handle;
   if (!Start(thread, stack_size))
   {
      SetLastErrorFromErrno();
      CloseHandle(handle);
      return nullptr;
   }
   if (thread_id != nullptr)
      *thread_id = thread->id;
   return handle;
}

// pthread_cancel can't end a thread safely (no unwinding through C frames on
// glibc, none at all on macOS), so the thread is abandoned instead: it counts as
// ended with exit_code from now on (waits on it return, GetExitCodeThread would
// report the code), but keeps running until its function returns on its own.
// The client only terminates its download thread after it stopped responding
// to an abort flag.
BOOL WINAPI TerminateThread(HANDLE h, DWORD exit_code)
{
   KernelRef<Thread> thread(RefKernelObject(h, KernelKind::Thread));
   if (!thread)
      return FALSE;
   {
      std::lock_guard<std::mutex> lock(WaitLock());
      if (!thread->done)
      {
         thread->done = true;
         thread->exit_code = exit_code;
      }
   }
   WakeWaiters();
   return TRUE;
}

DWORD WINAPI GetCurrentThreadId(void)
{
   if (this_thread_id == 0)
      this_thread_id = next_thread_id.fetch_add(4);
   return this_thread_id;
}

VOID WINAPI Sleep(DWORD ms)
{
   if (ms == 0)
   {
      sched_yield();
      return;
   }
   if (ms == INFINITE)
      for (;;)
         pause();
   timespec t = {(time_t) (ms / 1000), (long) (ms % 1000) * 1000000};
   while (nanosleep(&t, &t) != 0 && errno == EINTR)
      ;
}

HANDLE WINAPI CreateSemaphoreA(LPSECURITY_ATTRIBUTES, LONG initial, LONG max, LPCSTR name)
{
   // Named semaphores would be shared between processes; the client names none,
   // and a name here makes an unshared semaphore.
   (void) name;
   if (max <= 0 || initial < 0 || initial > max)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return nullptr;
   }
   return AddKernelObject(new Semaphore(initial, max));
}

BOOL WINAPI ReleaseSemaphore(HANDLE h, LONG release, LPLONG previous)
{
   KernelRef<Semaphore> semaphore(RefKernelObject(h, KernelKind::Semaphore));
   if (!semaphore)
      return FALSE;
   if (release <= 0)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
   }
   {
      std::lock_guard<std::mutex> lock(WaitLock());
      if (release > semaphore->max - semaphore->count)
      {
         SetLastError(ERROR_TOO_MANY_POSTS);
         return FALSE;
      }
      if (previous != nullptr)
         *previous = semaphore->count;
      semaphore->count += release;
   }
   WakeWaiters();
   return TRUE;
}

// MSVC's: the thread closes its handle when it ends, so the value returned may
// already be stale (calls on it then fail quietly).
W32_EXPORT uintptr_t _beginthread(void (*start)(void *), unsigned stack_size, void *arg)
{
   if (start == nullptr)
   {
      errno = EINVAL;
      return (uintptr_t) -1;
   }
   auto *thread = new Thread;
   thread->crt_start = start;
   thread->param = arg;
   thread->id = next_thread_id.fetch_add(4);
   thread->close_on_exit = true;
   thread->handle = AddKernelObject(thread);
   HANDLE handle = thread->handle;
   if (!Start(thread, stack_size))
   {
      int error = errno;
      CloseHandle(handle);
      errno = error;
      return (uintptr_t) -1;
   }
   return (uintptr_t) handle;
}

}  // extern "C"
