/*----------------------------------------------------------------------------*
 | This file is part of WinDEU, the port of DEU to Windows.                   |
 | WinDEU was created by the DEU team:                                        |
 |  Renaud Paquay, Raphael Quinet, Brendon Wyber and others...                |
 |                                                                            |
 | DEU is an open project: if you think that you can contribute, please join  |
 | the DEU team.  You will be credited for any code (or ideas) included in    |
 | the next version of the program.                                           |
 |                                                                            |
 | If you want to make any modifications and re-distribute them on your own,  |
 | you must follow the conditions of the WinDEU license. Read the file        |
 | LICENSE or README.TXT in the top directory.  If do not  have a copy of     |
 | these files, you can request them from any member of the DEU team, or by   |
 | mail: Raphael Quinet, Rue des Martyrs 9, B-4550 Nandrin (Belgium).         |
 |                                                                            |
 | This program comes with absolutely no warranty.  Use it at your own risks! |
 *----------------------------------------------------------------------------*

	Project WinDEU
	DEU team
	Jul-Dec 1994, Jan-Mar 1995

	FILE:         names.cpp

	OVERVIEW
	========
	Source file for Object name and type routines.
*/

#include "common.h"
#pragma hdrstop

#ifndef __names_h
	#include "names.h"
#endif

#ifndef __things_h
	#include "things.h"
#endif

#ifndef __objects_h
	#include "objects.h"
#endif
#include "bsp.h"


typedef struct
{
  char   *shortdesc; /* short description of the LineDef: 16 chars max */
  char   *longdesc;  /* long description of the LineDef */ /*! what max? */
  USHORT  props;     /* properties */
} LineDefDesc;


