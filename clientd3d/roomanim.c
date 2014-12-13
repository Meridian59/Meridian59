// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * roomanim.c:  Handle animations of room structures (sectors, sidedefs, etc.).
 */

#include "client.h"

// Max amount that flickering can affect light level
#define FLICKER_INTENSITY 40    

// Return values for RoomAnimateSingle
enum { RAS_NONE, RAS_CHANGED, RAS_DONE };

extern room_type current_room;
extern int	gD3DEnabled;

// A random number picked per frame, to keep all sectors with the same user id #
// to flicker identically
static int flicker_amount;

static Bool AnimateSectors(room_type *room, int dt);
static Bool AnimateSidedefs(room_type *room, int dt);
static int  RoomAnimateSingle(RoomAnimate *ra, int dt);
static void SectorAdjustHeight(room_type *room, Sector *s, BYTE type, int height);
static void SidedefDoAnimation(room_type *room, Sidedef *s, int return_code);
static void SectorDoAnimation(room_type *room, Sector *s, int return_code);
/************************************************************************/
/* 
 * AnimateRoom:  Animate wall and floor textures; return True iff any was animated.
 *   dt is number of milliseconds since last time animation timer went off.
 */
Bool AnimateRoom(room_type *room, int dt)
{
   Bool retval, need_redraw = False;

   retval = AnimateSectors(room, dt);
   need_redraw = need_redraw || retval;

   retval = AnimateSidedefs(room, dt);
   need_redraw = need_redraw || retval;

   return need_redraw;
}
/************************************************************************/
/* 
 * AnimateSectors:  Animate floor textures; return True iff any was animated.
 *   dt is number of milliseconds since last time animation timer went off.
 */
Bool AnimateSectors(room_type *room, int dt)
{
   int i, ras_retval;
   Bool need_redraw = False;

   // Setup flickering for this frame
   flicker_amount = rand();

   for (i=0; i < room->num_sectors; i++)
   {
      Sector *s = &room->sectors[i];

      if (s->animate == NULL)
	 continue;

      ras_retval = RoomAnimateSingle(s->animate, dt);
	  s->flags |= SF_HAS_ANIMATED;
      if (ras_retval == RAS_NONE)
	 continue;

      SectorDoAnimation(room, s, ras_retval);
      if (ras_retval == RAS_DONE)
      {
	 SafeFree(s->animate);
	 s->animate = NULL;
      }
      need_redraw = True;
   }
   return need_redraw;
}
/************************************************************************/
/* 
 * AnimateSidedefs:  Animate wall textures; return True iff any was animated.
 *   dt is number of milliseconds since last time animation timer went off.
 */
Bool AnimateSidedefs(room_type *room, int dt)
{
   int i, ras_retval;
   Bool need_redraw = False;

   for (i=0; i < room->num_sidedefs; i++)
   {
      Sidedef *s = &room->sidedefs[i];

      if (s->animate == NULL)
	 continue;

      ras_retval = RoomAnimateSingle(s->animate, dt);
	  s->flags |= WF_HAS_ANIMATED;
      if (ras_retval == RAS_NONE)
	 continue;

      SidedefDoAnimation(room, s, ras_retval);
      if (ras_retval == RAS_DONE)
      {
	 SafeFree(s->animate);
	 s->animate = NULL;
      }
	 
      need_redraw = True;
   }
   return need_redraw;
}
/************************************************************************/
/*
 * SidedefDoAnimation:  Update sidedef structures after animation timer goes off.
 *   return_code gives return value of RoomAnimateSingle.
 */
