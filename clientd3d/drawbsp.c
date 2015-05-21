// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * drawbsp.c:  The main 3D drawing routine.
 *
 * To do:
 *
 * Fix wall drawing to make it less jittery up close
 *
 * We sometimes need to know when an object is done being drawn (for example, to draw something
 * over the object).  To do this, we have a field "ncones" in the object that counts the number
 * of cones in which the object is drawn.  However, since an object might be split among different
 * leaf nodes and thus drawn at different times, we keep a pointer "ncones_ptr" to the "ncones"
 * field for one particular cone for the object.  Whenever we draw a cone of the object, we
 * go through the "ncones" pointer to decrement the overall count for the object.  When this
 * count reaches 0, the object has been completely drawn.
 */

#include "client.h"

#define MAX_OBJS_PER_LEAF 100

// Decimal places for computing fraction along wall
#define LOG_FRACTION 12
#define FRACTION  (1 << LOG_FRACTION)

#define MINCLIPROW (-16*MAXY)
#define MAXCLIPROW (17*MAXY)

// inline utility macros
#define SWAP(a,b,tmp) (tmp)=(a);(a)=(b);(b)=(tmp)
#define LERP(a,b,t)   ((a) + ((((b)-(a)) * (t)) >> LOG_FINENESS))

static ID   viewer_id;          /* Id of player's object */
static long viewer_angle;       /* Angle of viewing in pseudo degrees */
static long viewer_x, viewer_y; /* Position of player in fine coordinates */

long viewer_height;      /* Viewer's height above floor, in fine coordinates */

extern PDIB background;         /* Pointer to background bitmap */

static long screen_width;       /* width of screen */
static long screen_width2;      /* half width of screen */

static list_type background_overlays;

extern BYTE *gBits;
extern AREA area;
extern long horizon;
extern int sector_depths[];

// Directional lighting parameters
extern Vector3D sun_vect;
extern long shade_amount;

extern BOOL bg_overlayVisible;

/* vector pointing in direction of player's view */
static long center_a,center_b;
/* frustum edge parameters */
static long left_a,left_b,right_a,right_b;

/* return distance along viewer sight line */
#define GetDistance(x,y) ((center_a * (x) + center_b * (y)) / FINENESS)

static int world_width = 3328;

/* Hold info on objects to be drawn */
static ObjectData objectdata[MAXOBJECTS];
static long nobjects;

/* Hold info on items (walls, ceilings, floors, objects) to be drawn */
#define MAX_ITEMS 2400L
DrawItem drawdata[MAX_ITEMS];
long nitems;

static float fpytex;
static float fpyinc;
static DWORD tex_offset;

/* flag that controls whether background is done incrementally or all at once */
static Bool incremental_background = False;
/* Count the number of background cones.  If incremental_background is
 * False, background cones are counted in add_up and add_dn.  If
 * incremental_background is True, they are counted in doDrawBackground.
 */
static int background_cones = MAX_ITEMS;

static void doDrawWall(DrawWallStruct *wall, ViewCone *c);
static void doDrawBackground(ViewCone *c);
static void SetMappingValues(SlopeData *slope );

void MinimapUpdate(Draw3DParams *params, BSPnode *tree);

/*****************************************************************************/
/*  Additions to BSP tree made on a per-frame basis.  For now,
 *  this includes only objects and projectiles.
/*****************************************************************************/

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
	 debug(("BSPFindLeafByPoint got illegal node type %d\n", tree->type));
	 return NULL;
      }
   }
}
/*****************************************************************************/
/*
 * AddObject:  Add object to the object list of some leaf in tree, and set the parent pointer of
 *   object to its containing leaf.
 * Return True iff object successfully added.   
 */
static Bool AddObject(BSPnode *tree, ObjectData *object)
{
   float side0, side1;
   BSPnode *pos, *neg;
   Bool res;
   
   while (1)
   {
      if (tree == NULL)
      {
	 debug(("add_object got NULL tree for object %d!\n", object->draw.id));
	 object->parent = NULL;
	 return False;
      }

      switch(tree->type)
      {
      case BSPleaftype:
	 object->next = tree->u.leaf.objects;
	 tree->u.leaf.objects = object;
	 object->parent = &tree->u.leaf;
	 object->draw.light = tree->u.leaf.sector->light;
	 return True;

      case BSPinternaltype:
	 side0 = tree->u.internal.separator.a * object->x0 + 
	   tree->u.internal.separator.b * object->y0 +
	   tree->u.internal.separator.c;
	 side1 = tree->u.internal.separator.a * object->x1 + 
	   tree->u.internal.separator.b * object->y1 +
	   tree->u.internal.separator.c;
	 
	 pos = tree->u.internal.pos_side;
	 neg = tree->u.internal.neg_side;
	 
    if ((side0 < 0.0001f && side0 > -0.0001f) && (side1 < 0.0001f && side1 > -0.0001f))
	    tree = (pos != NULL) ? pos : neg;
	 else if (side0 >= 0.0001f && side1 >= 0.0001f)
	   tree = pos;
	 else if (side0 <= -0.0001f && side1 <= -0.0001f)
	   tree = neg;
	 else
	 {   /* object segment crosses separator! */
	    float f = side0 / (side0 - side1);
	    long xmid = FloatToInt(object->x0 + f * (object->x1 - object->x0));
	    long ymid = FloatToInt(object->y0 + f * (object->y1 - object->y0));
	    ObjectData *copy;
	    
	    if (nobjects >= MAXOBJECTS)
	    {
	       debug(("out of object memory\n"));
	       return False;
	    }
	    copy = &objectdata[nobjects++];
	    
	    *copy = *object;      // Note: this also copies ncones_ptr in object
	    object->x1 = xmid;
	    object->y1 = ymid;
	    copy->x0 = xmid;
	    copy->y0 = ymid;
	    
	    res = False;
	    if (pos)
	       res |= AddObject(pos, side0 > 0 ? object:copy);
	    if (neg)
	       res |= AddObject(neg, side0 > 0 ? copy:object);
	    return res;  /* return true if object added on either side */
	 }
	 break;
	 
      default:
	 debug(("add_object error!\n"));
	 return False;
      }
   }
}

/***************************************************************************
 *
 * GetFloorHeight
 *    returns height of floor in sector at point x,y
 *    accounts for the fact that floors may be sloped
 *    (may want to inline)
 */

inline long GetFloorHeight(long x, long y, Sector *sector) {

    if (sector->sloped_floor == (SlopeData *)NULL)
	return sector->floor_height;
    else
	return (-sector->sloped_floor->plane.a*x - sector->sloped_floor->plane.b*y - sector->sloped_floor->plane.d)/sector->sloped_floor->plane.c;
}

/***************************************************************************
 *
 * GetCeilingHeight
 *    returns height of ceiling in sector at point x,y
 *    accounts for the fact that floors may be sloped
 *    (may want to inline)
 */

inline long GetCeilingHeight(long x, long y, Sector *sector) {

    if (sector->sloped_ceiling == (SlopeData *)NULL)
	return sector->ceiling_height;
    else
	return (-sector->sloped_ceiling->plane.a*x - sector->sloped_ceiling->plane.b*y - sector->sloped_ceiling->plane.d)/sector->sloped_ceiling->plane.c;
}

/*****************************************************************************/
/*
 * GetRoomHeight: sets floor and ceiling to the height of the floor and
 * the ceiling at room coordinates (x,y).  Returns True if everything went
 * ok, and False otherwise.
 */
Bool GetRoomHeight(BSPnode *tree, long *ceiling, long *floor, int *flags, long x, long y)
{
   long side;
   BSPnode *pos, *neg;
   
   while (1)
   {
      if (tree == NULL)
      {
	 /* debug(("GetRoomHeight got NULL tree\n")); */
	 return False;
      }
      
      switch(tree->type)
      {
      case BSPleaftype:
	 *floor = GetFloorHeight(x, y, tree->u.leaf.sector);
	 *ceiling = GetCeilingHeight(x, y, tree->u.leaf.sector);
	 *flags   = tree->u.leaf.sector->flags;
	 return True;
	 
      case BSPinternaltype:
	 side = tree->u.internal.separator.a * x + 
	    tree->u.internal.separator.b * y +
	       tree->u.internal.separator.c;
	 
	 pos = tree->u.internal.pos_side;
	 neg = tree->u.internal.neg_side;
	 if (side > 0)
	    tree = pos;
	 else
	    tree = neg;
	 break;
	 
      default:
	 debug(("GetRoomHeight error!\n"));
	 return False;
      }
   }
}
/*****************************************************************************/
/*
 * GetRoomHeightRad: sets floor and ceiling to the maximum height of the
 * floor and the minimum height of the ceiling within a radius r of
 * room coordinates (x,y).  Returns True if everything went ok, and False
 * otherwise.  This routine is conservative in that sometimes points
 * outside the circle of radius r of (x,y) will be included in the min/max.
 */
Bool GetRoomHeightRad(BSPnode *tree, long *ceiling, long *floor, int *flags, int x, int y, long r)
{
   long side;
   BSPnode *pos, *neg;
   
   while (1)
   {
      if (tree == NULL)
      {
	 /* debug(("GetRoomHeightRad got NULL tree\n")); */
	 return False;
      }
      
      switch(tree->type)
      {
      case BSPleaftype:
	 *floor   = tree->u.leaf.sector->floor_height;
	 *ceiling = tree->u.leaf.sector->ceiling_height;
	 *flags   = tree->u.leaf.sector->flags;
	 return True;
	 
      case BSPinternaltype:
	 side = tree->u.internal.separator.a * x + 
	    tree->u.internal.separator.b * y +
	       tree->u.internal.separator.c;
	 
	 side >>= LOG_FINENESS;
	 
	 pos = tree->u.internal.pos_side;
	 neg = tree->u.internal.neg_side;

	 if (side > r)
	    tree = pos;
	 else if (side < -r)
	    tree = neg;
	 else
	 {
     int cel, flr, flgs;
     if (GetRoomHeightRad(pos, ceiling, floor, flags, x, y, r) == True)
     {
        if (GetRoomHeightRad(neg, (long *) &cel, (long *) &flr, &flgs, x, y, r) == True)
       {
         if (cel < *ceiling)
           *ceiling = cel;
         if (flr > *floor)
         {
           *floor = flr;
           // Take "depth" flags from highest floor
           *flags = flgs;
         }
         return True;
       }
       return True;
     }
     else
       return GetRoomHeightRad(neg, ceiling, floor, flags, x, y, r);
	 }
	 break;
	 
      default:
	 debug(("GetRoomHeightRad error!\n"));
	 return False;
      }
   }
}
/*****************************************************************************/
/*
 * AddObjects:  Add objects to BSP tree.
 */
static void AddObjects(room_type *room)
{
   list_type l, temp_list;
   ObjectData *d;
   room_contents_node *r;
   int count, i;
   int width, height, sector_flags;
   long dx,dy;
   long angle;
   long dist;
   long a,b;
   long xdif,ydif; // used to generate extra endpoints for height test
   long top,bottom;
   
   nobjects = 0;

   // Clear objects from previous frame
   for (i=0; i < room->num_nodes; i++)
   {
      if (room->nodes[i].type == BSPleaftype)
	 room->nodes[i].u.leaf.objects = NULL;
   }

   // Add normal objects
   for(l = room->contents; l != NULL; l=l->next)
   {
      r = (room_contents_node*) (l->data);

      r->visible = False;
      
      /* don't add ourselves to tree */
      if (r->obj.id == viewer_id)
	 continue;
      
      /* compute and check distance */
      dx = r->motion.x - viewer_x;
      dy = r->motion.y - viewer_y;
      dist = GetDistance(dx,dy);
      if (dist <= 0 || (dist < MIN_DISTANCE && r->obj.moveontype == MOVEON_YES))
	continue;
      
      /* compute angle that object is viewed at */
      angle = (r->angle - intATan2(-dy,-dx)) & NUMDEGREES_MASK;

      /* compute width of bitmap from this angle */
      if (!GetObjectSize(r->obj.icon_res, r->obj.animate->group, angle, *(r->obj.overlays), 
			 &width, &height))
	continue;

      if (nobjects >= MAXOBJECTS)
      {
	 debug(("out of object memory\n"));
	 return;
      }

      d = &objectdata[nobjects++];
      
      // debug(("%d %d %d %d\n", r->obj.icon_res, angle, r->obj.animate.group, width));
      xdif  = ((width * center_b) >> (FIX_DECIMAL-6+1));
      ydif  = ((width * center_a) >> (FIX_DECIMAL-6+1));
      d->x0       = r->motion.x + xdif;
      d->y0       = r->motion.y - ydif;
      d->x1       = r->motion.x - xdif;
      d->y1       = r->motion.y + ydif;
      d->draw.id       = r->obj.id;
      d->draw.distance = dist;
      d->draw.angle    = angle;
      d->draw.icon_res = r->obj.icon_res;
      d->draw.group    = r->obj.animate->group;
      d->draw.overlays = *(r->obj.overlays);
      d->draw.flags    = r->obj.flags;
      d->draw.drawingtype  = r->obj.drawingtype;
      d->draw.minimapflags  = r->obj.minimapflags;
      d->draw.namecolor = r->obj.namecolor;
      d->draw.objecttype = r->obj.objecttype;
      d->draw.moveontype = r->obj.moveontype;
      d->draw.translation = r->obj.translation;
      d->draw.secondtranslation = r->obj.secondtranslation;
      d->draw.obj      = r;
      d->draw.depth    = 0;
#if 1
      /* Make sure that object is above the floor. */
      if (!GetRoomHeight(room->tree, &top, &bottom, &sector_flags, r->motion.x, r->motion.y))
      {
	 nobjects--;
	 continue;
      }
      d->draw.height = max(bottom, r->motion.z);
#else
      d->draw.height = r->motion.z;
#endif

#if 1
      // Set object depth based on "depth" sector flags
      d->draw.depth = sector_depths[SectorDepth(sector_flags)];
      if (ROOM_OVERRIDE_MASK & GetRoomFlags()) // if depth flags are normal (no overrides)
      {
	 switch (SectorDepth(sector_flags)) {
	 case SF_DEPTH1:
	    if (ROOM_OVERRIDE_DEPTH1 & GetRoomFlags())
	    {
	       d->draw.height = GetOverrideRoomDepth(SF_DEPTH1);
	       d->draw.depth = 0;
	    }
	    break;
	 case SF_DEPTH2:
	    if (ROOM_OVERRIDE_DEPTH2 & GetRoomFlags())
	    {
	       d->draw.height = GetOverrideRoomDepth(SF_DEPTH2);
	       d->draw.depth = 0;
	    }
	    break;
	 case SF_DEPTH3:
	    if (ROOM_OVERRIDE_DEPTH3 & GetRoomFlags())
	    {
	       d->draw.height = GetOverrideRoomDepth(SF_DEPTH3);
	       d->draw.depth = 0;
	    }
	    break;
	 }
      }
#endif      
      d->ncones        = 0;
      d->ncones_ptr    = &d->ncones;

      /* set center field */
      a = (left_a * dx + left_b * dy) >> (FIX_DECIMAL - 6);
      b = (right_a * dx + right_b * dy) >> (FIX_DECIMAL - 6);
      if (a + b <= 0)
      {
	 debug(("a+b <= 0! (AddObjects) %ld\n",a+b));
	 continue;
      }
      d->draw.center = (a * screen_width + screen_width2) / (a + b);

      /* Don't draw more than a few objects per square, but always draw monsters and players */
      d->draw.draw     = True;
      count = 0;
      if (!(r->obj.flags & OF_ATTACKABLE))
	 for (temp_list = l->next; temp_list != NULL; temp_list = temp_list->next)
	 {
	    room_contents_node *other_obj = (room_contents_node *) (temp_list->data);
	    if (FineToSquare(other_obj->motion.x) == FineToSquare(r->motion.x) && 
		FineToSquare(other_obj->motion.y) == FineToSquare(r->motion.y))
	       if (count++ > MAX_OBJS_PER_SQUARE)
	       {
		  d->draw.draw = False;
		  break;
	       }
      }

      //      debug(("Adding object %d\n", r->obj.id));
      if (AddObject(room->tree, d) == False)
	 nobjects--;
   }

   // Add projectiles
   for(l = room->projectiles; l != NULL; l=l->next)
   {
      Projectile *proj = (Projectile *) (l->data);
      
      if (nobjects >= MAXOBJECTS)
      {
	 debug(("out of object memory\n"));
	 return;
      }

      /* compute and check distance */
      dx = proj->motion.x - viewer_x;
      dy = proj->motion.y - viewer_y;
      dist = GetDistance(dx,dy);
      if (dist <= 0)
	continue;
      
      /* compute angle that projectile is viewed at */
      angle = (proj->angle - intATan2(-dy,-dx)) & NUMDEGREES_MASK;

      /* find width of bitmap */
      if (!GetObjectSize(proj->icon_res, proj->animate.group, angle, NULL, &width, &height))
	continue;

      if (nobjects >= MAXOBJECTS)
      {
	 debug(("out of object memory\n"));
	 return;
      }

      d = &objectdata[nobjects++];
      
      d->x0       = proj->motion.x + ((width * center_b) >> (FIX_DECIMAL-6+1));
      d->y0       = proj->motion.y - ((width * center_a) >> (FIX_DECIMAL-6+1));
      d->x1       = proj->motion.x - ((width * center_b) >> (FIX_DECIMAL-6+1));
      d->y1       = proj->motion.y + ((width * center_a) >> (FIX_DECIMAL-6+1));
      d->draw.id       = INVALID_ID;
      d->draw.distance = dist;
      d->draw.angle    = angle;
      d->draw.icon_res = proj->icon_res;
      d->draw.group    = proj->animate.group;
      d->draw.overlays = NULL;
      d->draw.draw     = True;
      d->draw.flags    = 0;
      d->draw.drawingtype  = DRAWFX_DRAW_PLAIN;
      d->draw.minimapflags  = 0;
      d->draw.namecolor = 0;
      d->draw.objecttype = OT_NONE;
      d->draw.moveontype = MOVEON_YES;
      d->draw.height   = proj->motion.z;
      d->draw.translation = proj->translation;
      d->draw.secondtranslation = 0;
      d->ncones        = 0;
      d->ncones_ptr    = &d->ncones;
      d->draw.obj      = NULL;

      /* set center field */
      a = (left_a * dx + left_b * dy) >> (FIX_DECIMAL - 6);
      b = (right_a * dx + right_b * dy) >> (FIX_DECIMAL - 6);
      if (a + b <= 0)
      {
	 debug(("a+b <= 0! (AddObjects) %ld\n",a+b));
	 continue;
      }
      d->draw.center = (a * screen_width + screen_width2) / (a + b);

      if (AddObject(room->tree, d) == False)
	 nobjects--;
   }
}

/*****************************************************************************/
/*****************************************************************************/
/*  Stuff for walking the BSP tree
 */


