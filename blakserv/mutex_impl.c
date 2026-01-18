// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

#include "blakserv.h"
#include <chrono>

Mutex MutexCreate() {
  return new std::recursive_timed_mutex();
}

bool MutexAcquire(Mutex mutex) {
  mutex->lock();
  return true;
}

bool MutexAcquireWithTimeout(Mutex mutex, int timeoutMs) {
	return mutex->try_lock_for(std::chrono::milliseconds(timeoutMs));
}

bool MutexRelease(Mutex mutex) {
  mutex->unlock();
  return true;
}

bool MutexClose(Mutex mutex) {
  delete mutex;
  return true;
}