void SidedefDoAnimation(room_type *room, Sidedef *s, int return_code)
{
   grid_bitmap_type above_grid, normal_grid, below_grid;
   Animate *a;

   switch (s->animate->animation)
   {
   case ANIMATE_ROOM_BITMAP:
      a = &s->animate->u.bitmap.a;
      if (s->above_type != 0)
      {
	 above_grid    = GetGridBitmap(s->above_type);
	 if (!above_grid) { debug(("Unable to draw texture %d (overflown cache?)\n", s->above_type)); return; }
	 s->above_bmap = 
	    BitmapsGetPdib(above_grid->bmaps, 
			   a->group % BitmapsNumGroups(above_grid->bmaps), 0);
      }
      if (s->below_type != 0)
      {
	 below_grid    = GetGridBitmap(s->below_type);
	 if (!below_grid) { debug(("Unable to draw texture %d (overflown cache?)\n", s->below_type)); return; }
	 s->below_bmap = 
	    BitmapsGetPdib(below_grid->bmaps, 
			   a->group % BitmapsNumGroups(below_grid->bmaps), 0);
      }
      if (s->normal_type != 0)
      {
	 normal_grid    = GetGridBitmap(s->normal_type);
	 if (!normal_grid) { debug(("Unable to draw texture %d (overflown cache?)\n", s->normal_type)); return; }
	 s->normal_bmap = 
	    BitmapsGetPdib(normal_grid->bmaps, 
			   a->group % BitmapsNumGroups(normal_grid->bmaps), 0);
      }
      
      if (return_code == RAS_DONE)
      {      
	 // Animation has ended; perform action if appropriate
	 switch (s->animate->u.bitmap.action)
	 {
	 case RA_NONE:
	    break;
	    
	 case RA_PASSABLE_END:
	    s->flags |= WF_PASSABLE;
	    break;
	    
	 case RA_IMPASSABLE_END:
	    s->flags &= ~WF_PASSABLE;
	    break;

	 case RA_INVISIBLE_END:
	    s->flags |= WF_PASSABLE;
	    s->normal_type = 0;
	    s->normal_bmap = NULL;
	    break;
	    
	 default:
	    debug(("SidedefDoAnimation got bad action code %d\n", s->animate->u.bitmap.action));
	    break;
	 }
      }
      break;
      
   case ANIMATE_SCROLL:
      // Nothing to do
      break;

   default:
      debug(("SidedefDoAnimation got unknown animation type %d\n", s->animate->animation));
      break;
   }
}
/************************************************************************/
/*
 * SectorDoAnimation:  Update sector structures after animation timer goes off.
 *   return_code gives return value of RoomAnimateSingle.
 */
void SectorDoAnimation(room_type *room, Sector *s, int return_code)
{
   grid_bitmap_type floor_grid, ceiling_grid;
   Animate *a;

   switch (s->animate->animation)
   {
   case ANIMATE_FLOOR_LIFT:
   case ANIMATE_CEILING_LIFT:
      // Adjust height
      SectorAdjustHeight(room, s, s->animate->animation, s->animate->u.lift.z);
      break;
      
   case ANIMATE_ROOM_BITMAP:
      a = &s->animate->u.bitmap.a;
      if (s->floor_type != 0)
      {
	 floor_grid = GetGridBitmap(s->floor_type);
	 if (!floor_grid) { debug(("Unable to draw texture %d (overflown cache?)\n", s->floor_type)); return; }
	 s->floor   = 
	    BitmapsGetPdib(floor_grid->bmaps, 
			   a->group % BitmapsNumGroups(floor_grid->bmaps), 0);
      }
      if (s->ceiling_type != 0)
      {
	 ceiling_grid = GetGridBitmap(s->ceiling_type);
	 if (!ceiling_grid) { debug(("Unable to draw texture %d (overflown cache?)\n", s->ceiling_type)); return; }
	 s->ceiling = 
	    BitmapsGetPdib(ceiling_grid->bmaps, 
			   a->group % BitmapsNumGroups(ceiling_grid->bmaps), 0);
      }
      break;

   case ANIMATE_SCROLL:
      // Nothing to do
      break;

   case ANIMATE_FLICKER:
      s->light = s->animate->u.flicker.light;
      break;

   default:
      debug(("AnimateSectors got unknown animation type %d\n", s->animate->animation));
      break;
   }
}
/************************************************************************/
/*
 * WallChange:  Animate wall with given ID # in given way.
 */
void WallChange(WORD wall_num, Animate *a, BYTE action)
{
   int i;
   Sidedef *s;

   debug(("WallChange got wall %d\n", (int) wall_num));

   // Adjust animation if user has it turned off
   if (!config.animate)
      VerifyAnimation(a);

   for (i=0; i < current_room.num_sidedefs; i++)
   {
      s = &current_room.sidedefs[i];
      if (s->server_id != wall_num)
	 continue;

      // XXX Finish animation in progress here?
      if (s->animate == NULL)
	 s->animate = (RoomAnimate *) SafeMalloc(sizeof(RoomAnimate));

      s->animate->animation = ANIMATE_ROOM_BITMAP;
      memcpy(&s->animate->u.bitmap.a, a, sizeof(Animate));
      s->animate->u.bitmap.action = action;

      // If animation is a simple change of bitmap, do it now and throw it away
      if (!config.animate || a->animation == ANIMATE_NONE)
      {
	 SidedefDoAnimation(&current_room, s, RAS_DONE);
	 SafeFree(s->animate);
	 s->animate = NULL;
	 RedrawAll();
      }
   }
}
/************************************************************************/
/*
 * SectorAnimate:  Animate sector with given ID # in given way.
 */
