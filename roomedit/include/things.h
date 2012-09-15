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

	FILE:         things.h

	OVERVIEW
	========
	Doom thing type defines.
*/
#ifndef __things_h
#define __things_h

#ifndef __common_h
	#include "common.h"
#endif

/* Thing properties */
#define TP_BLOCK  0x0001  /* Blocks the player */
#define TP_PICK   0x0002  /* Can be picked up */
#define TP_SOUND  0x0004  /* Sound only: invisible, can be outside of map */
#define TP_INVIS  0x0008  /* Invisible or blurred */
#define TP_FLOAT  0x0010  /* Floats or hangs from the ceiling */
#define TP_ITEM   0x0020  /* Counts towards the item ratio at the end */
#define TP_KILL   0x0040  /* Counts towards the kill ratio at the end */
#define TP_LIGHT  0x0080  /* Can be seen in a dark room */
#define TP_PLAYER 0x0100  /* Player starting point */
#define TP_WEAPON 0x0200  /* Weapon */
#define TP_WAMMO  0x0400  /* Ammunition */
#define TP_CORPSE 0x0800  /* Corpse */
#define TP_BAD    0xFFFF  /* Invalid Thing - should not be used */

/* starting areas */
#define THING_PLAYER1         1
#define THING_PLAYER2         2
#define THING_PLAYER3         3
#define THING_PLAYER4         4
#define THING_DEATHMATCH      11

/* teleport arrival */
#define THING_TELEPORT        14

/* enhancements */
#define THING_BLUECARD        5
#define THING_YELLOWCARD      6
#define THING_REDCARD         13
#define THING_BLUESKULLKEY    40
#define THING_YELLOWSKULLKEY  39
#define THING_REDSKULLKEY     38
#define THING_MEGASPHERE      83
#define THING_ARMBONUS1       2015
#define THING_HLTBONUS1       2014
#define THING_GREENARMOR      2018
#define THING_BLUEARMOR       2019
#define THING_STIMPACK        2011
#define THING_MEDKIT          2012
#define THING_SOULSPHERE      2013
#define THING_RADSUIT         2025
#define THING_MAP             2026
#define THING_BLURSPHERE      2024
#define THING_BESERK          2023
#define THING_INVULN          2022
#define THING_LITEAMP         2045

/* weapons */
#define THING_SHOTGUN         2001
#define THING_SUPERSHOTGUN    82
#define THING_CHAINGUN        2002
#define THING_LAUNCHER        2003
#define THING_PLASMAGUN       2004
#define THING_CHAINSAW        2005
#define THING_BFG9000         2006
#define THING_AMMOCLIP        2007
#define THING_AMMOBOX         2048
#define THING_SHELLS          2008
#define THING_SHELLBOX        2049
#define THING_ROCKET          2010
#define THING_ROCKETBOX       2046
#define THING_ENERGYCELL      2047
#define THING_ENERGYPACK      17
#define THING_BACKPACK        8

/* enemies (monsters) */
#define THING_SARGEANT        9
#define THING_TROOPER         3004
#define THING_IMP             3001
#define THING_DEMON           3002
#define THING_BARON           3003
#define THING_SPECTOR         58
#define THING_CACODEMON       3005
#define THING_LOSTSOUL        3006
#define THING_SPIDERBOSS      7
#define THING_CYBERDEMON      16
#define THING_ARCHVILE        64
#define THING_CHAINGUNNER     65
#define THING_REVENANT        66
#define THING_MANCUBUS        67
#define THING_ARACHNOTRON     68
#define THING_HELLKNIGHT      69
#define THING_PAINELEMENTAL   71
#define THING_KEEN            72
#define THING_WOLFENSTEIN_SS  84
#define THING_BOSSBRAIN       88

/* special "ennemies" */
#define THING_BOSSSHOOTER     89
#define THING_SPAWNSPOT       87
#define THING_BARREL          2035

