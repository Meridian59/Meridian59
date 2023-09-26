// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * fixedpt.h:  Header file for fixedpt.c
 */

#ifndef _FIXEDPT_H
#define _FIXEDPT_H

/* Definitions for 32 bit fixed-point math */

typedef long fix;

#define FIX_INT     16   /* # of bits in integer part */
#define FIX_DECIMAL 16   /* # of bits in decimal part */

#define LongToFix(x)  (((long) (x)) << FIX_DECIMAL)
/* Round fix to nearest long */
#define FixToLong(x)  ((((long) (x)) + (1L << (FIX_DECIMAL - 1))) >> FIX_DECIMAL)

#define FloatToFix(x) ((fix) ((x) * (1L << FIX_DECIMAL)))

__inline fix FixMul(int x, int y) {
  return (fix) (((__int64) x * (__int64) y) >> FIX_DECIMAL);
}

#endif /* #ifndef _FIXEDPT_H */
