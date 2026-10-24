/*
 * strings.cpp:  Text encodings (the client's text is Windows-1252, the ANSI code
 *   page) and the string functions of kernel32/user32.
 */

#include "core/gdi.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace w32 {

namespace {

// Windows-1252 0x80-0x9F; the rest of 0x00-0xFF is Latin-1.
const char16_t kHigh[32] = {
   0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160,
   0x2039, 0x0152, 0x008D, 0x017D, 0x008F, 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022,
   0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178};

char32_t AnsiToCodepoint(unsigned char c)
{
   return (c >= 0x80 && c < 0xA0) ? kHigh[c - 0x80] : c;
}

int CodepointToAnsi(char32_t cp)
{
   if (cp < 0x80 || (cp >= 0xA0 && cp <= 0xFF))
      return (int) cp;
   for (int i = 0; i < 32; i++)
      if (kHigh[i] == cp)
         return 0x80 + i;
   return -1;
}

void AppendUtf8(std::string &out, char32_t cp)
{
   if (cp < 0x80)
      out += (char) cp;
   else if (cp < 0x800)
   {
      out += (char) (0xC0 | (cp >> 6));
      out += (char) (0x80 | (cp & 0x3F));
   }
   else if (cp < 0x10000)
   {
      out += (char) (0xE0 | (cp >> 12));
      out += (char) (0x80 | ((cp >> 6) & 0x3F));
      out += (char) (0x80 | (cp & 0x3F));
   }
   else
   {
      out += (char) (0xF0 | (cp >> 18));
      out += (char) (0x80 | ((cp >> 12) & 0x3F));
      out += (char) (0x80 | ((cp >> 6) & 0x3F));
      out += (char) (0x80 | (cp & 0x3F));
   }
}

// Next code point of utf8 from i (advanced); U+FFFD for malformed input.
char32_t NextCodepoint(const std::string &s, size_t &i)
{
   unsigned char c = (unsigned char) s[i++];
   int extra = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : c >= 0xC0 ? 1 : 0;
   char32_t cp = extra == 3 ? c & 0x07 : extra == 2 ? c & 0x0F : extra == 1 ? c & 0x1F : c;
   if (c >= 0x80 && extra == 0)
      return 0xFFFD;
   for (int k = 0; k < extra; k++)
   {
      if (i >= s.size() || ((unsigned char) s[i] & 0xC0) != 0x80)
         return 0xFFFD;
      cp = (cp << 6) | ((unsigned char) s[i++] & 0x3F);
   }
   return cp;
}

}  // namespace

std::string AnsiToUtf8(const char *text, int length)
{
   std::string out;
   if (text == nullptr)
      return out;
   if (length < 0)
      length = (int) strlen(text);
   for (int i = 0; i < length; i++)
      AppendUtf8(out, AnsiToCodepoint((unsigned char) text[i]));
   return out;
}

std::string Utf8ToAnsi(const std::string &utf8)
{
   std::string out;
   for (size_t i = 0; i < utf8.size();)
   {
      int c = CodepointToAnsi(NextCodepoint(utf8, i));
      out += c < 0 ? '?' : (char) c;
   }
   return out;
}

}  // namespace w32

using namespace w32;

/************************************************************************/
// Code pages: CP_ACP is Windows-1252; CP_UTF8 is UTF-8.

INT WINAPI MultiByteToWideChar(UINT codepage, DWORD, LPCSTR src, INT src_len, LPWSTR dst, INT dst_len)
{
   if (src == nullptr)
      return 0;
   if (src_len < 0)
      src_len = (INT) strlen(src) + 1;
   std::u16string out;
   if (codepage == CP_UTF8)
   {
      std::string s(src, src_len);
      for (size_t i = 0; i < s.size();)
      {
         char32_t cp = NextCodepoint(s, i);
         if (cp >= 0x10000)
         {
            cp -= 0x10000;
            out += (char16_t) (0xD800 + (cp >> 10));
            out += (char16_t) (0xDC00 + (cp & 0x3FF));
         }
         else
            out += (char16_t) cp;
      }
   }
   else
      for (INT i = 0; i < src_len; i++)
         out += (char16_t) AnsiToCodepoint((unsigned char) src[i]);
   if (dst_len == 0)
      return (INT) out.size();
   if ((INT) out.size() > dst_len)
   {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return 0;
   }
   memcpy(dst, out.data(), out.size() * sizeof(char16_t));
   return (INT) out.size();
}

