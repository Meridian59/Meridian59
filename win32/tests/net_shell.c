/*
 * net_shell.c:  ShellExecute and ShellExecuteEx: URLs and documents go to the
 *   desktop's opener (a stand-in script put first in PATH here, so nothing really
 *   opens), executables run with Windows-style parameters in lpDirectory, and
 *   the SE_ERR_ results.
 */

#include <string>

// Before Windows' headers: macOS's unistd.h declares select with its fd_set,
// which winsock.h replaces when it comes first.
#include <unistd.h>

#include <windows.h>
#include <shellapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>

#ifdef __APPLE__
#define OPENER "open"
#else
#define OPENER "xdg-open"
#endif

static int failures = 0;

#define CHECK(cond)                                                    \
   do {                                                                \
      if (!(cond)) {                                                   \
         printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);        \
         failures++;                                                   \
      }                                                                \
   } while (0)

static void WriteScript(const std::string &path, const char *body)
{
   FILE *f = fopen(path.c_str(), "w");
   fprintf(f, "#!/bin/sh\n%s", body);
   fclose(f);
   chmod(path.c_str(), 0755);
}

// Waits for a script's output file and returns it.
static std::string Output(const std::string &path)
{
   for (int i = 0; i < 500; i++)
   {
      if (FILE *f = fopen(path.c_str(), "r"))
      {
         // The script writes it then renames it into place: it is whole.
         std::string s;
         char buf[512];
         size_t n;
         while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
            s.append(buf, n);
         fclose(f);
         unlink(path.c_str());
         return s;
      }
      struct timespec ts = { 0, 10000000 };
      nanosleep(&ts, NULL);
   }
   return "(no output)";
}

int main()
{
   char cwd[1024];
   getcwd(cwd, sizeof(cwd));
   std::string dir = std::string(cwd) + "/net_shell_tmp";
   mkdir(dir.c_str(), 0755);
   std::string out = dir + "/out.txt";

   // The stand-in opener records its argument.
   WriteScript(dir + "/" OPENER, "printf '%s\\n' \"$@\" > \"$0.tmp\" && mv \"$0.tmp\" \"$(dirname \"$0\")/out.txt\"\n");
   std::string path = dir + ":" + getenv("PATH");
   setenv("PATH", path.c_str(), 1);

   // A URL, as web.c opens it.
   HINSTANCE h = ShellExecuteA(NULL, "open", "https://example.com/a?b=c", NULL, NULL, SW_SHOWNORMAL);
   CHECK((INT_PTR) h > 32);
   CHECK(Output(out) == "https://example.com/a?b=c\n");

   // A document, by a Windows name in another case.
   FILE *f = fopen((dir + "/Readme.TXT").c_str(), "w");
   fclose(f);
   h = ShellExecuteA(NULL, NULL, "net_shell_tmp\\readme.txt", NULL, NULL, SW_SHOWNORMAL);
   CHECK((INT_PTR) h > 32);
   // (Under its own name where the file system folds case itself, as macOS's.)
   CHECK(strcasecmp(Output(out).c_str(), (dir + "/Readme.TXT\n").c_str()) == 0);

   // An executable named as the Windows program, as download.c runs club.exe.
   WriteScript(dir + "/club", "{ pwd; printf '[%s]\\n' \"$@\"; } > \"$0.tmp\" && mv \"$0.tmp\" out.txt\n");
   SHELLEXECUTEINFOA info;
   memset(&info, 0, sizeof(info));
   info.cbSize = sizeof(info);
   info.lpVerb = "runas";
   info.lpFile = "net_shell_tmp\\club.exe";
   info.lpParameters = "\"C:\\m59\\meridian.exe\" UPDATE \"a b\" c\\\"d e\\\\f \"g\"\"h\" x\\\\\"y z\"";
   info.lpDirectory = "net_shell_tmp";
   info.nShow = SW_NORMAL;
   CHECK(ShellExecuteExA(&info));
   CHECK((INT_PTR) info.hInstApp > 32);
   CHECK(info.hProcess == NULL);
   std::string expect = dir + "\n[C:\\m59\\meridian.exe]\n[UPDATE]\n[a b]\n[c\"d]\n[e\\\\f]\n[g\"h]\n[x\\y z]\n";
   std::string got = Output(out);
   CHECK(got == expect);
   if (got != expect)
      printf("got:\n%sexpected:\n%s", got.c_str(), expect.c_str());

   // Failures.
   CHECK((INT_PTR) ShellExecuteA(NULL, "open", "net_shell_tmp\\missing.txt", NULL, NULL, SW_SHOW) == SE_ERR_FNF);
   CHECK((INT_PTR) ShellExecuteA(NULL, "open", "", NULL, NULL, SW_SHOW) == SE_ERR_FNF);
   CHECK((INT_PTR) ShellExecuteA(NULL, "open", "net_shell_tmp\\club", NULL, "no\\such\\dir", SW_SHOW) == SE_ERR_PNF);
   CHECK((INT_PTR) ShellExecuteA(NULL, "print", "net_shell_tmp\\readme.txt", NULL, NULL, SW_SHOW) == SE_ERR_NOASSOC);
   memset(&info, 0, sizeof(info));
   info.cbSize = sizeof(info);
   info.lpFile = "net_shell_tmp\\missing.exe";
   CHECK(!ShellExecuteExA(&info) && (INT_PTR) info.hInstApp == SE_ERR_FNF);
   info.cbSize = 0;
   CHECK(!ShellExecuteExA(&info));

   // Without an opener there is no association.
   unlink((dir + "/" OPENER).c_str());
   setenv("PATH", "/nonexistent", 1);
   CHECK((INT_PTR) ShellExecuteA(NULL, "open", "https://example.com/", NULL, NULL, SW_SHOW) == SE_ERR_NOASSOC);

   unlink((dir + "/club").c_str());
   unlink((dir + "/Readme.TXT").c_str());
   rmdir(dir.c_str());
   printf("%s: %d failure(s)\n", failures ? "FAILED" : "PASSED", failures);
   return failures ? 1 : 0;
}