static LineDefDesc LineDefInfo[] =
{
  { "-- Normal",        "-- Normal", /* 0 */
	0},
  { "DR Open door",     "DR Open door, closes after 4 seconds", /* 1 */
	LDP_SWITCH | LDP_MONSTER | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "W1 Open door O",   "W1 Open door", /* 2 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_CEIL | LDP_DIR_UP},
  { "W1 Close door",    "W1 Close door", /* 3 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_CEIL | LDP_DIR_UP},
  { "W1 Open door",     "W1 Open door, closes after 4 seconds", /* 4 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_CEIL | LDP_DIR_UP},
  { "W1 Raise floor C", "W1 Raise floor to nearest ceiling", /* 5 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_UP},
  { "W1 Fast crushing", "W1 Start fast crushing ceiling", /* 6 */
	LDP_NEEDTAG | LDP_WALK   | LDP_LOCK | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "S1 Raise stairs",  "S1 Raise stairs (+ 8)", /* 7 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_UP},
  { "W1 Raise stairs",  "W1 Raise stairs (+ 8)", /* 8 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_UP},
  { "S1 Change donut",  "S1 Lower pillar, raise & change donut (N)", /* 9 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_DOWN
	| LDP_TX_NUMTT},
  { "W1 Lower lift",    "W1 Lower lift for 3 seconds", /* 10 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "S- End level",     "S- End level", /* 11 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_ENDLEVEL},
  { "W1 Lights max. N", "W1 Change lights to max. nearby lights", /* 12 */
	LDP_NEEDTAG | LDP_WALK   },
  { "W1 Lights to 255", "W1 Change lights to 255", /* 13 */
	LDP_NEEDTAG | LDP_WALK   },
  { "S1 Floor up 32T",  "S1 Raise floor by 32 and change texture only", /* 14 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_LOCK | LDP_MV_FLOOR | LDP_DIR_UP
	| LDP_TX_TRGTO},
  { "S1 Floor up 24T",  "S1 Raise floor by 24 and change texture only", /* 15 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_LOCK | LDP_MV_FLOOR | LDP_DIR_UP
	| LDP_TX_TRGTO},
  { "W1 Close door 30", "W1 Close door for 30 seconds", /* 16 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "W1 Blink. lights", "W1 Start blinking lights 1.0 sec.", /* 17 */
	LDP_NEEDTAG | LDP_WALK   },
  { "S1 Raise floor N", "S1 Raise floor to nearest floor", /* 18 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_UP},
  { "W1 Lower floor N", "W1 Lower floor to nearest floor", /* 19 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "S1 Raise floor T", "S1 Raise floor to nearest and change texture only", /* 20 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_LOCK | LDP_MV_FLOOR | LDP_DIR_UP
	| LDP_TX_TRGTO},
  { "S1 Lower lift",    "S1 Lower lift for 3 seconds", /* 21 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "W1 Raise floor T", "W1 Raise floor to nearest and change texture only", /* 22 */
	LDP_NEEDTAG | LDP_WALK   | LDP_LOCK | LDP_MV_FLOOR | LDP_DIR_UP
	| LDP_TX_TRGTO},
  { "S1 Lower floor L", "S1 Lower floor to lowest floor", /* 23 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "G1 Raise floor C", "G1 Raise floor to nearest ceiling", /* 24 */
	LDP_NEEDTAG | LDP_GUN    | LDP_MV_FLOOR | LDP_DIR_UP},
  { "W1 Crushing ceil", "W1 Start slow crushing ceiling", /* 25 */
	LDP_NEEDTAG | LDP_WALK   | LDP_LOCK | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "DR Open blue",     "DR Open blue door, closes after 4 seconds", /* 26 */
	LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP | LDP_KEY_BLUE},
  { "DR Open yellow",   "DR Open yellow door, closes after 4 seconds", /* 27 */
	LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP | LDP_KEY_YLLW},
  { "DR Open red",      "DR Open red door, closes after 4 seconds", /* 28 */
	LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP | LDP_KEY_RED},
  { "S1 Open door",     "S1 Open door, closes after 4 seconds", /* 29 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_CEIL | LDP_DIR_UP},
  { "W1 Raise floor*2", "W1 Raise floor (double shortest height)", /* 30 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_UP},
  { "DR Open door O",   "DR Open door", /* 31 */
	LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "DR Open Blue O",   "DR Open blue door", /* 32 */
	LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP | LDP_KEY_BLUE},
  { "DR Open Red O",    "DR Open red door", /* 33 */
	LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP | LDP_KEY_RED},
  { "DR Open Yellow O", "DR Open yellow door", /* 34 */
	LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP | LDP_KEY_YLLW},
  { "W1 Lights to 0",   "W1 Change lights to 0", /* 35 */
	LDP_NEEDTAG | LDP_WALK   },
  { "W1 T Lower floor", "W1 Turbo lower floor to nearest floor + 8", /* 36 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "W1 Lower floor T", "W1 Lower floor to nearest and change texture and type (N)", /* 37 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_DOWN | LDP_TX_NUMTT},
  { "W1 Lower floor L", "W1 Lower floor to lowest floor", /* 38 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "W1 Teleport",      "W1 Teleport to sector", /* 39 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MONSTER | LDP_TELEPORT},
  { "W1 Raise ceil. H", "W1 Raise ceiling to highest ceiling", /* 40 */ /*! check if floor moves */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_CEIL | LDP_DIR_UP},
  { "S1 Lower ceil. F", "S1 Lower ceiling to floor", /* 41 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "SR Close door",    "SR Close door", /* 42 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "SR Lower ceil. F", "SR Lower ceiling to floor", /* 43 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "W1 Lower ceil.!F", "W1 Lower ceiling to floor + 8 (crush)", /* 44 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "SR Lower floor N", "SR Lower floor to nearest floor", /* 45 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "G1 Open door O",   "G1 Open door", /* 46 */
	LDP_NEEDTAG | LDP_GUN    | LDP_MV_CEIL | LDP_DIR_UP},
  { "G1 Raise floor T", "G1 Raise floor to nearest and change texture only", /* 47 */
	LDP_NEEDTAG | LDP_GUN    | LDP_LOCK | LDP_MV_FLOOR | LDP_DIR_UP
	| LDP_TX_TRGTO},
  { "-- Animated wall", "-- Texture scrolls horizontally", /* 48 */
	0},
  { "S1 Lower ceil.!F", "S1 Lower ceiling to floor + 8 (crush)", /* 49 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "S1 Close door",    "S1 Close door", /* 50 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "S- Secret level",  "S- Secret level", /* 51 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_ENDLEVEL},
  { "W- End level",     "W- End level", /* 52 */
	LDP_NEEDTAG | LDP_WALK   | LDP_ENDLEVEL},
  { "W1 Start movng f", "W1 Start up and down floor movement", /* 53 */
	LDP_NEEDTAG | LDP_WALK   | LDP_LOCK | LDP_MV_FLOOR /*! dir? */},
  { "W1 Stop moving f", "W1 Stop up and down floor movement", /* 54 */
	LDP_NEEDTAG | LDP_WALK   | LDP_LOCK | LDP_MV_FLOOR /*! dir? */},
  { "S1 Raise floor!C", "S1 Raise floor to nearest ceiling - 8 (crush)", /* 55 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_UP},
  { "W1 Raise floor!C", "W1 Raise floor to nearest ceiling - 8 (crush)", /* 56 */
	LDP_NEEDTAG | LDP_WALK   | LDP_LOCK | LDP_MV_FLOOR | LDP_DIR_UP},
  { "W1 Stop crushing", "W1 Stop crushing ceiling", /* 57 */
	LDP_NEEDTAG | LDP_WALK   | LDP_LOCK},
  { "W1 Raise floor24", "W1 Raise floor by 24", /* 58 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_UP},
  { "W1 Raise flr24 T", "W1 Raise floor by 24, change texture and type", /* 59 */
	LDP_NEEDTAG | LDP_WALK   | LDP_LOCK | LDP_MV_FLOOR | LDP_DIR_UP
	| LDP_TX_TRGTT},
  { "SR Lower floor L", "SR Lower floor to lowest floor", /* 60 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "SR Open door O",   "SR Open door", /* 61 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "SR Lower lift",    "SR Lower lift for 3 seconds", /* 62 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "SR Open door",     "SR Open door, closes after 4 seconds", /* 63 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "SR Raise floor C", "SR Raise floor to nearest ceiling", /* 64 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_UP},
  { "SR Raise floor!C", "SR Raise floor to nearest ceiling - 8 (crush)", /* 65 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_UP},
  { "SR Raise flr24 T", "SR Raise floor by 24, change texture only", /* 66 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_LOCK | LDP_MV_FLOOR
	| LDP_DIR_UP | LDP_TX_TRGTO},
  { "SR Raise flr32 T", "SR Raise floor by 32, change texture only", /* 67 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_LOCK | LDP_MV_FLOOR
	| LDP_DIR_UP | LDP_TX_TRGTO},
  { "SR Raise floor T", "SR Raise floor to nearest, change texture only", /* 68 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_LOCK | LDP_MV_FLOOR
	| LDP_DIR_UP | LDP_TX_TRGTO},
  { "SR Raise floor N", "SR Raise floor to nearest floor", /* 69 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_UP},
  { "SR T Lower flr>N", "SR Turbo lower floor to nearest + 8", /* 70 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "S1 T Lower flr>N", "S1 Turbo lower floor to nearest + 8", /* 71 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "WR Lower ceil.!F", "WR Lower ceiling to floor + 8 (crush)", /* 72 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "WR Start crushng", "WR Start crushing ceiling", /* 73 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_LOCK | LDP_MV_CEIL
	| LDP_DIR_DOWN},
  { "WR Stop crushing", "WR Stop crushing ceiling", /* 74 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_LOCK},
  { "WR Close door",    "WR Close door", /* 75 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "WR Close door 30", "WR Close door for 30 seconds", /* 76 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "WR Fast crushing", "WR Start fast crushing ceiling", /* 77 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_LOCK | LDP_MV_CEIL
	| LDP_DIR_DOWN},
  { "WR Lights min. N", "WR Change lights to min. nearby lights (1.2?)", /* 78 */ /*! check if it works */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT},
  { "WR Lights to 0",   "WR Change lights to 0", /* 79 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT},
  { "WR Lights max. N", "WR Change lights to max. nearby lights", /* 80 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT},
  { "WR Lights to 255", "WR Change lights to 255", /* 81 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT},
  { "WR Lower floor L", "WR Lower floor to lowest floor", /* 82 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "WR Lower floor N", "WR Lower floor to nearest floor", /* 83 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "WR Lower flr N&T", "WR Lower floor to nearest, change texture and type (N)", /* 84 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_DOWN
	| LDP_TX_NUMTT},
  { "WR LowerF RaiseC", "WR Lower floor and raise ceiling (1.2?)", /* 85 */ /*! check if it works */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "WR Open door O",   "WR Open door", /* 86 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "WR Start movng f", "WR Start up and down floor movement", /* 87 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_LOCK | LDP_MV_FLOOR /*! dir? */},
  { "WR Lower lift",    "WR Lower lift for 3 seconds", /* 88 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MONSTER | LDP_REPEAT | LDP_MV_FLOOR
	| LDP_DIR_DOWN},
  { "WR Stop moving f", "WR Stop up and down floor movement", /* 89 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_LOCK},
  { "WR Open door",     "WR Open door, closes after 4 seconds", /* 90 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "WR Raise floor C", "WR Raise floor to nearest ceiling", /* 91 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_UP},
  { "WR Raise floor24", "WR Raise floor by 24", /* 92 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_UP},
  { "WR Raise flr.24T", "WR Raise floor 24, change texture and type", /* 93 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_LOCK | LDP_MV_FLOOR
	| LDP_DIR_UP | LDP_TX_TRGTT},
  { "WR Raise floor!C", "WR Raise floor to nearest ceiling - 8 (crush)", /* 94 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_LOCK | LDP_MV_FLOOR
	| LDP_DIR_UP},
  { "WR Raise floor T", "WR Raise floor to nearest floor, change texture only", /* 95 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_LOCK | LDP_MV_FLOOR
	| LDP_DIR_UP | LDP_TX_TRGTO},
  { "WR Raise floor*2", "WR Raise floor (double shortest height)", /* 96 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_UP},
  { "WR Teleport",      "WR Teleport to sector", /* 97 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MONSTER | LDP_REPEAT | LDP_TELEPORT},
  { "WR T Lower flr>N", "WR Turbo lower floor to nearest + 8", /* 98 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "SR T Open blue O", "SR Turbo open blue door", /* 99 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP
	| LDP_KEY_BLUE},
  { "W1 T Stairs 16",   "W1 Turbo raise stairs (+ 16)", /* 100 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_UP},
  { "S1 Raise floor C", "S1 Raise floor to nearest ceiling", /* 101 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_UP},
  { "S1 Lower floor F", "S1 Lower floor to nearest floor", /* 102 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "S1 Open door O",   "S1 Open door", /* 103 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_CEIL | LDP_DIR_UP},
  { "W1 Lights min. N", "W1 Change lights to min. nearby lights", /* 104 */
	LDP_NEEDTAG | LDP_WALK   },
  { "WR T Open door",   "WR Turbo open door, closes after 4 secs", /* 105 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "WR T Open door O", "WR Turbo open door", /* 106 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "WR T Close door",  "WR Turbo close door", /* 107 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "W1 T Open door",   "W1 Turbo open door, closes after 4 secs", /* 108 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_CEIL | LDP_DIR_UP},
  { "W1 T Open door O", "W1 Turbo open door", /* 109 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_CEIL | LDP_DIR_UP},
  { "W1 T Close door",  "W1 Turbo close door", /* 110 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "S1 T Open door",   "S1 Turbo open door, closes after 4 secs", /* 111 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_CEIL | LDP_DIR_UP},
  { "S1 T Open door O", "S1 Turbo open door", /* 112 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_CEIL | LDP_DIR_UP},
  { "SR T Close door",  "SR Turbo close door", /* 113 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "SR T Open door",   "SR Turbo open door, closes after 4 secs", /* 114 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "SR T Open door O", "SR Turbo open door", /* 115 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "S1 T Close door",  "S1 Turbo close door", /* 116 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_CEIL | LDP_DIR_DOWN},
  { "DR T Open door",   "DR Turbo open door, closes after 4 secs", /* 117 */
	LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "DR T Open door O", "DR Turbo open door", /* 118 */
	LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP},
  { "W1 Raise floor N", "W1 Raise floor to nearest floor", /* 119 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_UP},
  { "WR T Lower lift",  "WR Turbo lower lift for 3 seconds", /* 120 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "W1 T Lower lift",  "W1 Turbo lower lift for 3 seconds", /* 121 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "S1 T Lower lift",  "S1 Turbo lower lift for 3 seconds", /* 122 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "SR T Lower lift",  "SR Turbo lower lift for 3 seconds", /* 123 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_DOWN},
  { "W- Secret level",  "W- Secret level", /* 124 */
	LDP_NEEDTAG | LDP_WALK   | LDP_ENDLEVEL},
  { "W1 Teleport Mnst", "W1 Teleport Monster to sector", /* 125 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MONSTER | LDP_TELEPORT},
  { "WR Teleport Mnst", "WR Teleport Monster to sector", /* 126 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MONSTER | LDP_REPEAT | LDP_TELEPORT},
  { "S1 T Stairs 16",   "S1 Turbo raise stairs (+ 16)", /* 127 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_UP},
  { "WR Raise floor N", "WR Raise floor to nearest floor", /* 128 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_UP},
  { "WR T Raise floor", "WR Turbo raise floor to nearest floor", /* 129 */
	LDP_NEEDTAG | LDP_WALK   | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_UP},
  { "W1 T Raise floor", "W1 Turbo raise floor to nearest floor", /* 130 */
	LDP_NEEDTAG | LDP_WALK   | LDP_MV_FLOOR | LDP_DIR_UP},
  { "S1 T Raise floor", "S1 Turbo raise floor to nearest floor", /* 131 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_UP},
  { "SR T Raise floor", "SR Turbo raise floor to nearest floor", /* 132 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_FLOOR | LDP_DIR_UP},
  { "S1 T Open Blue O", "S1 Turbo open blue door", /* 133 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_CEIL | LDP_DIR_UP | LDP_KEY_BLUE},
  { "SR T Open Red O",  "SR Turbo open red door", /* 134 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP
	| LDP_KEY_RED},
  { "S1 T Open Red O",  "S1 Turbo open red door", /* 135 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_CEIL | LDP_DIR_UP | LDP_KEY_RED},
  { "SR T Open Yllw O", "SR Turbo open yellow door", /* 136 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT | LDP_MV_CEIL | LDP_DIR_UP
	| LDP_KEY_YLLW},
  { "S1 T Open Yllw O", "S1 Turbo open yellow door", /* 137 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_CEIL | LDP_DIR_UP | LDP_KEY_YLLW},
  { "SR Lights min. N", "SR Change lights to min. nearby lights", /* 138 */ /*! check if min or 0 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT},
  { "SR Lights max. N", "SR Change lights to max. nearby lights", /* 139 */ /*! check if max or 255 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_REPEAT},
  { "S1 Raise flr 512", "S1 Raise floor by 512", /* 140 */
	LDP_NEEDTAG | LDP_SWITCH | LDP_MV_FLOOR | LDP_DIR_UP},
  { "W1 Silent Crush", "W1 Start silent crushing ceiling", /* 141 */
	LDP_NEEDTAG | LDP_WALK   | LDP_LOCK | LDP_MV_CEIL | LDP_DIR_DOWN}
};
#define MAXLINEDEFINFO  141


