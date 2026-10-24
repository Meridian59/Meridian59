/*
 * kernel.h:  Kernel objects (threads, semaphores, files, mappings, find
 *   searches) under one handle table, so CloseHandle frees any kind, and the
 *   errno -> Win32 error mapping the layer's file and socket calls share.
 */

#ifndef W32_CORE_KERNEL_KERNEL_H
#define W32_CORE_KERNEL_KERNEL_H

#include "core/win32.h"

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace w32 {

enum class KernelKind { Thread, Semaphore, Event, Mutex, File, Mapping, Find };

// Reference counted: the handle holds one reference; a running thread and a
// call blocked in a wait hold their own, so CloseHandle on another thread never
// frees an object under them.
class KernelObject
{
public:
   explicit KernelObject(KernelKind kind) : kind_(kind) {}
   virtual ~KernelObject() = default;

   KernelKind Kind() const { return kind_; }
   void AddRef() { refs_.fetch_add(1, std::memory_order_relaxed); }
   void Release()
   {
      if (refs_.fetch_sub(1, std::memory_order_acq_rel) == 1)
         delete this;
   }

private:
   const KernelKind kind_;
   std::atomic<int> refs_{1};
};

// Objects WaitForSingleObject accepts.  All waits share one lock and condition
// variable (kernel.cpp): signals are rare, and waits on several objects at
// once (WaitForMultipleObjects) need nothing more.
class Waitable : public KernelObject
{
public:
   using KernelObject::KernelObject;
   // Called with WaitLock() held.  Signaled(): the wait can be satisfied;
   // Acquire(): it is, so consume the signal (a semaphore count, an
   // auto-reset event).
   virtual bool Signaled() const = 0;
   virtual void Acquire() {}
};

std::mutex &WaitLock();
// After changing an object's state under WaitLock().
void WakeWaiters();

// A new handle for object (taking over its reference), or nullptr when out of
// handles.
HANDLE AddKernelObject(KernelObject *object);

// The object h names, with a reference the caller releases, or nullptr (and
// ERROR_INVALID_HANDLE); with a kind, only an object of that kind.
KernelObject *RefKernelObject(HANDLE h);
KernelObject *RefKernelObject(HANDLE h, KernelKind kind);

// Closes h if it names a find search (find) or any other object (!find), as
// FindClose and CloseHandle do; else FALSE and ERROR_INVALID_HANDLE.
BOOL CloseKernelHandle(HANDLE h, bool find);

// Holds a reference from RefKernelObject.
template <class T> class KernelRef
{
public:
   explicit KernelRef(KernelObject *object) : object_(static_cast<T *>(object)) {}
   ~KernelRef() { if (object_ != nullptr) object_->Release(); }
   KernelRef(const KernelRef &) = delete;
   KernelRef &operator=(const KernelRef &) = delete;
   T *operator->() const { return object_; }
   T *get() const { return object_; }
   explicit operator bool() const { return object_ != nullptr; }
private:
   T *object_;
};

// The Win32 error for an errno value (ENOENT -> ERROR_FILE_NOT_FOUND ...).
DWORD ErrorFromErrno(int error);
// SetLastError(ErrorFromErrno(errno)).
void SetLastErrorFromErrno();

}  // namespace w32

#endif
