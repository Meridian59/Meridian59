/* malloc.h:  alloca and friends; macOS has no <malloc.h>. */
#ifndef M59_MALLOC_H
#define M59_MALLOC_H
#include <stdlib.h>
#if defined(__APPLE__)
#include <malloc/malloc.h>
#else
#include_next <malloc.h>
#endif
#include <alloca.h>
#define _alloca alloca
#endif
