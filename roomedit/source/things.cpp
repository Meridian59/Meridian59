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

	FILE:         things.cpp

	OVERVIEW
	========
	Source file for Thing name and type routines.
*/

#include "common.h"
#pragma hdrstop

#ifndef __things_h
	#include "things.h"
#endif

#ifndef __gfx_h
	#include "gfx.h"	// Colors
#endif

/* Description of the Things properties. */
typedef struct
{
  SHORT  radius;  /* Radius of the Thing for collision detection */
  SHORT  height;  /* Height of the Thing: minimum ceiling-floor difference */
  SHORT  mass;    /* Mass of the Thing */ /*! What is it used for? */
  SHORT  health;  /* Number of health points, 0 = not alive */
  SHORT  speed;   /* Speed of the Thing (if alive) */
  SHORT  damage;  /* Number of damage points, 0 = no damage */ /*! Explain */
  char  *sprite;  /* Sprite to display - (4 chars for name + 2/4 for frame) */
  char  *name;    /* Name of the Thing (18 chars max) */
  USHORT props;    /* Properties (flags), see TP_* defines */
} ThingDesc;



/* Doom Things. */
#define MINDTHINGINFO0   1
#define MAXDTHINGINFO0   89
static ThingDesc DThingInfo0[MAXDTHINGINFO0 + 1 - MINDTHINGINFO0] =
{
  {  16,  56,  100,  100,  0,  500, "PLAYA1", /* 1 */
	"Player 1 Start", TP_PLAYER},
  {  16,  56,  100,  100,  0,  500, "PLAYA1", /* 2 */
	"Player 2 Start", TP_PLAYER},
  {  16,  56,  100,  100,  0,  500, "PLAYA1", /* 3 */
	"Player 3 Start", TP_PLAYER},
  {  16,  56,  100,  100,  0,  500, "PLAYA1", /* 4 */
	"Player 4 Start", TP_PLAYER},
  {  20,  16,    0,    0,  0,    0, "BKEYB0", /* 5 */
	"Blue KeyCard", TP_PICK},
  {  20,  16,    0,    0,  0,    0, "YKEYB0", /* 6 */
	"Yellow KeyCard", TP_PICK},
  { 128, 100, 1000, 3000, 12,  350, "SPIDH1", /* 7 */
	"Spider Boss*", TP_BLOCK | TP_KILL},
  {  20,  16,    0,    0,  0, 1080, "BPAKA0", /* 8 */
	"Backpack", TP_PICK | TP_WAMMO | TP_ITEM},
  {  20,  56,  100,   30,  8,   50, "SPOSF1", /* 9 */
	"Shotgun Sargeant*", TP_BLOCK | TP_KILL},
  {  16,  16,    0,    0,  0,    0, "PLAYW0", /* 10 */
	"Guts and Bones", TP_CORPSE},
  {  16,  16,    0,    0,  0,  500, "PLAYF1", /* 11 */
	"Deathmatch Start", TP_PLAYER},
  {  16,  16,    0,    0,  0,    0, "PLAYW0", /* 12 */
	"Guts and Bones 2", TP_CORPSE},
  {  20,  16,    0,    0,  0,    0, "RKEYB0", /* 13 */
	"Red KeyCard", TP_PICK},
  {  16,  16,    0,    0,  0,    0, "TFOGB0", /* 14 */
	"Teleport exit", 0},
  {  16,  16,    0,    0,  0,    0, "PLAYN0", /* 15 */
	"Dead Green Player", TP_CORPSE},
  {  40, 110, 1000, 4000, 16, 1500, "CYBRF1", /* 16 */
	"Cyber Demon*", TP_BLOCK | TP_KILL},
  {  20,  16,    0,    0,  0, 2000, "CELPA0", /* 17 */
	"Energy Pack", TP_PICK | TP_WAMMO},
  {  16,  16,    0,    0,  0,    0, "POSSL0", /* 18 */
	"Dead Trooper", TP_CORPSE},
  {  16,  16,    0,    0,  0,    0, "SPOSL0", /* 19 */
	"Dead Sargeant", TP_CORPSE},
  {  16,  16,    0,    0,  0,    0, "TROOM0", /* 20 */
	"Dead Imp", TP_CORPSE},
  {  16,  16,    0,    0,  0,    0, "SARGN0", /* 21 */
	"Dead Demon", TP_CORPSE},
  {  16,  16,    0,    0,  0,    0, "HEADL0", /* 22 */
	"Dead Cacodemon", TP_CORPSE},
  {  16,  16,    0,    0,  0,    0, "------", /* 23 */
	"Dead Lost Soul", TP_CORPSE},
  {  16,  16,    0,    0,  0,    0, "POL5A0", /* 24 */
	"Pool of Blood", TP_CORPSE},
  {  16,  16,    0,    0,  0,    0, "POL1A0", /* 25 */
	"Impaled body*", TP_CORPSE | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "POL6B0", /* 26 */
	"Twitching Impaled*", TP_CORPSE | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "POL4A0", /* 27 */
	"Pole with Skull*", TP_CORPSE | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "POL2A0", /* 28 */
	"Skewer with Heads*", TP_CORPSE | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "POL3A0", /* 29 */
	"Pile of Skulls*", TP_CORPSE | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "COL1A0", /* 30 */
	"Tall Green Pillar*", TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "COL2A0", /* 31 */
	"Short Green Pillar*", TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "COL3A0", /* 32 */
	"Tall Red Pillar*", TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "COL4A0", /* 33 */
	"Short Red Pillar*", TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "CANDA0", /* 34 */
	"Candle", TP_LIGHT},
  {  16,  16,    0,    0,  0,    0, "CBRAA0", /* 35 */
	"Candelabra*", TP_LIGHT | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "COL5B0", /* 36 */
	"Pillar w/Heart*", TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "COL6A0", /* 37 */
	"Red Pillar w/Skul*", TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "RSKUB0", /* 38 */
	"Red Skull Key", TP_PICK},
  {  20,  16,    0,    0,  0,    0, "YSKUB0", /* 39 */
	"Yellow Skull Key", TP_PICK},
  {  20,  16,    0,    0,  0,    0, "BSKUB0", /* 40 */
	"Blue Skull Key", TP_PICK},
  {  16,  16,    0,    0,  0,    0, "CEYEC0", /* 41 */
	"Eye in Symbol*", TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "FSKUB0", /* 42 */
	"Skulls in Flames*", TP_CORPSE | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "TRE1A0", /* 43 */
	"Grey Tree*", TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "TBLUA0", /* 44 */
	"Tall Blue Torch*", TP_LIGHT | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "TGRNA0", /* 45 */
	"Tall Green Torch*", TP_LIGHT | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "TREDA0", /* 46 */
	"Tall Red Torch*", TP_LIGHT | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "SMITA0", /* 47 */
	"Stalagmite / Stub*", TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "ELECA0", /* 48 */
	"Technical Column*", TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "GOR1A0", /* 49 */
	"Hanging Body*", TP_CORPSE | TP_BLOCK | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "GOR2A0", /* 50 */
	"Hanging Arms Out*", TP_CORPSE | TP_BLOCK | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "GOR3A0", /* 51 */
	"One-legged Body*", TP_CORPSE | TP_BLOCK | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "GOR4A0", /* 52 */
	"Hanging Torso*", TP_CORPSE | TP_BLOCK | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "GOR5A0", /* 53 */
	"Hanging Leg*", TP_CORPSE | TP_BLOCK | TP_FLOAT},
  {  32,  16,    0,    0,  0,    0, "TRE2A0", /* 54 */
	"Tall Brown Tree*", TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "SMBTA0", /* 55 */
	"Short Blue Torch*", TP_LIGHT | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "SMGTA0", /* 56 */
	"Short Green Torch*", TP_LIGHT | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "SMRTA0", /* 57 */
	"Short Red Torch*", TP_LIGHT | TP_BLOCK},
  {  30,  56,  400,  150, 10,    0, "SARGB5", /* 58 */
	"Specter*", TP_BLOCK | TP_KILL | TP_INVIS},
  {  16,  16,    0,    0,  0,    0, "GOR2A0", /* 59 */
	"Hanging Arms Out", TP_CORPSE | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "GOR4A0", /* 60 */
	"Hanging Torso", TP_CORPSE | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "GOR3A0", /* 61 */
	"One-legged Body", TP_CORPSE | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "GOR5A0", /* 62 */
	"Hanging Leg", TP_CORPSE | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "GOR1A0", /* 63 */
	"Hanging Body", TP_CORPSE | TP_FLOAT},
  {  20,  56,  500,  700, 15,  666, "VILEN1", /* 64 */
	"Arch-vile*", TP_BLOCK | TP_KILL},
  {  20,  56,  100,   70,  8,  200, "CPOSF1", /* 65 */
	"Chaingun Sargeant*", TP_BLOCK | TP_KILL},
  {  20,  56,  500,  300, 10,  375, "SKELJ1", /* 66 */
	"Revenant*", TP_BLOCK | TP_KILL},
  {  48,  64, 1000,  600,  8,  260, "FATTH1", /* 67 */
	"Mancubus*", TP_BLOCK | TP_KILL},
  {  64,  64,  600,  500, 12,  600, "BSPIG1", /* 68 */
	"Arachnotron*", TP_BLOCK | TP_KILL},
  {  24,  64, 1000,  500,  8,  333, "BOS2E1", /* 69 */
	"Hell Knight*", TP_BLOCK | TP_KILL},
  {  16,  16,    0,    0,  0,    0, "FCANA0", /* 70 */
	"Burning Barrel*", TP_LIGHT | TP_BLOCK},
  {  31,  56,  400,  400,  8,  250, "PAINF1", /* 71 */
	"Pain Elemental*", TP_BLOCK | TP_FLOAT | TP_KILL},
  {  16,  72, 6666,  100,  0,    0, "KEENA0", /* 72 */
	"Hanging Keen*", TP_BLOCK | TP_KILL},
  {  16,  16,    0,    0,  0,    0, "??????", /* 73 */ /*! missing... */
	"<! hang 73>", TP_CORPSE | TP_BLOCK | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "??????", /* 74 */ /*! missing... */
	"<! hang 74>", TP_CORPSE | TP_BLOCK | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "??????", /* 75 */ /*! missing... */
	"<! hang 75>", TP_CORPSE | TP_BLOCK | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "??????", /* 76 */ /*! missing... */
	"<! hang 76>", TP_CORPSE | TP_BLOCK | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "??????", /* 77 */ /*! missing... */
	"<! hang 77>", TP_CORPSE | TP_BLOCK | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "??????", /* 78 */ /*! missing... */
	"<! hang 78>", TP_CORPSE | TP_BLOCK | TP_FLOAT},
  {  16,  16,    0,    0,  0,    0, "??????", /* 79 */ /*! missing... */
	"<! mess 79>", TP_CORPSE},
  {  16,  16,    0,    0,  0,    0, "??????", /* 80 */ /*! missing... */
	"<! mess 80>", TP_CORPSE},
  {  16,  16,    0,    0,  0,    0, "??????", /* 81 */ /*! missing... */
	"<! mess 81>", TP_CORPSE},
  {  20,  16,    0,    0,  0,  760, "SGN2A0", /* 82 */
	"Super Shotgun", TP_PICK | TP_WEAPON},
  {  20,  16,    0,    0,  0,  400, "MEGAA0", /* 83 */
	"Mega Sphere", TP_PICK | TP_ITEM},
  {  20,  56,  100,   50,  8,   45, "SSWVG0", /* 84 */
	"Wolfenstein SS*", TP_BLOCK | TP_KILL},
  {  16,  16,    0,    0,  0,    0, "??????", /* 85 */ /*! missing... */
	"Tech. lamp*", TP_LIGHT | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "??????", /* 86 */ /*! missing... */
	"Tech. lamp 2*", TP_LIGHT | TP_BLOCK},
  {  16,  16,    0,    0,  0,    0, "TFOGB0", /* 87 */
	"Spawn Spot", 0},
  {  16,  16, 6666,  250,  0,    0, "BBRNA0", /* 88 */
	"Boss Brain*", TP_BLOCK | TP_KILL},
  {  16,  16,    0,    0,  0,    0, "BOSFA0", /* 89 */
	"Boss Shooter", 0}
};