INT WINAPI WideCharToMultiByte(UINT codepage, DWORD, LPCWSTR src, INT src_len, LPSTR dst, INT dst_len,
                               LPCSTR default_char, BOOL *used_default)
{
   if (src == nullptr)
      return 0;
   if (src_len < 0)
   {
      src_len = 0;
      while (src[src_len])
         src_len++;
      src_len++;
   }
   std::string out;
   bool defaulted = false;
   for (INT i = 0; i < src_len; i++)
   {
      char32_t cp = src[i];
      if (cp >= 0xD800 && cp < 0xDC00 && i + 1 < src_len)
         cp = 0x10000 + ((cp - 0xD800) << 10) + (src[++i] - 0xDC00);
      if (codepage == CP_UTF8)
         AppendUtf8(out, cp);
      else
      {
         int c = CodepointToAnsi(cp);
         if (c < 0)
         {
            out += default_char ? *default_char : '?';
            defaulted = true;
         }
         else
            out += (char) c;
      }
   }
   if (used_default)
      *used_default = defaulted;
   if (dst_len == 0)
      return (INT) out.size();
   if ((INT) out.size() > dst_len)
   {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return 0;
   }
   memcpy(dst, out.data(), out.size());
   return (INT) out.size();
}

UINT WINAPI GetACP(void)
{
   return 1252;
}

/************************************************************************/
// lstr* and wsprintf

LPSTR WINAPI lstrcpynA(LPSTR dst, LPCSTR src, INT n)
{
   if (dst == nullptr || n <= 0)
      return dst;
   INT i = 0;
   if (src)
      for (; i < n - 1 && src[i]; i++)
         dst[i] = src[i];
   dst[i] = 0;
   return dst;
}

LPSTR WINAPI lstrcpyA(LPSTR dst, LPCSTR src)
{
   return strcpy(dst, src ? src : "");
}

LPSTR WINAPI lstrcatA(LPSTR dst, LPCSTR src)
{
   return strcat(dst, src ? src : "");
}

INT WINAPI lstrlenA(LPCSTR s)
{
   return s ? (INT) strlen(s) : 0;
}

INT WINAPI lstrcmpA(LPCSTR a, LPCSTR b)
{
   return strcmp(a ? a : "", b ? b : "");
}

INT WINAPI lstrcmpiA(LPCSTR a, LPCSTR b)
{
   return strcasecmp(a ? a : "", b ? b : "");
}

INT WINAPIV wsprintfA(LPSTR buffer, LPCSTR format, ...)
{
   va_list ap;
   va_start(ap, format);
   INT n = wvsprintfA(buffer, format, ap);
   va_end(ap);
   return n;
}

INT WINAPI wvsprintfA(LPSTR buffer, LPCSTR format, va_list ap)
{
   // wsprintf writes at most 1024 characters.
   return vsnprintf(buffer, 1025, format, ap);
}

LPSTR WINAPI CharUpperA(LPSTR s)
{
   if (IS_INTRESOURCE(s))
      return (LPSTR) (UINT_PTR) toupper((int) (UINT_PTR) s);
   for (char *p = s; *p; p++)
      *p = (char) toupper((unsigned char) *p);
   return s;
}

LPSTR WINAPI CharLowerA(LPSTR s)
{
   if (IS_INTRESOURCE(s))
      return (LPSTR) (UINT_PTR) tolower((int) (UINT_PTR) s);
   for (char *p = s; *p; p++)
      *p = (char) tolower((unsigned char) *p);
   return s;
}

LPSTR WINAPI CharNextA(LPCSTR s)
{
   return (LPSTR) (*s ? s + 1 : s);
}

BOOL WINAPI IsCharAlphaA(CHAR c)
{
   return isalpha((unsigned char) c) || (unsigned char) c >= 0xC0;
}

BOOL WINAPI IsCharAlphaNumericA(CHAR c)
{
   return IsCharAlphaA(c) || isdigit((unsigned char) c);
}
