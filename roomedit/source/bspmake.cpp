/*
 * bspmake.c:  Build BSP tree for a list of walls.
 */
#include "common.h"
#pragma hdrstop

#include "bsp.h"
#include "wstructs.h"
#include "levels.h"
#include "gfx.h"

#define MAX_EDGE 256   // Biggest magnitude allowed for wall coefficient

#define SWAP(a, b, t) ((t) = (a), (a) = (b), (b) = (t));

static int num_walls = 0;
static int num_nodes = 0;

static void BSPFreeNode(BSPnode *node);
static void BSPFlipWall(WallData *wall);
static int  BSPSwapFlags(int flags, int flag1, int flag2);
static int  BSPSwapScroll(int direction);
/*****************************************************************************/
/*
 * GCD: GreatestCommonDenominator. Return GCD(a, b). Assumes that a is nonzero
        or b is nonzero. Now that we use doubles, this just reduces b to a
        small number, and scales a accordingly.
 */
double GCD(double a, double b)
{
   double temp;

   if (a < 1.0 && a > -1.0)
      return ABS(b);

   a = ABS(a);
   b = ABS(b);

   while (b > 1.0)
   {
      temp = b;
      b = a / b;
      a = temp;
   }
   return a;
}
/*
 * make a and b relatively prime.  Assumes that a is nonzero or b is nonzero.
 */
void reduce(double *a, double *b)
{
  double d = GCD(*a, *b);
  if (d <= 0.001 && d >= -0.001)
     dprintf("reduce got 0, 0!\n");
  else
  {
     *a /= d;
     *b /= d;
  }
}
/*
 * almost_equal: returns TRUE if the two doubles are essentially equal.
 */
Bool almost_equal(double a, double b)
{
   if (a - b <= 0.0001 && a - b >= -0.0001)
      return TRUE;
   return FALSE;
}
/*****************************************************************************/
/*
 * BSPDumpWall
 */
void BSPDumpWall(WallData *wall)
{
   dprintf("(%6.4f, %6.4f) to (%6.4f, %6.4f)\n", wall->x0, wall->y0, wall->x1, wall->y1);
}
/*****************************************************************************/
/*
 * BSPDumpWallList
 */
void BSPDumpWallList(WallDataList wall_list)
{
   WallData *wall;

   for (wall = wall_list; wall != NULL; wall = wall->next)
      BSPDumpWall(wall);
}
/*****************************************************************************/
/*
 * BSPDumpTree:  Print out BSP tree.  Pass in 0 for level to start.
 */
void BSPDumpTree(BSPnode *tree, int level)
{
   int i;
   char indent[100];
   BSPinternal *node;
   BSPleaf *leaf;
   WallData *wall;

   memset(indent, ' ', level * 3);
   indent[level * 3] = 0;

   if (tree == NULL)
   {
      dprintf("%s<empty>\n", indent);
      return;
   }

   switch (tree->type)
   {
   case BSPleaftype:
      leaf = &tree->u.leaf;
      dprintf("%sFloor type = %d, ceiling type = %d\n", indent, 
         leaf->floor_type, leaf->ceil_type);
      dprintf("%sPolygon has %d points: ", indent, leaf->poly.npts);
      for (i=0; i < leaf->poly.npts; i++)
      {
         dprintf("(%6.4f %6.4f) ", leaf->poly.p[i].x, leaf->poly.p[i].y);
      }
      dprintf("\n");
      return;

   case BSPinternaltype:
      node = &tree->u.internal;

      dprintf("%sBounding box = (%6.4f %6.4f) (%6.4f %6.4f)\n", indent, 
         tree->bbox.x0, tree->bbox.y0, 
         tree->bbox.x1, tree->bbox.y1);
      dprintf("%sPlane (a, b, c) = (%6.4f %6.4f %6.4f):\n", indent, node->separator.a,
         node->separator.b, node->separator.c);
      dprintf("%sWalls:\n", indent);
      for (wall = node->walls_in_plane; wall != NULL; wall = wall->next)
      {
         dprintf("%s   (%6.4f %6.4f)-(%6.4f %6.4f)", indent, 
            wall->x0, wall->y0, wall->x1, wall->y1);
         dprintf(" z = (%d %d %d %d), +x=%d, -x=%d\n", 
            wall->z0, wall->z1, wall->z2, wall->z3,
            wall->pos_xoffset, wall->neg_xoffset);
      }

      dprintf("%s+ side:\n", indent);
      BSPDumpTree(node->pos_side, level + 1);
      dprintf("%s- side:\n", indent);
      BSPDumpTree(node->neg_side, level + 1);
      return;

   default:
      LogError("Unknown node type!\n");
   }
}
/*****************************************************************************/
/*
 * BSPGetNewWall:  Return an empty WallData structure.
 *   Should eventually spit them out from an array.
 */
