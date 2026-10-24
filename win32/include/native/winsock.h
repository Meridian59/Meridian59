/*
 * winsock.h:  Wine's winsock.h, which declares htons and friends as functions;
 *   macOS's <machine/endian.h> (which the C library's headers include) defines
 *   them as macros.  It goes first, so including it again later changes nothing.
 */

#ifdef __APPLE__
// And macOS's stdlib.h, unlike glibc's, doesn't bring in select's declarations:
// they go first too, so winsock.h takes them as already there, as on Linux.
#include <sys/select.h>
#include <machine/endian.h>
#undef htonl
#undef htons
#undef ntohl
#undef ntohs
#endif

#include_next <winsock.h>
