/*
 * winsock_redirect.h:  The Windows sources call Winsock with Wine's winsock.h
 *   structs (SOCKET, sockaddr without sin_len, hostent with short fields) and
 *   read errors with WSAGetLastError; the C library's functions of the same
 *   names take its own structs and set errno.  Each call binds to the layer's
 *   m59_ function instead (core/net/winsock.cpp).  Every function winsock.h
 *   declares is renamed, so one the layer lacks fails to link rather than
 *   reaching the C library.  select is left alone: winsock.h disables it when
 *   the C library's headers came first, as they do here.
 */

#ifndef M59_WINSOCK_REDIRECT_H
#define M59_WINSOCK_REDIRECT_H

M59_REDIRECT(accept)
M59_REDIRECT(bind)
M59_REDIRECT(closesocket)
M59_REDIRECT(connect)
M59_REDIRECT(gethostbyaddr)
M59_REDIRECT(gethostbyname)
M59_REDIRECT(getpeername)
M59_REDIRECT(getprotobyname)
M59_REDIRECT(getprotobynumber)
M59_REDIRECT(getservbyname)
M59_REDIRECT(getservbyport)
M59_REDIRECT(getsockname)
M59_REDIRECT(getsockopt)
M59_REDIRECT(htonl)
M59_REDIRECT(htons)
M59_REDIRECT(inet_addr)
M59_REDIRECT(inet_ntoa)
M59_REDIRECT(ioctlsocket)
M59_REDIRECT(listen)
M59_REDIRECT(ntohl)
M59_REDIRECT(ntohs)
M59_REDIRECT(recv)
M59_REDIRECT(recvfrom)
M59_REDIRECT(send)
M59_REDIRECT(sendto)
M59_REDIRECT(setsockopt)
M59_REDIRECT(shutdown)
M59_REDIRECT(socket)

#endif
