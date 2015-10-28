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
 
 Server-side implementation of a ROO file.
 Loads the BSP tree like the client does and provides
 BSP queries on the tree, such as LineOfSightBSP or CanMoveInRoomBSP

 */

#include "blakserv.h"

#pragma region Macros
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
#define WF_PASSABLE        0x00000004      // wall can be walked through
#define WF_NOLOOKTHROUGH   0x00000020      // bitmap can't be seen through even though it's transparent
#pragma endregion

#pragma region Internal
/**************************************************************************************************************/
/*                                            INTERNAL                                                        */
/*                                   These are not defined in header                                          */
/**************************************************************************************************************/

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

__inline bool BSPCanMoveInRoomTreeInternal(Sector* SectorS, Sector* SectorE, Side* SideS, Side* SideE, V2* Q)
{
   // block moves with end outside
   if (!SectorE || !SideE)
      return false;

   // don't block moves with start outside and end inside
   if (!SectorS || !SideS)
      return true;

   // sides which have no passable flag set always block
   if (!((SideS->Flags & WF_PASSABLE) == WF_PASSABLE))
      return false;

   // get floor heights
   float hFloorS = GetSectorHeightFloorWithDepth(SectorS, Q);
   float hFloorE = GetSectorHeightFloorWithDepth(SectorE, Q);

   // check stepheight (this also requires a lower texture set)
   if (SideS->TextureLower > 0 && (hFloorE - hFloorS > MAXSTEPHEIGHT))
      return false;

   // get ceiling heights
   //float hCeilingS = SECTORHEIGHTCEILING(SectorS, Q);
   float hCeilingE = SECTORHEIGHTCEILING(SectorE, Q);

   // check ceilingheight (this also requires an upper texture set)
   if (SideS->TextureUpper > 0 && (hCeilingE - hFloorS < OBJECTHEIGHTROO))
      return false;

   // check endsector height
   if (hCeilingE - hFloorE < OBJECTHEIGHTROO)
      return false;

   return true;
}

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

bool BSPGetHeightTree(BspNode* Node, V2* P, float* HeightF, float* HeightFWD, float* HeightC, BspLeaf** Leaf)
{
   // note: we don't check for other nullptrs here because caller is doing it and we're recursive..
   if (!Node)
      return false;

   // reached a leaf
   if (Node->Type == BspLeafType && Node->u.leaf.Sector)
   {
      // set output params
      *Leaf = &Node->u.leaf;
      *HeightF = SECTORHEIGHTFLOOR(Node->u.leaf.Sector, P);
      *HeightFWD = GetSectorHeightFloorWithDepth(Node->u.leaf.Sector, P);
      *HeightC = SECTORHEIGHTCEILING(Node->u.leaf.Sector, P);
      return true;
   }

   // still internal node, climb down only one subtree
   else if (Node->Type == BspInternalType)
   {
      return (DISTANCETOSPLITTERSIGNED(&Node->u.internal, P) >= 0.0f) ?
         BSPGetHeightTree(Node->u.internal.RightChild, P, HeightF, HeightFWD, HeightC, Leaf) :
         BSPGetHeightTree(Node->u.internal.LeftChild, P, HeightF, HeightFWD, HeightC, Leaf);
   }

   return false;
}

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

