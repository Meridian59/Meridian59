/*
 * net.h:  What the network sources (Winsock, WinINet, shell) share.
 */

#ifndef W32_CORE_NET_NET_H
#define W32_CORE_NET_NET_H

#include "core/win32.h"

namespace w32::net {

// The thread's last error, which GetLastError and WSAGetLastError both read.
void  SetError(DWORD error);
DWORD GetError();

}  // namespace w32::net

#endif