/*
   Get a short (16 char.) description of the type of a linedef.
*/

char *GetLineDefTypeName(SHORT type)
{
  if (type >= 0 && type <= MAXLINEDEFINFO)
	return LineDefInfo[type].shortdesc;
  else
	return "?? UNKNOWN";
}


/*
   Get a long description of the type of a linedef.
*/

char *GetLineDefTypeLongName(SHORT type)
{
  if (type >= 0 && type <= MAXLINEDEFINFO)
	return LineDefInfo[type].longdesc;
  else
	return "?? UNKNOWN LINEDEF TYPE";
}


/*
   Get the properties (flags) for a type of linedef.
*/

USHORT GetLineDefProperties(SHORT type)
{
  if (type >= 0 && type <= MAXLINEDEFINFO)
	return LineDefInfo[type].props;
  else
	return LDP_BAD;
}


/*
   get the name of an object type
*/
char *GetObjectTypeName (SHORT objtype)
{
   switch (objtype)
   {
   case OBJ_THINGS:
	  return "Thing";
   case OBJ_LINEDEFS:
	  return "LineDef";
   case OBJ_SIDEDEFS:
	  return "SideDef";
   case OBJ_VERTEXES:
	  return "Vertex";
   case OBJ_SEGS:
	  return "Segment";
   case OBJ_SSECTORS:
	  return "SSector";
   case OBJ_NODES:
	  return "Node";
   case OBJ_SECTORS:
	  return "Sector";
   case OBJ_REJECT:
	  return "Reject";
   case OBJ_BLOCKMAP:
	  return "Blockmap";
   }
   return "< Bug! >";
}

