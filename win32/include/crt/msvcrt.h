/*
 * msvcrt.h:  The Microsoft C runtime functions the Windows sources call that the
 *   C library here lacks.  Included ahead of every source (-include).
 */

#ifndef M59_MSVCRT_H
#define M59_MSVCRT_H

/* File names in the Windows sources use '\' and any case; their calls into the C
 * library go through the Win32 layer, which translates them (core/paths.cpp).
 * redefine_extname renames only the symbol a call binds to, not the text, so
 * std::remove and members named open are untouched.  The new name is the
 * assembler's, with Mach-O's leading underscore on macOS. */
#define M59_PRAGMA(x) _Pragma(#x)
#ifdef __APPLE__
#define M59_REDIRECT(name) M59_PRAGMA(redefine_extname name _m59_##name)
#else
#define M59_REDIRECT(name) M59_PRAGMA(redefine_extname name m59_##name)
#endif
#ifdef __APPLE__
/* Apple's C library declares these with assembler names (fopen's own, stat's
 * "$INODE64" ...), which redefine_extname leaves alone: after its declarations,
 * calls are redirected by name.  Function-like, so struct stat and std::remove
 * stay. */
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __cplusplus
extern "C" {
#endif
FILE *m59_fopen(const char *path, const char *mode);
FILE *m59_freopen(const char *path, const char *mode, FILE *stream);
int m59_open(const char *path, int flags, ...);
int m59_stat(const char *path, struct stat *st);
int m59_access(const char *path, int mode);
int m59_unlink(const char *path);
int m59_remove(const char *path);
int m59_rename(const char *from, const char *to);
int m59_mkdir(const char *path, mode_t mode);
int m59_rmdir(const char *path);
int m59_chdir(const char *path);
DIR *m59_opendir(const char *path);
#ifdef __cplusplus
}
#endif
#define fopen(...) m59_fopen(__VA_ARGS__)
#define freopen(...) m59_freopen(__VA_ARGS__)
#define open(...) m59_open(__VA_ARGS__)
#define stat(...) m59_stat(__VA_ARGS__)
#define access(...) m59_access(__VA_ARGS__)
#define unlink(...) m59_unlink(__VA_ARGS__)
#define remove(...) m59_remove(__VA_ARGS__)
#define rename(...) m59_rename(__VA_ARGS__)
#define mkdir(...) m59_mkdir(__VA_ARGS__)
#define rmdir(...) m59_rmdir(__VA_ARGS__)
#define chdir(...) m59_chdir(__VA_ARGS__)
#define opendir(...) m59_opendir(__VA_ARGS__)
#else
M59_REDIRECT(fopen)
M59_REDIRECT(freopen)
M59_REDIRECT(open)
M59_REDIRECT(stat)
M59_REDIRECT(access)
M59_REDIRECT(unlink)
M59_REDIRECT(remove)
M59_REDIRECT(rename)
M59_REDIRECT(mkdir)
M59_REDIRECT(rmdir)
M59_REDIRECT(chdir)
M59_REDIRECT(opendir)
#endif
/* libarchive opens the client's downloaded packages by their Windows names too;
 * it declares its functions without assembler names, so on both systems. */
M59_REDIRECT(archive_read_open_filename)
#include "winsock_redirect.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>   // MSVC's headers bring in std::intptr_t and friends
#endif

/* The client's own debug printf is named dprintf, as POSIX's fd printf is: the
 * game's is renamed everywhere after stdio.h has declared POSIX's. */
#define dprintf m59_dprintf

/* MSVC's command line globals, which the layer fills in before WinMain. */
#ifdef __cplusplus
extern "C" {
#endif
extern int __argc;
extern char **__argv;
#ifdef __cplusplus
}
#endif

#ifndef _TRUNCATE
#define _TRUNCATE ((size_t) -1)
#endif

typedef int errno_t;

// MSVC's sized integer keywords.
#define __int8  char
#define __int16 short
#define __int32 int
#define __int64 __INT64_TYPE__   // int64_t, in keywords: signed/unsigned apply

#define _stricmp  strcasecmp
#define _strnicmp strncasecmp
#define stricmp   strcasecmp
#define strnicmp  strncasecmp
#define _strdup   strdup
#define _snprintf snprintf
#define _vsnprintf vsnprintf

static inline errno_t strcpy_s(char *dst, size_t size, const char *src)
{
   if (dst == NULL || size == 0)
      return EINVAL;
   size_t n = strlen(src);
   if (n >= size)
   {
      dst[0] = 0;
      return ERANGE;
   }
   memcpy(dst, src, n + 1);
   return 0;
}

static inline errno_t strncpy_s(char *dst, size_t size, const char *src, size_t count)
{
   if (dst == NULL || size == 0)
      return EINVAL;
   size_t n = strnlen(src, count == _TRUNCATE ? size - 1 : count);
   if (n >= size)
      n = size - 1;
   memcpy(dst, src, n);
   dst[n] = 0;
   return 0;
}

static inline errno_t strcat_s(char *dst, size_t size, const char *src)
{
   size_t used = strnlen(dst, size);
   if (used == size)
      return EINVAL;
   return strcpy_s(dst + used, size - used, src);
}

static inline char *_strupr(char *s)
{
   for (char *p = s; *p; p++)
      *p = (char) toupper((unsigned char) *p);
   return s;
}

static inline char *_strlwr(char *s)
{
   for (char *p = s; *p; p++)
      *p = (char) tolower((unsigned char) *p);
   return s;
}
#define strupr _strupr

static inline char *_itoa(int value, char *buf, int radix)
{
   if (radix == 10)
      sprintf(buf, "%d", value);
   else
   {
      // Other radixes print the value as unsigned, as MSVC's does.
      const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
      unsigned v = (unsigned) value;
      char tmp[33];
      int n = 0;
      do { tmp[n++] = digits[v % radix]; v /= radix; } while (v != 0);
      for (int i = 0; i < n; i++)
         buf[i] = tmp[n - 1 - i];
      buf[n] = 0;
   }
   return buf;
}
#define itoa _itoa
#define strlwr _strlwr

static inline errno_t fopen_s(FILE **file, const char *name, const char *mode)
{
   *file = fopen(name, mode);
   return *file != NULL ? 0 : errno;
}

#ifdef __cplusplus
// MSVC's one-argument mkdir.
inline int mkdir(const char *path) { return mkdir(path, 0777); }

// MSVC's array forms: the size comes from the destination array.
template <size_t N> inline errno_t strcpy_s(char (&dst)[N], const char *src) { return strcpy_s(dst, N, src); }
template <size_t N> inline errno_t strcat_s(char (&dst)[N], const char *src) { return strcat_s(dst, N, src); }
template <size_t N> inline errno_t strncpy_s(char (&dst)[N], const char *src, size_t count)
{
   return strncpy_s(dst, N, src, count);
}
#endif

#endif
