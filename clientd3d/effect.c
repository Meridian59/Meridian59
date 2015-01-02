// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * effect.c:  Cool special effects for use with the game.
 */

#include "client.h"

#define SHAKE_AMPLITUDE (FINENESS / 4)  // size of player motion for shake effect (FINENESS units)

Effects effects;

extern Bool gD3DRedrawAll;

static void EffectFlash(int duration);
/****************************************************************************/
/*
 * EffectsInit:  Initialize effects when game is entered.
 */
void EffectsInit(void)
{
   ZeroMemory(&effects,sizeof(effects));
   effects.flashxlat = XLAT_IDENTITY;
}
/****************************************************************************/
/*
 * EffectsExit:  Stop effects when game is exited.
 */
void EffectsExit(void)
{
   ZeroMemory(&effects,sizeof(effects));
}

/****************************************************************************/

Bool IsBlind()
{
   if (effects.blind)
      return TRUE;

   return FALSE;
}

/****************************************************************************/
/*
 * PerformEffect:  Perform given effect #.  Ptr and len are extra bytes from
 *   server message; their interpretation depends on the particular effect.
 *   Return True iff ptr and len are correct for given effect type.
 */
Bool PerformEffect(WORD effect, char *ptr, int len)
{
   int duration, xlat;
   char *start = ptr;

   switch (effect)
   {
   case EFFECT_XLATOVERRIDE:
      Extract(&ptr,&xlat,4);
      effects.xlatOverride = xlat;
      break;

   case EFFECT_INVERT:
      Extract(&ptr, &duration, 4);
      EffectFlash(duration);
      break;

   case EFFECT_SHAKE:
      Extract(&ptr, &duration, 4);
      if (config.animate)
	 effects.shake = duration;
      break;

   case EFFECT_PARALYZE:
      effects.paralyzed = True;
      break;

   case EFFECT_RELEASE:
      effects.paralyzed = False;
      break;

   case EFFECT_BLIND:
      effects.blind = True;
      RedrawAll();
      break;

   case EFFECT_SEE:
      effects.blind = False;
      gD3DRedrawAll |= D3DRENDER_REDRAW_ALL;
      RedrawAll();
      break;

   case EFFECT_RAINING:
	  effects.raining = True;
	  RedrawAll();
	  break;

   case EFFECT_SNOWING:
	  effects.snowing = True;
	  RedrawAll();
	  break;

   case EFFECT_FIREWORKS:
	  effects.fireworks = True;
	  RedrawAll();
	  break;

   case EFFECT_CLEARWEATHER:
	  effects.raining = False;
	  effects.snowing = False;
     effects.fireworks = False;
	  RedrawAll();
	  break;

   case EFFECT_PAIN:
	  Extract(&ptr, &duration, 4);
	  if (duration > 10000 || duration < 0)
		duration = 10000;
	  effects.pain = duration;
	  RedrawAll();
	  break;

   case EFFECT_WHITEOUT:
	  Extract(&ptr, &duration, 4);
	  if (duration > 10000 || duration < 0)
	    duration = 10000;
	  effects.whiteout = duration;
	  RedrawAll();
	  break;

   case EFFECT_FLASHXLAT:
	  /* duration field is actually xlat id */
	  Extract(&ptr, &duration, 4);
	  if (duration > 10000 || duration < 0)
		duration = 1000;
	  Extract(&ptr, &xlat, 4);
	  if (xlat < 0 || xlat > 0xFF)
		xlat = XLAT_IDENTITY;
	  effects.flashxlat = xlat;
	  effects.duration = duration;
	  RedrawAll();
	  break;

   case EFFECT_BLUR:
	  Extract(&ptr, &duration, 4);
	  if (duration < 0)
		duration = 10000;
	  effects.blur += duration;
	  if (effects.blur > 200000)
		effects.blur = 200000;
	  RedrawAll();
	  break;

   case EFFECT_WAVER:
	  Extract(&ptr, &duration, 4);
	  if (duration < 0)
		duration = 10000;
	  effects.waver += duration;
	  if (effects.waver > 200000)
		effects.waver = 200000;
	  RedrawAll();
	  break;

   case EFFECT_SAND:
	  effects.sand = True;
	  RedrawAll();
	  break;

   case EFFECT_CLEARSAND:
	  effects.sand = False;
	  RedrawAll();
	  break;

   default:
      debug(("PerformEffect got unknown effect type %d\n", effect));
      return False;
   }
   
   len -= (ptr - start);
   if (len == 0)
      return True;
   return False;
}

/****************************************************************************/
/*  
 * EffectFlash:  Display the main graphics area with an inverted color scheme.
 *   The effect lasts for "duration" milliseconds.
 */
void EffectFlash(int duration)
{
   if (!config.animate)
      return;

   effects.invert = duration;
}
/****************************************************************************/
/*
 * EffectShake:  Jiggle player's view around a little.
 */
void EffectShake(void)
{
   if (effects.shake <= 0)
   {
      if (effects.view_dx || effects.view_dy || effects.view_dz)
         RedrawAll();
      effects.view_dx = 0;
      effects.view_dy = 0;
      effects.view_dz = 0;
   }
   else
   {
      int amplitude = min(SHAKE_AMPLITUDE, effects.shake/3) + 1;
      effects.view_dx = (rand() % amplitude) - (amplitude / 2);
      effects.view_dy = (rand() % amplitude) - (amplitude / 2);
      effects.view_dz = (rand() % amplitude) - (amplitude / 2);
      RedrawAll();
   }
}

/************************************************************************/
/*
 * AnimateEffects: Animate special effects
 *   dt is number of milliseconds since last time animation timer went off.
 */
Bool AnimateEffects(int dt)
{
   Bool bRedraw = False;

   if (effects.blur > 0)
   {
      effects.blur = max(0, effects.blur - dt);
      bRedraw = True;
   }

   if (effects.waver > 0)
   {
      effects.waver = max(0, effects.waver - dt);
      bRedraw = True;
   }

   if (effects.pain > 0)
   {
      effects.pain = max(0, effects.pain - dt);
      bRedraw = True;
   }

   if (effects.whiteout > 0)
   {
      effects.whiteout = max(0, effects.whiteout - dt);
      bRedraw = True;
   }

   if (effects.invert > 0)
   {
      effects.invert = max(0, effects.invert - dt);
      bRedraw = True;
   }

   if (effects.shake > 0)
   {
      effects.shake = max(0, effects.shake - dt);
      EffectShake();
      bRedraw = True;
   }

   return bRedraw;
}
