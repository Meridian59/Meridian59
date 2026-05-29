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
	  effects.shakeDuration_s = static_cast<float>(duration) * 0.001f;
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
	  RedrawAll();
	  break;

   case EFFECT_SNOWING:
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
	  effects.painDuration_s = static_cast<float>(duration) * 0.001f;
	  RedrawAll();
	  break;

   case EFFECT_WHITEOUT:
	  Extract(&ptr, &duration, 4);
	  if (duration > 10000 || duration < 0)
	    duration = 10000;
	  effects.whiteoutDuration_s = static_cast<float>(duration) * 0.001f;
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
	  effects.flashxlatDuration_s = static_cast<float>(duration) * 0.001f;
	  RedrawAll();
	  break;

   case EFFECT_BLUR:
	  Extract(&ptr, &duration, 4);
	  if (duration < 0)
		duration = 10000;
	  effects.blurDuration_s += static_cast<float>(duration) * 0.001f;
	  effects.blurDuration_s = std::min(effects.blurDuration_s, 200.0f);
	  RedrawAll();
	  break;

   case EFFECT_WAVER:
	  Extract(&ptr, &duration, 4);
	  if (duration < 0)
		duration = 10000;
	  effects.waverDuration_s += static_cast<float>(duration) * 0.001f;
	  effects.waverDuration_s = std::min(effects.waverDuration_s, 200.0f);
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
   effects.invertDuration_s = static_cast<float>(duration) * 0.001f;
}
/****************************************************************************/
/*
 * EffectShake:  Jiggle player's view around a little.
 */
void EffectShake(void)
{
   if (effects.shakeDuration_s <= 0.0f)
   {
      if (effects.view_dx || effects.view_dy || effects.view_dz)
         RedrawAll();
      effects.view_dx = 0;
      effects.view_dy = 0;
      effects.view_dz = 0;
   }
   else
   {
      long shakeDuration_ms = static_cast<long>(effects.shakeDuration_s * 1000.0f);
	  int amplitude = static_cast<int>(std::min(SHAKE_AMPLITUDE, shakeDuration_ms / 3)) + 1;
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
bool AnimateEffects(float dt)
{
   bool bRedraw = false;

   if (effects.blurDuration_s > 0.0f)
   {
      effects.blurDuration_s = std::max(0.0f, effects.blurDuration_s - dt);
      bRedraw = true;
   }

   if (effects.waverDuration_s > 0.0f)
   {
      effects.waverDuration_s = std::max(0.0f, effects.waverDuration_s - dt);
      bRedraw = true;
   }

   if (effects.painDuration_s > 0.0f)
   {
      effects.painDuration_s = std::max(0.0f, effects.painDuration_s - dt);
      bRedraw = true;
   }

   if (effects.whiteoutDuration_s > 0.0f)
   {
      effects.whiteoutDuration_s = std::max(0.0f, effects.whiteoutDuration_s - dt);
      bRedraw = true;
   }

   if (effects.invertDuration_s > 0.0f)
   {
      effects.invertDuration_s = std::max(0.0f, effects.invertDuration_s - dt);
      bRedraw = true;
   }

   if (effects.shakeDuration_s > 0.0f)
   {
      effects.shakeDuration_s = std::max(0.0f, effects.shakeDuration_s - dt);
      EffectShake();
      bRedraw = true;
   }

   return bRedraw;
}