/*
   get the name of an object type (plurial)
*/
char *GetObjectsTypeName (SHORT objtype)
{
   switch (objtype)
   {
   case OBJ_THINGS:
	  return "Things";
   case OBJ_LINEDEFS:
	  return "LineDefs";
   case OBJ_SIDEDEFS:
	  return "SideDefs";
   case OBJ_VERTEXES:
	  return "Vertices";
   case OBJ_SEGS:
	  return "Segments";
   case OBJ_SSECTORS:
	  return "SSectors";
   case OBJ_NODES:
	  return "Nodes";
   case OBJ_SECTORS:
	  return "Sectors";
   case OBJ_REJECT:
	  return "Rejects";
   case OBJ_BLOCKMAP:
	  return "Blockmaps";
   }
   return "< Bug! >";
}



/*
   what are we editing?
*/
char *GetEditModeName (SHORT objtype)
{
   switch (objtype)
   {
   case OBJ_THINGS:
	  return "Things";
   case OBJ_LINEDEFS:
   case OBJ_SIDEDEFS:
	  return "LineDefs & SideDefs";
   case OBJ_VERTEXES:
	  return "Vertices";
   case OBJ_SEGS:
	  return "Segments";
   case OBJ_SSECTORS:
	  return "Seg-Sectors";
   case OBJ_NODES:
	  return "Nodes";
   case OBJ_SECTORS:
	  return "Sectors";
   }
   return "< Bug! >";
}