WallData *BSPGetNewWall(void)
{
   WallData *wall = (WallData *) GetMemory(sizeof(WallData));
   memset(wall, 0, sizeof(WallData));
   num_walls++;

   return wall;
}
/*****************************************************************************/
/*
 * BSPGetNewNode:  Return an empty BSPnode structure.
 *   Should eventually spit them out from an array.
 */
BSPnode *BSPGetNewNode(void)
{
   BSPnode *node = (BSPnode *) GetMemory(sizeof(BSPnode));
   memset(node, 0, sizeof(BSPnode));
   num_nodes++;

   return node;
}
/*****************************************************************************/
/*
 * BSPGetNumWalls:  Return # of walls in room.
 */
int BSPGetNumWalls(void)
{
   return num_walls;
}
/*****************************************************************************/
/*
 * BSPGetNumNodes:  Return # of nodes allocated for BSP tree.
 */
int BSPGetNumNodes(void)
{
   return num_nodes;
}
/*****************************************************************************/
/*
 * BSPFreeTree:  Free all memory associated with given tree and return NULL
 */
BSPTree BSPFreeTree(BSPTree tree)
{
   BSPFreeNode(tree);
   num_walls = 0;
   num_nodes = 0;
   return NULL;
}
/*****************************************************************************/
/*
 * BSPFreeNode:  Free all memory associated with given node and its children.
 */
void BSPFreeNode(BSPnode *node)
{
   BSPinternal *inode;
   WallData *wall, *next_wall;

   if (node == NULL)
      return;
   switch (node->type)
   {
   case BSPleaftype:
      FreeMemory(node);
      break;
   case BSPinternaltype:
      inode = &node->u.internal;
      if (inode->pos_side != NULL)
         BSPFreeNode(inode->pos_side);
      if (inode->neg_side != NULL)
         BSPFreeNode(inode->neg_side);
      wall = inode->walls_in_plane;
      while (wall != NULL)
      {
         next_wall = wall->next;
         FreeMemory(wall);
         wall = next_wall;
      }
      FreeMemory(node);
      break;
   default:
      LogError("BSPFreeNode got unknown node type!\n");
   }
}
/*****************************************************************************/
/*
 * BSPWallComputeLength:  Compute and return the length of the given wall.
 *   The units are 1 grid square = length 64.
 */
double BSPWallComputeLength(WallData *wall)
{
   double dx, dy;
   
   dx = (wall->x1 - wall->x0) / (double)BLAK_FACTOR;
   dy = (wall->y1 - wall->y0) / (double)BLAK_FACTOR;

   return ComputeDistDouble(dx, dy);
}
/*****************************************************************************/
/*
 * BSPGetLineEquationFromPoints:  Given two points wall, set the coefficients
 *   of the equation for the line containing the points.
 */
void BSPGetLineEquationFromPoints(Pnt p1, Pnt p2, double *a, double *b, double *c)
{
   /* <dx,dy> is a vector from p1 to p2 */
   double dx = p2.x - p1.x;
   double dy = p2.y - p1.y;

   /* reduce to smallest vector possible in same direction */
   reduce(&dx, &dy);

   /* rotate 90 degrees to form separator normal.                 */
   /* <dx,dy> rotated 90 degrees is <-dy,dx> (or is it <dy,-dx>?) */
   *a = -dy;
   *b = dx;
   
   /* calculate c by substituting in p1 (p2 would also work) */
   *c = -((*a) * p1.x + (*b) * p1.y);
}
/*****************************************************************************/
/*
 * BSPGetLineEquationFromWall:  Given a wall, set the coefficients
 *   of the equation for the line containing the wall.
 */
void BSPGetLineEquationFromWall(WallData *wall, double *a, double *b, double *c)
{
   Pnt p1, p2;

   p1.x = wall->x0;
   p1.y = wall->y0;
   p2.x = wall->x1;
   p2.y = wall->y1;
   BSPGetLineEquationFromPoints(p1, p2, a, b, c);
}
/*****************************************************************************/
/*
 * BSPFindLineSide:   Find out which side of given wall the point is.
 *   Return 1 if on + side, -1 if on - side, 0 if on line containing wall.
 */
