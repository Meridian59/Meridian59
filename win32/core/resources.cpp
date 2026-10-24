/*
 * resources.cpp:  Modules and their resources.  Each binary embeds its compiled
 *   .res (win32/res/embed.c.in) and registers it as it loads; the executable is
 *   GetModuleHandle(NULL), the client's modules come from LoadLibrary.
 */

#include "core/module.h"

#include "core/gdi.h"
#include "core/handles.h"
#include "core/paths.h"

#include <cstring>
#include <dlfcn.h>
#include <map>
#include <mutex>
#include <vector>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace w32 {

namespace {

// A resource ID: an ordinal, or a name (upper case, as Windows compares names).
struct ResId
{
   WORD ordinal = 0;
   std::string name;
   bool operator<(const ResId &o) const { return ordinal != o.ordinal ? ordinal < o.ordinal : name < o.name; }
   bool operator==(const ResId &o) const { return ordinal == o.ordinal && name == o.name; }
};

struct Resource
{
   const BYTE *data;
   DWORD size;
   WORD language;
};

struct Module
{
   std::string path;
   void *dl = nullptr;                        // dlopen handle (LoadLibrary)
   const BYTE *res_begin = nullptr, *res_end = nullptr;
   std::map<std::pair<ResId, ResId>, std::vector<Resource>> resources;   // (type, name)
   int refs = 1;
};

HandleTable<Module, HMODULE> &Modules()
{
   static HandleTable<Module, HMODULE> modules(kModuleHandleBase, 0x10000);
   return modules;
}

std::mutex modules_mutex;

// Resources registered by binaries that have loaded but aren't modules yet: the
// executable's (before main), or the one LoadLibrary is loading.
std::vector<std::pair<const BYTE *, const BYTE *>> &Pending()
{
   static std::vector<std::pair<const BYTE *, const BYTE *>> pending;
   return pending;
}

HMODULE main_module = nullptr;

ResId ReadId(const BYTE *&p)
{
   ResId id;
   const WORD *w = (const WORD *) p;
   if (w[0] == 0xFFFF)
   {
      id.ordinal = w[1];
      p += 4;
      return id;
   }
   while (*w)
   {
      id.name += (char) toupper((unsigned char) (*w < 0x100 ? *w : '?'));
      w++;
   }
   p = (const BYTE *) (w + 1);
   return id;
}

ResId FromApi(LPCSTR id)
{
   ResId r;
   if (IS_INTRESOURCE(id))
      r.ordinal = (WORD) (UINT_PTR) id;
   else if (id[0] == '#')
      r.ordinal = (WORD) atoi(id + 1);
   else
      for (const char *c = id; *c; c++)
         r.name += (char) toupper((unsigned char) *c);
   return r;
}

// Parses a .res: DataSize, HeaderSize, TYPE, NAME, DataVersion, MemoryFlags,
// LanguageId, Version, Characteristics; data DWORD-aligned.
void Index(Module *m)
{
   const BYTE *p = m->res_begin;
   while (p + 8 <= m->res_end)
   {
      DWORD data_size = ((const DWORD *) p)[0], header_size = ((const DWORD *) p)[1];
      if (header_size < 16 || p + header_size + data_size > m->res_end)
         break;
      const BYTE *q = p + 8;
      ResId type = ReadId(q);
      ResId name = ReadId(q);
      q = (const BYTE *) (((uintptr_t) q + 3) & ~(uintptr_t) 3);
      WORD language = ((const WORD *) q)[3];   // after DataVersion (4 bytes) and MemoryFlags (2)
      if (!(type.ordinal == 0 && type.name.empty()))
         m->resources[{type, name}].push_back({p + header_size, data_size, language});
      p += (header_size + data_size + 3) & ~3u;
   }
}

// The user's language, as Windows' resource lookup prefers it: from LANG.
WORD PreferredLanguage()
{
   static WORD lang = [] {
      const char *env = getenv("LC_ALL");
      if (env == nullptr || *env == 0)
         env = getenv("LC_MESSAGES");
      if (env == nullptr || *env == 0)
         env = getenv("LANG");
      std::string l = env ? env : "";
      if (l.rfind("de", 0) == 0)
         return (WORD) MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN);
      if (l.rfind("pt", 0) == 0)
         return (WORD) MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN);
      return (WORD) MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
   }();
   return lang;
}

