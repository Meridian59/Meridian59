// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * roofile.c
 * 
 
 Utility routine to load a room file, written by Andrew.

 */

#include "blakserv.h"

/*****************************************************************************************
********* macro functions ****************************************************************
*****************************************************************************************/
// distance from a point (b) to a BspInternal (a)
#define DISTANCETOSPLITTERSIGNED(a,b)	((a)->A * (b)->X + (a)->B * (b)->Y + (a)->C)

// floorheight of a point (b) in a sector (a)
#define SECTORHEIGHTFLOOR(a, b)	\
   ((!(a)->SlopeInfoFloor) ? (a)->FloorHeight : \
      (-(a)->SlopeInfoFloor->A * (b)->X \
       -(a)->SlopeInfoFloor->B * (b)->Y \
       -(a)->SlopeInfoFloor->D) / (a)->SlopeInfoFloor->C)

// ceilingheight of a point (b) in a sector (a)
#define SECTORHEIGHTCEILING(a, b)	\
   ((!(a)->SlopeInfoCeiling) ? (a)->CeilingHeight : \
      (-(a)->SlopeInfoCeiling->A * (b)->X \
       -(a)->SlopeInfoCeiling->B * (b)->Y \
       -(a)->SlopeInfoCeiling->D) / (a)->SlopeInfoCeiling->C)

/*****************************************************************************************
********* from clientd3d/draw3d.c ********************************************************
*****************************************************************************************/
#define DEPTHMODIFY0    0.0f
#define DEPTHMODIFY1    (ROOFINENESS / 5.0f)
#define DEPTHMODIFY2    (2.0f * ROOFINENESS / 5.0f)
#define DEPTHMODIFY3    (3.0f * ROOFINENESS / 5.0f)

/*****************************************************************************************
********* from clientd3d/bsp.h ***********************************************************
*****************************************************************************************/
#define SF_DEPTH0          0x00000000      // Sector has default (0) depth
#define SF_DEPTH1          0x00000001      // Sector has shallow depth
#define SF_DEPTH2          0x00000002      // Sector has deep depth
#define SF_DEPTH3          0x00000003      // Sector has very deep depth
#define SF_MASK_DEPTH      0x00000003      // Gets depth type from flags
#define SF_SLOPED_FLOOR    0x00000400      // Sector has sloped floor
#define SF_SLOPED_CEILING  0x00000800      // Sector has sloped ceiling
#define WF_TRANSPARENT     0x00000002      // normal wall has some transparency
#define WF_NOLOOKTHROUGH   0x00000020      // bitmap can't be seen through even though it's transparent

// returns floorheight additionally modified by depth sector flags
__inline float GetSectorHeightFloorWithDepth(Sector* Sector, V2* P)
{
   float height = SECTORHEIGHTFLOOR(Sector, P);
   unsigned int depthtype = Sector->Flags & SF_MASK_DEPTH;

   if (depthtype == SF_DEPTH0)
      return (height - DEPTHMODIFY0);

   if (depthtype == SF_DEPTH1)
      return (height - DEPTHMODIFY1);

   if (depthtype == SF_DEPTH2)
      return (height - DEPTHMODIFY2);

   if (depthtype == SF_DEPTH3)
      return (height - DEPTHMODIFY3);

   return height;
}

/*********************************************************************************************/
/*
* BSPGetHeightTree:  Returns the floor or ceiling height of a given location.
                     Returns -MIN_KOD_INT (-134217728) for a location outside of the map.
*/
float BSPGetHeightTree(BspNode* Node, V2* P, bool Floor, bool WithDepth)
{
   if (!Node)
      return (float)-MIN_KOD_INT;

   // reached a leaf, return its floor or ceiling height
   if (Node->Type == BspLeafType && Node->u.leaf.Sector)
   {
      if (Floor)
      {
         return (WithDepth) ? GetSectorHeightFloorWithDepth(Node->u.leaf.Sector, P) :
            SECTORHEIGHTFLOOR(Node->u.leaf.Sector, P);
      }
      else
         return SECTORHEIGHTCEILING(Node->u.leaf.Sector, P);
   }

   // still internal node, climb down only one subtree
   else if (Node->Type == BspInternalType)
   {
      return (DISTANCETOSPLITTERSIGNED(&Node->u.internal, P) >= 0.0f) ?
         BSPGetHeightTree(Node->u.internal.RightChild, P, Floor, WithDepth) :
         BSPGetHeightTree(Node->u.internal.LeftChild, P, Floor, WithDepth);
   }

   return (float)-MIN_KOD_INT;
}

/*********************************************************************************************/
/*
* BSPGetHeight:  Returns the floor or ceiling height in a room for a given location.
                 Returns -MIN_KOD_INT (-134217728) for a location outside of the map.
*/
float BSPGetHeight(room_type* Room, V2* P, bool Floor, bool WithDepth)
{
   if (!Room || Room->TreeNodesCount == 0 || !P)
      return 0.0f;

   return BSPGetHeightTree(&Room->TreeNodes[0], P, Floor, WithDepth);
}

