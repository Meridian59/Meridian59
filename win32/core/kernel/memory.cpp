/*
 * memory.cpp:  The heap, global and local memory beyond LocalAlloc/LocalFree,
 *   atoms, pointer checks, string comparison, the thread locale and a registry
 *   with no keys.  Memory is fixed: a handle is its block's address.
 */

#include "core/kernel/kernel.h"

#include "core/module.h"
#include "core/winproc.h"

#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>
#include <string>
#include <strings.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#define BlockSize(p) malloc_size(p)
#else
#include <malloc.h>
#define BlockSize(p) malloc_usable_size(p)
#endif

using namespace w32;

namespace {

void *Allocate(SIZE_T size, bool zero)
{
   void *p = zero ? calloc(1, size ? size : 1) : malloc(size ? size : 1);
   if (p == nullptr)
      SetLastError(ERROR_NOT_ENOUGH_MEMORY);
   return p;
}

void *Reallocate(void *p, SIZE_T size, bool zero)
{
   size_t old = p ? BlockSize(p) : 0;
   void *n = realloc(p, size ? size : 1);
   if (n == nullptr)
   {
      SetLastError(ERROR_NOT_ENOUGH_MEMORY);
      return nullptr;
   }
   if (zero && size > old)
      memset((char *) n + old, 0, size - old);
   return n;
}

// Atoms: strings (case-insensitive) numbered from 0xC000; "#n" and integers
// below 0xC000 are themselves.
std::mutex atoms_mutex;
std::map<std::string, ATOM> &AtomsByName()
{
   static std::map<std::string, ATOM> atoms;
   return atoms;
}
std::map<ATOM, std::pair<std::string, int>> &Atoms()
{
   static std::map<ATOM, std::pair<std::string, int>> atoms;   // name, references
   return atoms;
}

std::string Lowered(const char *s)
{
   std::string out = s;
   for (char &c : out)
      c = (char) tolower((unsigned char) c);
   return out;
}

ATOM MakeAtom(LPCSTR name)
{
   if (IS_INTRESOURCE(name))
      return (ATOM) (UINT_PTR) name;
   if (name[0] == '#')
      return (ATOM) atoi(name + 1);
   std::lock_guard<std::mutex> lock(atoms_mutex);
   std::string key = Lowered(name);
   auto it = AtomsByName().find(key);
   if (it != AtomsByName().end())
   {
      Atoms()[it->second].second++;
      return it->second;
   }
   static ATOM next = 0xC000;
   if (next == 0xFFFF)
      return 0;
   ATOM a = next++;
   AtomsByName()[key] = a;
   Atoms()[a] = {name, 1};
   return a;
}

ATOM LookupAtom(LPCSTR name)
{
   if (IS_INTRESOURCE(name))
      return (ATOM) (UINT_PTR) name;
   if (name[0] == '#')
      return (ATOM) atoi(name + 1);
   std::lock_guard<std::mutex> lock(atoms_mutex);
   auto it = AtomsByName().find(Lowered(name));
   return it == AtomsByName().end() ? 0 : it->second;
}

int Compare(DWORD flags, const std::basic_string<WCHAR> &a, const std::basic_string<WCHAR> &b)
{
   size_t n = std::min(a.size(), b.size());
   for (size_t i = 0; i < n; i++)
   {
      WCHAR x = a[i], y = b[i];
      if (flags & (NORM_IGNORECASE | LINGUISTIC_IGNORECASE))
         x = WideLower(x), y = WideLower(y);
      if (x != y)
         return x < y ? CSTR_LESS_THAN : CSTR_GREATER_THAN;
   }
   if (a.size() != b.size())
      return a.size() < b.size() ? CSTR_LESS_THAN : CSTR_GREATER_THAN;
   return CSTR_EQUAL;
}

std::basic_string<WCHAR> Counted(LPCWSTR s, INT length)
{
   if (length < 0)
   {
      length = 0;
      while (s[length])
         length++;
   }
   return std::basic_string<WCHAR>(s, length);
}

}  // namespace

HANDLE WINAPI GetProcessHeap(void)
{
   return (HANDLE) (UINT_PTR) 0x1000;
}

LPVOID WINAPI HeapAlloc(HANDLE, DWORD flags, SIZE_T size)
{
   return Allocate(size, (flags & HEAP_ZERO_MEMORY) != 0);
}

