/*
 * wine_runtime.cpp:  What Wine's sources get from Wine's ntdll and msvcrt: debug
 *   channels, and the 16-bit wide-string C runtime their prelude maps to.
 *
 *   M59_WINEDEBUG selects messages as WINEDEBUG does: "warn+listbox,+edit,-all"
 *   ("+name" is every class).  Default: errors only.
 */

#include "core/win32.h"

#include <wine/debug.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwctype>
#include <mutex>
#include <string>
#include <vector>

typedef unsigned short m59_wchar;

namespace {

struct Rule
{
   std::string channel;   // empty: all
   unsigned char set, clear;
};

std::vector<Rule> &Rules()
{
   static std::vector<Rule> rules = [] {
      std::vector<Rule> out;
      const char *env = getenv("M59_WINEDEBUG");
      std::string spec = env ? env : "";
      size_t start = 0;
      while (start < spec.size())
      {
         size_t comma = spec.find(',', start);
         std::string item = spec.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
         start = comma == std::string::npos ? spec.size() : comma + 1;
         size_t op = item.find_first_of("+-");
         if (op == std::string::npos)
            continue;
         std::string cls = item.substr(0, op), name = item.substr(op + 1);
         unsigned char bits;
         if (cls.empty())
            bits = (1 << __WINE_DBCL_FIXME) | (1 << __WINE_DBCL_ERR) | (1 << __WINE_DBCL_WARN) | (1 << __WINE_DBCL_TRACE);
         else if (cls == "fixme")
            bits = 1 << __WINE_DBCL_FIXME;
         else if (cls == "err")
            bits = 1 << __WINE_DBCL_ERR;
         else if (cls == "warn")
            bits = 1 << __WINE_DBCL_WARN;
         else if (cls == "trace")
            bits = 1 << __WINE_DBCL_TRACE;
         else
            continue;
         if (name == "all")
            name.clear();
         if (item[op] == '+')
            out.push_back({name, bits, 0});
         else
            out.push_back({name, 0, bits});
      }
      return out;
   }();
   return rules;
}

thread_local char strings[8][1024];
thread_local int next_string = 0;

const char *const class_names[] = {"fixme", "err", "warn", "trace"};

}  // namespace