int BSPFindLineSide(WallData *wall, double x, double y)
{
   double a, b, c;
   BSPGetLineEquationFromWall(wall, &a, &b, &c);
   return SGNDOUBLE(a * x + b * y + c);
}
/*****************************************************************************/
/*
 * BSPWallIntersection:  Find intersection of line containing wall1 with wall2.
 *   If there is an intersection, put its coordinates in (x, y) and return True.
 *   Otherwise, return False.
 */

/* everything is in a lattice, so wall1 and wall2 might not have an
 * intersection point!!!  This code returns an intersection point if
 * there is one.
 */

typedef enum {
   NoIntersection = 1,  /* wall1's line doesn't cross wall2   */
   Coincide,            /* wall2 is in wall1's line           */
   FirstEndpoint,       /* intersection at wall2's start      */
   SecondEndpoint,      /* intersection at wall2's end        */
   Middle,              /* intersection in middle of wall2,   */
}  IntersectionType;

/* Returns intersection type of wall1 with wall2.  Returns intersection point
 * if FirstEndpoint, SecondEndpoint, or Middle is returned. Returned point
 * should be on wall1.
 */
IntersectionType BSPWallIntersection(WallData *wall1, WallData *wall2, double *x, double *y)
{
   double a, b, c;
   double x0, y0, x1, y1;
   double side0,side1;
   double num,denom;
   double dx,dy;

   /* first, get line equation from first wall */
   BSPGetLineEquationFromWall(wall1, &a, &b, &c);

   /* work with second wall */
   x0 = wall2->x0;
   y0 = wall2->y0;
   x1 = wall2->x1;
   y1 = wall2->y1;

   side0 = a*x0 + b*y0 + c;
   side1 = a*x1 + b*y1 + c;

   if (almost_equal(side0,0.0) && almost_equal(side1,0.0))
      return Coincide;
   if (almost_equal(side0, 0.0))
   {
      *x = x0; *y = y0;
      return FirstEndpoint;
   }
   if (almost_equal(side1, 0.0))
   {
      *x = x1; *y = y1;
      return SecondEndpoint;
   }

   if ((side0 > 0.0 && side1 > 0.0) || (side0 < 0.0 && side1 < 0.0))
      return NoIntersection;

   if (side0 > 0.0)
   {
      num = side0;
      denom = side0 - side1;
   }
   else
   {
      num = -side0;
      denom = side1 - side0;
   }

   dx = x1 - x0;
   dy = y1 - y0;

   *x = (x0 + num * (dx / denom));
   *y = (y0 + num * (dy / denom));

   return Middle;
}
/*****************************************************************************/
/*
 * BSPChooseRoot:  Return a good root wall from the given list.
 *   We choose the root that minimizes max(walls on pos side, walls on neg side).
 *   Ties are won by the root with the fewest splits.
 *   Requires that walls isn't NULL.
 */
WallData *BSPChooseRoot(WallDataList walls)
{
   WallData *best_root, *wall, *root;
   int best_count = -1;      // Minimum # of walls in bigger side of best wall
   int best_splits = 999999; // # of splits for best root so far

   for (root = walls; root != NULL; root = root->next)
   {
      int pos, neg, splits;
      double a, b, c;
      int max_count;

      pos = 0;
      neg = 0;
      splits = 0;
      BSPGetLineEquationFromWall(root, &a, &b, &c);

      // Check position of each wall with this root
      for (wall = walls; wall != NULL; wall = wall->next)
      {
         double side0, side1;

         // Find out what side each endpoint is on
         side0 = SGNDOUBLE(a * wall->x0 + b * wall->y0 + c);
         side1 = SGNDOUBLE(a * wall->x1 + b * wall->y1 + c);

         // If both on same side, or one is on line, no split needed
         if (side0 * side1 >= 0)
         {
            // In plane of root?
            if (almost_equal(side0, 0.0) && almost_equal(side1, 0.0))
               continue;

            // On + side of root?
            if (side0 > 0.0 || side1 > 0.0)
            {
               pos++;
               continue;
            }

            // On - side of root
            neg++;
            continue;
         }

         // Split--one on each side
         pos++;
         neg++;
         splits++;
      }

      max_count = max(pos, neg);
      if (max_count < best_count || (max_count == best_count && splits < best_splits) ||
         best_count == -1)
      {
         best_count = max_count;
         best_root = root;
         best_splits = splits;
      }
   }
   return best_root;
}
/*****************************************************************************/
/*
 * BSPUpdateSplit:  A wall has just been split into two subwalls.
 *   Update the lengths and texture offsets of the two walls.
 *   Walking the unsplit wall from start to end must be equivalent to walking
 *    start of wall1 -> end of wall1 = start of wall 2 -> end of wall 2
 */
