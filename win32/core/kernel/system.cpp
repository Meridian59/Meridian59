/*
 * system.cpp:  Time, system and memory information, the version, last error,
 *   FormatMessage, MulDiv, LocalAlloc, the console, and the MSVC runtime globals.
 */

#include "core/kernel/kernel.h"

#include <new.h>   // crt/: _set_new_handler

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <unistd.h>

#ifdef __APPLE__
#include <mach/mach.h>
#include <sys/sysctl.h>
#else
#include <fstream>
#endif

// MSVC's command line, which the startup code fills in before WinMain.
extern "C" {
W32_EXPORT int __argc = 0;
W32_EXPORT char **__argv = nullptr;
}

namespace w32 {

// The thread's last error, where Wine's sources read it (core/wine_teb.cpp).
DWORD *LastErrorSlot();

namespace {

std::atomic<UINT> error_mode{0};
std::atomic<bool> console_allocated{false};

uint64_t MonotonicMs()
{
   timespec t;
   clock_gettime(CLOCK_MONOTONIC, &t);
   return (uint64_t) t.tv_sec * 1000 + (uint64_t) t.tv_nsec / 1000000;
}

struct Memory
{
   uint64_t total_phys = 0, avail_phys = 0, total_swap = 0, avail_swap = 0;
};

Memory QueryMemory()
{
   Memory m;
   uint64_t page = (uint64_t) sysconf(_SC_PAGESIZE);
#ifdef __APPLE__
   uint64_t memsize = 0;
   size_t len = sizeof(memsize);
   if (sysctlbyname("hw.memsize", &memsize, &len, nullptr, 0) == 0)
      m.total_phys = memsize;
   vm_statistics64_data_t vm;
   mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
   if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t) &vm, &count) == KERN_SUCCESS)
   {
      vm_size_t vm_page = 0;
      host_page_size(mach_host_self(), &vm_page);
      m.avail_phys = ((uint64_t) vm.free_count + vm.inactive_count + vm.purgeable_count) * vm_page;
   }
   xsw_usage swap;
   len = sizeof(swap);
   if (sysctlbyname("vm.swapusage", &swap, &len, nullptr, 0) == 0)
   {
      m.total_swap = swap.xsu_total;
      m.avail_swap = swap.xsu_avail;
   }
#else
   // MemAvailable is what can be had without swapping; the kB values.
   std::ifstream in("/proc/meminfo");
   std::string key;
   uint64_t value;
   bool have_available = false;
   uint64_t free = 0, cached = 0, buffers = 0;
   while (in >> key >> value)
   {
      if (key == "MemTotal:")
         m.total_phys = value * 1024;
      else if (key == "MemAvailable:")
         m.avail_phys = value * 1024, have_available = true;
      else if (key == "MemFree:")
         free = value * 1024;
      else if (key == "Cached:")
         cached = value * 1024;
      else if (key == "Buffers:")
         buffers = value * 1024;
      else if (key == "SwapTotal:")
         m.total_swap = value * 1024;
      else if (key == "SwapFree:")
         m.avail_swap = value * 1024;
      in.ignore(256, '\n');
   }
   if (!have_available)
      m.avail_phys = free + cached + buffers;
#endif
   if (m.total_phys == 0)
      m.total_phys = (uint64_t) sysconf(_SC_PHYS_PAGES) * page;
   if (m.avail_phys > m.total_phys)
      m.avail_phys = m.total_phys;
   return m;
}

// The user-mode address space of 64-bit Windows.
constexpr uint64_t kTotalVirtual = 0x7FFFFFFE0000ull;

// Windows' text for the errors the layer sets (FORMAT_MESSAGE_FROM_SYSTEM).
struct SystemMessage
{
   DWORD id;
   const char *text;
};