/*********************************************************************************************/
/*
* BSPLineOfSightTree:  Checks if location E(nd) can be seen from location S(tart)
*/
bool BSPLineOfSightTree(BspNode* Node, V3* S, V3* E)
{
   if (!Node)
      return true;

   /****************************************************************/

   // reached a leaf
   if (Node->Type == BspLeafType)
   {
      // no collisions with leafs without sectors
      if (!Node->u.leaf.Sector)
         return true;

      // floors and ceilings don't have backsides.
      // therefore a floor can only collide if
      // the start height is bigger than end height
      // and for ceiling the other way round.
      if (S->Z > E->Z && Node->u.leaf.Sector->FloorTexture > 0)
      {
         for (int i = 0; i < Node->u.leaf.PointsCount - 2; i++)
         {
            bool blocked = IntersectLineTriangle(
               &Node->u.leaf.PointsFloor[i + 2],
               &Node->u.leaf.PointsFloor[i + 1],
               &Node->u.leaf.PointsFloor[0], S, E);

            // blocked by floor
            if (blocked)
            {
#if DEBUGLOS
               dprintf("BLOCK - FLOOR");
#endif
               return false;
            }
         }
      }

      else if (S->Z < E->Z && Node->u.leaf.Sector->CeilingTexture > 0)
      {
         for (int i = 0; i < Node->u.leaf.PointsCount - 2; i++)
         {
            bool blocked = IntersectLineTriangle(
               &Node->u.leaf.PointsCeiling[i + 2],
               &Node->u.leaf.PointsCeiling[i + 1],
               &Node->u.leaf.PointsCeiling[0], S, E);

            // blocked by ceiling
            if (blocked)
            {
#if DEBUGLOS
               dprintf("BLOCK - CEILING");
#endif
               return false;
            }
         }
      }

      // not blocked by this leaf
      return true;
   }

   /****************************************************************/

   // expecting anything else/below to be a splitter
   if (Node->Type != BspInternalType)
      return true;

   // get signed distances to both endpoints of ray
   float distS = DISTANCETOSPLITTERSIGNED(&Node->u.internal, S);
   float distE = DISTANCETOSPLITTERSIGNED(&Node->u.internal, E);

   /****************************************************************/

   // both endpoints on positive (right) side
   // --> climb down only right subtree
   if (distS > EPSILON && distE > EPSILON)
      return BSPLineOfSightTree(Node->u.internal.RightChild, S, E);

   // both endpoints on negative (left) side
   // --> climb down only left subtree
   else if (distS < -EPSILON && distE < -EPSILON)
      return BSPLineOfSightTree(Node->u.internal.LeftChild, S, E);

   // endpoints are on different sides or one/both on infinite line
   // --> check walls of splitter first and then possibly climb down both
   else
   {
      // loop through walls in this splitter and check for collision
      Wall* wall = Node->u.internal.FirstWall;
      while (wall)
      {
         // must have at least a sector on one side of the wall
         // otherwise skip this wall
         if (!wall->RightSector && !wall->LeftSector)
         {
            wall = wall->NextWallInPlane;
            continue;
         }

         // pick side ray is coming from
         Side* side = (distS > 0.0f) ? wall->RightSide : wall->LeftSide;

         // no collision with unset sides
         if (!side)
         {
            wall = wall->NextWallInPlane;
            continue;
         }

         // get 2d line equation coefficients for infinite line through S and E
         float a1, b1, c1;
         a1 = E->Y - S->Y;
         b1 = S->X - E->X;
         c1 = a1 * S->X + b1 * S->Y;

         // get 2d line equation coefficients for infinite line through P1 and P2
         // NOTE: This should be using BspInternal A,B,C coefficients
         float a2, b2, c2;
         a2 = wall->P2.Y - wall->P1.Y;
         b2 = wall->P1.X - wall->P2.X;
         c2 = a2 * wall->P1.X + b2 * wall->P1.Y;

         float det = a1*b2 - a2*b1;

         // parallel (or identical) lines
         if (ISZERO(det))
         {
            wall = wall->NextWallInPlane;
            continue;
         }

         // intersection point of infinite lines
         V2 q;
         q.X = (b2*c1 - b1*c2) / det;
         q.Y = (a1*c2 - a2*c1) / det;

         //dprintf("intersect: %f %f \t p1.x:%f p1.y:%f p2.x:%f p2.y:%f \n", q.X, q.Y, wall->P1.X, wall->P1.Y, wall->P2.X, wall->P2.Y);

         // infinite intersection point must be in BOTH
         // finite segments boundingboxes, otherwise no intersect
         if (!ISINBOX(S, E, &q) || !ISINBOX(&wall->P1, &wall->P2, &q))
         {
            wall = wall->NextWallInPlane;
            continue;
         }

         // vector from (S)tart to (E)nd
         V3 se;
         V3SUB(&se, E, S);

         // find rayheight of (S->E) at intersection point
         float lambda = 1.0f;
         if (!ISZERO(se.X))
            lambda = (q.X - S->X) / se.X;

         else if (!ISZERO(se.Y))
            lambda = (q.Y - S->Y) / se.Y;

         float rayheight = S->Z + lambda * se.Z;

         // get heights of right and left floor/ceiling
         float hFloorRight = (wall->RightSector) ?
            SECTORHEIGHTFLOOR(wall->RightSector, &q) :
            SECTORHEIGHTFLOOR(wall->LeftSector, &q);

         float hFloorLeft = (wall->LeftSector) ?
            SECTORHEIGHTFLOOR(wall->LeftSector, &q) :
            SECTORHEIGHTFLOOR(wall->RightSector, &q);

         float hCeilingRight = (wall->RightSector) ?
            SECTORHEIGHTCEILING(wall->RightSector, &q) :
            SECTORHEIGHTCEILING(wall->LeftSector, &q);

         float hCeilingLeft = (wall->LeftSector) ?
            SECTORHEIGHTCEILING(wall->LeftSector, &q) :
            SECTORHEIGHTCEILING(wall->RightSector, &q);

         // build all 4 possible heights (h0 lowest)
         float h3 = fmax(hCeilingRight, hCeilingLeft);
         float h2 = fmax(fmin(hCeilingRight, hCeilingLeft), fmax(hFloorRight, hFloorLeft));
         float h1 = fmin(fmin(hCeilingRight, hCeilingLeft), fmax(hFloorRight, hFloorLeft));
         float h0 = fmin(hFloorRight, hFloorLeft);

         // above maximum or below minimum
         if (rayheight > h3 || rayheight < h0)
         {
            wall = wall->NextWallInPlane;
            continue;
         }

         // ray intersects middle wall texture
         if (rayheight <= h2 && rayheight >= h1 && side->TextureMiddle > 0)
         {
            // get some flags from the side we're coming from
            // these are applied only to the 'main' = 'middle' texture
            bool isNoLookThrough = ((side->Flags & WF_NOLOOKTHROUGH) == WF_NOLOOKTHROUGH);
            bool isTransparent   = ((side->Flags & WF_TRANSPARENT) == WF_TRANSPARENT);

            // 'transparent' middle textures block only
            // if they are set so by 'no-look-through'
            if (!isTransparent || (isTransparent && isNoLookThrough))
            {
#if DEBUGLOS
               dprintf("WALL %i - MID - (%f/%f/%f)", wall->Num, q.X, q.Y, rayheight);
#endif
               return false;
            }
         }

         // ray intersects upper wall texture
         if (rayheight <= h3 && rayheight >= h2 && side->TextureUpper > 0)
         {
#if DEBUGLOS
            dprintf("WALL %i - UP - (%f/%f/%f)", wall->Num, q.X, q.Y, rayheight);
#endif
            return false;
         }

         // ray intersects lower wall texture
         if (rayheight <= h1 && rayheight >= h0 && side->TextureLower > 0)				
         {
#if DEBUGLOS
            dprintf("WALL %i - LOW - (%f/%f/%f)", wall->Num, q.X, q.Y, rayheight);
#endif
            return false;
         }

         // next wall for next loop
         wall = wall->NextWallInPlane;
      }

      /****************************************************************/

      // try right subtree first
      bool retval = BSPLineOfSightTree(Node->u.internal.RightChild, S, E);

      // found a collision there? return it
      if (!retval)
         return retval;

      // otherwise try left subtree
      return BSPLineOfSightTree(Node->u.internal.LeftChild, S, E);
   }
}

