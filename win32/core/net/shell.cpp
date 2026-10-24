/*
 * shell.cpp:  ShellExecute.  URLs, documents and folders go to the desktop's
 *   opener (xdg-open; open on macOS); executables run with their parameters
 *   split as CommandLineToArgvW splits them.
 */

#include "core/win32.h"
#include "core/net/net.h"
#include "core/paths.h"

#include <shellapi.h>

#include <cerrno>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include <fcntl.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {

using w32::net::SetError;

#ifdef __APPLE__
const char kOpener[] = "open";
#else
const char kOpener[] = "xdg-open";
#endif

// "scheme:..." with a scheme of two or more characters (one is a drive).
bool IsUrl(const char *s)
{
   if (!isalpha((unsigned char) s[0]))
      return false;
   size_t i = 1;
   while (isalnum((unsigned char) s[i]) || s[i] == '+' || s[i] == '-' || s[i] == '.')
      i++;
   return s[i] == ':' && i >= 2;
}

// CommandLineToArgvW's rules: 2n backslashes before a quote are n and the quote
// toggles quoting; 2n+1 are n and a literal quote; "" inside quotes is a quote.
std::vector<std::string> SplitArgs(const char *s)
{
   std::vector<std::string> args;
   if (s == nullptr)
      return args;
   size_t i = 0, n = std::strlen(s);
   while (true)
   {
      while (i < n && (s[i] == ' ' || s[i] == '\t'))
         i++;
      if (i >= n)
         break;
      std::string arg;
      bool quoted = false;
      while (i < n && (quoted || (s[i] != ' ' && s[i] != '\t')))
      {
         if (s[i] == '\\')
         {
            size_t slashes = 0;
            while (i < n && s[i] == '\\')
               slashes++, i++;
            if (i < n && s[i] == '"')
            {
               arg.append(slashes / 2, '\\');
               if (slashes % 2)
               {
                  arg += '"';
                  i++;
               }
            }
            else
               arg.append(slashes, '\\');
         }
         else if (s[i] == '"')
         {
            if (quoted && i + 1 < n && s[i + 1] == '"')
            {
               arg += '"';
               i += 2;
            }
            else
            {
               quoted = !quoted;
               i++;
            }
         }
         else
            arg += s[i++];
      }
      args.push_back(arg);
   }
   return args;
}

// Starts argv[0] (looked up in PATH) in dir; 0, or the errno of the failed exec.
int Spawn(const std::vector<std::string> &args, const std::string &dir)
{
   std::vector<char *> argv;
   for (const std::string &a : args)
      argv.push_back(const_cast<char *>(a.c_str()));
   argv.push_back(nullptr);

   // The exec's errno comes back through a pipe that a successful exec closes.
   int fds[2];
   if (::pipe(fds) != 0)
      return errno;
   ::fcntl(fds[0], F_SETFD, FD_CLOEXEC);
   ::fcntl(fds[1], F_SETFD, FD_CLOEXEC);
   pid_t pid = ::fork();
   if (pid < 0)
   {
      int e = errno;
      ::close(fds[0]);
      ::close(fds[1]);
      return e;
   }
   if (pid == 0)
   {
      ::close(fds[0]);
      ::setsid();
      if (!dir.empty() && ::chdir(dir.c_str()) != 0)
      {
         int e = errno;
         (void) !::write(fds[1], &e, sizeof(e));
         ::_exit(127);
      }
      ::execvp(argv[0], argv.data());
      int e = errno;
      (void) !::write(fds[1], &e, sizeof(e));
      ::_exit(127);
   }
   ::close(fds[1]);
   int e = 0;
   ssize_t n;
   do
      n = ::read(fds[0], &e, sizeof(e));
   while (n < 0 && errno == EINTR);
   ::close(fds[0]);
   if (n != (ssize_t) sizeof(e))
      e = 0;
   // Reap it whenever it ends.
   std::thread([pid] { int status; while (::waitpid(pid, &status, 0) < 0 && errno == EINTR) {} }).detach();
   return e;
}

