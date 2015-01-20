// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * animate.c
 *
 * This file contains functions for animating objects. 
 *
 * Normally, animation is performed every time we poll the keyboard (i.e. whenever
 * the main window has no events in its queue).  This produces animation as fast as the
 * machine is capable.  However, when a menu or dialog is up, the main window's loop
 * is completely bypassed.  To keep animating during these times, we start an animation
 * timer at a constant rate (as fast as possible).  When the menu or dialog exits, we
 * kill the timer and start polling again.
 *   
 * Each object has a set of bitmaps associated with it.  These bitmaps are
 * divided into groups; for a given object, only one group is active at a time.
 * Animation simply cycles through the groups for each animated object.
 *
 * Animation also produces smooth motion out of position changes sent by the server.
 * This is done in move.c.
 */

#include "client.h"

#define ANIMATE_INTERVAL 165       // ms between background animation updates
#define FLICKER_LEVEL (LIGHT_LEVELS/2)
#define FLASH_LEVEL (LIGHT_LEVELS/2)
#define TIME_FLASH 1000

static int  animation_timer = 0;   // id of animation timer, or 0 if none
static DWORD timeLastFrame;

#define TIME_FULL_OBJECT_PHASE 1800
static int phaseStates[] = {
   DRAWFX_DRAW_PLAIN,DRAWFX_TRANSLUCENT75,DRAWFX_TRANSLUCENT50,DRAWFX_TRANSLUCENT25,DRAWFX_INVISIBLE,
   DRAWFX_INVISIBLE,DRAWFX_INVISIBLE,
   DRAWFX_TRANSLUCENT25,DRAWFX_TRANSLUCENT50,DRAWFX_TRANSLUCENT75,DRAWFX_DRAW_PLAIN};
static int numPhases = sizeof(phaseStates) / sizeof(int);

extern room_type current_room;
extern player_info player;

/* local function prototypes */
static Bool AnimateObjects(int dt);
static Bool AnimateGrids(int dt);
static Bool AnimateProjectiles(int dt);
static Bool AnimateBackgroundOverlays(int dt);
static Bool AnimatePlayerOverlays(int dt);
static void AnimationResetSingle(Animate *a);
/************************************************************************/
void AnimationTimerStart(void)
{
   // See if already started
   if (animation_timer != 0)
      return;

   animation_timer = SetTimer(hMain, TIMER_ANIMATE, ANIMATE_INTERVAL, NULL);
   if (animation_timer == 0)
      ClientError(hInst, hMain, IDS_NOTIMERS);
}
/************************************************************************/
void AnimationTimerAbort(void)
{
   if (animation_timer != 0)
   {
      KillTimer(hMain, TIMER_ANIMATE);
      animation_timer = 0;
   }
}
/************************************************************************/
void AnimationSleep(void)
{
   Sleep(ANIMATE_INTERVAL);
}

DWORD GetFrameTime(void)
{
   return timeLastFrame;
}

/************************************************************************/
void AnimationTimerProc(HWND hwnd, UINT timer)
{
   Bool need_redraw = False;
   static DWORD last_animate_time = 0;
   DWORD dt, now;

   PingTimerProc(hwnd, 0, 0, 0);

   if (!(GameGetState() == GAME_PLAY || GameGetState() == GAME_SELECT))
      return;

   if (last_animate_time == 0)
   {
	   last_animate_time = timeGetTime();
	   return;
   }

   config.quickstart = FALSE;
   now = timeGetTime();
   dt = now - last_animate_time;
   last_animate_time = now;
   timeLastFrame = dt;

   /* Send event to modules */
   ModuleEvent(EVENT_ANIMATE, dt);

   /* Send event to non-module child windows */
   if (config.animate)
   {
      Lagbox_Animate(dt);
   }

   /* Animate the first-person view elements */
   if (config.animate && GetGameDataValid())
   {
      // Avoid short-circuiting OR
      need_redraw |= ObjectsMove(dt);
      need_redraw |= ProjectilesMove(dt);
      need_redraw |= AnimateObjects(dt);
      need_redraw |= AnimateRoom(&current_room, dt);
      need_redraw |= AnimateProjectiles(dt);
      need_redraw |= AnimatePlayerOverlays(dt);
      need_redraw |= AnimateBackgroundOverlays(dt);

      AnimateDescription(dt);

      need_redraw |= AnimateEffects(dt);
      if (need_redraw)
	 RedrawAll();
   }

   if (GetGameDataValid())
      RedrawForce();

   return;
}
/************************************************************************/
/* 
 * Animate objects in current room; return True if any was animated.
 *   dt is number of milliseconds since last time animation timer went off.
 */
