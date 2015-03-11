/*
 * move.cpp:  Compute inter-square movement information.  This information goes into the
 *   room file, and is used by the server to determine which moves are legal without consulting
 *   the BSP tree.
 *
 *   This module also computes square flags, which tell the server about each grid square, such
 *   as whether the square is legal for monsters to enter, etc.
 */

#include "common.h"
#pragma hdrstop

#include "wstructs.h"
#include "bsp.h"

/* Flags for moving to adjacent squares */
#define MOVE_N  0x01
#define MOVE_NE 0x02
#define MOVE_E  0x04
#define MOVE_SE 0x08
#define MOVE_S  0x10
#define MOVE_SW 0x20
#define MOVE_W  0x40
#define MOVE_NW 0x80

/* Flags for per-square info */
#define SF_PLAYABLE 0x01       // Set when square has a floor

#define STEPS_PER_MOVE 5       // # of steps to divide inter-square move into for checking walls

// Depths (FINENESS units) corresponding to various sector depth settings
int sector_depths[] = {0, FinenessClientToKod(FINENESS / 5), 
		       FinenessClientToKod(2 * FINENESS / 5), 
		       FinenessClientToKod(3 * FINENESS / 5)};

#define MAX_STEP_HEIGHT 24     // Height of tallest climbable step
#define OBJECT_HEIGHT   48     // Minimum passable vertical space

extern FileSideDef *FileSideDefs;  /* List of file sidedefs to save */
extern Sector *Sectors;

static Bool MoveTooCloseToWall(BSPTree tree, int old_x, int old_y, int new_x, int new_y,
                               int min_distance);
static Bool MoveCheckBSPNode(BSPnode *node, int old_x, int old_y, int x, int y, int z,
                             int min_distance);
static BSPleaf *BSPFindLeafByPoint(BSPnode *tree, int x, int y);
static Bool GetPointHeights(BSPTree tree, int x, int y, int *floor, int *ceiling);
static int  GetPointFloor(BSPTree tree, int x, int y);
static FileSideDef *GetFileSideDef(int num);
/************************************************************************/
/*
 * ComputeMoveFlags:  Compute and return the movement flags for the given grid
 *   square (row, col), using the walls in tree. 
 *   (rows, cols) gives the size of the room.
 *   min_distance is the minimum allowable distance to an obstacle
 *   row, col, rows, cols are in server grid squares; 1 grid square = FINENESS client units
 */
BYTE ComputeMoveFlags(BSPTree tree, int row, int col, int rows, int cols,
                      int min_distance)
{
   int source_x, source_y;
   int dest_x, dest_y;
   int maxx, maxy;
   BYTE b = 0;

   // Try moving from center of one source square to center of adjacent square
   source_x = col * FINENESS + FINENESS / 2;
   source_y = row * FINENESS + FINENESS / 2;

   maxx = cols * FINENESS;
   maxy = rows * FINENESS;

   // North
   dest_x = source_x;
   dest_y = max(0, source_y - FINENESS);
   if (!MoveTooCloseToWall(tree, source_x, source_y, dest_x, dest_y,
                           min_distance))
      b |= MOVE_N;

   // Northeast
   dest_x = min(maxx, source_x + FINENESS);
   dest_y = max(0, source_y - FINENESS);
   if (!MoveTooCloseToWall(tree, source_x, source_y, dest_x, dest_y,
                           min_distance))
      b |= MOVE_NE;

   // East
   dest_x = min(maxx, source_x + FINENESS);
   dest_y = source_y;
   if (!MoveTooCloseToWall(tree, source_x, source_y, dest_x, dest_y,
                           min_distance))
      b |= MOVE_E;

   // Southeast
   dest_x = min(maxx, source_x + FINENESS);
   dest_y = min(maxy, source_y + FINENESS);
   if (!MoveTooCloseToWall(tree, source_x, source_y, dest_x, dest_y,
                           min_distance))
      b |= MOVE_SE;

   // South
   dest_x = source_x;
   dest_y = min(maxy, source_y + FINENESS);
   if (!MoveTooCloseToWall(tree, source_x, source_y, dest_x, dest_y,
                           min_distance))
      b |= MOVE_S;

   // Southwest
   dest_x = max(0, source_x - FINENESS);
   dest_y = min(maxy, source_y + FINENESS);
   if (!MoveTooCloseToWall(tree, source_x, source_y, dest_x, dest_y,
                           min_distance))
      b |= MOVE_SW;

   // West
   dest_x = max(0, source_x - FINENESS);
   dest_y = source_y;
   if (!MoveTooCloseToWall(tree, source_x, source_y, dest_x, dest_y,
                           min_distance))
      b |= MOVE_W;

   // Northwest
   dest_x = max(0, source_x - FINENESS);
   dest_y = max(0, source_y - FINENESS);
   if (!MoveTooCloseToWall(tree, source_x, source_y, dest_x, dest_y,
                           min_distance))
      b |= MOVE_NW;

   return b;
}
/************************************************************************/
/*
 * MoveTooCloseToWall:  User is moving from (old_x, old_y) to (new_x, new_y).
 *   Return True if this moves user too close to an impassable wall.
 */
