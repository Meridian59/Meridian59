/* tchar.h:  The ANSI (non-Unicode) build's text macros. */
#ifndef M59_TCHAR_H
#define M59_TCHAR_H
#define _T(x)    x
#define _TEXT(x) x
#define _tcscpy  strcpy
#define _tcslen  strlen
#define _tcscmp  strcmp
#endif
