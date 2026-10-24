/*
 * kernel_threads.c:  Threads, waits, semaphores, handles.
 */

#include <windows.h>
#include <process.h>
#include <string.h>

#include "kernel_check.h"

static volatile LONG counter = 0;
static volatile DWORD seen_id = 0;
static volatile int stop = 0;

static DWORD WINAPI Worker(LPVOID param)
{
   seen_id = GetCurrentThreadId();
   Sleep(50);
   counter += (LONG) (INT_PTR) param;
   return 42;
}

static HANDLE sem;

static DWORD WINAPI SemWaiter(LPVOID param)
{
   (void) param;
   WaitForSingleObject(sem, INFINITE);
   counter = 100;
   return 0;
}

static DWORD WINAPI Spinner(LPVOID param)
{
   (void) param;
   while (!stop)
      Sleep(5);
   return 0;
}

static volatile int crt_ran = 0;
static void __cdecl CrtThread(void *arg)
{
   Sleep(30);
   crt_ran = (int) (INT_PTR) arg;
}

int main(void)
{
   DWORD id = 0;
   DWORD main_id = GetCurrentThreadId();
   CHECK(main_id != 0);
   CHECK_INT(GetCurrentThreadId(), main_id);

   // CreateThread, wait with timeouts.
   HANDLE t = CreateThread(NULL, 0, Worker, (LPVOID) 5, 0, &id);
   CHECK(t != NULL);
   CHECK(id != 0 && id != main_id);
   CHECK_INT(WaitForSingleObject(t, 0), WAIT_TIMEOUT);
   CHECK_INT(WaitForSingleObject(t, 2000), WAIT_OBJECT_0);
   CHECK_INT(seen_id, id);
   CHECK_INT(counter, 5);
   // A finished thread stays signaled.
   CHECK_INT(WaitForSingleObject(t, 0), WAIT_OBJECT_0);
   CHECK(CloseHandle(t));
   SetLastError(0);
   CHECK(!CloseHandle(t));
   CHECK_INT(GetLastError(), ERROR_INVALID_HANDLE);
   CHECK_INT(WaitForSingleObject(t, 0), WAIT_FAILED);
   CHECK(!CloseHandle(NULL));
   CHECK(!CloseHandle(INVALID_HANDLE_VALUE));
   CHECK(CreateThread(NULL, 0, NULL, NULL, 0, NULL) == NULL);

   // Stack size.
   t = CreateThread(NULL, 256 * 1024, Worker, (LPVOID) 1, 0, NULL);
   CHECK(t != NULL);
   CHECK_INT(WaitForSingleObject(t, INFINITE), WAIT_OBJECT_0);
   CloseHandle(t);
   CHECK_INT(counter, 6);

   // Semaphores.
   CHECK(CreateSemaphoreA(NULL, 2, 1, NULL) == NULL);
   CHECK_INT(GetLastError(), ERROR_INVALID_PARAMETER);
   CHECK(CreateSemaphoreA(NULL, 0, 0, NULL) == NULL);
   CHECK(CreateSemaphoreA(NULL, -1, 1, NULL) == NULL);
   sem = CreateSemaphoreA(NULL, 1, 1, NULL);   // as transfer.c
   CHECK(sem != NULL);
   CHECK_INT(WaitForSingleObject(sem, 0), WAIT_OBJECT_0);
   CHECK_INT(WaitForSingleObject(sem, 0), WAIT_TIMEOUT);
   DWORD before = GetTickCount();
   CHECK_INT(WaitForSingleObject(sem, 100), WAIT_TIMEOUT);
   DWORD waited = GetTickCount() - before;
   CHECK(waited >= 95 && waited < 1000);
   LONG prev = -1;
   CHECK(ReleaseSemaphore(sem, 1, &prev));
   CHECK_INT(prev, 0);
   CHECK(!ReleaseSemaphore(sem, 1, &prev));
   CHECK_INT(GetLastError(), ERROR_TOO_MANY_POSTS);
   CHECK(!ReleaseSemaphore(sem, 0, NULL));
   CHECK_INT(GetLastError(), ERROR_INVALID_PARAMETER);
   CHECK_INT(WaitForSingleObject(sem, 0), WAIT_OBJECT_0);
   // A waiter on another thread wakes on release.
   counter = 0;
   t = CreateThread(NULL, 0, SemWaiter, NULL, 0, NULL);
   Sleep(50);
   CHECK_INT(counter, 0);
   CHECK(ReleaseSemaphore(sem, 1, NULL));
   CHECK_INT(WaitForSingleObject(t, 2000), WAIT_OBJECT_0);
   CHECK_INT(counter, 100);
   CloseHandle(t);
   HANDLE sem3 = CreateSemaphoreA(NULL, 0, 3, NULL);
   CHECK(ReleaseSemaphore(sem3, 2, &prev));
   CHECK_INT(prev, 0);
   CHECK(!ReleaseSemaphore(sem3, 2, &prev));
   CHECK(ReleaseSemaphore(sem3, 1, &prev));
   CHECK_INT(prev, 2);
   CHECK(CloseHandle(sem3));
   // Semaphore handles aren't threads.
   CHECK(!TerminateThread(sem, 0));
   CHECK(CloseHandle(sem));
   CHECK(!ReleaseSemaphore(sem, 1, NULL));
   CHECK_INT(GetLastError(), ERROR_INVALID_HANDLE);

   // TerminateThread: the thread counts as ended at once.
   stop = 0;
   t = CreateThread(NULL, 0, Spinner, NULL, 0, NULL);
   CHECK_INT(WaitForSingleObject(t, 50), WAIT_TIMEOUT);
   CHECK(TerminateThread(t, 7));
   CHECK_INT(WaitForSingleObject(t, 0), WAIT_OBJECT_0);
   CloseHandle(t);
   stop = 1;   // let it end for real

   // _beginthread: the handle closes itself when the thread ends.
   uintptr_t bt = _beginthread(CrtThread, 0, (void *) 9);
   CHECK(bt != (uintptr_t) -1);
   HANDLE bh = (HANDLE) bt;
   CHECK_INT(WaitForSingleObject(bh, 2000), WAIT_OBJECT_0);
   Sleep(50);
   CHECK_INT(crt_ran, 9);
   CHECK_INT(WaitForSingleObject(bh, 0), WAIT_FAILED);
   CHECK(!CloseHandle(bh));

   // Sleep.
   before = GetTickCount();
   Sleep(60);
   waited = GetTickCount() - before;
   CHECK(waited >= 59 && waited < 1000);
   Sleep(0);

   // Many threads at once.
   HANDLE many[32];
   counter = 0;
   for (int i = 0; i < 32; i++)
      many[i] = CreateThread(NULL, 0, Worker, (LPVOID) 0, 0, NULL);
   for (int i = 0; i < 32; i++)
   {
      CHECK_INT(WaitForSingleObject(many[i], 5000), WAIT_OBJECT_0);
      CHECK(CloseHandle(many[i]));
   }
   Sleep(20);
   return CheckReport("kernel_threads");
}
