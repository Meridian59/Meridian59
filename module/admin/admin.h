// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * admin.h:  Header file for admin.c
 */

#ifndef _ADMIN_H
#define _ADMIN_H

#include "adminrc.h"
#include "bkod.h"

#define MODULE_ID        101   // Unique module id #

#define BK_GOTTEXT       (WM_USER + 1000)
#define BK_SENDCMD       (WM_USER + 1001)
#define BK_GOTROW        (WM_USER + 1002)
#define BK_GOTCOL        (WM_USER + 1003)
#define BK_GOTANGLE      (WM_USER + 1004)
#define BK_GOTFINEROW    (WM_USER + 1005)
#define BK_GOTFINECOL    (WM_USER + 1006)
#define BK_RESETDATA     (WM_USER + 1007)

#define MAX_PROPERTYLEN  200       // Maximum length of a line containing a property's value

// Sending messages to server
#define RequestAdminCommand(str)    ToServer(BP_REQ_ADMIN, admin_msg_table, str)
extern client_message admin_msg_table[];

extern HINSTANCE   hInst;          // Handle of this DLL
extern HWND        hAdminDlg;
extern ClientInfo *c;              // Holds data passed from main client
extern Bool        exiting;        // True when module is exiting and should be unloaded

extern char        command[];      // Admin command typed by user
extern int         current_obj;    // Object we're currently displaying, if any

extern BOOL CALLBACK AdminDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK AdminValueDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern void AdminNewLine(char *str);
extern void AdminDisplayOwner(int num);
extern void AdminDisplayObject(int num, char *class_name);
extern void AdminShowProperty(int index);

#endif /* #ifndef _ADMIN_H */

