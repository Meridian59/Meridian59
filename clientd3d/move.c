// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * move.c:  Handle user motion in room.
 *
 * Moves are performed optimistically:  we inform the server of each move and update the
 * user's position without waiting for a response from the server.  The server can reject
 * a move by telling us to move the user back to his original position.
 *
 * A user's move is divided into a sequence of equally sized small "steps".  At each step,
 * we check if the user has moved off the room, too close to a wall, or across a grid 
 * line.  This allows us to check walls and grid lines in a straightforward way, without
 * worrying about a large move that crosses multiple grid lines, etc.  On fast machines,
 * dividing a small move into many steps can result in roundoff errors, so the number of steps
 * per move depends on the amount of time since the previous move.
 *
 * To check if a move is too close to a wall, we traverse the room's BSP tree.  Nodes
 * whose bounding boxes are too far away from the user's position are ignored.  For other
 * nodes, we first check the distance to each wall in the node, and then descend to the
 * node's children.  If a move is initially disallowed, we try to "slide" along the wall
 * in the way by moving parallel to it (i.e. taking the component of the move along the wall.)
 *
 * If a player's altitude changes as a result of a move, we set the player's vertical velocity
 * field (v_z).  This causes the player to climb steps and fall with gravity if animation is on.
 *
 * Since we need to wait for a response from the server before changing rooms,
 * it's wasteful to keep requesting room changes while waiting for the response.
 * In addition, the extra off-room moves generated while waiting for the response can
 * cause the player to teleport one he does change rooms.  Thus, we delay about a second
 * between consecutive attempts at moving off a room.
 *
 * The kod sends a "moveon" type with each object.  Most objects can be moved onto, but
 * the user cannot enter the square of such things such as monsters and other players.
 * We disallow these moves here to prevent the kod from having to reject them.  Teleporters
 * are a special case:  we are allowed to move onto them, but then the kod will move the
 * user to another square.  After moving onto a teleporter, we prevent any further user 
 * moves until the kod moves the user.
 */

#include "client.h"

#define MAX_CLOSE_WALLS 32

#define MOVE_DELAY    100   // Minimum # of milliseconds between moving MOVEUNITS
#define TURN_DELAY    100   // Minimum number of milliseconds between a full turn action

#define NUM_STEPS_PER_SECOND 200  // # of small "steps" within movement per second
#define STEPS_PER_MOVE 20         // Maximum number of small "steps" within a single move

#define MAX_STEP_HEIGHT (HeightKodToClient(24)) // Max vertical step size (FINENESS units)

#define MOVE_INTERVAL  250            // Inform server once per this many milliseconds
#define MOVE_THRESHOLD (FINENESS / 4)  // Inform server only of moves at least this large

#define MIN_NOMOVEON (FINENESS / 4)   // Closest we can get to a nomoveon object
#define MIN_HOTPLATE_DIST (FINENESS ) // Closest we can get to a hotplate before notification

#define TELEPORT_DELAY 5000           // # of milliseconds to wait for server to teleport us

#define MIN_SIDE_MOVE (MOVEUNITS / 4)

#define USER_WALKING_SPEED 25 // Speed we send to the server for walking.
#define USER_RUNNING_SPEED 50 // Speed we send to the server for running.

// Number of milliseconds between retrying to change rooms
#define MOVE_OFF_ROOM_INTERVAL 1000
// Time when we last tried to move off room
static DWORD move_off_room_time = 0;
// Time when next move is allowed; used only for teleporter nomoveons.
static DWORD next_move_time = 0;

/* Amplitude of player's bouncing motion */
#define BOUNCE_HEIGHT (FINENESS >> 5)

/* Offset from player's average height to give appearance of bouncing */
static long bounce_height;

/* Offset from player's average height to allow user to look up or down */
static long height_offset;

// Return values for MoveObjectAllowed
enum { MOVE_BLOCKED = 1, MOVE_OK, MOVE_CHANGED, };

extern player_info player;
extern room_type current_room;
extern int sector_depths[];

static DWORD server_time = 0;           // Last time we informed server of our position
static DWORD last_splash = 0;           // Time of the last play of the splash wading sound

static Bool pos_valid = FALSE;          // True when server_x and server_y are valid
static int  server_x = 0, server_y = 0; // Last position we've told server we are, in FINENESS units
static int  server_angle = 0;           // Last angle we've told server we have, in server angle units
static int  last_move_action = 0;       // Last movement action; used to determine speed of motion

static int  min_distance = 48;          // Minimum distance player is allowed to get to wall
static int  min_distance2 = 48*48;  	// Minimum distance squared
static Bool blocked = FALSE;            // Set to true when a move hits a wall