void BSPUpdateSplit(WallData *wall1, WallData *wall2)
{
   wall1->length = BSPWallComputeLength(wall1);
   wall2->length = BSPWallComputeLength(wall2);

   /* Move texture offsets to account for split */
   if (wall1->flags & BF_POS_BACKWARDS)
      wall1->pos_xoffset += (int)round(wall2->length);
   else wall2->pos_xoffset += (int)round(wall1->length);

#if 0
   if (wall1->flags & BF_NEG_BACKWARDS)
      wall1->neg_xoffset += (int)round(wall2->length);
   else wall2->neg_xoffset += (int)round(wall1->length);
#endif

   // Do this backwards, because client exchanges vertices of negative walls

   if (wall1->flags & BF_NEG_BACKWARDS)
      wall2->neg_xoffset += (int)round(wall1->length);
   else wall1->neg_xoffset += (int)round(wall2->length);
}
/*****************************************************************************/
/*
 * BSPFlipWall:  Flip orientation of given wall.
 */
void BSPFlipWall(WallData *wall)
{
   int temp, posoffset, negoffset, temp1, temp2;
   double doubletemp;
#if 0
   TextureInfo *info;
#endif

   SWAP(wall->pos_type, wall->neg_type, temp);
   SWAP(wall->pos_sector, wall->neg_sector, temp);
   SWAP(wall->x0, wall->x1, doubletemp);
   SWAP(wall->y0, wall->y1, doubletemp);

   // Swap x offsets; need to recompute
   // XXX Still not sure this is right
   posoffset = wall->pos_xoffset;
   negoffset = wall->neg_xoffset;

#if 0
   info = GetSidedefTextureInfo(wall->pos_sidedef);
   if (info != NULL)
      posoffset = wall->neg_xoffset + wall->length - info->Width / info->shrink;

   info = GetSidedefTextureInfo(wall->neg_sidedef);
   if (info != NULL)
      negoffset = wall->pos_xoffset + wall->length - info->Width / info->shrink;
#endif

//   dprintf("+ sector = %d, - sector = %d, + SD = %d, - SD = %d\n", 
//      wall->neg_sector, wall->pos_sector, wall->pos_sidedef, wall->neg_sidedef);
//   dprintf("old +x = %d, old -x = %d, new +x = %d, new -x = %d\n", 
//      wall->pos_xoffset, wall->neg_xoffset, negoffset, posoffset);
   wall->pos_xoffset = negoffset;
   wall->neg_xoffset = posoffset;

   SWAP(wall->pos_sidedef, wall->neg_sidedef, temp);

   wall->flags = BSPSwapFlags(wall->flags, BF_POS_BACKWARDS, BF_NEG_BACKWARDS);
   wall->flags = BSPSwapFlags(wall->flags, BF_POS_TRANSPARENT, BF_NEG_TRANSPARENT);
   wall->flags = BSPSwapFlags(wall->flags, BF_POS_PASSABLE, BF_NEG_PASSABLE);
   wall->flags = BSPSwapFlags(wall->flags, BF_POS_NOLOOKTHROUGH, BF_NEG_NOLOOKTHROUGH);
   wall->flags = BSPSwapFlags(wall->flags, BF_POS_ABOVE_BUP, BF_NEG_ABOVE_BUP);
   wall->flags = BSPSwapFlags(wall->flags, BF_POS_BELOW_TDOWN, BF_NEG_BELOW_TDOWN);
   wall->flags = BSPSwapFlags(wall->flags, BF_POS_NORMAL_TDOWN, BF_NEG_NORMAL_TDOWN);
   wall->flags = BSPSwapFlags(wall->flags, BF_POS_NO_VTILE, BF_NEG_NO_VTILE);

   // Switch around scrolling animations
   temp1 = WallScrollPosSpeed(wall->flags);
   temp2 = WallScrollNegSpeed(wall->flags);
   wall->flags = (wall->flags & ~0x00300000) | (temp2 << 20);
   wall->flags = (wall->flags & ~0x06000000) | (temp1 << 25);

   temp1 = BSPSwapScroll(WallScrollPosDirection(wall->flags));
   temp2 = BSPSwapScroll(WallScrollNegDirection(wall->flags));
   wall->flags = (wall->flags & ~0x01C00000) | (temp2 << 22);
   wall->flags = (wall->flags & ~0x38000000) | (temp1 << 27);
}
/*****************************************************************************/
/*
 * BSPSwapFlags:  Swap two given flags in given flags field, and return result.
 */