#define MINDTHINGINFO2   2001
#define MAXDTHINGINFO2   2049
static ThingDesc DThingInfo2[MAXDTHINGINFO2 + 1 - MINDTHINGINFO2] =
{
  {  20,  16,    0,    0,  0,  560, "SHOTA0", /* 2001 */
	"Shotgun", TP_PICK | TP_WEAPON},
  {  20,  16,    0,    0,  0,  200, "MGUNA0", /* 2002 */
	"Chaingun", TP_PICK | TP_WEAPON},
  {  20,  16,    0,    0,  0,  600, "LAUNA0", /* 2003 */
	"Rocket Launcher", TP_PICK | TP_WEAPON},
  {  20,  16,    0,    0,  0,  800, "PLASA0", /* 2004 */
	"Plasma Gun", TP_PICK | TP_WEAPON},
  {  20,  16,    0,    0,  0,  150, "CSAWA0", /* 2005 */
	"Chainsaw", TP_PICK | TP_WEAPON},
  {  20,  16,    0,    0,  0, 1000, "BFUGA0", /* 2006 */
	"BFG9000", TP_PICK | TP_WEAPON},
  {  20,  16,    0,    0,  0,  100, "CLIPA0", /* 2007 */
	"Ammo Clip", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,  280, "SHELA0", /* 2008 */
	"Shells", TP_PICK | TP_WAMMO},
  { 255,   0,    0,    0,  0,    0, "------", /* 2009 */
	"<UNKNOWN 2009>", TP_BAD},
  {  20,  16,    0,    0,  0,  300, "ROCKA0", /* 2010 */
	"Rocket", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,   10, "STIMA0", /* 2011 */
	"Stim Pack", TP_PICK},
  {  20,  16,    0,    0,  0,   25, "MEDIA0", /* 2012 */
	"Medical Kit", TP_PICK},
  {  20,  16,    0,    0,  0,  100, "SOULA0", /* 2013 */
	"Soul Sphere", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    1, "BON1D0", /* 2014 */
	"Health Potion", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    1, "BON2D0", /* 2015 */
	"Armour Helmet", TP_PICK | TP_ITEM},
  { 255,   0,    0,    0,  0,    0, "------", /* 2016 */
	"<UNKNOWN 2016>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2017 */
	"<UNKNOWN 2017>", TP_BAD},
  {  20,  16,    0,    0,  0,  100, "ARM1B0", /* 2018 */
	"Green Armour", TP_PICK},
  {  20,  16,    0,    0,  0,  200, "ARM2B0", /* 2019 */
	"Blue Armour", TP_PICK},
  { 255,   0,    0,    0,  0,    0, "------", /* 2020 */
	"<UNKNOWN 2020>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2021 */
	"<UNKNOWN 2021>", TP_BAD},
  {  20,  16,    0,    0,  0, 2500, "PINVA0", /* 2022 */
	"Invulnerability", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,  150, "PSTRA0", /* 2023 */
	"Beserk", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,  500, "PINSA0", /* 2024 */
	"Invisibility", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,   25, "SUITA0", /* 2025 */
	"Radiation Suit", TP_PICK},
  {  20,  16,    0,    0,  0,    0, "PMAPB0", /* 2026 */
	"Computer Map", TP_PICK | TP_ITEM},
  { 255,   0,    0,    0,  0,    0, "------", /* 2027 */
    "<UNKNOWN 2027>", TP_BAD},
  {  16,  16,    0,    0,  0,    0, "COLUA0", /* 2028 */
	"Lamp*", TP_LIGHT | TP_BLOCK},
  { 255,   0,    0,    0,  0,    0, "------", /* 2029 */
    "<UNKNOWN 2029>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2030 */
	"<UNKNOWN 2030>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2031 */
	"<UNKNOWN 2031>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2032 */
    "<UNKNOWN 2032>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2033 */
    "<UNKNOWN 2033>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2034 */
	"<UNKNOWN 2034>", TP_BAD},
  {  10,  42,  100,   20,  0,    0, "BAR1B0", /* 2035 */
	"Barrel*", TP_BLOCK | TP_KILL},
  { 255,   0,    0,    0,  0,    0, "------", /* 2036 */
    "<UNKNOWN 2036>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2037 */
    "<UNKNOWN 2037>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2038 */
	"<UNKNOWN 2038>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2039 */
	"<UNKNOWN 2039>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2040 */
	"<UNKNOWN 2040>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2041 */
	"<UNKNOWN 2041>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2042 */
	"<UNKNOWN 2042>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2043 */
	"<UNKNOWN 2043>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2044 */
	"<UNKNOWN 2044>", TP_BAD},
  {  20,  16,    0,    0,  0,   25, "PVISA0", /* 2045 */
	"Lite Amp. Goggles", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0, 1500, "BROKA0", /* 2046 */
	"Box of Rockets", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,  400, "CELLA0", /* 2047 */
	"Energy Cell", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,  500, "AMMOA0", /* 2048 */
	"Box of Ammo", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0, 1400, "SBOXA0", /* 2049 */
	"Box of Shells", TP_PICK | TP_WAMMO}
};

