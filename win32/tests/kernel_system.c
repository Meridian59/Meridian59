/*
 * kernel_system.c:  Time, system information, version, last error,
 *   FormatMessage, MulDiv, console, MSVC globals.
 */

#include <windows.h>
#include <mmsystem.h>
#include <new.h>
#include <string.h>

#include "kernel_check.h"

static DWORD WINAPI ErrorThread(LPVOID param)
{
   SetLastError(1234);
   return GetLastError() == 1234 ? 0 : 1;
}

static const char *Format(DWORD flags, const char *source, DWORD id, ...)
{
   static char buf[512];
   va_list args;
   va_start(args, id);
   memset(buf, 0, sizeof(buf));
   DWORD n = FormatMessageA(flags, source, id, 0, buf, sizeof(buf), &args);
   va_end(args);
   if (n == 0)
      sprintf(buf, "!%lu", (unsigned long) GetLastError());
   else
      CHECK_INT(n, strlen(buf));
   return buf;
}

static int handler_calls = 0;
static int NewHandler(size_t size) { handler_calls++; return 0; }

int main(void)
{
   // Ticks.
   DWORD t0 = GetTickCount(), m0 = timeGetTime();
   Sleep(30);
   DWORD t1 = GetTickCount(), m1 = timeGetTime();
   CHECK(t1 - t0 >= 29 && t1 - t0 < 1000);
   CHECK(m1 - m0 >= 29 && m1 - m0 < 1000);
   CHECK_INT(timeBeginPeriod(1), TIMERR_NOERROR);
   CHECK_INT(timeEndPeriod(1), TIMERR_NOERROR);
   CHECK_INT(timeBeginPeriod(0), TIMERR_NOCANDO);

   // System information.
   SYSTEM_INFO si;
   GetSystemInfo(&si);
   CHECK(si.dwNumberOfProcessors >= 1);
   CHECK(si.dwPageSize >= 4096);
   CHECK_INT(si.dwAllocationGranularity, 65536);
   CHECK(si.dwActiveProcessorMask != 0);
   printf("processors %lu, type %lu, arch %u\n", (unsigned long) si.dwNumberOfProcessors,
          (unsigned long) si.dwProcessorType, si.wProcessorArchitecture);

   MEMORYSTATUS ms;
   ms.dwLength = sizeof(ms);
   GlobalMemoryStatus(&ms);
   CHECK(ms.dwTotalPhys > 0 && ms.dwAvailPhys > 0 && ms.dwAvailPhys <= ms.dwTotalPhys);
   CHECK(ms.dwMemoryLoad <= 100);
   MEMORYSTATUSEX mx;
   mx.dwLength = sizeof(mx);
   CHECK(GlobalMemoryStatusEx(&mx));
   CHECK(mx.ullTotalPhys == ms.dwTotalPhys);
   CHECK(mx.ullAvailPhys > 0 && mx.ullAvailPhys <= mx.ullTotalPhys);
   CHECK(mx.ullTotalPageFile >= mx.ullTotalPhys);
   CHECK(mx.ullTotalVirtual > mx.ullTotalPhys);
   printf("memory %llu MB, available %llu MB, load %lu%%\n", (unsigned long long) (mx.ullTotalPhys >> 20),
          (unsigned long long) (mx.ullAvailPhys >> 20), (unsigned long) mx.dwMemoryLoad);
   mx.dwLength = 0;
   CHECK(!GlobalMemoryStatusEx(&mx));
   CHECK_INT(GetLastError(), ERROR_INVALID_PARAMETER);

   // Version.
   OSVERSIONINFOA v;
   memset(&v, 0x55, sizeof(v));
   v.dwOSVersionInfoSize = sizeof(v);
   CHECK(GetVersionExA(&v));
   CHECK_INT(v.dwMajorVersion, 10);
   CHECK_INT(v.dwMinorVersion, 0);
   CHECK_INT(v.dwBuildNumber, 19045);
   CHECK_INT(v.dwPlatformId, VER_PLATFORM_WIN32_NT);
   CHECK_STR(v.szCSDVersion, "");
   OSVERSIONINFOEXA vx;
   vx.dwOSVersionInfoSize = sizeof(vx);
   CHECK(GetVersionExA((OSVERSIONINFOA *) &vx));
   CHECK_INT(vx.wProductType, VER_NT_WORKSTATION);
   v.dwOSVersionInfoSize = 12;
   CHECK(!GetVersionExA(&v));
   CHECK_INT(GetLastError(), ERROR_INSUFFICIENT_BUFFER);

   // Last error is per thread.
   SetLastError(77);
   HANDLE t = CreateThread(NULL, 0, ErrorThread, NULL, 0, NULL);
   WaitForSingleObject(t, INFINITE);
   CloseHandle(t);
   CHECK_INT(GetLastError(), 77);

   CHECK_INT(SetErrorMode(SEM_NOOPENFILEERRORBOX), 0);
   CHECK_INT(SetErrorMode(0), SEM_NOOPENFILEERRORBOX);

   // FormatMessage.
   CHECK_STR(Format(FORMAT_MESSAGE_FROM_SYSTEM, NULL, ERROR_FILE_NOT_FOUND),
             "The system cannot find the file specified.\r\n");
   CHECK_STR(Format(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, ERROR_ACCESS_DENIED),
             "Access is denied. ");
   CHECK_STR(Format(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ERROR_MR_MID_NOT_FOUND),
             "The system cannot find message text for message number 0x%1 in the message file for %2.\r\n");
   CHECK_STR(Format(FORMAT_MESSAGE_FROM_SYSTEM, NULL, 999999), "!317");
   CHECK_STR(Format(FORMAT_MESSAGE_FROM_STRING, "%1 has %2!d! items%n", 0, "list", 3),
             "list has 3 items\r\n");
   CHECK_STR(Format(FORMAT_MESSAGE_FROM_STRING, "%2 %1 %2", 0, "a", "b"), "b a b");
   CHECK_STR(Format(FORMAT_MESSAGE_FROM_STRING, "[%1!5s!] [%2!-4d!] [%3!x!]", 0, "ab", 7, 255),
             "[   ab] [7   ] [ff]");
   CHECK_STR(Format(FORMAT_MESSAGE_FROM_STRING, "100%% done%0 ignored", 0), "100% done");
   CHECK_STR(Format(FORMAT_MESSAGE_FROM_STRING, "a%tb%. %!", 0), "a\tb. !");
   CHECK_STR(Format(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_IGNORE_INSERTS, "keep %1 and %2!d!", 0),
             "keep %1 and %2!d!");
   {
      DWORD_PTR array[2] = {(DWORD_PTR) "x", 5};
      char out[64];
      CHECK_INT(FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, "%1=%2!u!", 0, 0,
                               out, sizeof(out), (va_list *) array), 3);
      CHECK_STR(out, "x=5");
      CHECK_INT(FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, "%1=%2!u!", 0, 0,
                               out, 3, (va_list *) array), 0);
      CHECK_INT(GetLastError(), ERROR_INSUFFICIENT_BUFFER);
   }
   {
      // As util.c's GetLastErrorStr.
      char *error_str = (char *) "unknown";
      SetLastError(ERROR_PATH_NOT_FOUND);
      DWORD n = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                               MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPSTR) &error_str, 0, NULL);
      CHECK(n > 0);
      CHECK_STR(error_str, "The system cannot find the path specified.\r\n");
      CHECK(LocalFree(error_str) == NULL);
   }

   // MulDiv rounds half away from zero.
   CHECK_INT(MulDiv(10, 3, 4), 8);       // 7.5
   CHECK_INT(MulDiv(-10, 3, 4), -8);
   CHECK_INT(MulDiv(10, -3, 4), -8);
   CHECK_INT(MulDiv(10, 3, -4), -8);
   CHECK_INT(MulDiv(-10, -3, -4), -8);
   CHECK_INT(MulDiv(7, 1, 3), 2);
   CHECK_INT(MulDiv(8, 1, 3), 3);
   CHECK_INT(MulDiv(-8, 1, 3), -3);
   CHECK_INT(MulDiv(1, 1, 0), -1);
   CHECK_INT(MulDiv(0x7fffffff, 2, 1), -1);
   CHECK_INT(MulDiv(0x7fffffff, 0x7fffffff, 0x7fffffff), 0x7fffffff);
   CHECK_INT(MulDiv(-12, 96, 72), -16);   // font height scaling, as map.c

   // Console.
   CHECK(AllocConsole());
   CHECK(!AllocConsole());
   HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
   DWORD written;
   fflush(stdout);
   CHECK(WriteFile(out, "console line\n", 13, &written, NULL));

   // MSVC globals and new handler.
   CHECK_INT(__argc, 0);
   CHECK(__argv == NULL);
   CHECK(_set_new_handler(NewHandler) == NULL);
   CHECK(_set_new_handler(NULL) == NewHandler);

   return CheckReport("kernel_system");
}