bool BSPCanMoveInRoomTree(BspNode* Node, V2* S, V2* E, Wall** BlockWall)
{
   // reached a leaf or nullchild, movements not blocked by leafs
   if (!Node || Node->Type != BspInternalType)
      return true;

   /****************************************************************/

   // get signed distances from splitter to both endpoints of move
   float distS = DISTANCETOSPLITTERSIGNED(&Node->u.internal, S);
   float distE = DISTANCETOSPLITTERSIGNED(&Node->u.internal, E);

   /****************************************************************/

   // both endpoints far away enough on positive (right) side
   // --> climb down only right subtree
   if (distS > WALLMINDISTANCE && distE > WALLMINDISTANCE)
      return BSPCanMoveInRoomTree(Node->u.internal.RightChild, S, E, BlockWall);

   // both endpoints far away enough on negative (left) side
   // --> climb down only left subtree
   else if (distS < -WALLMINDISTANCE && distE < -WALLMINDISTANCE)
      return BSPCanMoveInRoomTree(Node->u.internal.LeftChild, S, E, BlockWall);

   // endpoints are on different sides, or one/both on infinite line or potentially too close
   // --> check walls of splitter first and then possibly climb down both subtrees
   else
   {
      V2 q;
      Side* sideS;
      Sector* sectorS;
      Side* sideE;
      Sector* sectorE;

      // CASE 1) The move line actually crosses this infinite splitter.
      // This case handles long movelines where S and E can be far away from each other and
      // just checking the distance of E to the line would fail.
      // q contains the intersection point
      if ((distS > 0.0f && distE < 0.0f) ||
          (distS < 0.0f && distE > 0.0f))
      {
         // get 2d line equation coefficients for infinite line through S and E
         double a1, b1, c1;
         a1 = E->Y - S->Y;
         b1 = S->X - E->X;
         c1 = a1 * S->X + b1 * S->Y;

         // get 2d line equation coefficients of splitter
         double a2, b2, c2;
         a2 = -Node->u.internal.A;
         b2 = -Node->u.internal.B;
         c2 = Node->u.internal.C;

         double det = a1*b2 - a2*b1;

         // shouldn't be zero at all, because distS and distE have different sign
         if (!ISZERO(det))
         {
            // intersection point of infinite lines
            q.X = (float)((b2*c1 - b1*c2) / det);
            q.Y = (float)((a1*c2 - a2*c1) / det);

            // must be in boundingbox of SE
            if (ISINBOX(S, E, &q))
            {
               // iterate finite segments (walls) in this splitter
               Wall* wall = Node->u.internal.FirstWall;
               while (wall)
               {
                  // infinite intersection point must also be in bbox of wall
                  // otherwise no intersect
                  if (!ISINBOX(&wall->P1, &wall->P2, &q))
                  {
                     wall = wall->NextWallInPlane;
                     continue;
                  }

                  // set from and to sector / side
                  if (distS > 0.0f)
                  {
                     sideS = wall->RightSide;
                     sectorS = wall->RightSector;
                  }
                  else
                  {
                     sideS = wall->LeftSide;
                     sectorS = wall->LeftSector;
                  }

                  if (distE > 0.0f)
                  {
                     sideE = wall->RightSide;
                     sectorE = wall->RightSector;
                  }
                  else
                  {
                     sideE = wall->LeftSide;
                     sectorE = wall->LeftSector;
                  }

                  // check the transition data for this wall
                  if (!BSPCanMoveInRoomTreeInternal(sectorS, sectorE, sideS, sideE, &q))
                  {
                     *BlockWall = wall;
                     return false;
                  }
                  wall = wall->NextWallInPlane;
               }
            }		 
         }
      }

      // CASE 2) The move line does not cross the infinite splitter, both move endpoints are on the same side.
      // This handles short moves where walls are not intersected, but the endpoint may be too close
      // q will store the too-close endpoint
      else
      {
         // check only getting closer
         if (fabs(distE) <= fabs(distS))
         {
            // iterate finite segments (walls) in this splitter
            Wall* wall = Node->u.internal.FirstWall;
            while (wall)
            {
               // get min. squared distance from move endpoint to line segment
               float dist2 = MinSquaredDistanceToLineSegment(E, &wall->P1, &wall->P2);

               // skip if far enough away
               if (dist2 > WALLMINDISTANCE2)
               {
                  wall = wall->NextWallInPlane;
                  continue;
               }

               q.X = E->X;
               q.Y = E->Y;

               // set from and to sector / side
               // for case 2 (too close) these are based on (S),
               // and (E) is assumed to be on the other side.
               if (distS >= 0.0f)
               {
                  sideS = wall->RightSide;
                  sectorS = wall->RightSector;
                  sideE = wall->LeftSide;
                  sectorE = wall->LeftSector;
               }
               else
               {
                  sideS = wall->LeftSide;
                  sectorS = wall->LeftSector;
                  sideE = wall->RightSide;
                  sectorE = wall->RightSector;
               }

               // check the transition data for this wall
               if (!BSPCanMoveInRoomTreeInternal(sectorS, sectorE, sideS, sideE, &q))
               {
                  *BlockWall = wall;
                  return false;
               }
               wall = wall->NextWallInPlane;
            }
         }
      }

      /****************************************************************/

      // try right subtree first
      bool retval = BSPCanMoveInRoomTree(Node->u.internal.RightChild, S, E, BlockWall);

      // found a collision there? return it
      if (!retval)
         return retval;

      // otherwise try left subtree
      return BSPCanMoveInRoomTree(Node->u.internal.LeftChild, S, E, BlockWall);
   }
}
#pragma endregion

