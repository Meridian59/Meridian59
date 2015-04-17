//--------------------------------------------------------------------------
// ObjectWindows                                                          //
// Copyright (c) 1993, 1996 by Borland International
// All Rights Reserved.
// Copyright (c) 1998  by Yura Bidus                                        //
//                                                                        //
//------------------------------------------------------------------------//

#include <owl/pch.h>

#if defined(BI_COMP_BORLANDC) 
#  pragma hdrstop
#endif

#if __DEBUG == 2 
#  define USE_MEMCHECK
#endif
#ifdef _DEBUG  
#  if !defined(USE_MEMCHECK)
#    define USE_MEMCHECK
#  endif
#endif


#include <owl/private/dumpstack.h>

#include <owl/module.h>
#include <owl/template.h>
#include <owl/contain.h>

namespace owl {

using namespace std;

//-----------------------------------------------------------------------------

# if ((__GNUC__ != 3) || ((__GNUC_MINOR__ != 3) && (__GNUC_MINOR__ != 2)))
#if !defined(_WINTRUST_) && !defined(WINTRUST_H)
#     include <wintrust.h>
#   endif   // !defined(_WINTRUST_) && !defined(WINTRUST_H)
# endif   // ((__GNUC__ != 3) || ((__GNUC_MINOR__ != 3) && (__GNUC_MINOR__ != 2)))

#include <imagehlp.h>
//#pragma comment(lib, "imagehlp.lib")

static const tchar imgHlpStr[]                = _T("imagehlp.dll");
static const char SymFunctionTableAccessStr[]  = "SymFunctionTableAccess";
static const char SymGetModuleInfoStr[]        = "SymGetModuleInfo";
static const char SymLoadModuleStr[]          = "SymLoadModule";
static const char SymGetSymFromAddrStr[]      = "SymGetSymFromAddr";
static const char UnDecorateSymbolNameStr[]    = "UnDecorateSymbolName";
static const char SymUnDNameStr[]              = "SymUnDName";
static const char SymInitializeStr[]          = "SymInitialize";
static const char SymGetOptionsStr[]          = "SymGetOptions";
static const char SymSetOptionsStr[]          = "SymSetOptions";
static const char StackWalkStr[]              = "StackWalk";
//static const char Str[]        = "";
//static const char Str[]        = "";
//static const char Str[]        = "";

//-----------------------------------------------------------------------------
// class IMAGEHLP
// ~~~~~ ~~~~~~~~
//
// delay loading IMAGEHLP.DLL
class TImageHelp {
  public:
    static  TModule&  GetModule();

