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
//typedef unsigned long fix;
#define FIX_INT     16   /* # of bits in integer part */
#define FIX_DECIMAL 16   /* # of bits in decimal part */

#define LongToFix(x)  (((long) (x)) << FIX_DECIMAL)
/* Round fix to nearest long */
#define FixToLong(x)  ((((long) (x)) + (1L << (FIX_DECIMAL - 1))) >> FIX_DECIMAL)
/* Truncate decimal */
#define FixToLongTrunc(x)  (((long) (x)) >> FIX_DECIMAL)

#define FloatToFix(x) ((fix) ((x) * (1L << FIX_DECIMAL)))
#define FixToFloat(x) (((float) (x)) / (1L << FIX_DECIMAL))


#define BIGFIXINT LongToFix(200L)  /* For "infinite" tan and cot values */

// Ignore lack of return value
#pragma warning(disable:4035)

/************************************************************************/
__inline fix FixMul(int x, int y)
{
   _asm                         
   {                           
      mov eax,x              
      imul y                 
      shrd eax,edx,16        
   }                           
}

__inline fix FixDiv(int x, int y)
{
   _asm                      
   {                         
      mov eax,x              
      mov edx,eax            
      sar edx,16             
      shl eax,16             
      idiv y                 
      shrd edx,eax,16        
   }                           
}

#pragma warning(default:4035)

#endif /* #ifndef _FIXEDPT_H */
