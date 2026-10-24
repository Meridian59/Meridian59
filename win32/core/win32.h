/*
 * win32.h:  What the layer's own sources share: the Windows headers and the
 *   export marker for the API it implements.
 */

#ifndef W32_CORE_WIN32_H
#define W32_CORE_WIN32_H

#ifndef NOMINMAX
#define NOMINMAX   // windef.h's min/max macros break the C++ library
#endif
#include <windows.h>

// The layer is built with hidden visibility; the API it implements is exported.
#define W32_EXPORT __attribute__((visibility("default")))

#endif