static WallData *worstWall = NULL;
WallData *lastBlockingWall = NULL;
static BSPnode *lastBlockingNode = NULL;

static int worstDistance = 0;

/* local function prototypes */
static void CheckPlayerMove();
static void BounceUser(int dt);
static int MoveObjectAllowed(room_type *room, int old_x, int old_y, int *new_x, int *new_y, int z);
static WallData *IntersectNode(BSPnode *node, int old_x, int old_y, int new_x, int new_y, int z);
static BSPnode *FindIntersection(BSPnode *node, int xOld, int yOld, int xNew, int yNew, int z, WallData **wallIntersect);
static void SlideAlongWall(WallData *wall, int xOld, int yOld, int *xNew, int *yNew);

void ResetPlayerPosition(void)
{
   lastBlockingWall = NULL;
   lastBlockingNode = NULL;
   worstWall = NULL;
   min_distance = player.width / 2;
   min_distance2 = min_distance * min_distance;
}

BOOL	gbMouselook = FALSE;

extern double gravityAdjust;

/************************************************************************/
void UserMovePlayer(int action)
{
   int depthMask[4] = {0,1,2,4};
   int dx, dy;
   long angle;
   int xinc, yinc;  // Size of small step
   int x, y, z;        // Position of player as we move him along
   int last_x, last_y, last_z;  // Position of player at previous step
   int original_x, original_y, original_z;  // Position of player at start of move
   int row, col;
   int retval;
   int i, move_distance, dt;
   int num_steps;   // Number of small steps to divide move into (for checking walls)
   int depth;
   DWORD now;
   room_contents_node *player_obj;
   static DWORD last_move_time = 0;
   Bool bounce = True;
   Bool changed_square = False;
   BSPleaf *leaf;

   if (effects.paralyzed)
      return;

   player_obj = GetRoomObjectById(player.id);
   if (player_obj == NULL)
   {
      debug(("UserMovePlayer got NULL player object!\n"));
      return;
   }

   if (!GetFrameDrawn())
      return;

   if (player.viewID)
   {
      if (player.viewFlags & REMOTE_VIEW_CONTROL)
         return;
      if (!(player.viewFlags & REMOTE_VIEW_MOVE))
         return;
   }
   // Find out how far to move based on time elapsed:  always move at a rate of 
   // constant rate per MOVE_DELAY milliseconds.
   switch (action)
   {
   case A_FORWARDFAST:
   case A_BACKWARDFAST:
   case A_SLIDELEFTFAST:
   case A_SLIDERIGHTFAST:
   case A_SLIDELEFTFORWARDFAST:
   case A_SLIDERIGHTFORWARDFAST:
   case A_SLIDELEFTBACKWARDFAST:
   case A_SLIDERIGHTBACKWARDFAST:
      move_distance = 2 * MOVEUNITS;
   break;

   default:
      move_distance = MOVEUNITS;
   break;
   }

   now = timeGetTime();

   // Wading slows player movement down.
   depth = GetPointDepth(player_obj->motion.x, player_obj->motion.y);
   if (SF_DEPTH1 == depth)
      move_distance = move_distance * 3/4;
   else if (SF_DEPTH2 == depth)
      move_distance = move_distance / 2;
   else if (SF_DEPTH3 == depth)
      move_distance = move_distance / 4;

   // See if we're waiting for server to move us out of current location
   if (now < next_move_time)
      return;

   dt = now - last_move_time;

   // Watch for int wraparound in timeGetTime()
   if (dt <= 0)
      dt = 1;
   
   if (dt < MOVE_DELAY && config.animate)
   {
      gravityAdjust = 1.0;
      move_distance = move_distance * dt / MOVE_DELAY;
   }
   else
   {
      gravityAdjust = (double)MOVE_DELAY / (double)dt;
   }
   last_move_time = now;

   switch (action)
   {
   case A_FORWARD:
   case A_FORWARDFAST:
      angle = player.angle;
      break;
   case A_BACKWARD:
   case A_BACKWARDFAST:
      angle = (player.angle + NUMDEGREES / 2) % NUMDEGREES;  /* angle + pi */
      break;
   case A_SLIDELEFT:
   case A_SLIDELEFTFAST:
      angle = (player.angle + 3 * NUMDEGREES / 4) % NUMDEGREES;  /* angle + 3pi/2 */
      break;
   case A_SLIDERIGHT:
   case A_SLIDERIGHTFAST:
      angle = (player.angle + NUMDEGREES / 4) % NUMDEGREES;  /* angle + pi/2 */
      break;
   case A_SLIDELEFTFORWARD:
   case A_SLIDELEFTFORWARDFAST:
      angle = (player.angle + 7 * NUMDEGREES / 8) % NUMDEGREES;
      break;
   case A_SLIDELEFTBACKWARD:
   case A_SLIDELEFTBACKWARDFAST:
      angle = (player.angle + 5 * NUMDEGREES / 8) % NUMDEGREES;
      break;
   case A_SLIDERIGHTFORWARD:
   case A_SLIDERIGHTFORWARDFAST:
      angle = (player.angle + 1 * NUMDEGREES / 8) % NUMDEGREES;
      break;
   case A_SLIDERIGHTBACKWARD:
   case A_SLIDERIGHTBACKWARDFAST:
      angle = (player.angle + 3 * NUMDEGREES / 8) % NUMDEGREES;
      break;
   default:
      debug(("Bad action type in UserMovePlayer\n"));
      return;
   }

   last_move_action = action;
   FindOffsets(move_distance, angle, &dx, &dy);

   num_steps = max(1, min(STEPS_PER_MOVE, NUM_STEPS_PER_SECOND * dt / 1000));

//   xinc = dx;// / num_steps;
//   yinc = dy;// / num_steps;
   xinc = dx / num_steps;
   yinc = dy / num_steps;

   last_x = player_obj->motion.x;
   last_y = player_obj->motion.y;
   last_z = player_obj->motion.z;
   original_x = player_obj->motion.x;
   original_y = player_obj->motion.y;
   original_z = player_obj->motion.z;

   // Closest player can get to wall is half his width.  Avoid divide by zero errors.
   min_distance = max(1, player.width / 2);
   min_distance2 = min_distance * min_distance;

   for (i=0; i < num_steps; i++)
   //for (i=0; i < 1; i++)
   {
      x = last_x + xinc;
      y = last_y + yinc;
      z = max(player_obj->motion.z, GetFloorBase(last_x, last_y));

      // Can't move into a place with no floor
      leaf = BSPFindLeafByPoint(current_room.tree, x, y);
      if (leaf == NULL || leaf->sector == NULL)
      {
         x = last_x;
         y = last_y;
         z = last_z;
         break;
      }

      if (lastBlockingNode)
      {
         lastBlockingWall = IntersectNode(lastBlockingNode,last_x,last_y,x,y,z);
         if (lastBlockingWall)
         {
            SlideAlongWall(lastBlockingWall,last_x,last_y,&x,&y);
         }
         else
            lastBlockingNode = NULL;
      }
      else
      {
         lastBlockingNode = FindIntersection(current_room.tree,last_x,last_y,x,y,z,&lastBlockingWall);
         if (lastBlockingNode)
         {
            SlideAlongWall(lastBlockingWall,last_x,last_y,&x,&y);
         }
      }
      if (FindIntersection(current_room.tree,last_x,last_y,x,y,z,&lastBlockingWall))
      {
         SlideAlongWall(lastBlockingWall,last_x,last_y,&x,&y);
         if (FindIntersection(current_room.tree,last_x,last_y,x,y,z,&lastBlockingWall))
         {
            FindOffsets(MIN_SIDE_MOVE, (angle + 3 * NUMDEGREES / 4) % NUMDEGREES, &dx, &dy);
            x = last_x + dx;
            y = last_y + dy;
            if (FindIntersection(current_room.tree,last_x,last_y,x,y,z,&lastBlockingWall))
            {
               FindOffsets(MIN_SIDE_MOVE, (angle + NUMDEGREES / 4) % NUMDEGREES, &dx, &dy);
               x = last_x + dx;
               y = last_y + dy;
               if (FindIntersection(current_room.tree,last_x,last_y,x,y,z,&lastBlockingWall))
               {
                  x = last_x;
                  y = last_y;
                  z = last_z;
                  break;
               }
            }
         }
      }

      // Don't try to slide to current location
      if (x == last_x && y == last_y)
         x = x;
//	 break;
      
      // Get around integer divide yuckiness
      if (y < 0)
         row = -1;
      else
         row = y / FINENESS;
      if (x < 0)
         col = -1;
      else
         col = x / FINENESS;

      // See if trying to move off room.
      if (!IsInRoom(row, col, current_room))
      {
         // Delay between consecutive attempts to move off room
         if (now - move_off_room_time >= MOVE_OFF_ROOM_INTERVAL) // current time 
         {
            // Need to send walking speed for room change, otherwise room
            // changes by players with vigor < 10 will be blocked.
            RequestMove(y, x, USER_WALKING_SPEED, player.room_id);
            move_off_room_time = now;
         }
         // Don't actually move player off room
         x = last_x;
         y = last_y;
         z = last_z;
         break;
      }
      else
      {
         // Reset moving-off-room timer, since this move doesn't go off the room
         move_off_room_time = 0;
      }

      // Check next part of step with new floor height or current player height, whichever
      // is higher (works correctly when user is falling)
      z = max(z, player_obj->motion.z);
      
      // See if an object prevents this move
      retval = MoveObjectAllowed(&current_room, last_x, last_y, &x, &y, z);

      if (retval == MOVE_BLOCKED)
      {
         // debug(("Move blocked by object\n"));
         x = last_x;
         y = last_y;
         z = last_z;
         bounce = False;
         break;
      }
      else if (retval == MOVE_CHANGED)
      {
         // Stop at this modified move
         break;
      }

      last_x = x;
      last_y = y;
      last_z = z;
   }

   // Set new player position
   player_obj->motion.x = x;
   player_obj->motion.y = y;
   //player_obj->motion.z = z;

   // XXX Don't really need to store position in player, since he is also a room object.
   //     The way things are currently done, we now need to update player.
   player.x = x;
   player.y = y;

   CheckPlayerMove();
   MoveUpdateServer();

   // Set up vertical motion, if necessary
   z = GetFloorBase(x, y);
   if (z == -1)
      z = player_obj->motion.z;

   if (config.animate)
   {
      player_obj->motion.dest_z = z;

      // Only set motion if not already moving that direction
      if (z > player_obj->motion.z && player_obj->motion.v_z <= 0)
      {
         player_obj->motion.v_z = CLIMB_VELOCITY_0;
      }
      else if (z < player_obj->motion.z && player_obj->motion.v_z >= 0)
      {
         player_obj->motion.v_z = FALL_VELOCITY_0;
      }
   }
   else player_obj->motion.z = z;

   if (bounce)
      BounceUser(dt);

   // Play the wading sound if they persist to wade,
   // but sounds get spaced out more if they're deeper.
   z = GetFloorBase(original_x, original_y);
   if (depth != SF_DEPTH0 &&
      !(depthMask[depth] & GetRoomFlags()) &&
       config.play_sound && effects.wadingsound &&
       player_obj->motion.z <= z &&
       ((last_splash == 0xFFFFFFFF) || ((now - last_splash) > (DWORD)(500*depth))))
   {
       PlayWaveRsc(effects.wadingsound, MAX_VOLUME, 0, 0, 0, 0, 0);
       last_splash = now;
   }
   if (depth == SF_DEPTH0)
   {
       last_splash = 0xFFFFFFFF;
   }

   // Update looping sounds to reflect the player's new position
//   debug(("Player now at: (%i,%i)\n",player.x >> LOG_FINENESS,player.y >> LOG_FINENESS));
   UpdateLoopingSounds( player.x >> LOG_FINENESS, player.y  >> LOG_FINENESS );

   RedrawAll();
}