const SystemMessage system_messages[] = {
   {ERROR_SUCCESS, "The operation completed successfully.\r\n"},
   {ERROR_INVALID_FUNCTION, "Incorrect function.\r\n"},
   {ERROR_FILE_NOT_FOUND, "The system cannot find the file specified.\r\n"},
   {ERROR_PATH_NOT_FOUND, "The system cannot find the path specified.\r\n"},
   {ERROR_TOO_MANY_OPEN_FILES, "The system cannot open the file.\r\n"},
   {ERROR_ACCESS_DENIED, "Access is denied.\r\n"},
   {ERROR_INVALID_HANDLE, "The handle is invalid.\r\n"},
   {ERROR_NOT_ENOUGH_MEMORY, "Not enough memory resources are available to process this command.\r\n"},
   {ERROR_OUTOFMEMORY, "Not enough memory resources are available to complete this operation.\r\n"},
   {ERROR_NOT_SAME_DEVICE, "The system cannot move the file to a different disk drive.\r\n"},
   {ERROR_NO_MORE_FILES, "There are no more files.\r\n"},
   {ERROR_BAD_UNIT, "The system cannot find the device specified.\r\n"},
   {ERROR_GEN_FAILURE, "A device attached to the system is not functioning.\r\n"},
   {ERROR_SHARING_VIOLATION, "The process cannot access the file because it is being used by another process.\r\n"},
   {ERROR_FILE_EXISTS, "The file exists.\r\n"},
   {ERROR_INVALID_PARAMETER, "The parameter is incorrect.\r\n"},
   {ERROR_DISK_FULL, "There is not enough space on the disk.\r\n"},
   {ERROR_INSUFFICIENT_BUFFER, "The data area passed to a system call is too small.\r\n"},
   {ERROR_DIR_NOT_EMPTY, "The directory is not empty.\r\n"},
   {ERROR_ALREADY_EXISTS, "Cannot create a file when that file already exists.\r\n"},
   {ERROR_FILENAME_EXCED_RANGE, "The filename or extension is too long.\r\n"},
   {ERROR_FILE_TOO_LARGE, "The file size exceeds the limit allowed and cannot be saved.\r\n"},
   {ERROR_DIRECTORY, "The directory name is invalid.\r\n"},
   {ERROR_TOO_MANY_POSTS, "Too many posts were made to a semaphore.\r\n"},
   {ERROR_MR_MID_NOT_FOUND, "The system cannot find message text for message number 0x%1 in the message file for %2.\r\n"},
   {ERROR_INVALID_ADDRESS, "Attempt to access invalid address.\r\n"},
   {ERROR_FILE_INVALID, "The volume for a file has been externally altered so that the opened file is no longer valid.\r\n"},
   {ERROR_MAPPED_ALIGNMENT, "The base address or the file offset specified does not have the proper alignment.\r\n"},
   {ERROR_CANT_RESOLVE_FILENAME, "The name of the file cannot be resolved by the system.\r\n"},
   {WSAEINTR, "A blocking operation was interrupted by a call to WSACancelBlockingCall.\r\n"},
   {WSAEBADF, "The file handle supplied is not valid.\r\n"},
   {WSAEACCES, "An attempt was made to access a socket in a way forbidden by its access permissions.\r\n"},
   {WSAEFAULT, "The system detected an invalid pointer address in attempting to use a pointer argument in a call.\r\n"},
   {WSAEINVAL, "An invalid argument was supplied.\r\n"},
   {WSAEMFILE, "Too many open sockets.\r\n"},
   {WSAEWOULDBLOCK, "A non-blocking socket operation could not be completed immediately.\r\n"},
   {WSAEINPROGRESS, "A blocking operation is currently executing.\r\n"},
   {WSAEALREADY, "An operation was attempted on a non-blocking socket that already had an operation in progress.\r\n"},
   {WSAENOTSOCK, "An operation was attempted on something that is not a socket.\r\n"},
   {WSAEADDRINUSE, "Only one usage of each socket address (protocol/network address/port) is normally permitted.\r\n"},
   {WSAEADDRNOTAVAIL, "The requested address is not valid in its context.\r\n"},
   {WSAENETDOWN, "A socket operation encountered a dead network.\r\n"},
   {WSAENETUNREACH, "A socket operation was attempted to an unreachable network.\r\n"},
   {WSAECONNABORTED, "An established connection was aborted by the software in your host machine.\r\n"},
   {WSAECONNRESET, "An existing connection was forcibly closed by the remote host.\r\n"},
   {WSAENOBUFS, "An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full.\r\n"},
   {WSAEISCONN, "A connect request was made on an already connected socket.\r\n"},
   {WSAENOTCONN, "A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using a sendto call) no address was supplied.\r\n"},
   {WSAESHUTDOWN, "A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call.\r\n"},
   {WSAETIMEDOUT, "A connection attempt failed because the connected party did not properly respond after a period of time, or established connection failed because connected host has failed to respond.\r\n"},
   {WSAECONNREFUSED, "No connection could be made because the target machine actively refused it.\r\n"},
   {WSAEHOSTDOWN, "A socket operation failed because the destination host was down.\r\n"},
   {WSAEHOSTUNREACH, "A socket operation was attempted to an unreachable host.\r\n"},
   {WSASYSNOTREADY, "WSAStartup cannot function at this time because the underlying system it uses to provide network services is currently unavailable.\r\n"},
   {WSANOTINITIALISED, "Either the application has not called WSAStartup, or WSAStartup failed.\r\n"},
   {WSAHOST_NOT_FOUND, "No such host is known.\r\n"},
   {WSATRY_AGAIN, "This is usually a temporary error during hostname resolution and means that the local server did not receive a response from an authoritative server.\r\n"},
   {WSANO_RECOVERY, "A non-recoverable error occurred during a database lookup.\r\n"},
   {WSANO_DATA, "The requested name is valid, but no data of the requested type was found.\r\n"},
};

