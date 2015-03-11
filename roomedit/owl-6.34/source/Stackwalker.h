/*////////////////////////////////////////////////////////////////////////////
 *  Project:
 *    Memory_and_Exception_Trace
 *
 * ///////////////////////////////////////////////////////////////////////////
 *  File:
 *    Stackwalker.h
 *
 *  Remarks:
 *
 *
 *  Note:
 *
 *
 *  Author:
 *    Jochen Kalmbach
 *
 *////////////////////////////////////////////////////////////////////////////

#ifndef __STACKWALKER_H__
#define __STACKWALKER_H__

// Only valid in the following environment: Intel platform, MS VC++ 5/6/7
#ifndef _M_IX86
#error Only INTEL envirnoments are supported!
#endif

// Only MS VC++ 6 to 10
#if (_MSC_VER < 1200) || (_MSC_VER > 1600)
#error Only MS VC++ 6/7/8/9/10 supported. Check if the '_CrtMemBlockHeader' has not changed with this compiler!
#endif

// Make extern "C", so it will also work with normal C-Programs

#ifdef __cplusplus
extern "C" {
#endif

enum eAllocCheckOutput
{
  ACOutput_Simple,
  ACOutput_Advanced,
  ACOutput_XML
};

extern int InitAllocCheckWN(eAllocCheckOutput eOutput, LPCTSTR pszFilename, ULONG ulShowStackAtAlloc = 0);
extern int InitAllocCheck(eAllocCheckOutput eOutput = ACOutput_Simple, BOOL bSetUnhandledExeptionFilter = TRUE, ULONG ulShowStackAtAlloc = 0);  // will create the filename by itself
extern ULONG DeInitAllocCheck();
extern DWORD StackwalkFilter(EXCEPTION_POINTERS *ep, DWORD status, LPCTSTR pszLogFile);

#ifdef __cplusplus
}
#endif

#endif  // __STACKWALKER_H__