/*
   get a short description of the flags of a linedef
*/

char *GetLineDefFlagsName (int flags)
{
   static char temp[50];

   if (flags & BF_POS_BACKWARDS)
      strcpy( temp, "+F");
   else strcpy( temp, "<>");
   if (flags & BF_NEG_BACKWARDS)
      strcat( temp, "-F");
   else strcat( temp, "<>");
   if (flags & BF_POS_TRANSPARENT)
      strcat( temp, "+T");
   else strcat( temp, "<>");
   if (flags & BF_NEG_TRANSPARENT)
      strcat( temp, "-T");
   else strcat( temp, "<>");
   if (flags & BF_POS_PASSABLE)
      strcat( temp, "+P");
   else strcat( temp, "<>");
   if (flags & BF_NEG_PASSABLE)
      strcat( temp, "-P");
   else strcat( temp, "<>");
   if (flags & BF_MAP_NEVER)
      strcat( temp, "Mn");
   else strcat( temp, "<>");
   if (flags & BF_MAP_ALWAYS)
      strcat( temp, "My");
   else strcat( temp, "<>");
   if (flags & BF_POS_NOLOOKTHROUGH)
      strcat( temp, "+L");
   else strcat( temp, "<>");
   if (flags & BF_NEG_NOLOOKTHROUGH)
      strcat( temp, "-L");
   else strcat( temp, "<>");
   if (flags & BF_POS_NO_VTILE)
      strcat( temp, "+V");
   else strcat( temp, "<>");
   if (flags & BF_NEG_NO_VTILE)
      strcat( temp, "-V");
   else strcat( temp, "<>");

   return temp;
}



