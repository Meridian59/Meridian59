/*
 * ctl_strings.cpp:  user32/kernel32's Unicode string functions (lstr*W, Char*W,
 *   IsChar*W).  Case mapping and classification are Windows' for Latin-1 and
 *   Latin Extended-A (what code page 1252 reaches), whatever the C locale.
 */

#include "core/win32.h"

#include <cwctype>

namespace w32 {

namespace {

bool InLatin1Letters(WCHAR c)
{
   return c >= 0xC0 && c <= 0xFF && c != 0xD7 && c != 0xF7;
}

}  // namespace

WCHAR WideUpper(WCHAR c)
{
   if (c < 0x80)
      return (c >= 'a' && c <= 'z') ? (WCHAR) (c - 32) : c;
   if (c == 0xB5)
      return 0x39C;
   if (c == 0xFF)
      return 0x178;
   if (c >= 0xE0 && c <= 0xFE && c != 0xF7)
      return (WCHAR) (c - 32);
   if (c < 0x100)
      return c;
   if (c == 0x131)
      return 'I';
   if (c == 0x17F)
      return 'S';
   // Latin Extended-A: pairs, upper case first, shifted by one in 0x139-0x148 and
   // 0x179-0x17E.
   if ((c >= 0x100 && c <= 0x137) || (c >= 0x14A && c <= 0x177))
      return (c & 1) ? (WCHAR) (c - 1) : c;
   if ((c >= 0x139 && c <= 0x148) || (c >= 0x179 && c <= 0x17E))
      return (c & 1) ? c : (WCHAR) (c - 1);
   return (WCHAR) towupper(c);
}

WCHAR WideLower(WCHAR c)
{
   if (c < 0x80)
      return (c >= 'A' && c <= 'Z') ? (WCHAR) (c + 32) : c;
   if (c >= 0xC0 && c <= 0xDE && c != 0xD7)
      return (WCHAR) (c + 32);
   if (c < 0x100)
      return c;
   if (c == 0x130)
      return 'i';
   if (c == 0x178)
      return 0xFF;
   if ((c >= 0x100 && c <= 0x137) || (c >= 0x14A && c <= 0x177))
      return (c & 1) ? c : (WCHAR) (c + 1);
   if ((c >= 0x139 && c <= 0x148) || (c >= 0x179 && c <= 0x17E))
      return (c & 1) ? (WCHAR) (c + 1) : c;
   return (WCHAR) towlower(c);
}

bool WideIsAlpha(WCHAR c)
{
   if (c < 0x80)
      return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
   if (c == 0xAA || c == 0xB5 || c == 0xBA || InLatin1Letters(c))
      return true;
   if (c < 0x100)
      return false;
   if (c <= 0x24F)
      return true;
   return iswalpha(c) != 0;
}

}  // namespace w32

using namespace w32;

/************************************************************************/
// lstr*

INT WINAPI lstrlenW(LPCWSTR s)
{
   if (s == nullptr)
      return 0;
   INT n = 0;
   while (s[n])
      n++;
   return n;
}

LPWSTR WINAPI lstrcpyW(LPWSTR dst, LPCWSTR src)
{
   if (dst == nullptr || src == nullptr)
      return nullptr;
   LPWSTR d = dst;
   while ((*d++ = *src++))
      ;
   return dst;
}

LPWSTR WINAPI lstrcpynW(LPWSTR dst, LPCWSTR src, INT n)
{
   // Copies at most n - 1 characters and always terminates (n > 0).
   if (dst == nullptr || n <= 0)
      return dst;
   INT i = 0;
   if (src)
      for (; i < n - 1 && src[i]; i++)
         dst[i] = src[i];
   dst[i] = 0;
   return dst;
}

LPWSTR WINAPI lstrcatW(LPWSTR dst, LPCWSTR src)
{
   if (dst == nullptr || src == nullptr)
      return nullptr;
   lstrcpyW(dst + lstrlenW(dst), src);
   return dst;
}

INT WINAPI lstrcmpW(LPCWSTR a, LPCWSTR b)
{
   return CompareStringW(LOCALE_USER_DEFAULT, 0, a ? a : u"", -1, b ? b : u"", -1) - CSTR_EQUAL;
}

