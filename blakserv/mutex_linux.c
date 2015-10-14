// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

#include "blakserv.h"

Mutex MutexCreate() {
   Mutex mutex = new MutexWrapper();
   pthread_mutex_init(&mutex->mutex, NULL);
}

bool MutexAcquire(Mutex mutex) {
   return pthread_mutex_lock(&mutex->mutex) == 0;
}

bool MutexAcquireWithTimeout(Mutex mutex, int timeoutMs) {
   timespec tspec;
   tspec.tv_sec = 0;
   tspec.tv_nsec = timeoutMs * 1000L * 1000L;
   return pthread_mutex_timedlock(&mutex->mutex, &tspec);
}

bool MutexRelease(Mutex mutex) {
   return pthread_mutex_unlock(&mutex->mutex) == 0;
}

bool MutexClose(Mutex mutex) {
   return pthread_mutex_destroy(&mutex->mutex) == 0;
}