    //  Environmental
    static  LPVOID  SymFunctionTableAccess(HANDLE hProcess, DWORD  AddrBase);
    static  BOOL    SymGetModuleInfo(HANDLE hProcess, DWORD dwAddr, PIMAGEHLP_MODULE ModuleInfo);
    static  BOOL    SymLoadModule(HANDLE hProcess, HANDLE hFile, PSTR ImageName, PSTR ModuleName, DWORD BaseOfDll, DWORD SizeOfDll);
    static  BOOL    SymGetSymFromAddr(HANDLE hProcess, DWORD dwAddr, PDWORD pdwDisplacement, PIMAGEHLP_SYMBOL Symbol);
    static  DWORD   UnDecorateSymbolName(LPCSTR DecoratedName, LPSTR UnDecoratedName, DWORD UndecoratedLength, DWORD Flags);
    static  BOOL    SymUnDName(PIMAGEHLP_SYMBOL sym, LPSTR UnDecName, DWORD UnDecNameLength);
    static  BOOL    SymInitialize(HANDLE hProcess, LPSTR UserSearchPath, BOOL fInvadeProcess);
    static  DWORD   SymGetOptions();
    static  DWORD   SymSetOptions(DWORD SymOptions);
    static  BOOL    StackWalk(DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME StackFrame, 
                              LPVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine, 
                              PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine, 
                              PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine, 
                              PTRANSLATE_ADDRESS_ROUTINE TranslateAddress);

};
//-----------------------------------------------------------------------------
TModule&
TImageHelp::GetModule()
{
  static TModule imgModule(imgHlpStr, true, true, false);
  return imgModule;
}
//-----------------------------------------------------------------------------
#  if defined(BI_COMP_BORLANDC)
#    pragma warn -inl
#  endif
LPVOID TImageHelp::SymFunctionTableAccess(HANDLE p1, DWORD p2)
{
  static TModuleProc2<LPVOID, HANDLE, DWORD>
         symFunctionTableAccess(GetModule(), SymFunctionTableAccessStr);
  return symFunctionTableAccess(p1,p2);
}
//-----------------------------------------------------------------------------
BOOL TImageHelp::SymGetModuleInfo(HANDLE p1, DWORD p2, PIMAGEHLP_MODULE p3)
{
  static TModuleProc3<BOOL, HANDLE,DWORD,PIMAGEHLP_MODULE>
         symGetModuleInfo(GetModule(), SymGetModuleInfoStr);
  return symGetModuleInfo(p1,p2,p3);
}
//-----------------------------------------------------------------------------
BOOL TImageHelp::SymLoadModule(HANDLE p1, HANDLE p2, PSTR p3,PSTR p4, DWORD p5, 
                               DWORD p6)
{
  static TModuleProc6<BOOL,HANDLE,HANDLE,PSTR,PSTR,DWORD, DWORD>
         symLoadModule(GetModule(), SymLoadModuleStr);
  return symLoadModule(p1,p2,p3,p4,p5,p6);
}
//-----------------------------------------------------------------------------
BOOL TImageHelp::SymGetSymFromAddr(HANDLE p1, DWORD p2, PDWORD p3, 
                                   PIMAGEHLP_SYMBOL p4)
{
  static TModuleProc4<BOOL,HANDLE,DWORD,PDWORD, PIMAGEHLP_SYMBOL>
         symGetSymFromAddr(GetModule(), SymGetSymFromAddrStr);
  return symGetSymFromAddr(p1,p2,p3,p4);
}
//-----------------------------------------------------------------------------
DWORD TImageHelp::UnDecorateSymbolName(LPCSTR p1, LPSTR p2, DWORD p3, DWORD p4)
{
  static TModuleProc4<DWORD,LPCSTR,LPSTR,DWORD,DWORD>
         unDecorateSymbolName(GetModule(), UnDecorateSymbolNameStr);
  return unDecorateSymbolName(p1,p2,p3,p4);
}
//-----------------------------------------------------------------------------
BOOL TImageHelp::SymUnDName(PIMAGEHLP_SYMBOL p1, LPSTR p2, DWORD p3)
{
  static TModuleProc3<BOOL,PIMAGEHLP_SYMBOL,LPSTR,DWORD>
         symUnDName(GetModule(), SymUnDNameStr);
  return symUnDName(p1,p2,p3);
}
//-----------------------------------------------------------------------------
BOOL TImageHelp::SymInitialize(HANDLE p1, LPSTR p2, BOOL p3)
{
  static TModuleProc3<BOOL,HANDLE,LPSTR,BOOL>
         symInitialize(GetModule(), SymInitializeStr);
  return symInitialize(p1,p2,p3);
}
//-----------------------------------------------------------------------------
DWORD TImageHelp::SymGetOptions()
{
  static TModuleProc0<DWORD>
         symGetOptions(GetModule(), SymGetOptionsStr);
  return symGetOptions();
}
//-----------------------------------------------------------------------------
DWORD TImageHelp::SymSetOptions(DWORD p1)
{
  static TModuleProc1<DWORD,DWORD>
         symSetOptions(GetModule(), SymSetOptionsStr);
  return symSetOptions(p1);
}
//-----------------------------------------------------------------------------
BOOL TImageHelp::StackWalk(DWORD p1, HANDLE p2, HANDLE p3, LPSTACKFRAME p4, 
                           LPVOID p5, PREAD_PROCESS_MEMORY_ROUTINE p6, 
                           PFUNCTION_TABLE_ACCESS_ROUTINE p7, 
                           PGET_MODULE_BASE_ROUTINE p8, 
                           PTRANSLATE_ADDRESS_ROUTINE p9)
{
  static TModuleProc9<BOOL,DWORD,HANDLE,HANDLE,LPSTACKFRAME,LPVOID,
                      PREAD_PROCESS_MEMORY_ROUTINE,PFUNCTION_TABLE_ACCESS_ROUTINE, 
                      PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE>
         stackWalk(GetModule(), StackWalkStr);
  return stackWalk(p1,p2,p3,p4,p5,p6,p7,p8,p9);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// Routine to produce stack dump
#if !defined(UNIX) //JJH we do not need this stuff in unix
static LPVOID __stdcall FunctionTableAccess(HANDLE hProcess, DWORD dwPCAddress);
static DWORD __stdcall GetModuleBase(HANDLE hProcess, DWORD dwReturnAddress);
#endif ///defined(UNIX)

#define MODULE_NAME_LEN 64
#define SYMBOL_NAME_LEN 128

#if defined(_DEF_DECLARED)

#define TRACE_STACK(m)\
  do{\
    if(__OwlDiagGroupDef.IsEnabled()){\
      tostringstream out; out << m;\
      __OwlDiagGroupDef.Trace(out.str().c_str(), 0, __FILE__, __LINE__);\
    }\
  }while(0)

#else //no _DEF_DECLARED
  #define TRACE_STACK(m) 
#endif //_DEF_DECLARED

/// \cond
struct OWL_SYMBOL_INFO{
  DWORD dwAddress;
  DWORD dwOffset;
  CHAR  szModule[MODULE_NAME_LEN];
  CHAR  szSymbol[SYMBOL_NAME_LEN];
};
/// \endcond

#if !defined(UNIX) //JJH see OwlDumpStack below...
static LPVOID __stdcall FunctionTableAccess(HANDLE hProcess, DWORD dwPCAddress)
{
  return TImageHelp::SymFunctionTableAccess(hProcess, dwPCAddress);
}

static DWORD __stdcall GetModuleBase(HANDLE hProcess, DWORD dwReturnAddress)
{
  IMAGEHLP_MODULE moduleInfo;

  if (TImageHelp::SymGetModuleInfo(hProcess, dwReturnAddress, &moduleInfo))
    return moduleInfo.BaseOfImage;
  else{
    MEMORY_BASIC_INFORMATION memoryBasicInfo;

    if (::VirtualQueryEx(hProcess, (LPVOID) dwReturnAddress, 
        &memoryBasicInfo, sizeof(memoryBasicInfo))){

      char szFile[MAX_PATH] = { 0 };
      DWORD cch = GetModuleFileNameA((HINSTANCE)memoryBasicInfo.AllocationBase,
                                    szFile, MAX_PATH);

      // Ignore the return code since we can't do anything with it.
      if(!TImageHelp::SymLoadModule(hProcess,NULL, ((cch) ? szFile : NULL),
                       NULL, (DWORD) memoryBasicInfo.AllocationBase, 0)){
        TRACE_STACK(_T("Error: ") << GetLastError());
      }
      return (DWORD) memoryBasicInfo.AllocationBase;
    }
    else{
      TRACE_STACK(_T("Error is ") << GetLastError());
    }
  }
  return 0;
}

static BOOL ResolveSymbol(HANDLE hProcess, DWORD dwAddress, OWL_SYMBOL_INFO& siSymbol)
{
#if defined(BI_COMP_GNUC)
  return FALSE;
#else
  BOOL fRetval = TRUE;

  siSymbol.dwAddress = dwAddress;

  union __SYMB{
    CHAR rgchSymbol[sizeof(IMAGEHLP_SYMBOL) + 255];
    IMAGEHLP_SYMBOL  sym;
  } u;

  CHAR szUndec[256];
  CHAR szWithOffset[256];
  LPSTR pszSymbol;
  IMAGEHLP_MODULE mi;

  memset(&siSymbol, 0, sizeof(OWL_SYMBOL_INFO));
  mi.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

  if (!TImageHelp::SymGetModuleInfo(hProcess, dwAddress, &mi))
    lstrcpyA(siSymbol.szModule, "<no module>");
  else{
    LPSTR pszModule = strchr(mi.ImageName, '\\');
    if (pszModule == NULL)
      pszModule = mi.ImageName;
    else
      pszModule++;

    lstrcpynA(siSymbol.szModule, pszModule, COUNTOF(siSymbol.szModule));
    lstrcatA(siSymbol.szModule, "! ");
  }

#if defined(BI_COMP_BORLANDC) || defined(WINELIB) //JJH added WINELIB
  try{
#else
  __try{
#endif
    u.sym.SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    u.sym.Address        = dwAddress;
    u.sym.MaxNameLength = 255;

    if (TImageHelp::SymGetSymFromAddr(hProcess, dwAddress, 
                                      &(siSymbol.dwOffset), &u.sym)){
      pszSymbol = u.sym.Name;
      if (TImageHelp::UnDecorateSymbolName(u.sym.Name, szUndec, COUNTOF(szUndec),
                      UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS)){
        pszSymbol = szUndec;
      }
      else if (TImageHelp::SymUnDName(&u.sym, szUndec, COUNTOF(szUndec))){
        pszSymbol = szUndec;
      }

      if(siSymbol.dwOffset != 0){
        wsprintfA(szWithOffset, "%s + %d bytes", pszSymbol, siSymbol.dwOffset);
        pszSymbol = szWithOffset;
      }
    }
    else
      pszSymbol = "<no symbol>";
  }
//JJH
#if defined (WINELIB)
  catch(...) {
#else
    //JBC EXCEPTION_EXECUTE_HANDLER == 1
  //__except(EXCEPTION_EXECUTE_HANDLER){
  __except(true) {
#endif
    pszSymbol = "<EX: no symbol>";
    siSymbol.dwOffset = dwAddress - mi.BaseOfImage;
  }
  __endexcept


  lstrcpynA(siSymbol.szSymbol, pszSymbol, COUNTOF(siSymbol.szSymbol));
  return fRetval;
#endif
}

#endif //if !defined(UNIX)

/// \cond
class __TTraceClipboardData{
  public:
    __TTraceClipboardData(uint32 target);
    ~__TTraceClipboardData();
    void SendOut(LPCSTR pszData);
  private:
    HGLOBAL  Memory;
    uint32  Size;
    uint32  Used;
    uint32  Target;

};
/// \endcond

__TTraceClipboardData::__TTraceClipboardData(uint32 target)
: 
  Memory(0)
  ,Size(0)
  ,Used(0)
  ,Target(target)
{
}

__TTraceClipboardData::~__TTraceClipboardData()
{
  if (Memory){
    // chuck it onto the clipboard
    // don't free it unless there's an error

    if(!OpenClipboard(NULL))
      GlobalFree(Memory);
    else if (!EmptyClipboard() || SetClipboardData(CF_TEXT, Memory) == NULL){
      GlobalFree(Memory);
    }
    else
      CloseClipboard();
  }
}

void __TTraceClipboardData::SendOut(LPCSTR pszData)
{
  int nLength;
  if (pszData == NULL || (nLength = lstrlenA(pszData)) == 0)
    return;

  // send it to TRACE (can be redirected)
  if (Target & OWL_STACK_DUMP_TARGET_TRACE){
    TRACE_STACK(pszData);
  }

  // send it to OutputDebugString() (can't redirect)
  if (Target & OWL_STACK_DUMP_TARGET_ODS)
    OutputDebugStringA(pszData);

  // build a buffer for the clipboard
  if (Target & OWL_STACK_DUMP_TARGET_CLIPBOARD){
    if(!Memory){
       Memory = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, 1024);
      if(!Memory){
        //TRACE("OwlDumpStack Error: No memory available for clipboard");
        Target &= ~OWL_STACK_DUMP_TARGET_CLIPBOARD;
      }
      else{
        Used = nLength;
        Size = 1024;
        LPSTR pstr = (LPSTR)GlobalLock(Memory);
        if (pstr){
          lstrcpyA(pstr, pszData);
          GlobalUnlock(Memory);
        }
        else{
          //TRACE("OwlDumpStack Error: Couldn't lock memory!");
          GlobalFree(Memory);
          Memory = 0;
          Target &= ~OWL_STACK_DUMP_TARGET_CLIPBOARD;
        }
      }
    }
    else{
      if ((Used + nLength + 1) >= Size){
        // grow by leaps and bounds
        Size *= 2;
        if(Size > (1024L*1024L)){
          //TRACE("OwlDumpStack Error: more than one megabyte on clipboard.");
          Target &= ~OWL_STACK_DUMP_TARGET_CLIPBOARD;
        }

        HGLOBAL hMemory = GlobalReAlloc(Memory, Size, GMEM_MOVEABLE);
        if (!hMemory){
          //TRACE(_T("OwlDumpStack Error: Couldn't get ") << Size << _T("bytes!"));
          Target &= ~OWL_STACK_DUMP_TARGET_CLIPBOARD;
        }
        else
          Memory = hMemory;
      }

      LPSTR pstr = (LPSTR) GlobalLock(Memory);
      if (pstr){
        lstrcpyA(pstr + Used, pszData);
        Used += nLength;
        GlobalUnlock(Memory);
      }
      else{
        //TRACE("OwlDumpStack Error: Couldn't lock memory!");
        Target &= ~OWL_STACK_DUMP_TARGET_CLIPBOARD;
      }
    }
  }
  return;
}

/////////////////////////////////////////////////////////////////////////////
// OwlDumpStack API
_OWLFUNC(void)
OwlDumpStack(uint32 target /* = OWL_STACK_DUMP_TARGET_DEFAULT */)
{
#if !defined(UNIX)

  __TTraceClipboardData clipboardData(target);

  clipboardData.SendOut("=== begin OwlDumpStack output ===\r\n");

  TUint32Array adwAddress;
  HANDLE hProcess = ::GetCurrentProcess();
  if (TImageHelp::SymInitialize(hProcess, NULL, FALSE)){
    // force undecorated names to get params
    DWORD dw = TImageHelp::SymGetOptions();
    dw &= ~SYMOPT_UNDNAME;
    TImageHelp::SymSetOptions(dw);

    HANDLE hThread = ::GetCurrentThread();
    CONTEXT threadContext;

    threadContext.ContextFlags = CONTEXT_FULL;

    if (::GetThreadContext(hThread, &threadContext))
    {
      STACKFRAME stackFrame;
      memset(&stackFrame, 0, sizeof(stackFrame));
      stackFrame.AddrPC.Mode = AddrModeFlat;

      DWORD dwMachType;

#if defined(_M_IX86)
      dwMachType                  = IMAGE_FILE_MACHINE_I386;

      // program counter, stack pointer, and frame pointer
      stackFrame.AddrPC.Offset    = threadContext.Eip;
      stackFrame.AddrStack.Offset = threadContext.Esp;
      stackFrame.AddrStack.Mode   = AddrModeFlat;
      stackFrame.AddrFrame.Offset = threadContext.Ebp;
      stackFrame.AddrFrame.Mode   = AddrModeFlat;
#elif defined(_M_MRX000)
      // only program counter
      dwMachType                  = IMAGE_FILE_MACHINE_R4000;
      stackFrame.AddrPC. Offset    = treadContext.Fir;
#elif defined(_M_ALPHA)
      // only program counter
      dwMachType                  = IMAGE_FILE_MACHINE_ALPHA;
      stackFrame.AddrPC.Offset    = (unsigned long) threadContext.Fir;
#elif defined(_M_PPC)
      // only program counter
      dwMachType                  = IMAGE_FILE_MACHINE_POWERPC;
      stackFrame.AddrPC.Offset    = threadContext.Iar;
#else
#error Unknown Target Machine
#endif

      for (int nFrame = 0; nFrame < 1024; nFrame++){
        if (!TImageHelp::StackWalk(dwMachType, hProcess, hProcess,&stackFrame, 
             &threadContext, NULL,FunctionTableAccess, GetModuleBase, NULL)){
          break;
        }
        adwAddress.AddAt(stackFrame.AddrPC.Offset, nFrame);
      }
    }
  }
  else{
    DWORD dw = GetLastError();
    char sz[100];
    wsprintfA(sz,
      "OwlDumpStack Error: IMAGEHLP.DLL wasn't found. "
      "GetLastError() returned 0x%8.8X\r\n", dw);
    clipboardData.SendOut(sz);
  }

  // dump it out now
  int nAddress;
  int cAddresses = adwAddress.Size();
  for (nAddress = 0; nAddress < cAddresses; nAddress++){
    OWL_SYMBOL_INFO info;
    DWORD dwAddress = adwAddress[nAddress];

    char sz[20];
    wsprintfA(sz, "%8.8X: ", dwAddress);
    clipboardData.SendOut(sz);

    if (ResolveSymbol(hProcess, dwAddress, info)){
      clipboardData.SendOut(info.szModule);
      clipboardData.SendOut(info.szSymbol);
    }
    else
      clipboardData.SendOut("symbol not found");
    clipboardData.SendOut("\r\n");
  }

  clipboardData.SendOut("=== end OwlDumpStack() output ===\r\n");
#endif //(ifdef UNIX)
}
#  if defined(BI_COMP_BORLANDC)
#    pragma warn .inl
#  endif


} // OWL namespace
/* ========================================================================== */