/*********************************************************************************************/
/*
* BSPLineOfSight:  Checks if location E(nd) can be seen from location S(tart)
*/
bool BSPLineOfSight(room_type* Room, V3* S, V3* E)
{
   if (!Room || Room->TreeNodesCount == 0 || !S || !E)
      return false;

   return BSPLineOfSightTree(&Room->TreeNodes[0], S, E);
}

/*********************************************************************************************/
/*
* BSPChangeTexture:  Sets textures of sides and/or sectors to given NewTexture num based on Flags
*/
void BSPChangeTexture(room_type* Room, unsigned int ServerID, unsigned short NewTexture, unsigned int Flags)
{
   bool isAboveWall  = ((Flags & CTF_ABOVEWALL) == CTF_ABOVEWALL);
   bool isNormalWall = ((Flags & CTF_NORMALWALL) == CTF_NORMALWALL);
   bool isBelowWall  = ((Flags & CTF_BELOWWALL) == CTF_BELOWWALL);
   bool isFloor      = ((Flags & CTF_FLOOR) == CTF_FLOOR);
   bool isCeiling    = ((Flags & CTF_CEILING) == CTF_CEILING);
   bool isReset      = ((Flags & CTF_RESET) == CTF_RESET);

   // change on sides
   if (isAboveWall || isNormalWall || isBelowWall)
   {
      for (int i = 0; i < Room->SidesCount; i++)
      {
         Side* side = &Room->Sides[i];

         // server ID does not match
         if (side->ServerID != ServerID)
            continue;

         if (isAboveWall)
            side->TextureUpper = (isReset ? side->TextureUpperOrig : NewTexture);

         if (isNormalWall)
            side->TextureMiddle = (isReset ? side->TextureMiddleOrig : NewTexture);

         if (isBelowWall)
            side->TextureLower = (isReset ? side->TextureLowerOrig : NewTexture);
      }
   }

   // change on sectors
   if (isFloor || isCeiling)
   {
      for (int i = 0; i < Room->SectorsCount; i++)
      {
         Sector* sector = &Room->Sectors[i];

         // server ID does not match
         if (sector->ServerID != ServerID)
            continue;

         if (isFloor)
            sector->FloorTexture = (isReset ? sector->FloorTextureOrig : NewTexture);

         if (isCeiling)
            sector->CeilingTexture = (isReset ? sector->CeilingTextureOrig : NewTexture);
      }
   }
}

