// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* moveobj.c:  Move objects other than the player.
*
* We occasionally hear from the server that an object has moved.  When this happens, we
* set up the "motion" structure in the room object for smooth motion from its current position
* to its new position.  If the object was already in motion, we adjust the motion to end
* at the new position and adjust the speed accordingly.
*/

#include "client.h"

#define OBJECT_BOUNCE_HEIGHT  (FINENESS >> 4)
#define TIME_FULL_OBJECT_BOUNCE 2000

extern player_info player;
extern room_type current_room;

static void MoveSingleVertically(Motion *m, int dt);
/************************************************************************/
/*  
* MoveObject2:  Server has told us to move given object to given coordinates.
	Now performs auto-correction whenever something (lag/incorrect speed) causes it
	to fall behind or ahead - mistery
*/
void MoveObject2(ID object_id, int x, int y, BYTE speed, BOOL turnToFace)
{
	room_contents_node *r;
	int dx, dy, dz, z;
	BOOL hanging;
	
	r = GetRoomObjectById(object_id);
	
	if (r == NULL)
	{
		debug(("Couldn't find object #%ld to move\n", object_id));
		return;
	}
	
	hanging = (r->obj.flags & OF_HANGING);
	z = GetFloorBase(x,y);
	if (turnToFace)
	{
		int angle = -1;
		dx = x - r->motion.x;
		dy = y - r->motion.y;
		/* Turn object to face direction traveling */
		if (dy > 0)
		{
			if (dx > 0)
				angle = 1;
			else if (dx < 0)
				angle = 3;
			else
				angle = 2;
		}
		else if (dy < 0)
		{
			if (dx > 0)
				angle = 7;
			else if (dx < 0)
				angle = 5;
			else
				angle = 6;
		}
		else // dy = 0
		{
			if (dx > 0)
				angle = 0;
			else if (dx < 0)
				angle = 4;
		}
		if (angle > -1)
			TurnObject(object_id,(WORD)(angle * MAX_ANGLE / 8));
	}
	
	RedrawAll();
	
	/* Player moves specially */
	if (object_id == player.id)
	{
		player.x = x;
		player.y = y;
		
		r->motion.x = x;
		r->motion.y = y;
		
		RoomObjectSetHeight(r);
		ServerMovedPlayer();
		
		// Don't interpolate or animate our own motion
		return;
	}

	// If animation off, don't interpolate motion
	if (speed == 0 || !config.animate)
	{
		r->motion.x = x;
		r->motion.y = y;
		RoomObjectSetHeight(r);
		ObjectStopAnimation(&r->obj);
		r->moving = False;
		return;
	}
	
	// Set up interpolated motion
	// See how far we should move per frame
	r->motion.progress = 0.0;
	
	// If already in motion, set things up so that combined motions will end at the same
	// time that new motion would end if existing motion were not present.
	if (r->moving)
	{
      float old_remaining, new_remaining, speed_factor;

      if (r->motion.speed < speed)
         r->motion.speed = speed;
      
		// If we're more than a tile behind where we should be, increase speed
      // by the ratio of
      // (new distance remaining to move) / (old distance remaining to move)
		dx = r->motion.x - x;
		dy = r->motion.y - y;
		new_remaining = GetFloatSqrt((float)(dx * dx + dy * dy)) / (float)FINENESS;

		if (new_remaining > 1.0f)
		{
			dx = r->motion.dest_x - r->motion.source_x;
			dy = r->motion.dest_y - r->motion.source_y;
			old_remaining = GetFloatSqrt((float)(dx * dx + dy * dy)) / (float)FINENESS;

			if (old_remaining == 0)
				old_remaining = 0.00001f;

			speed_factor = new_remaining / old_remaining;
			
			if (speed_factor > 1)
				r->motion.speed *= speed_factor;
		}
	}
	else
	{
		r->motion.speed = speed;
	}
   
   r->motion.source_x = r->motion.x;
   r->motion.source_y = r->motion.y;
   r->motion.source_z = r->motion.z;

	dx = x - r->motion.source_x;
	dy = y - r->motion.source_y;
	dz = z - r->motion.source_z;

	r->motion.dest_x = x;
	r->motion.dest_y = y;
	r->motion.dest_z = z;
	
	if (dx == 0 && dy == 0)
		r->motion.increment = 1.0;
	else 
	{
		float distance = GetFloatSqrt((float)(dx * dx + dy * dy)) / (float)FINENESS;
		// Object motion is given in # of grid squares per 10 seconds
		r->motion.increment = (((float) r->motion.speed) / 10000.0f) / distance;
	}
	
	r->motion.move_animating = True;
	r->moving = True;
	
	if (hanging)
	{
		RoomObjectSetHeight(r);
		r->motion.v_z = 0;
	}
	else if (z < r->motion.source_z)
		r->motion.v_z = FALL_VELOCITY_0;
	else if (z > r->motion.source_z)
		r->motion.v_z = CLIMB_VELOCITY_0;
	
	RoomObjectSetAnimation(r, True);
}
/************************************************************************/
/*  
* ObjectsMove:  Called when animation timer goes off.  Incrementally move
*   objects whose motion is interpolated.
*   dt is number of milliseconds since last time animation timer went off.
*   Return True iff at least one object was moved.
*/
Bool ObjectsMove(int dt)
{
	list_type l;
	Bool retval = False;
	
	for (l = current_room.contents; l != NULL; l = l->next)
	{
		room_contents_node *r = (room_contents_node *) (l->data);
		
		if (r->obj.boundingHeight == 0)
		{
			int width,height;
			if(GetObjectSize(r->obj.icon_res, r->obj.animate->group, 0, *(r->obj.overlays), 
				&width, &height))
			{
				r->obj.boundingHeight = height;
				r->obj.boundingWidth = width;
			}
		}
		if (r->moving)
		{
			retval = True;
			if (MoveSingle(&r->motion, dt))
			{
				// Object has finished its interpolated move
				r->moving = False;
				
				// Restore original animation, if using move animation
				if (r->motion.move_animating)
					RoomObjectSetAnimation(r, False);
				
				r->motion.move_animating = False;
			}
			
			// Rest object on floor if it's not moving vertically
			if (r->motion.v_z == 0)
				RoomObjectSetHeight(r);
		}
		if (OF_BOUNCING == (OF_BOUNCING & r->obj.flags))
		{
			if (!(OF_PLAYER & r->obj.flags))
			{
				int floor,ceiling,angleBounce,bounceHeight;
				r->obj.bounceTime += min(dt,40);
				if (r->obj.bounceTime > TIME_FULL_OBJECT_BOUNCE)
					r->obj.bounceTime -= TIME_FULL_OBJECT_BOUNCE;
				angleBounce = NUMDEGREES * r->obj.bounceTime / TIME_FULL_OBJECT_BOUNCE;
				bounceHeight = FIXED_TO_INT(fpMul(OBJECT_BOUNCE_HEIGHT, SIN(angleBounce)));
				if (GetPointHeights(r->motion.x,r->motion.y,&floor,&ceiling))
				{
					//int midPoint = floor + ((ceiling-floor)>>1);
					r->motion.z = floor + OBJECT_BOUNCE_HEIGHT + bounceHeight;
				}
				retval = True;
			}
		}
		if (r->motion.v_z != 0)
		{
			retval = True;
			MoveSingleVertically(&r->motion, dt);
		}
	}
	return retval;
}
/************************************************************************/
/*
* MoveSingle:  Move object described by given motion structure along its
*   path.  Return True if object has reached the end of its motion.
*   dt is number of milliseconds since last time animation timer went off.
*/
Bool MoveSingle(Motion *m, int dt)
{
	m->progress += (m->increment * dt);
	if (m->progress >= 1.0)
	{
		m->x = m->dest_x;
		m->y = m->dest_y;
		m->z = m->dest_z;
		return True;
	}
	
	m->x = FloatToInt(m->source_x + m->progress * (m->dest_x - m->source_x));
	m->y = FloatToInt(m->source_y + m->progress * (m->dest_y - m->source_y));
	m->z = FloatToInt(m->source_z + m->progress * (m->dest_z - m->source_z));

	return False;
}
/************************************************************************/
/*
* MoveSingleVertically:  Move object described by given motion structure 
*   vertically up or down.  Simulates gravity on falling objects.
*   dt is number of milliseconds since last time animation timer went off.
*/

double gravityAdjust = 1.0;

void MoveSingleVertically(Motion *m, int dt)
{
	int dz = dt * m->v_z / 1000;
	
	m->z += FloatToInt((double)dz * gravityAdjust);
	if (dz > 0)   // Rising
	{
		if (m->z >= m->dest_z)
		{
			// Reached destination height; stop rising
			m->z = m->dest_z;
			m->v_z = 0;
		}
	}
	else          // Falling
	{
		if (m->z <= m->dest_z)
		{
			// Reached destination height; stop falling
			m->z = m->dest_z;
			m->v_z = 0;
		}
		else
		{
			// Constant acceleration of gravity
			m->v_z += FloatToInt(gravityAdjust * (double)(GRAVITY_ACCELERATION * dt / 1000));
		}
	}
}