/*
   get a long description of one linedef flag
*/

char *GetLineDefFlagsLongName (SHORT flags)
{
   if (flags & 0x0100)
	  return "Already on the map at startup";
   if (flags & 0x80)
	  return "Invisible on the map";
   if (flags & 0x40)
	  return "Blocks sound";
   if (flags & 0x20)
	  return "Secret (shown as normal on the map)";
   if (flags & 0x10)
	  return "Lower texture is \"unpegged\"";
   if (flags & 0x08)
	  return "Upper texture is \"unpegged\"";
   if (flags & 0x04)
	  return "Two-sided (may be transparent)";
   if (flags & 0x02)
	  return "Monsters cannot cross this line";
   if (flags & 0x01)
	  return "Impassible";

   return "INVALID FLAG!";
}



/*
   get a short (13 char.) description of the type of a sector
*/

char *GetSectorTypeName (SHORT type)
{
	if (DoomVersion == 16)
		switch (type)
		{
		case 0:
			return "Normal";
		case 1:
			return "Flicker";
		case 2:
			return "Strobe fast";
		case 3:
			return "Strobe slow";
		case 4:
			return "Lava flow";
		case 5:
			return "Wimpy Lava";
		case 6:
			return "Crush ceiling";
		case 7:
			return "Sludge";
		case 8:
			return "Glow";
		case 9:
			return "Secret";
		case 10:
			return "Close door 30s";
		case 12:
			return "Synchs slow";
		case 13:
			return "Synchs fast";
		case 14:
			return "Door raise 5m";
		case 15:
			return "Friction";
		case 16:
			return "Hifty Lava";
		case 21:
			return "Scroll East (slow)";
		case 22:
			return "Scroll East (norm)";
		case 23:
			return "Scroll East (fast)";
		case 26:
			return "Scroll North (slow)";
		case 27:
			return "Scroll North (norm)";
		case 28:
			return "Scroll North (fast)";
		case 31:
			return "Scroll South (slow)";
		case 32:
			return "Scroll South (norm)";
		case 33:
			return "Scroll South (fast)";
		case 36:
			return "Scroll West (slow)";
		case 37:
			return "Scroll West (norm)";
		case 38:
			return "Scroll West (fast)";
		case 40:
			return "Wind East (slow)";
		case 41:
			return "Wind East (norm)";
		case 42:
			return "Wind East (fast)";
		case 43:
			return "Wind North (slow)";
		case 44:
			return "Wind North (norm)";
		case 45:
			return "Wind North (fast)";
		case 46:
			return "Wind South (slow)";
		case 47:
			return "Wind South (norm)";
		case 48:
			return "Wind South (fast)";
		case 49:
			return "Wind West (slow)";
		case 50:
			return "Wind West (norm)";
		case 51:
			return "Wind West (fast)";
		}
	else
		switch (type)
		{
		case 0:
			return "Normal";
		case 1:
			return "Random off";
		case 2:
			return "Fast blink";
		case 3:
			return "Slow blink";
		case 4:
			return "-20% & blink";
		case 5:
			return "-10% health";
//#ifdef ID_DID_IT
		case 6:
			return "Ceiling crush";
//#endif
		case 7:
			return "-5% health";
		case 8:
			return "Light glows";
		case 9:
			return "Secret";
		case 10:
			return "Close door 30s";
		case 11:
			return "-20% & end lev";
		case 12:
			return "Fast blink syn";
		case 13:
			return "Slow blink syn";
		case 14:
			return "Open Door 5 mn";
//#ifdef ID_DID_IT
		case 15:
			return "Creates ammo";
//#endif
		case 16:
			return "-20% health";
		case 17:
			return "Light flickers";
	}
	return "DO NOT USE!";
}



