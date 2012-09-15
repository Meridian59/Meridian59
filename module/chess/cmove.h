// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * cmove.h:  Header file for cmove.c
 */

#ifndef _CMOVE_H
#define _CMOVE_H

Bool ChessMoveIsLegal(Board *b, POINT p1, POINT p2, BYTE color);
void ChessMovePerform(Board *b, POINT p1, POINT p2, Bool interactive);
Bool IsInCheck(Board *b, BYTE color);
Bool HasLegalMove(Board *b, BYTE color);

#endif /* #ifndef _CMOVE_H */
