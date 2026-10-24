/*
 * wine_prelude.h:  Included first in Wine's sources.  They are Windows DLL code
 *   built with 16-bit wchar_t (-fshort-wchar): their wide C runtime calls go to
 *   16-bit versions in the layer, not the C library's 32-bit ones.
 */

#ifndef M59_WINE_PRELUDE_H
#define M59_WINE_PRELUDE_H

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define wcslen m59_wcslen
#define wcscmp m59_wcscmp
#define wcsncmp m59_wcsncmp
#define wcsicmp m59_wcsicmp
#define _wcsicmp m59_wcsicmp
#define wcsnicmp m59_wcsnicmp
#define _wcsnicmp m59_wcsnicmp
#define wcschr m59_wcschr
#define wcsrchr m59_wcsrchr
#define wcsstr m59_wcsstr
#define wcscpy m59_wcscpy
#define wcsncpy m59_wcsncpy
#define wcscat m59_wcscat
#define wcsdup m59_wcsdup
#define _wcsdup m59_wcsdup
#define wcstol m59_wcstol
#define wcstoul m59_wcstoul
#define _wtoi m59_wtoi
#define swprintf m59_swprintf
#define _snwprintf m59_snwprintf
#define vswprintf m59_vswprintf
#define towupper m59_towupper
#define towlower m59_towlower
#define iswalnum m59_iswalnum
#define iswalpha m59_iswalpha
#define iswdigit m59_iswdigit
#define iswspace m59_iswspace
#define iswupper m59_iswupper
#define iswlower m59_iswlower
#define iswpunct m59_iswpunct

/* Wine's headers inline GetLastError, SetLastError, GetCurrentThreadId,
 * GetProcessHeap ... as reads of NtCurrentTeb(): calls go to the layer's
 * per-thread block (core/wine_teb.cpp); the headers' own definition,
 * NtCurrentTeb(void), becomes an unused m59_unused_NtCurrentTeb. */
#define M59_TEB_CAT(a, b) a##b
#define NtCurrentTeb(...) M59_TEB_CAT(M59_NtCurrentTeb_, __VA_ARGS__)
#define M59_NtCurrentTeb_void m59_unused_NtCurrentTeb(void)
#define M59_NtCurrentTeb_ m59_NtCurrentTeb()
struct _TEB *m59_NtCurrentTeb(void);

typedef unsigned short m59_wchar;

size_t m59_wcslen(const m59_wchar *s);
int m59_wcscmp(const m59_wchar *a, const m59_wchar *b);
int m59_wcsncmp(const m59_wchar *a, const m59_wchar *b, size_t n);
int m59_wcsicmp(const m59_wchar *a, const m59_wchar *b);
int m59_wcsnicmp(const m59_wchar *a, const m59_wchar *b, size_t n);
m59_wchar *m59_wcschr(const m59_wchar *s, m59_wchar c);
m59_wchar *m59_wcsrchr(const m59_wchar *s, m59_wchar c);
m59_wchar *m59_wcsstr(const m59_wchar *s, const m59_wchar *find);
m59_wchar *m59_wcscpy(m59_wchar *dst, const m59_wchar *src);
m59_wchar *m59_wcsncpy(m59_wchar *dst, const m59_wchar *src, size_t n);
m59_wchar *m59_wcscat(m59_wchar *dst, const m59_wchar *src);
m59_wchar *m59_wcsdup(const m59_wchar *s);
long m59_wcstol(const m59_wchar *s, m59_wchar **end, int base);
unsigned long m59_wcstoul(const m59_wchar *s, m59_wchar **end, int base);
int m59_wtoi(const m59_wchar *s);
// Windows' swprintf: (buffer, count, format, ...), %s a wide string.
int m59_swprintf(m59_wchar *buffer, size_t count, const m59_wchar *format, ...);
int m59_snwprintf(m59_wchar *buffer, size_t count, const m59_wchar *format, ...);
int m59_vswprintf(m59_wchar *buffer, size_t count, const m59_wchar *format, va_list args);
m59_wchar m59_towupper(m59_wchar c);
m59_wchar m59_towlower(m59_wchar c);
int m59_iswalnum(m59_wchar c);
int m59_iswalpha(m59_wchar c);
int m59_iswdigit(m59_wchar c);
int m59_iswspace(m59_wchar c);
int m59_iswupper(m59_wchar c);
int m59_iswlower(m59_wchar c);
int m59_iswpunct(m59_wchar c);

#ifdef __cplusplus
}
#endif

/* Wine's winbase.h gives its own sources (__WINESRC__) inline versions of these
   that read the thread's TEB through %gs, which isn't Windows' TEB here (there
   is none).  The inline ones are renamed out of the way while winbase.h is read
   first; the sources call the layer's real ones. */
#define GetCurrentProcessId m59_teb_GetCurrentProcessId
#define GetCurrentThreadId m59_teb_GetCurrentThreadId
#define GetLastError m59_teb_GetLastError
#define GetProcessHeap m59_teb_GetProcessHeap
#define SetLastError m59_teb_SetLastError
#include <windef.h>
#include <winbase.h>
#undef GetCurrentProcessId
#undef GetCurrentThreadId
#undef GetLastError
#undef GetProcessHeap
#undef SetLastError
WINBASEAPI DWORD WINAPI GetCurrentProcessId(void);
WINBASEAPI DWORD WINAPI GetCurrentThreadId(void);
WINBASEAPI DWORD WINAPI GetLastError(void);
WINBASEAPI HANDLE WINAPI GetProcessHeap(void);
WINBASEAPI VOID WINAPI SetLastError(DWORD);

#endif