#pragma region Public
/**************************************************************************************************************/
/*                                            PUBLIC                                                          */
/*                     These are defined in header and can be called from outside                             */
/**************************************************************************************************************/

/*********************************************************************************************/
/* BSPGetHeight:  Returns true if location is inside any sector, false otherwise.
/*                  If true, heights are in parameters HeightF (floor), 
/*				    HeightFWD (floor with depth) and HeightC (ceiling) and Leaf is valid.
/*********************************************************************************************/
bool BSPGetHeight(room_type* Room, V2* P, float* HeightF, float* HeightFWD, float* HeightC, BspLeaf** Leaf)
{
   if (!Room || Room->TreeNodesCount == 0 || !P || !HeightF || !HeightFWD || !HeightC)
      return false;

   return BSPGetHeightTree(&Room->TreeNodes[0], P, HeightF, HeightFWD, HeightC, Leaf);
}

/*********************************************************************************************/
/* BSPLineOfSight:  Checks if location E(nd) can be seen from location S(tart)               */
/*********************************************************************************************/
bool BSPLineOfSight(room_type* Room, V3* S, V3* E)
{
   if (!Room || Room->TreeNodesCount == 0 || !S || !E)
      return false;

   return BSPLineOfSightTree(&Room->TreeNodes[0], S, E);
}

/*********************************************************************************************/
/* BSPCanMoveInRoom:  Checks if you can walk a straight line from (S)tart to (E)nd           */
/*********************************************************************************************/
bool BSPCanMoveInRoom(room_type* Room, V2* S, V2* E, int ObjectID, bool moveOutsideBSP, Wall** BlockWall)
{
   if (!Room || Room->TreeNodesCount == 0 || !S || !E)
      return false;

   // allow move to same location
   if (ISZERO(S->X - E->X) && ISZERO(S->Y - E->Y))
   {
#if DEBUGMOVE
      dprintf("MOVEALLOW (START=END)");
#endif
      return true;
   }

   // first check against room geometry
   bool roomok = (moveOutsideBSP || BSPCanMoveInRoomTree(&Room->TreeNodes[0], S, E, BlockWall));

   // already found a collision in room
   if (!roomok)
      return false;

   // otherwise also check against blockers
   Blocker* blocker = Room->Blocker;
   while (blocker)
   {
      // don't block ourself
      if (blocker->ObjectID == ObjectID)
      {
         blocker = blocker->Next;
         continue;
      }

      V2 ms; // from m to s  
      V2SUB(&ms, S, &blocker->Position);
      float ds2 = V2LEN2(&ms);

      // CASE 1) Start is too close
      // Note: IntersectLineCircle below will reject moves starting or ending exactly
      //   on the circle as well as moves going from inside to outside of the circle.
      //   So this case here must handle moves until the object is out of radius again.
      if (ds2 <= OBJMINDISTANCE2)
      {
         V2 me;
         V2SUB(&me, E, &blocker->Position); // from m to e
         float de2 = V2LEN2(&me);

         // end must be farer away than start
         if (de2 <= ds2)
            return false;
      }

      // CASE 2) Start is outside blockradius, verify by intersection algorithm.
      else
      {
         if (IntersectLineCircle(&blocker->Position, OBJMINDISTANCE, S, E))
         {
#if DEBUGMOVE
            dprintf("MOVEBLOCK BY OBJ %i",blocker->ObjectID);
#endif
            return false;
         }
      }
      blocker = blocker->Next;
   }

   return true;
}