void SlideAlongWall(WallData *wall, int xOld, int yOld, int *xNew, int *yNew)
{  // Try to "slide" along in direction of wall
   double dx = (double)(*xNew - xOld);
   double dy = (double)(*yNew - yOld);
   double wall_dx = (double)(wall->x1 - wall->x0);
   double wall_dy = (double)(wall->y1 - wall->y0);
   double num = dx * wall_dx + dy * wall_dy;
   double denom = wall_dx * wall_dx + wall_dy * wall_dy;
    
   *xNew = xOld + FloatToInt(wall_dx * num / denom);
   *yNew = yOld + FloatToInt(wall_dy * num / denom);
}

BSPnode *FindIntersection(BSPnode *node, int xOld, int yOld, int xNew, int yNew, int z,WallData **wallIntersect)
{
   WallData *wall;
   if ((NULL == node) || (BSPleaftype == node->type))
      return NULL;

   wall = IntersectNode(node, xOld, yOld, xNew, yNew, z);
   if (wall)
   {
      *wallIntersect = wall;
      return node;
   }
   else
   {
      BSPinternal *inode = &node->u.internal;
      BSPnode *nodeIntersect = FindIntersection(inode->pos_side, xOld, yOld, xNew, yNew, z, wallIntersect);
      if (nodeIntersect)
         return nodeIntersect;
      nodeIntersect = FindIntersection(inode->neg_side, xOld, yOld, xNew, yNew, z, wallIntersect);
      if (nodeIntersect)
         return nodeIntersect;
      return NULL;
   }
}

