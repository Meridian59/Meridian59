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

/********************************************************************/
/*
 * CompareProjectiles:  Utility comparison function for use with lists.
 */
Bool CompareProjectiles(void *p1, void *p2)
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

   // If animation off, don't bother with projectiles
   if (!config.animate)
     return;

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
   p->motion.source_z = s->motion.z; //GetPointFloor(s->motion.x, s->motion.y);
#if 0
   if (source_obj == player.id)
      p->motion.source_z += PlayerGetHeightOffset() / (FINENESS * 2);
   else
      p->motion.source_z += s->obj.boundingHeight / (FINENESS * 2);
#endif

   p->motion.dest_x = d->motion.x;
   p->motion.dest_y = d->motion.y;
   p->motion.dest_z = d->motion.z; //GetPointFloor(d->motion.x, d->motion.y);
#if 0
   if (dest_obj == player.id)
      p->motion.dest_z += PlayerGetHeightOffset() / (FINENESS * 2);
   else
      p->motion.dest_z += d->obj.boundingHeight / (FINENESS * 2);
#endif

//   debug(("source obj = %d, x = %d, y = %d, z = %d\n", source_obj, 
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
      distance = GetLongSqrt(dx * dx + dy * dy + dz * dz) / FINENESS;
      p->motion.increment = ((float) speed) / 1000.0 / distance;
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
Bool ProjectilesMove(int dt)
{
   list_type l, next;

   if (current_room.projectiles == NULL)
      return False;

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

   return True;
}
/********************************************************************/
/*
 * RadiusProjectileAdd:  Starts a new radius projectile on its way.
 * Projectile goes from source object to the range limit of the spell.
 */
void RadiusProjectileAdd(Projectile *p, ID source_obj, BYTE speed, WORD flags,
                         WORD reserved, BYTE range, BYTE number)
{
   float distance, destx, desty, radangle, fRange;
   int dx, dy, dz, destz;
   room_contents_node *s;
   fRange = range;
   float initangle = 0.0;

   // If animation off, don't bother with projectiles.
   if (!config.animate)
     return;

   for (int i=1; i <= number; i++)
   {
      Projectile *q = (Projectile *) ZeroSafeMalloc(sizeof(Projectile));

      q->icon_res = p->icon_res;
      q->translation = p->translation;
      q->animate = p->animate;
      q->dLighting = p->dLighting;

      // Set source coordinates based on object location
      s = GetRoomObjectById(source_obj);

      q->motion.source_x = s->motion.x;
      q->motion.source_y = s->motion.y;
      q->motion.source_z = s->motion.z; //GetPointFloor(s->motion.x, s->motion.y);
#if 0
if (source_obj == player.id)
   q->motion.source_z += PlayerGetHeightOffset() / (FINENESS * 2);
else
   q->motion.source_z += s->obj.boundingHeight / (FINENESS * 2);
#endif

      /* We're launching projectiles in a circle, so we need to determine
         which angle we need to shoot this projectile, and the destination. */

      radangle = (initangle*3.14159)/180.0;
      destx = s->motion.x + ((fRange*1000.0) * cos(radangle));
      desty = s->motion.y + ((fRange*1000.0) * sin(radangle));
      destz = s->motion.z;
#if 0
   destz += PlayerGetHeightOffset() / (FINENESS * 2);
#endif

      q->motion.dest_x = (destx);
      q->motion.dest_y = (desty);
      q->motion.dest_z = destz; //GetPointFloor(s->motion.x + range, s->motion.y + range);

      // See how far we should move per frame
      dx = q->motion.dest_x - q->motion.source_x;
      dy = q->motion.dest_y - q->motion.source_y;
      dz = q->motion.dest_z - q->motion.source_z;

      if (speed == 0 || (dx == 0 && dy == 0 && dz == 0))
         q->motion.increment = 1.0;
      else
      {
         distance = GetLongSqrt(dx * dx + dy * dy + dz * dz) / FINENESS;
         q->motion.increment = ((float) speed) / 1000.0 / distance;
      }

      q->motion.x = q->motion.source_x;
      q->motion.y = q->motion.source_y;
      q->motion.z = q->motion.source_z;

      q->motion.progress = 0.0;

      // Set projectile angle.
      q->angle = intATan2(dy, dx) & NUMDEGREES_MASK;

      q->flags = flags;
      q->reserved = reserved;

      current_room.projectiles = list_add_item(current_room.projectiles, q);

      // Set next angle.
      initangle = initangle + 360.0/number;
   }
   SafeFree(p);
}