// The file a Windows name means, absolute: looked up in directory first if it
// is relative, as ShellExecute does, then in the current directory.
bool Locate(const char *file, LPCSTR directory, std::string &path, struct stat &st)
{
   std::vector<std::string> names;
   bool relative = !(file[0] == '\\' || file[0] == '/' || (file[0] && file[1] == ':'));
   if (relative && directory != nullptr && *directory)
      names.push_back(std::string(directory) + "\\" + file);
   names.push_back(file);
   for (const std::string &name : names)
   {
      // "club.exe": the native build of a Windows program has no extension.
      std::string bare;
      if (name.size() > 4 && ::strcasecmp(name.c_str() + name.size() - 4, ".exe") == 0)
         bare = name.substr(0, name.size() - 4);
      for (const std::string &n : { name, bare })
      {
         if (n.empty())
            continue;
         path = w32::HostPath(n.c_str());
         if (::stat(path.c_str(), &st) != 0)
            continue;
         if (path[0] != '/')
         {
            char cwd[4096];
            if (::getcwd(cwd, sizeof(cwd)) != nullptr)
               path = std::string(cwd) + "/" + path;
         }
         return true;
      }
   }
   return false;
}

// ShellExecuteEx's work: the SE_ERR_ value (> 32 on success) and SetLastError.
UINT_PTR Execute(LPCSTR verb, LPCSTR file, LPCSTR parameters, LPCSTR directory)
{
   if (file == nullptr || *file == 0)
   {
      SetError(ERROR_FILE_NOT_FOUND);
      return SE_ERR_FNF;
   }
   if (verb != nullptr && *verb && ::strcasecmp(verb, "open") != 0 && ::strcasecmp(verb, "runas") != 0 &&
       ::strcasecmp(verb, "explore") != 0 && ::strcasecmp(verb, "edit") != 0)
   {
      SetError(ERROR_NO_ASSOCIATION);
      return SE_ERR_NOASSOC;
   }

   std::string dir;
   if (directory != nullptr && *directory)
   {
      dir = w32::HostPath(directory);
      struct stat st;
      if (::stat(dir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
      {
         SetError(ERROR_PATH_NOT_FOUND);
         return SE_ERR_PNF;
      }
   }

   std::vector<std::string> args;
   if (IsUrl(file))
      args = { kOpener, file };
   else
   {
      std::string path;
      struct stat st;
      if (!Locate(file, directory, path, st))
      {
         SetError(ERROR_FILE_NOT_FOUND);
         return SE_ERR_FNF;
      }
      if (S_ISREG(st.st_mode) && ::access(path.c_str(), X_OK) == 0)
      {
         args = { path };
         for (std::string &a : SplitArgs(parameters))
            args.push_back(a);
      }
      else
         args = { kOpener, path };
   }

   int e = Spawn(args, dir);
   switch (e)
   {
   case 0:
      return 42;   // any value > 32
   case ENOENT:
      // Only the opener can be missing: the file was there.
      SetError(args[0] == kOpener ? ERROR_NO_ASSOCIATION : ERROR_FILE_NOT_FOUND);
      return args[0] == kOpener ? SE_ERR_NOASSOC : SE_ERR_FNF;
   case EACCES:
   case EPERM:
      SetError(ERROR_ACCESS_DENIED);
      return SE_ERR_ACCESSDENIED;
   case ENOMEM:
      SetError(ERROR_NOT_ENOUGH_MEMORY);
      return SE_ERR_OOM;
   default:
      SetError(ERROR_FILE_NOT_FOUND);
      return SE_ERR_FNF;
   }
}

}  // namespace

HINSTANCE WINAPI ShellExecuteA(HWND, LPCSTR verb, LPCSTR file, LPCSTR parameters, LPCSTR directory, INT)
{
   return (HINSTANCE) Execute(verb, file, parameters, directory);
}

BOOL WINAPI ShellExecuteExA(LPSHELLEXECUTEINFOA info)
{
   if (info == nullptr || info->cbSize != sizeof(SHELLEXECUTEINFOA))
   {
      SetError(ERROR_INVALID_PARAMETER);
      return FALSE;
   }
   // No process handle (SEE_MASK_NOCLOSEPROCESS): the child is not a Win32 object.
   info->hProcess = nullptr;
   UINT_PTR result = Execute(info->lpVerb, info->lpFile, info->lpParameters, info->lpDirectory);
   info->hInstApp = (HINSTANCE) result;
   return result > 32;
}