void SectorAnimate(WORD sector_num, Animate *a, BYTE action)
{
   int i;
   Sector *s;

   debug(("SectorAnimate got sector %d\n", (int) sector_num));

   // Adjust animation if user has it turned off
   if (!config.animate)
      VerifyAnimation(a);

   for (i=0; i < current_room.num_sectors; i++)
   {
      s = &current_room.sectors[i];
      if (s->server_id != sector_num)
	 continue;

      if (s->animate == NULL)
	 s->animate = (RoomAnimate *) SafeMalloc(sizeof(RoomAnimate));

      // XXX Finish animation in progress here?

      s->animate->animation = ANIMATE_ROOM_BITMAP;
      memcpy(&s->animate->u.bitmap.a, a, sizeof(Animate));
      s->animate->u.bitmap.action = action;

      // If animation off, do it now and throw it away
      if (!config.animate || a->animation == ANIMATE_NONE)
      {
	 SectorDoAnimation(&current_room, s, RAS_DONE);
	 SafeFree(s->animate);
	 s->animate = NULL;
	 RedrawAll();
      }
   }
}
/************************************************************************/
/*
 * SectorChange:  Change the animation properties and flags of the sector
 *   with the given id number.
 */
void SectorChange(WORD sector_num, BYTE depth, BYTE scroll)
{
   int i;
   Sector *s;
   BYTE direction, floor, ceiling;

   for (i=0; i < current_room.num_sectors; i++)
   {
      s = &current_room.sectors[i];
      if (s->server_id != sector_num)
         continue;

      // Remove the current depth value and add the new one.
      if (depth != CHANGE_OVERRIDE)
      {
         s->flags &= ~SectorDepth(s->flags);
         s->flags |= depth;
      }

      // If we want to stop scrolling, remove all the scroll data.
      if (scroll != CHANGE_OVERRIDE)
      {
         if (scroll == SCROLL_NONE)
         {
            s->flags &= ~0x000001FC;
         }
         else
         {
            // Save other flag values that occupy the same space. Note that
            // if we're changing an already changed value, we'll be using those
            // values here. Client gets redrawn to prevent any issues with this
            // i.e. if the previous change was to delete them all.
            direction = SectorScrollDirection(s->flags);
            floor = s->flags & SF_SCROLL_FLOOR;
            ceiling = s->flags & SF_SCROLL_CEILING;

            s->flags &= ~0x000001FC;
            s->flags |= scroll << 2;
            // Replace the other ones.
            if (direction != SCROLL_N)
               s->flags |= direction << 4;
            if (floor)
               s->flags |= SF_SCROLL_FLOOR;
            if (ceiling)
               s->flags |= SF_SCROLL_CEILING;
         }
      }
   }
   RedrawAll();
}
/************************************************************************/
/*
 * TextureChange:  Change all walls, floors, and ceilings with given id num
 *   to have the given texture.  flags specifies which of wall, floor, and ceiling
 *   textures to change.
 */
void TextureChange(WORD id_num, WORD texture_num, BYTE flags)
{
   int i;

   for (i=0; i < current_room.num_sidedefs; i++)
   {
      Sidedef *s = &current_room.sidedefs[i];
      if (s->server_id != id_num)
	 continue;
      
      if (flags & CTF_ABOVEWALL)
      {
	 s->above_type = texture_num;
	 s->above_bmap = GetGridPdib(texture_num);
      }

      if (flags & CTF_NORMALWALL)
      {
	 s->normal_type = texture_num;
	 s->normal_bmap = GetGridPdib(texture_num);
      }

      if (flags & CTF_BELOWWALL)
      {
	 s->below_type = texture_num;
	 s->below_bmap = GetGridPdib(texture_num);
      }
   }

   for (i=0; i < current_room.num_sectors; i++)
   {
      Sector *s = &current_room.sectors[i];
      if (s->server_id != id_num)
	 continue;

      if (flags & CTF_FLOOR)
      {
	 s->floor_type = texture_num;
	 s->floor      = GetGridPdib(texture_num);
      }

      if (flags & CTF_CEILING)
      {
	 s->ceiling_type = texture_num;
	 s->ceiling      = GetGridPdib(texture_num);
      } 
   }
   RedrawAll();
}
/************************************************************************/
/*
 * SectorFlickerChange:  Set flickering on or off in given sector number.
 */
