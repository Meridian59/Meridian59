/*
 * winproc.cpp:  Translating messages between ANSI and Unicode window procedures,
 *   and the procedure handles GetWindowLongPtr gives across sets (after Wine's
 *   winproc.c).  ANSI is the code page GetACP reports, one byte per character, so
 *   lengths are the same in both sets.
 */

#include "core/winproc.h"

#include <commctrl.h>

#include <mutex>
#include <vector>

namespace w32 {

namespace {

// Handles: (kHandleBase | index), never a code address.
constexpr UINT_PTR kHandleBase = 0xFFFF0000;
constexpr UINT_PTR kHandleCount = 0x10000;

std::mutex handles_mutex;

std::vector<ProcRef> &Handles()
{
   static std::vector<ProcRef> handles;
   return handles;
}

bool IsHandle(WNDPROC value)
{
   UINT_PTR v = (UINT_PTR) value;
   return v >= kHandleBase && v < kHandleBase + kHandleCount && v - kHandleBase < Handles().size();
}

// Whether a list or combo box message carries a string: only with LBS_HASSTRINGS, or
// when the box isn't owner-drawn.
bool ListHasStrings(HWND hwnd, UINT msg)
{
   DWORD style = (DWORD) GetWindowLongA(hwnd, GWL_STYLE);
   if (msg >= CB_GETEDITSEL && msg <= CB_MSGMAX)
      return !(style & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)) || (style & CBS_HASSTRINGS);
   return !(style & (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE)) || (style & LBS_HASSTRINGS);
}

size_t WideLength(const WCHAR *s)
{
   size_t n = 0;
   while (s[n])
      n++;
   return n;
}

// Calls the Unicode procedure fn with a message in ANSI form.
LRESULT CallWWithA(WNDPROC fn, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_NCCREATE:
   case WM_CREATE:
   {
      auto *a = (CREATESTRUCTA *) lparam;
      CREATESTRUCTW cs;
      memcpy(&cs, a, sizeof(cs));
      std::basic_string<WCHAR> name, cls;
      if (!IS_INTRESOURCE(a->lpszName) && (BYTE) a->lpszName[0] == 0xFF)
      {
         // A dialog control's ordinal title (an icon or bitmap id): 0xFF and the id
         // in ANSI, 0xFFFF and the id in Unicode.
         name = {(WCHAR) 0xFFFF, (WCHAR) ((BYTE) a->lpszName[1] | ((BYTE) a->lpszName[2] << 8)), 0};
         cs.lpszName = name.c_str();
      }
      else if (!IS_INTRESOURCE(a->lpszName))
         name = AnsiToWide(a->lpszName), cs.lpszName = name.c_str();
      if (!IS_INTRESOURCE(a->lpszClass))
         cls = AnsiToWide(a->lpszClass), cs.lpszClass = cls.c_str();
      return fn(hwnd, msg, wparam, (LPARAM) &cs);
   }

   case WM_GETTEXT:
   case WM_ASKCBFORMATNAME:
   {
      if (wparam == 0 || lparam == 0)
         return fn(hwnd, msg, wparam, lparam);
      std::vector<WCHAR> buffer(wparam + 1, 0);
      LRESULT r = fn(hwnd, msg, wparam, (LPARAM) buffer.data());
      std::string s = WideToAnsi(buffer.data(), (int) std::min<LRESULT>(r, (LRESULT) wparam));
      size_t n = std::min(s.size(), (size_t) wparam - 1);
      memcpy((char *) lparam, s.data(), n);
      ((char *) lparam)[n] = 0;
      return (LRESULT) n;
   }

   case LB_ADDSTRING:
   case LB_INSERTSTRING:
   case LB_FINDSTRING:
   case LB_FINDSTRINGEXACT:
   case LB_SELECTSTRING:
   case CB_ADDSTRING:
   case CB_INSERTSTRING:
   case CB_FINDSTRING:
   case CB_FINDSTRINGEXACT:
   case CB_SELECTSTRING:
      if (lparam == 0 || !ListHasStrings(hwnd, msg))
         return fn(hwnd, msg, wparam, lparam);
      [[fallthrough]];
   case WM_SETTEXT:
   case WM_WININICHANGE:
   case WM_DEVMODECHANGE:
   case CB_DIR:
   case LB_DIR:
   case LB_ADDFILE:
   case EM_REPLACESEL:
   {
      if (lparam == 0)
         return fn(hwnd, msg, wparam, lparam);
      std::basic_string<WCHAR> s = AnsiToWide((const char *) lparam);
      return fn(hwnd, msg, wparam, (LPARAM) s.c_str());
   }

   case LB_GETTEXT:
   case CB_GETLBTEXT:
   {
      if (lparam == 0 || !ListHasStrings(hwnd, msg))
         return fn(hwnd, msg, wparam, lparam);
      LRESULT length = fn(hwnd, msg == LB_GETTEXT ? LB_GETTEXTLEN : CB_GETLBTEXTLEN, wparam, 0);
      if (length < 0)
         return length;
      std::vector<WCHAR> buffer(length + 1, 0);
      LRESULT r = fn(hwnd, msg, wparam, (LPARAM) buffer.data());
      if (r < 0)
         return r;
      std::string s = WideToAnsi(buffer.data());
      memcpy((char *) lparam, s.c_str(), s.size() + 1);
      return (LRESULT) s.size();
   }

   case EM_GETLINE:
   {
      WORD max = *(WORD *) lparam;
      std::vector<WCHAR> buffer(std::max<WORD>(max, 1) + 1, 0);
      *(WORD *) buffer.data() = max;
      LRESULT r = fn(hwnd, msg, wparam, (LPARAM) buffer.data());
      std::string s = WideToAnsi(buffer.data(), (int) r);
      size_t n = std::min(s.size(), (size_t) max);
      memcpy((char *) lparam, s.data(), n);
      if (n < max)
         ((char *) lparam)[n] = 0;
      return (LRESULT) n;
   }

   case WM_GETDLGCODE:
      if (lparam)
      {
         MSG m = *(MSG *) lparam;
         if (m.message == WM_CHAR || m.message == WM_SYSCHAR || m.message == WM_DEADCHAR ||
             m.message == WM_SYSDEADCHAR)
            m.wParam = CharAtoW(m.wParam);
         return fn(hwnd, msg, wparam, (LPARAM) &m);
      }
      return fn(hwnd, msg, wparam, lparam);

   case WM_CHARTOITEM:
   case WM_MENUCHAR:
   case WM_CHAR:
   case WM_DEADCHAR:
   case WM_SYSCHAR:
   case WM_SYSDEADCHAR:
   case EM_SETPASSWORDCHAR:
      return fn(hwnd, msg, CharAtoW(wparam), lparam);

   case EM_GETPASSWORDCHAR:
      return (LRESULT) CharWtoA(fn(hwnd, msg, wparam, lparam));
   }
   return fn(hwnd, msg, wparam, lparam);
}

