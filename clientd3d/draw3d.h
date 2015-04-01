// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * draw3d.h:  Header file for draw3d.c
 */

#ifndef _DRAW3D_H
#define _DRAW3D_H

#define NUM_VIEW_ELEMENTS 8

#include "drawdefs.h"    // Basic constant definitions

/* a divided by b, rounded down */
#define DIVDOWN(a,b) (a)/(b)
/* a divided by b, rounded up (doesn't work for b<0!!!!) */
#define DIVUP(a,b) ((a)+(b)-1)/(b)

/* Given a distance x, return palette index to use.  Must have x > 0 */
/* "v" is strength of light source at viewer; "a" strength of ambient light */
#define LIGHT_INDEX(x, v, a) (min(MAX_LIGHT - 1, \
				  (4*FINENESS*FINENESS) / (x) * (v) / KOD_LIGHT_LEVELS + (a)))

#define LIGHT_NEUTRAL 192       // Light level of sector drawn at ambient light level

// XXX Temporary maximum on # of objects in room 
#define MAXOBJECTS 2000

#define MAX_OBJS_PER_SQUARE 3

#define BITMAP_WIDTH 64  
#define LOG_BITMAP_WIDTH 6
#define BITMAP_HEIGHT 64  

/* Palette index of transparent color */
#define TRANSPARENT_INDEX 254

#define MIN_DISTANCE (FINENESS >> 1)    /* Closest an object can be drawn */

/* How many psuedo degrees to turn on each user command */
#define TURNDEGREES (NUMDEGREES >> 6)   /* 1/64 of a turn per command */

/* What distance (1 grid square = FINENESS units) to move player at a time */
#define MOVEUNITS (FINENESS >> 2)

//	Added for corner treatment. Similar strategy as in merintr\drawint.c. ajw
typedef struct {
  int id;          // Windows resource ID of bitmap
  int width, height;
  BYTE *bits;      // Pointer to bits of bitmap, NULL if none
  int x, y;        // Position on view window to draw elements
} ViewElement;

typedef struct
{
   short leftedge, rightedge;   /* viewable columns of screen (inclusive) */
   long top_a,top_b,top_d;
   long bot_a,bot_b,bot_d;     /* see comments in drawbsp.c */
} ViewCone;

/* Parameters to be passed to DrawRoom3D */
typedef struct {
   ID    player_id;           /* ID of player's object */
   long  viewer_angle;        /* Angle player is facing */
   BYTE  viewer_light;        /* Strength of light source at player */
   long  viewer_x, viewer_y;  /* Player's position, in fine units */
   HDC   hdc;                 /* DC to draw room on */
   int   x, y;                /* Coordinates of upper left corner of room in hdc */
   int   width, height;       /* Size of area to draw */
   int   stretchfactor;       /* 1 = draw normally, 2 = stretch to twice normal size */
   long  viewer_height;       /* Player's head height: 0 = floor, FINENESS = ceiling */
} Draw3DParams;

// Structure used for drawing objects and projectiles
typedef struct {
   Bool draw;                  /* True if object is drawn; false if visible but not drawn */
   ID   id;                    /* object's ID number, or INVALID_ID if a projectile */
   long distance;              /* Distance to object */
   long angle;                 /* Angle object is facing */
   ID   icon_res;              /* Icon resource of object */
   int  group;                 /* Bitmap group to use to display object */
   list_type overlays;         /* Bitmaps to draw over object */
   BYTE light;                 /* Strength of sector light at object */
   int  flags;                 // Boolean object flags.
   BYTE drawingtype;          // Object flags for drawing effects (invisibility, lighting type etc.)
   int  minimapflags;          /* Minimap dot color flags */
   unsigned int  namecolor;    /* Player name color flags */
   object_type objecttype;     /* Enum of object type (i.e. outlaw, murderer, NPC) */
   moveon_type moveontype;     /* MoveOn type of the object */
   int  height;                /* Height to draw object (FINENESS units) */
   int  center;                /* Screen column of center of object */
   int  depth;                 /* Depth under ground to draw object (FINENESS units) */
   BYTE translation;           /* Color translation type */
   BYTE secondtranslation;     /* Overriding color translation type */
   room_contents_node *obj;    // Pointer to room_contents_node for object
} DrawnObject;

Bool InitializeGraphics3D(void);
void CloseGraphics3D(void);
void ViewElementsReposition( AREA* pAreaView );
void GraphicsResetFont(void);
void RecopyRoom3D( HDC hdc, int x, int y, int width, int height, Bool bMiniMap );
void SetLightingInfo(int sun_x, int sun_y, BYTE intensity);

int  DiscreteLog(int x);
void FindOffsets(int d, long theta, int *dx, int *dy);
void NewBackground3D(DWORD bkgnd);
void LightChanged3D(BYTE viewer_light, BYTE ambient_light);
BYTE *GetLightPalette(int distance, BYTE sector_light, long scale, int lightOffset);
int GetLightPaletteIndex(int distance, BYTE sector_light, long scale, int lightOffset);
int  GetFlicker(room_contents_node *obj);

/* 
void DrawMapAsView(room_type *room, Draw3DParams *params); // defined in drawbsp.h
void DrawMiniMap(room_type *room, Draw3DParams *params); // defined in drawbsp.h
*/

#endif /* #ifndef _DRAW3D_H */