INT WINAPI lstrcmpiW(LPCWSTR a, LPCWSTR b)
{
   return CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, a ? a : u"", -1, b ? b : u"", -1) - CSTR_EQUAL;
}

/************************************************************************/
// Char*

// A pointer argument whose high word is zero is one character (CharUpper's "single
// character" form).
static bool IsSingleChar(const void *p)
{
   return ((UINT_PTR) p >> 16) == 0;
}

LPWSTR WINAPI CharUpperW(LPWSTR s)
{
   if (IsSingleChar(s))
      return (LPWSTR) (UINT_PTR) WideUpper((WCHAR) (UINT_PTR) s);
   for (LPWSTR p = s; *p; p++)
      *p = WideUpper(*p);
   return s;
}

LPWSTR WINAPI CharLowerW(LPWSTR s)
{
   if (IsSingleChar(s))
      return (LPWSTR) (UINT_PTR) WideLower((WCHAR) (UINT_PTR) s);
   for (LPWSTR p = s; *p; p++)
      *p = WideLower(*p);
   return s;
}

DWORD WINAPI CharUpperBuffW(LPWSTR s, DWORD length)
{
   if (s == nullptr)
      return 0;
   for (DWORD i = 0; i < length; i++)
      s[i] = WideUpper(s[i]);
   return length;
}

DWORD WINAPI CharLowerBuffW(LPWSTR s, DWORD length)
{
   if (s == nullptr)
      return 0;
   for (DWORD i = 0; i < length; i++)
      s[i] = WideLower(s[i]);
   return length;
}

DWORD WINAPI CharUpperBuffA(LPSTR s, DWORD length)
{
   if (s == nullptr)
      return 0;
   for (DWORD i = 0; i < length; i++)
   {
      WCHAR w;
      MultiByteToWideChar(CP_ACP, 0, &s[i], 1, &w, 1);
      w = WideUpper(w);
      WideCharToMultiByte(CP_ACP, 0, &w, 1, &s[i], 1, nullptr, nullptr);
   }
   return length;
}

DWORD WINAPI CharLowerBuffA(LPSTR s, DWORD length)
{
   if (s == nullptr)
      return 0;
   for (DWORD i = 0; i < length; i++)
   {
      WCHAR w;
      MultiByteToWideChar(CP_ACP, 0, &s[i], 1, &w, 1);
      w = WideLower(w);
      WideCharToMultiByte(CP_ACP, 0, &w, 1, &s[i], 1, nullptr, nullptr);
   }
   return length;
}

LPWSTR WINAPI CharNextW(LPCWSTR s)
{
   return (LPWSTR) (*s ? s + 1 : s);
}

LPWSTR WINAPI CharPrevW(LPCWSTR start, LPCWSTR s)
{
   return (LPWSTR) (s > start ? s - 1 : start);
}

LPSTR WINAPI CharPrevA(LPCSTR start, LPCSTR s)
{
   // Code page 1252 has one byte per character.
   return (LPSTR) (s > start ? s - 1 : start);
}

BOOL WINAPI IsCharAlphaW(WCHAR c)
{
   return WideIsAlpha(c);
}

BOOL WINAPI IsCharAlphaNumericW(WCHAR c)
{
   return WideIsAlpha(c) || (c >= '0' && c <= '9') || (c >= 0x100 && iswdigit(c));
}

BOOL WINAPI IsCharUpperW(WCHAR c)
{
   return WideIsAlpha(c) && WideLower(c) != c;
}

BOOL WINAPI IsCharLowerW(WCHAR c)
{
   return WideIsAlpha(c) && WideUpper(c) != c;
}

static WCHAR ToWide(CHAR c)
{
   WCHAR w = 0;
   MultiByteToWideChar(CP_ACP, 0, &c, 1, &w, 1);
   return w;
}

BOOL WINAPI IsCharUpperA(CHAR c)
{
   return IsCharUpperW(ToWide(c));
}

BOOL WINAPI IsCharLowerA(CHAR c)
{
   return IsCharLowerW(ToWide(c));
}
