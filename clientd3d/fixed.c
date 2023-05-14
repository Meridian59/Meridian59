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

// disable warning for missing return statements
//  inline asm functions don't need them.
#pragma warning( disable : 4035 )

//----------------------------------------------
//
// Divide two fixed point numbers giving
// a fixed point result.
//

FixedPoint fpDiv(FixedPoint d1, FixedPoint d2) 
{
   __asm 
   {
      mov   eax, d1;
      mov   ebx, d2;
      cdq;
      shld  edx, eax, FIXED_POINT_PRECISION;
      sal   eax, FIXED_POINT_PRECISION;
      idiv  ebx;
   }
}

//----------------------------------------------
//
// Multiply two fixed point numbers giving
// a fixed point result.
//

FixedPoint fpMul(FixedPoint m1, FixedPoint m2) 
{
   __asm 
   {
      mov   eax, m1;
      mov   edx, m2;
      imul  edx;
      add   eax, FIXED_ONE_HALF;
      adc   edx, 0;
      shrd  eax, edx, FIXED_POINT_PRECISION;
   }
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
