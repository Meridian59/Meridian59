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


FixedPoint tempFP;
long tempLong;

// disable warning for missing return statements
//  inline asm functions don't need them.
#pragma warning( disable : 4035 )

//----------------------------------------------
//
// Multiply two 32 bit numbers giving a 64 bit
// result then divide the 64 bit number by a 32 
// bit value and return the result.
//

long mulDiv(long value, long mulBy, long divBy)
{
   __asm 
   {
      mov   eax, value;
      mov   ecx, mulBy;
      mov   ebx, divBy;
      imul  ecx;
      idiv  ebx;
   }
}

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

//----------------------------------------------
//
// Calculate 1/x in FixedPoint
//   This is faster than using FixedDiv.
//    <verify this really works>

FixedPoint fpInverse(FixedPoint x) 
{
   __asm 
   {
      mov   ebx, x;
      xor   eax, eax;
      mov   edx, 1;
      idiv  ebx;
   }
}

//----------------------------------------------
//
// Calculate x*x in FixedPoint
//   This is faster than using FixedMul.
// This is faster than using FixedMul for squares.

FixedPoint fpSquare(FixedPoint x) 
{
   __asm 
   {
      mov   eax,x;
      imul  eax;
      add   eax, FIXED_ONE_HALF;
      adc   edx, 0;
      shrd  eax, edx, FIXED_POINT_PRECISION;
   }
}

//----------------------------------------------
//
// Calculate square root of x in FixedPoint
//   faster low precision version
//    seems to assume a 16.16 system need to fix this
FixedPoint fpSqrtFast(FixedPoint x)
{
   __asm 
   {
      mov   ecx, x;
      xor   eax, eax;
      mov   ebx, 40000000h;
sqrtLP1:
      mov   edx, ecx;
      sub   edx, ebx;
      jl    sqrtLP2;
      sub   edx, eax;
      jl    sqrtLP2;
      mov   ecx,edx;
      shr   eax, 1;
      or    eax, ebx;
      shr   ebx, 2;
      jnz   sqrtLP1;
      shl   eax, 8;
      jmp   sqrtLP3;
sqrtLP2: 
      shr   eax, 1;
      shr   ebx, 2;
      jnz   sqrtLP1;
      shl   eax, 8;
sqrtLP3: 
      nop;
   }
}

//----------------------------------------------
//
// Calculate square root of x in FixedPoint
//   slower higher precision version
//    seems to assume a 16.16 system need to fix this
FixedPoint fpSqrtSlow(FixedPoint x) {

    _asm {
	mov ecx,x
         xor eax, eax
         mov ebx, 40000000h
sqrtHP1: mov edx, ecx
         sub edx, ebx
         jb  sqrtHP2
         sub edx, eax
         jb  sqrtHP2
         mov ecx,edx
         shr eax, 1
         or  eax, ebx
         shr ebx, 2
         jnz sqrtHP1
         jz  sqrtHP5
sqrtHP2: shr eax, 1
         shr ebx, 2
         jnz sqrtHP1
sqrtHP5: mov ebx, 00004000h
         shl eax, 16
         shl ecx, 16
sqrtHP3: mov edx, ecx
         sub edx, ebx
         jb  sqrtHP4
         sub edx, eax
         jb  sqrtHP4
         mov ecx, edx
         shr eax, 1
         or  eax, ebx
         shr ebx, 2
         jnz sqrtHP3
         jmp sqrtHP6
sqrtHP4: shr eax, 1
         shr ebx, 2
         jnz sqrtHP3
sqrtHP6: nop
    }
}

//----------------------------------------------
//
// Calculate square root of x in FixedPoint
//   slowest highest precision version
//    use this for now...
//
FixedPoint fpSqrtSlowest(FixedPoint x) 
{
   double r = FIXED_TO_DOUBLE(x);

   r = sqrt(r);
   x = DOUBLE_TO_FIXED(r);

   return(x);
}

#define CIRCLE (NUMDEGREES)
#define CIRCLE2 (NUMDEGREES >> 1)
#define CIRCLE4 (NUMDEGREES >> 2)
#define CIRCLE8 (NUMDEGREES >> 3)

int intATan2(int dy, int dx)
{
#if 1
   return RadToDeg(atan2((float) dy,dx));
#else
   int dxAbs = abs(dx);
   int dyAbs = abs(dy);
   int dxSgn = dx >= 0;
   int dySgn = dy >= 0;
   int angle;
   
   if (0 == dx)
   {
      return 0;
   }
   else if (0 == dy)
   {
      if (dx > 0)
	 return CIRCLE4;
      else
	 return -CIRCLE4;
   }
   
   if (dxAbs > dyAbs)
   {
      angle = CIRCLE4 - (dyAbs * CIRCLE8 / dxAbs);
   }
   else if (dxAbs < dyAbs)
   {
      angle = dxAbs * CIRCLE8 / dyAbs;
   }
   else // (dxAbs == dyAbs)
   {
      angle = CIRCLE8;
   }
   if (dx < 0)
   {
      angle = -angle;
   }
   if (dy < 0)
   {
      if (dx >= 0)
	 angle += CIRCLE2;
      else 
	 angle += -CIRCLE2;
   }
   return angle;
#endif
}

long GetLongSqrt(long value)
{
   return Dbl2Long(sqrt((double)value));
}

float GetFloatSqrt(float value)
{
   return (float)sqrt((double)value);
}

FixedPoint fpSqrt(FixedPoint x)
{
   return fpSqrtSlowest(x);
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

BOOL IsPower2(int val)
{
   __asm {
      mov   eax,val;
      bsf   ebx,eax;
      bsr   ecx,eax;
      cmp   ebx,ecx;
      je    IS_POWER_2;
      mov   eax,FALSE;
      jmp   DONE;
IS_POWER_2:
      mov   eax,TRUE;
DONE:
   }
}

