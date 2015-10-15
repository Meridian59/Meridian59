// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * roofile.h
 *
 */

#ifndef _ROOFILE_H
#define _ROOFILE_H

#include "geometry.h"

#pragma region Macros
/**************************************************************************************************************/
/*                                           MACROS                                                           */
/**************************************************************************************************************/
#define DEBUGLOS            0                  // switch to 1 to enable debug output for BSP LOS
#define DEBUGMOVE           0                  // switch to 1 to enable debug output for BSP MOVES
#define ROO_VERSION         14                 // required roo fileformat version (V14 = floatingpoint)
#define ROO_SIGNATURE       0xB14F4F52         // signature of ROO files (first 4 bytes)
#define OBJECTHEIGHTROO     768                // estimated object height (used in LOS and MOVE calcs)
#define ROOFINENESS         1024.0f            // fineness used in ROO files
#define FINENESSKODTOROO(x) ((x) * 16.0f)      // scales a value from KOD fineness to ROO fineness
#define FINENESSROOTOKOD(x) ((x) * 0.0625f)    // scales a value from ROO fineness to KOD fineness

#define MAXSTEPHEIGHT       ((float)(24 << 4))                     // (from clientd3d/move.c)
#define PLAYERWIDTH         (31.0f * (float)KODFINENESS * 0.25f)   // (from clientd3d/game.c)
#define WALLMINDISTANCE     (PLAYERWIDTH / 2.0f)                   // (from clientd3d/game.c)
#define WALLMINDISTANCE2    (WALLMINDISTANCE * WALLMINDISTANCE)    // (from clientd3d/game.c)
#define OBJMINDISTANCE      768.0f                                 // 3 highres rows/cols, old value from kod
#define OBJMINDISTANCE2     (OBJMINDISTANCE * OBJMINDISTANCE)

// converts grid coordinates
// input: 1-based value of big row (or col), 0-based value of fine-row (or col)
// output: 0-based value in ROO fineness
#define GRIDCOORDTOROO(big, fine) \
   (FINENESSKODTOROO((float)(big - 1) * (float)KODFINENESS + (float)fine))

#define ROOCOORDTOGRIDBIG(x)  (((int)FINENESSROOTOKOD(x) / KODFINENESS)+1)
#define ROOCOORDTOGRIDFINE(x) ((int)FINENESSROOTOKOD(x) % KODFINENESS)

// converts a floatingpoint-value into KOD integer (boxes into MAX/MIN KOD INT)
#define FLOATTOKODINT(x) \
   (((x) > (float)MAX_KOD_INT) ? MAX_KOD_INT : (((x) < (float)-MIN_KOD_INT) ? -MIN_KOD_INT : (int)x))

// rounds a floatingpoint-value in ROO fineness to next close
// value exactly expressable in KOD fineness units
#define ROUNDROOTOKODFINENESS(a) FINENESSKODTOROO(roundf(FINENESSROOTOKOD(a)))

// from blakston.khd, used in BSPGetNextStepTowards across calls
#define ESTATE_AVOIDING  0x00004000
#define ESTATE_CLOCKWISE 0x00008000

// from blakston.khd, used for monster that can move outside BSP tree
#define MSTATE_MOVE_OUTSIDE_BSP 0x00100000

// query flags for BSPGetLocationInfo
#define LIQ_GET_SECTORINFO           0x00000001
#define LIQ_CHECK_THINGSBOX          0x00000002
#define LIQ_CHECK_OBJECTBLOCK        0x00000004

// return flags for BSPGetLocationInfo
#define LIR_TBOX_OUT_N     0x00000001
#define LIR_TBOX_OUT_E     0x00000002
#define LIR_TBOX_OUT_S     0x00000004
#define LIR_TBOX_OUT_W     0x00000008
#define LIR_TBOX_OUT_NE    0x00000003 //N+E
#define LIR_TBOX_OUT_SE    0x00000006 //S+E
#define LIR_TBOX_OUT_NW    0x00000009 //N+W
#define LIR_TBOX_OUT_SW    0x0000000C //S+W
#define LIR_SECTOR_INSIDE  0x00000010
#define LIR_SECTOR_HASFTEX 0x00000020
#define LIR_SECTOR_HASCTEX 0x00000040
#define LIR_BLOCKED_OBJECT 0x00000100
#pragma endregion

#pragma region Structs
/**************************************************************************************************************/
/*                                          STRUCTS                                                           */
/**************************************************************************************************************/
typedef struct BoundingBox2D
{
   V2 Min;
   V2 Max;
} BoundingBox2D;

