// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * club.h
 *
 */

#ifndef _CLUB_H
#define _CLUB_H


#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdarg.h>
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <process.h>
#include "wininet.h"

#define Bool char
#define True 1
#define False 0

/* our special window messages */
#define CM_DEARCHIVE  (WM_USER + 1002)
#define CM_ASYNCDONE  (WM_USER + 1003)
#define CM_RETRYABORT (WM_USER + 1004)
#define CM_FILESIZE   (WM_USER + 1005)
#define CM_PROGRESS   (WM_USER + 1006)

#include "resource.h"
#include "clubarchive.h"
#include "util.h"
#include "transfer.h"

/* libarchive */
#include "archive.h"
#include "archive_entry.h"

#define sprintf wsprintf


/* timer ID's */
#define TIMER_START_TRANSFER 2

void Status(char *fmt, ...);
void Error(char *fmt, ...);
char *GetLastErrorStr();

extern HINSTANCE hInst;
extern HWND hwndMain;
extern char *transfer_machine;
extern char *transfer_filename;
extern char *transfer_local_filename;

extern Bool success;

#endif