Bool MoveTooCloseToWall(BSPTree tree, int old_x, int old_y, int new_x, int new_y,
                        int min_distance)
{
   int i, last_x, last_y, x, y, z;

   // Divide move into a few pieces to prevent a move that jumps clear through
   // a wall, but where either endpoint isn't too close to a wall

   last_x = old_x;
   last_y = old_y;   

   for (i=1; i <= STEPS_PER_MOVE; i++)
   {
      // Find height of floor at object's current location
      z = GetPointFloor(tree, last_x, last_y);

      x = old_x + (new_x - old_x) * i / STEPS_PER_MOVE;
      y = old_y + (new_y - old_y) * i / STEPS_PER_MOVE;

      if (MoveCheckBSPNode(tree, last_x, last_y, x, y, z, min_distance) == True)
	 return True;

      last_x = x;
      last_y = y;
   }
   
   return False;
}
/************************************************************************/
/*
 * MoveCheckBSPNode:  Return True iff given move places player too close 
 *   to a wall in or below the given BSP node (less than min_distance away).
 *
 *   This procedure works by walking the tree inorder, and checking for moving
 *   too close to a wall at each internal node.  Nodes with bounding boxes that can't
 *   be too close to (x, y) are skipped.
 *   z gives height of bottom of player.
 */
Bool MoveCheckBSPNode(BSPnode *node, int old_x, int old_y, int x, int y, int z,
                      int min_distance)
{
   BSPinternal *inode;
   WallData *wall;
	int a, b, c, distance, temp, old_distance;
   int below_height, sector_num;
   FileSideDef *sidedef;

   if (node == NULL || node->type == BSPleaftype)
      return False;

   // Skip this node if we're too far away from its bounding box
   if (node->bbox.x0 - x > min_distance || x - node->bbox.x1 > min_distance ||
       node->bbox.y0 - y > min_distance || y - node->bbox.y1 > min_distance)
      return False;
       
   inode = &node->u.internal;

   // See if we're close to plane of this node--distance between point and line
   a = inode->separator.a;
   b = inode->separator.b;
   c = inode->separator.c;
   distance = (a * x + b * y + c);

   temp = ABS(distance) / min_distance;  // Avoid square root calculation
   if (temp * temp < a * a + b * b)
   {  
//    dprintf("Close to plane a = %d, b = %d, c = %d, distance = %x\n", a, b, c, distance);
      
      // Check walls in this node
      for (wall = inode->walls_in_plane; wall != NULL; wall = wall->next)
      {
	 int minx, miny, maxx, maxy;

	 minx = min(wall->x0, wall->x1);
	 maxx = max(wall->x0, wall->x1);
	 miny = min(wall->y0, wall->y1);
	 maxy = max(wall->y0, wall->y1);
	 
	 // See if we are near the wall itself, and not just the wall's plane
	 if ((minx - min_distance <= x && x <= maxx + min_distance) && 
	     (miny - min_distance <= y && y <= maxy + min_distance))
	 {
	    // OK to move away from wall (prevents user from getting trapped),
	    // but only if both old and new positions are within bounding box.
	    old_distance = (a * old_x + b * old_y + c);
	    if (ABS(distance) > ABS(old_distance) &&
		(minx - min_distance <= old_x && old_x <= maxx + min_distance) &&
		(miny - min_distance <= old_y && old_y <= maxy + min_distance))
	       continue;

	    if (old_distance > 0)
	    {
	       if (wall->pos_sidedef == 0)
		  continue;
	       sidedef = GetFileSideDef(wall->pos_sidedef);
	       sector_num = wall->neg_sector;
	    }
	    else
	    {
	       if (wall->neg_sidedef == 0)
		  continue;
	       sidedef = GetFileSideDef(wall->neg_sidedef);
	       sector_num = wall->pos_sector;
	    }

	    // Check for wading on far side of wall; reduce effective height of wall if found
	    below_height = 0;
	    if (sector_num != -1)
	       below_height = sector_depths[SectorDepth(Sectors[sector_num].blak_flags)];

	    // Can't step up too far; watch bumping your head; see if pasasble
	    if ((sidedef->type_below == 0 || 
		 (sidedef->type_below != 0 && 
		  (wall->z1 - z - below_height) <= MAX_STEP_HEIGHT))
		&&
		(sidedef->type_above == NULL || 
		 (sidedef->type_above != NULL && wall->z2 - z >= OBJECT_HEIGHT)) 
		&&
		(sidedef->flags & WF_PASSABLE))
	       continue;

//	    dprintf("Rejecting wall top = %d, bottom = %d, z = %d, flags = %x\n", wall->z2, wall->z1, z, sidedef->flags);
	    return True;   // Move is illegal
	 }
      }
   }

	// Check children
	return MoveCheckBSPNode(inode->pos_side, old_x, old_y, x, y, z, min_distance) ||
		MoveCheckBSPNode(inode->neg_side, old_x, old_y, x, y, z, min_distance);
}
/************************************************************************/
/*
 * ComputeSquareFlags:  Compute and return the square flags for the given grid
 *   square (row, col), using the given BSP tree.
 *   (rows, cols) gives the size of the room.
 *   row, col, rows, cols are in server grid squares; 1 grid square = FINENESS client units
 */
