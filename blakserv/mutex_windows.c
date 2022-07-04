// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

#include "blakserv.h"

Mutex MutexCreate() {
   return CreateMutex(NULL, FALSE, NULL);
}

bool MutexAcquire(Mutex mutex) {
   return WaitForSingleObject(mutex, INFINITE) == WAIT_OBJECT_0;
}

bool MutexAcquireWithTimeout(Mutex mutex, int timeoutMs) {
   return WaitForSingleObject(mutex, timeoutMs) == WAIT_OBJECT_0;
}

bool MutexRelease(Mutex mutex) {
   return ReleaseMutex(mutex) != 0;
}

bool MutexClose(Mutex mutex) {
   return CloseHandle(mutex) != 0;
}