/*********************************************************************************************/
/* BSPChangeTexture: Sets textures of sides and/or sectors to given NewTexture num based on Flags
/*********************************************************************************************/
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
/* BSPMoveSector:  Adjust floor or ceiling height of a non-sloped sector. 
/*                 Always instant for now. Otherwise only for speed=0. Height must be in 1:1024.
/*********************************************************************************************/
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
/* BSPGetLocationInfo:  Returns several infos about a location depending on 'QueryFlags'
/*********************************************************************************************/
bool BSPGetLocationInfo(room_type* Room, V2* P, unsigned int QueryFlags, unsigned int* ReturnFlags,
                        float* HeightF, float* HeightFWD, float* HeightC, BspLeaf** Leaf)
{
   if (!Room || !P || !ReturnFlags)
      return false;

   // see what to query
   bool isCheckThingsBox = ((QueryFlags & LIQ_CHECK_THINGSBOX) == LIQ_CHECK_THINGSBOX);
   bool isCheckOjectBlock = ((QueryFlags & LIQ_CHECK_OBJECTBLOCK) == LIQ_CHECK_OBJECTBLOCK);
   bool isGetSectorInfo = ((QueryFlags & LIQ_GET_SECTORINFO) == LIQ_GET_SECTORINFO);
   
   // check if output parameters are provided if query-type needs them
   if (isGetSectorInfo && (!HeightF || !HeightFWD || !HeightC || !Leaf))
      return false;

   // default returnflags
   *ReturnFlags = 0;

   // check outside thingsbox
   if (isCheckThingsBox)
   {
      // out west
      if (P->X <= Room->ThingsBox.Min.X)
         *ReturnFlags |= LIR_TBOX_OUT_W;

      // out east
      else if (P->X >= Room->ThingsBox.Max.X)
         *ReturnFlags |= LIR_TBOX_OUT_E;

      // out north
      if (P->Y <= Room->ThingsBox.Min.Y)
         *ReturnFlags |= LIR_TBOX_OUT_N;

      // out south
      else if (P->Y >= Room->ThingsBox.Max.Y)
         *ReturnFlags |= LIR_TBOX_OUT_S;
   }

   // check too close to blocker
   if (isCheckOjectBlock)
   {
      Blocker* blocker = Room->Blocker;
      while (blocker)
      {
         V2 b;
         V2SUB(&b, P, &blocker->Position);

         // too close
         if (V2LEN2(&b) < OBJMINDISTANCE2)
         {
            *ReturnFlags |= LIR_BLOCKED_OBJECT;
            break;
         }
         blocker = blocker->Next;
      }
   }

   // bsp lookup
   if (isGetSectorInfo && BSPGetHeight(Room, P, HeightF, HeightFWD, HeightC, Leaf))
   {
      *ReturnFlags |= LIR_SECTOR_INSIDE;

      if ((*Leaf)->Sector->FloorTexture > 0)
         *ReturnFlags |= LIR_SECTOR_HASFTEX;

      if ((*Leaf)->Sector->CeilingTexture > 0)
         *ReturnFlags |= LIR_SECTOR_HASCTEX;
   }

   return true;
}

/*********************************************************************************************/
/* BSPGetRandomPoint: Tries up to 'MaxAttempts' times to create a randompoint in 'Room'.
/*                    If return is true, P's coordinates are guaranteed to be:
/*                    (a) inside a sector (b) inside thingsbox (c) outside any obj blockradius
/*********************************************************************************************/
bool BSPGetRandomPoint(room_type* Room, int MaxAttempts, V2* P)
{
	if (!Room || !P)
		return false;

	float heightF, heightFWD, heightC;
	BspLeaf* leaf = NULL;

	for (int i = 0; i < MaxAttempts; i++)
	{
		// generate random coordinates inside the things box
		// we first map the random value to [0.0f , 1.0f] and then to [0.0f , BBOXMAX]
		// note: the minimum of thingsbox is always at 0/0
		P->X = ((float)rand() / (float)RAND_MAX) * Room->ThingsBox.Max.X;
		P->Y = ((float)rand() / (float)RAND_MAX) * Room->ThingsBox.Max.Y;

		// make sure point is exactly expressable in KOD fineness units also
		P->X = (float)ROUNDROOTOKODFINENESS(P->X);
		P->Y = (float)ROUNDROOTOKODFINENESS(P->Y);

		// 1. check for inside valid sector, otherwise roll again
		// note: locations quite close to a wall pass this check!
		if (!BSPGetHeight(Room, P, &heightF, &heightFWD, &heightC, &leaf))
			continue;
		
		// 2. must also have floor texture set
		if (leaf && leaf->Sector->FloorTexture == 0)
			continue;

		// 3. check for being too close to a blocker
		Blocker* blocker = Room->Blocker;
		bool collision = false;
		while (blocker)
		{
			V2 b;
			V2SUB(&b, P, &blocker->Position);

			// too close
			if (V2LEN2(&b) < OBJMINDISTANCE2)
			{
				collision = true;
				break;
			}

			blocker = blocker->Next;
		}

		// too close to a blocker, roll again
		if (collision)
			continue;

		// all good with P
		return true;
	}

	// max attempts reached without success
	return false;
}

