// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * effect.h:  Header for effect.c
 */

#ifndef _EFFECT_H
#define _EFFECT_H

// Maximum size of blur effect
#define EFFECT_BLUR_AMPLITUDE 6
// # of milliseconds between changing size of blur
#define EFFECT_BLUR_RATE    150

// Hold status of current effects
struct Effects
{
   float  invertDuration_s;    // # of seconds remaining in invert effect (0 if off)
   float  shakeDuration_s;     // # of seconds remaining in shake effect (0 if off)
   float  painDuration_s;      // # of seconds remaining in pain effect (0 if off)
   float  blurDuration_s;      // # of seconds remaining in blur effect (0 if off)
   float  waverDuration_s;     // # of seconds remaining in wavering effect (0 if off)
   float  whiteoutDuration_s;  // # of seconds remaining in whiteout effect (0 if off)

   int  view_dx, view_dy, view_dz;  // Draw player offset by this amount (to shake screen)
   bool paralyzed; // true when user shouldn't be allowed to move
   bool blind;     // true when user can't see anything

   bool raining;   // true when weather is rainy
   bool snowing;   // true when weather is snowy
   bool sand;      // true when in sandstorm

   ID   wadingsound;  // ID of wav resource for wading in current room
   ID   ambientsound; // ID of wav resource played constantly in current room

   int  	flashxlat;
   float	flashxlatDuration_s;
   int  	xlatOverride;
};

void EffectsInit(void);
void EffectsExit(void);
bool IsBlind(void);
bool PerformEffect(WORD effect, char *ptr, int len);
bool AnimateEffects(float dt);

#endif /* #ifndef _EFFECT_H */
