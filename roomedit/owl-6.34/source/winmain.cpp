//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of WinMain for user exes
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/applicat.h>
#include <owl/lclstrng.h>

#if defined(BI_COMP_BORLANDC)
#include <dos.h>
#endif

//#ifndef MAX_PATH
//#if !defined(WINELIB)
//#include <dos.h>
//#endif
//#endif

//Changes by Peter Sliepenbeek, revised by Sebastian Ledesma
#if defined __GNUC__ //since 3.x and above (probably 2.x also, but not tested)
# include <vector>
#endif 

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlMain);

} // OWL namespace

using namespace std;
using namespace owl;

// MSC code in module.cpp
#if !defined(_BUILDOWLDLL) && !defined(_OWLDLL)
#if defined(BI_COMP_BORLANDC)
// NOTE: We use priority 31 to come just before/after ctr/dtr of global
//       objects (which are assigned a priorority of 32)
//
long TlsAddRefs();
long TlsRelease();

static void __initOWL()
{
  TlsAddRefs();
}
#pragma startup __initOWL 31

//
static void __termOWL()
{
  TlsRelease();
}
#pragma exit __termOWL 31
#endif // BI_COMP_BORLANDC
#endif // if !_BUILDOWLDLL

using owl::TModule;
using owl::TApplication;

#if defined(BI_COMP_BORLANDC) && defined(_UNICODE)

#if __BORLANDC__ < 0x610
extern wchar_t ** _RTLENTRY _EXPDATA _wargv;
#endif
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR cmdLine, int cmdShow);

#elif defined(BI_COMP_GNUC)

extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR cmdLine, int cmdShow);

#endif

//
// Default WinMain calls OwlMain, after setting params into TApplication
//
int WINAPI 
_tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR cmdLine, int cmdShow)
{
  TRACEX(OwlMain, 0, _T("WinMain(") << hex << uint(hInstance) << _T(", ") <<
                                  hex << uint(hPrevInstance) << _T(", \"") <<
                                  TResId(cmdLine) << _T("\", ") <<
                                  cmdShow << _T(") called"));

  InitGlobalModule(hInstance);

  TApplication::SetWinMainParams(hInstance, hPrevInstance, 
                                 cmdLine?cmdLine:_T(""), cmdShow);
  try {
#if defined(BI_COMP_BORLANDC)
    int retVal = OwlMain(_argc, _targv);
#elif defined(BI_COMP_GNUC)
    std::basic_string<TCHAR> cmdLine(::GetCommandLine());
    std::vector<std::basic_string<TCHAR> > argumentVector;
    
    std::basic_string<TCHAR>::size_type tokenStart = 0;
    std::basic_string<TCHAR>::size_type commandLineStringLength = 
      cmdLine.size();
    while (tokenStart != std::basic_string<TCHAR>::npos && 
     tokenStart != commandLineStringLength)
      {
  std::basic_string<TCHAR>::size_type tokenEnd = 
    cmdLine.find(' ', tokenStart);
  argumentVector.push_back
    (std::basic_string<TCHAR>
     (cmdLine.substr(tokenStart, 
         tokenEnd == std::basic_string<TCHAR>::npos ? 
         tokenEnd : tokenEnd - tokenStart)));
  std::cerr << (argumentVector.end()-1)->c_str() << std::endl;
  if (tokenEnd != std::basic_string<TCHAR>::npos && 
      tokenEnd != commandLineStringLength) // <- should never happen
    tokenStart = tokenEnd + 1;
  else
    tokenStart = tokenEnd;
      }

    // argvArray is never freed, since it must be kept around until 
    // at least the end of OwlMain.  We could probably release
    // it after the call to OwlMain, but I'd rather not risk
    // then chance of crashes over a few hundred bytes
    TCHAR** argvArray = new TCHAR*[argumentVector.size() + 1];
    for (int i = 0; i < argumentVector.size(); i++)
      argvArray[i] = (TCHAR*)argumentVector[i].c_str();
    argvArray[argumentVector.size() + 1] = 0;
    int retVal = OwlMain(argumentVector.size(), argvArray);
#else
    int retVal = OwlMain(__argc, __targv);
#endif
    TRACEX(OwlMain, 0, _T("WinMain() returns ") << retVal);
    return retVal;
  }
  catch (owl::TXEndSession&) {return 0;} 
  catch (owl::TXBase& x) {return owl::HandleGlobalException(x, 0);} 
}

//} // OWL namespace
/* ========================================================================== */