Bool AnimateObjects(int dt)
{
   Bool need_redraw = False;
   list_type l;

   for (l = current_room.contents; l != NULL; l = l->next)
   {
      room_contents_node *r = (room_contents_node *) (l->data);
      int old_animate = r->obj.animate->animation;

      need_redraw |= AnimateObject(&r->obj, dt);

      // If room object animation finished, restore motion animation if appropriate.
      // (Done after drawing so that last frame of animation not skipped).
      
      if (r->obj.animate->animation == ANIMATE_NONE && old_animate != ANIMATE_NONE && r->moving)
      {
	 RoomObjectSetAnimation(r, True);
	 r->motion.move_animating = True;
      }
   }
   return need_redraw;
}
/************************************************************************/
/*
 * AnimateObject:  Animate a single object; return True iff animated.
 *   dt is number of milliseconds since last time animation timer went off.
 */

Bool AnimateObject(object_node *obj, int dt)
{
   Bool need_redraw = False;
   list_type over_list;

   if (OF_FLICKERING == (OF_FLICKERING & obj->flags))
   {
      obj->lightAdjust = rand() % FLICKER_LEVEL;
      need_redraw = TRUE;
   }

   if (OF_FLASHING == (OF_FLASHING & obj->flags))
   {
      DWORD angleFlash;
      obj->bounceTime += min(dt,50);
      if (obj->bounceTime > TIME_FLASH)
         obj->bounceTime -= TIME_FLASH;
      angleFlash = NUMDEGREES * obj->bounceTime / TIME_FLASH;
      obj->lightAdjust = FIXED_TO_INT(fpMul(FLASH_LEVEL, SIN(angleFlash)));
      need_redraw = TRUE;
   }

   if (obj->animate->animation != ANIMATE_NONE)
   {
      object_bitmap_type obj_bmap;
      obj_bmap = FindObjectBitmap(obj->icon_res);
      if (obj_bmap != NULL)
	 need_redraw |= AnimateSingle(obj->animate, BitmapsNumGroups(obj_bmap->bmaps), dt);
   }
   
   if (OF_PHASING == (OF_PHASING & obj->flags))
   {
      int anglePhase;
      obj->phaseTime += min(dt,40);
      if (obj->phaseTime > TIME_FULL_OBJECT_PHASE)
         obj->phaseTime -= TIME_FULL_OBJECT_PHASE;
      anglePhase = numPhases * obj->phaseTime / TIME_FULL_OBJECT_PHASE;
      obj->drawingtype = phaseStates[anglePhase];
      need_redraw = TRUE;
   }
   // Animate object's overlays
   for (over_list = *(obj->overlays); over_list != NULL; over_list = over_list->next)
   {
      object_bitmap_type obj_bmap;
      Overlay *overlay = (Overlay *) (over_list->data);
      if (overlay->animate.animation == ANIMATE_NONE)
	 continue;

      obj_bmap = FindObjectBitmap(overlay->icon_res);
      if (obj_bmap != NULL)
      {
	 need_redraw |= AnimateSingle(&overlay->animate, BitmapsNumGroups(obj_bmap->bmaps), dt);
      }	   
   }
   return need_redraw;
}
/************************************************************************/
/* 
 * Animate projectiles active in in current room; return True if any was animated.
 *   dt is number of milliseconds since last time animation timer went off.
 */
Bool AnimateProjectiles(int dt)
{
   object_bitmap_type obj;
   list_type l;
   Projectile *p;
   Bool need_redraw = False, retval;

   for (l = current_room.projectiles; l != NULL; l = l->next)
   {
      p = (Projectile *) (l->data);

      if (p->animate.animation == ANIMATE_NONE)
	 continue;

      obj = FindObjectBitmap(p->icon_res);
      if (obj != NULL)
      {
	 retval = AnimateSingle(&p->animate, BitmapsNumGroups(obj->bmaps), dt);
	 need_redraw = need_redraw || retval;
      }
   }
   return need_redraw;
}
/************************************************************************/
/* 
 * Animate background overlays in current room; return True if any was animated.
 *   dt is number of milliseconds since last time animation timer went off.
 */
