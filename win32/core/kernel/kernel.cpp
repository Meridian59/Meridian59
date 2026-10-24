/*
 * kernel.cpp:  The kernel handle table, CloseHandle, WaitForSingleObject, and
 *   errno -> Win32 errors.
 */

#include "core/kernel/kernel.h"
#include "core/handles.h"

#include <cerrno>
#include <chrono>

namespace w32 {

namespace {

HandleTable<KernelObject, HANDLE> &Handles()
{
   static HandleTable<KernelObject, HANDLE> handles(kKernelHandleBase);
   return handles;
}

// Held across a lookup and its AddRef, and across a removal, so a handle closed
// on another thread can't free the object in between.
std::mutex handles_lock;

std::condition_variable &WaitCondition()
{
   static std::condition_variable condition;
   return condition;
}

}  // namespace

std::mutex &WaitLock()
{
   static std::mutex lock;
   return lock;
}

void WakeWaiters()
{
   WaitCondition().notify_all();
}

HANDLE AddKernelObject(KernelObject *object)
{
   return Handles().Add(object);
}

KernelObject *RefKernelObject(HANDLE h)
{
   std::lock_guard<std::mutex> lock(handles_lock);
   KernelObject *object = Handles().Get(h);
   if (object == nullptr)
   {
      SetLastError(ERROR_INVALID_HANDLE);
      return nullptr;
   }
   object->AddRef();
   return object;
}

KernelObject *RefKernelObject(HANDLE h, KernelKind kind)
{
   KernelObject *object = RefKernelObject(h);
   if (object != nullptr && object->Kind() != kind)
   {
      object->Release();
      SetLastError(ERROR_INVALID_HANDLE);
      return nullptr;
   }
   return object;
}

DWORD ErrorFromErrno(int error)
{
   switch (error)
   {
   case 0:            return ERROR_SUCCESS;
   case ENOENT:       return ERROR_FILE_NOT_FOUND;
   case ENOTDIR:      return ERROR_PATH_NOT_FOUND;
   case EACCES:
   case EPERM:
   case EISDIR:
   case EROFS:        return ERROR_ACCESS_DENIED;
   case EEXIST:       return ERROR_FILE_EXISTS;
   case EBADF:        return ERROR_INVALID_HANDLE;
   case ENOMEM:       return ERROR_NOT_ENOUGH_MEMORY;
   case EMFILE:
   case ENFILE:       return ERROR_TOO_MANY_OPEN_FILES;
   case ENOSPC:       return ERROR_DISK_FULL;
   case EINVAL:       return ERROR_INVALID_PARAMETER;
   case ENAMETOOLONG: return ERROR_FILENAME_EXCED_RANGE;
   case ENOTEMPTY:    return ERROR_DIR_NOT_EMPTY;
   case EBUSY:
   case ETXTBSY:      return ERROR_SHARING_VIOLATION;
   case EAGAIN:       return ERROR_NOT_ENOUGH_MEMORY;
   case EIO:          return ERROR_GEN_FAILURE;
   case ELOOP:        return ERROR_CANT_RESOLVE_FILENAME;
   case EXDEV:        return ERROR_NOT_SAME_DEVICE;
   case ENODEV:
   case ENXIO:        return ERROR_BAD_UNIT;
   case EFBIG:        return ERROR_FILE_TOO_LARGE;
   default:           return ERROR_GEN_FAILURE;
   }
}

void SetLastErrorFromErrno()
{
   SetLastError(ErrorFromErrno(errno));
}

BOOL CloseKernelHandle(HANDLE h, bool find)
{
   KernelObject *object;
   {
      std::lock_guard<std::mutex> lock(handles_lock);
      object = Handles().Get(h);
      if (object == nullptr || (object->Kind() == KernelKind::Find) != find)
      {
         SetLastError(ERROR_INVALID_HANDLE);
         return FALSE;
      }
      Handles().Remove(h);
   }
   object->Release();
   return TRUE;
}

}  // namespace w32

using namespace w32;

extern "C" {

BOOL WINAPI CloseHandle(HANDLE h)
{
   return CloseKernelHandle(h, false);
}

DWORD WINAPI WaitForSingleObject(HANDLE h, DWORD ms)
{
   KernelRef<KernelObject> object(RefKernelObject(h));
   if (!object)
      return WAIT_FAILED;
   auto *waitable = dynamic_cast<Waitable *>(object.get());
   if (waitable == nullptr)
   {
      SetLastError(ERROR_INVALID_HANDLE);
      return WAIT_FAILED;
   }

   std::unique_lock<std::mutex> lock(WaitLock());
   auto signaled = [waitable] { return waitable->Signaled(); };
   bool ok;
   if (ms == INFINITE)
   {
      WaitCondition().wait(lock, signaled);
      ok = true;
   }
   else
      ok = WaitCondition().wait_for(lock, std::chrono::milliseconds(ms), signaled);
   if (!ok)
      return WAIT_TIMEOUT;
   waitable->Acquire();
   return WAIT_OBJECT_0;
}

}  // extern "C"
