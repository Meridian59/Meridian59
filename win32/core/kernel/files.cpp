/*
 * files.cpp:  File handles, directory searches (FindFirstFile) and file mappings
 *   on POSIX files.  Windows names go through HostPath.  Share modes are not
 *   enforced: nothing else here opens the client's files.
 */

#include "core/kernel/kernel.h"
#include "core/paths.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <map>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace w32 {

namespace {

class File : public KernelObject
{
public:
   File(int fd, bool owned) : KernelObject(KernelKind::File), fd(fd), owned(owned) {}
   ~File() override { if (owned) ::close(fd); }
   const int fd;
   const bool owned;   // standard handles don't close the process's fds
};

class Mapping : public KernelObject
{
public:
   Mapping() : KernelObject(KernelKind::Mapping) {}
   ~Mapping() override { if (fd >= 0) ::close(fd); }
   int fd = -1;          // a duplicate: a mapping outlives its file handle; -1: memory only
   uint64_t size = 0;
   DWORD protect = 0;    // PAGE_*
};

struct FindEntry
{
   std::string name;
   struct stat st;
};

class Find : public KernelObject
{
public:
   Find() : KernelObject(KernelKind::Find) {}
   std::vector<FindEntry> entries;
   std::mutex lock;
   size_t next = 0;   // under lock
};

// Views: base -> length, for UnmapViewOfFile.
std::mutex views_lock;
std::map<const void *, size_t> views;

// 100 ns intervals since 1601-01-01.
FILETIME ToFileTime(const timespec &t)
{
   uint64_t v = ((uint64_t) t.tv_sec + 11644473600ull) * 10000000ull + (uint64_t) t.tv_nsec / 100;
   return {(DWORD) v, (DWORD) (v >> 32)};
}

#ifdef __APPLE__
const timespec &ModifyTime(const struct stat &st) { return st.st_mtimespec; }
const timespec &AccessTime(const struct stat &st) { return st.st_atimespec; }
const timespec &CreateTime(const struct stat &st) { return st.st_birthtimespec; }
#else
const timespec &ModifyTime(const struct stat &st) { return st.st_mtim; }
const timespec &AccessTime(const struct stat &st) { return st.st_atim; }
// No birth time in struct stat; a file's contents are as old as its last write.
const timespec &CreateTime(const struct stat &st) { return st.st_mtim; }
#endif

DWORD Attributes(const std::string &name, const struct stat &st)
{
   DWORD a = 0;
   if (S_ISDIR(st.st_mode))
      a |= FILE_ATTRIBUTE_DIRECTORY;
   else
      a |= FILE_ATTRIBUTE_ARCHIVE;
   if ((st.st_mode & S_IWUSR) == 0)
      a |= FILE_ATTRIBUTE_READONLY;
   // Unix hidden files, as Wine shows them.
   if (name.size() > 1 && name[0] == '.' && name != "..")
      a |= FILE_ATTRIBUTE_HIDDEN;
   return a;
}

void FillFindData(const FindEntry &e, WIN32_FIND_DATAA *data)
{
   memset(data, 0, sizeof(*data));
   data->dwFileAttributes = Attributes(e.name, e.st);
   data->ftCreationTime = ToFileTime(CreateTime(e.st));
   data->ftLastAccessTime = ToFileTime(AccessTime(e.st));
   data->ftLastWriteTime = ToFileTime(ModifyTime(e.st));
   if (!S_ISDIR(e.st.st_mode))
   {
      data->nFileSizeHigh = (DWORD) ((uint64_t) e.st.st_size >> 32);
      data->nFileSizeLow = (DWORD) e.st.st_size;
   }
   size_t n = std::min(e.name.size(), sizeof(data->cFileName) - 1);
   memcpy(data->cFileName, e.name.data(), n);
}

int Lower(char c) { return tolower((unsigned char) c); }

// Windows wildcard match ignoring case: '*' any run, '?' any one character.
bool WildMatch(const char *p, const char *s)
{
   const char *star = nullptr, *resume = nullptr;
   while (*s)
   {
      if (*p == '?' || (*p != '*' && *p && Lower(*p) == Lower(*s)))
      {
         p++;
         s++;
      }
      else if (*p == '*')
      {
         star = p++;
         resume = s;
      }
      else if (star != nullptr)
      {
         p = star + 1;
         s = ++resume;
      }
      else
         return false;
   }
   while (*p == '*' || *p == '?')
      p++;
   return *p == 0;
}

// As Windows matches: "*.*" is everything, and "name.*" matches "name" too
// (names without an extension match an empty one).
bool PatternMatch(const std::string &pattern, const std::string &name)
{
   if (WildMatch(pattern.c_str(), name.c_str()))
      return true;
   if (pattern.size() >= 2 && pattern.compare(pattern.size() - 2, 2, ".*") == 0 &&
       name.find('.') == std::string::npos)
      return WildMatch(pattern.substr(0, pattern.size() - 2).c_str(), name.c_str());
   return false;
}

// NTFS returns names in upper-case order.
bool NameLess(const FindEntry &a, const FindEntry &b)
{
   size_t n = std::min(a.name.size(), b.name.size());
   for (size_t i = 0; i < n; i++)
   {
      int ca = toupper((unsigned char) a.name[i]), cb = toupper((unsigned char) b.name[i]);
      if (ca != cb)
         return ca < cb;
   }
   return a.name.size() < b.name.size();
}

}  // namespace

}  // namespace w32

