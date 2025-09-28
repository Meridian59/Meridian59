// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * osd_linux.h
 *
 */

#ifndef _OSD_LINUX_H
#define _OSD_LINUX_H

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/epoll.h>

#define MAX_PATH PATH_MAX
#define O_BINARY 0
#define O_TEXT 0
#define stricmp strcasecmp
#define strnicmp strncasecmp

typedef int SOCKET;
#define closesocket close
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define WSAEWOULDBLOCK EWOULDBLOCK
typedef sockaddr SOCKADDR;
typedef sockaddr_in SOCKADDR_IN;
typedef struct in_addr IN_ADDR;
extern int GetLastError();

#define FD_CLOSE 32
#define FD_ACCEPT 8
#define FD_READ 1
#define FD_WRITE 2

#define VER_PLATFORM_WIN32_WINDOWS 1
#define VER_PLATFORM_WIN32_NT 2
#define PROCESSOR_INTEL_386 386
#define PROCESSOR_INTEL_486 486
#define PROCESSOR_INTEL_PENTIUM 586

typedef int DWORD;
typedef int HANDLE;
typedef int HINSTANCE;
typedef int HWND;
typedef uint64_t UINT64;
typedef int64_t INT64;

#define MAXGETHOSTSTRUCT 64

void RunMainLoop(void);
int GetLastError();
char * GetLastErrorStr();

// Fill in "files" with the names of all files matching the given pattern.
// Return true on success.
bool FindMatchingFiles(const char *path, const char *extension, StringVector *files);

bool BlakMoveFile(const char *source, const char *dest);

void InitInterface(void);

int GetUsedSessions(void);

void StartupPrintf(const char *fmt,...);
void StartupComplete(void);

void InterfaceUpdate(void);
void InterfaceLogon(void *s);
void InterfaceLogoff(void *s);
void InterfaceUpdateSession(void *s);
void InterfaceUpdateChannel(void);
void InterfaceSignalConsole();

void InterfaceSendBufferList(void *blist);
void InterfaceSendBytes(char *buf,int len_buf);

void StartAsyncSocketAccept(SOCKET sock,int connection_type);
HANDLE StartAsyncNameLookup(char *peer_addr,char *buf);
void StartAsyncSession(void *s);

void FatalErrorShow(const char *filename,int line,const char *str);

#endif