const Resource *Pick(const std::vector<Resource> &list)
{
   WORD want = PreferredLanguage();
   for (const Resource &r : list)
      if (r.language == want)
         return &r;
   for (const Resource &r : list)
      if (PRIMARYLANGID(r.language) == PRIMARYLANGID(want))
         return &r;
   for (const Resource &r : list)
      if (PRIMARYLANGID(r.language) == LANG_NEUTRAL || PRIMARYLANGID(r.language) == LANG_ENGLISH)
         return &r;
   return list.empty() ? nullptr : &list.front();
}

std::string ExecutablePath()
{
#if defined(__APPLE__)
   char buf[4096];
   uint32_t size = sizeof(buf);
   if (_NSGetExecutablePath(buf, &size) == 0)
      return buf;
   return "meridian";
#else
   char buf[4096];
   ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
   return n > 0 ? std::string(buf, n) : std::string("meridian");
#endif
}

Module *MakeModule(const std::string &path, void *dl)
{
   auto *m = new Module;
   m->path = path;
   m->dl = dl;
   std::lock_guard<std::mutex> lock(modules_mutex);
   if (!Pending().empty())
   {
      m->res_begin = Pending().back().first;
      m->res_end = Pending().back().second;
      Pending().pop_back();
      Index(m);
   }
   return m;
}

}  // namespace

WORD UserLanguage()
{
   return PreferredLanguage();
}

HMODULE MainModule()
{
   if (main_module == nullptr)
   {
      // The executable registered its resources before main: the first ones.
      std::vector<std::pair<const BYTE *, const BYTE *>> others;
      {
         std::lock_guard<std::mutex> lock(modules_mutex);
         if (Pending().size() > 1)
         {
            others.assign(Pending().begin() + 1, Pending().end());
            Pending().resize(1);
         }
      }
      main_module = Modules().Add(MakeModule(ExecutablePath(), nullptr));
      std::lock_guard<std::mutex> lock(modules_mutex);
      Pending() = others;
   }
   return main_module;
}

bool FindResourceData(HMODULE module, LPCSTR name, LPCSTR type, const BYTE **data, DWORD *size)
{
   Module *m = Modules().Get(module ? module : MainModule());
   if (m == nullptr || name == nullptr || type == nullptr)
      return false;
   auto it = m->resources.find({FromApi(type), FromApi(name)});
   if (it == m->resources.end())
      return false;
   const Resource *r = Pick(it->second);
   if (r == nullptr)
      return false;
   *data = r->data;
   *size = r->size;
   return true;
}

}  // namespace w32

using namespace w32;

extern "C" W32_EXPORT void M59RegisterResources(const void *begin, const void *end)
{
   std::lock_guard<std::mutex> lock(modules_mutex);
   Pending().push_back({(const BYTE *) begin, (const BYTE *) end});
}

/************************************************************************/
// Modules

HMODULE WINAPI GetModuleHandleA(LPCSTR name)
{
   if (name == nullptr)
      return MainModule();
   // A loaded module by file name, ignoring directory, case and extension.
   std::string want = name;
   size_t slash = want.find_last_of("\\/");
   if (slash != std::string::npos)
      want = want.substr(slash + 1);
   size_t dot = want.find('.');
   if (dot != std::string::npos)
      want.resize(dot);
   HMODULE found = nullptr;
   Modules().ForEach([&](HMODULE h, Module *m) {
      std::string base = m->path.substr(m->path.find_last_of('/') + 1);
      size_t d = base.find('.');
      if (d != std::string::npos)
         base.resize(d);
      if (strcasecmp(base.c_str(), want.c_str()) == 0)
         found = h;
   });
   return found;
}

DWORD WINAPI GetModuleFileNameA(HMODULE module, LPSTR buffer, DWORD size)
{
   Module *m = Modules().Get(module ? module : MainModule());
   if (m == nullptr || buffer == nullptr || size == 0)
      return 0;
   lstrcpynA(buffer, m->path.c_str(), (INT) size);
   return (DWORD) strlen(buffer);
}

// Resources of the built-in DLLs, by file name (lower case).
static std::map<std::string, std::pair<const BYTE *, const BYTE *>> &BuiltinResources()
{
   static std::map<std::string, std::pair<const BYTE *, const BYTE *>> resources;
   return resources;
}

extern "C" W32_EXPORT void M59RegisterModuleResources(const char *name, const void *begin, const void *end)
{
   BuiltinResources()[name] = {(const BYTE *) begin, (const BYTE *) end};
}