/*********************************************************************************************/
/*
* BSPUpdateTreeLeafHeights:  Refreshes the cached Z coordinate of floor/ceiling leaf poly points
*/
void BSPUpdateLeafHeights(room_type* Room, Sector* Sector, bool Floor)
{
   for (int i = 0; i < Room->TreeNodesCount; i++)
   {
      BspNode* node = &Room->TreeNodes[i];

      if (node->Type != BspLeafType || !node->u.leaf.Sector || node->u.leaf.Sector != Sector)
         continue;

      for (int j = 0; j < node->u.leaf.PointsCount; j++)
      {
         V2 p = { node->u.leaf.PointsFloor[j].X, node->u.leaf.PointsFloor[j].Y };

         if (Floor)
            node->u.leaf.PointsFloor[j].Z = SECTORHEIGHTFLOOR(node->u.leaf.Sector, &p);

         else
            node->u.leaf.PointsCeiling[j].Z = SECTORHEIGHTCEILING(node->u.leaf.Sector, &p);
      }
   }
}

/*********************************************************************************************/
/*
* BSPMoveSector:  Adjust floor or ceiling height of a non-sloped sector. 
*                 Always instant for now. Otherwise only for speed=0. Height must be in 1:1024.
*/
void BSPMoveSector(room_type* Room, unsigned int ServerID, bool Floor, float Height, float Speed)
{
   for (int i = 0; i < Room->SectorsCount; i++)
   {
      Sector* sector = &Room->Sectors[i];

      // server ID does not match
      if (sector->ServerID != ServerID)
         continue;

      // move floor
      if (Floor)
      {
         sector->FloorHeight = Height;
         BSPUpdateLeafHeights(Room, sector, true);
      }

      // move ceiling
      else
      {
         sector->CeilingHeight = Height;
         BSPUpdateLeafHeights(Room, sector, false);
      }
   }
}

/*********************************************************************************************/
/*
 * BSPRooFileLoadServer:  Fill "room" with server-relevant data from given roo file.
 *   Return True on success.
 */
