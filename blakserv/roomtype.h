// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * roomtype.h:  
 */

#ifndef _ROOMTYPE_H
#define _ROOMTYPE_H

#include "geometry.h"

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
   BspLeafType     = 2
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

/* Room contents to draw */
typedef struct
{
   short rows;             /* Size of room in grid squares */
   short cols;
   short rowshighres;      /* Size of room in highres grid squares */
   short colshighres;

   int  width, height;    /* Size of room in FINENESS units */

   unsigned char **grid;           /* Array that tells whether its legal to move between adjacent squares 
			   * (used only in server) */
   unsigned char **flags;          /* Array that gives per-square flags 
			   * (used only in server) */
   unsigned char **monster_grid;   /* Array that tells whether its legal for monsters to move between adjacent squares  (used only in server) */
   unsigned int **highres_grid;
   int bkgnd;           /* Resource ID of background bitmap; 0 if none */
   unsigned char ambient_light;    /* Intensity of ambient light, 0 = min, 15 = max */

   int security;          /* Security number, unique to each roo file, to ensure that client
                             loads the correct roo file */
   int resource_id;

   /* BSP stuff */
   BspNode*       TreeNodes;
   unsigned short TreeNodesCount;
   Wall*          Walls;
   unsigned short WallsCount;
   Side*          Sides;
   unsigned short SidesCount;
   Sector*        Sectors;
   unsigned short SectorsCount;
} room_type;

#endif /* #ifndef _ROOMTYPE_H */