// Calls the ANSI procedure fn with a message in Unicode form.
LRESULT CallAWithW(WNDPROC fn, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
   case WM_NCCREATE:
   case WM_CREATE:
   {
      auto *w = (CREATESTRUCTW *) lparam;
      CREATESTRUCTA cs;
      memcpy(&cs, w, sizeof(cs));
      std::string name, cls;
      if (!IS_INTRESOURCE(w->lpszName) && w->lpszName[0] == 0xFFFF)
      {
         name = {(char) 0xFF, (char) (w->lpszName[1] & 0xFF), (char) (w->lpszName[1] >> 8)};
         cs.lpszName = name.c_str();
      }
      else if (!IS_INTRESOURCE(w->lpszName))
         name = WideToAnsi(w->lpszName), cs.lpszName = name.c_str();
      if (!IS_INTRESOURCE(w->lpszClass))
         cls = WideToAnsi(w->lpszClass), cs.lpszClass = cls.c_str();
      return fn(hwnd, msg, wparam, (LPARAM) &cs);
   }

   case WM_GETTEXT:
   case WM_ASKCBFORMATNAME:
   {
      if (wparam == 0 || lparam == 0)
         return fn(hwnd, msg, wparam, lparam);
      std::vector<char> buffer(wparam + 1, 0);
      LRESULT r = fn(hwnd, msg, wparam, (LPARAM) buffer.data());
      std::basic_string<WCHAR> s = AnsiToWide(buffer.data(), (int) std::min<LRESULT>(r, (LRESULT) wparam));
      size_t n = std::min(s.size(), (size_t) wparam - 1);
      memcpy((WCHAR *) lparam, s.data(), n * sizeof(WCHAR));
      ((WCHAR *) lparam)[n] = 0;
      return (LRESULT) n;
   }

   case LB_ADDSTRING:
   case LB_INSERTSTRING:
   case LB_FINDSTRING:
   case LB_FINDSTRINGEXACT:
   case LB_SELECTSTRING:
   case CB_ADDSTRING:
   case CB_INSERTSTRING:
   case CB_FINDSTRING:
   case CB_FINDSTRINGEXACT:
   case CB_SELECTSTRING:
      if (lparam == 0 || !ListHasStrings(hwnd, msg))
         return fn(hwnd, msg, wparam, lparam);
      [[fallthrough]];
   case WM_SETTEXT:
   case WM_WININICHANGE:
   case WM_DEVMODECHANGE:
   case CB_DIR:
   case LB_DIR:
   case LB_ADDFILE:
   case EM_REPLACESEL:
   {
      if (lparam == 0)
         return fn(hwnd, msg, wparam, lparam);
      std::string s = WideToAnsi((const WCHAR *) lparam);
      return fn(hwnd, msg, wparam, (LPARAM) s.c_str());
   }

   case LB_GETTEXT:
   case CB_GETLBTEXT:
   {
      if (lparam == 0 || !ListHasStrings(hwnd, msg))
         return fn(hwnd, msg, wparam, lparam);
      LRESULT length = fn(hwnd, msg == LB_GETTEXT ? LB_GETTEXTLEN : CB_GETLBTEXTLEN, wparam, 0);
      if (length < 0)
         return length;
      std::vector<char> buffer(length + 1, 0);
      LRESULT r = fn(hwnd, msg, wparam, (LPARAM) buffer.data());
      if (r < 0)
         return r;
      std::basic_string<WCHAR> s = AnsiToWide(buffer.data());
      memcpy((WCHAR *) lparam, s.c_str(), (s.size() + 1) * sizeof(WCHAR));
      return (LRESULT) s.size();
   }

   case EM_GETLINE:
   {
      WORD max = *(WORD *) lparam;
      std::vector<char> buffer(std::max<WORD>(max, 2) + 1, 0);
      *(WORD *) buffer.data() = max;
      LRESULT r = fn(hwnd, msg, wparam, (LPARAM) buffer.data());
      std::basic_string<WCHAR> s = AnsiToWide(buffer.data(), (int) r);
      size_t n = std::min(s.size(), (size_t) max);
      memcpy((WCHAR *) lparam, s.data(), n * sizeof(WCHAR));
      if (n < max)
         ((WCHAR *) lparam)[n] = 0;
      return (LRESULT) n;
   }

   case WM_GETDLGCODE:
      if (lparam)
      {
         MSG m = *(MSG *) lparam;
         if (m.message == WM_CHAR || m.message == WM_SYSCHAR || m.message == WM_DEADCHAR ||
             m.message == WM_SYSDEADCHAR)
            m.wParam = CharWtoA(m.wParam);
         return fn(hwnd, msg, wparam, (LPARAM) &m);
      }
      return fn(hwnd, msg, wparam, lparam);

   case WM_CHARTOITEM:
   case WM_MENUCHAR:
   case WM_CHAR:
   case WM_DEADCHAR:
   case WM_SYSCHAR:
   case WM_SYSDEADCHAR:
   case EM_SETPASSWORDCHAR:
      return fn(hwnd, msg, CharWtoA(wparam), lparam);

   case EM_GETPASSWORDCHAR:
      return (LRESULT) CharAtoW(fn(hwnd, msg, wparam, lparam));
   }
   return fn(hwnd, msg, wparam, lparam);
}

}  // namespace