const char *FindSystemMessage(DWORD id)
{
   for (const SystemMessage &m : system_messages)
      if (m.id == id)
         return m.text;
   return nullptr;
}

// The inserts' arguments, each read as pointer-sized, in order, as Windows
// reads a va_list (an int argument reads with its slot's upper bits).
class InsertArgs
{
public:
   InsertArgs(va_list *list, const DWORD_PTR *array) : list_(list), array_(array) {}
   bool Get(unsigned nr, DWORD_PTR *value)
   {
      if (nr == 0)
         return false;
      if (array_ != nullptr)
      {
         *value = array_[nr - 1];
         return true;
      }
      if (list_ == nullptr)
         return false;
      while (read_.size() < nr)
         read_.push_back(va_arg(*list_, DWORD_PTR));
      *value = read_[nr - 1];
      return true;
   }
private:
   va_list *list_;
   const DWORD_PTR *array_;
   std::basic_string<DWORD_PTR> read_;
};

// One insert: "%<nr>" with printf format fmt ("s" when none).
bool FormatInsert(std::string &out, unsigned nr, std::string fmt, InsertArgs &args)
{
   // MSVC's I64 / I32 size prefixes.
   for (size_t at; (at = fmt.find("I64")) != std::string::npos;)
      fmt.replace(at, 3, "ll");
   for (size_t at; (at = fmt.find("I32")) != std::string::npos;)
      fmt.erase(at, 3);
   char conv = fmt.empty() ? 's' : fmt.back();
   int stars = 0;
   for (char c : fmt)
      stars += c == '*';
   DWORD_PTR star_values[2] = {0, 0}, value;
   for (int i = 0; i < stars && i < 2; i++)
      if (!args.Get(nr + i, &star_values[i]))
         return false;
   if (!args.Get(nr + stars, &value))
      return false;
   std::string f = "%" + fmt;
   bool wide_int = fmt.find("ll") != std::string::npos || fmt.find('z') != std::string::npos ||
                   fmt.find('j') != std::string::npos;
   char buf[512];
   int a = (int) star_values[0], b = (int) star_values[1];
   int n;
#define W32_FORMAT(v) \
   (stars == 0 ? snprintf(buf, sizeof(buf), f.c_str(), v) \
    : stars == 1 ? snprintf(buf, sizeof(buf), f.c_str(), a, v) \
    : snprintf(buf, sizeof(buf), f.c_str(), a, b, v))
   switch (conv)
   {
   case 's':
   case 'S':
   {
      if (conv == 'S')
         f.back() = 's';
      const char *s = value != 0 ? (const char *) value : "(null)";
      if (stars == 0 && fmt.size() <= 1)
      {
         out += s;
         return true;
      }
      n = W32_FORMAT(s);
      break;
   }
   case 'c':
   case 'C':
      f.back() = 'c';
      n = W32_FORMAT((int) value);
      break;
   case 'p':
      n = W32_FORMAT((void *) value);
      break;
   case 'd': case 'i': case 'u': case 'x': case 'X': case 'o':
      if (wide_int)
         n = W32_FORMAT((long long) value);
      else
         n = W32_FORMAT((int) value);
      break;
   default:
      n = W32_FORMAT((void *) value);
      break;
   }
#undef W32_FORMAT
   if (n > 0)
      out.append(buf, std::min<size_t>(n, sizeof(buf) - 1));
   return true;
}