int BSPSwapFlags(int flags, int flag1, int flag2)
{
   int temp;

   temp = flags & flag1;
   if (flags & flag2)
      flags = flags | flag1;
   else
      flags = flags & ~flag1;
   if (temp)
      flags = flags | flag2;
   else
      flags = flags & ~flag2;

   return flags;
}
/*****************************************************************************/
/*
 * BSPSwapScroll:  Return new scrolling direction when linedef is 
 *   horizontally swapped.
 */
int BSPSwapScroll(int direction)
{
   switch (direction)
   {
   case SCROLL_NE:  direction = SCROLL_NW;  break;
   case SCROLL_E:   direction = SCROLL_W;   break;
   case SCROLL_SE:  direction = SCROLL_SW;  break;
   case SCROLL_SW:  direction = SCROLL_SE;  break;
   case SCROLL_W:   direction = SCROLL_E;   break;
   case SCROLL_NW:  direction = SCROLL_NE;  break;
   }

   return direction;
}
/*****************************************************************************/
/*
 * BSPSplitWalls:  Split all walls that cross the given root.  
 *   Walls in the plane of the root are placed in root_walls.
 *   Walls on the positive side of the root are placed in pos_walls.
 *   Walls on the negative side of the root are placed in neg_walls.
 */
void BSPSplitWalls(WallDataList walls, WallData *root, WallDataList *root_walls,
            WallDataList *pos_walls, WallDataList *neg_walls)
{
   double x, y;
   double side0, side1;
   double a, b, c;     // Coeffs in Ax + By + C = 0 for root
   double a2, b2, c2;
   WallData *wall, *cur_wall, *neg_wall;
   
   *root_walls = *pos_walls = *neg_walls = NULL;

   for (cur_wall = walls; cur_wall != NULL; )
   {
      // We'll modify cur_wall below, so move on to next wall in list first.
      wall = cur_wall;
      cur_wall = cur_wall->next;

      // Check for root specially
      if (wall == root)
      {
         wall->next = *root_walls;
         *root_walls = wall;
         continue;
      }

      BSPGetLineEquationFromWall(root, &a, &b, &c);

      // Find out what side each endpoint is on
      side0 = SGNDOUBLE(a * wall->x0 + b * wall->y0 + c);
      side1 = SGNDOUBLE(a * wall->x1 + b * wall->y1 + c);

      // If both on same side, or one is on line, no split needed
      if (side0 * side1 >= 0.0)
      {
         // In plane of root?
         if (almost_equal(side0, 0.0) && almost_equal(side1, 0.0))
         {
            /* we may need to reverse the wall */
            BSPGetLineEquationFromWall(wall, &a2, &b2, &c2);
            if (almost_equal(a, a2) && almost_equal(b,b2))
               ;          /* we're OK */
            else if (almost_equal(a,-a2) || almost_equal(b,-b2))
               BSPFlipWall(wall);
            else
            {
               //LogError("something is wacky in BSPSplitWalls! "
               //"(a = %6.4f, a2 = %6.4f, b = %6.4f, b2= %6.4f\n", ra1, ra2, rb1, rb2);
               LogError("something is wacky in BSPSplitWalls! "
                  "(a = %6.4f, a2 = %6.4f, b = %6.4f, b2= %6.4f\n", a, a2, b, b2);
            }

            wall->next = *root_walls;
            *root_walls = wall;
            continue;
         }

         // On + side of root?
         if (side0 > 0.0 || side1 > 0.0)
         {
            wall->next = *pos_walls;
            *pos_walls = wall;
            continue;
         }

         // On - side of root 
         wall->next = *neg_walls;
         *neg_walls = wall;
         continue;
      }

      // Find intersection point; put in (x, y).  Need to be careful of overflows
      switch(BSPWallIntersection(root, wall, &x, &y))
      {
      case NoIntersection:
      case Coincide:
      case FirstEndpoint:
      case SecondEndpoint:
         /* these should have been handled already... */
         LogError("something wrong in BSPSplitWalls...\n");
         continue;
      case Middle:
         break;
      }

      // Make a new wall for negative side; use old wall for positive side
      neg_wall = BSPGetNewWall();
      memcpy(neg_wall, wall, sizeof(WallData));

      if (side0 > 0.0)   // (x0, y0) on positive side
      {
         wall->x1 = x;
         wall->y1 = y;

         neg_wall->x0 = x;
         neg_wall->y0 = y;
         BSPUpdateSplit(wall, neg_wall);
      }
      else
      {
         wall->x0 = x;
         wall->y0 = y;

         neg_wall->x1 = x;
         neg_wall->y1 = y;
         BSPUpdateSplit(neg_wall, wall);
      }

      if (wall->x0 == wall->x1 && wall->y0 == wall->y1)
      {
         LogError(">>> Wall %d has same start/end endpoints (Linedef %d): ignoring it.\n",(int)wall->num,(int)wall->linedef_num);
         num_walls--;
      }
      else
      {
         wall->next = *pos_walls;
         *pos_walls = wall;
      }

      if (neg_wall->x0 == neg_wall->x1 && neg_wall->y0 == neg_wall->y1)
      {
         LogError(">>> Negative wall %d has same start/end endpoints (Linedef %d): ignoring it.\n",(int)neg_wall->num,(int)neg_wall->linedef_num);
         num_walls--;
      }
      else
      {
         neg_wall->next = *neg_walls;
         *neg_walls = neg_wall;
      }
   }
}
/*****************************************************************************/
/*
 * BSPFindPolyBox:  Put the bounding box of p in b.
 */
