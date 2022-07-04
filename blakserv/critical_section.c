// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * critical_section.c
 */
#include "blakserv.h"

void InitializeCriticalSection(CRITICAL_SECTION *m)
{
  // Make the mutex reentrant, the same way a Windows critical
  // section is.
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

  pthread_mutex_init(m, &attr);
}

void EnterCriticalSection(CRITICAL_SECTION *m)
{
   pthread_mutex_lock(m);
}

void LeaveCriticalSection(CRITICAL_SECTION *m)
{
   pthread_mutex_unlock(m);
}

void DeleteCriticalSection(CRITICAL_SECTION *m)
{
   pthread_mutex_destroy(m);
}