extern "C" {

unsigned char __cdecl __wine_dbg_get_channel_flags(struct __wine_debug_channel *channel)
{
   if (channel->flags & (1 << __WINE_DBCL_INIT))
   {
      unsigned char flags = 1 << __WINE_DBCL_ERR;
      for (const Rule &r : Rules())
         if (r.channel.empty() || r.channel == channel->name)
            flags = (flags | r.set) & ~r.clear;
      channel->flags = flags;
   }
   return channel->flags;
}

const char *__cdecl __wine_dbg_strdup(const char *str)
{
   char *out = strings[next_string++ & 7];
   snprintf(out, sizeof(strings[0]), "%s", str);
   return out;
}

int WINAPI __wine_dbg_write(const char *str, unsigned int len)
{
   return (int) fwrite(str, 1, len, stderr);
}

int __cdecl __wine_dbg_output(const char *str)
{
   return fputs(str, stderr) < 0 ? -1 : (int) strlen(str);
}

int __cdecl __wine_dbg_header(enum __wine_debug_class cls, struct __wine_debug_channel *channel, const char *function)
{
   if (!(__wine_dbg_get_channel_flags(channel) & (1 << cls)))
      return -1;
   return fprintf(stderr, "%s:%s:%s ", cls < 4 ? class_names[cls] : "?", channel->name, function ? function : "");
}

// --- Wide strings ----------------------------------------------------------------

size_t m59_wcslen(const m59_wchar *s)
{
   size_t n = 0;
   while (s[n])
      n++;
   return n;
}

int m59_wcscmp(const m59_wchar *a, const m59_wchar *b)
{
   while (*a && *a == *b)
      a++, b++;
   return (int) *a - (int) *b;
}

int m59_wcsncmp(const m59_wchar *a, const m59_wchar *b, size_t n)
{
   for (; n; n--, a++, b++)
      if (*a != *b || !*a)
         return (int) *a - (int) *b;
   return 0;
}

m59_wchar m59_towupper(m59_wchar c)
{
   return (m59_wchar) towupper(c);
}

m59_wchar m59_towlower(m59_wchar c)
{
   return (m59_wchar) towlower(c);
}

int m59_wcsicmp(const m59_wchar *a, const m59_wchar *b)
{
   while (*a && m59_towlower(*a) == m59_towlower(*b))
      a++, b++;
   return (int) m59_towlower(*a) - (int) m59_towlower(*b);
}

int m59_wcsnicmp(const m59_wchar *a, const m59_wchar *b, size_t n)
{
   for (; n; n--, a++, b++)
      if (m59_towlower(*a) != m59_towlower(*b) || !*a)
         return (int) m59_towlower(*a) - (int) m59_towlower(*b);
   return 0;
}

m59_wchar *m59_wcschr(const m59_wchar *s, m59_wchar c)
{
   for (;; s++)
   {
      if (*s == c)
         return (m59_wchar *) s;
      if (!*s)
         return nullptr;
   }
}

m59_wchar *m59_wcsrchr(const m59_wchar *s, m59_wchar c)
{
   const m59_wchar *found = nullptr;
   for (;; s++)
   {
      if (*s == c)
         found = s;
      if (!*s)
         return (m59_wchar *) found;
   }
}

m59_wchar *m59_wcsstr(const m59_wchar *s, const m59_wchar *find)
{
   size_t n = m59_wcslen(find);
   for (; *s; s++)
      if (m59_wcsncmp(s, find, n) == 0)
         return (m59_wchar *) s;
   return n == 0 ? (m59_wchar *) s : nullptr;
}

m59_wchar *m59_wcscpy(m59_wchar *dst, const m59_wchar *src)
{
   memcpy(dst, src, (m59_wcslen(src) + 1) * sizeof(m59_wchar));
   return dst;
}

m59_wchar *m59_wcsncpy(m59_wchar *dst, const m59_wchar *src, size_t n)
{
   size_t i = 0;
   for (; i < n && src[i]; i++)
      dst[i] = src[i];
   for (; i < n; i++)
      dst[i] = 0;
   return dst;
}

m59_wchar *m59_wcscat(m59_wchar *dst, const m59_wchar *src)
{
   m59_wcscpy(dst + m59_wcslen(dst), src);
   return dst;
}

m59_wchar *m59_wcsdup(const m59_wchar *s)
{
   if (s == nullptr)
      return nullptr;
   size_t size = (m59_wcslen(s) + 1) * sizeof(m59_wchar);
   auto *out = (m59_wchar *) malloc(size);
   if (out)
      memcpy(out, s, size);
   return out;
}

static std::string Narrow(const m59_wchar *s, size_t *used = nullptr)
{
   // Numbers only: ASCII digits, signs, spaces, x.
   std::string out;
   size_t i = 0;
   for (; s[i] && s[i] < 128; i++)
      out += (char) s[i];
   if (used)
      *used = i;
   return out;
}

long m59_wcstol(const m59_wchar *s, m59_wchar **end, int base)
{
   std::string n = Narrow(s);
   char *e;
   long v = strtol(n.c_str(), &e, base);
   if (end)
      *end = (m59_wchar *) s + (e - n.c_str());
   return v;
}

unsigned long m59_wcstoul(const m59_wchar *s, m59_wchar **end, int base)
{
   std::string n = Narrow(s);
   char *e;
   unsigned long v = strtoul(n.c_str(), &e, base);
   if (end)
      *end = (m59_wchar *) s + (e - n.c_str());
   return v;
}

int m59_wtoi(const m59_wchar *s)
{
   return (int) m59_wcstol(s, nullptr, 10);
}

int m59_iswalnum(m59_wchar c) { return iswalnum(c); }
int m59_iswalpha(m59_wchar c) { return iswalpha(c); }
int m59_iswdigit(m59_wchar c) { return c >= '0' && c <= '9'; }
int m59_iswspace(m59_wchar c) { return iswspace(c); }
int m59_iswupper(m59_wchar c) { return iswupper(c); }
int m59_iswlower(m59_wchar c) { return iswlower(c); }
int m59_iswpunct(m59_wchar c) { return iswpunct(c); }

// Windows' wide printf: %s and %c are wide, %S/%hs and %C/%hc narrow (code page),
// the rest as printf.
int m59_vswprintf(m59_wchar *buffer, size_t count, const m59_wchar *format, va_list args)
{
   std::basic_string<m59_wchar> out;
   auto append_narrow = [&](const char *s, int precision) {
      if (s == nullptr)
         s = "(null)";
      size_t n = strlen(s);
      if (precision >= 0 && (size_t) precision < n)
         n = precision;
      std::vector<WCHAR> w(n + 1, 0);
      int m = MultiByteToWideChar(CP_ACP, 0, s, (int) n, w.data(), (int) n);
      out.append((const m59_wchar *) w.data(), m > 0 ? m : 0);
   };
   for (const m59_wchar *p = format; *p; p++)
   {
      if (*p != '%')
      {
         out += *p;
         continue;
      }
      // One conversion: %[flags][width][.precision][length]type
      std::string spec = "%";
      p++;
      while (*p && strchr("-+ #0", (char) *p))
         spec += (char) *p++;
      int width = -1, precision = -1;
      if (*p == '*')
      {
         width = va_arg(args, int);
         spec += std::to_string(width);
         p++;
      }
      else
         while (*p >= '0' && *p <= '9')
            spec += (char) *p++;
      if (*p == '.')
      {
         spec += *p++;
         if (*p == '*')
         {
            precision = va_arg(args, int);
            spec += std::to_string(precision);
            p++;
         }
         else
         {
            std::string digits;
            while (*p >= '0' && *p <= '9')
               digits += (char) *p++;
            spec += digits;
            precision = digits.empty() ? 0 : atoi(digits.c_str());
         }
      }
      std::string length;
      bool narrow = false, wide = false;
      while (*p && strchr("hlLqjztwI", (char) *p))
      {
         if (*p == 'h')
            narrow = true;
         else if (*p == 'l' || *p == 'w')
            wide = true;
         if (*p == 'I')
         {
            // I64 / I32 / I
            if (p[1] == '6' && p[2] == '4')
               length = "ll", p += 2;
            else if (p[1] == '3' && p[2] == '2')
               p += 2;
            else
               length = "z";
         }
         else if (*p != 'w')
            length += (char) *p;
         p++;
      }
      char type = (char) *p;
      if (!type)
         break;
      switch (type)
      {
      case '%': out += '%'; break;
      case 's':
      case 'S':
         if ((type == 's' && !narrow) || (type == 'S' && wide))
         {
            const m59_wchar *s = va_arg(args, const m59_wchar *);
            static const m59_wchar null_text[] = {'(', 'n', 'u', 'l', 'l', ')', 0};
            if (s == nullptr)
               s = null_text;
            size_t n = m59_wcslen(s);
            if (precision >= 0 && (size_t) precision < n)
               n = precision;
            out.append(s, n);
         }
         else
            append_narrow(va_arg(args, const char *), precision);
         break;
      case 'c':
      case 'C':
         if ((type == 'c' && !narrow) || (type == 'C' && wide))
            out += (m59_wchar) va_arg(args, int);
         else
         {
            char c = (char) va_arg(args, int);
            append_narrow(std::string(1, c).c_str(), -1);
         }
         break;
      default:
      {
         char text[128];
         std::string f = spec;
         if (length == "h")
            length.clear();
         f += length;
         f += type;
         if (strchr("eEfgGaA", type))
            snprintf(text, sizeof(text), f.c_str(), va_arg(args, double));
         else if (type == 'p')
            snprintf(text, sizeof(text), "%p", va_arg(args, void *));
         else if (length == "ll" || length == "q" || length == "j")
            snprintf(text, sizeof(text), f.c_str(), va_arg(args, long long));
         else if (length == "l" || length == "z" || length == "t")
            snprintf(text, sizeof(text), f.c_str(), va_arg(args, long));
         else
            snprintf(text, sizeof(text), f.c_str(), va_arg(args, int));
         for (char *t = text; *t; t++)
            out += (m59_wchar) (unsigned char) *t;
      }
      }
   }
   if (count == 0)
      return -1;
   size_t n = std::min(out.size(), count - 1);
   memcpy(buffer, out.data(), n * sizeof(m59_wchar));
   buffer[n] = 0;
   return out.size() < count ? (int) out.size() : -1;
}

int m59_swprintf(m59_wchar *buffer, size_t count, const m59_wchar *format, ...)
{
   va_list args;
   va_start(args, format);
   int r = m59_vswprintf(buffer, count, format, args);
   va_end(args);
   return r;
}

int m59_snwprintf(m59_wchar *buffer, size_t count, const m59_wchar *format, ...)
{
   va_list args;
   va_start(args, format);
   int r = m59_vswprintf(buffer, count, format, args);
   va_end(args);
   return r;
}

}  // extern "C"
