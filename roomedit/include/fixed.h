//----------------------------------------------
//
// Declarations for Fixed point math routines
//

typedef long FixedPoint;

#define FIXED_POINT_PRECISION  8L

#define FIXED_ONE		(1 << FIXED_POINT_PRECISION)
#define FIXED_ONE_HALF		(1 << (FIXED_POINT_PRECISION-1))

#define INT_TO_FIXED(x)         ((x) << FIXED_POINT_PRECISION)
#define DOUBLE_TO_FIXED(x)      ((long)((x) * (double)FIXED_ONE + 0.5))
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
