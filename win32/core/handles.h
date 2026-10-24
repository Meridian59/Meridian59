/*
 * handles.h:  Handle tables.  A Win32 handle names an object only while it lives:
 *   calls with a stale or invalid handle fail quietly, as on Windows, instead of
 *   touching freed memory.  Values are never reused within a run.
 */

#ifndef W32_CORE_HANDLES_H
#define W32_CORE_HANDLES_H

#include "core/win32.h"

#include <cstdint>
#include <mutex>
#include <unordered_map>

namespace w32 {

// Objects of one kind (kernel objects, GDI objects, windows) under handles of
// type H.  Thread-safe: kernel objects are made and closed on any thread.
template <class T, class H>
class HandleTable
{
public:
   // first: the lowest handle value, so kinds don't overlap (GetObjectType and
   // CloseHandle tell kinds apart by value); step keeps values aligned like
   // Windows' (multiples of 4).
   HandleTable(uintptr_t first, uintptr_t step = 4) : next_(first), step_(step) {}

   H Add(T *object)
   {
      std::lock_guard<std::mutex> lock(mutex_);
      uintptr_t value = next_;
      next_ += step_;
      objects_[value] = object;
      return (H) value;
   }

   // The object h names, or nullptr.
   T *Get(H h) const
   {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = objects_.find((uintptr_t) h);
      return it == objects_.end() ? nullptr : it->second;
   }

   // Forgets h; returns its object (the caller frees it), or nullptr.
   T *Remove(H h)
   {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = objects_.find((uintptr_t) h);
      if (it == objects_.end())
         return nullptr;
      T *object = it->second;
      objects_.erase(it);
      return object;
   }

   template <class F> void ForEach(F f) const
   {
      std::lock_guard<std::mutex> lock(mutex_);
      for (auto &entry : objects_)
         f((H) entry.first, entry.second);
   }

private:
   mutable std::mutex mutex_;
   std::unordered_map<uintptr_t, T *> objects_;
   uintptr_t next_, step_;
};

// Ranges of handle values by kind.
constexpr uintptr_t kKernelHandleBase = 0x00010000;   // HANDLE: threads, events, files ...
constexpr uintptr_t kGdiHandleBase    = 0x10000000;   // HGDIOBJ, HDC
constexpr uintptr_t kWindowHandleBase = 0x20000000;   // HWND
constexpr uintptr_t kMenuHandleBase   = 0x30000000;   // HMENU
constexpr uintptr_t kModuleHandleBase = 0x40000000;   // HMODULE, HINSTANCE, HRSRC, HGLOBAL (resources)
constexpr uintptr_t kInternetHandleBase = 0x50000000; // HINTERNET
constexpr uintptr_t kMiscHandleBase   = 0x60000000;   // HHOOK, HCURSOR, HICON, HIMAGELIST ...

}  // namespace w32

#endif
