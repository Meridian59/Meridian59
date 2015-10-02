// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * room.h:  Contains definition of room type.
 */

#ifndef _ROOM_H
#define _ROOM_H

#define FINENESSKODTOROO(x) ((x) * 16.0f)      // scales a value from KOD fineness to ROO fineness

typedef struct V2
{
   float X;
   float Y;
} V2;

typedef struct BoundingBox2D
{
   V2 Min;
   V2 Max;
} BoundingBox2D;

/* Room contents to draw */
typedef struct
{
   WORD rows;             /* Size of room in grid squares */
   WORD cols;
   int  width, height;    /* Size of room in FINENESS units */

   list_type contents;    /* Objects in room */
   list_type projectiles; /* Projectiles currently active in room */

   DWORD bkgnd;           /* Resource ID of background bitmap; 0 if none */
   BYTE ambient_light;    /* Intensity of ambient light, 0 = min, 15 = max */

   BSPTree tree;          /* BSP tree of room */
   BSPnode *nodes;        // Array of nodes in use
   WallDataList walls;    // Array of walls in use
   Sector *sectors;       // Array of sectors in use
   Sidedef *sidedefs;     // Array of sidedefs in use
   BoundingBox2D ThingsBox; // Bounding box of room

   int num_nodes, num_walls, num_sectors, num_sidedefs;

   int security;          /* Security number, unique to each roo file, to ensure that client
                             loads the correct roo file */

   list_type bg_overlays; // Overlays to draw on background bitmap

   MapAnnotation annotations[MAX_ANNOTATIONS];
   Bool annotations_changed;  // True when user has changed an annotation in the room
   int  annotations_offset;   // Offset of current room's annotations in map file

   DWORD flags;
   DWORD overrideDepth[4];
} room_type;

Bool AnimateRoom(room_type *room, int dt);

#endif /* #ifndef _ROOM_H */