Bool BSPRooFileLoadServer(char *fname, room_type *room)
{
   int i, j, temp;
   unsigned char byte;
   unsigned short unsigshort;
   int offset_client, offset_tree, offset_walls, offset_sides, offset_sectors;
   int offset_server;
   char tmpbuf[128];

   FILE *infile = fopen(fname, "rb");
   if (infile == NULL)
      return False;

   /****************************************************************************/
   /*                                HEADER                                    */
   /****************************************************************************/
   
   // check signature
   if (fread(&temp, 1, 4, infile) != 4 || temp != ROO_SIGNATURE)
   { fclose(infile); return False; }

   // check version
   if (fread(&temp, 1, 4, infile) != 4 || temp < ROO_VERSION)
   { fclose(infile); return False; }

   // read room security
   if (fread(&room->security, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   // read pointer to client info
   if (fread(&offset_client, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   // read pointer to server info
   if (fread(&offset_server, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   /****************************************************************************/
   /*                               CLIENT DATA                                */
   /****************************************************************************/
   fseek(infile, offset_client, SEEK_SET);
   
   // skip width
   if (fread(&temp, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   // skip height
   if (fread(&temp, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   // read pointer to bsp tree
   if (fread(&offset_tree, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   // read pointer to walls
   if (fread(&offset_walls, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   // skip offset to editor walls
   if (fread(&temp, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   // read pointer to sides
   if (fread(&offset_sides, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   // read pointer to sectors
   if (fread(&offset_sectors, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   /************************ BSP-TREE ****************************************/

   fseek(infile, offset_tree, SEEK_SET);

   // read count of nodes
   if (fread(&room->TreeNodesCount, 1, 2, infile) != 2)
   { fclose(infile); return False; }

   // allocate tree mem
   room->TreeNodes = (BspNode*)AllocateMemory(
      MALLOC_ID_ROOM, room->TreeNodesCount * sizeof(BspNode));

   for (i = 0; i < room->TreeNodesCount; i++)
   {
      BspNode* node = &room->TreeNodes[i];

      // type
      if (fread(&byte, 1, 1, infile) != 1)
      { fclose(infile); return False; }
      node->Type = (BspNodeType)byte;

      // boundingbox
      if (fread(&node->BoundingBox.Min.X, 1, 4, infile) != 4)
      { fclose(infile); return False; }
      if (fread(&node->BoundingBox.Min.Y, 1, 4, infile) != 4)
      { fclose(infile); return False; }
      if (fread(&node->BoundingBox.Max.X, 1, 4, infile) != 4)
      { fclose(infile); return False; }
      if (fread(&node->BoundingBox.Max.Y, 1, 4, infile) != 4)
      { fclose(infile); return False; }

      if (node->Type == BspInternalType)
      {
         // line equation coefficients of splitter line
         if (fread(&node->u.internal.A, 1, 4, infile) != 4)
         { fclose(infile); return False; }
         if (fread(&node->u.internal.B, 1, 4, infile) != 4)
         { fclose(infile); return False; }
         if (fread(&node->u.internal.C, 1, 4, infile) != 4)
         { fclose(infile); return False; }

         // nums of children
         if (fread(&node->u.internal.RightChildNum, 1, 2, infile) != 2)
         { fclose(infile); return False; }
         if (fread(&node->u.internal.LeftChildNum, 1, 2, infile) != 2)
         { fclose(infile); return False; }

         // first wall in splitter
         if (fread(&node->u.internal.FirstWallNum, 1, 2, infile) != 2)
         { fclose(infile); return False; }
      }
      else if (node->Type == BspLeafType)
      {
         // sector num
         if (fread(&node->u.leaf.SectorNum, 1, 2, infile) != 2)
         { fclose(infile); return False; }

         // points count
         if (fread(&node->u.leaf.PointsCount, 1, 2, infile) != 2)
         { fclose(infile); return False; }

         // allocate memory for points of polygon
         node->u.leaf.PointsFloor = (V3*)AllocateMemory(
            MALLOC_ID_ROOM, node->u.leaf.PointsCount * sizeof(V3));
         node->u.leaf.PointsCeiling = (V3*)AllocateMemory(
            MALLOC_ID_ROOM, node->u.leaf.PointsCount * sizeof(V3));

         // read points
         for (j = 0; j < node->u.leaf.PointsCount; j++)
         {
            if (fread(&node->u.leaf.PointsFloor[j].X, 1, 4, infile) != 4)
            { fclose(infile); return False; }
            if (fread(&node->u.leaf.PointsFloor[j].Y, 1, 4, infile) != 4)
            { fclose(infile); return False; }
			   
            // x,y are same on floor/ceiling
            node->u.leaf.PointsCeiling[j].X = node->u.leaf.PointsFloor[j].X;
            node->u.leaf.PointsCeiling[j].Y = node->u.leaf.PointsFloor[j].Y;
         }
      }
   }

   /*************************** WALLS ****************************************/
   
   fseek(infile, offset_walls, SEEK_SET);

   // count of walls
   if (fread(&room->WallsCount, 1, 2, infile) != 2)
   { fclose(infile); return False; }

   // allocate walls mem
   room->Walls = (Wall*)AllocateMemory(
      MALLOC_ID_ROOM, room->WallsCount * sizeof(Wall));

   for (i = 0; i < room->WallsCount; i++)
   {
      Wall* wall = &room->Walls[i];

      // save 1-based num for debugging
      wall->Num = i + 1;

      // nextwallinplane num
      if (fread(&wall->NextWallInPlaneNum, 1, 2, infile) != 2)
      { fclose(infile); return False; }

      // side nums
      if (fread(&wall->RightSideNum, 1, 2, infile) != 2)
      { fclose(infile); return False; }
      if (fread(&wall->LeftSideNum, 1, 2, infile) != 2)
      { fclose(infile); return False; }

      // endpoints
      if (fread(&wall->P1.X, 1, 4, infile) != 4)
      { fclose(infile); return False; }
      if (fread(&wall->P1.Y, 1, 4, infile) != 4)
      { fclose(infile); return False; }
      if (fread(&wall->P2.X, 1, 4, infile) != 4)
      { fclose(infile); return False; }
      if (fread(&wall->P2.Y, 1, 4, infile) != 4)
      { fclose(infile); return False; }

      // skip length
      if (fread(&temp, 1, 4, infile) != 4)
      { fclose(infile); return False; }

      // skip texture offsets
      if (fread(&temp, 1, 2, infile) != 2)
      { fclose(infile); return False; }
      if (fread(&temp, 1, 2, infile) != 2)
      { fclose(infile); return False; }
      if (fread(&temp, 1, 2, infile) != 2)
      { fclose(infile); return False; }
      if (fread(&temp, 1, 2, infile) != 2)
      { fclose(infile); return False; }

      // sector nums
      if (fread(&wall->RightSectorNum, 1, 2, infile) != 2)
      { fclose(infile); return False; }
      if (fread(&wall->LeftSectorNum, 1, 2, infile) != 2)
      { fclose(infile); return False; }
   }

   /***************************** SIDES ****************************************/

   fseek(infile, offset_sides, SEEK_SET);

   // count of sides
   if (fread(&room->SidesCount, 1, 2, infile) != 2)
   { fclose(infile); return False; }

   // allocate sides mem
   room->Sides = (Side*)AllocateMemory(
      MALLOC_ID_ROOM, room->SidesCount * sizeof(Side));

   for (i = 0; i < room->SidesCount; i++)
   {
      Side* side = &room->Sides[i];

      // serverid
      if (fread(&side->ServerID, 1, 2, infile) != 2)
      { fclose(infile); return False; }

      // middle,upper,lower texture
      if (fread(&side->TextureMiddle, 1, 2, infile) != 2)
      { fclose(infile); return False; }
      if (fread(&side->TextureUpper, 1, 2, infile) != 2)
      { fclose(infile); return False; }
      if (fread(&side->TextureLower, 1, 2, infile) != 2)
      { fclose(infile); return False; }

      // keep track of original texture nums (can change at runtime)
	  side->TextureLowerOrig  = side->TextureLower;
	  side->TextureMiddleOrig = side->TextureMiddle;
	  side->TextureUpperOrig  = side->TextureUpper;

      // flags
     if (fread(&side->Flags, 1, 4, infile) != 4)
      { fclose(infile); return False; }

      // skip speed byte
     if (fread(&temp, 1, 1, infile) != 1)
      { fclose(infile); return False; }
   }

   /***************************** SECTORS ****************************************/

   fseek(infile, offset_sectors, SEEK_SET);

   // count of sectors
   if (fread(&room->SectorsCount, 1, 2, infile) != 2)
   { fclose(infile); return False; }

   // allocate sectors mem
   room->Sectors = (Sector*)AllocateMemory(
      MALLOC_ID_ROOM, room->SectorsCount * sizeof(Sector));

   for (i = 0; i < room->SectorsCount; i++)
   {
      Sector* sector = &room->Sectors[i];
	   
      // serverid
      if (fread(&sector->ServerID, 1, 2, infile) != 2)
      { fclose(infile); return False; }

      // floor+ceiling texture
      if (fread(&sector->FloorTexture, 1, 2, infile) != 2)
      { fclose(infile); return False; }
      if (fread(&sector->CeilingTexture, 1, 2, infile) != 2)
      { fclose(infile); return False; }

	  // keep track of original texture nums (can change at runtime)
      sector->FloorTextureOrig   = sector->FloorTexture;
      sector->CeilingTextureOrig = sector->CeilingTexture;

      // skip texture offsets
      if (fread(&temp, 1, 2, infile) != 2)
      { fclose(infile); return False; }
      if (fread(&temp, 1, 2, infile) != 2)
      { fclose(infile); return False; }

      // floor+ceiling heights (from 1:64 to 1:1024 like the rest)
      if (fread(&unsigshort, 1, 2, infile) != 2)
      { fclose(infile); return False; }
      sector->FloorHeight = FINENESSKODTOROO((float)unsigshort);
      if (fread(&unsigshort, 1, 2, infile) != 2)
      { fclose(infile); return False; }
      sector->CeilingHeight = FINENESSKODTOROO((float)unsigshort);

      // skip light byte
      if (fread(&temp, 1, 1, infile) != 1)
      { fclose(infile); return False; }

      // flags
      if (fread(&sector->Flags, 1, 4, infile) != 4)
      { fclose(infile); return False; }

      // skip speed byte
      if (fread(&temp, 1, 1, infile) != 1)
      { fclose(infile); return False; }
	   
      // possibly load floor slopeinfo
      if ((sector->Flags & SF_SLOPED_FLOOR) == SF_SLOPED_FLOOR)
      {
         sector->SlopeInfoFloor = (SlopeInfo*)AllocateMemory(
            MALLOC_ID_ROOM, sizeof(SlopeInfo));

         // read 3d plane equation coefficients (normal vector)
         if (fread(&sector->SlopeInfoFloor->A, 1, 4, infile) != 4)
         { fclose(infile); return False; }
         if (fread(&sector->SlopeInfoFloor->B, 1, 4, infile) != 4)
         { fclose(infile); return False; }
         if (fread(&sector->SlopeInfoFloor->C, 1, 4, infile) != 4)
         { fclose(infile); return False; }
         if (fread(&sector->SlopeInfoFloor->D, 1, 4, infile) != 4)
         { fclose(infile); return False; }

         // skip x0, y0, textureangle
         if (fread(&temp, 1, 4, infile) != 4)
         { fclose(infile); return False; }
         if (fread(&temp, 1, 4, infile) != 4)
         { fclose(infile); return False; }
         if (fread(&temp, 1, 4, infile) != 4)
         { fclose(infile); return False; }

         // skip unused payload (vertex indices for roomedit)
         if (fread(&tmpbuf, 1, 18, infile) != 18)
         { fclose(infile); return False; }
      }
      else
         sector->SlopeInfoFloor = NULL;

      // possibly load ceiling slopeinfo
      if ((sector->Flags & SF_SLOPED_CEILING) == SF_SLOPED_CEILING)
      {
         sector->SlopeInfoCeiling = (SlopeInfo*)AllocateMemory(
            MALLOC_ID_ROOM, sizeof(SlopeInfo));

         // read 3d plane equation coefficients (normal vector)
         if (fread(&sector->SlopeInfoCeiling->A, 1, 4, infile) != 4)
         { fclose(infile); return False; }
         if (fread(&sector->SlopeInfoCeiling->B, 1, 4, infile) != 4)
         { fclose(infile); return False; }
         if (fread(&sector->SlopeInfoCeiling->C, 1, 4, infile) != 4)
         { fclose(infile); return False; }
         if (fread(&sector->SlopeInfoCeiling->D, 1, 4, infile) != 4)
         { fclose(infile); return False; }

         // skip x0, y0, textureangle
         if (fread(&temp, 1, 4, infile) != 4)
         { fclose(infile); return False; }
         if (fread(&temp, 1, 4, infile) != 4)
         { fclose(infile); return False; }
         if (fread(&temp, 1, 4, infile) != 4)
         { fclose(infile); return False;}

         // skip unused payload (vertex indices for roomedit)
         if (fread(&tmpbuf, 1, 18, infile) != 18)
         { fclose(infile); return False; }
      }
      else
         sector->SlopeInfoCeiling = NULL;
   }

   /*************************************************************************/
   /*                      RESOLVE NUMS TO POINTERS                         */
   /*************************************************************************/

   // walls
   for (int i = 0; i < room->WallsCount; i++)
   {
      Wall* wall = &room->Walls[i];

      // right sector
      if (wall->RightSectorNum > 0 &&
          room->SectorsCount > wall->RightSectorNum - 1)
            wall->RightSector = &room->Sectors[wall->RightSectorNum - 1];
      else
         wall->RightSector = NULL;

      // left sector
      if (wall->LeftSectorNum > 0 &&
         room->SectorsCount > wall->LeftSectorNum - 1)
            wall->LeftSector = &room->Sectors[wall->LeftSectorNum - 1];
      else
         wall->LeftSector = NULL;

      // right side
      if (wall->RightSideNum > 0 &&
         room->SidesCount > wall->RightSideNum - 1)
            wall->RightSide = &room->Sides[wall->RightSideNum - 1];
      else
         wall->RightSide = NULL;

      // left side
      if (wall->LeftSideNum > 0 &&
         room->SidesCount > wall->LeftSideNum - 1)
            wall->LeftSide = &room->Sides[wall->LeftSideNum - 1];
      else
         wall->LeftSide = NULL;

      // next wall in splitter
      if (wall->NextWallInPlaneNum > 0 &&
         room->WallsCount > wall->NextWallInPlaneNum - 1)
            wall->NextWallInPlane = &room->Walls[wall->NextWallInPlaneNum - 1];
      else
         wall->NextWallInPlane = NULL;
   }

   // bsp nodes
   for (int i = 0; i < room->TreeNodesCount; i++)
   {
      BspNode* node = &room->TreeNodes[i];

      // internal nodes
      if (node->Type == BspInternalType)
      {
         // first wall
         if (node->u.internal.FirstWallNum > 0 &&
             room->WallsCount > node->u.internal.FirstWallNum - 1)
               node->u.internal.FirstWall = &room->Walls[node->u.internal.FirstWallNum - 1];
         else
            node->u.internal.FirstWall = NULL;

         // right child
         if (node->u.internal.RightChildNum > 0 &&
             room->TreeNodesCount > node->u.internal.RightChildNum - 1)
               node->u.internal.RightChild = &room->TreeNodes[node->u.internal.RightChildNum - 1];
         else
            node->u.internal.RightChild = NULL;

         // left child
         if (node->u.internal.LeftChildNum > 0 &&
             room->TreeNodesCount > node->u.internal.LeftChildNum - 1)
               node->u.internal.LeftChild = &room->TreeNodes[node->u.internal.LeftChildNum - 1];
         else
            node->u.internal.LeftChild = NULL;
      }

      // leafs
      else if (node->Type == BspLeafType)
      {
         // sector this leaf belongs to
         if (node->u.leaf.SectorNum > 0 &&
             room->SectorsCount > node->u.leaf.SectorNum - 1)
               node->u.leaf.Sector = &room->Sectors[node->u.leaf.SectorNum - 1];
         else
            node->u.leaf.Sector = NULL;
      }
   }

   /*************************************************************************/
   /*                RESOLVE HEIGHTS OF LEAF POLY POINTS                    */
   /*************************************************************************/

   for (int i = 0; i < room->TreeNodesCount; i++)
   {
      BspNode* node = &room->TreeNodes[i];

      if (node->Type != BspLeafType)
         continue;

      for (int j = 0; j < node->u.leaf.PointsCount; j++)
      {
         if (!node->u.leaf.Sector)
            continue;

         V2 p = { node->u.leaf.PointsFloor[j].X, node->u.leaf.PointsFloor[j].Y };

         node->u.leaf.PointsFloor[j].Z = 
            SECTORHEIGHTFLOOR(node->u.leaf.Sector, &p);

         node->u.leaf.PointsCeiling[j].Z =
            SECTORHEIGHTCEILING(node->u.leaf.Sector, &p);
      }
   }

   /****************************************************************************/
   /*                               SERVER GRIDS                               */
   /****************************************************************************/
   
   fseek(infile, offset_server, SEEK_SET);

   // Read size of room
   if (fread(&room->rows, 1, 4, infile) != 4)
   { fclose(infile); return False; }
   if (fread(&room->cols, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   // Allocate and read movement grid
   room->grid = (unsigned char **)AllocateMemory(MALLOC_ID_ROOM,room->rows * sizeof(char *));
   for (i=0; i < room->rows; i++)
   {
      room->grid[i] = (unsigned char *)AllocateMemory(MALLOC_ID_ROOM,room->cols);
      if (fread(room->grid[i], 1, room->cols, infile) != room->cols)
      {
         for (j=0; j <= i; j++)
            FreeMemory(MALLOC_ID_ROOM,room->grid[i],room->cols);
         FreeMemory(MALLOC_ID_ROOM,room->grid,room->rows * sizeof(char *));

         fclose(infile);
         return False;
      }
   }

   // Allocate and read flag grid
   room->flags = (unsigned char **)AllocateMemory(MALLOC_ID_ROOM,room->rows * sizeof(char *));
   for (i=0; i < room->rows; i++)
   {
      room->flags[i] = (unsigned char *)AllocateMemory(MALLOC_ID_ROOM,room->cols);
      if (fread(room->flags[i], 1, room->cols, infile) != room->cols)
      {
         for (j=0; j <= i; j++)
            FreeMemory(MALLOC_ID_ROOM,room->flags[i],room->cols);
         FreeMemory(MALLOC_ID_ROOM,room->flags,room->rows * sizeof(char *));

         fclose(infile);
         return False;
      }
   }

   // Allocate and read monster movement grid
   room->monster_grid = (unsigned char **)AllocateMemory(MALLOC_ID_ROOM,room->rows * sizeof(char *));
   for (i=0; i < room->rows; i++)
   {
      room->monster_grid[i] = (unsigned char *)AllocateMemory(MALLOC_ID_ROOM,room->cols);
      if (fread(room->monster_grid[i], 1, room->cols, infile) != room->cols)
      {
         for (j=0; j <= i; j++)
            FreeMemory(MALLOC_ID_ROOM,room->monster_grid[i],room->cols);
         FreeMemory(MALLOC_ID_ROOM,room->monster_grid,room->rows * sizeof(char *));
			   
         fclose(infile);
         return False;
      }
   }

   // Read highres gridsize
   if (fread(&room->rowshighres, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   if (fread(&room->colshighres, 1, 4, infile) != 4)
   { fclose(infile); return False; }

   // Allocate and read highres grid
   room->highres_grid = (unsigned int **)AllocateMemory(MALLOC_ID_ROOM,room->rowshighres * sizeof(int *));
   for (i=0; i < room->rowshighres; i++)
   {
      room->highres_grid[i] = (unsigned int *)AllocateMemory(MALLOC_ID_ROOM,room->colshighres * sizeof(int));
      if (fread(room->highres_grid[i], 1, room->colshighres * sizeof(int), infile) != room->colshighres * sizeof(int))
      {
         for (j=0; j <= i; j++)
            FreeMemory(MALLOC_ID_ROOM,room->highres_grid[i],room->colshighres * sizeof(int));
         FreeMemory(MALLOC_ID_ROOM,room->highres_grid,room->rowshighres * sizeof(int *));
			   
         fclose(infile);
         return False;
      }
   }	   

   fclose(infile);

   return True;
}

/*********************************************************************************************/
/*
 * BSPRoomFreeServer:  Free the parts of a room structure used by the server.
 */
void BSPRoomFreeServer(room_type *room)
{
   int i;

   /****************************************************************************/
   /*                               CLIENT PARTS                               */
   /****************************************************************************/
   
   // free bsp nodes 'submem'
   for (i = 0; i < room->TreeNodesCount; i++)
   {
      if (room->TreeNodes[i].Type == BspLeafType)
      {
         FreeMemory(MALLOC_ID_ROOM, room->TreeNodes[i].u.leaf.PointsFloor,
            room->TreeNodes[i].u.leaf.PointsCount * sizeof(V3));
         FreeMemory(MALLOC_ID_ROOM, room->TreeNodes[i].u.leaf.PointsCeiling,
            room->TreeNodes[i].u.leaf.PointsCount * sizeof(V3));
      }
   }

   // free sectors submem
   for (i = 0; i < room->SectorsCount; i++)
   {
      if ((room->Sectors[i].Flags & SF_SLOPED_FLOOR) == SF_SLOPED_FLOOR)
         FreeMemory(MALLOC_ID_ROOM, room->Sectors[i].SlopeInfoFloor, sizeof(SlopeInfo));
      if ((room->Sectors[i].Flags & SF_SLOPED_CEILING) == SF_SLOPED_CEILING)
         FreeMemory(MALLOC_ID_ROOM, room->Sectors[i].SlopeInfoCeiling, sizeof(SlopeInfo));
   }

   FreeMemory(MALLOC_ID_ROOM, room->TreeNodes, room->TreeNodesCount * sizeof(BspNode));
   FreeMemory(MALLOC_ID_ROOM, room->Walls, room->WallsCount * sizeof(Wall));
   FreeMemory(MALLOC_ID_ROOM, room->Sides, room->SidesCount * sizeof(Side));
   FreeMemory(MALLOC_ID_ROOM, room->Sectors, room->SectorsCount * sizeof(Sector));

   room->TreeNodesCount = 0;
   room->WallsCount = 0;
   room->SidesCount = 0;
   room->SectorsCount = 0;

   /****************************************************************************/
   /*                               SERVER PARTS                               */
   /****************************************************************************/

   // free lowres movement grid
   for (i=0; i < room->rows; i++)
      FreeMemory(MALLOC_ID_ROOM,room->grid[i],room->cols);
   FreeMemory(MALLOC_ID_ROOM,room->grid,room->rows * sizeof(char *));
   
   // free lowres flags grid
   for (i=0; i < room->rows; i++)
      FreeMemory(MALLOC_ID_ROOM,room->flags[i],room->cols);
   FreeMemory(MALLOC_ID_ROOM,room->flags,room->rows * sizeof(char *));

   // free lowres monster grid
   for (i=0; i < room->rows; i++)
      FreeMemory(MALLOC_ID_ROOM,room->monster_grid[i],room->cols);
   FreeMemory(MALLOC_ID_ROOM,room->monster_grid,room->rows * sizeof(char *));

   // free highres grid
   for (i=0; i < room->rowshighres; i++)
      FreeMemory(MALLOC_ID_ROOM,room->highres_grid[i],room->colshighres * sizeof(int));
   FreeMemory(MALLOC_ID_ROOM,room->highres_grid,room->rowshighres * sizeof(int *));
   
   room->grid = NULL;
   room->flags = NULL;
   room->monster_grid = NULL;
   room->highres_grid = NULL;
   room->rows = 0;
   room->cols = 0;
   room->rowshighres = 0;
   room->colshighres = 0;
   room->resource_id = 0;
}

