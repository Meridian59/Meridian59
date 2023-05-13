// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//----------------------------------------------
//
// Declarations for Fixed point math routines
//

typedef long FixedPoint;

#define FIXED_POINT_PRECISION  8L

#define FIXED_ONE		(1 << FIXED_POINT_PRECISION)
#define FIXED_ONE_HALF		(1 << (FIXED_POINT_PRECISION-1))

extern long tempLong;

__inline int FloatToInt(float val)
{
   __asm {
      fld   val;
      fistp dword ptr tempLong;
   }
   return tempLong;
}

#define INT_TO_FIXED(x)         ((x) << FIXED_POINT_PRECISION)
#define FIXED_TO_INT(x)         ((x) >> FIXED_POINT_PRECISION)
#define FIXED_TO_DOUBLE(x)      (((double)(x)) / (double)FIXED_ONE)
#define ROUND_FIXED_TO_INT(x)   (((x) + FIXED_ONE_HALF) >> FIXED_POINT_PRECISION)

// difference between fixed point precision & FINENESS
// (fails utterly if fixed precision is larger)
#define BASE_DIF (LOG_FINENESS-FIXED_POINT_PRECISION)
#define BASE_DIF_FACTOR (1 << BASE_DIF)

FixedPoint fpDiv(FixedPoint d1, FixedPoint d2);
FixedPoint fpMul(FixedPoint m1, FixedPoint m2);

int intATan2(int dy, int dx);
int Distance(int dx, int dy);