void BSPFindPolyBox(Poly *p, Box *b)
{
   int i;
   double minx, miny, maxx, maxy;

   if (p->npts == 0)
      LogError("BSPFindPolyBox got a polygon with 0 points!\n");

   minx = maxx = p->p[0].x;
   miny = maxy = p->p[0].y;

   for (i=1; i < p->npts; i++)
   {
      minx = min(minx, p->p[i].x);
      miny = min(miny, p->p[i].y);
      maxx = max(maxx, p->p[i].x);
      maxy = max(maxy, p->p[i].y);
   }
   b->x0 = minx;
   b->y0 = miny;
   b->x1 = maxx;
   b->y1 = maxy;
}
/*****************************************************************************/
/*
 * BSPMergeBoxes:  Put the union of bounding boxes b1 and b2 in b.
 */
void BSPMergeBoxes(Box *b, Box *b1, Box *b2)
{
   b->x0 = min(b1->x0, b2->x0);
   b->y0 = min(b1->y0, b2->y0);
   b->x1 = max(b1->x1, b2->x1);
   b->y1 = max(b1->y1, b2->y1);
}
/*****************************************************************************/
/*
 * BSPFindBoundingBoxes:  Fill in bounding box fields in given BSP tree.
 */
void BSPFindBoundingBoxes(BSPnode *tree)
{
   BSPinternal *node;

   // The idea is to compute the bounding boxes for leaves, and then union them
   // for internal nodes.

   if (tree == NULL)
      return;
   
   switch (tree->type)
   {
   case BSPleaftype:
      BSPFindPolyBox(&tree->u.leaf.poly, &tree->bbox);
      return;

   case BSPinternaltype:
      node = &tree->u.internal;
      BSPFindBoundingBoxes(node->pos_side);
      BSPFindBoundingBoxes(node->neg_side);
      if (node->pos_side == NULL && node->neg_side == NULL)
      {
         LogError("BSPFindBoundingBoxes found internal node with no children!\n");
         return;
      }
      if (node->neg_side == NULL)
         memcpy(&tree->bbox, &node->pos_side->bbox, sizeof(Box));
      else if (node->pos_side == NULL)
         memcpy(&tree->bbox, &node->neg_side->bbox, sizeof(Box));
      else
         BSPMergeBoxes(&tree->bbox, &node->pos_side->bbox, &node->neg_side->bbox);
      return;

   default:
      LogError("Illegal node type in BSPFindBoundingBoxes!\n");
      return;
   }
}
/*****************************************************************************/
/*
 * BSPSplitPoly:  Split given polygon with given wall.  Fill in pos_poly and
 *   neg_poly with polygons on + and - side.
 *   Modifies poly (essentially destroys it)
 *   Returns True iff split goes OK.
 */
