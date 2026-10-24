/* io.h:  MSVC low-level I/O, as POSIX. */
#ifndef M59_IO_H
#define M59_IO_H
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#ifndef _O_BINARY
#define _O_BINARY 0
#define O_BINARY  0
#define _O_TEXT   0
#define O_TEXT    0
#endif
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_RDWR   O_RDWR
#define _O_CREAT  O_CREAT
#define _O_TRUNC  O_TRUNC
#define _S_IREAD  S_IRUSR
#define _S_IWRITE S_IWUSR
#define _open   open
#define _close  close
#define _read   read
#define _write  write
#define _lseek  lseek
#define _access access
#define _unlink unlink
#define _isatty isatty
#endif
