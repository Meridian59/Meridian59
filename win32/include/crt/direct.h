/* direct.h:  MSVC directory functions, as POSIX. */
#ifndef M59_DIRECT_H
#define M59_DIRECT_H
#include <sys/stat.h>
#include <unistd.h>
static inline int _mkdir(const char *path) { return mkdir(path, 0777); }
#define _getcwd getcwd
#define _chdir  chdir
#define _rmdir  rmdir
#endif
