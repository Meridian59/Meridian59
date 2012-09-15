// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * guest.h:  Header file for guest.c
 */

#ifndef _GUEST_H
#define _GUEST_H

Bool GuestGetLogin(void);
void GuestConnectError(void);
void GuestLoggingIn(BYTE status, int low, int high);

void GuestMove(HWND hwnd, int x, int y);
void GuestResize(HWND hwnd, UINT resize_flag, int xsize, int ysize);

#endif /* #ifndef _GUEST_H */