// template's escapes and inserts expanded.  max_width: FORMAT_MESSAGE_MAX_WIDTH_MASK's
// width; any nonzero width turns the template's own line breaks into spaces
// (lines aren't wrapped to it).
bool FormatTemplate(std::string &out, const char *t, bool ignore_inserts, DWORD max_width,
                    InsertArgs &args)
{
   for (const char *p = t; *p;)
   {
      if (max_width != 0 && (*p == '\r' || *p == '\n'))
      {
         if (p[0] == '\r' && p[1] == '\n')
            p++;
         p++;
         out += ' ';
         continue;
      }
      if (*p != '%')
      {
         out += *p++;
         continue;
      }
      p++;
      if (*p >= '1' && *p <= '9')
      {
         const char *start = p - 1;
         unsigned nr = *p++ - '0';
         if (*p >= '0' && *p <= '9')
            nr = nr * 10 + (*p++ - '0');
         std::string fmt = "s";
         if (*p == '!')
         {
            const char *end = strchr(p + 1, '!');
            if (end == nullptr)
               return false;
            fmt.assign(p + 1, end);
            p = end + 1;
         }
         if (ignore_inserts)
            out.append(start, p);
         else if (!FormatInsert(out, nr, fmt, args))
            return false;
         continue;
      }
      switch (*p)
      {
      case 0:   return true;
      case '0': return true;   // ends the message, without a line break
      case 'n': out += "\r\n"; break;
      case 'r': out += '\r'; break;
      case 't': out += '\t'; break;
      default:  out += *p; break;   // %% %. %! and "% "
      }
      p++;
   }
   return true;
}

}  // namespace

}  // namespace w32

using namespace w32;