/* List of cones.  Cones define a viewable quadralateral of the screen.
 * A pixel (h,v) is in a cone if it satisfies the following 4 equations:
 * (1) h >= leftedge
 * (2) h <= rightedge
 * (3) top_a * v >= top_b * h + top_d
 * (4) bot_a * v <= bot_b * h + bot_d
 *
 * For the following quadralateral, we have the shown settings for top &
 * bottom coefficents.
 *
 *                 +   -- topright
 *                /|
 *               / |
 *              /  |
 *  topleft -- +   |
 *             |   |
 *             |   |
 *             |   |
 *             |   |
 * topright -- +   |
 *              \  |
 *               \ |
 *                \|
 *                 +   -- botright
 *
 *             |   |
 *  leftedge _/     \_ rightedge
 *
 * top_a = rightedge - leftedge
 * top_b = topright - topleft
 * top_d = top_a * topleft - top_b * leftedge
 *
 * Note that top_* and bot_* should NOT be recalculated for a sub-cone
 * because roundoff errors will creep in.  Use the same top and bottom edge
 * coefficients with different leftedge/rightedge values.
 *
 * Given a column h, the open rows can be computed using the equations:
 *   vmin = DIVUP(top_b * h + top_d, top_a)
 *   vmax = DIVDOWN(bot_b * h + bot_d, bot_a)
 * note the rounding.  Also note that *_a is always non-zero.
 *
 * Given a row v, the fist pixel in that row which is in the viewcone can
 * be computed using the equations (top only):
 *       / (top_a * v - top_d) / top_b                      top_b > 0
 *   h = | (top_a * v - top_d + top_b + 1) / top_b          top_b < 0
 *       \ undefined                                        top_b = 0
 *
 * Similar, but not the same for the bottom.
 */

#define blakassert(x) { if (!(x)) debug(("assert failed %s, %d\n", __FILE__, __LINE__)); }

/*
 * Cone tree: this data structure keeps track of which parts of the
 * screen are still visible.  This data structure is updated during
 * the front-to-back tree traversal.  The tree is organized as follows:
 *
 *                  x
 *                 / \
 *                /   \
 *               x     x
 *              / \   / \
 *             x   x x   x
 *
 * Each node in the tree is a cone.  There is at most one cone containing
 * each column.  The tree is a binary tree.  A node's left subtree
 * contains cones with smaller columns, and a node's right subtree
 * contains cones with larger columns.  The tree is also threaded with
 * a doubly-linked list which connects the cones in order.
 */

/* pointer to root node of cone tree */
static ConeTreeNode *cone_tree_root;

/* cone tree storage and free list */
static ConeTreeNode cone_node_array[MAXX];
static int allocd_cone_nodes;
static ConeTreeNode *free_cone_node_list;

/* anchors for threading doubly-linked list */
static ConeTreeNode start_anchor, end_anchor;

#define CHECK_DEPTH 0
#if CHECK_DEPTH
static max_depth;
#endif

static void init_cone_tree()
{
  ConeTreeNode *c;
//  debug(("init_cone_tree\n"));
  
  c = cone_tree_root = &cone_node_array[0];
  free_cone_node_list = NULL;
  allocd_cone_nodes = 1;
  
  c->parent = c->left = c->right = NULL;
  c->prev = &start_anchor;
  start_anchor.next = c;
  c->next = &end_anchor;
  end_anchor.prev = c;
  c->height = 1;
  
  start_anchor.cone.rightedge = -10;
  end_anchor.cone.leftedge = MAXX+10;
  
  c->cone.leftedge = 0;
  c->cone.rightedge = screen_width-1;
  c->cone.top_a = 1;
  c->cone.top_b = 0;
  c->cone.top_d = 0;
  c->cone.bot_a = 1;
  c->cone.bot_b = 0;
  c->cone.bot_d = area.cy-1;

#if CHECK_DEPTH
  max_depth = 0;
#endif
}

static void print_cone_tree_1(ConeTreeNode *root)
{
  if (root->left)
    print_cone_tree_1(root->left);
  debug(("[%d %d] ", root->cone.leftedge, root->cone.rightedge));
  if (root->right)
    print_cone_tree_1(root->right);
}
static void print_cone_tree(void)
{
  ConeTreeNode *c;
  print_cone_tree_1(cone_tree_root);
  debug(("\n"));
  for(c = start_anchor.next; c != &end_anchor; c = c->next)
    debug(("<%d %d> ", c->cone.leftedge, c->cone.rightedge));
  debug(("\n"));
}

/* finds lowest-columned cone with some pixels >= column <col> */
/* O(h) time */
ConeTreeNode *search_for_first(long col)
{
  ConeTreeNode *root = cone_tree_root;
  ConeTreeNode *sofar = &end_anchor;  /* lowest-columned cone found so far */
#if CHECK_DEPTH
  int depth = 0;
#endif
//  debug(("search_for_first\n"));
  
  while(root != NULL)
    {
      if (col < root->cone.leftedge)
	{
	  sofar = root;
	  root = root->left;
	}
      else if (col > root->cone.rightedge)
	root = root->right;
      else
	return root;
#if CHECK_DEPTH
      depth++;
      if (depth > max_depth)
	max_depth = depth;
#endif
    }
  return sofar;
}

static void update_height(ConeTreeNode *node)
{
  int h;
  
  blakassert(node);
loop:
  if (node->left)
    {
      if (node->right)
	h = max(node->left->height, node->right->height) + 1;
      else
	h = node->left->height + 1;
    }
  else if (node->right)
    h = node->right->height + 1;
  else
    h = 1;
  
  if (h == node->height)
    return;
  else
    {
      node->height = h;
      node = node->parent;
      if (node)
	goto loop;
    }
}

/* split cone <node> into [node->cone.leftedge,col] and
 * [col+1,node->cone.rightedge].  Returns pointer to the node representing
 * the first interval if <low_half> is True, and returns a pointer to 
 * the node representing the second interval if <low_half> is False.
 */
/* O(h) time. */
static ConeTreeNode *split_cone(ConeTreeNode *node, int col, Bool low_half)
{
  ConeTreeNode *new_node, *prev, *next;
  ConeTreeNode *left, *right;
  int lh, rh;
//  debug(("split_cone\n"));
  
  blakassert(node);
  blakassert(col >= node->cone.leftedge);
  blakassert(col < node->cone.rightedge);
  
  /* allocate new node */
  new_node = free_cone_node_list;
  if (new_node != NULL)
    free_cone_node_list = new_node->next;
  else
    new_node = &cone_node_array[allocd_cone_nodes++];
  
  blakassert(allocd_cone_nodes < MAXX);
  
  /* copy cone */
  new_node->cone = node->cone;
  new_node->parent = node;
  
  left = node->left;
  if (left == NULL)
    lh = 0;
  else
    lh = left->height;
  
  right = node->right;
  if (right == NULL)
    rh = 0;
  else
    rh = right->height;
  
  if (lh <= rh)
    {
      /* initialize new */
      new_node->left = left; if (left != NULL) left->parent = new_node;
      new_node->right = NULL;
      new_node->height = lh + 1;
      new_node->cone.rightedge = col;
      
      /* modify node */
      node->left = new_node;
      node->cone.leftedge = col+1;
      
      /* update doubly-linked list */
      prev = node->prev;
      new_node->prev = prev;  prev->next = new_node;
      new_node->next = node;  node->prev = new_node;
      
      update_height(node);
      
//      print_cone_tree();
      if (low_half) return new_node;
      else return node;
    }
  else
    {
      /* initialize new */
      new_node->left = NULL;
      new_node->right = right; if (right != NULL) right->parent = new_node;
      new_node->height = rh + 1;
      new_node->cone.leftedge = col+1;
      
      /* modify node */
      node->right = new_node;
      node->cone.rightedge = col;
      
      /* update doubly-linked list */
      next = node->next;
      new_node->next = next;  next->prev = new_node;
      new_node->prev = node;  node->next = new_node;
      
      update_height(node);
      
//      print_cone_tree();
      if (low_half) return node;
      else return new_node;
    }
}

/* O(1) time. */
static void delete_cone(ConeTreeNode *node)
{
  ConeTreeNode *prev, *next;
  ConeTreeNode *left, *right, *parent;
  ConeTreeNode *new_node;                   /* new replaces node in tree */
//  debug(("delete_cone\n"));
  
  /* splice tree */
  left = node->left;
  right = node->right;
  parent = node->parent;
  prev = node->prev;
  next = node->next;
  
  if (left == NULL)
    new_node = right;
  else if (right == NULL)
    new_node = left;
  else if (next == right)  /* right has no left child */
    {
      new_node = right;
      new_node->left = left;    /* splice left into new node */
      left->parent = new_node;
    }
  else if (prev == left)   /* left has no right child */
    {
      new_node = left;
      new_node->right = right;  /* splice right into new node */
      right->parent = new_node;
    }
  else
    {
      ConeTreeNode *p, *c;
//      debug(("special delete\n"));
      new_node = next;
      blakassert(new_node->left == NULL);
      
      /* splice new out of current location in tree */
      p = new_node->parent;
      blakassert(p);
      c = new_node->right;
      if (c) c->parent = p;
      blakassert(p->left == new_node);
      p->left = c;
      update_height(p);
      
      /* splice new into where node was */
      new_node->left = left; left->parent = new_node;
      new_node->right = right; right->parent = new_node;
      new_node->height = node->height;
    }
  
  /* update doubly-linked list */
  prev->next = next;
  // XXX Added this check ARK
  if (next != NULL)
     next->prev = prev;
  
  if (parent != NULL)
    {
      if (node == parent->left)
	parent->left = new_node;
      else
	parent->right = new_node;
    }
  else
    cone_tree_root = new_node;
  
  if (new_node != NULL)
    {
      new_node->parent = parent;
      update_height(new_node);
    }
  
  /* add freed node to free list */
  node->next = free_cone_node_list;
  free_cone_node_list = node;

//  debug(("end of delete_cone\n"));
}

static void check_tree(ConeTreeNode *root, ConeTreeNode *parent)
{
  blakassert(root);
  blakassert(root->parent == parent);
  if (root->left)
    check_tree(root->left, root);
  if (root->right)
    check_tree(root->right, root);
}

/* Merges a cone into the cone to its immediate left, if it can be
 * merged.
 */
static void merge_cone(ConeTreeNode *node)
{
  ConeTreeNode *prev = node->prev;
  
  if (prev->cone.rightedge == node->cone.leftedge - 1)
    {
      if (prev->cone.top_a == node->cone.top_a &&
	  prev->cone.top_b == node->cone.top_b &&
	  prev->cone.top_d == node->cone.top_d &&
	  prev->cone.bot_a == node->cone.bot_a &&
	  prev->cone.bot_b == node->cone.bot_b &&
	  prev->cone.bot_d == node->cone.bot_d)
	{
	  prev->cone.rightedge = node->cone.rightedge;
	  delete_cone(node);
	}
      else if (prev->cone.leftedge == prev->cone.rightedge)
	{
	  long col = prev->cone.rightedge;
	  if (DIVUP(prev->cone.top_b * col + prev->cone.top_d, prev->cone.top_a) ==
	      DIVUP(node->cone.top_b * col + node->cone.top_d, node->cone.top_a) &&
	      DIVDOWN(prev->cone.bot_b * col + prev->cone.bot_d, prev->cone.bot_a) ==
	      DIVDOWN(node->cone.bot_b * col + node->cone.bot_d, node->cone.bot_a))
	    {
	      delete_cone(prev);
	      node->cone.leftedge = col;
	    }
	}
      else if (node->cone.leftedge == node->cone.rightedge)
	{
	  long col = node->cone.rightedge;
	  if (DIVUP(node->cone.top_b * col + node->cone.top_d, node->cone.top_a) ==
	      DIVUP(prev->cone.top_b * col + prev->cone.top_d, prev->cone.top_a) &&
	      DIVDOWN(node->cone.bot_b * col + node->cone.bot_d, node->cone.bot_a) ==
	      DIVDOWN(prev->cone.bot_b * col + prev->cone.bot_d, prev->cone.bot_a))
	    {
	      delete_cone(node);
	      prev->cone.rightedge = col;
	    }
	}
    }
}


/* Try go get rid of small cones: a cone must be at least SLACK_PIXELS
 * pixels high on either the left or right edge to be visible.
 */
#define SLACK_PIXELS 1

/*****************************************************************************/
/* Add item <item> to the draw list, where the item includes all pixels
 * above (a,b,d) between columns col0 and col1, inclusive.  Remove from
 * the remaining cones all of these pixels.  Returns True if any of the
 * item is actually visible.
 * "above (a,b,d)" means having coordinates (x,y) such that ay <= bx+d
 */
/*****************************************************************************/
static Bool add_up(DrawItem *item_template, long a, long b, long d, long col0, long col1)
{
  ConeTreeNode *c,*next;
  DrawItem *item = NULL;
  long c0,c1,cmid;
  long num,denom;
  Bool viewable = False;
  long a_topb, a_topd, topa_b, topa_d;
  long a_botb, a_botd, bota_b, bota_d;
  long slack;
  Bool additem = item_template->type != DrawBackgroundType || incremental_background;
  
  blakassert(col1 < MAXX);
  for(c = search_for_first(col0); c->cone.leftedge <= col1; c = next)
    {
      next = c->next;  /* get next pointer now, before we munge c */
      
      c0 = max(col0, c->cone.leftedge);
      c1 = min(col1, c->cone.rightedge);
      
      /* some common subexpressions */
      a_topb = a * c->cone.top_b;
      a_topd = a * c->cone.top_d;
      topa_b = c->cone.top_a * b;
      topa_d = c->cone.top_a * d;
      slack = a * c->cone.top_a * SLACK_PIXELS;
      
      if (a_topb * c0 + a_topd + slack <= topa_b * c0 + topa_d)
	{
	  if (a_topb * c1 + a_topd + slack <= topa_b * c1 + topa_d)
	    {
	      /* both c0 and c1 columns of the item are visible */
	    }
	  else
	    {
	      /* right side of item isn't visible - get a smaller c1 */
	      denom = topa_b - a_topb;
	      num = a_topd - topa_d;
	      cmid = num / denom;
	      if (cmid < c1)
		c1 = cmid;
	    }
	}
      else if (a_topb * c1 + a_topd + slack <= topa_b * c1 + topa_d)
	{
	  /* left side of item isn't visible - get a bigger c0 */
	  denom = topa_b - a_topb;
	  num = a_topd - topa_d;
	  if (denom > 0)
	    cmid = (num + denom - 1) / denom;
	  else
	    cmid = (num + denom + 1) / denom;
	  if (cmid > c0)
	    c0 = cmid;
	}
      else
	continue;   /* item can't be seen in this cone */
      
      /* we now know we can see the object */
      viewable = True;
      
      /* make cones for non-overlapping regions */
      if (c0 > c->cone.leftedge)
	c = split_cone(c, c0-1, False);
      if (c1 < c->cone.rightedge)
	c = split_cone(c, c1, True);
      
      a_botb = a * c->cone.bot_b;
      a_botd = a * c->cone.bot_d;
      bota_b = c->cone.bot_a * b;
      bota_d = c->cone.bot_a * d;
      
      if (bota_b * c0 + bota_d < a_botb * c0 + a_botd)
	{
	  if (bota_b * c1 + bota_d < a_botb * c1 + a_botd)
	    {
	      /* old cone bottom is lower than new item bottom -
	       * entire cone will survive */
	    }
	  else
	    {
	      /* right bottom of new item is lower than cone */
	      denom = a_botb - bota_b;
	      num = bota_d - a_botd;
	      if (denom > 0)
		cmid = (num + denom - 1) / denom;
	      else
		cmid = (num + denom + 1) / denom;
	      
	      if (additem)
		{
		  /* add right part of item to draw list */
		  if (nitems >= MAX_ITEMS)
		    {
		      debug(("out of item memory add_up 1\n"));
		      return viewable;
		    }
		  item = &drawdata[nitems++];
		  item->type = item_template->type; item->u = item_template->u;
		  item->cone = c->cone;
		  item->cone.leftedge = cmid;
		}
	      else
		background_cones++;
	      
	      c1 = c->cone.rightedge = cmid-1;
	    }
	}
      else if (bota_b * c1 + bota_d < a_botb * c1 + a_botd)
	{
	  /* left bottom of new item is lower than cone */
	  denom = a_botb - bota_b;
	  num = bota_d - a_botd;
	  cmid = num / denom;
	  
	  if (additem)
	    {
	      /* add left part of item to draw list */
	      if (nitems >= MAX_ITEMS)
		{
		  debug(("out of item memory add_up 2\n"));
		  return viewable;
		}
	      item = &drawdata[nitems++];
	      item->type = item_template->type; item->u = item_template->u;
	      item->cone = c->cone;
	      item->cone.rightedge = cmid;
	    }
	  else
	    background_cones++;
	  
	  c0 = c->cone.leftedge = cmid+1;
	}
      else
	{
	  /* new item is completely lower than cone */
	  if (additem)
	    {
	      /* add item to draw list */
	      if (nitems >= MAX_ITEMS)
		{
		  debug(("out of item memory add_up 3\n"));
		  return viewable;
		}
	      item = &drawdata[nitems++];
	      item->type = item_template->type; item->u = item_template->u;
	      item->cone = c->cone;
	    }
	  else
	    background_cones++;
	  
	  /* cone is filled - remove it from cone tree */
	  delete_cone(c);
	  continue;
	}
      
      if (additem)
	{
	  if (item &&
	      item->cone.rightedge == c0-1 &&
	      item->cone.top_a == c->cone.top_a &&
	      item->cone.top_b == c->cone.top_b &&
	      item->cone.top_d == c->cone.top_d &&
	      item->cone.bot_a == a &&
	      item->cone.bot_b == b &&
	      item->cone.bot_d == d)
	    {
//	      debug(("add_up: simplifying %d %d %d\n", item->cone.leftedge, c0, c1));
	      item->cone.rightedge = c1;
	    }
	  else
	    {
	      /* add item to draw list */
	      if (nitems >= MAX_ITEMS)
		{
		  debug(("out of item memory add_up 4\n"));
		  return viewable;
		}
	      item = &drawdata[nitems++];
	      item->type = item_template->type; item->u = item_template->u;
	      item->cone.leftedge = c0;
	      item->cone.rightedge = c1;
	      item->cone.top_a = c->cone.top_a;
	      item->cone.top_b = c->cone.top_b;
	      item->cone.top_d = c->cone.top_d;
	      item->cone.bot_a = a;
	      item->cone.bot_b = b;
	      item->cone.bot_d = d;
	    }
	}
      else
	background_cones++;
      
      /* calculate remnant cone */
      c->cone.top_a = a;
      c->cone.top_b = b;
      c->cone.top_d = d+1;   /* +1 should give no overlap, no gap!!! */
      merge_cone(c);
    }
  merge_cone(c);
  return viewable;
}

/*****************************************************************************/
/* Add item <item> to the draw list, where the item includes all pixels
 * below (a,b,d) between columns col0 and col1, inclusive.  Remove from
 * the remaining cones all of these pixels.  Returns True if any of the
 * object is actually visible.
 * "below (a,b,d)" means having coordinates (x,y) such that ay >= bx+d
 */
