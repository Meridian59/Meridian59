// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//----------------------------------------------
//
// Fixed point math routines
//
#include "client.h"

//----------------------------------------------
//
// Divide two fixed point numbers giving
// a fixed point result.
//

FixedPoint fpDiv(FixedPoint d1, FixedPoint d2) 
{
  return (((__int64) d1 << FIXED_POINT_PRECISION) / d2);
}

//----------------------------------------------
//
// Multiply two fixed point numbers giving
// a fixed point result.
//

FixedPoint fpMul(FixedPoint m1, FixedPoint m2) 
{
  // + FIXED_ONE_HALF presumably for rounding.  This used
  // to be an assembly language function.
  return (m1 * m2 + FIXED_ONE_HALF) >> FIXED_POINT_PRECISION;
}

int intATan2(int dy, int dx)
{
   return RadToDeg(atan2((float) dy,dx));
}

int Distance(int dx, int dy)
{
   int lessor;

   dx = abs(dx);
   dy = abs(dy);
   if (dx > dy)
      lessor = dy;
   else
      lessor = dx;
   return dx + dy - (lessor >> 1);
}