Bool BSPSplitPoly(Poly *poly, WallData *wall, Poly *pos_poly, Poly *neg_poly)
{
   double a, b, c;
   int i,j;
   Poly tmp;
   WallData temp_wall;
   double x, y, side;

   int intr[2];
   int intrcount;

   intrcount = 0;

   BSPGetLineEquationFromWall(wall, &a, &b, &c);

   /* copy first point */
   j = 0;
   tmp.p[j] = poly->p[0];
   j++;

   for(i=0; i<poly->npts; i++)
   {
      temp_wall.x0 = poly->p[i].x;
      temp_wall.y0 = poly->p[i].y;
      temp_wall.x1 = poly->p[i+1].x;
      temp_wall.y1 = poly->p[i+1].y;

      switch(BSPWallIntersection(wall, &temp_wall, &x, &y))
      {
      case NoIntersection:
         tmp.p[j] = poly->p[i+1];
         j++;
         break;
      case Coincide:
         goto oneside;
      case FirstEndpoint:
         if (intrcount < 2)
         {
            intr[intrcount] = j-1;
            intrcount++;
         }
         else
         {
            LogError("more than 2 intersections! (1)\n");
            pos_poly->npts = neg_poly->npts = 0;
            return False;
         }
         tmp.p[j] = poly->p[i+1];
         j++;
         break;
      case SecondEndpoint:
         tmp.p[j] = poly->p[i+1];
         j++;
         break;
      case Middle:
         tmp.p[j].x = x;
         tmp.p[j].y = y;

         if (intrcount < 2)
         {
            intr[intrcount] = j;
            intrcount++;
         }
         else
         {
            LogError("more than 2 intersections! (2)\n");
            pos_poly->npts = neg_poly->npts = 0;
            return False;
         }
         j++;
         tmp.p[j] = poly->p[i+1];
         j++;
         break;
      }
   }
   tmp.npts = j-1;  /* j points added, but last is == to first */

#if 0
   dprintf("expanded: %d ", tmp.npts);
   for (i=0; i<=tmp.npts; i++)
      dprintf("(%d %d) ", tmp.p[i].x, tmp.p[i].y);
   dprintf("\n");
#endif

   if (intrcount < 2)
   {
      LogError("less than 2 intersections!\n");
      pos_poly->npts = neg_poly->npts = 0;
      return False;
   }

   i = 0;
   while(1)
   {
      int k = (i + intr[0]) % tmp.npts;
      pos_poly->p[i] = tmp.p[k];
      i++;
      if (k == intr[1]) break;
   }
   pos_poly->p[i] = pos_poly->p[0];
   pos_poly->npts = i;

   i = 0;
   while(1)
   {
      int k = (i + intr[1]) % tmp.npts;
      neg_poly->p[i] = tmp.p[k];
      i++;
      if (k == intr[0]) break;
   }
   neg_poly->p[i] = neg_poly->p[0];
   neg_poly->npts = i;

   /* check to see if we need to switch polys */
   side = 0.0;
   for(i=0; i<pos_poly->npts; i++)
   {
      side += a * pos_poly->p[i].x + b * pos_poly->p[i].y + c;
   }
   if (side < 0.0)
   {
      memcpy(&tmp, pos_poly, sizeof(Poly));
      memcpy(pos_poly, neg_poly, sizeof(Poly));
      memcpy(neg_poly, &tmp, sizeof(Poly));
   }

   return True;

   // GOTO from the earlier Switch, case Coincide.
   oneside:        /* polygon all on one side of wall */
   for(i=0; i<poly->npts; i++)
   {
      side = a * poly->p[i].x + b * poly->p[i].y + c;
      if (side > 0.0)
      {
         memcpy(pos_poly, poly, sizeof(Poly));
         neg_poly->npts = 0;
         return True;
      }
      else if (side < 0.0)
      {
         memcpy(neg_poly, poly, sizeof(Poly));
         pos_poly->npts = 0;
         return True;
      }
   }
   LogError("all polygon points in plane!\n");
   pos_poly->npts = neg_poly->npts = 0;
   return False;
}
/*****************************************************************************/
/*
 * BSPMakeLeafNode:  We've bottomed out at a node with no walls; put together
 *   a leaf node with the floor/ceiling polygon.
 */