std::string WideToAnsi(const WCHAR *s, int length)
{
   if (s == nullptr)
      return {};
   if (length < 0)
      length = (int) WideLength(s);
   std::string out(length, '\0');
   int n = WideCharToMultiByte(CP_ACP, 0, s, length, out.data(), length, nullptr, nullptr);
   out.resize(n > 0 ? n : 0);
   return out;
}

std::basic_string<WCHAR> AnsiToWide(const char *s, int length)
{
   if (s == nullptr)
      return {};
   if (length < 0)
      length = (int) strlen(s);
   std::basic_string<WCHAR> out(length, 0);
   int n = MultiByteToWideChar(CP_ACP, 0, s, length, out.data(), length);
   out.resize(n > 0 ? n : 0);
   return out;
}

AnsiArg::AnsiArg(LPCWSTR s)
{
   if (IS_INTRESOURCE(s))
      atom_ = (ULONG_PTR) s;
   else
      text_ = WideToAnsi(s);
}

WPARAM CharAtoW(WPARAM wparam)
{
   char c = (char) (wparam & 0xFF);
   WCHAR w = 0;
   MultiByteToWideChar(CP_ACP, 0, &c, 1, &w, 1);
   return (wparam & ~(WPARAM) 0xFFFF) | w;
}

WPARAM CharWtoA(WPARAM wparam)
{
   WCHAR w = (WCHAR) (wparam & 0xFFFF);
   char c = '?';
   WideCharToMultiByte(CP_ACP, 0, &w, 1, &c, 1, nullptr, nullptr);
   return (wparam & ~(WPARAM) 0xFFFF) | (BYTE) c;
}

WNDPROC ProcValue(ProcRef p, bool unicode)
{
   if (p.fn == nullptr || p.unicode == unicode)
      return p.fn;
   std::lock_guard<std::mutex> lock(handles_mutex);
   auto &handles = Handles();
   for (size_t i = 0; i < handles.size(); i++)
      if (handles[i].fn == p.fn && handles[i].unicode == p.unicode)
         return (WNDPROC) (kHandleBase + i);
   if (handles.size() >= kHandleCount)
      return p.fn;
   handles.push_back(p);
   return (WNDPROC) (kHandleBase + handles.size() - 1);
}

ProcRef ProcFromValue(WNDPROC value, bool unicode)
{
   std::lock_guard<std::mutex> lock(handles_mutex);
   if (IsHandle(value))
      return Handles()[(UINT_PTR) value - kHandleBase];
   return {value, unicode};
}

LRESULT CallProcRef(ProcRef p, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, bool unicode)
{
   if (p.fn == nullptr)
      return 0;
   if (p.unicode == unicode)
      return p.fn(hwnd, msg, wparam, lparam);
   return unicode ? CallAWithW(p.fn, hwnd, msg, wparam, lparam) : CallWWithA(p.fn, hwnd, msg, wparam, lparam);
}

}  // namespace w32
