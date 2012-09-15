// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * rops.h:  Header for rops.c
 */

#ifndef _ROPS_H
#define _ROPS_H

/***************************************************************************/

M59EXPORT void RainDib(BYTE* pabyBits, int width, int height, int level, int heading, int windheading, int windstrength, int torch);
M59EXPORT void SnowDib(BYTE* pabyBits, int width, int height, int level, int heading, int windheading, int windstrength, int torch);
M59EXPORT void BlurDib(BYTE* pabyBits, int width, int height, int amount);
M59EXPORT void WaverDib(BYTE* pabyBits, int width, int height, int amount);
M59EXPORT void SandDib(BYTE* pabyBits, int width, int height, int level);

/***************************************************************************/

#endif /* #ifndef _ROPS_H */
