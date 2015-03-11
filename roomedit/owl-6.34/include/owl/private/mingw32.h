//----------------------------------------------------------------------------
// ObjectWindows
// OWL NExt
//
// Created by Kenneth Haley ( khaley@bigfoot.com ) 
//
//----------------------------------------------------------------------------

#if !defined(OWL_PRIVATE_MINGW32_H)
#define OWL_PRIVATE_MINGW32_H

#ifndef _WIN32_IE
# define _WIN32_IE 0x0400
#endif

#ifdef  _UNICODE
// define before GNU headers currently tchar.h not complete
//#  ifndef _TCHAR_DEFINED
    typedef wchar_t     TCHAR;
    typedef wchar_t     _TCHAR;
    typedef wchar_t     _TSCHAR;
    typedef wchar_t     _TUCHAR;
    typedef wchar_t     _TXCHAR;
    typedef wint_t      _TINT;
#    define _TCHAR_DEFINED
//#  endif

#  define __T(x)      L ## x
//#  define _T(x)       __T(x)

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


#else  // _UNICODE

//#  ifndef _TCHAR_DEFINED
    typedef char            TCHAR;
    typedef char            _TCHAR;
    typedef signed char     _TSCHAR;
    typedef unsigned char   _TUCHAR;
    typedef unsigned char   _TXCHAR;
    typedef unsigned int    _TINT;
#    define _TCHAR_DEFINED
//#  endif

#  define __T(x)      x
//#  define _T(x)       __T(x)

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
#define _ltot _ltoa
#define _ttoi atoi
//#define _itot itoa  // Y.B. gcc 2.5.2
#define _tfopen fopen
#define _ttol atol
#define _tfullpath _fullpath
#define _tWinMain WinMain
#define _tmain main
#endif

#endif // OWL_PRIVATE_MINGW32_H