WallData *IntersectNode(BSPnode *node, int old_x, int old_y, int new_x, int new_y, int z)
{
   BSPinternal *inode;
   WallData *wall;
   float a, b, c, plane_distance, old_distance, newDistance;

   if (node == NULL || node->type == BSPleaftype)
      return NULL;

   // Skip this node if we're too far away from its bounding box
   if((node->bbox.x0 - new_x > min_distance) || 
      (new_x - node->bbox.x1 > min_distance) ||
      (node->bbox.y0 - new_y > min_distance) || 
      (new_y - node->bbox.y1 > min_distance))
      return NULL;
       
   inode = &node->u.internal;

   // See if we're close to plane of this node--distance between point and line
   a = inode->separator.a;
   b = inode->separator.b;
   c = inode->separator.c;
   plane_distance = (a * new_x + b * new_y + c);
   old_distance = (a * old_x + b * old_y + c);
   newDistance = ABS(plane_distance) / FINENESS;
   if ((newDistance > min_distance) || (ABS(plane_distance) > ABS(old_distance)))
      return NULL;

   // Check walls in this node
   for (wall = inode->walls_in_plane; wall != NULL; wall = wall->next)
   {
      Sidedef *sidedef;
      Sector *other_sector;
      int below_height;
      float d0, d1, dx, dy, lenWall2;
      float minx = min(wall->x0, wall->x1) - min_distance;
      float maxx = max(wall->x0, wall->x1) + min_distance;
      float miny = min(wall->y0, wall->y1) - min_distance;
      float maxy = max(wall->y0, wall->y1) + min_distance;

      // See if we are near the wall itself, and not just the wall's plane
      if ((new_x >= minx) && (new_x <= maxx) && (new_y >= miny) && (new_y <= maxy))
      {
         // Skip floor->ceiling wall if player can walk through it
         if (SGNDOUBLE(old_distance) > 0)
         {
            sidedef = wall->pos_sidedef;
            other_sector = wall->neg_sector;
         }
         else
         {
            sidedef = wall->neg_sidedef;
            other_sector = wall->pos_sector;
         }
         if (sidedef == NULL)
            continue;

         // Check for wading on far side of wall; reduce effective height of wall if found
         below_height = 0;
         if (other_sector != NULL)
            below_height = sector_depths[SectorDepth(other_sector->flags)];

         // Can't step up too far; watch bumping your head; see if passable
         if ((sidedef->below_bmap == NULL || 
            (sidedef->below_bmap != NULL && 
            (wall->z1 - below_height - z) <= MAX_STEP_HEIGHT))
            &&
            (sidedef->above_bmap == NULL || 
            (sidedef->above_bmap != NULL && wall->z2 - z >= player.height)) 
            &&
            (sidedef->flags & WF_PASSABLE))
            continue;

         // If distance to either vertex is > wall length, then destination of move is
         // past end of wall; use distance to closer vertex as distance to line
         dx = new_x - wall->x0;
         dy = new_y - wall->y0;
         d0 = dx * dx + dy * dy;

         dx = new_x - wall->x1;
         dy = new_y - wall->y1;
         d1 = dx * dx + dy * dy;

         // Recheck distance to wall based on distance to closest vertex
         //len = FinenessKodToClient(wall->length);
         //lenWall2 = len * len;
         dx = wall->x1 - wall->x0;
         dy = wall->y1 - wall->y0;
         lenWall2 = dx * dx + dy * dy;
         if (d0 > lenWall2) // d1 is closest vertex
         {
            float dx = old_x - wall->x1;
            float dy = old_y - wall->y1;
            float oldEndDist = dx * dx + dy * dy;
            if ((d1 < min_distance2) && (d1 <= oldEndDist))
            {
               return wall;
            }
         }
         else if (d1 > lenWall2) // d0 is closest vertex
         {
            float dx = old_x - wall->x0;
            float dy = old_y - wall->y0;
            float oldEndDist = dx * dx + dy * dy;
            if ((d0 < min_distance2) && (d0 <= oldEndDist))
            {
               return wall;
            }
         }
         else // within wall range!
         {
            return wall;
         }
      }
   }
   return NULL;
}
/************************************************************************/
/*
 * MoveObjectAllowed:  See if an object in the room prevents motion
 *   from (old_x, old_y) to (new_x, new_y) to the given location.
 *   May modify new_x and new_y to attempt to slide along object.
 *   If player is moving to a square with a teleporter, set next_move_time.
 *
 *   Returns:
 * 
 *   MOVE_BLOCKED if move is illegal.
 *   MOVE_OK if move is legal.
 *   MOVE_CHANGED if a modified move is legal.
 */