LPVOID WINAPI HeapReAlloc(HANDLE, DWORD flags, LPVOID p, SIZE_T size)
{
   if (flags & HEAP_REALLOC_IN_PLACE_ONLY)
      return BlockSize(p) >= size ? p : nullptr;
   return Reallocate(p, size, (flags & HEAP_ZERO_MEMORY) != 0);
}

BOOL WINAPI HeapFree(HANDLE, DWORD, LPVOID p)
{
   free(p);
   return TRUE;
}

SIZE_T WINAPI HeapSize(HANDLE, DWORD, LPCVOID p)
{
   return p ? BlockSize((void *) p) : (SIZE_T) -1;
}

HLOCAL WINAPI LocalReAlloc(HLOCAL mem, SIZE_T size, UINT flags)
{
   if (flags & LMEM_MODIFY)
      return mem;
   return Reallocate(mem, size, (flags & LMEM_ZEROINIT) != 0);
}

SIZE_T WINAPI LocalSize(HLOCAL mem)
{
   return mem ? BlockSize(mem) : 0;
}

LPVOID WINAPI LocalLock(HLOCAL mem)
{
   return mem;
}

BOOL WINAPI LocalUnlock(HLOCAL)
{
   return FALSE;
}

HLOCAL WINAPI LocalHandle(LPCVOID p)
{
   return (HLOCAL) p;
}

HGLOBAL WINAPI GlobalAlloc(UINT flags, SIZE_T size)
{
   return Allocate(size, (flags & GMEM_ZEROINIT) != 0);
}

HGLOBAL WINAPI GlobalReAlloc(HGLOBAL mem, SIZE_T size, UINT flags)
{
   if (flags & GMEM_MODIFY)
      return mem;
   return Reallocate(mem, size, (flags & GMEM_ZEROINIT) != 0);
}

HGLOBAL WINAPI GlobalFree(HGLOBAL mem)
{
   free(mem);
   return nullptr;
}

LPVOID WINAPI GlobalLock(HGLOBAL mem)
{
   return mem;
}

BOOL WINAPI GlobalUnlock(HGLOBAL)
{
   return FALSE;
}

SIZE_T WINAPI GlobalSize(HGLOBAL mem)
{
   return mem ? BlockSize(mem) : 0;
}

HGLOBAL WINAPI GlobalHandle(LPCVOID p)
{
   return (HGLOBAL) p;
}

ATOM WINAPI GlobalAddAtomA(LPCSTR name)
{
   return name ? MakeAtom(name) : 0;
}

ATOM WINAPI GlobalAddAtomW(LPCWSTR name)
{
   return name ? MakeAtom(AnsiArg(name)) : 0;
}

ATOM WINAPI AddAtomA(LPCSTR name)
{
   return GlobalAddAtomA(name);
}

ATOM WINAPI AddAtomW(LPCWSTR name)
{
   return GlobalAddAtomW(name);
}

ATOM WINAPI GlobalFindAtomA(LPCSTR name)
{
   return name ? LookupAtom(name) : 0;
}

ATOM WINAPI GlobalFindAtomW(LPCWSTR name)
{
   return name ? LookupAtom(AnsiArg(name)) : 0;
}

ATOM WINAPI GlobalDeleteAtom(ATOM atom)
{
   std::lock_guard<std::mutex> lock(atoms_mutex);
   auto it = Atoms().find(atom);
   if (it != Atoms().end() && --it->second.second == 0)
   {
      AtomsByName().erase(Lowered(it->second.first.c_str()));
      Atoms().erase(it);
   }
   return 0;
}

ATOM WINAPI DeleteAtom(ATOM atom)
{
   return GlobalDeleteAtom(atom);
}

UINT WINAPI GlobalGetAtomNameA(ATOM atom, LPSTR buffer, INT size)
{
   if (buffer == nullptr || size <= 0)
      return 0;
   std::string name;
   if (atom < 0xC000)
      name = "#" + std::to_string(atom);
   else
   {
      std::lock_guard<std::mutex> lock(atoms_mutex);
      auto it = Atoms().find(atom);
      if (it == Atoms().end())
      {
         SetLastError(ERROR_INVALID_HANDLE);
         return 0;
      }
      name = it->second.first;
   }
   lstrcpynA(buffer, name.c_str(), size);
   return (UINT) strlen(buffer);
}

