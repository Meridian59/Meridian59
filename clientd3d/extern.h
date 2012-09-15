// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * extern.h:  External references for client.  This is put in a separate file so
 *   that modules that include client.h don't also include these external references.
 */

#ifndef _EXTERN_H
#define _EXTERN_H

extern Config config;   // User configuration
extern char inihost[];

extern HWND hMain;
extern HINSTANCE hInst;

extern int state;       /* Are we in terminal mode, game mode, or neither? */

extern char *ini_file;  /* Name of private INI file */
extern char *szAppName; /* Name of program */

extern Effects effects; /* State of special effects */

extern client_message user_msg_table[];   // BP_USERCOMMAND message table

#endif /* #ifndef _EXTERN_H */