BYTE ComputeSquareFlags(BSPTree tree, int row, int col, int /*rows*/, int /*cols*/)
{
	int x, y;
	BSPleaf *leaf;
	BYTE byte;

	byte = 0;

	// Check for a real foor in the center of the square
	x = col * FINENESS + FINENESS / 2;
	y = row * FINENESS + FINENESS / 2;

	leaf = BSPFindLeafByPoint(tree, x, y);
	if (leaf != NULL && leaf->floor_type != 0)
		byte |= SF_PLAYABLE;

	return byte;
}

/************************************************************************/
/*
 * ComputeHighResSquareFlags: The new high resolution square has
 *   4 bytes per square. The layout is like:
 *   Bit 0:		Set to 1 if the gridsquare belongs to a sector (0=outside map)
 *   Bit 1-9:	Each bit represents a direction (N, NE, E, SE, ...)
 *   Bit 10-32:	The height of the floor
 *   
 *   Bits 0-9 are the old bits from move and flag grid combined.
 *   Bits 10-32 are new.
 */
int ComputeHighResSquareFlags(BSPTree tree, int row, int col, int rows, int cols, int min_distance)
{
	int x, y;
	int dest_x, dest_y;
	int maxx, maxy;
	int allowed_dirs = 0;
	BSPleaf *leaf;
	
	unsigned int flags = 0;
	
	// 
	x = col * FINENESSHIGHRESGRID + FINENESSHIGHRESGRID / 2;
	y = row * FINENESSHIGHRESGRID + FINENESSHIGHRESGRID / 2;

	/******** DIRECTION FLAGS ************/

	maxx = cols * FINENESSHIGHRESGRID;
	maxy = rows * FINENESSHIGHRESGRID;

	// North
	dest_x = x;
	dest_y = max(0, y - FINENESSHIGHRESGRID);
	if (!MoveTooCloseToWall(tree, x, y, dest_x, dest_y,
							min_distance))
	{
		flags |= ((unsigned int)MOVE_N << 1);
		allowed_dirs++;
	}

	// Northeast
	dest_x = min(maxx, x + FINENESSHIGHRESGRID);
	dest_y = max(0, y - FINENESSHIGHRESGRID);
	if (!MoveTooCloseToWall(tree, x, y, dest_x, dest_y,
							min_distance))
	{
		flags |= ((unsigned int)MOVE_NE << 1);
		allowed_dirs++;
	}

	// East
	dest_x = min(maxx, x + FINENESSHIGHRESGRID);
	dest_y = y;
	if (!MoveTooCloseToWall(tree, x, y, dest_x, dest_y,
							min_distance))
	{
		flags |= ((unsigned int)MOVE_E << 1);
		allowed_dirs++;
	}

	// Southeast
	dest_x = min(maxx, x + FINENESSHIGHRESGRID);
	dest_y = min(maxy, y + FINENESSHIGHRESGRID);
	if (!MoveTooCloseToWall(tree, x, y, dest_x, dest_y,
							min_distance))
	{
		flags |= ((unsigned int)MOVE_SE << 1);
		allowed_dirs++;
	}

	// South
	dest_x = x;
	dest_y = min(maxy, y + FINENESSHIGHRESGRID);
	if (!MoveTooCloseToWall(tree, x, y, dest_x, dest_y,
							min_distance))
	{
		flags |= ((unsigned int)MOVE_S << 1);
		allowed_dirs++;
	}

	// Southwest
	dest_x = max(0, x - FINENESSHIGHRESGRID);
	dest_y = min(maxy, y + FINENESSHIGHRESGRID);
	if (!MoveTooCloseToWall(tree, x, y, dest_x, dest_y,
							min_distance))
	{
		flags |= ((unsigned int)MOVE_SW << 1);
		allowed_dirs++;
	}

	// West
	dest_x = max(0, x - FINENESSHIGHRESGRID);
	dest_y = y;
	if (!MoveTooCloseToWall(tree, x, y, dest_x, dest_y,
							min_distance))
	{
		flags |= ((unsigned int)MOVE_W << 1);
		allowed_dirs++;
	}

	// Northwest
	dest_x = max(0, x - FINENESSHIGHRESGRID);
	dest_y = max(0, y - FINENESSHIGHRESGRID);
	if (!MoveTooCloseToWall(tree, x, y, dest_x, dest_y,
							min_distance))
	{
		flags |= ((unsigned int)MOVE_NW << 1);
		allowed_dirs++;
	}

	/******** MOVABLE / HEIGHT ************/

	leaf = BSPFindLeafByPoint(tree, x, y);
	if (leaf != NULL && leaf->floor_type != 0)
	{
		// set bit 0: walkable square
		// only mark it as walkable
		// if there is at least 1 direction
		// you can get away from that square
		if (allowed_dirs > 0)
	 	  flags |= SF_PLAYABLE;

		// if there is no allowed direction
		// we don't set walkable, but
		// allow any direction
		// in case we still end up there
		else
		{
			flags |= ((unsigned int)MOVE_N << 1);
			flags |= ((unsigned int)MOVE_NE << 1);
			flags |= ((unsigned int)MOVE_E << 1);
			flags |= ((unsigned int)MOVE_SE << 1);
			flags |= ((unsigned int)MOVE_S << 1);
			flags |= ((unsigned int)MOVE_SW << 1);
			flags |= ((unsigned int)MOVE_W << 1);
			flags |= ((unsigned int)MOVE_NW << 1);
		}

		// set bits 9-31: height
		// note: height in sectors is not stored in the same scale
		// as the lines/walls. Instead it uses the EditorScale,
		// which is the same as the fine serverscale.
		// here we turn height into the same scale as highresgrid
		flags |= ((leaf->floor_height * BLAK_FACTOR / FINENESSHIGHRESGRID) << 9);
	}
	else
	{
		// allow any outgoing direction from squares which should not be reached
		flags |= ((unsigned int)MOVE_N << 1);
		flags |= ((unsigned int)MOVE_NE << 1);
		flags |= ((unsigned int)MOVE_E << 1);
		flags |= ((unsigned int)MOVE_SE << 1);
		flags |= ((unsigned int)MOVE_S << 1);
		flags |= ((unsigned int)MOVE_SW << 1);
		flags |= ((unsigned int)MOVE_W << 1);
		flags |= ((unsigned int)MOVE_NW << 1);
	}

	return flags;
}

