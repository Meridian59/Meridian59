/*
 * paths.cpp:  Windows file names on a POSIX file system.  The Windows sources name
 *   files with '\' separators and in any case ("resource\\Splash.BGF"); the C
 *   runtime calls they make are redirected here (include/crt/msvcrt.h), and so are
 *   the layer's own file APIs.
 */

#include "core/paths.h"

#include <cstdarg>
#include <dlfcn.h>
#include <cstdio>
#include <dirent.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

namespace w32 {

namespace {

bool Exists(const std::string &path)
{
   struct stat st;
   return ::lstat(path.c_str(), &st) == 0;
}

// The entry of dir whose name matches name ignoring case, or name itself.
std::string MatchEntry(const std::string &dir, const std::string &name)
{
   DIR *d = ::opendir(dir.empty() ? "." : dir.c_str());
   if (d == nullptr)
      return name;
   std::string found = name;
   while (dirent *e = ::readdir(d))
      if (::strcasecmp(e->d_name, name.c_str()) == 0)
      {
         found = e->d_name;
         break;
      }
   ::closedir(d);
   return found;
}

}  // namespace

std::string HostPath(const char *path)
{
   if (path == nullptr)
      return std::string();
   std::string p(path);
   for (char &c : p)
      if (c == '\\')
         c = '/';
   // A drive ("C:") names the root; the game only ever means its own directory.
   if (p.size() >= 2 && p[1] == ':' && isalpha((unsigned char) p[0]))
      p.erase(0, 2);
   if (p.empty() || Exists(p))
      return p;

   // Resolve each component that doesn't exist as given, ignoring case.
   std::string out;
   size_t start = 0;
   if (p[0] == '/')
   {
      out = "/";
      start = 1;
   }
   while (start <= p.size())
   {
      size_t end = p.find('/', start);
      if (end == std::string::npos)
         end = p.size();
      std::string part = p.substr(start, end - start);
      if (!part.empty())
      {
         std::string dir = out.empty() ? "." : out;
         std::string candidate = (out.empty() || out == "/") ? out + part : out + "/" + part;
         if (!Exists(candidate) && part != "." && part != "..")
            candidate = (out.empty() || out == "/" ? out : out + "/") + MatchEntry(dir, part);
         out = candidate;
      }
      start = end + 1;
   }
   return out;
}

}  // namespace w32

// The C runtime's file functions as the Windows sources call them.
extern "C" {

W32_EXPORT FILE *m59_fopen(const char *path, const char *mode)
{
   return ::fopen(w32::HostPath(path).c_str(), mode);
}

W32_EXPORT FILE *m59_freopen(const char *path, const char *mode, FILE *stream)
{
   return ::freopen(path ? w32::HostPath(path).c_str() : nullptr, mode, stream);
}

W32_EXPORT int m59_open(const char *path, int flags, ...)
{
   mode_t mode = 0;
   if (flags & O_CREAT)
   {
      va_list ap;
      va_start(ap, flags);
      mode = (mode_t) va_arg(ap, int);
      va_end(ap);
   }
   return ::open(w32::HostPath(path).c_str(), flags, mode);
}

// libarchive's, by its Windows name: the client links libarchive, the layer
// finds it there.
W32_EXPORT int m59_archive_read_open_filename(void *archive, const char *path, size_t block_size)
{
   using Open = int (*)(void *, const char *, size_t);
   static Open open = (Open) dlsym(RTLD_DEFAULT, "archive_read_open_filename");
   if (open == nullptr)
      return -30;   // ARCHIVE_FATAL
   return open(archive, path ? w32::HostPath(path).c_str() : nullptr, block_size);
}

W32_EXPORT int m59_stat(const char *path, struct stat *st)
{
   return ::stat(w32::HostPath(path).c_str(), st);
}

W32_EXPORT int m59_access(const char *path, int mode)
{
   return ::access(w32::HostPath(path).c_str(), mode);
}

W32_EXPORT int m59_unlink(const char *path)
{
   return ::unlink(w32::HostPath(path).c_str());
}

W32_EXPORT int m59_remove(const char *path)
{
   return ::remove(w32::HostPath(path).c_str());
}

W32_EXPORT int m59_rename(const char *from, const char *to)
{
   return ::rename(w32::HostPath(from).c_str(), w32::HostPath(to).c_str());
}

W32_EXPORT int m59_mkdir(const char *path, mode_t mode)
{
   return ::mkdir(w32::HostPath(path).c_str(), mode);
}

W32_EXPORT int m59_rmdir(const char *path)
{
   return ::rmdir(w32::HostPath(path).c_str());
}

W32_EXPORT int m59_chdir(const char *path)
{
   return ::chdir(w32::HostPath(path).c_str());
}

W32_EXPORT DIR *m59_opendir(const char *path)
{
   return ::opendir(w32::HostPath(path).c_str());
}

}  // extern "C"
