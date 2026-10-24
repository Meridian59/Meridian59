/* process.h:  MSVC threads and processes, on pthreads. */
#ifndef M59_PROCESS_H
#define M59_PROCESS_H
#include <stdint.h>
#include <unistd.h>
#ifdef __cplusplus
extern "C" {
#endif
// Starts start_address(arglist) on a new thread; returns -1 on failure.
uintptr_t _beginthread(void (*start_address)(void *), unsigned stack_size, void *arglist);
#ifdef __cplusplus
}
#endif
#define _getpid getpid
#endif