void SectorFlickerChange(WORD sector_num, BYTE type)
{
   int i;

   for (i=0; i < current_room.num_sectors; i++)
   {
      Sector *s = &current_room.sectors[i];
      if (s->server_id != sector_num)
	 continue;

      switch (type)
      {
      case SL_FLICKER_ON:
	 // Set up flickering animation--recall that an animation may be going
	 if (s->animate == NULL)
	    s->animate = (RoomAnimate *) SafeMalloc(sizeof(RoomAnimate));

	 s->flags |= SF_FLICKER;
	 RoomSetupFlickerAnimation(s->animate, s->light, sector_num);
	 break;

      case SL_FLICKER_OFF:
	 s->flags &= ~SF_FLICKER;
	 SafeFree(s->animate);
	 s->animate = NULL;
	 break;
      }
   }
   RedrawAll();
}
/************************************************************************/
/* 
 * MoveSector:  Move sector(s) with given ID # to given height at given speed.
 *   type gives type of animation (ANIMATE_FLOOR_LIFT or ANIMATE_CEILING_LIFT).
 */
void MoveSector(BYTE type, WORD sector_num, WORD height, BYTE speed)
{
   int i;
   Sector *s;
   RoomLift *lift;

//   debug(("MoveSector got sector %d to move to height %d at speed %d (type %d))\n",
//	   (int) sector_num, (int) height, (int) speed, (int) type);

   for (i=0; i < current_room.num_sectors; i++)
   {
      s = &current_room.sectors[i];
      if (s->server_id != sector_num)
	 continue;

	  s->flags |= SF_HAS_ANIMATED;

      // Special case:  if speed = 0, perform lift immediately
      if (speed == 0 || !config.animate)
      {
	 SectorAdjustHeight(&current_room, s, type, HeightKodToClient(height));
	 RedrawAll();
	 continue;
      }

      // Set up animation structure
      if (s->animate == NULL)
	 s->animate = (RoomAnimate *) SafeMalloc(sizeof(RoomAnimate));

      s->animate->animation = type;
      lift = &s->animate->u.lift;

      switch (type)
      {
      case ANIMATE_FLOOR_LIFT:
	 lift->source_z = s->floor_height;
	 break;

      case ANIMATE_CEILING_LIFT:
	 lift->source_z = s->ceiling_height;
	 break;

      default:
	 debug(("MoveSector got unknown animation type %d\n", (int) type));
	 SafeFree(s->animate);
	 s->animate = NULL;
	 return;
      }

      lift->z = lift->source_z;
      lift->dest_z = HeightKodToClient(height);
      lift->progress = (float) 0.0;

      // Check for not moving at all
      if (lift->source_z == lift->dest_z)
      {
	 SafeFree(s->animate);
	 s->animate = NULL;
	 return;
      }

      lift->increment = (((float) HeightKodToClient(speed)) / 1000.0) /
	 abs(lift->dest_z - lift->source_z);
   }
}
/************************************************************************/
/*
 * RoomAnimateSingle:  Animate the given room animation structure.  
 *   dt is number of milliseconds since last time animation timer went off.
 *   Returns:
 *   RAS_NONE    if no bitmap change took place.
 *   RAS_ANIMATE if the bitmap group changed, and the animation is continuing.
 *   RAS_DONE    if the bitmap group changed, and the animation is done.
 */