/*
   get a long description of the type of a sector
*/

char *GetSectorTypeLongName (SHORT type)
{
	if (DoomVersion == 16)
		switch (type)
		{
		case 0:
			return "Normal";
		case 1:
			return "Light flickers";
		case 2:
			return "Light strobes fast (twice per second)";
		case 3:
			return "Light strobes slow (once per second)";
		case 4:
			return "Lava flow damage";
		case 5:
			return "Wimpy lava flow damage";
		case 6:
			return "Start crushing ceiling";
		case 7:
			return "Sludge damage";
		case 8:
			return "Light glow between level and nearly level";
		case 9:
			return "Secret (credit if discovered)";
		case 10:
			return "Doos closes in 30 sec";
		case 12:
			return "Light synchronizes and strobes slowly";
		case 13:
			return "Light synchronizes and strobes fast";
		case 14:
			return "Door close and raise in 5 minutes";
		case 15:
			return "Low friction";
		case 16:
			return "Hefty lava flow damage";
		case 21:
			return "Scroll texture eastwards slowly";
		case 22:
			return "Scroll texture eastwards normal";
		case 23:
			return "Scroll texture eastwards fast";
		case 26:
			return "Scroll texture nothwards slowly";
		case 27:
			return "Scroll texture nothwards normal";
		case 28:
			return "Scroll texture nothwards fast";
		case 31:
			return "Scroll texture southwards slowly";
		case 32:
			return "Scroll texture southwards normal";
		case 33:
			return "Scroll texture southwards fast";
		case 36:
			return "Scroll texture westwards slowly";
		case 37:
			return "Scroll texture westwards normal";
		case 38:
			return "Scroll texture westwards fast";
		case 40:
			return "Eastern wind (slow)";
		case 41:
			return "Eastern wind (normal)";
		case 42:
			return "Eastern wind (fast)";
		case 43:
			return "Nothern wind (slow)";
		case 44:
			return "Nothern wind (normal)";
		case 45:
			return "Nothern wind (fast)";
		case 46:
			return "Southern wind (slow)";
		case 47:
			return "Southern wind (normal)";
		case 48:
			return "Southern wind (fast)";
		case 49:
			return "Western wind (slow)";
		case 50:
			return "Western wind (normal)";
		case 51:
			return "Western wind (fast)";
		}
	else
		switch (type)
		{
		case 0:
			return "Normal";
		case 1:
			return "Light goes off randomly (normally on)";
		case 2:
			return "Light blinks fast (twice per second)";
		case 3:
			return "Light blinks slow (once per second)";
		case 4:
			return "-20% health and light blinks fast";
		case 5:
			return "-10% health";
//#ifdef ID_DID_IT
		case 6:
			return "Crushing ceiling";
//#endif
		case 7:
			return "-5% health (nukage floor)";
		case 8:
			return "Light glows between set level and nearby level";
		case 9:
			return "Secret (credit if discovered)";
		case 10:
			return "Door closes after 30 seconds";
		case 11:
			return "-20% health and end level when health <= 10%";
		case 12:
			return "Light blinks fast, syncronized";
		case 13:
			return "Light blinks slow, syncronized";
		case 14:
			return "Door closes, then raises after 5 minutes";
//#ifdef ID_DID_IT
		case 15:
			return "Creates ammo";
//#endif
		case 16:
			return "-20% health";
		case 17:
			return "Light goes on and off randomly (flickers)";
   }
   return "INVALID - DO NOT USE!";
}


/*
	Check if an entry is the name of a doom (1 or 2) level
*/

BOOL IsDoomLevelName (char *name, int doomVersion)
{
	if ( (doomVersion == 1) || (doomVersion == 16) )
		return IsDoom1LevelName(name);
	else
		return IsDoom2LevelName(name);
}


/*
	Check if an entry is the name of a doom (1) level
*/

BOOL IsDoom1LevelName (char *name)
{
	return name[0] == 'E' && name[2] == 'M' && name[4] == '\0';
}

/*
	Check if an entry is the name of a doom (2) level
*/

BOOL IsDoom2LevelName (char *name)
{
	return name[0] == 'M' && name[1] == 'A' && name[2] == 'P' && name[5] == '\0';
}

/* end of file */
