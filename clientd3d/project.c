// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * project.c:  Deal with "shooting" bitmaps, that is, bitmaps which move continuously
 *   from one place to another.  Objects can't be shot this way; this is essentially
 *   a special effect.
 *
 * Projectile motion is part of animation.  When the animation timer goes off,
 * ProjectilesMove is called to move each projectile the correct amount, depending
 * on the time since the last frame.  The actual guts of motion are in MoveSingle in move.c.
 */

#include "client.h"

extern room_type current_room;
extern player_info player;

// Maximum time (in seconds) for a projectile to reach its target.
// This ensures long-range projectiles arrive quickly enough to match attack sounds.
// Note: Since motion updates happen at variable frame rates (dt in milliseconds),
// this value is used to calculate a speed multiplier to cap total travel time.
static const float MAX_PROJECTILE_TRAVEL_TIME = 2.0;

/********************************************************************/
/*
 * CompareProjectiles:  Utility comparison function for use with lists.
 */
bool CompareProjectiles(void *p1, void *p2)
{
   return (Projectile *) p1 == (Projectile *) p2;
}
/********************************************************************/
/*
 * ProjectileAdd:  Start a new projectile on its way.  Projectile goes
 *   from source object to destination object (both objects must be in the room).
 */
void ProjectileAdd(Projectile *p, ID source_obj, ID dest_obj, BYTE speed, WORD flags, WORD reserved)
{
   float distance;
   int dx, dy, dz;
   room_contents_node *s, *d;

   debug(("Adding new projectile\n"));

   // Set source and destination coordinates based on object locations
   s = GetRoomObjectById(source_obj);
   d = GetRoomObjectById(dest_obj);
   if (s == NULL || d == NULL)
   {
      debug(("Projectile add got object not in room\n"));
      return;
   }
   if (s == d)
   {
      debug(("Projectile add got source == dest\n"));
      return;
   }

   p->motion.source_x = s->motion.x;
   p->motion.source_y = s->motion.y;
   p->motion.source_z = s->motion.z;

   p->motion.dest_x = d->motion.x;
   p->motion.dest_y = d->motion.y;
   p->motion.dest_z = d->motion.z;

   // debug(("source obj = %d, x = %d, y = %d, z = %d\n", source_obj,
   //	   p->motion.source_x, p->motion.source_y, p->motion.source_z));
   //
   //   debug(("dest obj = %d, x = %d, y = %d, z = %d\n", dest_obj,
   //	   p->motion.dest_x, p->motion.dest_y, p->motion.dest_z));

   // See how far we should move per frame
   dx = p->motion.dest_x - p->motion.source_x;
   dy = p->motion.dest_y - p->motion.source_y;
   dz = p->motion.dest_z - p->motion.source_z;

   if (speed == 0 || (dx == 0 && dy == 0 && dz == 0))
      p->motion.increment = 1.0;
   else
   {
      float dx_f = (float) dx;
      float dy_f = (float) dy;
      float dz_f = (float) dz;

      distance = sqrtf(dx_f * dx_f + dy_f * dy_f + dz_f * dz_f) / FINENESS;

      // Calculate normal increment based on speed
      // speed is in grid squares per 10 seconds, distance is in grid squares
      float normal_increment = ((float) speed) / 1000.0f / distance;

      // Cap maximum travel time to prevent extremely long-range projectiles from taking
      // 30+ seconds to arrive, which breaks gameplay (attack sounds/animations finish
      // while projectile is still mid-flight). This only affects edge cases at extreme
      // ranges; normal combat distances use the unmodified speed parameter.
      // Calculate minimum increment needed to arrive within MAX_PROJECTILE_TRAVEL_TIME:
      // min_increment ensures: (1.0 / min_increment / 1000.0) <= MAX_PROJECTILE_TRAVEL_TIME
      float min_increment = 1.0f / (MAX_PROJECTILE_TRAVEL_TIME * 1000.0f);

      // Use the faster of the two speeds (normal or minimum required)
      p->motion.increment = (std::max)(normal_increment, min_increment);
   }

   p->motion.x = p->motion.source_x;
   p->motion.y = p->motion.source_y;
   p->motion.z = p->motion.source_z;

   p->motion.progress = 0.0;

   // Set projectile angle
   p->angle = intATan2(dy, dx) & NUMDEGREES_MASK;

   p->flags = flags;
   p->reserved = reserved;

   current_room.projectiles = list_add_item(current_room.projectiles, p);
}
/********************************************************************/
/*
 * ProjectilesMove; called for every frame.  
 *   dt is number of milliseconds since last time animation timer went off.
 * Return True iff some projectile moved.
 */
bool ProjectilesMove(int dt)
{
   list_type l, next;

   if (current_room.projectiles == NULL)
      return false;

   // Have to be careful here, since deleting items from list as we go down it
   l = current_room.projectiles;
   while (l != NULL)
   {
      Projectile *p = (Projectile *) (l->data);
      next = l->next;

      if (MoveSingle(&p->motion, dt))
      {
         current_room.projectiles = list_delete_item(current_room.projectiles, p, 
                                                     CompareProjectiles);
         SafeFree(p);
      }
      else if (p->flags & PROJ_FLAG_FOLLOWGROUND)
      {
	 p->motion.z = GetPointFloor(p->motion.x, p->motion.y);
      }
      l = next;
   }

   return true;
}
