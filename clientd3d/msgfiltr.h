// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * msgfiltr.h:  Header file for msgfiltr.c
 */

#ifndef _MSGFILTR_H
#define _MSGFILTR_H

void UpdateWho(object_node* pUser, BOOL bAdded);
BOOL CALLBACK WhoDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
void AbortWhoDialog(void);

void MessageSaid(ID sender_id, ID sender_name, BYTE say_type, char *message);

void IgnoreListSet(list_type list);
list_type IgnoreListGet(void);

#endif /* #ifndef _MSGFILTR_H */