/*! The following definitions are not really needed (except for the Thing
	selection dialog box).  They should be removed if possible. */

/* decorations */
#define THING_FUELCAN         70
#define THING_TECHCOLUMN      48
#define THING_TGREENPILLAR    30
#define THING_TREDPILLAR      32
#define THING_SGREENPILLAR    31
#define THING_SREDPILLAR      33
#define THING_PILLARHEART     36
#define THING_PILLARSKULL     37
#define THING_EYEINSYMBOL     41
#define THING_GREYTREE        43
#define THING_BROWNTREE       54
#define THING_BROWNSTUB       47
#define THING_STALAGMITE      47

/* decorations 2 */
#define THING_LAMP            2028
#define THING_CANDLE          34
#define THING_CANDELABRA      35
#define THING_TBLUETORCH      44
#define THING_TGREENTORCH     45
#define THING_TREDTORCH       46
#define THING_SBLUETORCH      55
#define THING_SGREENTORCH     56
#define THING_SREDTORCH       57

/* decorations 3 */
#define THING_DEADPLAYER      15
#define THING_DEADTROOPER     18
#define THING_DEADSARGEANT    19
#define THING_DEADIMP         20
#define THING_DEADDEMON       21
#define THING_DEADCACODEMON   22
#define THING_DEADLOSTSOUL    23
#define THING_BONES           10
#define THING_BONES2          12
#define THING_POOLOFBLOOD     24
#define THING_SKULLTOPPOLE    27
#define THING_HEADSKEWER      28
#define THING_PILEOFSKULLS    29
#define THING_IMPALEDBODY     25
#define THING_IMPALEDBODY2    26
#define THING_SKULLSINFLAMES  42

/* decorations 4 */
#define THING_HANGINGSWAYING  49
#define THING_HANGINGARMSOUT  50
#define THING_HANGINGONELEG   51
#define THING_HANGINGTORSO    52
#define THING_HANGINGLEG      53
#define THING_HANGINGSWAYING2 63
#define THING_HANGINGARMSOUT2 59
#define THING_HANGINGONELEG2  61
#define THING_HANGINGTORSO2   60
#define THING_HANGINGLEG2     62


/* from things.cpp */
USHORT GetThingProperties(SHORT type);
int GetThingColour (SHORT);
char *GetThingName (SHORT);
char  *GetThingPicName(SHORT type);
SHORT GetThingRadius (SHORT);
SHORT  GetThingHeight(SHORT type);
SHORT  GetThingHealth(SHORT type);
char *GetAngleName (SHORT);
char *GetWhenName (SHORT);

enum KodThings {
	kodPlayerBlocking,
	kodExit,
	kodMonsterGenerator,
	kodEntrance,
	kodExtraPreObject2,
	kodExtraPreObject3,
	kodExtraPreObject4,
	kodObject,					//objects run this value and higher
	kodMax
};

void LoadKodObjects(const char *filename);
void UnloadKodObjects(void);
int GetNumKodObjects(void);
LPCSTR GetKodObjectName(int index);
LPCSTR GetKodObjectDecorativeName(int index);
int GetKodObjectID(int index);
int GetKodObjectRadius(int index);
int GetKodObjectColor(int index);
int FindKodObjectByName(const char *objectName);

int GetNumKodTypes(int objectNum);
LPCSTR GetKodTypeName(int objectNum, int index);
int GetKodTypeID(int objectNum, int index);
int FindKodTypeID(int objectNum, const char *objectName);
LPCSTR GetKodTypeDecorativeName(int objectNum, int index);

void LoadKodRooms(const char *filename);
void UnloadKodRooms(void);
int GetNumKodRooms(void);
LPCSTR GetKodRoomName(int index);
LPCSTR GetKodRoomDecorativeName(int index);
int GetKodRoomID(int index);
LPCSTR GetKodRoomNameByRoomID(int roomID);
int FindKodRoomID(LPCSTR roomName);
int GetKodIndexFromID(int id);

#endif
/* end of file */
