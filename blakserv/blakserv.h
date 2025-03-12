// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * blakserv.h
 *
 */

#ifndef _BLAKSERV_H
#define _BLAKSERV_H

/* charlie: i moved the defines above the local includes, as some of
	them seem to be required in them, and since some of the include
	filenames clash with that of most compilers */

#define BLAKSERV_VERSION "2.4"

#define MAX_LOGIN_NAME 50
#define MAX_LOGIN_PASSWORD 32

#define INVALID_CLASSVAR -1
#define INVALID_PROPERTY -1
#define INVALID_OBJECT -1
#define INVALID_TAG -1
#define INVALID_DATA -1
#define INVALID_ID -1
#define INVALID_CLASS -1
#define INVALID_DSTR -1
#define NO_SUPERCLASS 0

#define MAX_DEPTH 2000

enum
{
   USER_CLASS = 1,
   USER_ENTER_GAME_MSG = 2,
   SESSION_ID_PARM = 3,
   SYSTEM_CLASS = 4,
   SYSTEM_PARM = 5,
   RECEIVE_CLIENT_MSG = 6,
   CLIENT_PARM = 7,
   GARBAGE_MSG = 8,
   LOADED_GAME_MSG = 9,
   CONSTRUCTOR_MSG = 10,
   NUMBER_STUFF_PARM = 12,
   GARBAGE_DONE_MSG = 13,
   STRING_PARM = 14,
   SYSTEM_STRING_MSG = 15,
   USER_NAME_MSG = 16,
   USER_ICON_MSG = 17,
   SEND_CHAR_INFO_MSG = 18,
   NEW_HOUR_MSG = 19,
   GUEST_CLASS = 20,
   NAME_PARM = 21,
   ICON_PARM = 22,
   ADMIN_CLASS = 23,
   SYSTEM_ENTER_GAME_MSG = 24,
   FIND_USER_BY_INTERNET_NAME_MSG = 25,
   RECEIVE_INTERNET_MAIL_MSG = 26,
   PERM_STRING_PARM = 27,
   IS_FIRST_TIME_MSG = 28,
   DELETE_MSG = 29,
   TIMER_PARM = 30,
   TYPE_PARM = 31,
   DM_CLASS = 32,
   FIND_USER_BY_STRING_MSG = 33,
   CREATOR_CLASS = 34,
   RECYCLED_MSG = 35,
   OLD_PARM = 36

   // To add other C-accessible KOD identifiers,
   // see the BLAKCOMP's table of BuiltinIds[].
   //
   // The compiler assumes those builtins before
   // it reads any KODBASE.TXT.
};

#define MAX_PROC_TIME 5000
#define TIMER_DELAY_WARN 2000

#define COMM_READ_TIMEOUT_MS 30000

#define LEN_MAX_CLIENT_MSG 6000

#define NO_WALL 0
#define MIN_WALL 1
#define MAX_WALL 1000
#define NO_WALK_FLOOR 999

#define MIN_DYNAMIC_RSC 1000000

#define CONFIG_FILE "blakserv.cfg"

/* these three get the date/time appended to them */
#define ACCOUNT_FILE_SAVE "accounts."
#define GAME_FILE_SAVE "gameuser."
#define STRING_FILE_SAVE "striings."
#define DYNAMIC_RSC_FILE_SAVE "dynarscs."

#define SAVE_CONTROL_FILE "lastsave.txt"

#define MOTD_FILE "motd.txt"
#define REGFORM_FILE "regform.txt"
#define NOTE_FILE "admnote.txt"
#define PROFANE_FILE "profane.txt"

#define DEBUG_FILE_BASE "debug"
#define ERROR_FILE_BASE "error"
#define LOG_FILE_BASE "log"

#define KODBASE_FILE "kodbase.txt"

#define PACKAGE_FILE "packages.txt"
#define SPROCKET_FILE "sprocket.dll"

#include <string>
#include <vector>
typedef std::vector<std::string> StringVector;

#ifdef BLAK_PLATFORM_WINDOWS
#include "osd_windows.h"
#endif  // BLAK_PLATFORM_WINDOWS

#ifdef BLAK_PLATFORM_LINUX
#include "osd_linux.h"
#endif  // BLAK_PLATFORM_LINUX

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <cinttypes>

#include "btime.h"

#include "bool.h"
#include "rscload.h"
#include "roomtype.h"
#include "bkod.h"
#include "crc.h"
#include "md5.h"

// Originally, we stored Bkod values in a 32-bit int, but due to the function
// GetTime(), which stored millisecond-resolution times in 28-bit Bkod values,
// the time values would roll over every 8 years or so.  The workaround is
// to store Bkod values in 64-bit ints at runtime, although they are still
// 32 bits in the bof files and in client communication.  This effectively
// expands the range of Bkod values to 32 bits, delaying time rollover visible to
// to clients to 8*16 > 100 years.
typedef INT64 blak_int;

typedef struct
{
   INT64 data:60;
   UINT64 tag:4;
} server_constant_type;

typedef union 
{
   blak_int int_val;
   server_constant_type v;
} val_type;

typedef struct
{
   blak_int value;
   int name_id; /* for call-by-name parm list only */
   char type; /* for normal c parms (not call by name) only */
} parm_node;

typedef struct
{
   unsigned short len;
   unsigned short crc16;
   unsigned short len_verify;
   unsigned char seqno; /* 0 = synched, 1-255 = game epoch */
   char data[LEN_MAX_CLIENT_MSG];
} client_msg;

/* in main.c */
extern DWORD main_thread_id;
#define WM_BLAK_MAIN_READ           (WM_APP + 4000)
#define WM_BLAK_MAIN_RECALIBRATE    (WM_APP + 4001)
#define WM_BLAK_MAIN_DELETE_ACCOUNT (WM_APP + 4002)
#define WM_BLAK_MAIN_VERIFIED_LOGIN (WM_APP + 4003)

#include "bof.h"

#include "config.h"

// these hashes pre-date the use of C++
#include "stringinthash.h"
#include "intstringhash.h"

#include "blakres.h"
#include "channel.h"
#include "kodbase.h"
#include "message.h"
#include "class.h"
#include "object.h"
#include "list.h"
#include "loadkod.h"
#include "sendmsg.h"
#include "ccode.h"
#include "timer.h"
#include "account.h"
#include "user.h"
#include "system.h"
#include "loadrsc.h"
#include "loadgame.h"
#include "roomdata.h"
#include "roofile.h"

#include "bufpool.h"
#include "admin.h"
#include "game.h"
#include "trysync.h"
#include "synched.h"
#include "resync.h"
#include "session.h"

#include "term.h"

#include "proto.h"
#include "commcli.h"
#include "parsecli.h"
#include "sprocket.h"
#include "bstring.h"
#include "admin.h"
#include "garbage.h"
#include "savegame.h"

#include "loadacco.h"
#include "saveacco.h"
#include "savestr.h"
#include "loadstr.h"
#include "nameid.h"
#include "time.h"
#include "dllist.h"
#include "motd.h"
#include "gamelock.h"
#include "apndfile.h"
#include "builtin.h"
#include "version.h"
#include "rscfile.h"
#include "systimer.h"
#include "memory.h"

#include "interface.h"
#include "intrlock.h"
#include "chanbuf.h"

#include "saveall.h"
#include "loadall.h"

#include "saversc.h"

#include "adminfn.h"

#include "async.h"
#include "debug.h"

#include "admincons.h"

#include "table.h"

#include "maintenance.h"
#include "block.h"

#endif