/*********************************************************************************************/
/* BSPGetStepTowards:  Returns a location in P param, in a distant of 16 kod fineness units
/*                     away from S on the way towards E.
/*********************************************************************************************/
bool BSPGetStepTowards(room_type* Room, V2* S, V2* E, V2* P, unsigned int* Flags, int ObjectID)
{
   if (!Room || !S || !E || !P || !Flags)
      return false;

   // Monsters that can move through walls or outside the tree will
   // send this flag with state.
   bool moveOutsideBSP = ((*Flags & MSTATE_MOVE_OUTSIDE_BSP) == MSTATE_MOVE_OUTSIDE_BSP);

   // but must not give these back in piState
   *Flags &= ~MSTATE_MOVE_OUTSIDE_BSP;
   
   V2 se, stepend;
   V2SUB(&se, E, S);

   // get length from start to end
   float len = V2LEN(&se);

   // trying to step to old location?
   if (ISZERO(len))
   {
      // set step destination to end
      *P = *E;
      *Flags &= ~ESTATE_AVOIDING;
      *Flags &= ~ESTATE_CLOCKWISE;
      return true;
   }

   // this first normalizes the se vector,
   // then scales to a length of 16 kod-fineunits (=256  roo-fineunits)
   float scale = (1.0f / len) * FINENESSKODTOROO(16.0f);

   // apply the scale on se
   V2SCALE(&se, scale);

   /****************************************************/
   // 1) try direct step towards destination first
   /****************************************************/
   Wall* blockWall = NULL;

   // note: we must verify the location the object is actually going to end up in KOD,
   // this means we must round to the next closer kod-fineness value,  
   // so these values are also exactly expressable in kod coordinates.
   // in fact this makes the vector a variable length between ~15.5 and ~16.5 fine units
   V2ADD(&stepend, S, &se);
   stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
   stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
   if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
   {
      *P = stepend;
      *Flags &= ~ESTATE_AVOIDING;
      *Flags &= ~ESTATE_CLOCKWISE;
      return true;
   }
   
   /****************************************************/
   // 2) can't do direct step
   /****************************************************/

   bool isAvoiding = ((*Flags & ESTATE_AVOIDING) == ESTATE_AVOIDING);
   bool isLeft = ((*Flags & ESTATE_CLOCKWISE) == ESTATE_CLOCKWISE);

   // not yet in clockwise or cclockwise mode
   if (!isAvoiding)
   {
      // if not blocked by a wall, roll a dice to decide
      // how to get around the blocking obj.
      if (!blockWall)
         isLeft = (rand() % 2 == 1);

      // blocked by wall, go first into 'slide-along' direction
      // based on vector towards target
      else
      {
         V2 p1p2;
         V2SUB(&p1p2, &blockWall->P2, &blockWall->P1);

         // note: walls can be aligned in any direction like left->right, right->left,
         //   same with up->down and same also with the movement vector.
         //   The typical angle between vectors, acosf(..) is therefore insufficient to differ.
         //   What is done here is a convert into polar-coordinates (= angle in 0..2pi from x-axis)
         //   The difference (or sum) (-2pi..2pi) then provides up to 8 different cases (quadrants) which must be mapped
         //   to the left or right decision.
         float f1 = atan2f(se.Y, se.X);
         float f2 = atan2f(p1p2.Y, p1p2.X);
         float df = f1 - f2;

         bool q1_pos = (df >= 0.0f && df <= (float)M_PI_2);
         bool q2_pos = (df >= (float)M_PI_2 && df <= (float)M_PI);
         bool q3_pos = (df >= (float)M_PI && df <= (float)(M_PI + M_PI_2));
         bool q4_pos = (df >= (float)(M_PI + M_PI_2) && df <= (float)M_PI*2.0f);
         bool q1_neg = (df <= 0.0f && df >= (float)-M_PI_2);
         bool q2_neg = (df <= (float)-M_PI_2 && df >= (float)-M_PI);
         bool q3_neg = (df <= (float)-M_PI && df >= (float)-(M_PI + M_PI_2));
         bool q4_neg = (df <= (float)-(M_PI + M_PI_2) && df >= (float)-M_PI*2.0f);
 
         isLeft = (q1_pos || q2_pos || q1_neg || q3_neg) ? false : true;

         /*if (isLeft)
            dprintf("trying left first  r: %f", df);
         else
            dprintf("trying right first   r: %f", df);*/
      }
   }

   // must run this possibly twice
   // e.g. left after right failed or right after left failed
   for (int i = 0; i < 2; i++)
   {
      if (isLeft)
      {
         V2 v = se;

		 // try 22.5° left
		 V2ROTATE(&v, 0.5f * (float)-M_PI_4);
		 V2ADD(&stepend, S, &v);
         stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
         stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
         if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
		 {
            *P = stepend;
            *Flags |= ESTATE_AVOIDING;
            *Flags |= ESTATE_CLOCKWISE;
            return true;
         }

         // try 45° left
         V2ROTATE(&v, 0.5f * (float)-M_PI_4);
         V2ADD(&stepend, S, &v);
         stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
         stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
         if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
         {
            *P = stepend;
            *Flags |= ESTATE_AVOIDING;
            *Flags |= ESTATE_CLOCKWISE;
            return true;
         }

         // try 67.5° left
         V2ROTATE(&v, 0.5f * (float)-M_PI_4);
         V2ADD(&stepend, S, &v);
         stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
         stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
         if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
         {
            *P = stepend;
            *Flags |= ESTATE_AVOIDING;
            *Flags |= ESTATE_CLOCKWISE;
            return true;
		 }

         // try 90° left
         V2ROTATE(&v, 0.5f * (float)-M_PI_4);
         V2ADD(&stepend, S, &v);
         stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
         stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
         if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
         {
            *P = stepend;
            *Flags |= ESTATE_AVOIDING;
            *Flags |= ESTATE_CLOCKWISE;
            return true;
         }

         // try 112.5° left
         V2ROTATE(&v, 0.5f * (float)-M_PI_4);
         V2ADD(&stepend, S, &v);
         stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
         stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
         if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
         {
            *P = stepend;
            *Flags |= ESTATE_AVOIDING;
            *Flags |= ESTATE_CLOCKWISE;
            return true;
         }

         // try 135° left
         V2ROTATE(&v, (float)-M_PI_4);
         V2ADD(&stepend, S, &v);
         stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
         stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
         if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
         {
            *P = stepend;
            *Flags |= ESTATE_AVOIDING;
            *Flags |= ESTATE_CLOCKWISE;
            return true;
         }

         // failed to circumvent by going left, switch to right
         isLeft = false;
         *Flags |= ESTATE_AVOIDING;
         *Flags &= ~ESTATE_CLOCKWISE;
      }
      else
      {
         V2 v = se;

         // try 22.5° right
         V2ROTATE(&v, 0.5f * (float)M_PI_4);
         V2ADD(&stepend, S, &v);
         stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
         stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
         if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
         {
            *P = stepend;
            *Flags |= ESTATE_AVOIDING;
            *Flags &= ~ESTATE_CLOCKWISE;
            return true;
         }

         // try 45° right
         V2ROTATE(&v, 0.5f * (float)M_PI_4);
         V2ADD(&stepend, S, &v);
         stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
         stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
         if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
         {
            *P = stepend;
            *Flags |= ESTATE_AVOIDING;
            *Flags &= ~ESTATE_CLOCKWISE;
            return true;
         }

         // try 67.5° right
         V2ROTATE(&v, 0.5f * (float)M_PI_4);
         V2ADD(&stepend, S, &v);
         stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
         stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
         if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
         {
            *P = stepend;
            *Flags |= ESTATE_AVOIDING;
            *Flags &= ~ESTATE_CLOCKWISE;
            return true;
		 }

         // try 90° right
         V2ROTATE(&v, 0.5f * (float)M_PI_4);
         V2ADD(&stepend, S, &v);
         stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
         stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
         if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
         {
            *P = stepend;
            *Flags |= ESTATE_AVOIDING;
            *Flags &= ~ESTATE_CLOCKWISE;
            return true;
         }

         // try 112.5° right
         V2ROTATE(&v, 0.5f * (float)M_PI_4);
         V2ADD(&stepend, S, &v);
         stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
         stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
         if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
         {
            *P = stepend;
            *Flags |= ESTATE_AVOIDING;
            *Flags &= ~ESTATE_CLOCKWISE;
            return true;
         }

         // try 135° right
         V2ROTATE(&v, 0.5f * (float)M_PI_4);
         V2ADD(&stepend, S, &v);
         stepend.X = ROUNDROOTOKODFINENESS(stepend.X);
         stepend.Y = ROUNDROOTOKODFINENESS(stepend.Y);
         if (BSPCanMoveInRoom(Room, S, &stepend, ObjectID, moveOutsideBSP, &blockWall))
         {
            *P = stepend;
            *Flags |= ESTATE_AVOIDING;
            *Flags &= ~ESTATE_CLOCKWISE;
            return true;
         }

         // failed to circumvent by going right, switch to left
         isLeft = true;
         *Flags |= ESTATE_AVOIDING;
         *Flags |= ESTATE_CLOCKWISE;
      }
   }

   /****************************************************/
   // 3) fully stuck
   /****************************************************/

   *P = *S;
   *Flags &= ~ESTATE_AVOIDING;
   *Flags &= ~ESTATE_CLOCKWISE;
   return false;
}

