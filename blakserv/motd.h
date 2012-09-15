// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * motd.c
 *
 */

#ifndef _MOTD_H
#define _MOTD_H

void InitMotd(void);
void LoadMotd(void);
void ResetLoadMotd(void);
void SetMotd(char *new_motd);
int GetMotdLength(void);
char * GetMotd(void);

#endif