extern "C" {

DWORD WINAPI GetLastError(void)
{
   return *LastErrorSlot();
}

VOID WINAPI SetLastError(DWORD error)
{
   *LastErrorSlot() = error;
}

UINT WINAPI SetErrorMode(UINT mode)
{
   return error_mode.exchange(mode);
}

DWORD WINAPI GetTickCount(void)
{
   return (DWORD) MonotonicMs();
}

DWORD WINAPI timeGetTime(void)
{
   return (DWORD) MonotonicMs();
}

// The host's timers are as fine as they get already.
MMRESULT WINAPI timeBeginPeriod(UINT period)
{
   return period == 0 ? TIMERR_NOCANDO : TIMERR_NOERROR;
}

MMRESULT WINAPI timeEndPeriod(UINT period)
{
   return period == 0 ? TIMERR_NOCANDO : TIMERR_NOERROR;
}

VOID WINAPI GetSystemInfo(LPSYSTEM_INFO info)
{
   memset(info, 0, sizeof(*info));
#if defined(__aarch64__) || defined(__arm64__)
   info->wProcessorArchitecture = PROCESSOR_ARCHITECTURE_ARM64;
   info->dwProcessorType = 0;
   info->wProcessorLevel = 8;
#else
   info->wProcessorArchitecture = PROCESSOR_ARCHITECTURE_AMD64;
   info->dwProcessorType = PROCESSOR_AMD_X8664;
   info->wProcessorLevel = 6;
#endif
   info->dwPageSize = (DWORD) sysconf(_SC_PAGESIZE);
   info->lpMinimumApplicationAddress = (LPVOID) 0x10000;
   info->lpMaximumApplicationAddress = (LPVOID) (kTotalVirtual + 0x10000 - 1);
   long cpus = sysconf(_SC_NPROCESSORS_ONLN);
   if (cpus < 1)
      cpus = 1;
   if (cpus > 64)
      cpus = 64;   // one processor group's worth, as Windows reports
   info->dwNumberOfProcessors = (DWORD) cpus;
   info->dwActiveProcessorMask = cpus == 64 ? ~(DWORD_PTR) 0 : ((DWORD_PTR) 1 << cpus) - 1;
   info->dwAllocationGranularity = 0x10000;
}

BOOL WINAPI GlobalMemoryStatusEx(LPMEMORYSTATUSEX status)
{
   if (status == nullptr || status->dwLength != sizeof(MEMORYSTATUSEX))
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
   }
   Memory m = QueryMemory();
   status->dwMemoryLoad = m.total_phys != 0
      ? (DWORD) ((m.total_phys - m.avail_phys) * 100 / m.total_phys) : 0;
   status->ullTotalPhys = m.total_phys;
   status->ullAvailPhys = m.avail_phys;
   // The commit limit: memory and swap.
   status->ullTotalPageFile = m.total_phys + m.total_swap;
   status->ullAvailPageFile = m.avail_phys + m.avail_swap;
   status->ullTotalVirtual = kTotalVirtual;
   status->ullAvailVirtual = kTotalVirtual;
   status->ullAvailExtendedVirtual = 0;
   return TRUE;
}

VOID WINAPI GlobalMemoryStatus(LPMEMORYSTATUS status)
{
   MEMORYSTATUSEX ex;
   ex.dwLength = sizeof(ex);
   GlobalMemoryStatusEx(&ex);
   status->dwLength = sizeof(MEMORYSTATUS);
   status->dwMemoryLoad = ex.dwMemoryLoad;
   status->dwTotalPhys = (SIZE_T) ex.ullTotalPhys;
   status->dwAvailPhys = (SIZE_T) ex.ullAvailPhys;
   status->dwTotalPageFile = (SIZE_T) ex.ullTotalPageFile;
   status->dwAvailPageFile = (SIZE_T) ex.ullAvailPageFile;
   status->dwTotalVirtual = (SIZE_T) ex.ullTotalVirtual;
   status->dwAvailVirtual = (SIZE_T) ex.ullAvailVirtual;
}

// Windows 10 22H2.
BOOL WINAPI GetVersionExA(OSVERSIONINFOA *info)
{
   if (info == nullptr || (info->dwOSVersionInfoSize != sizeof(OSVERSIONINFOA) &&
                           info->dwOSVersionInfoSize != sizeof(OSVERSIONINFOEXA)))
   {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return FALSE;
   }
   DWORD size = info->dwOSVersionInfoSize;
   memset((char *) info + sizeof(DWORD), 0, size - sizeof(DWORD));
   info->dwMajorVersion = 10;
   info->dwMinorVersion = 0;
   info->dwBuildNumber = 19045;
   info->dwPlatformId = VER_PLATFORM_WIN32_NT;
   if (size == sizeof(OSVERSIONINFOEXA))
   {
      auto *ex = (OSVERSIONINFOEXA *) info;
      ex->wSuiteMask = VER_SUITE_SINGLEUSERTS;
      ex->wProductType = VER_NT_WORKSTATION;
   }
   return TRUE;
}