#define MINDTHINGINFO3   3001
#define MAXDTHINGINFO3   3006
static ThingDesc DThingInfo3[MAXDTHINGINFO3 + 1 - MINDTHINGINFO3] =
{
  {  20,  56,  100,   60,  8,  180, "TROOE1", /* 3001 */
	"Imp*", TP_BLOCK | TP_KILL},
  {  30,  56,  400,  150, 10,  225, "SARGF1", /* 3002 */
	"Demon*", TP_BLOCK | TP_KILL},
  {  24,  64, 1000, 1000,  8,  333, "BOSSE1", /* 3003 */
	"Baron of Hell*", TP_BLOCK | TP_KILL},
  {  20,  56,  100,   60,  8,   50, "POSSF1", /* 3004 */
	"Trooper*", TP_BLOCK | TP_KILL},
  {  31,  56,  400,  400,  8,  210, "HEADD1", /* 3005 */
	"Cacodemon*", TP_BLOCK | TP_FLOAT | TP_KILL},
  {  16,  56,   50,  100,  8,   85, "SKULD1", /* 3006 */
	"Lost Soul*", TP_BLOCK | TP_FLOAT | TP_KILL},
};



/* Heretic Things */
#define MINHTHINGINFO0   1
#define MAXHTHINGINFO0   96
static ThingDesc HThingInfo0[MAXHTHINGINFO0 + 1 - MINHTHINGINFO0] =
{
  {  16,  16,    0,    0,  0,    0, "PLAYA1", /* 1 */ /*! Use Composite bitmap */
	"Player 1 Start", TP_PLAYER},
  {  16,  16,    0,    0,  0,    0, "PLAYA1", /* 2 */ /*! Use Composite bitmap */
	"Player 2 Start", TP_PLAYER},
  {  16,  16,    0,    0,  0,    0, "PLAYA1", /* 3 */ /*! Use Composite bitmap */
	"Player 3 Start", TP_PLAYER},
  {  16,  16,    0,    0,  0,    0, "PLAYA1", /* 4 */ /*! Use Composite bitmap */
	"Player 4 Start", TP_PLAYER},
  {  16,  16,    0,    0,  0,    0, "IMPXD1", /* 5 */
	"Flying Gargoyle Leader", TP_BLOCK | TP_KILL | TP_FLOAT},
  {  40,  16,    0,    0,  0,    0, "HEADB1", /* 6 */
	"Ironlich", TP_BLOCK | TP_KILL},
  {  60,  16,    0,    0,  0,    0, "SRCRS1", /* 7 */ /*! Use Composite bitmap */
	"D'sparil", TP_BLOCK | TP_KILL},
  {  20,  16,    0,    0,  0,    0, "BAGHA0", /* 8 */
	"Bag of Holding", TP_PICK | TP_WAMMO | TP_ITEM},
  {  40,  16,    0,    0,  0,    0, "MNTRV1", /* 9 */
	"Maulotaur", TP_BLOCK | TP_KILL},
  {  20,  16,    0,    0,  0,    0, "AMG1A0", /* 10 */
	"Wand Crystal", TP_PICK | TP_WAMMO},
  {  16,  16,    0,    0,  0,    0, "PLAYF1", /* 11 */ /*! Use Composite bitmap */
	"Deathmatch Start", TP_PLAYER},
  {  20,  16,    0,    0,  0,    0, "AMG2A0", /* 12 */
	"Wand Geode", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,    0, "AMM1A0", /* 13 */
	"Mace Spheres", TP_PICK | TP_WAMMO},
  {  10,  16,    0,    0,  0,    0, "TELEH0", /* 14 */
	"Teleport Exit", 0},
  {  24,  16,    0,    0,  0,    0, "WZRDC1", /* 15 */
	"Disciple", TP_BLOCK | TP_KILL | TP_FLOAT},
  {  20,  16,    0,    0,  0,    0, "AMM2A0", /* 16 */
	"Pile of Mace Spheres", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,    0, "SKH1A0", /* 17 */
	"Hanging Skull 70", TP_BLOCK | TP_FLOAT},
  {  20,  16,    0,    0,  0,    0, "AMC1A0", /* 18 */
	"Ethereal Arrows", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,    0, "AMC2A0", /* 19 */
	"Ethereal Quiver", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,    0, "AMS1A0", /* 20 */
	"Lesser Runes", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,    0, "AMS2A0", /* 21 */
	"Greater Runes", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,    0, "AMP1C0", /* 22 */
	"Flame Orb", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,    0, "AMP2B0", /* 23 */
	"Inferno Orb", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,    0, "SKH2A0", /* 24 */
	"Hanging Skull 60", TP_BLOCK | TP_FLOAT},
  {  20,  16,    0,    0,  0,    0, "SKH3A0", /* 25 */
	"Hanging Skull 45", TP_BLOCK | TP_FLOAT},
  {  20,  16,    0,    0,  0,    0, "SKH4A0", /* 26 */
	"Hanging Skull 35", TP_BLOCK | TP_FLOAT},
  {  20,  16,    0,    0,  0,    0, "SRTCA0", /* 27 */
	"Serpent Torch", TP_LIGHT | TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "CHDLA0", /* 28 */
	"Chandelier", TP_LIGHT | TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "SMPLA0", /* 29 */
	"Small Pillar", TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "EGGCC0", /* 30 */
	"Morph Ovum", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "SHD2A0", /* 31 */
	"Enchanted Shield", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "SPHLA0", /* 32 */
	"Mystic Urn", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "TRCHA0", /* 33 */
	"Torch", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "FBMBB0", /* 34 */
	"Time Bomb", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "SPMPA0", /* 35 */
	"Map Scroll", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "ARTIATLP", /* 36 */
	"Chaos Device", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "STGSA0", /* 37 */
	"Small Stalagmite", TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "STGLA0", /* 38 */
	"Large Stalagmite", TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "STCSA0", /* 39 */ /*! Check if it blocks */
	"Small Stalactite", TP_BLOCK | TP_FLOAT},
  {  20,  16,    0,    0,  0,    0, "STCLA0", /* 40 */ /*! Check if it blocks */
	"Large Stalactite", TP_BLOCK | TP_FLOAT},
  {  10,  16,    0,    0,  0,    0, "------", /* 41 */
	"Sound: Waterfall", TP_SOUND},
  {  10,  16,    0,    0,  0,    0, "------", /* 42 */
	"Sound: Wind", TP_SOUND},
  {  10,  16,    0,    0,  0,    0, "PPODG0", /* 43 */ /*! Check props */
	"Pod Spawn Spot", TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "BARLA0", /* 44 */
	"Barrel", TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "MUMMY1", /* 45 */
	"Golem Leader", TP_BLOCK | TP_KILL},
  {  20,  16,    0,    0,  0,    0, "MUMMY1", /* 46 */
	"Golem Leader Ghost", TP_BLOCK | TP_KILL | TP_INVIS},
  {  20,  16,    0,    0,  0,    0, "BRPLA0", /* 47 */
	"Brown Pillar", TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "MOS1A0", /* 48 */
	"Moss 1", TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "MOS2A0", /* 49 */
	"Moss 2", TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "WTRHA0", /* 50 */
	"Wall Torch", TP_LIGHT | TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "HCORA0", /* 51 */
	"Hanging Corpse", TP_BLOCK | TP_FLOAT},
  {  10,  16,    0,    0,  0,    0, "TGLTG0", /* 52 */ /*! Use Composite bitmap */
	"Exit Glitter", TP_LIGHT},
  {  20,  16,    0,    0,  0,    0, "WBLSA0", /* 53 */
	"Dragonclaw", TP_PICK | TP_WEAPON},
  {  20,  16,    0,    0,  0,    0, "AMB1A0", /* 54 */
	"Claw Orb", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,    0, "AMB2A0", /* 55 */
	"Energy Orb", TP_PICK | TP_WAMMO},
  {  20,  16,    0,    0,  0,    0, "SOR2I0", /* 56 */ /*! Check bitmap */
	"D'sparil Spot", 0},
  { 255,   0,    0,    0,  0,    0, "------", /* 57 */
	"<UNKNOWN 57>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 58 */
	"<UNKNOWN 58>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 59 */
	"<UNKNOWN 59>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 60 */
	"<UNKNOWN 60>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 61 */
	"<UNKNOWN 61>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 62 */
	"<UNKNOWN 62>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 63 */
	"<UNKNOWN 63>", TP_BAD},
  {  24,  16,    0,    0,  0,    0, "KNIGG1", /* 64 */
	"Undead Warrior", TP_BLOCK | TP_KILL},
  {  24,  16,    0,    0,  0,    0, "KNIGG1", /* 65 */
	"Undead Warrior Ghost", TP_BLOCK | TP_KILL | TP_INVIS},
  {  16,  16,    0,    0,  0,    0, "IMPXF1", /* 66 */
	"Flying Gargoyle", TP_BLOCK | TP_KILL | TP_FLOAT},
  { 255,   0,    0,    0,  0,    0, "------", /* 67 */
	"<UNKNOWN 67>", TP_BAD},
  {  20,  16,    0,    0,  0,    0, "MUMME1", /* 68 */
	"Golem", TP_BLOCK | TP_KILL},
  {  20,  16,    0,    0,  0,    0, "MUMME1", /* 69 */
	"Golem Ghost", TP_BLOCK | TP_KILL | TP_INVIS},
  {  28,  16,    0,    0,  0,    0, "BEASI1", /* 70 */
	"Weredragon", TP_BLOCK | TP_KILL},
  { 255,   0,    0,    0,  0,    0, "------", /* 71 */
	"<UNKNOWN 71>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 72 */
	"<UNKNOWN 72>", TP_BAD},
  {  20,  16,    0,    0,  0,    0, "AKYYB0", /* 73 */
	"Green Key", TP_PICK},
  {  10,  16,    0,    0,  0,    0, "TGLTB0", /* 74 */ /*! Use Composite bitmap */
	"Teleport Glitter", TP_LIGHT},
  {  20,  16,    0,    0,  0,    0, "INVSA0", /* 75 */
	"Shadowsphere", TP_INVIS | TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "KFR1F0", /* 76 */
	"Fire Brazier", TP_LIGHT | TP_BLOCK},
  { 255,   0,    0,    0,  0,    0, "------", /* 77 */
	"<UNKNOWN 77>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 78 */
	"<UNKNOWN 78>", TP_BAD},
  {  20,  16,    0,    0,  0,    0, "BKYYB0", /* 79 */
	"Blue Key", TP_PICK},
  {  20,  16,    0,    0,  0,    0, "CKYYB0", /* 80 */
	"Yellow Key", TP_PICK},
  {  20,  16,    0,    0,  0,    0, "PTN1A0", /* 81 */
	"Crystal Vial", TP_PICK},
  {  20,  16,    0,    0,  0,    0, "PTN2C0", /* 82 */
	"Quartz Flask", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "SOARA0", /* 83 */
	"Wings of Wrath", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "INVUD0", /* 84 */
	"Ring of Invulnerability", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "SHLDA0", /* 85 */
	"Silver Shield", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "PWBKA0", /* 86 */
	"Tome of Power", TP_PICK | TP_ITEM},
  {  20,  16,    0,    0,  0,    0, "VLCOE0", /* 87 */
	"Volcano", TP_BLOCK},
  { 255,   0,    0,    0,  0,    0, "------", /* 88 */
	"<UNKNOWN 88>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 89 */
	"<UNKNOWN 89>", TP_BAD},
  {  28,  16,    0,    0,  0,    0, "CLNKG1", /* 90 */
	"Sabreclaw", TP_BLOCK | TP_KILL},
  { 255,   0,    0,    0,  0,    0, "------", /* 91 */
	"<UNKNOWN 91>", TP_BAD},
  {  28,  16,    0,    0,  0,    0, "SNKEF1", /* 92 */
	"Ophidian", TP_BLOCK | TP_KILL},
  { 255,   0,    0,    0,  0,    0, "------", /* 93 */
	"<UNKNOWN 93>", TP_BAD},
  {  20,  16,    0,    0,  0,    0, "KGZBA0", /* 94 */ /*! Use Composite bitmap */
	"Blue Key Statue", TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "KGZGA0", /* 95 */ /*! Use Composite bitmap */
	"Green Key Statue", TP_BLOCK},
  {  20,  16,    0,    0,  0,    0, "KGZYA0", /* 96 */ /*! Use Composite bitmap */
	"Yellow Key Statue", TP_BLOCK}
};


