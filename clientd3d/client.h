// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * client.h
 */

#ifndef _CLIENT_H
#define _CLIENT_H

#pragma warning(disable: 4244) // cast double to float loses information

#ifndef _INC_WINDOWS
#include <windows.h>
#include <windowsx.h>
#include "winxblak.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <commdlg.h>
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <sys\stat.h>
#include <dlgs.h>
#include <mmsystem.h>
#include <winsock.h>
#include <math.h>
#include <commctrl.h>
#include <richedit.h>
#include <process.h>
#include <tchar.h>
#include "wininet.h"

#include <d3d9.h>
#include <DxErr.h>

typedef unsigned char Bool;
enum {False = 0, True = 1};

#define MAJOR_REV 50   /* Major version of client program */
#define MINOR_REV 35  /* Minor version of client program; must be in [0, 99] */

#define MAXAMOUNT 9     /* Max # of digits in a server integer */
#define MAXSTRINGLEN 512 /* Max length of a string loaded from string table */

#define MAX_LANGUAGE_ID 184 /* Max number of languages */

/* Main window size & position */
#define MAIN_DEF_LEFT     0
#define MAIN_DEF_TOP      0
#define MAIN_DEF_WIDTH  693
#define MAIN_DEF_HEIGHT 554

/* Timer identifiers */
#define TIMER_ANIMATE   2

/* Due to problems with packet sniffers, include this constant in the AP_REQ_GAME packet */
/* To make sure we are using the right version of the client */
#define P_CATCH 3

extern void GetGamePath( char *szGamePath );

extern long CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
extern void ClearMessageQueue(void);


extern Bool is_foreground;   // True when program is in the foreground

/* This list of include files is good for precompiled headers */
/* The __cplusplus block and M59EXPORT symbol enable mixed C and C++ modules and client */

#ifdef __cplusplus
extern "C" { };
#define M59EXPORT extern "C"
#else
#define M59EXPORT /* nothing */
#endif

// Remove debugging strings in final version
#ifndef NODPRINTFS
#define debug(x) (dprintf x)
#else
#define debug(x)
#endif

M59EXPORT void _cdecl dprintf(char *fmt,...);

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "resource.h"
#include "proto.h"
#include "list.h"
#include "util.h"
#include "table.h"
#include "messages.h"
#include "dibutil.h"
#include "object.h"
#include "idlist.h"
#include "annotate.h"
#include "fixed.h"
#include "draw3d.h"
#include "animate.h"
#include "roomanim.h"
#include "bsp.h"
#include "room.h"
#include "object3d.h"
#include "project.h"
#include "boverlay.h"
#include "game.h"
#include "gameuser.h"
#include "winmsg.h"
#include "ownerdrw.h"
#include "loadrsc.h"
#include "protocol.h"
#include "dialog.h"
#include "bitmap.h"
#include "graphics.h"
#include "drawbmp.h"
#include "draw.h"
#include "fixedpt.h"
#include "trig.h"
#include "login.h"
#include "intrface.h"
#include "server.h"
#include "com.h"
#include "statterm.h"
#include "statgame.h"
#include "font.h"
#include "editbox.h"
#include "maindlg.h"
#include "color.h"
#include "offer.h"
#include "buy.h"
#include "key.h"
#include "say.h"
#include "statstrt.h"
#include "effect.h"
#include "download.h"
#include "select.h"
#include "logoff.h"
#include "statoffl.h"
#include "cursor.h"
#include "graphctl.h"
#include "client3d.h"
#include "msgbox.h"
#include "uselist.h"
#include "move.h"
#include "startup.h"
#include "audio.h"
#include "config.h"
#include "palette.h"
#include "module.h"     // header common to client and module files
#include "modules.h"
#include "textin.h"
#include "toolbar.h"
#include "drawbsp.h"
#include "map.h"
#include "bspload.h"
#include "parse.h"
#include "winmenu.h"
#include "cache.h"
#include "overlay.h"
#include "moveobj.h"
#include "msgfiltr.h"
#include "statconn.h"
#include "memmap.h"
#include "mapfile.h"
#include "srvrstr.h"
#include "tooltip.h"
#include "lagbox.h"
#include "transfer.h"
#include "hook.h"
#include "web.h"
#include "about.h"
#include "md5.h"
#include "xlat.h"
#include "rops.h"
#include "guest.h"
#include "ping.h"
#include "objdraw.h"
#include "profane.h"
#include "png.h"
#include "pngstruct.h"
#include "d3dtypes.h"
#include "d3dcache.h"
#include "d3drender.h"
#include "d3dparticle.h"
#include "matrix.h"
#include "xform.h"
#include "d3ddriver.h"
#include "rscload.h"
#include "crc.h"

// Only include externs if compiling main client
#ifdef BLAKCLIENT
#include "extern.h"
#endif

#endif /* #ifndef _CLIENT_H */