typedef struct Side
{
   unsigned short ServerID;
   unsigned short TextureMiddle;
   unsigned short TextureUpper;
   unsigned short TextureLower;
   unsigned int   Flags;
   unsigned short TextureMiddleOrig;
   unsigned short TextureUpperOrig;
   unsigned short TextureLowerOrig;
} Side;

typedef struct SlopeInfo
{
   float A;
   float B;
   float C;
   float D;
} SlopeInfo;

typedef struct Sector
{
   unsigned short ServerID;
   unsigned short FloorTexture;
   unsigned short CeilingTexture;
   float          FloorHeight;
   float          CeilingHeight;
   unsigned int   Flags;
   SlopeInfo*     SlopeInfoFloor;
   SlopeInfo*     SlopeInfoCeiling;
   unsigned short FloorTextureOrig;
   unsigned short CeilingTextureOrig;
} Sector;

typedef struct Wall
{
   unsigned short Num;
   unsigned short NextWallInPlaneNum;
   unsigned short RightSideNum;
   unsigned short LeftSideNum;
   V2             P1;
   V2             P2;
   unsigned short RightSectorNum;
   unsigned short LeftSectorNum;
   Sector*        RightSector;
   Sector*        LeftSector;
   Side*          RightSide;
   Side*          LeftSide;
   Wall*          NextWallInPlane;
} Wall;

typedef enum BspNodeType
{
   BspInternalType = 1,
   BspLeafType = 2
} BspNodeType;

typedef struct BspInternal
{
   float           A;
   float           B;
   float           C;
   unsigned short  RightChildNum;
   unsigned short  LeftChildNum;
   unsigned short  FirstWallNum;
   struct BspNode* RightChild;
   struct BspNode* LeftChild;
   Wall*           FirstWall;
} BspInternal;

typedef struct BspLeaf
{
   unsigned short SectorNum;
   unsigned short PointsCount;
   V3*            PointsFloor;
   V3*            PointsCeiling;
   Sector*        Sector;
} BspLeaf;

typedef struct BspNode
{
   BspNodeType    Type;
   BoundingBox2D  BoundingBox;

   union
   {
      BspInternal internal;
      BspLeaf     leaf;
   } u;

} BspNode;

typedef struct Blocker
{
   int ObjectID;
   V2 Position;
   Blocker* Next;
} Blocker;

typedef struct room_type
{
   int roomdata_id; 
   int resource_id;
   
   int rows;             /* Size of room in grid squares */
   int cols;
   int rowshighres;      /* Size of room in highres grid squares */
   int colshighres;

   int security;         /* Security number, to ensure that client loads the correct roo file */
   
   BoundingBox2D  ThingsBox;
   Blocker*       Blocker;

   BspNode*       TreeNodes;
   unsigned short TreeNodesCount;
   Wall*          Walls;
   unsigned short WallsCount;
   Side*          Sides;
   unsigned short SidesCount;
   Sector*        Sectors;
   unsigned short SectorsCount; 
} room_type;
#pragma endregion

#pragma region Methods
/**************************************************************************************************************/
/*                                          METHODS                                                           */
/**************************************************************************************************************/
bool  BSPGetHeight(room_type* Room, V2* P, float* HeightF, float* HeightFWD, float* HeightC, BspLeaf** Leaf);
bool  BSPCanMoveInRoom(room_type* Room, V2* S, V2* E, int ObjectID, bool moveOutsideBSP, Wall** BlockWall);
bool  BSPLineOfSight(room_type* Room, V3* S, V3* E);
void  BSPChangeTexture(room_type* Room, unsigned int ServerID, unsigned short NewTexture, unsigned int Flags);
void  BSPMoveSector(room_type* Room, unsigned int ServerID, bool Floor, float Height, float Speed);
bool  BSPGetLocationInfo(room_type* Room, V2* P, unsigned int QueryFlags, unsigned int* ReturnFlags, float* HeightF, float* HeightFWD, float* HeightC, BspLeaf** Leaf);
bool  BSPGetRandomPoint(room_type* Room, int MaxAttempts, V2* P);
bool  BSPGetStepTowards(room_type* Room, V2* S, V2* E, V2* P, unsigned int* Flags, int ObjectID);
bool  BSPBlockerAdd(room_type* Room, int ObjectID, V2* P);
bool  BSPBlockerMove(room_type* Room, int ObjectID, V2* P);
bool  BSPBlockerRemove(room_type* Room, int ObjectID);
void  BSPBlockerClear(room_type* Room);
bool  BSPLoadRoom(char *fname, room_type *room);
void  BSPFreeRoom(room_type *room);
#pragma endregion

#endif
