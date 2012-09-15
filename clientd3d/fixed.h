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

extern FixedPoint tempFP;
extern long tempLong;

__inline FixedPoint Dbl2FP(double val)
{
   val = val * (double)FIXED_ONE + 0.5;
   __asm {
      fld   val;
      fistp dword ptr tempFP;
   }
   return tempFP;
}

__inline long Dbl2Long(double val)
{
   __asm {
      fld   val;
      fistp dword ptr tempLong;
   }
   return tempLong;
}

__inline int FloatToInt(float val)
{
   __asm {
      fld   val;
      fistp dword ptr tempLong;
   }
   return tempLong;
}

#define INT_TO_FIXED(x)         ((x) << FIXED_POINT_PRECISION)
//#define DOUBLE_TO_FIXED(x)      ((long)((x) * (double)FIXED_ONE + 0.5))
#define DOUBLE_TO_FIXED(x)	(Dbl2FP(x))
#define FIXED_TO_INT(x)         ((x) >> FIXED_POINT_PRECISION)
#define FIXED_TO_DOUBLE(x)      (((double)(x)) / (double)FIXED_ONE)
#define ROUND_FIXED_TO_INT(x)   (((x) + FIXED_ONE_HALF) >> FIXED_POINT_PRECISION)

// difference between fixed point precision & FINENESS
// (fails utterly if fixed precision is larger)
#define BASE_DIF (LOG_FINENESS-FIXED_POINT_PRECISION)


long mulDiv(long value, long mulBy, long divBy);
FixedPoint fpDiv(FixedPoint d1, FixedPoint d2);
FixedPoint fpMul(FixedPoint m1, FixedPoint m2);
FixedPoint fpInverse(FixedPoint x);
FixedPoint fpSquare(FixedPoint x);
FixedPoint fpSqrtFast(FixedPoint x);
FixedPoint fpSqrtSlow(FixedPoint x);
FixedPoint fpSqrtSlowest(FixedPoint x);
FixedPoint fpSqrt(FixedPoint x);

int intATan2(int dy, int dx);
long GetLongSqrt(long value);
float GetFloatSqrt(float value);
int Distance(int dx, int dy);
BOOL IsPower2(int val);