/*****************************************************************************/
static Bool add_dn(DrawItem *item_template, long a, long b, long d, long col0, long col1)
{
  ConeTreeNode *c, *next;
  DrawItem *item = NULL;
  long c0,c1,cmid;
  long num,denom;
  Bool viewable = False;
  long a_botb, a_botd, bota_b, bota_d;
  long a_topb, a_topd, topa_b, topa_d;
  long slack;
  Bool additem = item_template->type != DrawBackgroundType || incremental_background;
  
  blakassert(col1 < MAXX);
  for(c = search_for_first(col0); c->cone.leftedge <= col1; c = next)
    {
      next = c->next;  /* get next pointer now, before we munge c */
      
      /* get left and right edges of item under consideration */
      c0 = max(col0, c->cone.leftedge);
      c1 = min(col1, c->cone.rightedge);
      
      a_botb = a * c->cone.bot_b;
      a_botd = a * c->cone.bot_d;
      bota_b = c->cone.bot_a * b;
      bota_d = c->cone.bot_a * d;
      slack = a * c->cone.bot_a * SLACK_PIXELS;
      
      /* check to see if c0 and c1 are tight enough */
      if (bota_b * c0 + bota_d + slack <= a_botb * c0 + a_botd)
	{
	  if (bota_b * c1 + bota_d + slack <= a_botb * c1 + a_botd)
	    {
	      /* both c0 and c1 columns of the item are visible */
	    }
	  else
	    {
	      /* right side of item isn't visible - get a smaller c1 */
	      denom = a_botb - bota_b;
	      num = bota_d - a_botd;
	      cmid = num / denom;
	      if (cmid < c1)
		c1 = cmid;
	    }
	}
      else if (bota_b * c1 + bota_d + slack <= a_botb * c1 + a_botd)
	{
	  /* left side of item isn't visible - get a bigger c0 */
	  denom = a_botb - bota_b;
	  num = bota_d - a_botd;
	  if (denom > 0)
	    cmid = (num + denom - 1) / denom;
	  else
	    cmid = (num + denom + 1) / denom;
	  if (cmid > c0)
	    c0 = cmid;
	}
      else
	continue;   /* item can't be seen in this cone */
      
      viewable = True;
      
      /* make cones for non-overlapping regions */
      if (c0 > c->cone.leftedge)
	c = split_cone(c, c0-1, False);
      if (c1 < c->cone.rightedge)
	c = split_cone(c, c1, True);
      
      a_topb = a * c->cone.top_b;
      a_topd = a * c->cone.top_d;
      topa_b = c->cone.top_a * b;
      topa_d = c->cone.top_a * d;
      
      if (topa_b * c0 + topa_d > a_topb * c0 + a_topd)
	{
	  if (topa_b * c1 + topa_d > a_topb * c1 + a_topd)
	    {
	      /* old cone top is higher than new item top -
	       * entire cone will survive */
	    }
	  else
	    {
	      /* right top of new item is higher than cone */
	      denom = a_topb - topa_b;
	      num = topa_d - a_topd;
	      if (denom > 0)
		cmid = (num + denom - 1) / denom;
	      else
		cmid = (num + denom + 1) / denom;
	      
	      if (additem)
		{
		  /* add right part of item to draw list */
		  if (nitems >= MAX_ITEMS)
		    {
		      debug(("out of item memory add_dn 1\n"));
		      return viewable;
		    }
		  item = &drawdata[nitems++];
		  item->type = item_template->type; item->u = item_template->u;
		  item->cone = c->cone;
		  item->cone.leftedge = cmid;
		}
	      else
		background_cones++;
	      
	      c1 = c->cone.rightedge = cmid-1;
	    }
	}
      else if (topa_b * c1 + topa_d > a_topb * c1 + a_topd)
	{
	  /* left top of new item is higher than cone */
	  denom = a_topb - topa_b;
	  num = topa_d - a_topd;
	  cmid = num / denom;
	  
	  if (additem)
	    {
	      /* add left part of item to draw list */
	      if (nitems >= MAX_ITEMS)
		{
		  debug(("out of item memory add_dn 2\n"));
		  return viewable;
		}
	      item = &drawdata[nitems++];
	      item->type = item_template->type; item->u = item_template->u;
	      item->cone = c->cone;
	      item->cone.rightedge = cmid;
	    }
	  else
	    background_cones++;
	  
	  c0 = c->cone.leftedge = cmid+1;
	}
      else
	{
	  /* new item is completely higher than cone */
	  if (additem)
	    {
	      /* add item to draw list */
	      if (nitems >= MAX_ITEMS)
		{
		  debug(("out of item memory add_dn 3\n"));
		  return viewable;
		}
	      item = &drawdata[nitems++];
	      item->type = item_template->type; item->u = item_template->u;
	      item->cone = c->cone;
	    }
	  else
	    background_cones++;
	  
	  /* cone is filled - remove it from cone tree */
	  delete_cone(c);
	  continue;
	}
      
      if (additem)
	{
	  if (item &&
	      item->cone.rightedge == c0-1 &&
	      item->cone.bot_a == c->cone.bot_a &&
	      item->cone.bot_b == c->cone.bot_b &&
	      item->cone.bot_d == c->cone.bot_d &&
	      item->cone.top_a == a &&
	      item->cone.top_b == b &&
	      item->cone.top_d == d)
	    {
//	      debug(("add_dn: simplifying %d %d %d\n", item->cone.leftedge, c0, c1));
	      item->cone.rightedge = c1;
	    }
	  else
	    {
	      /* add item to draw list */
	      if (nitems >= MAX_ITEMS)
		{
		  debug(("out of item memory add_dn 4\n"));
		  return viewable;
		}
	      item = &drawdata[nitems++];
	      item->type = item_template->type; item->u = item_template->u;
	      item->cone.leftedge = c0;
	      item->cone.rightedge = c1;
	      item->cone.bot_a = c->cone.bot_a;
	      item->cone.bot_b = c->cone.bot_b;
	      item->cone.bot_d = c->cone.bot_d;
	      item->cone.top_a = a;
	      item->cone.top_b = b;
	      item->cone.top_d = d;
	    }
	}
      else
	background_cones++;
      
      /* calculate remnant cone */
      c->cone.bot_a = a;
      c->cone.bot_b = b;
      c->cone.bot_d = d-1; /* -1 should give no overlap, no gap!!! */
      merge_cone(c);
    }
  merge_cone(c);
  return viewable;
}

/* converts a line from (x0,y0) to (x1,y1) in the world to a line
 * from col0 to col1 on the screen at distance d0 to d1.  Returns
 * 0 if line does not map to screen.  Returns non-zero if it does.
 * Returned line is clipped to screen (left-right only) and is oriented
 * in the same direction as input line.
 * This routine now sets the values t0 & t1 to be used by the calling
 * procedure. When a wall is clipped to the screen, these are set to
 * indicate how far along the original line the new, clipped, endpoints
 * lie. These values are use to interpolate the height(s) at each endpoint
 * of a wall that bounds a sloped sector.
 */