#define MINHTHINGINFO1   1200
#define MAXHTHINGINFO1   1209
static ThingDesc HThingInfo1[MAXHTHINGINFO1 + 1 - MINHTHINGINFO1] =
{
  {  10,  16,    0,    0,  0,    0, "------", /* 1200 */
	"Sound: Scream", TP_SOUND},
  {  10,  16,    0,    0,  0,    0, "------", /* 1201 */
	"Sound: Squish", TP_SOUND},
  {  10,  16,    0,    0,  0,    0, "------", /* 1202 */
	"Sound: Water dripping", TP_SOUND},
  {  10,  16,    0,    0,  0,    0, "------", /* 1203 */
	"Sound: Slow footsteps", TP_SOUND},
  {  10,  16,    0,    0,  0,    0, "------", /* 1204 */
	"Sound: Heart beating", TP_SOUND},
  {  10,  16,    0,    0,  0,    0, "------", /* 1205 */
	"Sound: Bells ringing", TP_SOUND},
  {  10,  16,    0,    0,  0,    0, "------", /* 1206 */
	"Sound: Growling", TP_SOUND},
  {  10,  16,    0,    0,  0,    0, "------", /* 1207 */
	"Sound: Magic", TP_SOUND},
  {  10,  16,    0,    0,  0,    0, "------", /* 1208 */
	"Sound: Laughter", TP_SOUND},
  {  10,  16,    0,    0,  0,    0, "------", /* 1209 */
	"Sound: Fast footsteps", TP_SOUND}
};


