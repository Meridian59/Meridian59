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

	FILE:         names.h
*/
#ifndef __names_h
#define __names_h

#ifndef __common_h
	#include "common.h"
#endif

/*
   LineDef Properties.
*/

#define LDP_NEEDTAG  0x0001  /* Requires a tag number */

#define LDP_WALK     0x0002  /* Activated when the player walks over it */
#define LDP_SWITCH   0x0004  /* Activated when pushed (is door if no tag) */
#define LDP_GUN      0x0008  /* Activated when shot (guns) or hit (fist) */
#define LDP_MONSTER  0x0010  /* Can be activated by a monster */

#define LDP_REPEAT   0x0020  /* Can be activated multiple times */
#define LDP_LOCK     0x0040  /* Affected sectors are "locked" */

#define LDP_MV_NONE  0x0000  /* Nothing moves */
#define LDP_MV_CEIL  0x0100  /* The ceiling moves */
#define LDP_MV_FLOOR 0x0200  /* The floor moves */
#define LDP_MV_MASK  0x0300

#define LDP_DIR_UP   0x0400  /* The floor or ceiling moves up */
#define LDP_DIR_DOWN 0x0000  /* The floor or ceiling moves down */

/* if LDP_MV_FLOOR: the floor moves */
#define LDP_TX_NONE  0x0000  /* No texture change */
#define LDP_TX_NUMTT 0x1000  /* Numeric model: texture and type change */
#define LDP_TX_TRGTT 0x2000  /* Trigger model: texture and type change */
#define LDP_TX_TRGTO 0x3000  /* Trigger model: texture change only */
#define LDP_TX_MASK  0x3000

/* if LDP_MV_CEIL: the ceiling moves (doors) */
#define LDP_KEY_NONE 0x0000  /* Requires no special key */
#define LDP_KEY_BLUE 0x1000  /* Requires blue key */
#define LDP_KEY_YLLW 0x2000  /* Requires yellow key */
#define LDP_KEY_RED  0x3000  /* Requires red key */
#define LDP_KEY_MASK 0x3000

/* if LDP_MV_NONE: no move */
#define LDP_TELEPORT 0x1000  /* Requires teleport exit in tagged sector */
#define LDP_ENDLEVEL 0x2000  /* Ends the current level */

#define LDP_BAD      0xFFFF  /* Invalid LineDef - should not be used */

/* Note: I could have coded some info in fewer bits but using one bit
   for each property, even if two of them are mutually exclusive, makes
   testing easier and faster.
*/

/* from names.cpp */
char *GetObjectTypeName (SHORT objtype);
char *GetObjectsTypeName (SHORT objtype);
char *GetEditModeName (SHORT objtype);
char *GetLineDefTypeName (SHORT type);
char *GetLineDefTypeLongName (SHORT type);
char *GetLineDefFlagsName (int flags);
char *GetLineDefFlagsLongName (SHORT flags);
char *GetSectorTypeName (SHORT type);
char *GetSectorTypeLongName (SHORT type);

BOOL IsDoomLevelName (char *name, int doomVersion);
BOOL IsDoom1LevelName (char *name);
BOOL IsDoom2LevelName (char *name);


#endif