/*********************************************************************************************/
/* BSPBlockerClear:   Clears all registered blocked locations.                               */
/*********************************************************************************************/
void BSPBlockerClear(room_type* Room)
{
   Blocker* blocker = Room->Blocker;
   while (blocker)
   {
      Blocker* tmp = blocker->Next;
      FreeMemory(MALLOC_ID_ROOM, blocker, sizeof(Blocker));
      blocker = tmp;
   }
   Room->Blocker = NULL;
}

/*********************************************************************************************/
/* BSPBlockerRemove:  Removes a blocked location.                                            */
/*********************************************************************************************/
bool BSPBlockerRemove(room_type* Room, int ObjectID)
{
   if (!Room)
      return false;

   Blocker* blocker = Room->Blocker;
   Blocker* previous = NULL;

   while (blocker)
   {
      if (blocker->ObjectID == ObjectID)
      {
         // removing first element
         if (!previous)
            Room->Blocker = blocker->Next;

         // removing not first element
         else
            previous->Next = blocker->Next;

         // now cleanup node
         FreeMemory(MALLOC_ID_ROOM, blocker, sizeof(Blocker));

         return true;
      }

      previous = blocker;
      blocker = blocker->Next;
   }

   return false;
}

/*********************************************************************************************/
/* BSPBlockerAdd:     Adds a blocked location in the room.                                   */
/*********************************************************************************************/
bool BSPBlockerAdd(room_type* Room, int ObjectID, V2* P)
{
   if (!Room || !P)
      return false;

   // alloc
   Blocker* newblocker = (Blocker*)AllocateMemory(MALLOC_ID_ROOM, sizeof(Blocker));

   // set values on new blocker
   newblocker->ObjectID = ObjectID;
   newblocker->Position = *P;
   newblocker->Next = NULL;

   // first blocker
   if (!Room->Blocker)
      Room->Blocker = newblocker;

   else
   {
      // we insert at the beginning because it's
      // (a) faster
      // (b) it makes sure 'static' objects are at the end (unlikely to be touched again)
      newblocker->Next = Room->Blocker;
      Room->Blocker = newblocker;
   }

   return true;
}

