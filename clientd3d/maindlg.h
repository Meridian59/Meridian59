// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * maindlg.h:  Header file for maindlg.c
 */

#ifndef _MAINDLG_H
#define _MAINDLG_H

#define MAXMINUTES 5   /* Max # of digits in # of a # of minutes */
#define MAXTIMEOUT 5   /* Max # of characters in timeout edit box */

#define MAXREGISTERSTR 50  /* Max length of a typed string in register dialog */
#define MAXNOTE      2048  /* Max length of a typed string in admin note dialog */

void PasswordOk(void);
void PasswordNotOk(void);
BOOL CALLBACK CommSettingsDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL CALLBACK PasswordDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL CALLBACK RegisterDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL CALLBACK AdminNoteDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL CALLBACK PreferencesDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

void AbortPasswordDialog(void);
void AbortPreferencesDialog(void);
void AbortRegisterDialog(void);
void AbortAdminNoteDialog(void);


#endif /* #ifndef _MAINDLG_H */