int MoveObjectAllowed(room_type *room, int old_x, int old_y, int *new_x, int *new_y, int z)
{
   static int idLastObjNotify = -1;
   list_type l;
   int dx, dy;
   int old_distance, new_distance;
   WallData *wall;
   BYTE speed;
   BOOL moveReported = FALSE;
   int idObjNotify = -1;
   
   for (l = room->contents; l != NULL; l = l->next)
   {
      room_contents_node *r = (room_contents_node *) (l->data);
      
      if (r->obj.id == player.id)
      {
         if ((int)r->obj.id == idLastObjNotify)
            idLastObjNotify = -1;
         continue;
      }
      
      dx = abs(r->motion.x - *new_x);
      dy = abs(r->motion.y - *new_y);
      
      switch (r->obj.moveontype)
      {
      case MOVEON_NOTIFY:
         if ((dx > MIN_HOTPLATE_DIST) || (dy > MIN_HOTPLATE_DIST))
         {
            if ((int)r->obj.id == idLastObjNotify)
               idLastObjNotify = -1;
            continue;
         }

         idObjNotify = r->obj.id;
         if (idObjNotify != idLastObjNotify)
         {
            if (!moveReported)
            {
               speed = 10;
               //if (last_move_action == A_FORWARDFAST || last_move_action == A_BACKWARDFAST)
               if (IsMoveFastAction(last_move_action))
                  speed *= 2;
               MoveUpdateServer();
               moveReported = TRUE;
               idLastObjNotify = idObjNotify;
            }
         }
         break;

      case MOVEON_NO:
         if (dx > MIN_NOMOVEON || dy > MIN_NOMOVEON ||
             (dx * dx + dy * dy) > MIN_NOMOVEON * MIN_NOMOVEON)
            continue;
         
         // Allowed to move away from object
         new_distance = dx * dx + dy * dy;
         
         dx = abs(r->motion.x - player.x);
         dy = abs(r->motion.y - player.y);
         old_distance = dx * dx + dy * dy;
         if (new_distance > old_distance)
            break;
         
         // Try to slide along object -- object represented by a square of side MIN_NOMOVEON
         dx = abs(r->motion.x - *new_x);
         dy = abs(r->motion.y - *new_y);
         
         if (dx < MIN_NOMOVEON)
            if (r->motion.x > *new_x)
               *new_x = r->motion.x - MIN_NOMOVEON;
            else *new_x = r->motion.x + MIN_NOMOVEON;
         else
            if (dy < MIN_NOMOVEON)
               if (r->motion.y > *new_y)
                  *new_y = r->motion.y - MIN_NOMOVEON;
               else *new_y = r->motion.y + MIN_NOMOVEON;
         
         // See if modified move is legal
         if (!FindIntersection(current_room.tree, old_x, old_y, *new_x, *new_y, z ,&wall))
            return MOVE_CHANGED;
         
         return MOVE_BLOCKED;
         break;
         
      case MOVEON_TELEPORTER:
         if (dx > MIN_NOMOVEON || dy > MIN_NOMOVEON ||
             (dx * dx + dy * dy) > MIN_NOMOVEON * MIN_NOMOVEON)
            continue;
         
         next_move_time = timeGetTime() + TELEPORT_DELAY;
         break;
      }
   }
   
   return MOVE_OK;
}

