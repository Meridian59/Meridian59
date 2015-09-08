// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * adminfn.h
 *
 */

#ifndef _ADMINFN_H
#define _ADMINFN_H

void SendSessionAdminText(int session_id,const char *fmt,...);
void TryAdminCommand(int session_id,char *admin_command);

void AdminBufferSend(char *buf,int len_buf);

void AdminDeleteEachUserObject(user_node *u);

#endif
