// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * osd_windows.h
 *
 */

#ifndef _OSD_WINDOWS_H
#define _OSD_WINDOWS_H

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winsock2.h>
#include "resource.h"
#include <crtdbg.h>
#include <io.h>
#include <process.h>
#include "mutex_windows.h"

typedef int socklen_t;

void RunMainLoop(void);
char * GetLastErrorStr();

// Fill in "files" with the names of all files matching the given pattern.
// Return true on success.
bool FindMatchingFiles(const char *path, const char *extension, StringVector *files);

bool BlakMoveFile(const char *source, const char *dest);

// a lot of stuff that really belongs here is in interface.h instead

#endif