/*****************************************************************************/
/*
 * BSPFindLeafByPoint:  Return leaf node of tree containing given point, or
 *   NULL if none.
 */
BSPleaf *BSPFindLeafByPoint(BSPnode *tree, int x, int y)
{
   long side;
   BSPnode *pos, *neg;
   
   while (1)
   {
      if (tree == NULL)
	 return NULL;

      switch(tree->type)
      {
      case BSPleaftype:
	 return &tree->u.leaf;

      case BSPinternaltype:
	 side = tree->u.internal.separator.a * x + 
	    tree->u.internal.separator.b * y +
	       tree->u.internal.separator.c;
	 
	 pos = tree->u.internal.pos_side;
	 neg = tree->u.internal.neg_side;
	 if (side == 0)
	    tree = (pos != NULL) ? pos : neg;
	 else if (side > 0)
	    tree = pos;
	 else
	    tree = neg;
	 break;

      default:
	 dprintf("BSPFindLeafByPoint got illegal node type %d\n", tree->type);
	 return NULL;
      }
   }
}
/************************************************************************/
/*
 * GetPointHeights:  If (x, y) is in a valid leaf node, fill in the floor and ceiling
 *   heights of the leaf and return True.  
 *   Otherwise return False.
 */
Bool GetPointHeights(BSPTree tree, int x, int y, int *floor, int *ceiling)
{
   BSPleaf *leaf = BSPFindLeafByPoint(tree, x, y);

   if (leaf == NULL)
      return False;

   *floor   = leaf->floor_height;
   *ceiling = leaf->ceiling_height;
   return True;
}
/************************************************************************/
/*
 * GetPointFloor:  Return height of floor at (x, y).  If (x, y) is not in
 *   a leaf node, return 0.
 */
int GetPointFloor(BSPTree tree, int x, int y)
{
   int floor, ceiling;

   if (!GetPointHeights(tree, x, y, &floor, &ceiling))
      return 0;
   return floor;
}
/************************************************************************/
/*
 * GetFileSideDef:  Return FileSideDef with given number, or NULL if none.
 *   First sidedef = 1.
 */
FileSideDef *GetFileSideDef(int num)
{
   FileSideDef *ptr;
   int count = 0;

   for (ptr = FileSideDefs; ptr != NULL; ptr = ptr->next)
   {
      if (++count == num)
	 return ptr;
   }
   
   return NULL;
}