#define MINHTHINGINFO2   2001
#define MAXHTHINGINFO2   2035
static ThingDesc HThingInfo2[MAXHTHINGINFO2 + 1 - MINHTHINGINFO2] =
{
  {  20,  16,    0,    0,  0,    0, "WBOWA0", /* 2001 */
	"Ethereal Crossbow", TP_PICK | TP_WEAPON},
  {  20,  16,    0,    0,  0,    0, "WMCEA0", /* 2002 */
	"Mace", TP_PICK | TP_WEAPON},
  {  20,  16,    0,    0,  0,    0, "WPHXA0", /* 2003 */
	"Phoenix Rod", TP_PICK | TP_WEAPON},
  {  20,  16,    0,    0,  0,    0, "WSKLA0", /* 2004 */
	"Hellstaff", TP_PICK | TP_WEAPON},
  {  20,  16,    0,    0,  0,    0, "WGNTA0", /* 2005 */
	"Gauntlets of Necromancer", TP_PICK | TP_WEAPON},
  { 255,   0,    0,    0,  0,    0, "------", /* 2006 */
	"<UNKNOWN 2006>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2007 */
	"<UNKNOWN 2007>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2008 */
	"<UNKNOWN 2008>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2009 */
	"<UNKNOWN 2009>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2010 */
	"<UNKNOWN 2010>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2011 */
	"<UNKNOWN 2011>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2012 */
	"<UNKNOWN 2012>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2013 */
	"<UNKNOWN 2013>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2014 */
	"<UNKNOWN 2014>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2015 */
	"<UNKNOWN 2015>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2016 */
	"<UNKNOWN 2016>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2017 */
	"<UNKNOWN 2017>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2018 */
	"<UNKNOWN 2018>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2019 */
	"<UNKNOWN 2019>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2020 */
	"<UNKNOWN 2020>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2021 */
	"<UNKNOWN 2021>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2022 */
	"<UNKNOWN 2022>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2023 */
	"<UNKNOWN 2023>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2024 */
	"<UNKNOWN 2024>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2025 */
	"<UNKNOWN 2025>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2026 */
	"<UNKNOWN 2026>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2027 */
	"<UNKNOWN 2027>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2028 */
	"<UNKNOWN 2028>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2029 */
	"<UNKNOWN 2029>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2030 */
	"<UNKNOWN 2030>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2031 */
	"<UNKNOWN 2031>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2032 */
	"<UNKNOWN 2032>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2033 */
	"<UNKNOWN 2033>", TP_BAD},
  { 255,   0,    0,    0,  0,    0, "------", /* 2034 */
	"<UNKNOWN 2034>", TP_BAD},
  {  10,  16,    0,    0,  0,    0, "PPODA0", /* 2035 */
	"Puff Pod", TP_BLOCK}
};


/*
	Get the name of a Thing.
*/

char *GetThingName(SHORT type)
{
  static char temp[20];

  if (DoomVersion < 16)
	{
	  /* Doom Things */
	  if (type >= MINDTHINGINFO0 && type <= MAXDTHINGINFO0)
		return DThingInfo0[type - MINDTHINGINFO0].name;
	  if (type >= MINDTHINGINFO2 && type <= MAXDTHINGINFO2)
		return DThingInfo2[type - MINDTHINGINFO2].name;
	  if (type >= MINDTHINGINFO3 && type <= MAXDTHINGINFO3)
		return DThingInfo3[type - MINDTHINGINFO3].name;
	}
  else
	{
	  /* Heretic Things */
	  if (type >= MINHTHINGINFO0 && type <= MAXHTHINGINFO0)
		return HThingInfo0[type - MINHTHINGINFO0].name;
	  if (type >= MINHTHINGINFO1 && type <= MAXHTHINGINFO1)
		return HThingInfo1[type - MINHTHINGINFO1].name;
	  if (type >= MINHTHINGINFO2 && type <= MAXHTHINGINFO2)
		return HThingInfo2[type - MINHTHINGINFO2].name;
	}
  sprintf(temp, "<UNKNOWN %d>", type);
  return temp;
}



/*
   Get the sprite name of a thing.
*/

char *GetThingPicName(SHORT type)
{
  if (DoomVersion < 16)
	{
	  /* Doom Things */
	  if (type >= MINDTHINGINFO0 && type <= MAXDTHINGINFO0)
		return DThingInfo0[type - MINDTHINGINFO0].sprite;
	  if (type >= MINDTHINGINFO2 && type <= MAXDTHINGINFO2)
		return DThingInfo2[type - MINDTHINGINFO2].sprite;
	  if (type >= MINDTHINGINFO3 && type <= MAXDTHINGINFO3)
		return DThingInfo3[type - MINDTHINGINFO3].sprite;
	}
  else
	{
	  /* Heretic Things */
	  if (type >= MINHTHINGINFO0 && type <= MAXHTHINGINFO0)
		return HThingInfo0[type - MINHTHINGINFO0].sprite;
	  if (type >= MINHTHINGINFO1 && type <= MAXHTHINGINFO1)
		return HThingInfo1[type - MINHTHINGINFO1].sprite;
	  if (type >= MINHTHINGINFO2 && type <= MAXHTHINGINFO2)
		return HThingInfo2[type - MINHTHINGINFO2].sprite;
	}
  return "------";
}



/*
   Get the size of a thing.
*/

