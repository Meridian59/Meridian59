// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * login.h:  Header file for login.c
 */

#ifndef _LOGIN_H
#define _LOGIN_H

/* Maximum length of username and password */
#define MAXUSERNAME 50
#define MAXPASSWORD 32

/* User types */
enum { USER_NORMAL = 0, USER_ADMIN = 1, USER_GUEST = 2 };

void LoginInit(void);
void LoginExit(void);

void LoginSendInfo(void);
Bool GetLogin(void);
void LoginOk(BYTE type);
void LoginError(int err_string);
void LoginErrorMessage(char *message, BYTE action);
void LoginTimeout(void);
void EnterGame(void);

#endif /* #ifndef _LOGIN_H */
