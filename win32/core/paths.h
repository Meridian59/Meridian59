/*
 * paths.h:  Windows file names on a POSIX file system.
 */

#ifndef W32_CORE_PATHS_H
#define W32_CORE_PATHS_H

#include "core/win32.h"

#include <string>

namespace w32 {

// path with '/' separators, without a drive, and with each component that does
// not exist as written matched to an existing entry ignoring case.
std::string HostPath(const char *path);

}  // namespace w32

#endif