SHORT GetThingRadius(SHORT type)
{
  if (DoomVersion < 16)
	{
	  /* Doom Things */
	  if (type >= MINDTHINGINFO0 && type <= MAXDTHINGINFO0)
		return DThingInfo0[type - MINDTHINGINFO0].radius;
	  if (type >= MINDTHINGINFO2 && type <= MAXDTHINGINFO2)
		return DThingInfo2[type - MINDTHINGINFO2].radius;
	  if (type >= MINDTHINGINFO3 && type <= MAXDTHINGINFO3)
		return DThingInfo3[type - MINDTHINGINFO3].radius;
	}
  else
	{
	  /* Heretic Things */
	  if (type >= MINHTHINGINFO0 && type <= MAXHTHINGINFO0)
		return HThingInfo0[type - MINHTHINGINFO0].radius;
	  if (type >= MINHTHINGINFO1 && type <= MAXHTHINGINFO1)
		return HThingInfo1[type - MINHTHINGINFO1].radius;
	  if (type >= MINHTHINGINFO2 && type <= MAXHTHINGINFO2)
		return HThingInfo2[type - MINHTHINGINFO2].radius;
	}
  return 255;
}



/*
	Get the height of a thing.
*/

SHORT GetThingHeight(SHORT type)
{
  if (DoomVersion < 16)
	{
	  /* Doom Things */
	  if (type >= MINDTHINGINFO0 && type <= MAXDTHINGINFO0)
		return DThingInfo0[type - MINDTHINGINFO0].height;
	  if (type >= MINDTHINGINFO2 && type <= MAXDTHINGINFO2)
		return DThingInfo2[type - MINDTHINGINFO2].height;
	  if (type >= MINDTHINGINFO3 && type <= MAXDTHINGINFO3)
		return DThingInfo3[type - MINDTHINGINFO3].height;
	}
  else
	{
	  /* Heretic Things */
	  if (type >= MINHTHINGINFO0 && type <= MAXHTHINGINFO0)
		return HThingInfo0[type - MINHTHINGINFO0].height;
	  if (type >= MINHTHINGINFO1 && type <= MAXHTHINGINFO1)
		return HThingInfo1[type - MINHTHINGINFO1].height;
	  if (type >= MINHTHINGINFO2 && type <= MAXHTHINGINFO2)
		return HThingInfo2[type - MINHTHINGINFO2].height;
	}
  return 0;
}



/*
   Get the number of health points that a thing has (0 = not alive).
*/

SHORT GetThingHealth(SHORT type)
{
  if (DoomVersion < 16)
	{
	  /* Doom Things */
	  if (type >= MINDTHINGINFO0 && type <= MAXDTHINGINFO0)
		return DThingInfo0[type - MINDTHINGINFO0].health;
	  if (type >= MINDTHINGINFO2 && type <= MAXDTHINGINFO2)
		return DThingInfo2[type - MINDTHINGINFO2].health;
	  if (type >= MINDTHINGINFO3 && type <= MAXDTHINGINFO3)
		return DThingInfo3[type - MINDTHINGINFO3].health;
	}
  else
	{
	  /* Heretic Things */
	  if (type >= MINHTHINGINFO0 && type <= MAXHTHINGINFO0)
		return HThingInfo0[type - MINHTHINGINFO0].health;
	  if (type >= MINHTHINGINFO1 && type <= MAXHTHINGINFO1)
		return HThingInfo1[type - MINHTHINGINFO1].health;
	  if (type >= MINHTHINGINFO2 && type <= MAXHTHINGINFO2)
		return HThingInfo2[type - MINHTHINGINFO2].health;
	}
  return 0;
}



/*
   Get the properties (flags) for a type of thing.
*/

USHORT GetThingProperties(SHORT type)
{
  if (DoomVersion < 16)
	{
	  /* Doom Things */
	  if (type >= MINDTHINGINFO0 && type <= MAXDTHINGINFO0)
		return DThingInfo0[type - MINDTHINGINFO0].props;
	  if (type >= MINDTHINGINFO2 && type <= MAXDTHINGINFO2)
		return DThingInfo2[type - MINDTHINGINFO2].props;
	  if (type >= MINDTHINGINFO3 && type <= MAXDTHINGINFO3)
		return DThingInfo3[type - MINDTHINGINFO3].props;
	}
  else
	{
	  /* Heretic Things */
	  if (type >= MINHTHINGINFO0 && type <= MAXHTHINGINFO0)
		return HThingInfo0[type - MINHTHINGINFO0].props;
	  if (type >= MINHTHINGINFO1 && type <= MAXHTHINGINFO1)
		return HThingInfo1[type - MINHTHINGINFO1].props;
	  if (type >= MINHTHINGINFO2 && type <= MAXHTHINGINFO2)
		return HThingInfo2[type - MINHTHINGINFO2].props;
	}
  return TP_BAD;
}



/*
   Get the colour of a thing (for the map display).
*/