void BSPMakeLeafNode(BSPnode *node, Poly *poly, int sector)
{
   BSPleaf *leaf;
   SPtr cur_sector;

   node->type = BSPleaftype;
   leaf = &node->u.leaf;
   memcpy(&leaf->poly, poly, sizeof(Poly));

   if (sector < 0)
   {
      // dprintf("Missing sector in BSPMakeLeafNode!\n");
      return;
   }

   cur_sector = &Sectors[sector];
   leaf->sector = sector;
  
   leaf->floor_type = cur_sector->floor_type;
   leaf->ceil_type  = cur_sector->ceiling_type;

   leaf->floor_height   = cur_sector->floorh;
   leaf->ceiling_height = cur_sector->ceilh;

   leaf->light = (BYTE) cur_sector->light;

   leaf->tx = cur_sector->xoffset * BLAK_FACTOR;
   leaf->ty = cur_sector->yoffset * BLAK_FACTOR;
}
/*****************************************************************************/
/*
 * BSPBuildNode:  Return BSP tree for given list of walls.
 *   poly gives polygon of region containing walls; it's used in leaves.
 *   sector gives sector # we've descended to; used in leaves.
 */
BSPnode *BSPBuildNode(WallDataList walls, Poly *poly, int sector)
{
   WallData *root; 
   WallDataList pos_walls, neg_walls, plane_walls;
   BSPnode *node;
   Poly pos_poly, neg_poly;
   double a, b, c;
#if 0
   int i;
#endif

   // If empty polygon, there is nothing on this side of wall
   // Also skip if no sector (= part of start boundingbox, but not of any sector)
   if (poly->npts == 0 || (walls == NULL && sector == -1))
      return NULL;

	node = BSPGetNewNode();

   if (walls == NULL)
   {
      // No walls left ==> leaf node
      BSPMakeLeafNode(node, poly, sector);
      return node;
   }

   root = BSPChooseRoot(walls);

//   dprintf("Walls = \n");
//   BSPDumpWallList(walls);
//   dprintf("Root = ");
//   BSPDumpWall(root);

   // Split walls that cross root
   BSPSplitWalls(walls, root, &plane_walls, &pos_walls, &neg_walls);

   // Build up internal BSP node
   node->type = BSPinternaltype;

//   dprintf(" + Walls = \n");
//   BSPDumpWallList(pos_walls);
//   dprintf(" - Walls = \n");
//   BSPDumpWallList(neg_walls);

   // Save plane containing root
   BSPGetLineEquationFromWall(root, &a, &b, &c);
   node->u.internal.separator.a = a;
   node->u.internal.separator.b = b;
   node->u.internal.separator.c = c;

   node->u.internal.walls_in_plane = plane_walls;

#if 0
   dprintf("splitting: %d ", poly->npts);
   for (i=0; i<=poly->npts; i++)
      dprintf("(%d %d) ", poly->p[i].x, poly->p[i].y);
   dprintf("\n with ");
   BSPDumpWall(root);
#endif

   // Split polygon with root
   if (!BSPSplitPoly(poly, root, &pos_poly, &neg_poly))
      LogError("problem splitting sector %d with linedef %d\n",
         sector, (int) root->linedef_num);

#if 0
   dprintf("positive: %d ", pos_poly.npts);
   for (i=0; i<=pos_poly.npts; i++)
      dprintf("(%d %d) ", pos_poly.p[i].x, pos_poly.p[i].y);
   dprintf("negative: %d ", neg_poly.npts);
   for (i=0; i<=neg_poly.npts; i++)
      dprintf("(%d %d) ", neg_poly.p[i].x, neg_poly.p[i].y);
   dprintf("\n\n");
#endif

   // Descend to children
   node->u.internal.pos_side = BSPBuildNode(pos_walls, &pos_poly, root->pos_sector);
   node->u.internal.neg_side = BSPBuildNode(neg_walls, &neg_poly, root->neg_sector);

   return node;
}
/*****************************************************************************/
/*
 * BSPBuildTree:  Return a BSP tree for the given room.  The room is bounded
 * by the box (min_x, min_y) (max_x, max_y).
 */
BSPnode *BSPBuildTree(WallData *wall_list, int min_x, int min_y, int max_x, int max_y)
{
   BSPnode *tree;
   Poly poly;

   dprintf("BSPBuildTree got box (%d %d)-(%d %d)\n", min_x, min_y, max_x, max_y);

   // Set up initial polygon as rectangle enclosing room
   poly.npts = 4;
   poly.p[0].x = min_x;
   poly.p[0].y = min_y;
   poly.p[1].x = max_x;
   poly.p[1].y = min_y;
   poly.p[2].x = max_x;
   poly.p[2].y = max_y;
   poly.p[3].x = min_x;
   poly.p[3].y = max_y;
   poly.p[4] = poly.p[0];

   tree = BSPBuildNode(wall_list, &poly, -1);

   BSPFindBoundingBoxes(tree);

//   BSPDumpTree(tree, 0);
   return tree;
}
