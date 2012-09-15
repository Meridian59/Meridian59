// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * gameuser.h:  Header file for gameuser.c
 */

#ifndef _GAMEUSER_H
#define _GAMEUSER_H

#define CLOSE_DISTANCE (5 * FINENESS)  // Objects must be this close for get, put, etc.

list_type UserInventoryList(HWND hwnd, char *title);
void UserAttack(int action);
void UserAttackClosest(int action);
void UserLookMouseSquare(void);
void UserPickup(void);
void UserInventory(void);
void UserLook(void);
void UserLookMouseSquare(void);
void UserDrop(void);
void UserPut(void);
void UserUse(void);
void UserUnuse(void);
void UserToggleMusic(Bool music_on);
void UserMakeOffer(void);
void Offered(list_type items);
void UserApply(void);
M59EXPORT void StartApply(ID id);
void UserBuy(void);
void UserDeposit(void);
void UserWithdraw(void);
void UserQuit(void);
void UserLookInside(void);
void UserActivateMouse(void);
void UserActivate(void);
void UserTargetNextOrPrevious( Bool bTargetNext );
void SetUserTargetID( ID idTargetNew );
ID   GetUserTargetID();
void UserMouselookToggle(void);
Bool UserMouselookIsEnabled(void);
void UserTargetSelf(void);

void GotObjectContents(ID object_id, list_type contents);

#endif /* #ifndef _GAMEUSER_H */
