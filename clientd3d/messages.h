// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * messages.h:  User window message codes.
 */

/* User window messages */
#define BK_SOCKETEVENT    (WM_USER + 100)
#define BK_PROCESSBUFFER  (WM_USER + 102)
#define BK_SETDLGFONTS    (WM_USER + 106)
#define BK_SETDLGCOLORS   (WM_USER + 107)
#define BK_COUNTEROFFER   (WM_USER + 108)
#define BK_OFFERDONE      (WM_USER + 109)
#define BK_UPDATEUSERS    (WM_USER + 110)
#define BK_ADDUSER        (WM_USER + 111)
#define BK_REMOVEUSER     (WM_USER + 112)
#define BK_COUNTEROFFERED (WM_USER + 113)
#define BK_ALIGNDLGS      (WM_USER + 114)
//#define BK_NEWSOUND       (WM_USER + 115)
#define BK_DIALOGDONE     (WM_USER + 116)
#define BK_TRANSFERDONE   (WM_USER + 117)
#define BK_TRANSFERSTART  (WM_USER + 118)
#define BK_MODEMMSG       (WM_USER + 119)
#define BK_COMBOCHANGED   (WM_USER + 120)
#define BK_REDIAL         (WM_USER + 123)
#define BK_NONE           (WM_USER + 124)
#define BK_GOTUSERS       (WM_USER + 125)
#define BK_NORESOURCE     (WM_USER + 126)
#define BK_ANIMATE        (WM_USER + 127)
#define BK_CALLWRITE      (WM_USER + 128)
#define BK_MODULEUNLOAD   (WM_USER + 129)
#define BK_FILESIZE       (WM_USER + 130)
#define BK_PROGRESS       (WM_USER + 131)
#define BK_FILEDONE       (WM_USER + 132)