// A module's .def exports by ordinal and name, and its DllMain
// (win32/tools/defexports.py).
static const void *TableExport(void *dl, LPCSTR name)
{
   struct OrdinalExport
   {
      unsigned short ordinal;
      const char *name;
      const void *address;
   };
   auto *table = dl ? (const OrdinalExport *) dlsym(dl, "m59_ordinal_exports") : nullptr;
   for (const OrdinalExport *e = table; e && e->name; e++)
      if (IS_INTRESOURCE(name) ? e->ordinal && e->ordinal == (UINT_PTR) name : strcmp(e->name, name) == 0)
         return e->address;
   return nullptr;
}

static BOOL CallDllMain(void *dl, HMODULE module, DWORD reason)
{
   auto main = (BOOL(WINAPI *)(HINSTANCE, DWORD, LPVOID)) TableExport(dl, "DllMain");
   return main ? main((HINSTANCE) module, reason, nullptr) : TRUE;
}

// Windows DLLs the layer itself provides: loading them gives a module of their
// name, without a file.
static HMODULE BuiltinModule(LPCSTR name)
{
   std::string base = name;
   size_t slash = base.find_last_of("\\/");
   if (slash != std::string::npos)
      base = base.substr(slash + 1);
   for (char &c : base)
      c = (char) tolower((unsigned char) c);
   if (base.find('.') == std::string::npos)
      base += ".dll";
   static const char *const builtin[] = {"riched32.dll", "riched20.dll", "comctl32.dll", "comdlg32.dll",
                                         "user32.dll", "gdi32.dll", "kernel32.dll", "shell32.dll",
                                         "wininet.dll", "ws2_32.dll", "wsock32.dll", "winmm.dll", "dwmapi.dll",
                                         "msvfw32.dll", "uxtheme.dll"};
   for (const char *b : builtin)
      if (base == b)
      {
         static std::map<std::string, HMODULE> loaded;
         auto it = loaded.find(base);
         if (it != loaded.end())
            return it->second;
         auto *m = new Module;
         m->path = base;
         // Its resources, if the layer embeds them (M59RegisterModuleResources).
         auto res = BuiltinResources().find(base);
         if (res != BuiltinResources().end())
         {
            m->res_begin = res->second.first;
            m->res_end = res->second.second;
            Index(m);
         }
         HMODULE h = Modules().Add(m);
         loaded[base] = h;
         return h;
      }
   return nullptr;
}

namespace w32 {

HMODULE BuiltinDll(const char *name)
{
   return BuiltinModule(name);
}

}  // namespace w32

HMODULE WINAPI LoadLibraryA(LPCSTR name)
{
   if (name == nullptr)
      return nullptr;
   if (HMODULE h = BuiltinModule(name))
      return h;
   std::string path = HostPath(name);
   // Already loaded: one more reference.
   HMODULE existing = nullptr;
   Modules().ForEach([&](HMODULE h, Module *m) {
      if (m->path == path)
         existing = h;
   });
   if (existing)
   {
      Modules().Get(existing)->refs++;
      return existing;
   }
   void *dl = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
   if (dl == nullptr)
   {
      SetLastError(ERROR_MOD_NOT_FOUND);
      return nullptr;
   }
   HMODULE h = Modules().Add(MakeModule(path, dl));
   // The loader's DLL_PROCESS_ATTACH; FALSE fails the load.
   if (!CallDllMain(dl, h, DLL_PROCESS_ATTACH))
   {
      delete Modules().Remove(h);
      dlclose(dl);
      SetLastError(ERROR_DLL_INIT_FAILED);
      return nullptr;
   }
   return h;
}

HMODULE WINAPI LoadLibraryExA(LPCSTR name, HANDLE, DWORD)
{
   return LoadLibraryA(name);
}

BOOL WINAPI FreeLibrary(HMODULE module)
{
   Module *m = Modules().Get(module);
   if (m == nullptr)
      return FALSE;
   if (m->dl == nullptr)
      return TRUE;   // the executable and built-in DLLs stay
   if (--m->refs > 0)
      return TRUE;
   CallDllMain(m->dl, module, DLL_PROCESS_DETACH);
   Modules().Remove(module);
   dlclose(m->dl);
   delete m;
   return TRUE;
}