// Rounds half away from zero; -1 for a zero divisor or a result out of range.
INT WINAPI MulDiv(INT a, INT b, INT divisor)
{
   if (divisor == 0)
      return -1;
   int64_t n = a, d = divisor;
   if (d < 0)
   {
      n = -n;
      d = -d;
   }
   int64_t product = n * b;
   bool positive = (n < 0 && b < 0) || (n >= 0 && b >= 0);
   int64_t ret = positive ? (product + d / 2) / d : (product - d / 2) / d;
   if (ret > 2147483647 || ret < -2147483647)
      return -1;
   return (INT) ret;
}

HLOCAL WINAPI LocalAlloc(UINT flags, SIZE_T size)
{
   // Fixed memory only: LMEM_MOVEABLE blocks are fixed too (their handle is the
   // pointer, which LocalLock would return).
   void *p = (flags & LMEM_ZEROINIT) ? calloc(1, size ? size : 1) : malloc(size ? size : 1);
   if (p == nullptr)
      SetLastError(ERROR_NOT_ENOUGH_MEMORY);
   return p;
}

HLOCAL WINAPI LocalFree(HLOCAL mem)
{
   free(mem);
   return nullptr;
}

DWORD WINAPI FormatMessageA(DWORD flags, LPCVOID source, DWORD id, DWORD lang, LPSTR buffer,
                            DWORD size, va_list *args)
{
   (void) lang;   // English only
   const char *t = nullptr;
   if (flags & FORMAT_MESSAGE_FROM_STRING)
      t = static_cast<const char *>(source);
   else if (flags & FORMAT_MESSAGE_FROM_SYSTEM)
      t = FindSystemMessage(id);
   // FORMAT_MESSAGE_FROM_HMODULE (message table resources) is not supported.
   if (t == nullptr)
   {
      SetLastError((flags & FORMAT_MESSAGE_FROM_STRING) ? ERROR_INVALID_PARAMETER : ERROR_MR_MID_NOT_FOUND);
      return 0;
   }
   if (buffer == nullptr)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return 0;
   }

   InsertArgs inserts((flags & FORMAT_MESSAGE_ARGUMENT_ARRAY) ? nullptr : args,
                      (flags & FORMAT_MESSAGE_ARGUMENT_ARRAY) ? (const DWORD_PTR *) args : nullptr);
   std::string out;
   if (!FormatTemplate(out, t, (flags & FORMAT_MESSAGE_IGNORE_INSERTS) != 0,
                       flags & FORMAT_MESSAGE_MAX_WIDTH_MASK, inserts))
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return 0;
   }

   if (flags & FORMAT_MESSAGE_ALLOCATE_BUFFER)
   {
      // buffer is where the LocalAlloc'd text goes; size its least size.
      size_t n = std::max<size_t>(out.size() + 1, size);
      char *p = (char *) LocalAlloc(LMEM_FIXED, n);
      if (p == nullptr)
         return 0;
      memcpy(p, out.c_str(), out.size() + 1);
      *(char **) buffer = p;
      return (DWORD) out.size();
   }
   if (out.size() + 1 > size)
   {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return 0;
   }
   memcpy(buffer, out.c_str(), out.size() + 1);
   return (DWORD) out.size();
}

// A GUI process has no console until it asks; here stdout and stderr are the
// console it gets.
BOOL WINAPI AllocConsole(void)
{
   if (console_allocated.exchange(true))
   {
      SetLastError(ERROR_ACCESS_DENIED);
      return FALSE;
   }
   return TRUE;
}

}  // extern "C"

// MSVC calls the handler with the failed size and retries while it returns
// nonzero; std::new_handler takes none, so the handler gets 0.
namespace {
std::atomic<_PNH> crt_new_handler{nullptr};

void NewHandlerTrampoline()
{
   _PNH handler = crt_new_handler.load();
   if (handler == nullptr || handler(0) == 0)
      throw std::bad_alloc();
}
}  // namespace

W32_EXPORT _PNH _set_new_handler(_PNH handler)
{
   _PNH old = crt_new_handler.exchange(handler);
   std::set_new_handler(handler != nullptr ? NewHandlerTrampoline : nullptr);
   return old;
}
