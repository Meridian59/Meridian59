// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * logoff.c:  Deal with timer that logs user off from game if he hasn't
 *   pressed a key or clicked the mouse in a certain amount of time.
 */

#include "client.h"

#define MS_PER_MINUTE 60000L  /* # of milliseconds per minute */
#define MAXTIMERMS   65530L         /* Max # of milliseconds in a Windows timer delay */

static int timer_id = 0;          /* current timer id, or 0 if none */
static DWORD last_time;           /* Time when user last did something */

void CALLBACK LogoffTimerProc(HWND hwnd, UINT msg, UINT timer, DWORD dwTime);
void LogoffTimerReset(void);

/****************************************************************************/
void UserSetTimeout(void)
{
   DialogBox(hInst, MAKEINTRESOURCE(IDD_TIMEOUT), hMain, TimeoutDialogProc);
}
/****************************************************************************/
BOOL CALLBACK TimeoutDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   static HWND hEnable, hMinutes;
   char temp[5];

   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, GetParent(hDlg));

      hEnable = GetDlgItem(hDlg, IDC_TIMEOUTENABLE);
      hMinutes = GetDlgItem(hDlg, IDC_TIMEOUTMINUTES);

      Edit_LimitText(hMinutes, MAXMINUTES);

      SetWindowFont(hMinutes, GetFont(FONT_INPUT), FALSE);

      if (config.timeout == 0)
      {
	 CheckDlgButton(hDlg, IDC_TIMEOUTENABLE, FALSE);
	 EnableWindow(hMinutes, FALSE);
	 strcpy(temp, "120");  /* Default value */
      }
      else 
      {
	 CheckDlgButton(hDlg, IDC_TIMEOUTENABLE, TRUE);
	 sprintf(temp, "%d", config.timeout);
      }

      Edit_SetText(hMinutes, temp);
      return TRUE;
      
   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {

      case IDOK:
	 /* Get typed # of minutes, if enabled */
	 if (!IsDlgButtonChecked(hDlg, IDC_TIMEOUTENABLE))
	 {
	    config.timeout = 0;
	    LogoffTimerAbort();
	 }
	 else
	 {
	    Edit_GetText(hMinutes, temp, MAXMINUTES);
	    config.timeout = max(atoi(temp), 0);
	    if (state == STATE_GAME)
	       LogoffTimerReset();
	 }
	 UserDidSomething();   /* Reset timer */
	 EndDialog(hDlg, IDOK);
	 return TRUE;

      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
   }
   return FALSE;
}
/****************************************************************************/



/************************************************************************/
/*
 * UserDidSomething:  User pressed a key or clicked mouse; save current time
 *   so that timer doesn't log us off.
 */
void UserDidSomething(void)
{
   last_time = GetTickCount();
}
/************************************************************************/
/*
 * LogoffTimerStart:  Start up a logoff timer for config.timeout minutes.
 */
void LogoffTimerStart(void)
{
   UINT delay;

   if (config.timeout == 0)
      return;

   delay = (UINT) (min((long) ((long) config.timeout * MS_PER_MINUTE), MAXTIMERMS));
   timer_id = SetTimer(NULL, 0, (UINT) delay, LogoffTimerProc);
   if (timer_id == 0)
   {
      ClientError(hInst, hMain, IDS_NOTIMERS);
      return;
   }
   UserDidSomething();  /* Start the clock... */
}
/************************************************************************/
/*
 * LogoffTimerAbort:  Stop currently running logoff timer, if any
 */
void LogoffTimerAbort(void)
{
   if (timer_id != 0)
   {
      KillTimer(NULL, timer_id);
      timer_id = 0;
   }
}
/************************************************************************/
/* 
 * LogoffTimerReset:  If a logoff timer is running, stop it and start a new one.
 */
void LogoffTimerReset(void)
{
   LogoffTimerAbort();
   LogoffTimerStart();
}
/************************************************************************/
void CALLBACK LogoffTimerProc(HWND hwnd, UINT msg, UINT timer, DWORD dwTime)
{
   UINT delay;
   long elapsed;

   KillTimer(NULL, timer_id);

   elapsed = GetTickCount() - last_time;
   if (elapsed >= (long) (config.timeout * MS_PER_MINUTE))
   {
      /* User is history! */
      RequestQuit();
      return;
   }

   /* Restart timer */
   delay = (UINT) (min((long) config.timeout * MS_PER_MINUTE - elapsed, MAXTIMERMS));
   timer_id = SetTimer(NULL, 0, (UINT) delay, LogoffTimerProc);   
   if (timer_id == 0)
      ClientError(hInst, hMain, IDS_NOTIMERS);
}

