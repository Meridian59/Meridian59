// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

#ifndef _MUTEX_IMPL_H
#define _MUTEX_IMPL_H

#include <mutex>

typedef std::recursive_timed_mutex *Mutex;

Mutex MutexCreate();
bool MutexAcquire(Mutex mutex);
bool MutexAcquireWithTimeout(Mutex mutex, int timeoutMs);
bool MutexRelease(Mutex mutex);
bool MutexClose(Mutex mutex);


#endif