int world_to_screen(float x0, float y0, float x1, float y1,
		    long *t0, long *t1, /* special parameters for sloped walls */
          long *col0, float *d0, long *col1, float *d1)
{
   float left0, right0, left1, right1;
   float x, y;
    long t,tleft,tright;
  
  /* go to viewer-relative coords */
  x0 -= viewer_x;
  y0 -= viewer_y;
  x1 -= viewer_x;
  y1 -= viewer_y;

  /* calculate perpindicular distance of endpoints from left and right view frustum bounds */
  /* positive distance is in viewable half-space (both positive => in view frustum) */
  left0 = (left_a * x0 + left_b * y0) / FINENESS;
  right0 = (right_a * x0 + right_b * y0) / FINENESS;
  left1 = (left_a * x1 + left_b * y1) / FINENESS;
  right1 = (right_a * x1 + right_b * y1) / FINENESS;
  
  /* make sure left and right are not both zero.  If they are,   */
  /* move them to a point that maps to the center of the screen. */
  if (right0 <= 0.001 && right0 >= -0.001)
     right0 = 1.0f;
  if (right1 <= 0.001 && right1 >= -0.001)
     right1 = 1.0f;
  
  if (left0 < 0)
  {
     if (left1 < 0)
	return 0;              /* wall completely behind left ray */
     if (right0 < 0)
     {
	if (right1 < 0)
	   return 0;          /* wall completely behind right ray */
	/* quad 3 to quad 0 wall */
   tleft = ((-left0) * FINENESS) / (left1 - left0);
   tright = ((-right0) * FINENESS) / (right1 - right0);
	if (tleft > tright)
	{	/* left of viewer */
	   x = x0 + (((x1 - x0) * tleft) / FINENESS);
	   y = y0 + (((y1 - y0) * tleft) / FINENESS);
	   *col0 = 0;
	   *d0 = GetDistance(x,y);
	   *t0 = tleft;
	   
	   *col1 = (left1 * screen_width + screen_width2) / (left1 + right1);
	   *d1 = GetDistance(x1,y1);
	   *t1 = FINENESS;
	}
	else
	{	/* right of viewer */
	   x = x0 + (((x1-x0) * tright) / FINENESS);
	   y = y0 + (((y1-y0) * tright) / FINENESS);
	   *col0 = screen_width;
	   *d0 = GetDistance(x,y);
	   *t0 = tright;
	   
	   *col1 = (left1 * screen_width + screen_width2) / (left1 + right1);
	   *d1 = GetDistance(x1,y1);
	   *t1 = FINENESS;
	}
     }
     else if (right1 < 0)
     {	/* quad 1 to quad 2 wall */
	t = ((-left0) * FINENESS) / (left1 - left0);
	x = x0 + (((x1-x0) * t) / FINENESS);
	y = y0 + (((y1-y0) * t) / FINENESS);
	*col0 = 0;
	*d0 = GetDistance(x,y);
	*t0 = t;
	
	if (*d0 <= 0)
	   return 0;
	
   t = (right0 * FINENESS) / (right0 - right1);
	x = x0 + (((x1-x0) * t) / FINENESS);
	y = y0 + (((y1-y0) * t) / FINENESS);
	*col1 = screen_width;
	*d1 = GetDistance(x,y);
	*t1 = t;
     }
     else
     {	/* quad 1 to quad 0 wall */
        t = ((-left0) * FINENESS) / (left1 - left0);
	x = x0 + (((x1-x0) * t) / FINENESS);
	y = y0 + (((y1-y0) * t) / FINENESS);
	*col0 = 0;
	*d0 = GetDistance(x,y);
	*t0 = t;
	
	*col1 = (left1 * screen_width + screen_width2) / (left1 + right1);
	*d1 = GetDistance(x1,y1);
	*t1 = FINENESS;
     }
  }
  else if (left1 < 0)
  {	
     if (right1 < 0)
     {
	if (right0 < 0)
	   return 0;          /* wall completely behind right ray */
	/* quad 0 to quad 3 wall */
   tleft = (left0 * FINENESS) / (left0 - left1);
   tright = (right0 * FINENESS) / (right0 - right1);
	if (tleft < tright)
	{	/* left of viewer */
	   *col0 = (left0 * screen_width + screen_width2) / (left0 + right0);
	   *d0 = GetDistance(x0,y0);
	   *t0 = 0;
	   
	   x = x0 + (((x1-x0) * tleft) / FINENESS);
	   y = y0 + (((y1-y0) * tleft) / FINENESS);
	   *col1 = 0;
	   *d1 = GetDistance(x,y);
	   *t1 = tleft;
	}
	else
	{	/* right of viewer */
	   *col0 = (left0 * screen_width + screen_width2) / (left0 + right0);
	   *d0 = GetDistance(x0,y0);
	   *t0 = 0;
	   
	   x = x0 + (((x1-x0) * tright) / FINENESS);
	   y = y0 + (((y1-y0) * tright) / FINENESS);
	   *col1 = screen_width;
	   *d1 = GetDistance(x,y);
	   *t1 = tright;
	}
     }
     else if (right0 < 0)
     {	/* quad 2 to quad 1 wall */
        t = ((-right0) * FINENESS) / (right1 - right0);
	x = x0 + (((x1-x0) * t) / FINENESS);
	y = y0 + (((y1-y0) * t) / FINENESS);
	*col0 = screen_width;
	*d0 = GetDistance(x,y);
	*t0 = t;
	
	if (*d0 <= 0)
	   return 0;
	
   t = (left0 * FINENESS) / (left0 - left1);
	x = x0 + (((x1-x0) * t) / FINENESS);
	y = y0 + (((y1-y0) * t) / FINENESS);
	*col1 = 0;
	*d1 = GetDistance(x,y);
	*t1 = t;
     }
     else
     {	/* quad 0 to quad 1 wall */
	*col0 = (left0 * screen_width + screen_width2) / (left0 + right0);
	*d0 = GetDistance(x0,y0);
	*t0 = 0;
	
   t = (left0 * FINENESS) / (left0 - left1);
	x = x0 + (((x1-x0) * t) / FINENESS);
	y = y0 + (((y1-y0) * t) / FINENESS);
	*col1 = 0;
	*d1 = GetDistance(x,y);
	*t1 = t;
     }
  }
  else if (right0 < 0)
  {
     if (right1 < 0)
	return 0;              /* wall completely behind right ray */
     /* quad 2 to quad 0 wall */
     t = ((-right0) * FINENESS) / (right1 - right0);
     x = x0 + (((x1-x0) * t) / FINENESS);
     y = y0 + (((y1-y0) * t) / FINENESS);
     *col0 = screen_width;
     *d0 = GetDistance(x,y);
     *t0 = t;
     
     *col1 = (left1 * screen_width + screen_width2) / (left1 + right1);
     *d1 = GetDistance(x1,y1);
     *t1 = FINENESS;
  }
  else if (right1 < 0)
  {	/* quad 0 to quad 2 wall */
     *col0 = (left0 * screen_width + screen_width2) / (left0 + right0);
     *d0 = GetDistance(x0,y0);
     *t0 = 0;
     
     t = (right0 * FINENESS) / (right0 - right1);
     x = x0 + (((x1-x0) * t) / FINENESS);
     y = y0 + (((y1-y0) * t) / FINENESS);
     *col1 = screen_width;
     *d1 = GetDistance(x,y);
     *t1 = t;
  }
  else
  {   /* quad 0 to quad 0 wall */
     *col0 = (left0 * screen_width + screen_width2) / (left0 + right0);
     *d0 = GetDistance(x0,y0);
     *t0 = 0;
     
     *col1 = (left1 * screen_width + screen_width2) / (left1 + right1);
     *d1 = GetDistance(x1,y1);
     *t1 = FINENESS;
  }
  
  if (*d0 <= 0) *d0 = 1;
  if (*d1 <= 0) *d1 = 1;
  
#if 0
  debug(("converted (%d %d)-(%d %d),(%d) to (%d %d)-(%d %d))\n",
	  x0+viewer_x,y0+viewer_y,x1+viewer_x,y1+viewer_y,h,
	  *col0,*row0,*col1,*row1);
#endif
  return 1;
}



/* quadrants are numbered as follows:
 *       \      /
 *        \  0 /
 *         \  /
 *      1   \/   2
 *          /\
 *         /  \
 *        /    \
 *       /   3  \
 *
 *   the center is the viewer.
 *   quadrant 0 is the view frustum. (i.e., viewer is facing up)
 *
 *  The quadrant of a point (x,y) (viewer-relative) is determined as follows:
 *  1) compute left_a * x + left_b * y.  This number gives the perpindicular distance
 *     of (x,y) from the left edge of the view frustum.  Positive values mean quadrants
 *     0 or 2, negative ones quadrants 1 or 3.
 *  2) compute right_a * x + right_b * y.  Same as above for right edge of view frustum.
 *
 *  If a wall's vertices are in quadrants 1&3, or in quadrants 2&3, it is not visible.
 *  A wall going between quadrants 1&2 may or may not be visible: it depends on whether it
 *  goes in front of or behind the viewer.  This can be determined with a hacky computation.
 *  Similar hacking can determine whether a 0&3 line passes to the left or right of the viewer.
 *  Everything else is pretty simple.
 */

/*****************************************************************************/
/* Determine whether the bounding box (x0,y0,x1,y1) is viewable.
 * Returns True if it is definitely not visible.  Returns False otherwise.
 * A return value of False does NOT guarantee visibility.
 */
Bool Bbox_shadowed(long x0, long y0, long x1, long y1)
{
   long d;
   
   long l0,l1,l2,l3,r0,r1,r2,r3;
   long offleftedge,offrightedge;
   long col0,col1;
   
   x0 -= viewer_x;
   y0 -= viewer_y;
   x1 -= viewer_x;
   y1 -= viewer_y;
   
   d = center_a * x0 + center_b * y0;
   if (d < 0)
   {	
      d = center_a * x0 + center_b * y1;
      if (d < 0)
      {
	 d = center_a * x1 + center_b * y0;
	 if (d < 0)
	 {
	    d = center_a * x1 + center_b * y1;
	    if (d < 0)
	    {
	       return True;                /* all behind viewer */
	    }
	 }
      }
   }
   
   l0 = left_a * x0 + left_b * y0;
   l1 = left_a * x0 + left_b * y1;
   l2 = left_a * x1 + left_b * y1;
   l3 = left_a * x1 + left_b * y0;
   
   if (l0 < 0 && l1 < 0 && l2 < 0 && l3 < 0)
   {
      return True;                /* all in quadrants 1&3 */
   }
   
   r0 = right_a * x0 + right_b * y0;
   r1 = right_a * x0 + right_b * y1;
   r2 = right_a * x1 + right_b * y1;
   r3 = right_a * x1 + right_b * y0;
   
   if (r0 < 0 && r1 < 0 && r2 < 0 && r3 < 0)
   {
      return True;                /* all in quadrants 2&3 */
   }
   
   l0 >>= FIX_DECIMAL - 6;
   l1 >>= FIX_DECIMAL - 6;
   l2 >>= FIX_DECIMAL - 6;
   l3 >>= FIX_DECIMAL - 6;
   r0 >>= FIX_DECIMAL - 6;
   r1 >>= FIX_DECIMAL - 6;
   r2 >>= FIX_DECIMAL - 6;
   r3 >>= FIX_DECIMAL - 6;
   
   /* We will conservatively assume that any 1-2 lines are visible.  We could
    * determine that some 1-2 lines are invisible here, and thus discard the box.
    * However, I don't think this occurs often enough to warrant computing it.
    */
   
   if (l0 >= 0 && l1 >= 0 && l2 >= 0 && l3 >= 0)
      offleftedge = 0;
   else
      offleftedge = 1;
   
   if (r0 >= 0 && r1 >= 0 && r2 >= 0 && r3 >= 0)
      offrightedge = 0;
   else
      offrightedge = 1;
   
   /* if box crosses both edges of screen, there's a lot of possibilites: It's simpler to
    * just let box be visible.
    */
   if (offleftedge && offrightedge)
      return(0);
   
   if (offleftedge)
      col0 = 0;
   else
   {	/* find minimum column */
      col0 = screen_width;
      if (r0 > 0)
	 col0 = min(col0, (screen_width * l0 + screen_width2) / (l0 + r0));
      if (r1 > 0)
	 col0 = min(col0, (screen_width * l1 + screen_width2) / (l1 + r1));
      if (r2 > 0)
	 col0 = min(col0, (screen_width * l2 + screen_width2) / (l2 + r2));
      if (r3 > 0)
	 col0 = min(col0, (screen_width * l3 + screen_width2) / (l3 + r3));
   }
   
   if (offrightedge)
      col1 = screen_width;
   else
   {	/* find maximum column */
      col1 = 0;
      if (l0 > 0)
	 col1 = max(col1, (screen_width * l0 + screen_width2) / (l0 + r0));
      if (l1 > 0)
	 col1 = max(col1, (screen_width * l1 + screen_width2) / (l1 + r1));
      if (l2 > 0)
	 col1 = max(col1, (screen_width * l2 + screen_width2) / (l2 + r2));
      if (l3 > 0)
	 col1 = max(col1, (screen_width * l3 + screen_width2) / (l3 + r3));
   }
   
   /* Note: we only use the left and right edges of the view cone because
    * we have no way to determine how high a bounding box is.
    */
   if (search_for_first(col0)->cone.leftedge <= col1)
     return 0;
   
   return(1);
}

/*****************************************************************************/
/*****************************************************************************/
/*  A WORD ABOUT BOWTIES - by Colin Andrews
 */

/*   Bowties are walls on the boundary between a sloped sector and another
 * sloped or normal sector. They have the property that at one endpoint the sector
 * in front of the wall is higher than the one behind, and at the other end of the
 * wall, the sector behind is higher than the one in front. So, it looks something
 * like this if your are looking at it from in front:
 *
 *      .
 *      |\     .
 *      | \   /|
 *      |  \ / |
 *      |   x  |
 *      |  / \ |
 *      | /   \|
 *      |/     '
 *      '
 *
 *   What the user should see when looking at one of these walls depends on which
 * side of the wall the user is standing. The sector on the side of the wall closer
 * to the user always covers everything from one of the diagonals down (for lower
 * walls, up for upper walls) so all that needs to be drawn is the part of the bowtie
 * that extends above the nearer sector:
 *
 *      .
 *       \     .
 *        \   /|
 *         \ / |
 *          x  |
 *           \ |
 *            \|
 *             \
 *              \
 *
 *   Which part of the bowtie shape gets drawn depends on which side of wall the user
 * is on. From the other side of the above example, the other point of the bowtie
 * would extend above the nearer sector. Which point of the bowtie gets drawn also
 * depends on which sector is higher at each endpoint of the wall. In the above example,
 * the closer sector is above at the left endpoint of the wall. If the sector behind
 * the wall is above at the left endpoint, then it would look like this:
 *
 *      .       
 *      |\     .
 *      | \   / 
 *      |  \ /  
 *      |   x   
 *      |  /    
 *      | /     
 *      |/      
 *      /
 *     /
 *
 *   There are four cases that need to be handled by the wall rendering code:
 *
 *                              |
 *                +             |             +
 *                |\     -      |      -     /|
 *                | \   /|      |      |\   /-|
 *                |  \ /+|      |      | \ /--|         Positive sector is above
 *                |   x++|      |      |  x---|          at endpoint 0
 *                |  / \+|      |      | / \--|
 *                | /   \|      |      |/   \-|
 *                |/     +      |      +     \|
 *                -             |             -
 *      endpoint: 0      1      |      1      0
 *                              |
 *  ----------------------------+----------------------------------
 *                              |
 *                -             |             -
 *                |\     +      |      +     /|
 *                |+\   /|      |      |\   / |
 *                |++\ / |      |      |-\ /  |         Negative sector is above
 *                |+++x  |      |      |--x   |          at endpoint 0
 *                |++/ \ |      |      |-/ \  |
 *                |+/   \|      |      |/   \ |
 *                |/     -      |      -     \|
 *                +             |             +
 *      endpoint: 0      1      |      1      0
 *                              |
 *                              |
 *      User is on Positive     |  User is on Negative side of wall
 *        side of wall          |
 *
 *
 *   In the table above, the top row shows how bowties should be drawn when
 * the positive sector is above at the first endpoint. The lower row shows
 * how bowties should be drawn when the negative sector is on top at the
 * first endpoint. The left coloumn shows the bowties when the user is on
 * the positive side of the wall and the right column shows the view from
 * the negative side. The part of the bowtie that is filled in with +'s or -'s
 * is the part that needs to get filled by the wall rendering code. A '+'
 * indicates the positive sidedef and a '-' indicates the negative sidedef.
 *
 *   Bowties in the upper or lower walls are identified by SetWallHeights.
 * It is assumed that bowtied normal walls cannot (or should not) exist. The
 * height values for the WallData struct are filled in as they be would for a
 * normal wall. That is: z0 <= z1 <= z2 <= z3 and zz0 <= zz1 <= zz2 <= zz3.
 * Flag bits are set in the wall struct indicating that the above and or below
 * wall is bowtied and whether each has the positive sector on top at the first
 * endpoint.
 *
 *   When WalkWall sees that a wall is flaged as bowtie, it modifies the z
 * values at either endpoint so that the wall has the proper shape for the side
 * facing the user.
 *
 *   For example, assume that the following is a lower wall bowtie with the
 * negative sector on top at the first endpoint:
 *
 *    z1 -       + zz1
 *       |\     /|
 *       | \   / |
 *       |  \ /  |
 *       |   x   |
 *       |  / \  |
 *       | /   \ |
 *       |/     \|
 *    z0 +       - zz0
 *
 *   When this wall is viewed from the positive side, WalkWall sets z1 = z0 so
 * the wall will look like this:
 *
 *               + zz1
 *              /|
 *             / |
 *            /  |
 *           /   |
 *          /    |
 *         /     |
 *        /      |
 *    z0 + ----- - zz0
 *
 *   The routine can then go on to handle this wall just like a normal wall. Because
 * everything closer than this has already been placed in the cone tree, everything
 * below the original line from z1 to zz0 will be hidden, and the wall will get
 * rendered as:
 *
 *               . 
 *        \     /|
 *         \   / |
 *          \ /  |
 *           \   |
 *            \  |
 *             \ |
 *              \|
 *               \ 
 *                \
 *
 *   This is exactly what needs to get drawn for this bowtie for this situation.
 *
 *****************************************************************************/

/*****************************************************************************/
/*  WalkWall
 *    project wall to screen and generate draw item for wall
 */
static void WalkWall(WallData *wall, long side)
{
   long col0,col1;
   float d0,d1;
   ConeTreeNode *c, *next;
   long toprow0,toprow1,botrow0,botrow1;
   long a,b,d;
   long z0,zz0,z1,zz1,z2,zz2,z3,zz3;
   long t0,t1;

   DrawItem item_template;
   
   DrawItem *item;
   Sidedef *sidedef;

   // These booleans are used in d3drender.c to decide whether to add the wall to
   // the rendering pipeline. If these are modified or removed, d3drender.c needs
   // to be modified also.
   wall->drawbelow = FALSE;
   wall->drawabove = FALSE;
   wall->drawnormal = FALSE;

   /* Skip if nothing on this side */
   if (side > 0)
      sidedef = wall->pos_sidedef;
   else sidedef = wall->neg_sidedef;

   if (sidedef == NULL)
       return;

#if 0
   debug(("WalkWall %d %d %d %d\n",wall->x0,wall->y0,wall->x1,wall->y1));
#endif
   
   z0 = wall->z0;
   zz0 = wall->zz0;
   z1 = wall->z1;
   zz1 = wall->zz1;
   z2 = wall->z2;
   zz2 = wall->zz2;
   z3 = wall->z3;
   zz3 = wall->zz3;

   // bugger values so bowties render properly (see notes above)
   if ((wall->bowtie_bits&BT_BELOW_BOWTIE) != 0) {
       // lower wall is bowtie
       if (wall->bowtie_bits&BT_BELOW_POS) {
	   if (side >= 0)
	       z1 = z0;
	   else
	       zz1 = zz0;
       }
       else {
	   if (side >= 0)
	       zz1 = zz0;
	   else
	       z1 = z0;
       }
   }

   if ((wall->bowtie_bits&BT_ABOVE_BOWTIE) != 0) {
       // upper wall is bowtie
       if (wall->bowtie_bits&BT_ABOVE_POS) {
	   if (side >= 0)
	       z2 = z3;
	   else
	       zz2 = zz3;
       }
       else {
	   if (side >= 0)
	       zz2 = zz3;
	   else
	       z2 = z3;
       }
   }

   // transform endpoints of wall and clip to screen
   if (!world_to_screen(wall->x0,wall->y0,wall->x1,wall->y1,&t0,&t1,&col0,&d0,&col1,&d1))
     return;
   
   // if endpoint 0 was clipped, calculate new height at clipped endpoint
   if (t0 != 0) {
       z0 = LERP(z0,zz0,t0);
       z1 = LERP(z1,zz1,t0);
       z2 = LERP(z2,zz2,t0);
       z3 = LERP(z3,zz3,t0);
   }
   
   // if endpoint 1 was clipped, calculate new height at clipped endpoint
   if (t1 != FINENESS) {
       zz0 = LERP(wall->z0,zz0,t1);
       zz1 = LERP(wall->z1,zz1,t1);
       zz2 = LERP(wall->z2,zz2,t1);
       zz3 = LERP(wall->z3,zz3,t1);
   }

   /* put col0 on the left */
   if (col1 < col0)
   {
      long tmp;
      SWAP(col0,col1,tmp);
      SWAP(d0,d1,tmp);
      SWAP(z0,zz0,tmp);
      SWAP(z1,zz1,tmp);
      SWAP(z2,zz2,tmp);
      SWAP(z3,zz3,tmp);
   }

   // This pixel doesn't overlap in D3D.
   if (!D3DRenderIsEnabled() || col1 == MAXX)
      col1--;  /* fix overlap! */
   if (col1 < col0) return;

#if 0
   debug(("WalkWall %d %d\n",col0,col1));
#endif

   // Look for lower wall
   if (sidedef->below_bmap != NULL && ((z1 != z0)||(zz1 != zz0))) {

      toprow0 = horizon + (viewer_height - z1) * VIEWER_DISTANCE / d0;
      toprow1 = horizon + (viewer_height - zz1) * VIEWER_DISTANCE / d1;
      if (toprow0 < MINCLIPROW)
	  toprow0 = MINCLIPROW;
      if (toprow1 < MINCLIPROW)
	  toprow1 = MINCLIPROW;
      if (toprow0 > MAXCLIPROW)
	  toprow0 = MAXCLIPROW;
      if (toprow1 > MAXCLIPROW)
	  toprow1 = MAXCLIPROW;
      
      /* calculate top of wall coefficients */
      if (col0 == col1)
	{
	  a = 1;
	  b = 0;
	  if (toprow0 < toprow1)
	    d = toprow0;
	  else
	    d = toprow1;
	}
      else
	{
	  a = col1 - col0;
	  b = toprow1 - toprow0;
	  d = a * toprow0 - b * col0;
	}
      item_template.type = DrawWallType;
      item_template.u.wall.wall = wall;
      item_template.u.wall.side = SGN(side);
      item_template.u.wall.wall_type = WALL_BELOW;
      wall->drawbelow = TRUE;
      if (add_dn(&item_template, a, b, d, col0, col1))
      {
         wall->seen = True;
      }
   }

   // Look for above wall
   if (sidedef->above_bmap != NULL && ((z3 != z2)||(zz3 != zz2)))
   {
      botrow0 = horizon + (viewer_height - z2) * VIEWER_DISTANCE / d0;
      botrow1 = horizon + (viewer_height - zz2) * VIEWER_DISTANCE / d1;
      if (botrow0 < MINCLIPROW)
	  botrow0 = MINCLIPROW;
      if (botrow1 < MINCLIPROW)
	  botrow1 = MINCLIPROW;
      if (botrow0 > MAXCLIPROW)
	  botrow0 = MAXCLIPROW;
      if (botrow1 > MAXCLIPROW)
	  botrow1 = MAXCLIPROW;
      
      /* calculate bottom of wall coefficients */
      if (col0 == col1)
	{
	  a = 1;
	  b = 0;
	  if (botrow0 > botrow1)
	    d = botrow0;
	  else
	    d = botrow1;
	}
      else
	{
	  a = col1 - col0;
	  b = botrow1 - botrow0;
	  d = a * botrow0 - b * col0;
	}
      item_template.type = DrawWallType;
      item_template.u.wall.wall = wall;
      item_template.u.wall.side = SGN(side);
      item_template.u.wall.wall_type = WALL_ABOVE;
      wall->drawabove = TRUE;
      if (add_up(&item_template, a, b, d, col0, col1))
      {
         wall->seen = True;
      }
   }
   
   // Look for normal wall
   if (sidedef->normal_bmap != NULL && ((z2 > z1)||(zz2 > zz1)))
   {
     /* wall completes a column, so we don't need to calculate
      * its vertical extent.
      */
      // The following code is not accurate enough to calculate correctly
      // whether we should draw the wall or not, so if we've got this far,
      // flag it for drawing.
      wall->drawnormal = TRUE;
     blakassert(col1 < MAXX);
     for(c = search_for_first(col0); c->cone.leftedge <= col1; c = next)
     {
	    next = c->next;
	    wall->seen = True;
	    
	    if (sidedef->flags & WF_TRANSPARENT &&
		 !(sidedef->flags & WF_NOLOOKTHROUGH))
	    {
	       if (nitems >= MAX_ITEMS)
	       {
		  debug(("out of item memory Walk Wall 1\n"));
		  return;
	       }
	       item = &drawdata[nitems++];
	       item->type = DrawWallType;
	       item->u.wall.wall = wall;
	       item->u.wall.side = SGN(side);
	       item->u.wall.wall_type = WALL_NORMAL;
	       item->cone = c->cone;
	       if (col0 > item->cone.leftedge)
		  item->cone.leftedge = col0;
	       if (col1 < item->cone.rightedge)
		  item->cone.rightedge = col1;
	       continue;
	    }
	    
	    if (col0 > c->cone.leftedge)
	      c = split_cone(c, col0-1, False);
	    if (col1 < c->cone.rightedge)
	      c = split_cone(c, col1, True);
	    
	    /* add wall to draw list */
	    if (nitems >= MAX_ITEMS)
	    {
	       debug(("out of item memory Walk Wall 2\n"));
	       return;
	    }
	    item = &drawdata[nitems++];
	    item->type = DrawWallType;
	    item->u.wall.wall = wall;
	    item->u.wall.side = SGN(side);
	    item->u.wall.wall_type = WALL_NORMAL;
	    item->cone = c->cone;
	    
	    /* delete cone */
	    delete_cone(c);
	 }
   }
}
/*****************************************************************************/
static void WalkObjects(ObjectData *objects)
{
   ObjectData *object;
   long num,i,j;
   ObjectData *sort[MAX_OBJS_PER_LEAF];
   long x,y,a,b;
   long left,right;
   ConeTreeNode *c;
   DrawItem *item;
   room_contents_node *r;
   
   num = 0;
   for(object=objects; object; object=object->next)
   {
      sort[num] = object;
      num++;
      if (num == MAX_OBJS_PER_LEAF)
      {
	 debug(("too many objects per leaf: throwing some away\n"));
	 break;
      }
   }

   /* simple bubble sort, closest first */
   for(i=0; i<num-1; i++)
      for(j=i+1; j<num; j++)
	 if (sort[i]->draw.distance > sort[j]->draw.distance)
	 {
	    ObjectData *tmp = sort[i];
	    sort[i] = sort[j];
	    sort[j] = tmp;
	 }

   for(i=0; i<num; i++)
   {
      object = sort[i];
      
      x = object->x0 - viewer_x;
      y = object->y0 - viewer_y;
      a = (left_a * x + left_b * y) >> (FIX_DECIMAL - 6);
      b = (right_a * x + right_b * y) >> (FIX_DECIMAL - 6);
      if (a + b <= 0)
      {
	 debug(("a+b <= 0! (WalkObjects(1)) %ld\n",a+b));
	 continue;
      }
      left = (a * screen_width + screen_width2) / (a + b);
      
      x = object->x1 - viewer_x;
      y = object->y1 - viewer_y;
      a = (left_a * x + left_b * y) >> (FIX_DECIMAL - 6);
      b = (right_a * x + right_b * y) >> (FIX_DECIMAL - 6);
      if (a + b <= 0)
      {
	 debug(("a+b <= 0! (WalkObjects(2)) %ld\n",a+b));
	 continue;
      }
      right = (a * screen_width + screen_width2) / (a + b);
      
      right--;  /* fix overlap */
      
      if (right >= MAXX)
	right = MAXX-1;
      for(c = search_for_first(left); c->cone.leftedge <= right; c = c->next)
      {
	if (nitems >= MAX_ITEMS)
	  {	
	    debug(("out of item memory Walk Wall 3\n"));
	    return;
	  }
	item = &drawdata[nitems++];
	item->type = DrawObjectType;
	item->u.object.object = object;
	
	// Mark room objects (not projectiles) as visible
	if (object->draw.obj != NULL)
	{
	   r = (room_contents_node *) object->draw.obj;
	   r->visible = True;
	}

	item->cone = c->cone;
	if (left > item->cone.leftedge)
	  item->cone.leftedge = left;
	if (right < item->cone.rightedge)
	  item->cone.rightedge = right;
	
	*(item->u.object.object->ncones_ptr) += 1;
      }
   }
}

/***************************************************************************
 *
 * EyeAboveFloor
 *  used by walk leaf to determine if the floor should be handled. Takes into
 * account sloped floors
 *
 */
Bool EyeAboveFloor(Sector *sector) {
    if (sector->sloped_floor != NULL) {
	Plane3D	*plane = &sector->sloped_floor->plane;

	return ((plane->a*viewer_x + plane->b*viewer_y + plane->c*viewer_height + plane->d) >= 0);
    }
    else
	return (viewer_height > sector->floor_height);
}
/***************************************************************************
 *
 * EyeBelowCeiling
 *  used by walk leaf to determine if the floor should be handled. Takes into
 * account sloped floors
 *
 */
Bool EyeBelowCeiling(Sector *sector) {
    if (sector->sloped_ceiling != NULL) {
	Plane3D	*plane = &sector->sloped_ceiling->plane;

	return ((plane->a*viewer_x + plane->b*viewer_y + plane->c*viewer_height + plane->d) >= 0);
    }
    else
	return (viewer_height < sector->ceiling_height);
}

/*****************************************************************************/
static void WalkLeaf(BSPnode *tree)
{
   long i;
   long col0,col1;
   float d0, d1;
   long row0,row1;
   DrawItem item_template;
   long a,b,d;
   float x0,y0,x1,y1;
   BSPleaf *leaf = &tree->u.leaf;
   long t0,t1,height,height0,height1;
   SlopeData *sloped_floor = leaf->sector->sloped_floor;  // only a tiny bit faster but much easier to read
   SlopeData *sloped_ceiling = leaf->sector->sloped_ceiling;
   Bool process_floor,process_ceiling;
   Vector3D surface_norm;
   long lightscale;

   // These booleans are used in d3drender.c to decide whether to add the floor
   // or ceiling to the the rendering pipeline. If these are modified or removed,
   // d3drender.c needs to be modified also.
   tree->drawfloor = FALSE;
   tree->drawceiling = FALSE;
#if 0
   debug(("WalkLeaf"));
   for(i=0; i<leaf->poly.npts; i++)
     debug((" %d %d", leaf->poly.p[i].x, leaf->poly.p[i].y));
   debug(("\n"));
#endif
   
   if (leaf->objects)
      WalkObjects(leaf->objects);
   
   // check if floor and ceiling are visible
   process_floor = EyeAboveFloor(leaf->sector);
   process_ceiling = EyeBelowCeiling(leaf->sector);

   // If directional lighting is in effect, check to see if we have a sloped floor
   //  or ceiling that needs directional lighting calculated.
   if (shade_amount!=0) {

       // floor is sloped, and it's marked as being steep enough to be eligible for
       // directional lighting
       if ((sloped_floor != NULL)&&process_floor&&(sloped_floor->flags & SLF_DIRECTIONAL)) {
	   long lo_end = FINENESS-shade_amount;

	   surface_norm.x = sloped_floor->plane.a;
	   surface_norm.y = sloped_floor->plane.b;
	   surface_norm.z = sloped_floor->plane.c;

	   // light scale is based on dot product of surface normal and sun vector
	   lightscale = (long)(surface_norm.x*sun_vect.x + surface_norm.y*sun_vect.y + surface_norm.z*sun_vect.z)>>LOG_FINENESS;

#if PERPENDICULAR_DARK
	   lightscale = ABS(lightscale);
#else
	   lightscale = (long)(lightscale + FINENESS)>>1; // map to 0 to 1 range
#endif

	   lightscale = lo_end + (long)((lightscale * shade_amount)>>LOG_FINENESS);

	   if (lightscale > FINENESS)
	       lightscale = FINENESS;
	   else if ( lightscale < 0)
	       lightscale = 0;

	   sloped_floor->lightscale = lightscale;
       }

       // ceiling is sloped, and it's marked as being steep enough to be eligible for
       // directional lighting then calculate a light scaling factor for this sector 
       if ((sloped_ceiling != NULL)&&process_ceiling&&(sloped_ceiling->flags & SLF_DIRECTIONAL)) {
	   long lo_end = FINENESS-shade_amount;

	   surface_norm.x = sloped_ceiling->plane.a;
	   surface_norm.y = sloped_ceiling->plane.b;
	   surface_norm.z = sloped_ceiling->plane.c;

	   // light scale is based on dot product of surface normal and sun vector
	   lightscale = (long)(surface_norm.x*sun_vect.x + surface_norm.y*sun_vect.y + surface_norm.z*sun_vect.z)>>LOG_FINENESS;

#if PERPENDICULAR_DARK
	   lightscale = ABS(lightscale);
#else
	   lightscale = (lightscale + FINENESS)>>1; // map to 0 to 1 range
#endif

	   lightscale = lo_end + ((lightscale * shade_amount)>>LOG_FINENESS);

	   if (lightscale > FINENESS)
	       lightscale = FINENESS;
	   else if ( lightscale < 0)
	       lightscale = 0;

	   sloped_ceiling->lightscale = lightscale;
       }
   }
   else { // normal light intensity
       if (sloped_floor != NULL)
	   sloped_floor->lightscale = FINENESS;
       if (sloped_ceiling != NULL)
	   sloped_ceiling->lightscale = FINENESS;
   }


   /* find close and far walls */
   for(i=0; i<leaf->poly.npts; i++)
     {
       if (!world_to_screen(x0 = leaf->poly.p[i].x, y0 = leaf->poly.p[i].y,
			   x1 = leaf->poly.p[i+1].x, y1 = leaf->poly.p[i+1].y,
			   &t0,&t1,&col0, &d0, &col1, &d1))
	 continue;
       
       // This probably needs looking at - why take one here? The other instance
       // this was done caused lines in walls in D3D renderer.
       //if (!D3DRenderIsEnabled())
         col1--;
       if (col1 < col0)
	 continue;
       
       if (process_floor)
	 {
	   // get height at each endpoint of wall
	   // might want to store these in leaf struct to avoid recalculating all the time
	   height0 = GetFloorHeight(x0, y0, leaf->sector);
	   height1 = GetFloorHeight(x1, y1, leaf->sector);

	   // if endpoint 0 was clipped, calculate height at new endpoint
	   if (t0 != 0)
	       height = LERP(height0,height1,t0);
	   else
	       height = height0;

	   // calculate screen row of endpoint 0
	   row0 = horizon + (viewer_height - height) * VIEWER_DISTANCE / d0;
	   
	   // if endpoint 1 was clipped, calculate height at new endpoint
	   if (t1 != FINENESS)
	       height = LERP(height0,height1,t1);
	   else
	       height = height1;

	   // calculate screen row of endpoint 1
	   row1 = horizon + (viewer_height - height) * VIEWER_DISTANCE / d1;

	   if ((row0 <= horizon) && (sloped_floor==NULL)) // normal floors shouldn't cross horizon - 4 slopes it's ok
	       row0 = horizon + 1;
	   if ((row1 <= horizon) && (sloped_floor==NULL)) 
	       row1 = horizon + 1;
	   if (row0 > MAXCLIPROW) row0 = MAXCLIPROW;
	   if (row1 > MAXCLIPROW) row1 = MAXCLIPROW;
	   
	   // generate cone parameters for region below wall top
	   if (col0 == col1)
	     {
	       a = 1;
	       b = 0;
	       d = min(row0, row1);
	     }
	   else
	     {
	       a = col1 - col0;
	       b = row1 - row0;
	       d = a * row0 - b * col0;
	     }
	   
	   if ((sloped_floor!=NULL) && (leaf->sector->floor!=NULL)) {
	       item_template.type = DrawSlopedFloorType;
	       item_template.u.slope.leaf = leaf;
	   } else if (leaf->sector->floor)
	     {
	       item_template.type = DrawFloorType;
	       item_template.u.floor.leaf = leaf;
	     }
	   else
	     item_template.type = DrawBackgroundType;
	   
      add_dn(&item_template, a, b, d, col0, col1);
      tree->drawfloor = TRUE;
	 }
       
       if (process_ceiling)
	 {
	   // get height at each endpoint of wall
	   height0 = GetCeilingHeight(x0, y0, leaf->sector);
	   height1 = GetCeilingHeight(x1, y1, leaf->sector);

	   // if endpoint 0 was clipped, calculate height at new endpoint
	   if (t0 != 0)
	       height = LERP(height0,height1,t0);
	   else
	       height = height0;

	   // calculate screen row of endpoint 0
	   row0 = horizon + (viewer_height - height) * VIEWER_DISTANCE / d0;
	   
	   // if endpoint 1 was clipped, calculate height at new endpoint
	   if (t1 != FINENESS)
	       height = LERP(height0,height1,t1);
	   else
	       height = height1;

	   // calculate screen row of endpoint 1
	   row1 = horizon + (viewer_height - height) * VIEWER_DISTANCE / d1;

	   if ((row0 >= horizon) && (sloped_ceiling==NULL))
	       row0 = horizon - 1;
	   if ((row1 >= horizon) && (sloped_ceiling==NULL))
	       row1 = horizon - 1;
	   if (row0 < MINCLIPROW) row0 = MINCLIPROW;
	   if (row1 < MINCLIPROW) row1 = MINCLIPROW;
	   
	   // generate cone parameters for region above wall bottom
	   if (col0 == col1)
	     {
	       a = 1;
	       b = 0;
	       d = min(row0, row1);
	     }
	   else
	     {
	       a = col1 - col0;
	       b = row1 - row0;
	       d = a * row0 - b * col0;
	     }
	   
	   if ((sloped_ceiling!=NULL) && (leaf->sector->ceiling!=NULL)) {
	       item_template.type = DrawSlopedCeilingType;
	       item_template.u.slope.leaf = leaf;
	   } else if (leaf->sector->ceiling!=NULL)
	     {
	       item_template.type = DrawCeilingType;
	       item_template.u.ceiling.leaf = leaf;
	     }
	   else
	     item_template.type = DrawBackgroundType;
	   
      add_up(&item_template, a, b, d, col0, col1);
      tree->drawceiling = TRUE;
	 }
     }
}
/*****************************************************************************/
// this controls where darkest point is relative to sun_vect
// when non-zero walls are darkest when perpendicular to sun vect
// and light when facing directly towards or directly away from sun_vect (obsolete)
// when zero, wall are lightest when facing towards sun_vect and
// darkest when facing directly away
#define PERPENDICULAR_DARK  0

static void WalkBSPtree(BSPnode *tree)
{
   long side;
   long a,b,lightscale;
   
   if (!tree)
      return;

   /* don't open box if not in view */
   if (Bbox_shadowed(tree->bbox.x0,tree->bbox.y0,tree->bbox.x1,tree->bbox.y1))
   {
      /*
	 debug(("box (%ld %ld) (%ld %ld)) shadowed\n",tree->bbox.x0, tree->bbox.y0,
	 tree->bbox.x1, tree->bbox.y1);
	 */
      return;
   }
   /* debug(("box (%ld %ld) (%ld %ld)) opened\n",tree->bbox.x0, tree->bbox.y0,
      tree->bbox.x1, tree->bbox.y1); */
   
   switch(tree->type)
   {
   case BSPleaftype:
      WalkLeaf(tree);
      return;
      
   case BSPinternaltype:
      side = (a = tree->u.internal.separator.a) * viewer_x + 
	 (b = tree->u.internal.separator.b) * viewer_y +
	    tree->u.internal.separator.c;
      
      if (shade_amount!=0) {
	  long lo_end = FINENESS-shade_amount;

	  if (side < 0) {
	      a = -a;
	      b = -b;
	  }

	  lightscale = (long)(a*sun_vect.x + b*sun_vect.y)>>LOG_FINENESS;

#if PERPENDICULAR_DARK
	  lightscale = ABS(lightscale);
#else
	  lightscale = (long)(lightscale + FINENESS)>>1; // map to 0 to 1 range
#endif

	  lightscale = lo_end + ((lightscale * shade_amount)>>LOG_FINENESS);
	
	  if (lightscale > FINENESS)
	      lightscale = FINENESS;
	  else if ( lightscale < 0)
	      lightscale = 0;
      }
      else
	  lightscale = FINENESS;


      /* first, traverse closer side */
      if (side > 0)
	 WalkBSPtree(tree->u.internal.pos_side);
      else
	 WalkBSPtree(tree->u.internal.neg_side);
      
      /* if entire screen is covered, we're done */
      if (cone_tree_root == NULL)
	 return;
      
      /* then do walls on the separator */
      if (side != 0)
      {
	 WallList list;
	 //long     d,c = tree->u.internal.separator.c;
	 
	 for(list = tree->u.internal.walls_in_plane; list; list = list->next)
	 {
	    WalkWall(list, side);

	    list->lightscale = lightscale;

	    if (cone_tree_root == NULL)
	       return;
	 }
      }
      
      /* lastly, traverse farther side */
      if (side > 0)
	 WalkBSPtree(tree->u.internal.neg_side);
      else
	 WalkBSPtree(tree->u.internal.pos_side);
      
      return;
   default:
      debug(("WalkBSPtree error!\n"));
      return;
   }
}

void MinimapUpdate(Draw3DParams *params, BSPnode *tree)
{
/*	viewer_x = params->viewer_x;
	viewer_y = params->viewer_y;
	viewer_angle = params->viewer_angle;
	viewer_height = params->viewer_height;
	area.cx = min(params->width  / params->stretchfactor, MAXX);
	area.cy = min(params->height / params->stretchfactor, MAXY);

	// Force size to be even
	area.cy = area.cy & ~1;  
	area.cx = area.cx & ~1;
	screen_width = area.cx;
	screen_width2 = area.cx / 2;

	// find equations that bound viewing frustum
	// keep 10 = FIX_DECIMAL-6 bits of accuracy
	center_a = COS(viewer_angle) >> 6;
	center_b = SIN(viewer_angle) >> 6;

	left_a = -center_b + ((center_a * screen_width2) >> LOG_VIEWER_DISTANCE);
	left_b =  center_a + ((center_b * screen_width2) >> LOG_VIEWER_DISTANCE);

	right_a =  center_b + ((center_a * screen_width2) >> LOG_VIEWER_DISTANCE);
	right_b = -center_a + ((center_b * screen_width2) >> LOG_VIEWER_DISTANCE);

	horizon = area.cy / 2 + PlayerGetHeightOffset();

	init_cone_tree();
	WalkBSPtree(tree);*/
}

/*****************************************************************
 * Routines to draw object list, back to front.  One routine for
 * each object type, plus the iterator DrawItems.
 *****************************************************************/

/* for debugging, draw wall cones as blocks instead of texture-mapped */
#define DRAW_WALL_CONES 0
/* for debugging, draw leaf cones as blocks instead of texture-mapped */
#define DRAW_LEAF_CONES 0
/* for debugging, draw object cones as blocks instead of texture-mapped */
#define DRAW_OBJ_CONES 0
/* for debugging, draw all cone outlines */
#define DRAW_CONE_OUTLINES 0

static char fillcolor;
static void fillcone(ViewCone *c)
{
  int row,col;
  int minrow,maxrow;
  
  for(col=c->leftedge; col<=c->rightedge; col++)
    {
      minrow = DIVUP(c->top_b * col + c->top_d, c->top_a);
      maxrow = DIVDOWN(c->bot_b * col + c->bot_d, c->bot_a);
      
      if (minrow < 0)
	minrow = 0;
      if (maxrow >= area.cy)
	maxrow = area.cy-1;
      
      for(row = minrow; row <= maxrow; row++)
	*(gBits + row*MAXX + col) = fillcolor;
    }
  fillcolor+=15;
}
/*****************************************************************************/
static void outlinecone(ViewCone *c)
{
  int row,col;
  int minrow,maxrow;
#if 0
  /* hunting down a bug... */
  minrow = DIVUP(c->top_b * c->rightedge + c->top_d, c->top_a);
  if (minrow >= 0) return;
#endif  
  if (c->leftedge < 0 || c->rightedge >= screen_width || c->leftedge > c->rightedge)
    {
      debug(("bad left/right in outlinecone! %d %d\n", c->leftedge, c->rightedge));
      return;
    }
  
  for(col=c->leftedge; col<=c->rightedge; col++)
    {
      minrow = DIVUP(c->top_b * col + c->top_d, c->top_a);
      maxrow = DIVDOWN(c->bot_b * col + c->bot_d, c->bot_a);
      
      if (minrow > maxrow)
	continue;
      
      if (minrow >= 0 && minrow < area.cy)
	*(gBits + minrow*MAXX + col) = fillcolor;
      if (maxrow >= 0 && maxrow < area.cy)
	*(gBits + maxrow*MAXX + col) = fillcolor;
    }
  
  minrow = DIVUP(c->top_b * c->leftedge + c->top_d, c->top_a);
  maxrow = DIVDOWN(c->bot_b * c->leftedge + c->bot_d, c->bot_a);
  if (minrow < 0) debug(("minrow left (%d %d): %d\n", c->leftedge, c->rightedge, minrow));
  if (maxrow >= area.cy) debug(("maxrow left (%d %d): %d (%d)\n", c->leftedge, c->rightedge, maxrow, area.cy));
  for(row = max(0,minrow); row <= min(area.cy-1,maxrow); row++)
    *(gBits + row*MAXX + c->leftedge) = fillcolor;
  
  minrow = DIVUP(c->top_b * c->rightedge + c->top_d, c->top_a);
  maxrow = DIVDOWN(c->bot_b * c->rightedge + c->bot_d, c->bot_a);
  if (minrow < 0) debug(("minrow right (%d %d): %d\n", c->leftedge, c->rightedge, minrow));

  if (minrow < 0) debug(("minrow right %d (%d %d), %d (%d %d)\n", c->leftedge, DIVUP(c->top_b * c->leftedge + c->top_d, c->top_a), DIVDOWN(c->bot_b * c->leftedge + c->bot_d, c->bot_a), c->rightedge, minrow, maxrow));
  if (maxrow >= area.cy) debug(("maxrow right (%d %d): %d (%d)\n", c->leftedge, c->rightedge, maxrow, area.cy));
  for(row = max(0,minrow); row <= min(area.cy-1,maxrow); row++)
    *(gBits + row*MAXX + c->rightedge) = fillcolor;

  fillcolor+=15;
}
/*****************************************************************************/
void doDrawWall(DrawWallStruct *wall, ViewCone *c)
{
   long col;
   long rowstart,rowend;
   long clipstart,clipend;
   long textpos;
   long a,b;
   float d0,d1;
   long xoffset, yoffset;
   WallData *BSPwall = wall->wall;
   PDIB bmap;
   BYTE *bits, light;
   int backwards, transparent, bitmap_height, bitmap_width;
   float x0, y0, x1, y1;
   long  num_steps, total_steps;
   float d, f;
   float z0, z1;
   float oldf = 0.0;
   BOOL hasMipMaps = FALSE;
   grid_bitmap_type grid = NULL;
   Animate *pAnim = NULL;
   int group = 0;
   
   long yinc,ytex;
   BYTE *palette;
   BYTE *screen_ptr, *end_screen_ptr;
   BYTE *square_base_ptr;
   int length, top, bottom;
   Sidedef *sidedef;
   Bool top_down;
   Bool no_vtile;  // True when wall texture doesn't tile vertically
   bixlat* pBiXlat = NULL;

#if DRAW_WALL_CONES
   fillcone(c);
   return;
#endif
   
   if (wall->side > 0)
   {
      sidedef = BSPwall->pos_sidedef;
      x0 = BSPwall->x0 - viewer_x;
      y0 = BSPwall->y0 - viewer_y;
      x1 = BSPwall->x1 - viewer_x;
      y1 = BSPwall->y1 - viewer_y;
      xoffset = BSPwall->pos_xoffset;
      yoffset = BSPwall->pos_yoffset;

      if (BSPwall->pos_sector == NULL)
	 light = 0;
      else light = BSPwall->pos_sector->light;
   }
   else
   {
      sidedef = BSPwall->neg_sidedef;
      x0 = BSPwall->x1 - viewer_x;
      y0 = BSPwall->y1 - viewer_y;
      x1 = BSPwall->x0 - viewer_x;
      y1 = BSPwall->y0 - viewer_y;
      xoffset = BSPwall->neg_xoffset;
      yoffset = BSPwall->neg_yoffset;

      if (BSPwall->neg_sector == NULL)
	 light = 0;
      else light = BSPwall->neg_sector->light;
   }

   if (sidedef->animate)
   {
      pAnim = &sidedef->animate->u.bitmap.a;
      group = (int)pAnim->group;
   }

   backwards = sidedef->flags & WF_BACKWARDS;
   transparent = sidedef->flags & WF_TRANSPARENT;
   if ((sidedef->flags & WF_NOLOOKTHROUGH) && incremental_background)
      doDrawBackground(c);

   // For scrolling textures, add texture offsets
   if (sidedef->animate != NULL && sidedef->animate->animation == ANIMATE_SCROLL)
   {
      if (backwards)
      {
	 xoffset -= sidedef->animate->u.scroll.xoffset;
	 yoffset -= sidedef->animate->u.scroll.yoffset;
      }
      else
      {
	 xoffset += sidedef->animate->u.scroll.xoffset;
	 yoffset += sidedef->animate->u.scroll.yoffset;
      }
   }

   //REVIEW: Currently, if we have *any* transparent parts, we're
   //        drawing the background to the whole area, and
   //        using the slower transparent drawing support for upper, normal *and* lower.

   no_vtile = False;
   switch (wall->wall_type)
   {
   case WALL_ABOVE:
      bmap = sidedef->above_bmap;
      grid = GetGridBitmap(sidedef->above_type);
      if (!grid) { debug(("Unable to draw texture %d (overflown cache?)\n", sidedef->above_type)); return ; }

      if (BSPwall->z3 == BSPwall->zz3)
      {
	  top = BSPwall->z3;
      }
      else
      {   // top of wall is sloped, choose abitrary texture endpoint
	  // CA> this is just a hack to fool the hack below that calculates
	  //  yinc & ytex into doing the right thing. Top & bottom really
	  //  only serve as reference points for vertical texture offsets.
	  //  when a wall is sloped, it chooses the nearest multiple of
	  //  FINENESS above the wall for the 'top' value or the nearest
	  //  multiple below the wall for the bottom value. This greatly
	  //  simplifies texture alignment for sloped walls. All the wall
	  //  segments adjoining a sloped surface all have (effectively)
	  //  the same vertical origin for their texture alignments. This
	  //  allows their testures to be aligned automatically with the
	  //  align texture X command in the editor instead of requiring
	  //  the level designer to align each wall vertically by hand.

	  // This is the last piece of code I wrote as an employee of 3DO
	  // 3-24-1997 Colin Andrews
	  top = max(BSPwall->z3,BSPwall->zz3);
	  top = (top + FINENESS-1) & ~(FINENESS-1);
      }
      
      if (BSPwall->z2 == BSPwall->zz2)
	  bottom = BSPwall->z2;
      else
      {   // bottom of wall is sloped, choose abitrary texture endpoint
	  bottom = min(BSPwall->z2,BSPwall->zz2);
	  bottom = bottom & ~(FINENESS-1);
      }

      top_down = !(sidedef->flags & WF_ABOVE_BOTTOMUP);
      break;

   case WALL_BELOW:
      bmap = sidedef->below_bmap;
      grid = GetGridBitmap(sidedef->below_type);
      if (!grid) { debug(("Unable to draw texture %d (overflown cache?)\n", sidedef->above_type)); return ; }

      if (BSPwall->z1 == BSPwall->zz1)
	  top = BSPwall->z1;
      else
      {   // top of wall is sloped, choose abitrary texture endpoint
	  top = max(BSPwall->z1,BSPwall->zz1);
	  top = (top + FINENESS-1) & ~(FINENESS-1);
      }
      
      if (BSPwall->z0 == BSPwall->zz0)
	  bottom = BSPwall->z0;
      else
      {   // bottom of wall is sloped, choose abitrary texture endpoint
	  bottom = min(BSPwall->z0,BSPwall->zz0);
	  bottom = bottom & ~(FINENESS-1);
      }

      top_down = ((sidedef->flags & WF_BELOW_TOPDOWN) != 0);
      break;

   case WALL_NORMAL:
      bmap = sidedef->normal_bmap;
      grid = GetGridBitmap(sidedef->normal_type);
      if (!grid) { debug(("Unable to draw texture %d (overflown cache?)\n", sidedef->above_type)); return ; }

      if (BSPwall->z2 == BSPwall->zz2)
	  top = BSPwall->z2;
      else
      {   // top of wall is sloped, choose abitrary texture endpoint
	  top = max(BSPwall->z2,BSPwall->zz2);
	  top = (top + FINENESS-1) & ~(FINENESS-1);
      }
      
      if (BSPwall->z1 == BSPwall->zz1)
	  bottom = BSPwall->z1;
      else
      {   // bottom of wall is sloped, choose abitrary texture endpoint
	  bottom = min(BSPwall->z1,BSPwall->zz1);
	  bottom = bottom & ~(FINENESS-1);
      }

      top_down = ((sidedef->flags & WF_NORMAL_TOPDOWN) != 0);

      if (transparent && (sidedef->flags & WF_NO_VTILE))
	 no_vtile = True;

      break;

   default:
      debug(("doDrawWall got bad wall type %d\n", wall->wall_type));
      return;
   }

   if (/*wallflagtranslucent*/FALSE)
   {
      pBiXlat = FindStandardBiXlat(BIXLAT_BLEND50);
   }

   d0 = GetDistance(x0,y0);
   d1 = GetDistance(x1,y1);
   
   xoffset *= (int) DibShrinkFactor(bmap);
   yoffset *= (int) DibShrinkFactor(bmap);

   bits = DibPtr(bmap);
   bitmap_width  = DibWidth(bmap);   // Remember that wall bitmaps are turned on their sides
   bitmap_height = DibHeight(bmap);

   /*
      Determine if this is a mipmap wall - it has multiple frames in the group with
      different widths.
   */
   group = group % BitmapsNumGroups(grid->bmaps);
   if (BitmapsInGroup(grid->bmaps,group) > 1)
   {
      PDIB pBmp1 = BitmapsGetPdib(grid->bmaps,group,0);
      PDIB pBmp2 = BitmapsGetPdib(grid->bmaps,group,1);
      hasMipMaps = (pBmp1->height != pBmp2->height);
   }

   for(col = c->leftedge; col <= c->rightedge; col++)
   {
      long horzDownScale = 0;
      long xGraphicOffset = xoffset;
      clipstart = DIVUP(c->top_b * col + c->top_d, c->top_a);
      clipend = DIVDOWN(c->bot_b * col + c->bot_d, c->bot_a);
      
      if (clipstart < 0)
	{
	  /* debug(("small clipstart: %d\n", clipstart)); */
	  clipstart = 0;
	}
      if (clipend >= area.cy)
	{
	  /* debug(("large clipend: %d (%d)\n", clipend, (int)area.cy)); */
	  clipend = area.cy-1;
	}
      
      if (clipstart > clipend)
	continue;
      
      /* find the equation ax + by = 0 of the column ray */
      a = -center_b - ((center_a * (col - screen_width2)) >> LOG_VIEWER_DISTANCE);
      b =  center_a - ((center_b * (col - screen_width2)) >> LOG_VIEWER_DISTANCE);
      
      /* compute fraction along wall, in fixed point */
      z0 = (a * x0 + b * y0) / FINENESS;
      z1 = (a * x1 + b * y1) / FINENESS;
      
      if (z0 > 0)
	 z0 = 0.0f;
      if (z1 <= 0)
	 z1 = 1.0f;

      f = ((-z0) * (FINENESS * 4)) / (z1 - z0);
      if (f < oldf)
	f = oldf;
      oldf = f;
      
      // Compute extent of wall on screen
      d = d0 + (((d1-d0)*f) / (FINENESS * 4));
      if (d <= 1.0)
	d = 1.0f;

      rowstart = horizon + ((viewer_height - top) << LOG_VIEWER_DISTANCE) / d;
      rowend = horizon + ((viewer_height - bottom) << LOG_VIEWER_DISTANCE) / d;
      
      /* extend wall to cover entire cone - ensures there's no holes! */
      if (rowstart > clipstart)
	 rowstart = clipstart;
      if (rowend < clipend)
	 rowend = clipend;
      
      /* get palette */
      palette = GetLightPalette(d, light,BSPwall->lightscale,0);
      
      // Tile wall bitmap vertically -- BITMAP_HEIGHT bitmap is FINENESS tall
      if (rowend > rowstart)
	 yinc = ((BITMAP_HEIGHT << FIX_DECIMAL) - 1) / (rowend-rowstart);
      else
	 yinc = 0;
      
      // should shift by LOG_FINENESS
      yinc = yinc * (top - bottom) / FINENESS * DibShrinkFactor(bmap);

      screen_ptr = gBits + col + clipend*MAXX;

      /*  PICK OUT GRAPHIC TO DISPLAY */
      if (hasMipMaps)
      {
	 int lastFrame = (int)BitmapsInGroup(grid->bmaps,group) - 1;
	 int frame = 0;
	 int numGroups = BitmapsNumGroups(grid->bmaps);
	 int groupNum = group % numGroups;
	 int renderHeight = clipend - clipstart + 1;

	 bmap = BitmapsGetPdib(grid->bmaps,groupNum,frame);
	 while ((frame < lastFrame) && (yinc >= (2 << FIX_DECIMAL)))
	 {
	    frame++;
	    bmap = BitmapsGetPdib(grid->bmaps,groupNum,frame);
	    horzDownScale++;
	    yinc >>= 1;
	 }
	 bits = DibPtr(bmap);
	 bitmap_width  = DibWidth(bmap); // remember that wall bitmaps are turned on their sides
	 bitmap_height = DibHeight(bmap);
      }

      // Align texture bottom-up or top-down
      if (top_down)
	 ytex = yinc * (rowstart - clipend) + (yoffset << FIX_DECIMAL);
      else 
	 ytex = yinc * (rowend - clipend) + (yoffset << FIX_DECIMAL);

      // Compute starting point in textures (use texture offset)
      length = BSPwall->length * DibShrinkFactor(bmap);
      if (horzDownScale > 0)
      {
	 length >>= horzDownScale;
	 xGraphicOffset >>= horzDownScale;
      }
      if (backwards)
	 textpos = (long)(((( FRACTION-1 - f) * length) / (FINENESS * 4)) + xGraphicOffset)
	    % (bitmap_height);
      else
         textpos = (long)(((f * length) / (FINENESS * 4)) + xGraphicOffset)
	    % (bitmap_height);
      
      // Make textpos positive
      if (textpos < 0)
	 textpos += bitmap_height;

      // Go to end of row; 
      square_base_ptr = bits + textpos * bitmap_width + bitmap_width - 1;

      total_steps = clipend - clipstart + 1;
      
      // If this wall doesn't tile vertically, stop at top of texture
      if (no_vtile && yinc != 0)
      {	 
	 int temp;

	 temp = yinc * (rowend - clipend) + (yoffset << FIX_DECIMAL);
	 total_steps = min(total_steps, DIVUP((bitmap_width << FIX_DECIMAL) - temp, yinc));
      }

      // Make sure ytex >= 0
      if (ytex < 0)
	 ytex = ytex - (ytex / (bitmap_width << FIX_DECIMAL) - 1) * (bitmap_width << FIX_DECIMAL);
      
      while (total_steps > 0)
      {
	 // should really try to avoid modulus here
	 ytex %= bitmap_width << FIX_DECIMAL;
	 
	 // Draw until we reach the top of the wall on the screen, or the top of the bitmap
	 if (yinc)
	 {
	    num_steps = min(total_steps, DIVUP((bitmap_width << FIX_DECIMAL) - ytex, yinc));
	    num_steps = max(1, num_steps);
	 }
	 else
	    num_steps = total_steps;
	 
	 // debug(("num_steps = %d, ytex = %d\n", num_steps, ytex));
	 
	 end_screen_ptr = screen_ptr - MAXX * num_steps;
	 if (transparent)
	 {
		if (NULL != pBiXlat)
		{
		    while(screen_ptr > end_screen_ptr)
		    {
		       BYTE val = *(square_base_ptr - (ytex >> FIX_DECIMAL));
		       if (val != TRANSPARENT_INDEX)
			  *screen_ptr = fastBIXLAT(palette[val], *screen_ptr, pBiXlat);
		       
		       screen_ptr -= MAXX;
		       ytex += yinc;
		    }
		}
		else
		{
		    while(screen_ptr > end_screen_ptr)
		    {
		       BYTE val = *(square_base_ptr - (ytex >> FIX_DECIMAL));
		       if (val != TRANSPARENT_INDEX)
			  *screen_ptr = palette[val];
		       
		       screen_ptr -= MAXX;
		       ytex += yinc;
		    }
		}
	 }
	 else
	 {
#if 0
	    __asm
	    {
	       mov   edi, screen_ptr;
	       cmp   edi, end_screen_ptr;
	       jle   END_WHILE_SCREEN_PTR_GT_END_SCREEN_PTR;
	       mov   edx, ytex;
	       mov   esi, DWORD PTR palette;
	       xor   ecx, ecx
WHILE_SCREEN_PTR_GT_END_SCREEN_PTR:
	       mov   eax, edx;
	       mov   ebx, square_base_ptr;
	       shr   eax, 16;
	       sub   ebx, eax;
	       mov   cl, BYTE PTR [ebx];
	       sub   edi, MAXX;
	       add   edx, yinc;
	       mov   al, BYTE PTR [esi + ecx];
	       mov   BYTE PTR [edi + MAXX],al
	       cmp   edi, end_screen_ptr;
	       jg    WHILE_SCREEN_PTR_GT_END_SCREEN_PTR;
	       mov   ytex, edx;
	       mov   screen_ptr, edi;
END_WHILE_SCREEN_PTR_GT_END_SCREEN_PTR:
	    }
#else
	    while(screen_ptr > end_screen_ptr)
	    {
	       *screen_ptr = 
		  palette[*(square_base_ptr - (ytex >> FIX_DECIMAL))];
	       
	       screen_ptr -= MAXX;
	       ytex += yinc;
	    }
#endif
	 }
	 total_steps -= num_steps;
      }
   }
}


/********************************************************************************/
/*  biLerp - bi-linear interpolation
 *    performs inner loop of slope texture mapping using a modified Bresenhams
 *    algorithm for efficiency. Loosly derived from paul Heckbert's line drawing
 *    code in Graphics Gems.
 *
 */
void biLerp(FixedPoint u_1, FixedPoint u_2, FixedPoint v_1, FixedPoint v_2, 
	    long count, long bitmap_width, BYTE *bits, BYTE *screen_ptr, BYTE *palette)
{
   FixedPoint u;
   FixedPoint v;  // intermediate texture coordinates
   FixedPoint du;
   FixedPoint dv;
   FixedPoint au;
   FixedPoint av;
   FixedPoint ax; // discriminator variables for Bresenhams
   FixedPoint su;
   FixedPoint sv; // step values for Bresenhams
   FixedPoint tmp; // temporary variable for assignments
   FixedPoint mask; // for masking u & v to bitmap dimensions
   FixedPoint uround;
   FixedPoint vround; // rounded values of u & v for initializing the descriminators

   mask = bitmap_width-1; // fails utterly if width is not power of 2

   // set up discriminators
   ax = INT_TO_FIXED(count); 
   tmp = u_2 - u_1; 
   au = ABS(tmp);
   if (tmp < 0)
      su = -FIXED_ONE;
   else
      su = FIXED_ONE;

   tmp = v_2-v_1; 
   av = ABS(tmp); 
   if (tmp < 0)
      sv = -FIXED_ONE;
   else
      sv = FIXED_ONE;

   u = u_1;
   v = v_1;

   uround = (u + FIXED_ONE_HALF)&~(FIXED_ONE-1);
   vround = (v + FIXED_ONE_HALF)&~(FIXED_ONE-1);

   // This routine treats the interpolation of u & v as though it is drawing two lines,
   //  one from (0, u_1) to (count, u_2) and the other from (0, v_1) to (count, v_2)
   // Normally, a bresenham's (or midpoint) algorithm steps along the longer axis
   //  of the line being drawn. The algorithm coninues along a row (or column) until
   //  the discriminator values indicate it's time to change to the next row (or column).
   // In this application we only care about the values of u & v at each new pixel along
   //  the span. So, when bresenhams would step along u or v, we drive that variable in
   //  an inner loop, updating the next pixel when the discriminators choose a new column.
   // When the change in u or v is extremely large with respect to count, this inner 
   //  looping will be extremely inefficient. For this reason, this routine should only
   //  be called when the change in u & v are smaller than some experimental constant.
   // There is a different main loop for each four possible cases:
   //  (u2-u1) < count && (v2-v1) < count
   //  (u2-u1) >= count && (v2-v1) < count
   //  (u2-u1) < count && (v2-v1) >= count
   //  (u2-u1) >= count && (v2-v1) >= count
   // Also, since the u & v values here are actually fixed point numbers representing
   //  rational values rather than integers, the initialization of the discriminators is 
   //  different from the usual case in order to gain higher precision. 
   // For a good discussion of the midpoint algorith, check Folley, Van Dam, for the derivation.

   if ((ax > au) && (ax > av))  /* x dominant for both u & v */
   {
      du = au - fpMul(FIXED_ONE_HALF + ((su >= 0) ? uround - u: u-uround),ax);
      dv = av - fpMul(FIXED_ONE_HALF + ((sv >= 0) ? vround - v: v-vround),ax);
      do
      {
	 int vOffset = bitmap_width * (ROUND_FIXED_TO_INT(v) & mask);
	 int uOffset = ROUND_FIXED_TO_INT(u) & mask;
	 int offset = vOffset + uOffset;
	 *screen_ptr++ = palette[bits[offset]];

	 if (du>=0) {
	    u += su;
	    du -= ax;
	 }
	 if (dv>=0) {
	    v += sv;
	    dv -= ax;
	 }
	 du += au;
	 dv += av;
      }
      while (--count);
   }
   else if ((ax > av) && (ax <= au)) /* x dominant over v & u dominant over x */
   {
      du = fpMul(FIXED_ONE + uround - u,ax) - (au >> 1);
      dv = av-fpMul(FIXED_ONE_HALF + ((sv >= 0) ? vround - v : v - vround),ax);
      do
      {
	 int vOffset = bitmap_width * (ROUND_FIXED_TO_INT(v) & mask);
	 int uOffset = ROUND_FIXED_TO_INT(u) & mask;
	 int offset = vOffset + uOffset;
	 *screen_ptr++ = palette[bits[offset]];
	    
	 if (dv>=0) {
	    v += sv;
	    dv -= ax;
	 }
	 while (du < 0) {
	    u += su;
	    du += ax;
	 }
	 du -= au;
	 dv += av;
      }
      while (--count);
   }
   else if ((ax <= av) && (ax > au)) /* x dominant over u & v dominant over x */
   {
      du = au-fpMul(FIXED_ONE_HALF + ((su >= 0) ? uround - u : u - uround),ax);
      dv = fpMul(FIXED_ONE + vround - v,ax) - (av >> 1);
      do
      {
	 int vOffset = bitmap_width * (ROUND_FIXED_TO_INT(v) & mask);
	 int uOffset = ROUND_FIXED_TO_INT(u) & mask;
	 int offset = vOffset + uOffset;
	 *screen_ptr++ = palette[bits[offset]];
	    
	 if (du>=0) 
	 {
	    u += su;
	    du -= ax;
	 }
	 while (dv < 0) 
	 {
	    v += sv;
	    dv += ax;
	 }
	 du += au;
	 dv -= av;
      }
      while (--count);
   }
   else if ((ax <= au) && (ax <= av)) /* u & v both dominant */
   {
      du = fpMul(FIXED_ONE + uround - u,ax) - (au >> 1);
      dv = fpMul(FIXED_ONE + vround - v,ax) - (av >> 1);
      do
      {
	 int vOffset = bitmap_width * (ROUND_FIXED_TO_INT(v) & mask);
	 int uOffset = ROUND_FIXED_TO_INT(u) & mask;
	 int offset = vOffset + uOffset;
	 *screen_ptr++ = palette[bits[offset]];

	 while (du < 0) 
	 {
	    u += su;
	    du += ax;
	 }
	 while (dv < 0) 
	 {
	    v += sv;
	    dv += ax;
	 }
	 du -= au;
	 dv -= av;
      }
      while (--count);
   }
   else
      debug(("didn't handle all cases in biLerp\n"));
}

// tuning value : determines the length of the sub-affine step in
//  the perspective correct texture mapping. Might want to make this a variable to
//  allow the user to set this in the settings dialog, or to change it dynamically based
//  on the time it's taking to draw frames.
#define SLOPE_TEXTURE_STEP (24)

// tuning value : determines when a divide is is used instead of the DDA (See below)
#define MAX_DDA   256

/*****************************************************************************/
/*
 * doDrawSloped: Draw a sloped surface (floor for now, ceiling later).
 *  understands perspective correct texturing
 */
static void doDrawSloped(ViewCone *c, BSPleaf *leaf, BYTE floor) {
    
    long    top_a,top_b,top_d;      // top line of view cone
    long    bot_a,bot_b,bot_d;      // bottom line of view cone
    long    lefttop, righttop;      // top corners of view cone (y)
    long    leftbot, rightbot;      // bottom corners of view cone (y)
    long    left, right;	    // left & right sides of cone (x)
    long    mincol, maxcol;	    // min & max column of surrent row
    long    minrow,maxrow;          // min & max row touched by cone
    long    row,col;                // row & column of current pixel in inner loop
    PDIB    texture;                // texture DIB
    BYTE    *screen_ptr;            // pointer to actual screen pixels
    long    bitmap_width;           // size of texture bitmap
    BYTE    *bits;                  // pointer to actual texture pixels
    BYTE    *palette;               // pallete to use for bitmap based on lighting
    FixedPoint sx,sy;               // coordinates of current pixel in screen coords (origin at center of view)
    SlopeData  *slope;              // slope parameters of the surface
    long       shade;               // shading value for directional lighting
    FixedPoint ox, oy, oz;          // magic vectors for texture mapping: relate x, y, & z to screen coords (see docs)
    FixedPoint hx, hy, hz;          // x = ox + sx*hx + sy*vx , y = oy + sx*hy + sy*vy
    FixedPoint vx, vy, vz;          // z = oz + sx*hz + sy*vz
    FixedPoint x, y, z;             // intermediate values in texture mapping u = x/z & v = y/z
    FixedPoint u, v;                // texture coordinates
    FixedPoint tu,tv;               // texture offsets from sector struct
    long       iu, iv;              // integer versions of texture coordinates used to indicies into texture pixels
    long       len, count;          // used to count length of sub-affine step
    FixedPoint u_1, u_2, du;        // u of each endpoint of sub-affine step and per pixel change
    FixedPoint v_1, v_2, dv;        // v of each endpoint of sub-affine step and per pixel change
    FixedPoint z0, z_du, z_dv;      // express lighting distance from user in terms of u & v: z = z0 + u*z_du + v*z_dv
    FixedPoint light1,light2;       // lighting distance at each endpoint of sub-affine step
    
    // get appropriate slope record and texture
    if (floor != 0) {
	slope = leaf->sector->sloped_floor;
	texture = leaf->sector->floor;
    }
    else {
	slope = leaf->sector->sloped_ceiling;
	texture = leaf->sector->ceiling;
    }

    blakassert((slope != NULL));

      // get local copies of slope parameters
    shade = slope->lightscale;

    ox = slope->o.x;
    oy = slope->o.y;
    oz = slope->o.z;
    
    hx = slope->h.x;
    hy = slope->h.y;
    hz = slope->h.z;
    
    vx = slope->v.x;
    vy = slope->v.y;
    vz = slope->v.z;

    z0 = slope->z0;
    z_du = slope->z_du;
    z_dv = slope->z_dv;

    // get screen boundaries of cone to texture
    left = c->leftedge; right = c->rightedge;
    
    bits = DibPtr(texture);

    bitmap_width  = DibWidth(texture);

    // load texture offsets into local fixed point variables
    tu = leaf->sector->tx<<(FIXED_POINT_PRECISION-4); // texture offsets are store so 16 units = 1 pixel
    tv = leaf->sector->ty<<(FIXED_POINT_PRECISION-4);

    // Look for scrolling texture
    if (leaf->sector->animate != NULL && leaf->sector->animate->animation == ANIMATE_SCROLL)
    {
	if (((leaf->sector->flags & SF_SCROLL_FLOOR) && (floor!=0)) ||
	 ((leaf->sector->flags & SF_SCROLL_CEILING) && (floor==0)))
	{
	tu -= (leaf->sector->animate->u.scroll.xoffset*bitmap_width)>>BASE_DIF; // convertworld units to pixels
	tv += (leaf->sector->animate->u.scroll.yoffset*bitmap_width)>>BASE_DIF;
	}
    }

    top_a = c->top_a; top_b = c->top_b; top_d = c->top_d;
    bot_a = c->bot_a; bot_b = c->bot_b; bot_d = c->bot_d;

    lefttop = DIVUP(top_b * left + top_d, top_a);
    righttop = DIVUP(top_b * right + top_d, top_a);
    
    if (lefttop < righttop)
	minrow = lefttop;
    else
	minrow = righttop;
    
    if (minrow < 0)
	{
	debug(("small minrow in doDrawSloped\n"));
	minrow = 0;
	}

    leftbot = DIVDOWN(bot_b * left + bot_d, bot_a);
    rightbot = DIVDOWN(bot_b * right + bot_d, bot_a);
    
    if (leftbot > rightbot)
	maxrow = leftbot;
    else
	maxrow = rightbot;
    
    if (maxrow >= area.cy)
	{
	debug(("large maxrow in doDrawSloped\n"));
	maxrow = area.cy-1;
	}
    
    for (row = minrow; row <= maxrow; row++) {
	mincol = left;
	maxcol = right;
	
	if (top_b > 0) {
	    /* x <= ay-d / b, b>0 */
	    col = (top_a * row - top_d) / top_b;
	    if (col < maxcol)
		maxcol = col;
	}
	else if (top_b < 0) {
	    /* x >= ay-d / b, b<0 */
	    col = (top_a * row - top_d + top_b + 1) / top_b;
	    if (col > mincol)
		mincol = col;
	}

	if (bot_b > 0) {
	    /* x >= ay-d / b, b>0 */
	    col = (bot_a * row - bot_d + bot_b - 1) / bot_b;
	    if (col > mincol)
		mincol = col;
	}
	else if (bot_b < 0) {
	    /* x <= ay-d / b, b<0 */
	    col = (bot_a * row - bot_d) / bot_b;
	    if (col < maxcol)
		maxcol = col;
	}

	// get screen coords of current pixel - the first one in the current row
	sx = mincol - screen_width2;
	sy = horizon - row;

	// calculate x, y, & z for start of row
	x = ox + hx*sx + vx*sy;  // Don't need to use fpMul for
	y = oy + hy*sx + vy*sy;  //  int * fixed multiply
	z = oz + hz*sx + vz*sy;

	if (z == 0) // hack out ugly case - perhaps use assert?
	{
	    return;
	}

	u_2 = fpDiv(y, z); // get initial texture coordinates
	v_2 = fpDiv(x, z);

	// get initial lighting distance
	light2 = z0 + fpMul(u_2, z_du) - fpMul(v_2, z_dv);

	// pointer to first pixel in row
	screen_ptr = gBits + row * MAXX + mincol;

	// length of current row
	len = 1 + maxcol - mincol;

	while (len > 0) {
	    
	    // how many pixels to skip in this sub-affine step
	    count = min(SLOPE_TEXTURE_STEP, len);
	    len -= SLOPE_TEXTURE_STEP;

	    // save values for current endpoint
	    u = u_1 = u_2;
	    v = v_1 = v_2;
	    light1 = light2;

	    // get x, y & z for next endpoint
	    x += hx * count;
	    y += hy * count;
	    z += hz * count;

	    // get u & v of next endpoint
	    u_2 = fpDiv(y, z);
	    v_2 = fpDiv(x, z);

	    // get lighting distance of next endpoint
	    light2 = z0 + fpMul(u_2, z_du) - fpMul(v_2, z_dv);

	    // use average of enpoints for light lookup
	    light1 = (light1+light2)<<(BASE_DIF-1);

	    if (light1 <= 0)
		light1 = 1; // punt on errors & boundary values ( *mostly* gone now anyways)
	    
	    palette = GetLightPalette(light1, leaf->sector->light, shade,0);

	    // When the change in u & v over the span are relatively small, call
	    //  biLerp to do the linear interpolation inner loop using a modified
	    //  bresenhams algorithm
	    // When the change in u & v are large with respect to count, it is cheaper 
	    //  to just do the divide because DDA will go into large tight loop
	    // MAX_DDA controls when biLerp is used and should be on the order of
	    //  magnitude of the bitmap size

	    if ((ABS(u_2 - u_1) < MAX_DDA) && (ABS(v_2 - v_1) < MAX_DDA)) {
		biLerp(u_1*bitmap_width + tu, u_2*bitmap_width + tu, v_1*bitmap_width + tv, v_2*bitmap_width + tv, count, bitmap_width, bits, screen_ptr, palette);

		screen_ptr += count;
	    }
	    else {

		// (Debug remove) palette = GetLightPalette(light1, leaf->sector->light,FINENESS/2);

		du = ((u_2 - u_1) * bitmap_width) / count;
		dv = ((v_2 - v_1) * bitmap_width) / count;

		u *= bitmap_width;
		v *= bitmap_width;

		do {
		    iu = (ROUND_FIXED_TO_INT(u + tu)) & (bitmap_width-1);
		    iv = (ROUND_FIXED_TO_INT(v + tv)) & (bitmap_width-1);

		    *screen_ptr++ = palette[bits[iv*bitmap_width + iu]];

		    u += du;
		    v += dv;
		} while (--count);
	    }
	}
    }
}

/*****************************************************************************/
/*
 * doDrawLeaf: Draw given leaf node (floor or ceiling).  
 *   cone gives area to draw on screen.
 *   texture is the bitmap to use.
 *   height is the height at which to draw the item, in FINENESS units.
 *   leaf_type is either FLOOR or CEILING
 */
static void doDrawLeaf(BSPleaf *leaf, ViewCone *c, PDIB texture, int height, char leaf_type)
{
   long minrow, maxrow, tex_width, tex_height;
   long mincol, maxcol;
   long row;
   long d;
   int lefttop, righttop, leftbot, rightbot;
   long col;
   long new_hv;
   int xoffset, yoffset;

   long shade;
   
   register int tx, ty, tx_inc, ty_inc;
   register BYTE *palette, *bits;
   register BYTE *screen_ptr, *end_screen_ptr;
   Sector *sector;
   
#if DRAW_LEAF_CONES
   fillcone(c);
   return;
#endif
   
   bits = DibPtr(texture);
   if (bits == NULL)
      return;
   
   tex_width  = DibWidth(texture);
   tex_height = DibHeight(texture);
#if 0
   if ((tex_width != BITMAP_WIDTH || tex_height != BITMAP_WIDTH) &&
       (tex_width != BITMAP_WIDTH * 2 || tex_height != BITMAP_WIDTH * 2))
   {
      debug(("warning: bad leaf bitmap size %d x %d\n", tex_width, tex_height));
   }
#endif
   
   new_hv = (viewer_height - height) * VIEWER_DISTANCE;
   
   // experimental dark eaves
   if (new_hv < 0)
       shade = FINENESS-(shade_amount>>1);
   else
       shade = FINENESS;
   
   lefttop = DIVUP(c->top_b * c->leftedge + c->top_d, c->top_a);
   righttop = DIVUP(c->top_b * c->rightedge + c->top_d, c->top_a);
   if (lefttop < righttop)
     minrow = lefttop;
   else
     minrow = righttop;
   if (minrow < 0)
     {
       debug(("small minrow in doDrawLeaf\n"));
       minrow = 0;
     }
   
   leftbot = DIVDOWN(c->bot_b * c->leftedge + c->bot_d, c->bot_a);
   rightbot = DIVDOWN(c->bot_b * c->rightedge + c->bot_d, c->bot_a);
   if (leftbot > rightbot)
     maxrow = leftbot;
   else
     maxrow = rightbot;
   if (maxrow >= area.cy)
     {
       debug(("large maxrow in doDrawLeaf\n"));
       maxrow = area.cy-1;
     }
   
   if (minrow <= horizon && maxrow >= horizon)
     {
       debug(("leaf can't be in horizon's row!\n"));
       return;
    }
   
   for (row = minrow; row <= maxrow; row++)
   {
      mincol = c->leftedge;
      maxcol = c->rightedge;
      
      if (c->top_b > 0)
      {
	 /* x <= ay-d / b, b>0 */
	 col = (c->top_a * row - c->top_d) / c->top_b;
	 if (col < maxcol) maxcol = col;
      }
      else if (c->top_b < 0)
      {
	 /* x >= ay-d / b, b<0 */
	 col = (c->top_a * row - c->top_d + c->top_b + 1) / c->top_b;
	 if (col > mincol) mincol = col;
      }
      
      if (c->bot_b > 0)
      {
	 /* x >= ay-d / b, b>0 */
	 col = (c->bot_a * row - c->bot_d + c->bot_b - 1) / c->bot_b;
	 if (col > mincol) mincol = col;
      }
      else if (c->bot_b < 0)
      {
	 /* x <= ay-d / b, b<0 */
	 col = (c->bot_a * row - c->bot_d) / c->bot_b;
	 if (col < maxcol) maxcol = col;
      }
      
      d = new_hv / (row - horizon);
      
      if (d <= 0)
      {
	 debug(("d <= 0 for floor or ceiling!\n"));
	 continue;
      }
      
      /* get palette */
      palette = GetLightPalette(d, leaf->sector->light,shade,0);

      sector = leaf->sector;
      xoffset = sector->tx;
      yoffset = sector->ty;

      // Look for scrolling texture
      if (sector->animate != NULL && sector->animate->animation == ANIMATE_SCROLL)
      {
	 if (((sector->flags & SF_SCROLL_FLOOR) && leaf_type == FLOOR) ||
	     ((sector->flags & SF_SCROLL_CEILING) && leaf_type == CEILING))
	 {
	    xoffset -= sector->animate->u.scroll.xoffset;
	    yoffset += sector->animate->u.scroll.yoffset;
	 }
      }
  
#define TEXTURE_ACC (FIX_DECIMAL+4)
      tx_inc = - ((center_b * d) >> (LOG_FINENESS+(FIX_DECIMAL-6)+LOG_VIEWER_DISTANCE-TEXTURE_ACC)); // == LOG_VIEWER_DISTANCE
      ty_inc = + ((center_a * d) >> (LOG_FINENESS+(FIX_DECIMAL-6)+LOG_VIEWER_DISTANCE-TEXTURE_ACC));
      
      tx = ((viewer_x - xoffset) << (TEXTURE_ACC-LOG_FINENESS)) +
	 ((center_a * d) << (TEXTURE_ACC-LOG_FINENESS-(FIX_DECIMAL-6))) +
	    ((mincol - screen_width2) * tx_inc);
      ty = ((viewer_y - yoffset) << (TEXTURE_ACC-LOG_FINENESS)) +
	 ((center_b * d) << (TEXTURE_ACC-LOG_FINENESS-(FIX_DECIMAL-6))) +
	    ((mincol - screen_width2) * ty_inc);
      
      tx &= ((1L << TEXTURE_ACC)-1);
      ty &= ((1L << TEXTURE_ACC)-1);
      
      screen_ptr = gBits + row * MAXX + mincol;
      
      end_screen_ptr = screen_ptr + (maxcol - mincol);
      
      /* Allow 64x64 or 128x128 leaf bitmaps */
      if (tex_width == BITMAP_WIDTH * 2)
	 while (screen_ptr <= end_screen_ptr)
	 {
	    *screen_ptr = palette[*(bits + 
				    (( (ty & ~((1L << (TEXTURE_ACC - (LOG_BITMAP_WIDTH + 1)))-1)) >> (TEXTURE_ACC - 2 * (LOG_BITMAP_WIDTH + 1)) ) +
				     ( tx >> (TEXTURE_ACC - (LOG_BITMAP_WIDTH + 1)) )) )];
	    
	    screen_ptr++;
	    tx=(tx+tx_inc)&((1L << TEXTURE_ACC)-1);
	    ty=(ty+ty_inc)&((1L << TEXTURE_ACC)-1);
	 }
      else
      {
	 while (screen_ptr <= end_screen_ptr)
	 {
	    *screen_ptr = palette[*(bits + 
				    (( (ty & ~((1L << (TEXTURE_ACC - LOG_BITMAP_WIDTH))-1)) >> (TEXTURE_ACC - 2 * LOG_BITMAP_WIDTH) ) +
				     ( tx >> (TEXTURE_ACC - LOG_BITMAP_WIDTH) )) )];
	    
	    screen_ptr++;
	    tx=(tx+tx_inc)&((1L << TEXTURE_ACC)-1);
	    ty=(ty+ty_inc)&((1L << TEXTURE_ACC)-1);
	 }
      }
   }
}
/*****************************************************************************/
static void doDrawFloor(DrawFloorStruct *floor, ViewCone *c)
{
   BSPleaf *leaf = floor->leaf;

   doDrawLeaf(leaf, c, leaf->sector->floor, leaf->sector->floor_height, FLOOR);
}
/*****************************************************************************/
static void doDrawCeiling(DrawCeilingStruct *ceiling, ViewCone *c)
{
   BSPleaf *leaf = ceiling->leaf;

   doDrawLeaf(leaf, c, leaf->sector->ceiling, leaf->sector->ceiling_height, CEILING);
}
/*****************************************************************************/
static void doDrawObject(DrawObjectStruct *object, ViewCone *c)
{
   BOOL bVisible;

#if DRAW_OBJ_CONES
   fillcone(c);
   return;
#endif

   bVisible = DrawObject3D(&object->object->draw, c);

   // Check to see if an object has been completely drawn   
   *(object->object->ncones_ptr) -= 1;

   if (bVisible && (0 == *(object->object->ncones_ptr)))
      DrawObjectDecorations(&object->object->draw);
}
/*****************************************************************************/
static void DrawItems()
{
   DrawItem *item;
   
//   debug(("nitems: %ld\n",nitems));
//   debug(("nitems:%ld depth:%ld max:%ld\n", nitems, max_depth, allocd_cone_nodes));
//   debug(("nitems:%ld maxcones:%ld\n", nitems, allocd_cone_nodes));
  
   /* return; */ /* uncomment this to measure overhead for making draw list.*/
   
   for(item = drawdata+nitems-1; item >= drawdata; item--)
   {
      switch(item->type)
      {
      case DrawWallType:
	 doDrawWall(&item->u.wall, &item->cone);
	 break;
      case DrawFloorType:
	 doDrawFloor(&item->u.floor, &item->cone);
	 break;
      case DrawCeilingType:
	 doDrawCeiling(&item->u.ceiling, &item->cone);
	 break;
      case DrawObjectType:
	 doDrawObject(&item->u.object, &item->cone);
	 break;
      case DrawBackgroundType:
	 doDrawBackground(&item->cone);
	 break;
      case DrawSlopedFloorType:
	 doDrawSloped(&item->cone,item->u.slope.leaf,True);
	 break;
      case DrawSlopedCeilingType:
	 doDrawSloped(&item->cone,item->u.slope.leaf,False);
	 break;
      default:
	 debug(("DrawItemList error!\n"));
	 return;
      }
   }
}

/*****************************************************************************/
static void doDrawBackground(ViewCone *c)
{
   long lefttop,righttop,leftbot,rightbot;
   long minrow,maxrow;
   long mincol,maxcol;
   long row,col;
   long width,height;
   BYTE *bkgnd_bmap, *bkgnd_ptr;
   long length;
   long xoffset;
   list_type l;
   int midScreen = horizon - PlayerGetHeightOffset();

   if (incremental_background)
     background_cones++;

   if (background != NULL)
   {
      width  = DibWidth(background);
      height = DibHeight(background);
      bkgnd_bmap = DibPtr(background);
   }
   lefttop = DIVUP(c->top_b * c->leftedge + c->top_d, c->top_a);
   righttop = DIVUP(c->top_b * c->rightedge + c->top_d, c->top_a);
   if (lefttop < righttop)
     minrow = lefttop;
   else
     minrow = righttop;
   if (minrow < 0)
     minrow = 0;
   
   leftbot = DIVDOWN(c->bot_b * c->leftedge + c->bot_d, c->bot_a);
   rightbot = DIVDOWN(c->bot_b * c->rightedge + c->bot_d, c->bot_a);
   if (leftbot > rightbot)
     maxrow = leftbot;
   else
     maxrow = rightbot;
   if (maxrow >= area.cy)
     maxrow = area.cy-1;

   for (row = minrow; row <= maxrow; row++)
   {
      mincol = c->leftedge;
      maxcol = c->rightedge;
      
      if (c->top_b > 0)
	{
	  /* x <= ay-d / b, b>0 */
	  col = (c->top_a * row - c->top_d) / c->top_b;
	  if (col < maxcol) maxcol = col;
	}
      else if (c->top_b < 0)
	{
	  /* x >= ay-d / b, b<0 */
	  col = (c->top_a * row - c->top_d + c->top_b + 1) / c->top_b;
	  if (col > mincol) mincol = col;
	}
      
      if (c->bot_b > 0)
	{
	  /* x >= ay-d / b, b>0 */
	  col = (c->bot_a * row - c->bot_d + c->bot_b - 1) / c->bot_b;
	  if (col > mincol) mincol = col;
	}
      else if (c->bot_b < 0)
	{
	  /* x <= ay-d / b, b<0 */
	  col = (c->bot_a * row - c->bot_d) / c->bot_b;
	  if (col < maxcol) maxcol = col;
	}
      
      if (background == NULL || IsBlind())
	{
	  if (mincol <= maxcol)
	    memset(gBits + row*MAXX + mincol, 0, maxcol-mincol+1);
	  continue;
	}
      else
      {
	 int y = (row-midScreen) - PlayerGetHeightOffset();
	 while (y < 0)
	    y += height;
	 while (y >= height)
	    y -= height;
	 bkgnd_ptr = bkgnd_bmap + (y * width);
	 xoffset = (world_width * viewer_angle / NUMDEGREES) + mincol;
	 xoffset = xoffset % width;
	 while(mincol <= maxcol)
	 {
	    length = min(maxcol-mincol+1, width-xoffset);
	    
	    memcpy(gBits + row*MAXX + mincol,
   		bkgnd_ptr + xoffset,
   		length);
	    mincol += length;
	    xoffset = 0;
	 }
      }
   }

   // Don't draw bg overlays if blind
   if (IsBlind())
      return;
   
   for (l = background_overlays; l != NULL; l = l->next)
   {
      long starty, endy;
      long over_width, over_height;
      long px,py;
      long deltaAngle;
      BYTE val, *row_bits, *screen_ptr, *end_screen_ptr;
      BackgroundOverlay *overlay = (BackgroundOverlay *) (l->data);
      PDIB pdib_ov = GetObjectPdib(overlay->obj.icon_res, 0, overlay->obj.animate->group);

      overlay->drawn = FALSE;

      if (pdib_ov == NULL)
         continue;
      
      bkgnd_bmap = DibPtr(pdib_ov);
      if (bkgnd_bmap == NULL)
         continue;
      over_width = DibWidth(pdib_ov);
      over_height = DibHeight(pdib_ov);
     
#if 0
      /* arbitrary definition: offsets of a background overlay are:
       * x: angle from due east
       * y: pixels up from horizon
       * negative y gives an object location below the horizon.
       * The given bitmap is centered at the offset location.
       */
      px = overlay->x;
      py = overlay->y;
      /* convert from arbitrary definition to real screen coordinates */
      //px = px/2 + screen_width/2 - viewer_angle/2;
      while (px >= 0)
         px -= NUMDEGREES/2;
      while (px < 0)
	 px += NUMDEGREES/2;
#else
      deltaAngle = overlay->x - viewer_angle;
      if (deltaAngle > (NUMDEGREES/2))
	 deltaAngle -= NUMDEGREES;
      if (deltaAngle < -(NUMDEGREES/2))
	 deltaAngle += NUMDEGREES;
      px = (world_width * deltaAngle / NUMDEGREES);
      px += screen_width2;
      py = horizon - overlay->y;
#endif
      
      /* adjust for center of bitmap */
      px -= over_width/2;
      py -= over_height/2;
      
      if (px + over_width <= c->leftedge || px > c->rightedge)
         continue;
      starty = max(minrow, py);
      endy = min(maxrow, py + over_height - 1);
      overlay->rcScreen.top = starty;
      overlay->rcScreen.bottom = endy;
      bg_overlayVisible = TRUE;
      for (row = starty; row <= endy; row++)
      {
	 mincol = max(c->leftedge, px);
	 maxcol = min(c->rightedge, px + over_width - 1);
	 
	 if (c->top_b > 0)
	 {
	    /* x <= ay-d / b, b>0 */
	    col = (c->top_a * row - c->top_d) / c->top_b;
	    if (col < maxcol) maxcol = col;
	 }
	 else if (c->top_b < 0)
	 {
	    /* x >= ay-d / b, b<0 */
	    col = (c->top_a * row - c->top_d + c->top_b + 1) / c->top_b;
	    if (col > mincol) mincol = col;
	 }
	 
	 if (c->bot_b > 0)
	 {
	    /* x >= ay-d / b, b>0 */
	    col = (c->bot_a * row - c->bot_d + c->bot_b - 1) / c->bot_b;
	    if (col > mincol) mincol = col;
	 }
	 else if (c->bot_b < 0)
	 {
	    /* x <= ay-d / b, b<0 */
	    col = (c->bot_a * row - c->bot_d) / c->bot_b;
	    if (col < maxcol) maxcol = col;
	 }
	 
	 overlay->rcScreen.left = mincol;
	 overlay->rcScreen.right = maxcol;
	 overlay->drawn = TRUE;

	 row_bits = bkgnd_bmap + (row - py) * over_width + (mincol - px);
	 screen_ptr = gBits + row * MAXX + mincol;
	 end_screen_ptr = screen_ptr + (maxcol - mincol);
	 while(screen_ptr <= end_screen_ptr)
	 {
	    val = *(row_bits++);
	    if (val != TRANSPARENT_INDEX)
	       *screen_ptr = val;
	    screen_ptr++;
	 }
      }
   }
}

/**********************************************************************
 * check_viewer_height(tree): makes sure that viewer_height is below
 * the ceiling and above the floor in tree.  It will mutate viewer_height
 * to enforce this condition.  If no value for viewer_height will work,
 * or if something is wrong with the tree, returns false.  Otherwise, it
 * returns true.
 **********************************************************************/
#define MIN_HEIGHT_DIFF 10
static Bool check_viewer_height(BSPnode *tree)
{
   long side;
   long floorheight,ceilingheight;
   BSPnode *pos, *neg;
   
   while (1)
   {
      if (tree == NULL)
      {
	 debug(("check_viewer_height got NULL tree\n"));
	 return False;
      }
      
      switch(tree->type)
      {
      case BSPleaftype:
	 ceilingheight = GetCeilingHeight(viewer_x, viewer_y, tree->u.leaf.sector);
	 
	 if (viewer_height > ceilingheight - MIN_HEIGHT_DIFF)
	   viewer_height = ceilingheight - MIN_HEIGHT_DIFF;
	 
	 floorheight = GetFloorHeight(viewer_x, viewer_y, tree->u.leaf.sector);
	 
	 if (viewer_height < floorheight + MIN_HEIGHT_DIFF)
	   viewer_height = floorheight + MIN_HEIGHT_DIFF;
	 
	 if (viewer_height > ceilingheight - MIN_HEIGHT_DIFF)
	   {
	     debug(("not enough room between floor and ceiling!!!\n"));
	     return True;

	     return False;
	   }
	 return True;
	 
      case BSPinternaltype:
	 side = tree->u.internal.separator.a * viewer_x + 
	    tree->u.internal.separator.b * viewer_y +
	       tree->u.internal.separator.c;
	 
	 pos = tree->u.internal.pos_side;
	 neg = tree->u.internal.neg_side;
	 if (side == 0)
	   {
	     /* make sure we satisfy height restriction on both sides! */
	     /* this isn't quite right, but it's close XXX */
	     Bool ok = True;
	     if (pos)
	       ok = ok && check_viewer_height(pos);
	     if (neg)
	       ok = ok && check_viewer_height(neg);
	     return ok;
	   }
	 else if (side > 0)
	    tree = pos;
	 else
	    tree = neg;
	 break;
	 
      default:
	 debug(("check_viewer_height error!\n"));
	 return False;
      }
   }
}
/*****************************************************************************/
/* SetMappingValues
 *    Calculates values used in texture mapping all the cones of a 
 * given sloped surface from the current view point. Must be called once per
 * frame for every visible sloped sector. Refer to slope documentation for
 * an explanation of the math involved.
 */
static void SetMappingValues(SlopeData *slope ) {

    Pnt3D v0; // Texture enpoints in un-projected screen space (view space?)
    Pnt3D v1;
    Pnt3D v2;
    Pnt3D m;  // Basis vectors in un-projected screen space (view space?)
    Pnt3D n;

    // Transform texture endpoints into view space. This amounts to translating
    // the origin to the viewer position and rotating by viewer angle. 
    // center_a & b happen to be the Cos & Sin of the viewing angle multiplied
    // by FINENESS. BASE_DIF helps translate the results into FIXED_PRECISION
    // units. (1<<(LOG_FINENESS+BASE_DIF-1)) is added to round instead of trunc
    // in final divide. x & y values are multiplied by VIEWER_DISTANCE to
    // convert them to pixel units.
    
    v0.z = (long)((slope->p0.x - viewer_x) * center_a + (slope->p0.y - viewer_y) * center_b + (1<<(LOG_FINENESS+BASE_DIF-1)))>>(LOG_FINENESS+BASE_DIF);
    v0.x = (VIEWER_DISTANCE * ((long)((slope->p0.x - viewer_x) * -center_b + (slope->p0.y - viewer_y) * center_a + (1<<(LOG_FINENESS-1)))>>LOG_FINENESS) + (1<<(BASE_DIF-1)))>>BASE_DIF;
    v0.y = (long)(VIEWER_DISTANCE * (slope->p0.z - viewer_height) + (1<<(BASE_DIF-1)))>>BASE_DIF;

    v1.z = (long)((slope->p1.x - viewer_x) * center_a + (slope->p1.y - viewer_y) * center_b + (1<<(LOG_FINENESS+BASE_DIF-1)))>>(LOG_FINENESS+BASE_DIF);
    v1.x = (VIEWER_DISTANCE * ((long)((slope->p1.x - viewer_x) * -center_b + (slope->p1.y - viewer_y) * center_a + (1<<(LOG_FINENESS-1)))>>LOG_FINENESS) + (1<<(BASE_DIF-1)))>>BASE_DIF;
    v1.y = (long)(VIEWER_DISTANCE * (slope->p1.z - viewer_height) + (1<<(BASE_DIF-1)))>>BASE_DIF;

    v2.z = (long)((slope->p2.x - viewer_x) * center_a + (slope->p2.y - viewer_y) * center_b + (1<<(LOG_FINENESS+BASE_DIF-1)))>>(LOG_FINENESS+BASE_DIF);
    v2.x = (VIEWER_DISTANCE * ((long)((slope->p2.x - viewer_x) * -center_b + (slope->p2.y - viewer_y) * center_a + (1<<(LOG_FINENESS-1)))>>LOG_FINENESS) + (1<<(BASE_DIF-1)))>>BASE_DIF;
    v2.y = (long)(VIEWER_DISTANCE * (slope->p2.z - viewer_height) + (1<<(BASE_DIF-1)))>>BASE_DIF;

    // now generate basis vectors for texture transform
    
    m.x = v1.x - v0.x;
    m.y = v1.y - v0.y;
    m.z = v1.z - v0.z;

    n.x = v2.x - v0.x;
    n.y = v2.y - v0.y;
    n.z = v2.z - v0.z;

    // h, v, & o vectors express relation between texture coordinates 
    // and pixel coordinates
    
    slope->h.x = fpMul(v0.y, m.z) - fpMul(v0.z, m.y);
    slope->h.y = fpMul(v0.y, n.z) - fpMul(v0.z, n.y);
    slope->h.z = fpMul(n.y, m.z) - fpMul(n.z, m.y);

    slope->v.x = fpMul(v0.z, m.x) - fpMul(v0.x, m.z);
    slope->v.y = fpMul(v0.z, n.x) - fpMul(v0.x, n.z);
    slope->v.z = fpMul(n.z, m.x) - fpMul(n.x, m.z);

    slope->o.x = fpMul(v0.x, m.y) - fpMul(v0.y, m.x);
    slope->o.y = fpMul(v0.x, n.y) - fpMul(v0.y, n.x);
    slope->o.z = fpMul(n.x, m.y) - fpMul(n.y, m.x);
    
    // save these for calculating distance from viewer
    // in terms of u & v
    slope->z_du = m.z;
    slope->z_dv = n.z;
    slope->z0 = v0.z;

}

/**********************************************************************
 * DrawBSP: draw view of player standing at (x,y) (in FINENESS coords)
 *          with view width `width'.
 *  If draw is False, just trace BSP tree without drawing anything.
 **********************************************************************/
void DrawBSP(room_type *room, Draw3DParams *params, long width, Bool draw)
{
   ConeTreeNode *c;
   long         index;
  
//  debug(("DrawBSP debug\n"));
   nitems = 0;
   
   if (draw)
      bg_overlayVisible = FALSE;
  
   /* set up some drawing parameters */
   viewer_id = params->player_id;
   viewer_x = params->viewer_x;
   viewer_y = params->viewer_y;
   viewer_angle = params->viewer_angle;
   viewer_height = params->viewer_height;
   screen_width = width;
   screen_width2 = width/2;

   background_overlays = room->bg_overlays;

   /* make sure viewer is below ceiling and above floor */
   if (!check_viewer_height(room->tree))
      return;
  
   /* find equations that bound viewing frustum */
   /* keep 10 = FIX_DECIMAL-6 bits of accuracy */
   center_a = COS(viewer_angle) >> 6;
   center_b = SIN(viewer_angle) >> 6;

   // If the D3D render is in use, we need to increase the frustum
   if (D3DRenderIsEnabled())
   {
      left_a = -center_b + ((center_a * screen_width) >> LOG_VIEWER_DISTANCE);
      left_b = center_a + ((center_b * screen_width) >> LOG_VIEWER_DISTANCE);

      right_a = center_b + ((center_a * screen_width) >> LOG_VIEWER_DISTANCE);
      right_b = -center_a + ((center_b * screen_width) >> LOG_VIEWER_DISTANCE);
   }
   else
   {
      left_a = -center_b + ((center_a * screen_width2) >> LOG_VIEWER_DISTANCE);
      left_b = center_a + ((center_b * screen_width2) >> LOG_VIEWER_DISTANCE);

      right_a = center_b + ((center_a * screen_width2) >> LOG_VIEWER_DISTANCE);
      right_b = -center_a + ((center_b * screen_width2) >> LOG_VIEWER_DISTANCE);
   }


   /* add moving objects to BSP tree */
   AddObjects(room);

   /* set up original view cone */
   init_cone_tree();

#if DRAW_WALL_CONES || DRAW_LEAF_CONES || DRAW_OBJ_CONES
   fillcolor = 7;
#endif

   if (background_cones > 40)
   {
      incremental_background = False;
	  if (draw)
		doDrawBackground(&cone_tree_root->cone);
   }
   else
   {
      incremental_background = True;
   }
   background_cones = 0;

   // set up texture mapping variables for all slopes for this frame
   for (index = 0; index < room->num_sectors; index++) 
   {
      if (room->sectors[index].sloped_floor != NULL)
	  SetMappingValues(room->sectors[index].sloped_floor);
      if (room->sectors[index].sloped_ceiling != NULL)
	  SetMappingValues(room->sectors[index].sloped_ceiling);
   }

   /* find items in view */
   WalkBSPtree(room->tree);

   if (draw)
   {
      /* draw background in still-exposed cones */
      if (incremental_background)
	 for(c=start_anchor.next; c != &end_anchor; c = c->next)
	    doDrawBackground(&c->cone);
  
      /* draw items, back to front */
		DrawItems();

#if DRAW_CONE_OUTLINES
      {
	 int i;
	 if (incremental_background)
	    for(c=start_anchor.next; c != &end_anchor; c = c->next)
	       outlinecone(&c->cone);
	 for(i=0; i<nitems; i++)
	    if (drawdata[i].type != DrawObjectType)
	       outlinecone(&drawdata[i].cone);
      }
#endif
   }
}

/**********************************************************************/
void BSPInitialize(void)
{
}
/**********************************************************************/
void BSPEnterRoom(room_type *room)
{
   // Upon entering a room, erase information about drawing the background
   // incrementally in the previous room.  If we don't reset it, sometimes
   // we use up all item memory because the previous room draw the background
   // incrementally.
   background_cones = MAX_ITEMS;
}