/************************************************************************/
/*
 * CheckPlayerMove:  Called whenever the player is relocated, either
 *                   by the server or by the player's own efforts.
 *                   If the player's moving somewhere they shouldn't,
 *                   then we tattle to the server.  We don't stop the
 *                   player from being there; that's up to the server.
 */
void CheckPlayerMove()
{
   room_contents_node *player_obj;
   BSPleaf* leaf;

   player_obj = GetRoomObjectById(player.id);
   if (!player_obj)
      return;

   // Server's trying to move to a place the client finds suspicious.
   // We undo the move, and tell the server that we're doing it.
   //
   leaf = BSPFindLeafByPoint(current_room.tree, player_obj->motion.x, player_obj->motion.y);
   if (leaf == NULL || leaf->sector == NULL ||
       (/* current_room->secure_zero */ TRUE && leaf->sector == &current_room.sectors[0]))
   {
//    debug(("Player is out of bounds.\n"));
/*
      // Set new player position.
      player_obj->motion.x = server_x;
      player_obj->motion.y = server_y;
      player.x = server_x;
      player.y = server_y;

      MoveUpdateServer();
*/
   }
}

/************************************************************************/
/*
 * ServerMovedPlayer:  Called whenever the player is relocated by the server.
 *   Stop restricting user motion for teleporter nomoveons.
 */
void ServerMovedPlayer(void)
{
   room_contents_node *player_obj;

   // Allow user to move again (used for teleporter nomoveons).
   next_move_time = 0;

   // Put player on floor
   player_obj = GetRoomObjectById(player.id);
   if (!player_obj)
      return;

   CheckPlayerMove();
   RoomObjectSetHeight(player_obj);
   server_x = player_obj->motion.x;
   server_y = player_obj->motion.y;
   //player_obj->motion.z = GetFloorBase(server_x,server_y);

   // Update looping sounds to reflect the player's new position
//   debug(("Player now at: (%i,%i)\n",player.x >> LOG_FINENESS,player.y >> LOG_FINENESS));
   UpdateLoopingSounds( player.x >> LOG_FINENESS, player.y  >> LOG_FINENESS);
}
/************************************************************************/
/*
 * MoveUpdateServer:  Update the server's knowledge of our position and angle, if necessary.
 */ 
