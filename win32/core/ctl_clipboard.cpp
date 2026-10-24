/*
 * ctl_clipboard.cpp:  The clipboard, as Windows': opened by one window at a
 *   time, emptied by its new owner, holding global memory handles by format.
 *   CF_TEXT, CF_OEMTEXT and CF_UNICODETEXT stand in for each other (synthesized
 *   on request).  The store is in process; core/clipboard.h links it to the
 *   platform's.
 */

#include "core/clipboard.h"

#include "core/gdi.h"
#include "core/winproc.h"

#include <cstring>
#include <map>
#include <vector>

namespace w32 {

namespace {

struct Clipboard
{
   bool open = false;
   HWND open_window = nullptr;
   HWND owner = nullptr;
   bool changed = false;            // since it was opened
   DWORD sequence = 1;
   std::map<UINT, HANDLE> data;     // NULL: delayed rendering (WM_RENDERFORMAT)
   std::vector<UINT> order;         // formats in the order they were set
   std::map<UINT, HANDLE> synthesized;
   ClipboardBackend backend;
};

Clipboard &Board()
{
   static Clipboard board;
   return board;
}

bool IsText(UINT format)
{
   return format == CF_TEXT || format == CF_OEMTEXT || format == CF_UNICODETEXT;
}

HGLOBAL Copy(const void *bytes, size_t size)
{
   HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, size);
   if (h)
   {
      memcpy(GlobalLock(h), bytes, size);
      GlobalUnlock(h);
   }
   return h;
}

void FreeData(HANDLE h, UINT format)
{
   if (h == nullptr)
      return;
   // Bitmaps and palettes are GDI objects; the rest global memory.
   if (format == CF_BITMAP || format == CF_PALETTE || format == CF_DSPBITMAP)
      DeleteObject(h);
   else
      GlobalFree(h);
}

void DropSynthesized()
{
   Clipboard &c = Board();
   for (auto &e : c.synthesized)
      FreeData(e.second, e.first);
   c.synthesized.clear();
}

void Clear()
{
   Clipboard &c = Board();
   for (auto &e : c.data)
      FreeData(e.second, e.first);
   c.data.clear();
   c.order.clear();
   DropSynthesized();
}

// Text another program copied, when the backend has some: it replaces ours.
void FetchNative()
{
   Clipboard &c = Board();
   std::string utf8;
   if (c.backend.fetch == nullptr || !c.backend.fetch(&utf8))
      return;
   Clear();
   c.owner = nullptr;
   std::basic_string<WCHAR> w;
   int n = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int) utf8.size(), nullptr, 0);
   w.resize(n);
   MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int) utf8.size(), w.data(), n);
   c.data[CF_UNICODETEXT] = Copy(w.c_str(), (w.size() + 1) * sizeof(WCHAR));
   c.order.push_back(CF_UNICODETEXT);
   c.sequence++;
}

HANDLE Available(UINT format)
{
   Clipboard &c = Board();
   auto it = c.data.find(format);
   if (it != c.data.end())
   {
      if (it->second == nullptr && c.owner && IsWindow(c.owner))
      {
         // Delayed rendering: the owner puts it there now.
         SendMessageA(c.owner, WM_RENDERFORMAT, format, 0);
         it = c.data.find(format);
      }
      return it == c.data.end() ? nullptr : it->second;
   }
   if (!IsText(format))
      return nullptr;
   if (auto s = c.synthesized.find(format); s != c.synthesized.end())
      return s->second;
   // A text format from another one.
   for (UINT from : {CF_UNICODETEXT, CF_TEXT, CF_OEMTEXT})
   {
      if (from == format || !c.data.count(from))
         continue;
      HANDLE src = Available(from);
      if (src == nullptr)
         continue;
      const void *p = GlobalLock(src);
      SIZE_T size = GlobalSize(src);
      HGLOBAL out = nullptr;
      if (from == CF_UNICODETEXT)
      {
         const WCHAR *w = (const WCHAR *) p;
         size_t n = 0;
         while (n < size / sizeof(WCHAR) && w[n])
            n++;
         std::string a = WideToAnsi(w, (int) n);
         out = Copy(a.c_str(), a.size() + 1);
      }
      else
      {
         const char *a = (const char *) p;
         size_t n = strnlen(a, size);
         std::basic_string<WCHAR> w = AnsiToWide(a, (int) n);
         if (format == CF_UNICODETEXT)
            out = Copy(w.c_str(), (w.size() + 1) * sizeof(WCHAR));
         else
         {
            // CF_TEXT and CF_OEMTEXT: both the code page here.
            std::string copy(a, n);
            out = Copy(copy.c_str(), n + 1);
         }
      }
      GlobalUnlock(src);
      c.synthesized[format] = out;
      return out;
   }
   return nullptr;
}

bool HasFormat(UINT format)
{
   Clipboard &c = Board();
   if (c.data.count(format))
      return true;
   if (IsText(format))
      for (UINT f : {CF_UNICODETEXT, CF_TEXT, CF_OEMTEXT})
         if (c.data.count(f))
            return true;
   return false;
}

std::map<std::string, UINT> &FormatNames()
{
   static std::map<std::string, UINT> names;
   return names;
}

}  // namespace

void SetClipboardBackend(const ClipboardBackend &backend)
{
   Board().backend = backend;
}

}  // namespace w32

using namespace w32;