int RoomAnimateSingle(RoomAnimate *ra, int dt)
{
   RoomLift *lift;
   RoomScroll *scroll;
   RoomFlicker *flicker;
   int dx, dy, step;

   switch (ra->animation)
   {
   case ANIMATE_ROOM_BITMAP:
      if (AnimateSingle(&ra->u.bitmap.a, 0, dt) == False)
	 return RAS_NONE;

      if (ra->u.bitmap.a.animation == ANIMATE_NONE)
	 return RAS_DONE;
      return RAS_CHANGED;
      
   case ANIMATE_FLOOR_LIFT:
   case ANIMATE_CEILING_LIFT:
      lift = &ra->u.lift;
      lift->progress += (lift->increment * dt);
      if (lift->progress >= 1.0)
	 lift->z = lift->dest_z;
      else
	 lift->z = FloatToInt (lift->source_z + lift->progress * (lift->dest_z - lift->source_z));
      
      if (lift->progress >= 1.0)
	 return RAS_DONE;
      return RAS_CHANGED;

   case ANIMATE_SCROLL:
      dx = 0;
      dy = 0;
      scroll = &ra->u.scroll;
      scroll->tick = scroll->tick - dt;

      if (scroll->tick > 0)
	 return RAS_NONE;

      // Amount to move texture offsets; account for slow machines by skipping frames
      step = 1;
      if (scroll->period != 0)
	 step = step - scroll->tick / scroll->period;

      scroll->tick = scroll->period;
      
      switch (scroll->direction)
      {
      case SCROLL_N:  dx = 0;       dy = -step;   break;
      case SCROLL_NE: dx = -step;   dy = -step;   break;
      case SCROLL_E:  dx = -step;   dy = 0;       break;
      case SCROLL_SE: dx = -step;   dy = step;    break;
      case SCROLL_S:  dx = 0;       dy = step;    break;
      case SCROLL_SW: dx = step;    dy = step;    break;
      case SCROLL_W:  dx = step;    dy = 0;       break;
      case SCROLL_NW: dx = step;    dy = -step;   break;
      default:
	 debug(("RoomAnimateSingle got unknown scroll type %d\n", scroll->direction));
	 break;
      }
      scroll->xoffset += dx;
      scroll->yoffset += dy;

      return RAS_CHANGED;

   case ANIMATE_FLICKER:
      flicker = &ra->u.flicker;
      flicker->tick = flicker->tick - dt;

      if (flicker->tick > 0)
	 return RAS_NONE;

      flicker->tick = flicker->period;

      // Account for ambient light bit
	  if (!gD3DEnabled)
	  {
      if (flicker->server_id == 0)
	 flicker->light = (flicker->original_light & 0x7F) + 
	    rand() % 
	    min(FLICKER_INTENSITY, (0x80 - (flicker->original_light & 0x7F)));
      else 
	 // Cause all sectors with the same id to flicker identically
	 flicker->light = (flicker->original_light & 0x7F) + 
	    (flicker_amount * flicker->server_id) % 
	    min(FLICKER_INTENSITY, (0x80 - (flicker->original_light & 0x7F)));
	  }
	  else
	  {
		 flicker->light = (flicker->original_light & 0x7F);
	  }

      flicker->light |= (flicker->original_light & 0x80);
      
      return RAS_CHANGED;

   default:
      debug(("Unknown animation type %d in RoomAnimateSingle\n", ra->animation));
      return RAS_NONE;
   }
   // Can't get here
}
/************************************************************************/
/*
 * SectorAdjustHeight:  Adjust room structures when given sector changes floor or ceiling height.
 *   Sets sector s's floor or ceiling height to "height", according to animation type "type".
 */
void SectorAdjustHeight(room_type *room, Sector *s, BYTE type, int height)
{
   int i;
   list_type l;

   switch (type)
   {
   case ANIMATE_FLOOR_LIFT:
      s->floor_height = height;
      break;

   case ANIMATE_CEILING_LIFT:
      s->ceiling_height = height;
      break;
   }

//   debug(("SectorAdjust height sector %d to height %d\n", s->server_id, height));

   // Adjust height of neighboring walls
   for (i = 0; i < room->num_walls; i++)
   {
      WallData *wall = &room->walls[i];
      
      if (wall->pos_sector == s || wall->neg_sector == s)
	 SetWallHeights(wall);
   }

   // Adjust height of objects in sector, if floor is moving
   if (type == ANIMATE_FLOOR_LIFT)
      for (l = room->contents; l != NULL; l = l->next)
      {
	 room_contents_node *r = (room_contents_node *) (l->data);
	 BSPleaf *leaf = BSPFindLeafByPoint(room->tree, r->motion.x, r->motion.y);
	 
	 if (leaf == NULL || leaf->sector != s)
	    continue;
	 
	 r->motion.z = s->floor_height;
      }
}