void MoveUpdateServer(void)
{
   DWORD now = timeGetTime();
   int angle;

   // Inform server if necessary
   if (now - server_time < MOVE_INTERVAL || !pos_valid)
      return;

   MoveUpdatePosition();

   angle = ANGLE_CTOS(player.angle);
   if (server_angle != angle)
   {
      RequestTurn(player.id, angle);
      server_angle = angle;
      server_time = now;
   }
}
/************************************************************************/
/*
 * MoveUpdatePosition:  Update the server's knowledge of our position.
 *   This procedure does not care about elapsed interval, but
 *   sends the update only when the position has changed at least a bit.
 */ 
void MoveUpdatePosition(void)
{
   int x, y;
   BYTE speed;

   x = player.x;
   y = player.y;

   // don't send update if we didn't move
   if ((server_x - x) * (server_x - x) + (server_y - y) * (server_y - y) > MOVE_THRESHOLD)
   {
      // debug output
      debug(("MoveUpdatePosition: x (%d -> %d), y (%d -> %d)\n", server_x, x, server_y, y));
   
      // the following speed values must match
      // the actual speed a player is moving
      // defined by MOVEUNITS and MOVE_DELAY
      // as well as those defined in user.kod

      // walk-speed (USER_WALKING_SPEED from user.kod)
      speed = USER_WALKING_SPEED;
   
      // run-speed (USER_RUNNING_SPEED from user.kod)
      if (IsMoveFastAction(last_move_action))
         speed = USER_RUNNING_SPEED;

      // send update
      RequestMove(y, x, speed, player.room_id);
   
      // save last sent position and tick
      server_x = x;
      server_y = y;
      server_time = timeGetTime();
   }
}
/************************************************************************/
/*
 * MoveSetValidity:  valid tells whether our knowledge of the player's position
 *   is correct.  This should be False during initialization.
 */ 
void MoveSetValidity(Bool valid)
{
   room_contents_node *player_obj;

   pos_valid = valid;

   if (pos_valid)
   {
      server_time = timeGetTime();

      player_obj = GetRoomObjectById(player.id);

      if (player_obj == NULL)
      {
         debug(("MoveSetValidity got NULL player object\n"));
         return;
      }

      player.x = player_obj->motion.x;
      player.y = player_obj->motion.y;
      server_x = player.x;
      server_y = player.y;
   }
}
/************************************************************************/
void UserTurnPlayer(int action)
{
   int dt, delta;  /* # of degrees to turn */
   DWORD now;
   static DWORD last_turn_time = 0;

   if (effects.paralyzed)
      return;

   switch(action)
   {
   case A_TURNLEFT:
      delta = - TURNDEGREES;
      break;
   case A_TURNRIGHT:
      delta = TURNDEGREES;
      break;
   case A_TURNFASTLEFT:
      delta = - 3 * TURNDEGREES;
      break;
   case A_TURNFASTRIGHT:
      delta = 3 * TURNDEGREES;
      break;
   default:
      debug(("Bad action type in UserTurnPlayer\n"));
      return;
   }

   // Find out how far to turn based on time elapsed:  always turn at a rate of
   // TURNDEGREES per TURN_DELAY milliseconds.
   now = timeGetTime();
   dt = now - last_turn_time;
   if (last_turn_time == 0 || dt <= 0)
      dt = 1;
   if (dt < TURN_DELAY && config.animate)
   {
      delta = delta * dt / TURN_DELAY;
   }
   else if (dt > (4*TURN_DELAY) && config.animate)
   {
      delta = (delta * (int)(GetFrameTime()) / TURN_DELAY) / 2;
   }
   last_turn_time = now;

   if (player.viewID)
   {
      if (!(player.viewFlags & REMOTE_VIEW_TURN))
         return;
      if (player.viewFlags & REMOTE_VIEW_CONTROL)
      {
         room_contents_node *viewObject = GetRoomObjectById(player.viewID);
         if (viewObject)
         {
            viewObject->angle += delta;
            if (viewObject->angle < 0)
               viewObject->angle += NUMDEGREES;
            viewObject->angle = viewObject->angle % NUMDEGREES;
            RedrawAll();
            return;
         }
      }
   }

   player.angle += delta;
   if (player.angle < 0)
      player.angle += NUMDEGREES;
   player.angle = player.angle % NUMDEGREES;

   // Inform server of turn if necessary
   MoveUpdateServer();

   RedrawAll();
}