using namespace w32;

extern "C" {

HANDLE WINAPI CreateFileA(LPCSTR name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES,
                          DWORD disposition, DWORD flags, HANDLE templ)
{
   (void) share;
   (void) templ;
   if (name == nullptr || *name == 0)
   {
      SetLastError(ERROR_PATH_NOT_FOUND);
      return INVALID_HANDLE_VALUE;
   }
   std::string path = HostPath(name);

   int mode;
   bool read = (access & (GENERIC_READ | GENERIC_ALL | FILE_READ_DATA)) != 0;
   bool write = (access & (GENERIC_WRITE | GENERIC_ALL | FILE_WRITE_DATA | FILE_APPEND_DATA)) != 0;
   if (read && write)
      mode = O_RDWR;
   else if (write)
      mode = O_WRONLY;
   else
      mode = O_RDONLY;

   struct stat st;
   bool existed = ::stat(path.c_str(), &st) == 0;
   switch (disposition)
   {
   case CREATE_NEW:        mode |= O_CREAT | O_EXCL; break;
   case CREATE_ALWAYS:     mode |= O_CREAT | O_TRUNC; break;
   case OPEN_EXISTING:     break;
   case OPEN_ALWAYS:       mode |= O_CREAT; break;
   case TRUNCATE_EXISTING:
      if (!write)
      {
         SetLastError(ERROR_INVALID_PARAMETER);
         return INVALID_HANDLE_VALUE;
      }
      mode |= O_TRUNC;
      break;
   default:
      SetLastError(ERROR_INVALID_PARAMETER);
      return INVALID_HANDLE_VALUE;
   }
   if (existed && S_ISDIR(st.st_mode) && !(flags & FILE_FLAG_BACKUP_SEMANTICS))
   {
      SetLastError(ERROR_ACCESS_DENIED);
      return INVALID_HANDLE_VALUE;
   }

   // Windows' read-only attribute is the owner's write bit.
   mode_t perms = (flags & FILE_ATTRIBUTE_READONLY) ? 0444 : 0666;
   int fd = ::open(path.c_str(), mode | O_CLOEXEC, perms);
   if (fd < 0)
   {
      int error = errno;
      if (error == ENOENT)
      {
         // A missing directory on the way is a missing path.
         size_t slash = path.rfind('/');
         struct stat dir;
         if (slash != std::string::npos && slash > 0 &&
             ::stat(path.substr(0, slash).c_str(), &dir) != 0)
         {
            SetLastError(ERROR_PATH_NOT_FOUND);
            return INVALID_HANDLE_VALUE;
         }
      }
      SetLastError(ErrorFromErrno(error));
      return INVALID_HANDLE_VALUE;
   }
   HANDLE h = AddKernelObject(new File(fd, true));
   // Opening an existing file with CREATE_ALWAYS/OPEN_ALWAYS succeeds and says so.
   if (existed && (disposition == CREATE_ALWAYS || disposition == OPEN_ALWAYS))
      SetLastError(ERROR_ALREADY_EXISTS);
   else
      SetLastError(ERROR_SUCCESS);
   return h;
}

BOOL WINAPI WriteFile(HANDLE h, LPCVOID buffer, DWORD count, LPDWORD written, LPOVERLAPPED overlapped)
{
   if (written != nullptr)
      *written = 0;
   KernelRef<File> file(RefKernelObject(h, KernelKind::File));
   if (!file)
      return FALSE;
   if (overlapped != nullptr)
   {
      // Synchronous handles with an OVERLAPPED write at its offset.
      off_t offset = (off_t) (((uint64_t) overlapped->OffsetHigh << 32) | overlapped->Offset);
      ssize_t n = ::pwrite(file->fd, buffer, count, offset);
      if (n < 0)
      {
         SetLastErrorFromErrno();
         return FALSE;
      }
      if (written != nullptr)
         *written = (DWORD) n;
      return TRUE;
   }
   const char *p = static_cast<const char *>(buffer);
   DWORD done = 0;
   while (done < count)
   {
      ssize_t n = ::write(file->fd, p + done, count - done);
      if (n < 0)
      {
         if (errno == EINTR)
            continue;
         SetLastErrorFromErrno();
         if (written != nullptr)
            *written = done;
         return FALSE;
      }
      done += (DWORD) n;
   }
   if (written != nullptr)
      *written = done;
   return TRUE;
}

DWORD WINAPI GetFileSize(HANDLE h, LPDWORD high)
{
   KernelRef<File> file(RefKernelObject(h, KernelKind::File));
   if (!file)
      return INVALID_FILE_SIZE;
   struct stat st;
   if (::fstat(file->fd, &st) != 0)
   {
      SetLastErrorFromErrno();
      return INVALID_FILE_SIZE;
   }
   uint64_t size = (uint64_t) st.st_size;
   if (high != nullptr)
      *high = (DWORD) (size >> 32);
   // A real size whose low half is INVALID_FILE_SIZE reads as success.
   SetLastError(ERROR_SUCCESS);
   return (DWORD) size;
}

BOOL WINAPI DeleteFileA(LPCSTR name)
{
   if (name == nullptr)
   {
      SetLastError(ERROR_PATH_NOT_FOUND);
      return FALSE;
   }
   std::string path = HostPath(name);
   struct stat st;
   if (::lstat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
   {
      SetLastError(ERROR_ACCESS_DENIED);
      return FALSE;
   }
   if (::unlink(path.c_str()) != 0)
   {
      SetLastErrorFromErrno();
      return FALSE;
   }
   return TRUE;
}

HANDLE WINAPI FindFirstFileA(LPCSTR spec, LPWIN32_FIND_DATAA data)
{
   if (spec == nullptr || *spec == 0 || data == nullptr)
   {
      SetLastError(ERROR_FILE_NOT_FOUND);
      return INVALID_HANDLE_VALUE;
   }
   std::string s(spec);
   size_t sep = s.find_last_of("\\/");
   std::string dir = sep == std::string::npos ? std::string() : s.substr(0, sep + 1);
   std::string pattern = sep == std::string::npos ? s : s.substr(sep + 1);
   // A drive alone ("C:foo") names the current directory here.
   if (sep == std::string::npos && s.size() >= 2 && s[1] == ':')
      pattern = s.substr(2);
   if (pattern.empty())
   {
      SetLastError(ERROR_FILE_NOT_FOUND);
      return INVALID_HANDLE_VALUE;
   }

   std::string host_dir = dir.empty() ? std::string(".") : HostPath(dir.c_str());
   if (host_dir.empty())
      host_dir = "/";
   DIR *d = ::opendir(host_dir.c_str());
   if (d == nullptr)
   {
      SetLastError(errno == ENOENT || errno == ENOTDIR ? ERROR_PATH_NOT_FOUND : ErrorFromErrno(errno));
      return INVALID_HANDLE_VALUE;
   }
   auto *find = new Find;
   std::string base = host_dir.back() == '/' ? host_dir : host_dir + "/";
   while (dirent *e = ::readdir(d))
   {
      std::string name = e->d_name;
      if (!PatternMatch(pattern, name))
         continue;
      FindEntry entry;
      entry.name = name;
      if (::stat((base + name).c_str(), &entry.st) != 0 &&
          ::lstat((base + name).c_str(), &entry.st) != 0)
         continue;
      find->entries.push_back(std::move(entry));
   }
   ::closedir(d);
   if (find->entries.empty())
   {
      find->Release();
      SetLastError(ERROR_FILE_NOT_FOUND);
      return INVALID_HANDLE_VALUE;
   }
   std::sort(find->entries.begin(), find->entries.end(), NameLess);
   FillFindData(find->entries[0], data);
   find->next = 1;
   return AddKernelObject(find);
}

BOOL WINAPI FindNextFileA(HANDLE h, LPWIN32_FIND_DATAA data)
{
   KernelRef<Find> find(RefKernelObject(h, KernelKind::Find));
   if (!find)
      return FALSE;
   std::lock_guard<std::mutex> lock(find->lock);
   if (find->next >= find->entries.size())
   {
      SetLastError(ERROR_NO_MORE_FILES);
      return FALSE;
   }
   FillFindData(find->entries[find->next++], data);
   return TRUE;
}

BOOL WINAPI FindClose(HANDLE h)
{
   return CloseKernelHandle(h, true);
}

UINT WINAPI GetCurrentDirectoryA(UINT size, LPSTR buffer)
{
   char cwd[4096];
   if (::getcwd(cwd, sizeof(cwd)) == nullptr)
   {
      SetLastErrorFromErrno();
      return 0;
   }
   UINT n = (UINT) strlen(cwd);
   if (buffer == nullptr || size <= n)
      return n + 1;
   memcpy(buffer, cwd, n + 1);
   return n;
}

HANDLE WINAPI CreateFileMappingA(HANDLE h, LPSECURITY_ATTRIBUTES, DWORD protect, DWORD size_high,
                                 DWORD size_low, LPCSTR name)
{
   (void) name;   // named mappings would be shared between processes
   uint64_t size = ((uint64_t) size_high << 32) | size_low;
   DWORD page = protect & 0xff;
   bool writable = page == PAGE_READWRITE || page == PAGE_EXECUTE_READWRITE;
   if (page != PAGE_READONLY && page != PAGE_READWRITE && page != PAGE_WRITECOPY &&
       page != PAGE_EXECUTE_READ && page != PAGE_EXECUTE_READWRITE)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return nullptr;
   }

   auto *mapping = new Mapping;
   mapping->protect = page;
   if (h == INVALID_HANDLE_VALUE)
   {
      // Backed by the page file: memory only.
      if (size == 0)
      {
         mapping->Release();
         SetLastError(ERROR_INVALID_PARAMETER);
         return nullptr;
      }
      mapping->size = size;
      return AddKernelObject(mapping);
   }

   KernelRef<File> file(RefKernelObject(h, KernelKind::File));
   if (!file)
   {
      mapping->Release();
      return nullptr;
   }
   struct stat st;
   if (::fstat(file->fd, &st) != 0)
   {
      SetLastErrorFromErrno();
      mapping->Release();
      return nullptr;
   }
   uint64_t file_size = (uint64_t) st.st_size;
   if (size == 0)
   {
      if (file_size == 0)
      {
         mapping->Release();
         SetLastError(ERROR_FILE_INVALID);
         return nullptr;
      }
      size = file_size;
   }
   if (size > file_size)
   {
      // Windows grows the file to the mapping's size, if it may write.
      if (!writable || ::ftruncate(file->fd, (off_t) size) != 0)
      {
         mapping->Release();
         SetLastError(writable ? ErrorFromErrno(errno) : ERROR_NOT_ENOUGH_MEMORY);
         return nullptr;
      }
   }
   mapping->fd = ::fcntl(file->fd, F_DUPFD_CLOEXEC, 0);
   if (mapping->fd < 0)
   {
      SetLastErrorFromErrno();
      mapping->Release();
      return nullptr;
   }
   mapping->size = size;
   SetLastError(ERROR_SUCCESS);
   return AddKernelObject(mapping);
}

LPVOID WINAPI MapViewOfFile(HANDLE h, DWORD access, DWORD offset_high, DWORD offset_low, SIZE_T bytes)
{
   KernelRef<Mapping> mapping(RefKernelObject(h, KernelKind::Mapping));
   if (!mapping)
      return nullptr;
   uint64_t offset = ((uint64_t) offset_high << 32) | offset_low;
   if (offset % 0x10000 != 0)
   {
      SetLastError(ERROR_MAPPED_ALIGNMENT);
      return nullptr;
   }
   if (offset > mapping->size || (bytes != 0 && bytes > mapping->size - offset))
   {
      SetLastError(ERROR_ACCESS_DENIED);
      return nullptr;
   }
   size_t length = bytes != 0 ? bytes : (size_t) (mapping->size - offset);
   if (length == 0)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return nullptr;
   }

   // FILE_MAP_COPY alone: private writable pages.
   bool copy = (access & (FILE_MAP_COPY | FILE_MAP_WRITE | FILE_MAP_READ)) == FILE_MAP_COPY;
   bool write = (access & FILE_MAP_WRITE) != 0;
   bool may_write = mapping->protect == PAGE_READWRITE || mapping->protect == PAGE_EXECUTE_READWRITE;
   if (copy)
      write = true;
   else if (write && !may_write)
   {
      SetLastError(ERROR_ACCESS_DENIED);
      return nullptr;
   }
   if (mapping->protect == PAGE_WRITECOPY && write)
      copy = true;

   int prot = PROT_READ | (write ? PROT_WRITE : 0);
   if (access & FILE_MAP_EXECUTE)
      prot |= PROT_EXEC;
   void *p;
   if (mapping->fd < 0)
      // Memory only: one view of it is all the layer supports (each view of an
      // anonymous mapping would be its own memory).
      p = ::mmap(nullptr, length, prot, MAP_ANON | MAP_SHARED, -1, 0);
   else
      p = ::mmap(nullptr, length, prot, copy ? MAP_PRIVATE : MAP_SHARED, mapping->fd, (off_t) offset);
   if (p == MAP_FAILED)
   {
      SetLastError(errno == EACCES ? ERROR_ACCESS_DENIED : ERROR_NOT_ENOUGH_MEMORY);
      return nullptr;
   }
   std::lock_guard<std::mutex> lock(views_lock);
   views[p] = length;
   return p;
}