int GetThingColour(SHORT type)
{
  USHORT props;

  props = GetThingProperties(type);
  if (Colour2 == FALSE)
	{
	  if (props & TP_PLAYER)
		return GREEN;
	  if (props & TP_KILL)
		return LIGHTRED;
	  if ((props & TP_WEAPON) || (props & TP_WAMMO))
		return BROWN;
	  if (props & TP_PICK)
		return LIGHTGREEN;
	  if (type == THING_BARREL)
		return MAGENTA;
	  if ((type == THING_TELEPORT) || (type == THING_SPAWNSPOT)
		  || (type == THING_BOSSSHOOTER))
		return YELLOW;
	  return WHITE;
	}
  else
	{
	  /*! This should be cleaned up.  Use the thing propeties. */
	  switch(type)
		{
		case THING_PLAYER1:
		case THING_PLAYER2:
		case THING_PLAYER3:
		case THING_PLAYER4:
		  return GREEN;
		case THING_DEATHMATCH:
		  return LIGHTGREEN;

		case THING_SARGEANT:
		case THING_CHAINGUNNER:
		case THING_TROOPER:
		case THING_IMP:
		case THING_DEMON:
		case THING_SPECTOR:
		case THING_LOSTSOUL:
		case THING_ARACHNOTRON:
		case THING_HELLKNIGHT:
		case THING_WOLFENSTEIN_SS:
		  return RED;
		case THING_CACODEMON:
		case THING_BARON:
		case THING_SPIDERBOSS:
		case THING_CYBERDEMON:
		case THING_ARCHVILE:
		case THING_PAINELEMENTAL:
		case THING_REVENANT:
		case THING_MANCUBUS:
		case THING_BOSSBRAIN:
		case THING_BOSSSHOOTER:
		  return LIGHTRED;

		/* Things you can always pick up */
		case THING_BLUECARD:
		case THING_YELLOWCARD:
		case THING_REDCARD:
		case THING_ARMBONUS1:
		case THING_HLTBONUS1:
		  return MAGENTA;
		case THING_BLUESKULLKEY:
		case THING_YELLOWSKULLKEY:
		case THING_REDSKULLKEY:
		case THING_SOULSPHERE:
		case THING_MEGASPHERE:
		case THING_RADSUIT:
		case THING_MAP:
		case THING_BLURSPHERE:
		case THING_BESERK:
		case THING_INVULN:
		case THING_LITEAMP:
		case THING_BACKPACK:
		  return LIGHTMAGENTA;

		/* Guns */
		case THING_SHOTGUN:
		case THING_CHAINSAW:
		case THING_CHAINGUN:
		  return BLUE;
		case THING_SUPERSHOTGUN:
		case THING_LAUNCHER:
		case THING_PLASMAGUN:
		case THING_BFG9000:
		  return LIGHTBLUE;

		/* Things you can't always pick up */
		case THING_AMMOCLIP:
		case THING_SHELLS:
		case THING_ROCKET:
		case THING_ENERGYCELL:
		case THING_GREENARMOR:
		case THING_STIMPACK:
		  return CYAN;
		case THING_AMMOBOX:
		case THING_SHELLBOX:
		case THING_ROCKETBOX:
		case THING_ENERGYPACK:
		case THING_BLUEARMOR:
		case THING_MEDKIT:
		  return LIGHTCYAN;

		/* Decorations, et al */
		case THING_FUELCAN:
		case THING_TECHCOLUMN:
		case THING_TGREENPILLAR:
		case THING_TREDPILLAR:
		case THING_SGREENPILLAR:
		case THING_SREDPILLAR:
		case THING_PILLARHEART:
		case THING_PILLARSKULL:
		case THING_EYEINSYMBOL:
		case THING_GREYTREE:
		case THING_BROWNSTUB:
		case THING_BROWNTREE:
		case THING_LAMP:
		case THING_CANDLE:
		case THING_CANDELABRA:
		case THING_TBLUETORCH:
		case THING_TGREENTORCH:
		case THING_TREDTORCH:
		case THING_SBLUETORCH:
		case THING_SGREENTORCH:
		case THING_SREDTORCH:
		case THING_DEADPLAYER:
		case THING_DEADTROOPER:
		case THING_DEADSARGEANT:
		case THING_DEADIMP:
		case THING_DEADDEMON:
		case THING_DEADCACODEMON:
		case THING_DEADLOSTSOUL:
		case THING_BONES:
		case THING_BONES2:
		case THING_POOLOFBLOOD:
		case THING_SKULLTOPPOLE:
		case THING_HEADSKEWER:
		case THING_PILEOFSKULLS:
		case THING_IMPALEDBODY:
		case THING_IMPALEDBODY2:
		case THING_SKULLSINFLAMES:
		case THING_HANGINGSWAYING:
		case THING_HANGINGARMSOUT:
		case THING_HANGINGONELEG:
		case THING_HANGINGTORSO:
		case THING_HANGINGLEG:
		case THING_HANGINGSWAYING2:
		case THING_HANGINGARMSOUT2:
		case THING_HANGINGONELEG2:
		case THING_HANGINGTORSO2:
		case THING_HANGINGLEG2:
		  return BROWN;
		case THING_BARREL:
		case THING_TELEPORT:
		case THING_SPAWNSPOT:
		  return YELLOW;
		}
	}
  return WHITE;
}


/*
   get the name of the angle
*/

char *GetAngleName (SHORT angle)
{
   switch (angle)
   {
   case 0:
	  return "East";
	case 45:
	  return "NorthEast";
   case 90:
	  return "North";
   case 135:
	  return "NorthWest";
   case 180:
	  return "West";
	case 225:
	  return "SouthWest";
   case 270:
	  return "South";
   case 315:
	  return "SouthEast";
   }

   static char anglemsg[40];
	wsprintf (anglemsg, "!ILLEGAL! (%d)", angle);

	return (anglemsg);
}



/*
   get string of when something will appear
*/

char *GetWhenName (SHORT when)
{
   static char temp[ 40];

   temp[ 0] = '\0';
   if (when & 0x01)
	  strcat( temp, "D12 ");
   if (when & 0x02)
	  strcat( temp, "D3 ");
	if (when & 0x04)
	  strcat( temp, "D45 ");
	if (when & 0x08)
	  strcat( temp, "Deaf ");
	if (when & 0x10)
	  strcat( temp, "Multi ");
	return temp;
}

typedef struct
{
	int id;
	char name[64];
} KodType;

typedef struct
{
	int id;
	char name[64];
	int numTypes;
	KodType *pType;
} KodObject;

typedef struct
{
	int id;
	char name[64];
} KodRoom;

static int numKodObjects = 0;
static KodObject *pKodObjects = NULL;
static int numKodRooms = 0;
static KodRoom *pKodRooms = NULL;
static char szEmpty[] = "";

int GetNumKodTypes(int objectNum)
{
	if ((objectNum >= 0) && (objectNum < numKodObjects))
	{
		KodObject *pObject = &pKodObjects[objectNum];
		return pObject->numTypes;
	}
	return 0;
}

LPCSTR GetKodTypeName(int objectNum, int index)
{
	if ((objectNum >= 0) && (objectNum < numKodObjects))
	{
		KodObject *pObject = &pKodObjects[objectNum];
		if ((index >= 0) && (index < pObject->numTypes))
			return pObject->pType[index].name;
	}
	return "";
}

int GetKodTypeID(int objectNum, int index)
{
	if ((objectNum >= 0) && (objectNum < numKodObjects))
	{
		KodObject *pObject = &pKodObjects[objectNum];
		if ((index >= 0) && (index < pObject->numTypes))
			return pObject->pType[index].id;
	}
	return 0;
}

int FindKodTypeID(int objectNum, const char *typeName)
{
	if ((objectNum >= 0) && (objectNum < numKodObjects))
	{
		KodObject *pObject = &pKodObjects[objectNum];
		KodType *pType = pObject->pType;
		for (int i=0; i < pObject->numTypes; i++, pType++)
		{
			if (0 == strcmpi(typeName, GetKodTypeDecorativeName(objectNum,i)))
				return pType->id;
			else if (0 == strcmpi(typeName, GetKodTypeName(objectNum,i)))
				return pType->id;
		}
	}
	return 0;
}

LPCSTR GetKodTypeDecorativeName(int objectNum, int index)
{
	if ((objectNum >= 0) && (objectNum < numKodObjects))
	{
		if (0 == strnicmp(GetKodTypeName(objectNum, index), "OO_", 3))
			return GetKodTypeName(objectNum, index)+3;
		else
			return GetKodTypeName(objectNum, index);
	}
	return "";
}

static void UnloadKodTypes(KodObject *pObject)
{
	if (pObject->pType)
		delete []pObject->pType;
	pObject->pType = NULL;
	pObject->numTypes = 0;
}

static void LoadKodTypes(KodObject *pObject, const char *filename)
{
	KodType *pType;
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char file[_MAX_FNAME];
	char ext[_MAX_EXT];
	char typeName[_MAX_PATH];
	FILE *pFile;

	_splitpath(filename,drive,dir,file,ext);
	strcpy(typeName,drive);
	strcat(typeName,dir);
	strcat(typeName,pObject->name);
	strcat(typeName,".txt");

	UnloadKodTypes(pObject);
	pFile = fopen(typeName,"rt");
	if (pFile)
	{
		char buffer[256];
		while (!feof(pFile))
		{
			fgets(buffer,sizeof(buffer),pFile);
			while (strchr(buffer,'\n'))
				*strchr(buffer,'\n') = '\0';
			if ((strlen(buffer) > 0) && (NULL == strchr(" [;%#",buffer[0])))
				pObject->numTypes++;
		}
		fseek(pFile,0,SEEK_SET); // rewind to beginning
		if (pObject->numTypes > 0)
		{
			pObject->pType = new KodType[pObject->numTypes];
			memset(pObject->pType,0,sizeof(KodType) * pObject->numTypes);
			pType = pObject->pType;
			for (int entry = 0; entry < pObject->numTypes; entry++,pType++)
			{
				fgets(buffer,sizeof(buffer),pFile);
				while (strchr(buffer,'\n'))
					*strchr(buffer,'\n') = '\0';
				if ((strlen(buffer) > 0) && (NULL == strchr(" [;%#",buffer[0])))
				{
					strcpy(pType->name,buffer);
					pType->id = entry;
				}
			}
		}
		fclose(pFile);
	}
}