Bool AnimateBackgroundOverlays(int dt)
{
   Overlay *overlay;
   list_type l;
   Bool need_redraw = False, retval;
   object_bitmap_type obj_bmap;

   for (l = current_room.bg_overlays; l != NULL; l = l->next)
   {
      overlay = (Overlay *) (l->data);

      if (overlay->animate.animation == ANIMATE_NONE)
	 continue;

      obj_bmap = FindObjectBitmap(overlay->icon_res);
      if (obj_bmap != NULL)
      {
	 retval = AnimateSingle(&overlay->animate, BitmapsNumGroups(obj_bmap->bmaps), dt);
	 need_redraw = need_redraw || retval;
      }
   }
   return need_redraw;
}
/************************************************************************/
/* 
 * Animate player overlays; return True if any was animated.
 *   dt is number of milliseconds since last time animation timer went off.
 */
Bool AnimatePlayerOverlays(int dt)
{
   int i;
   Bool need_redraw = False, retval;

   for (i=0; i < NUM_PLAYER_OVERLAYS; i++)
   {
      PlayerOverlay *poverlay = &player.poverlays[i];

      if (poverlay->obj == NULL || poverlay->hotspot == 0)
	 continue;
      
      retval = AnimateObject(poverlay->obj, dt);
      need_redraw = need_redraw || retval;

      // If animation is over, group becomes -1 => we should remove overlay
      if (poverlay->obj->animate->group == (WORD) -1)
	 poverlay->hotspot = 0;
   }
   return need_redraw;
}
/************************************************************************/
/*
 * AnimateSingle:  Animate the given animation structure for a PDIB with the
 *   given number of groups.  Return True iff the display bitmap changes.
 *   dt is number of milliseconds since last time animation timer went off.
 *   If num_groups is 0, act as if the PDIB has an infinite number of groups.
 */
Bool AnimateSingle(Animate *a, int num_groups, int dt)
{
   Bool need_redraw = False;

   switch (a->animation)
   {
   case ANIMATE_NONE:
      break;
      
   case ANIMATE_CYCLE:
      // See if it's time to change bitmap
      a->tick = a->tick - dt;
      if (a->tick > 0)
	 break;
      
      // Look for special case of cycling through ALL bitmaps
      if (a->group_low == a->group_high)
	 if (num_groups == 0)
	    a->group++;
	 else a->group = (a->group + 1) % num_groups;
      else a->group = a->group_low + 
	 (a->group - a->group_low + 1) % (a->group_high - a->group_low + 1);
      
      // Reset object timer
      a->tick = a->period;
      need_redraw = True;
      break;
      
   case ANIMATE_ONCE:
      // See if it's time to change bitmap
      a->tick = a->tick - dt;
      if (a->tick > 0)
	 break;

      if (a->group == a->group_high)
      {
	 a->animation = ANIMATE_NONE;
	 a->group     = a->group_final;
      }
      else a->group++;

      // Reset object timer
      a->tick = a->period;
      need_redraw = True;
      break;

   default:
      debug(("Unknown animation type %d in AnimateSingle\n", a->animation));
      break;
   }

   if (a->group < 0 || (num_groups > 0 && a->group >= num_groups))
   {
      debug(("Animation produced out of bounds bitmap group %d\n", a->group));
      // Don't fix it up; player overlays rely on group going negative to signal end
      //      a->group = 0;
   }

   return need_redraw;
}

/************************************************************************/
/*
 * VerifyAnimation:  See if given animation should be performed, given whether
 *   animation is currently on.  If animation should be performed, return True
 *   (this may modify given animation structure if a modified animation should
 *   be performed).  If animation shouldn't be done at all, return False.
 */
Bool VerifyAnimation(Animate *a)
{
   if (config.animate)
      return True;

   switch (a->animation)
   {
   case ANIMATE_NONE:
      return True;
      
   case ANIMATE_CYCLE:
      return False;
      
   case ANIMATE_ONCE:
      // Skip right to end frame of animation
      a->animation = ANIMATE_NONE;
      a->group     = a->group_final;
      return True;

   default:
      debug(("Unknown animation type %d in VerifyAnimation\n", a->animation));
      return False;
   }
}
/************************************************************************/
/*
 * AnimateStop:  Stop given animation.
 */
void AnimateStop(Animate *a)
{
   // Stop cycle and one-time animations; leave others alone
   switch (a->animation)
   {
   case ANIMATE_CYCLE:
      a->animation = ANIMATE_NONE;
      a->group     = 0;
      break;

   case ANIMATE_ONCE:
      a->animation = ANIMATE_NONE;
      a->group = a->group_final;
      break;
   }
}