// The process's fds 0-2, as handles made on first use (and again after a
// CloseHandle; the fds stay open).
HANDLE WINAPI GetStdHandle(DWORD which)
{
   int fd;
   switch (which)
   {
   case STD_INPUT_HANDLE:  fd = 0; break;
   case STD_OUTPUT_HANDLE: fd = 1; break;
   case STD_ERROR_HANDLE:  fd = 2; break;
   default:
      SetLastError(ERROR_INVALID_HANDLE);
      return INVALID_HANDLE_VALUE;
   }
   static std::mutex lock;
   static HANDLE handles[3];
   std::lock_guard<std::mutex> guard(lock);
   KernelObject *object = handles[fd] != nullptr ? RefKernelObject(handles[fd], KernelKind::File) : nullptr;
   if (object != nullptr)
      object->Release();
   else
      handles[fd] = AddKernelObject(new File(fd, false));
   return handles[fd];
}

BOOL WINAPI UnmapViewOfFile(LPCVOID base)
{
   size_t length;
   {
      std::lock_guard<std::mutex> lock(views_lock);
      auto it = views.find(base);
      if (it == views.end())
      {
         SetLastError(ERROR_INVALID_ADDRESS);
         return FALSE;
      }
      length = it->second;
      views.erase(it);
   }
   ::munmap(const_cast<void *>(base), length);
   return TRUE;
}

}  // extern "C"
