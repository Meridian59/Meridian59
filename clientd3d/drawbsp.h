// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * drawbsp.h:  Header for drawbsp.c
 */

#ifndef _DRAWBSP_H
#define _DRAWBSP_H

/* data structures generated during BSP tree walk that record what
 * should be drawn.
 */

typedef enum
{
   DrawWallType = 1,
   DrawFloorType,
   DrawCeilingType,
   DrawObjectType,
   DrawBackgroundType,
   DrawSlopedFloorType,
   DrawSlopedCeilingType
} DrawType;

enum { WALL_ABOVE = 1, WALL_BELOW, WALL_NORMAL };  // Wall types
enum { FLOOR, CEILING };

typedef struct
{
   WallData *wall;
   BYTE wall_type;
   signed char side;
} DrawWallStruct;

typedef struct
{
   BSPleaf *leaf;
} DrawFloorStruct;

typedef struct
{
   BSPleaf *leaf;
} DrawCeilingStruct;

typedef struct
{
   ObjectData *object;
} DrawObjectStruct;

typedef struct
{
  int foo;
} DrawBackgroundStruct;

// CA: place holder structure for sloped surface draw info
typedef struct
{
  BSPleaf *leaf; // other use pointer to Leaf ?
} DrawSlopedStruct;

typedef struct
{
   BYTE type;
   ViewCone cone;
   union
   {
      DrawWallStruct       wall;
      DrawFloorStruct      floor;
      DrawCeilingStruct    ceiling;
      DrawObjectStruct     object;
      DrawBackgroundStruct back;
      DrawSlopedStruct	   slope;
   } u;
} DrawItem;

void DrawBSP(room_type *room, Draw3DParams *params, long width, Bool draw);
BSPleaf *BSPFindLeafByPoint(BSPnode *tree, int x, int y);

BSPnode  *BSPBuildTree(room_type *room);
WallData *BSPGetNewWall(void);
BSPnode  *BSPGetNewNode(void);
BSPTree   BSPFreeTree(BSPTree tree);
int       BSPGetNumWalls(void);

Bool GetRoomHeight(BSPnode *tree, long *ceiling, long *floor, int *flags, long x, long y);
long GetFloorHeight(long x, long y, Sector *sector);
long GetCeilingHeight(long x, long y, Sector *sector);

void BSPInitialize(void);
void BSPEnterRoom(room_type *room);
void DrawRoom3D(room_type *room, Draw3DParams *params);
void DrawMapAsView(room_type *room, Draw3DParams *params); /* in Draw3d.h */
void DrawMiniMap(room_type *room, Draw3DParams *params); /* in Draw3d.h */
void EnterNewRoom3D(room_type *room);
Bool GetRoomHeightRad(BSPnode *tree, long *ceiling, long *floor, int *flags, int x, int y, long r);

typedef struct ConeTreeNode
{
   ViewCone cone;
   int height;           /* height of subtree rooted at node */
   struct ConeTreeNode *parent;        /* up the cone tree   */
   struct ConeTreeNode *left, *right;  /* down the cone tree */
   struct ConeTreeNode *prev, *next;   /* pre-order walk     */
} ConeTreeNode;
ConeTreeNode *search_for_first(long col);
#endif /* #ifndef _DRAWBSP_H */