UINT WINAPI GlobalGetAtomNameW(ATOM atom, LPWSTR buffer, INT size)
{
   char a[256];
   UINT n = GlobalGetAtomNameA(atom, a, sizeof(a));
   if (buffer == nullptr || size <= 0)
      return 0;
   std::basic_string<WCHAR> w = AnsiToWide(a, (int) n);
   size_t copy = std::min(w.size(), (size_t) size - 1);
   memcpy(buffer, w.data(), copy * sizeof(WCHAR));
   buffer[copy] = 0;
   return (UINT) copy;
}

// Pointers aren't probed: only null ones are bad.
BOOL WINAPI IsBadReadPtr(LPCVOID p, UINT_PTR size)
{
   return size && p == nullptr;
}

BOOL WINAPI IsBadWritePtr(LPVOID p, UINT_PTR size)
{
   return size && p == nullptr;
}

BOOL WINAPI IsBadCodePtr(FARPROC p)
{
   return p == nullptr;
}

BOOL WINAPI IsBadStringPtrA(LPCSTR s, UINT_PTR size)
{
   return size && s == nullptr;
}

BOOL WINAPI IsBadStringPtrW(LPCWSTR s, UINT_PTR size)
{
   return size && s == nullptr;
}

INT WINAPI CompareStringW(LCID, DWORD flags, LPCWSTR a, INT alen, LPCWSTR b, INT blen)
{
   if (a == nullptr || b == nullptr)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return 0;
   }
   return Compare(flags, Counted(a, alen), Counted(b, blen));
}

INT WINAPI CompareStringA(LCID, DWORD flags, LPCSTR a, INT alen, LPCSTR b, INT blen)
{
   if (a == nullptr || b == nullptr)
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return 0;
   }
   return Compare(flags, AnsiToWide(a, alen), AnsiToWide(b, blen));
}

LCID WINAPI GetUserDefaultLCID(void)
{
   return MAKELCID(UserLanguage(), SORT_DEFAULT);
}

LANGID WINAPI GetUserDefaultLangID(void)
{
   return UserLanguage();
}

LCID WINAPI GetThreadLocale(void)
{
   return GetUserDefaultLCID();
}

BOOL WINAPI DisableThreadLibraryCalls(HMODULE)
{
   return TRUE;
}

// The registry: empty and read-only.
LSTATUS WINAPI RegOpenKeyExA(HKEY, LPCSTR, DWORD, REGSAM, PHKEY key)
{
   if (key)
      *key = nullptr;
   return ERROR_FILE_NOT_FOUND;
}

LSTATUS WINAPI RegOpenKeyExW(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY key)
{
   if (key)
      *key = nullptr;
   return ERROR_FILE_NOT_FOUND;
}

LSTATUS WINAPI RegCreateKeyExA(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, SECURITY_ATTRIBUTES *, PHKEY key,
                               DWORD *)
{
   if (key)
      *key = nullptr;
   return ERROR_ACCESS_DENIED;
}

LSTATUS WINAPI RegCreateKeyExW(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, SECURITY_ATTRIBUTES *, PHKEY key,
                               DWORD *)
{
   if (key)
      *key = nullptr;
   return ERROR_ACCESS_DENIED;
}

LSTATUS WINAPI RegQueryValueExA(HKEY, LPCSTR, DWORD *, DWORD *, BYTE *, DWORD *)
{
   return ERROR_FILE_NOT_FOUND;
}

LSTATUS WINAPI RegQueryValueExW(HKEY, LPCWSTR, DWORD *, DWORD *, BYTE *, DWORD *)
{
   return ERROR_FILE_NOT_FOUND;
}

LSTATUS WINAPI RegSetValueExA(HKEY, LPCSTR, DWORD, DWORD, const BYTE *, DWORD)
{
   return ERROR_ACCESS_DENIED;
}

LSTATUS WINAPI RegSetValueExW(HKEY, LPCWSTR, DWORD, DWORD, const BYTE *, DWORD)
{
   return ERROR_ACCESS_DENIED;
}

LSTATUS WINAPI RegCloseKey(HKEY)
{
   return ERROR_SUCCESS;
}