FARPROC WINAPI GetProcAddress(HMODULE module, LPCSTR name)
{
   Module *m = Modules().Get(module);
   if (m == nullptr)
      return nullptr;
   if (const void *e = TableExport(m->dl, name))
      return (FARPROC) e;
   void *sym = IS_INTRESOURCE(name) ? nullptr : dlsym(m->dl ? m->dl : RTLD_DEFAULT, name);
   if (sym == nullptr)
      SetLastError(ERROR_PROC_NOT_FOUND);
   return (FARPROC) sym;
}

/************************************************************************/
// Resources

namespace {

// HRSRC and HGLOBAL: the resource's data and size.
struct ResourceHandle
{
   const BYTE *data;
   DWORD size;
};

HandleTable<ResourceHandle, HRSRC> &ResourceHandles()
{
   static HandleTable<ResourceHandle, HRSRC> handles(kModuleHandleBase + 0x8000000);
   return handles;
}

}  // namespace

HRSRC WINAPI FindResourceA(HMODULE module, LPCSTR name, LPCSTR type)
{
   const BYTE *data;
   DWORD size;
   if (!FindResourceData(module, name, type, &data, &size))
   {
      SetLastError(ERROR_RESOURCE_NAME_NOT_FOUND);
      return nullptr;
   }
   // One handle per resource, for the run.
   static std::map<const BYTE *, HRSRC> handles;
   auto it = handles.find(data);
   if (it != handles.end())
      return it->second;
   HRSRC h = ResourceHandles().Add(new ResourceHandle{data, size});
   handles[data] = h;
   return h;
}

// As on Windows, the HGLOBAL is the data's address: code uses it directly
// without LockResource (Wine's propsheet.c, for one).
HGLOBAL WINAPI LoadResource(HMODULE, HRSRC resource)
{
   ResourceHandle *r = ResourceHandles().Get(resource);
   return r ? (HGLOBAL) r->data : nullptr;
}

LPVOID WINAPI LockResource(HGLOBAL h)
{
   return (LPVOID) h;
}

DWORD WINAPI SizeofResource(HMODULE, HRSRC resource)
{
   ResourceHandle *r = ResourceHandles().Get(resource);
   return r ? r->size : 0;
}

BOOL WINAPI FreeResource(HGLOBAL)
{
   return FALSE;
}

INT WINAPI LoadStringA(HINSTANCE instance, UINT id, LPSTR buffer, INT size)
{
   // String tables: block (id / 16) + 1 holds 16 counted UTF-16 strings.
   const BYTE *data;
   DWORD length;
   if (buffer == nullptr || size <= 0)
      return 0;
   buffer[0] = 0;
   if (!FindResourceData(instance, MAKEINTRESOURCEA((id >> 4) + 1), (LPCSTR) RT_STRING, &data, &length))
      return 0;
   const WORD *p = (const WORD *) data, *end = (const WORD *) (data + length);
   for (UINT i = 0; i < (id & 15) && p < end; i++)
      p += 1 + *p;
   if (p >= end || *p == 0)
      return 0;
   INT n = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) (p + 1), *p, buffer, size - 1, nullptr, nullptr);
   buffer[n] = 0;
   return n;
}

HBITMAP WINAPI LoadBitmapA(HINSTANCE instance, LPCSTR name)
{
   const BYTE *data;
   DWORD size;
   if (!FindResourceData(instance, name, (LPCSTR) RT_BITMAP, &data, &size))
      return nullptr;
   // A packed DIB: header, color table, bits (bottom-up unless the height is negative).
   const BITMAPINFO *bi = (const BITMAPINFO *) data;
   const BITMAPINFOHEADER &h = bi->bmiHeader;
   int colors = h.biBitCount <= 8 ? (h.biClrUsed ? h.biClrUsed : 1 << h.biBitCount) : 0;
   const BYTE *bits = data + h.biSize + colors * sizeof(RGBQUAD) +
                      ((h.biCompression == BI_BITFIELDS) ? 12 : 0);
   void *out = nullptr;
   HBITMAP bmp = CreateDIBSection(nullptr, bi, DIB_RGB_COLORS, &out, nullptr, 0);
   if (bmp == nullptr)
      return nullptr;
   int stride = ((h.biWidth * h.biBitCount + 31) / 32) * 4;
   memcpy(out, bits, std::min<size_t>((size_t) stride * abs(h.biHeight), size - (bits - data)));
   return bmp;
}
