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

extern room_type current_room;

Effects effects;

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

bool IsBlind()
{
   if (effects.blind)
      return TRUE;

   return FALSE;
}

/****************************************************************************/
/*
 * PerformEffect:  Perform given effect #.  Ptr and len are extra bytes from
 *   server message; their interpretation depends on the particular effect.
 *   Return true iff ptr and len are correct for given effect type.
 */
bool PerformEffect(WORD effect, char *ptr, int len)
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
	   effects.shake = duration;
      break;

   case EFFECT_PARALYZE:
      effects.paralyzed = true;
      break;

   case EFFECT_RELEASE:
      effects.paralyzed = false;
      break;

   case EFFECT_BLIND:
      effects.blind = true;
      RedrawAll();
      break;

   case EFFECT_SEE:
      effects.blind = false;
      RedrawAll();
      break;

   case EFFECT_RAINING:
	  effects.raining = true;
	  effects.snowing = false;
	  RedrawAll();
	  break;

   case EFFECT_SNOWING:
	  effects.raining = false;
	  effects.snowing = true;
	  RedrawAll();
	  break;

   case EFFECT_CLEARWEATHER:
	  effects.raining = false;
	  effects.snowing = false;
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
	  effects.sand = true;
	  RedrawAll();
	  break;

   case EFFECT_CLEARSAND:
	  effects.sand = false;
	  RedrawAll();
	  break;

   default:
      debug(("PerformEffect got unknown effect type %d\n", effect));
      return false;
   }
   
   len -= (ptr - start);
   if (len == 0)
      return true;
   return false;
}

/****************************************************************************/
/*  
 * EffectFlash:  Display the main graphics area with an inverted color scheme.
 *   The effect lasts for "duration" milliseconds.
 */
void EffectFlash(int duration)
{
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
bool AnimateEffects(int dt)
{
   bool bRedraw = false;

   if (effects.blur > 0)
   {
      effects.blur = max(0, effects.blur - dt);
      bRedraw = true;
   }

   if (effects.waver > 0)
   {
      effects.waver = max(0, effects.waver - dt);
      bRedraw = true;
   }

   if (effects.pain > 0)
   {
      effects.pain = max(0, effects.pain - dt);
      bRedraw = true;
   }

   if (effects.whiteout > 0)
   {
      effects.whiteout = max(0, effects.whiteout - dt);
      bRedraw = true;
   }

   if (effects.invert > 0)
   {
      effects.invert = max(0, effects.invert - dt);
      bRedraw = true;
   }

   if (effects.shake > 0)
   {
      effects.shake = max(0, effects.shake - dt);
      EffectShake();
      bRedraw = true;
   }

   return bRedraw;
}

/************************************************************************/
/*
 * WeatherEnterRoom: Checks if the room has a weather effect that differs
 *   from the client's weather, and updates the client weather accordingly.
 */
void WeatherEnterRoom()
{
	if (current_room.weather_effect == WEATHER_CLEAR)
	{
		effects.raining = false;
		effects.snowing = false;
		RedrawAll();
	}	
	
	if (current_room.weather_effect == WEATHER_RAIN && effects.raining == false)
	{
		effects.raining = true;
		effects.snowing = false;
		RedrawAll();
	}
	
	if (current_room.weather_effect == WEATHER_SNOW && effects.snowing == false)
	{
		effects.raining = false;
		effects.snowing = true;
		RedrawAll();
	}	
	
	return;
}

/************************************************************************/

bool IsClearWeather()
{
	if (current_room.weather_effect == WEATHER_CLEAR)
	{
		return TRUE;
	}
	return FALSE;
}
