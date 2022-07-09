// owlpch.cpp : source file that includes just the standard includes
//  again .pch will be the pre-compiled header
//  owlpch.obj will contain the pre-compiled type information

#include <coolprj/pch.h>
#pragma hdrstop
/*
#ifdef _DLL
// The following symbol used to force inclusion of this module for _OCFDLL
#if defined(_M_IX86) || defined(_MAC)
extern "C" { int _afxForceSTDAFX; }
#else
extern "C" { int __afxForceSTDAFX; }
#endif
#endif
*/
/*
void __Trace(LPCTSTR lpszFormat, ...)
{
  TCHAR szBuffer[2048];
  wvsprintf(szBuffer, lpszFormat, (_TCHAR*)(&lpszFormat+1));

  HANDLE hFile =::CreateFile("c:\\cooledit.log", GENERIC_WRITE, 0, NULL,
                               OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile != INVALID_HANDLE_VALUE){
    SetFilePointer(hFile,0,0,FILE_END);
    DWORD num;
    WriteFile(hFile, szBuffer, strlen(szBuffer),&num,NULL);
    CloseHandle(hFile);
  }
}
*/