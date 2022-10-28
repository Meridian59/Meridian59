// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * fastasm.c:  Assembly routines for fast drawing
 */

#include "client.h"

unsigned long l_ecx;

#pragma warning(push )
#pragma warning(disable : 4731)  // ebp modified by inline assembly code

void StretchAsm1To2(BYTE *source,BYTE *dest,int width,int height)
{
	_asm {
	// setup esi and edi to be at source and dest

	mov		esi,[source]
	mov		edi,[dest]
	mov		ecx,[width]
	mov		ebx,ecx
	shr		ecx,1

	mov		eax,[height]

	push	ebp
	mov		[l_ecx],ecx			// get rid of me
	mov		ebp,eax				// can`t use passed in parameters or locals

	mov		eax,0		          // 0000  xxxx

	;---------------------------------------------------------------------------;
Stretch20:
	// the strategy here is load two bytes in at once (ax=HL), play around to
	// get eax = HHLL, and then write all 4 bytes at once

	// we use edx for scratch work area

	// trace:                    eax   edx
	//                           ----  ----
	                          // xxxx  xxxx


	mov		ax,[esi]              // 00HL  xxxx
	add		esi,2

	mov		edx,eax               // 00HL  00HL
	shl		eax,16                // HL00  00HL
	or		eax,edx                // HLHL  00HL
   	shl		edx,8                 // HLHL  0HL0
	and		eax,0FF0000FFh        // H00L  0HL0
	or		eax,edx                // HHLL

	mov		[edi],eax				// this probably isn`t so hot..

	mov		[edi+(MAXX*2)],eax
	add		edi,4
	mov		eax,0

	dec		ecx
	jnz		Stretch20

	// [width] must be a multiple of 2 (andrew said so), so we are done.
	// if it were odd, then we'd have to do a leftover lodsb stosw
	// type of deal here.

	// adjust esi, edi to be at beginning of next read/write lines

	mov		eax,ebx					// get rid  of this (gone) relook at the algo for rewrite

	add		esi,MAXX

	shl		eax,1
	mov		ecx,[l_ecx]				// get rid of me
	sub		esi,ebx
	sub		edi,eax
	add		edi,MAXX*4



	dec		ebp
	jnz		Stretch20

	pop		ebp
  }
}

#pragma warning( pop )