int GetNumKodObjects(void)
{
	return numKodObjects;
}

int FindKodObjectByName(const char *objectName)
{
	for (int index = kodObject; index < numKodObjects; index++)
	{
		if (0 == strcmpi(objectName,pKodObjects[index].name))
			return index;
	}
	return 0;
}

LPCSTR GetKodObjectName(int index)
{
  // Commented out 7/25/04 ARK
//	if (index < numKodObjects)
//		return pKodObjects[index].name;
//	else
//		return szEmpty;

  return szEmpty;
}

LPCSTR GetKodObjectDecorativeName(int index)
{
	return GetKodObjectName(index);
}

int GetKodObjectID(int index)
{
	if (index < numKodObjects)
		return pKodObjects[index].id;
	else
		return 0;
}

void LoadKodObjects(const char *filename)
{
	FILE *file = fopen(filename,"rt");

	UnloadKodObjects();
	if (file)
	{
		char buffer[256];
		while (!feof(file)) // first count the lines
		{
			fgets(buffer,sizeof(buffer),file);
			while (strchr(buffer,'\n'))
				*strchr(buffer,'\n') = '\0';
			if ((strlen(buffer) > 0) && (NULL == strchr(" [;%#",buffer[0])))
				numKodObjects++;
		}
		fseek(file,0,SEEK_SET); // rewind to beginning
		if (numKodObjects > 0)
		{
			pKodObjects = new KodObject[numKodObjects];
			memset(pKodObjects,0,sizeof(KodObject) * numKodObjects);
			for (int entry = kodPlayerBlocking; entry < numKodObjects; entry++)
			{
				pKodObjects[entry].id = entry;
				switch (entry)
				{
				case kodPlayerBlocking:
					strcpy(pKodObjects[entry].name,"Player Blocking");
					break;
				case kodExit:
					strcpy(pKodObjects[entry].name,"Room Exit");
					break;
				case kodMonsterGenerator:
					strcpy(pKodObjects[entry].name,"Monster Generator");
					break;
				case kodEntrance:
					strcpy(pKodObjects[entry].name,"Room Entrance");
               break;
				case kodExtraPreObject2:
				case kodExtraPreObject3:
				case kodExtraPreObject4:
					strcpy(pKodObjects[entry].name,"Undefined");
					break;
				case kodObject:
				default:
					fgets(buffer,sizeof(buffer),file);
					while (strchr(buffer,'\n'))
						*strchr(buffer,'\n') = '\0';
					if ((strlen(buffer) > 0) && (NULL == strchr(" [;%#",buffer[0])))
					{
						strcpy(pKodObjects[entry].name,buffer);
						pKodObjects[entry].id = entry;
						LoadKodTypes(&pKodObjects[entry], filename);
					}
				}
			}
		}
		fclose(file);
	}
	else
		MessageBox(NULL,filename,"Could not open file",MB_OK);
}

int GetKodObjectRadius(int type)
{
	switch (type)
	{
	case kodPlayerBlocking:
		return 64;
	case kodExit:
		return 24;
	case kodMonsterGenerator:
		return 24;
	case kodEntrance:
		return 24;
	case kodExtraPreObject2:
	case kodExtraPreObject3:
	case kodExtraPreObject4:
	case kodObject:					//objects run this value and higher
	default:
		return 24;
	}
}

int GetKodObjectColor(int type)
{
	switch (type)
	{
	case kodPlayerBlocking:
		return RED;
	case kodExit:
		return YELLOW;
	case kodMonsterGenerator:
		return GREEN;
	case kodEntrance:
   	return WHITE;
	case kodExtraPreObject2:
	case kodExtraPreObject3:
	case kodExtraPreObject4:
	case kodObject:					//objects run this value and higher
	default:
		return GREEN;
	}
}

void UnloadKodObjects(void)
{
	if (pKodObjects)
		delete []pKodObjects;
	pKodObjects = NULL;
	numKodObjects = kodObject; // space for player blocking and exit objects ...
}

void LoadKodRooms(const char *filename)
{
	FILE *file = fopen(filename,"rt");

	UnloadKodRooms();
	if (file)
	{
		char buffer[256];
		while (!feof(file)) // first count the lines
		{
			fgets(buffer,sizeof(buffer),file);
			while (strchr(buffer,'\n'))
				*strchr(buffer,'\n') = '\0';
			if ((strlen(buffer) > 0) && (NULL == strchr(" [;%#",buffer[0])))
				numKodRooms++;
		}
		fseek(file,0,SEEK_SET); // rewind to beginning
		if (numKodRooms > 0)
		{
			pKodRooms = new KodRoom[numKodRooms];
			for (int entry = 0; entry < numKodRooms; entry++)
			{
				fgets(buffer,sizeof(buffer),file);
				while (strchr(buffer,'\n'))
					*strchr(buffer,'\n') = '\0';
				if ((strlen(buffer) > 0) && (NULL == strchr(" [;%#",buffer[0])))
				{
					strcpy(pKodRooms[entry].name,buffer);
					pKodRooms[entry].id = entry;
				}
			}
		}
		fclose(file);
	}
	else
		MessageBox(NULL,filename,"Could not open file",MB_OK);
}

void UnloadKodRooms(void)
{
	if (pKodRooms)
		delete []pKodRooms;
	pKodRooms = NULL;
	numKodRooms = 0;
}

int GetNumKodRooms(void)
{
	return numKodRooms;
}

LPCSTR GetKodRoomNameByRoomID(int roomID)
{
	for(int i = 0; i < numKodRooms; i++)
	{
		if (pKodRooms[i].id == roomID)
			return pKodRooms[i].name;
	}
	return "";
}

LPCSTR GetKodRoomName(int index)
{
	if ((index >= 0) && (index < numKodRooms))
		return pKodRooms[index].name;
	else
		return "";
}

LPCSTR GetKodRoomDecorativeName(int index)
{
	if ((index >= 0) && (index < numKodRooms))
	{
		if (0 == strnicmp(pKodRooms[index].name,"RID_",4))
		{
			return &pKodRooms[index].name[4];
		}
		else
		{
			return pKodRooms[index].name;
		}
	}
	else
		return "";
}

int GetKodIndexFromID(int id)
{
	for(int i=0; i < numKodRooms; i++)
	{
		if (id == pKodRooms[i].id)
			return i;
	}
   return -1;
}

int GetKodRoomID(int index)
{
	if ((index >= 0) && (index < numKodRooms))
		return pKodRooms[index].id;
	else
		return 0;
}

int FindKodRoomID(LPCSTR roomName)
{
	char name[256];
	if (0 != strnicmp(roomName,"RID_",4))
	{
		strcpy(name,"RID_");
		strcat(name,roomName);
	}
	else
	{
		strcpy(name,roomName);
	}
	for(int i=0; i < numKodRooms; i++)
	{
		if (0== strcmpi(pKodRooms[i].name,name))
		{
			return pKodRooms[i].id;
		}
	}
	return 0;
}

/* end of file */