/*********************************************************************************************/
/* BSPBlockerMove:    Moves an existing blocked location to somewhere else.                  */
/*********************************************************************************************/
bool BSPBlockerMove(room_type* Room, int ObjectID, V2* P)
{
   if (!Room || !P)
      return false;

   Blocker* blocker = Room->Blocker;
   while (blocker)
   {
      if (blocker->ObjectID == ObjectID)
      {
         blocker->Position = *P;
         return true;
      }
      blocker = blocker->Next;
   }

   return false;
}

/*********************************************************************************************/
/* BSPRooFileLoadServer:  Fill "room" with server-relevant data from given roo file.         */
/*********************************************************************************************/
bool BSPLoadRoom(char *fname, room_type *room)
{
   int i, j, temp;
   unsigned char byte;
   unsigned short unsigshort;
   int offset_client, offset_tree, offset_walls, offset_sides, offset_sectors, offset_things;
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

   // skip pointer to server info
   if (fread(&temp, 1, 4, infile) != 4)
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

   // read pointer to things
   if (fread(&offset_things, 1, 4, infile) != 4)
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

   /***************************** THINGS ****************************************/
   
   fseek(infile, offset_things, SEEK_SET);

   // count of things
   if (fread(&unsigshort, 1, 2, infile) != 2)
   { fclose(infile); return False; }

   // must have exactly two things describing bbox (each thing a vertex)
   if (unsigshort != 2)
   { fclose(infile); return False; }

   // note: Things vertices are stored as INT in (1:64) fineness, based on the
   // coordinate-system origin AS SHOWN IN ROOMEDIT (Y-UP).
   // Also these can be ANY variant of the 2 possible sets describing
   // a diagonal in a rectangle, so not guaranteed to be ordered like min/or max first.
   float x0, x1, y0, y1;

   if (fread(&temp, 1, 4, infile) != 4)
   { fclose(infile); return False; }
   x0 = (float)temp;
   if (fread(&temp, 1, 4, infile) != 4)
   { fclose(infile); return False; }
   y0 = (float)temp;
   if (fread(&temp, 1, 4, infile) != 4)
   { fclose(infile); return False; }
   x1 = (float)temp;
   if (fread(&temp, 1, 4, infile) != 4)
   { fclose(infile); return False; }
   y1 = (float)temp;
   
   // from the 4 bbox points shown in roomedit (defined by 2 vertices)
   // 1) Pick the left-bottom one as minimum (and scale to ROO fineness)
   // 2) Pick the right-up one as maximum (and scale to ROO fineness)
   room->ThingsBox.Min.X = FINENESSKODTOROO(fmin(x0, x1));
   room->ThingsBox.Min.Y = FINENESSKODTOROO(fmin(y0, y1));
   room->ThingsBox.Max.X = FINENESSKODTOROO(fmax(x0, x1));
   room->ThingsBox.Max.Y = FINENESSKODTOROO(fmax(y0, y1));

   // when roomedit saves the ROO, it translates the origin (0/0)
   // into one boundingbox point, so that origin in ROO (0/0)
   // later is roughly equal to (row=1 col=1)
   
   // translate box so minimum is at (0/0)
   room->ThingsBox.Max.X = room->ThingsBox.Max.X - room->ThingsBox.Min.X;
   room->ThingsBox.Max.Y = room->ThingsBox.Max.Y - room->ThingsBox.Min.Y;
   room->ThingsBox.Min.X = 0.0f;
   room->ThingsBox.Min.Y = 0.0f;

   // calculate the old cols/rows values rather than loading them
   room->cols = (int)(room->ThingsBox.Max.X / 1024.0f);
   room->rows = (int)(room->ThingsBox.Max.Y / 1024.0f);
   room->colshighres = (int)(room->ThingsBox.Max.X / 256.0f);
   room->rowshighres = (int)(room->ThingsBox.Max.Y / 256.0f);

   /************************** DONE READNG **********************************/

   fclose(infile);

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
   /****************************************************************************/

   // no initial blockers
   room->Blocker = NULL;

   return True;
}

/*********************************************************************************************/
/* BSPRoomFreeServer:  Free the parts of a room structure used by the server.                */
/*********************************************************************************************/
void BSPFreeRoom(room_type *room)
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

   BSPBlockerClear(room);

   /****************************************************************************/
   /*                               SERVER PARTS                               */
   /****************************************************************************/
  
   room->rows = 0;
   room->cols = 0;
   room->rowshighres = 0;
   room->colshighres = 0;
   room->resource_id = 0;
   room->roomdata_id = 0;
}
#pragma endregion
