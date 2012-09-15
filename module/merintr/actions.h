// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * actions.h:  Header file for actions.c
 */

#ifndef _ACTIONS_H
#define _ACTIONS_H

#define ID_ACTION 4000        // Windows base ID for action menu items
#define MAX_ACTIONS 50

void ActionsInit(void);
void ActionsExit(void);

void MenuActionChosen(int id);

#endif /* #ifndef _ACTIONS_H */