/************************************************************************/
void UserTurnPlayerMouse(int delta)
{
   if (effects.paralyzed)
      return;

   if (player.viewID)
   {
      if (!(player.viewFlags & REMOTE_VIEW_TURN))
         return;
      if (player.viewFlags & REMOTE_VIEW_CONTROL)
      {
         room_contents_node *viewObject = GetRoomObjectById(player.viewID);
         if (viewObject)
         {
            viewObject->angle += delta;
            if (viewObject->angle < 0)
               viewObject->angle += NUMDEGREES;
            viewObject->angle = viewObject->angle % NUMDEGREES;
            RedrawAll();
            return;
         }
      }
   }

   player.angle += delta;
   if (player.angle < 0)
      player.angle += NUMDEGREES;
   player.angle = player.angle % NUMDEGREES;

   // Inform server of turn if necessary
   MoveUpdateServer();

   RedrawAll();
}
/************************************************************************/
void UserFlipPlayer(void)
{
   if (effects.paralyzed)
      return;

   if (player.viewID)
   {
      if (!(player.viewFlags & REMOTE_VIEW_TURN))
         return;
      if (player.viewFlags & REMOTE_VIEW_CONTROL)
      {
         room_contents_node *viewObject = GetRoomObjectById(player.viewID);
         if (viewObject)
         {
            viewObject->angle += NUMDEGREES / 2;
            viewObject->angle = viewObject->angle % NUMDEGREES;
            RedrawAll();
            return;
         }
      }
   }

   // Turn 180 degrees around
   player.angle += NUMDEGREES / 2;
   player.angle = player.angle % NUMDEGREES;
   MoveUpdateServer();
   RedrawAll();
}

// Move at most HEIGHT_INCREMENT per HEIGHT_DELAY milliseconds
#define HEIGHT_INCREMENT (MAXY / 4)    
#define HEIGHT_DELAY     100
#define HEIGHT_MAX_OFFSET (3 * MAXY / 2)    // Farthest you can look up or down
/************************************************************************/
/* 
 * BounceUser:  Modify user's height a little to give appearance that 
 *   he's bouncing a little as he walks.
 *   dt is # of milliseconds since last user motion
 */
void BounceUser(int dt)
{
   static float bounce_time = 0.0;

   if (config.bounce)
   {
     dt = min(dt, MOVE_DELAY);
     bounce_time += ((float) dt) / MOVE_DELAY;  /* In radians */
     bounce_height = (long) (BOUNCE_HEIGHT * sin(bounce_time));
   }
   else
      bounce_height = 0;
}
/************************************************************************/
/*
 * PlayerChangeHeight:  If dz > 0, move player's eye level up.  If dz < 0,
 *   move eye level down.
 *   Also make sure that height stays within reasonable range.
 *   This is used for allowing the player to look up and down.
 */
void PlayerChangeHeight(int dz)
{
   DWORD now;
   int dt;
   static DWORD last_time = 0;

   dz = SGN(dz) * HEIGHT_INCREMENT;

   now = timeGetTime();
   dt = now - last_time;
   if (last_time == 0 || dt <= 0)
      dt = 1;
   if (dt < HEIGHT_DELAY && config.animate)
   {
      dz = dz * dt / HEIGHT_DELAY;
   }
   else if (dt > (4*HEIGHT_DELAY) && config.animate)
   {
      dz = (dz * (int)(GetFrameTime()) / HEIGHT_DELAY) / 2;
   }
   last_time = now;

   height_offset += dz;
   height_offset = min(height_offset, HEIGHT_MAX_OFFSET);
   height_offset = max(height_offset, - HEIGHT_MAX_OFFSET);
   RedrawAll();
}

void PlayerChangeHeightMouse(int dz)
{
   height_offset += dz;
   height_offset = min(height_offset, HEIGHT_MAX_OFFSET);
   height_offset = max(height_offset, - HEIGHT_MAX_OFFSET);
}
/************************************************************************/
/*
 * PlayerResetHeight:  Set player's eye height so that he's looking straight ahead.
 */
void PlayerResetHeight(void)
{
   height_offset = 0;
   RedrawAll();
}
/************************************************************************/
/*
 * PlayerGetHeight:  Return height of player's eye above the floor, in FINENESS units.
 */
int PlayerGetHeight(void)
{
   // Save last player height for when data becomes invalid (e.g. GC)
   // but we still want to redraw.
   static int height;
   room_contents_node *r;

   if (player.viewID)
   {
      return player.viewHeight;
   }
   r = GetRoomObjectById(player.id);
   if (r != NULL)
      height = r->motion.z + player.height + bounce_height;

   return height;
}
/************************************************************************/
/*
 * PlayerGetHeightOffset: 
 */
int PlayerGetHeightOffset(void)
{
   return height_offset;
}