BOOL WINAPI OpenClipboard(HWND hwnd)
{
   Clipboard &c = Board();
   if (c.open && c.open_window != hwnd)
   {
      SetLastError(ERROR_ACCESS_DENIED);
      return FALSE;
   }
   if (hwnd && !IsWindow(hwnd))
   {
      SetLastError(ERROR_INVALID_WINDOW_HANDLE);
      return FALSE;
   }
   c.open = true;
   c.open_window = hwnd;
   c.changed = false;
   FetchNative();
   return TRUE;
}

BOOL WINAPI CloseClipboard(void)
{
   Clipboard &c = Board();
   if (!c.open)
   {
      SetLastError(ERROR_CLIPBOARD_NOT_OPEN);
      return FALSE;
   }
   c.open = false;
   c.open_window = nullptr;
   if (c.changed && c.backend.publish)
      if (HANDLE h = Available(CF_UNICODETEXT))
      {
         const WCHAR *w = (const WCHAR *) GlobalLock(h);
         size_t n = 0, max = GlobalSize(h) / sizeof(WCHAR);
         while (n < max && w[n])
            n++;
         int bytes = WideCharToMultiByte(CP_UTF8, 0, w, (int) n, nullptr, 0, nullptr, nullptr);
         std::string utf8(bytes, '\0');
         WideCharToMultiByte(CP_UTF8, 0, w, (int) n, utf8.data(), bytes, nullptr, nullptr);
         GlobalUnlock(h);
         c.backend.publish(utf8);
      }
   c.changed = false;
   return TRUE;
}

BOOL WINAPI EmptyClipboard(void)
{
   Clipboard &c = Board();
   if (!c.open)
   {
      SetLastError(ERROR_CLIPBOARD_NOT_OPEN);
      return FALSE;
   }
   if (c.owner && c.owner != c.open_window && IsWindow(c.owner))
      SendMessageA(c.owner, WM_DESTROYCLIPBOARD, 0, 0);
   Clear();
   c.owner = c.open_window;
   c.changed = true;
   c.sequence++;
   return TRUE;
}

HANDLE WINAPI SetClipboardData(UINT format, HANDLE data)
{
   Clipboard &c = Board();
   // Delayed rendering (data NULL) needs the owner; rendering may happen with the
   // clipboard closed (WM_RENDERFORMAT).
   if (!c.open && !(c.owner && data))
   {
      SetLastError(ERROR_CLIPBOARD_NOT_OPEN);
      return nullptr;
   }
   if (format == 0)
      return nullptr;
   auto it = c.data.find(format);
   if (it != c.data.end())
   {
      if (it->second != data)
         FreeData(it->second, format);
   }
   else
      c.order.push_back(format);
   c.data[format] = data;
   if (IsText(format))
      DropSynthesized();
   c.changed = true;
   c.sequence++;
   return data;
}

HANDLE WINAPI GetClipboardData(UINT format)
{
   Clipboard &c = Board();
   if (!c.open)
   {
      SetLastError(ERROR_CLIPBOARD_NOT_OPEN);
      return nullptr;
   }
   return Available(format);
}

BOOL WINAPI IsClipboardFormatAvailable(UINT format)
{
   if (!Board().open)
      FetchNative();
   return HasFormat(format);
}

INT WINAPI CountClipboardFormats(void)
{
   Clipboard &c = Board();
   int n = (int) c.order.size();
   for (UINT f : {CF_UNICODETEXT, CF_TEXT, CF_OEMTEXT})
      if (!c.data.count(f) && HasFormat(f))
         n++;
   return n;
}

UINT WINAPI EnumClipboardFormats(UINT format)
{
   Clipboard &c = Board();
   if (!c.open)
   {
      SetLastError(ERROR_CLIPBOARD_NOT_OPEN);
      return 0;
   }
   std::vector<UINT> all = c.order;
   for (UINT f : {CF_UNICODETEXT, CF_TEXT, CF_OEMTEXT})
      if (!c.data.count(f) && HasFormat(f))
         all.push_back(f);
   if (format == 0)
      return all.empty() ? 0 : all.front();
   for (size_t i = 0; i + 1 < all.size(); i++)
      if (all[i] == format)
         return all[i + 1];
   SetLastError(ERROR_SUCCESS);
   return 0;
}

HWND WINAPI GetClipboardOwner(void)
{
   HWND owner = Board().owner;
   return IsWindow(owner) ? owner : nullptr;
}

HWND WINAPI GetOpenClipboardWindow(void)
{
   return Board().open ? Board().open_window : nullptr;
}

DWORD WINAPI GetClipboardSequenceNumber(void)
{
   return Board().sequence;
}

UINT WINAPI RegisterClipboardFormatA(LPCSTR name)
{
   if (name == nullptr || *name == 0)
      return 0;
   std::string key = name;
   for (char &ch : key)
      ch = (char) tolower((unsigned char) ch);
   auto &names = FormatNames();
   auto it = names.find(key);
   if (it != names.end())
      return it->second;
   UINT id = 0xC000 + (UINT) names.size();
   names[key] = id;
   return id;
}

UINT WINAPI RegisterClipboardFormatW(LPCWSTR name)
{
   return name ? RegisterClipboardFormatA(WideToAnsi(name).c_str()) : 0;
}

INT WINAPI GetClipboardFormatNameA(UINT format, LPSTR buffer, INT size)
{
   for (auto &e : FormatNames())
      if (e.second == format && buffer && size > 0)
      {
         lstrcpynA(buffer, e.first.c_str(), size);
         return (INT) strlen(buffer);
      }
   return 0;
}
