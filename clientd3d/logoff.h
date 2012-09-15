// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * logoff.h:  Header file for logoff.c
 */

#ifndef _LOGOFF_H
#define _LOGOFF_H

int GetLogoffTimeout(void);
void SetLogoffTimeout(int timeout);
BOOL CALLBACK TimeoutDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
void UserSetTimeout(void);

M59EXPORT void UserDidSomething(void);
void LogoffTimerStart(void);
void LogoffTimerAbort(void);


#endif /* #ifndef _LOGOFF_H */
