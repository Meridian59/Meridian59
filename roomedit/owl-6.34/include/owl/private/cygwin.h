//----------------------------------------------------------------------------
// ObjectWindows
// OWL NExt
//
// Created by Kenneth Haley ( khaley@bigfoot.com ) 
//
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_CYGWIN_H)
#define OWL_PRIVATE_CYGWIN_H

#ifdef  _UNICODE

// define before GNU headers currently tchar.h not complete
#  ifndef __TCHAR_DEFINED
    typedef wchar_t     _TCHAR;
    typedef wchar_t     _TSCHAR;
    typedef wchar_t     _TUCHAR;
    typedef wchar_t     _TXCHAR;
    typedef wint_t      _TINT;
#    define __TCHAR_DEFINED
#  endif

#  define __T(x)      L ## x
#  define _T(x)       __T(x)

#  define AnsiToOem CharToOemW
#  define OemToAnsi OemToCharW
#  define AnsiToOemBuff CharToOemBuffW
#  define OemToAnsiBuff OemToCharBuffW
#  define AnsiUpper CharUpperW
#  define AnsiUpperBuff CharUpperBuffW
#  define AnsiLower CharLowerW
#  define AnsiLowerBuff CharLowerBuffW
#  define AnsiNext CharNextW
#  define AnsiPrev CharPrevW
#error Various wide function not defined

#else // _UNICODE

#  ifndef __TCHAR_DEFINED
    typedef char            _TCHAR;
    typedef signed char     _TSCHAR;
    typedef unsigned char   _TUCHAR;
    typedef unsigned char   _TXCHAR;
    typedef unsigned int    _TINT;
#    define __TCHAR_DEFINED
#  endif

#  define __T(x)      x
#  define _T(x)       __T(x)

#  define AnsiToOem CharToOemA
#  define OemToAnsi OemToCharA
#  define AnsiToOemBuff CharToOemBuffA
#  define OemToAnsiBuff OemToCharBuffA
#  define AnsiUpper CharUpperA
#  define AnsiUpperBuff CharUpperBuffA
#  define AnsiLower CharLowerA
#  define AnsiLowerBuff CharLowerBuffA
#  define AnsiNext CharNextA
#  define AnsiPrev CharPrevA
#define _ltot my_itoa
#define _ttoi atoi
#define _ttol atol
#define _itot my_itoa
#define _tcstol strtol
#define _tcstod strtod
#define _tfopen fopen
#define _istdigit isdigit
#define _istalpha isalpha
#define _istalnum isalnum
#define _totupper toupper
#define _tcslen strlen
#define _tcscmp strcmp
#define _tcsicmp stricmp
#define _tcsnicmp strnicmp
#define _tcsncmp strncmp
#define _tcscpy strcpy
#define _tcsncpy strncpy
#define _tcsspn strspn
#define _tcscspn strcspn
#define _tcscat strcat
#define _tcstok strtok
#define _tcschr strchr
#define _tcsrchr strrchr
#define _stscanf sscanf
#define _stprintf sprintf
#define _fputts fputs
#define _tfullpath myfullpath

#define _tWinMain WinMain
#define _tmain main

#endif // _UNICODE

#define _MAX_PATH 260
#define MAX_PATH _MAX_PATH
char *my_itoa(int,char*,int);
char *myfullpath(char *,const char *,int );

wchar_t* __stdcall wcscpy(wchar_t* dst, const wchar_t* src);
size_t __stdcall wcslen(const wchar_t* str);

#endif // OWL_PRIVATE_CYGWIN_H
