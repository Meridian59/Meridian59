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

// Maximum time (in seconds) for a projectile to reach its target
// This ensures long-range projectiles arrive quickly enough to match attack sounds
#define MAX_PROJECTILE_TRAVEL_TIME 10.0f

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
      // Use 64-bit integers to prevent overflow when calculating distance
      __int64 dx64 = (__int64) dx;
      __int64 dy64 = (__int64) dy;
      __int64 dz64 = (__int64) dz;

      distance = sqrtf((float) (dx64 * dx64 + dy64 * dy64 + dz64 * dz64)) / FINENESS;

      // Calculate normal increment based on speed
      float normal_increment = ((float) speed) / 1000.0f / distance;

      // Calculate the time it would take at normal speed (in seconds)
      // increment * dt * frames = 1.0, so time = 1.0 / (increment * fps)
      // Assuming 60 FPS and dt in milliseconds: time = 1000.0 / (increment * 60)
      float estimated_travel_time = 1.0f / normal_increment / 60.0f;

      // If travel time exceeds maximum, scale up the speed
      if (estimated_travel_time > MAX_PROJECTILE_TRAVEL_TIME)
      {
         // Scale increment to ensure arrival within MAX_PROJECTILE_TRAVEL_TIME
         float speed_multiplier = estimated_travel_time / MAX_PROJECTILE_TRAVEL_TIME;
         p->motion.increment = normal_increment * speed_multiplier;

         debug(("Long range projectile: distance=%.1f, speed boosted by %.2fx\n", distance, speed_multiplier));
      }
      else
      {
         p->motion.increment = normal_increment;
      }
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
