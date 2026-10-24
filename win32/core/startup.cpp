/*
 * startup.cpp:  The program's entry: what Windows does before WinMain.
 */

#include "core/module.h"
#include "core/native.h"

#include <string>

extern "C" int __argc;
extern "C" char **__argv;

// The client's entry point.  Weak: programs with their own main (build tools)
// link the layer without one.
extern "C" int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command_line, int show)
   __attribute__((weak));

namespace w32 {
void AttachDlls();
}

namespace {

std::string command_line;

}  // namespace

extern "C" W32_EXPORT LPSTR WINAPI GetCommandLineA(void)
{
   return (LPSTR) command_line.c_str();
}

W32_EXPORT int main(int argc, char **argv)
{
   __argc = argc;
   __argv = argv;
   // The command line WinMain gets: the arguments after the program, quoted when
   // they have spaces (GetCommandLine has the program too).
   std::string args;
   for (int i = 1; i < argc; i++)
   {
      std::string a = argv[i];
      if (!args.empty())
         args += ' ';
      args += a.find(' ') != std::string::npos ? "\"" + a + "\"" : a;
   }
   command_line = std::string("\"") + argv[0] + "\"" + (args.empty() ? "" : " " + args);
   if (WinMain == nullptr)
      return 1;
   w32::native::Init();
   w32::AttachDlls();
   return WinMain(w32::MainModule(), nullptr, (LPSTR) (command_line.c_str() + command_line.size() - args.size()),
                  SW_SHOWNORMAL);
}
