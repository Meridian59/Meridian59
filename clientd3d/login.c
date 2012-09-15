// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * login.c:  Handle login state, which logs user in and brings up main menu.
 */

#include "client.h"

#define MAXSERVERNUM 3      /* Max # of digits in server number */

static int user_type;       /* Tells if user is administrator, guest, etc.. */
Bool logged_in;             /* True iff we're past login dialog */
extern int connection;
Bool admin_mode;            // True when user wants to go into admin mode

static BOOL CALLBACK LoginDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static HWND GetMessageBoxParent(void);
static void LoginReset(void);

SystemInfo sysinfo;

/****************************************************************************/
/*  
 * LoginInit:  Enter STATE_LOGIN.
 */
void LoginInit(void)
{
}
/****************************************************************************/
/*  
 * LoginExit:  Leave STATE_LOGIN.
 */
void LoginExit(void)
{
}
/****************************************************************************/
/*  
 * LoginSendInfo:  Send login information (username, password, etc.) to server.
 */
void LoginSendInfo(void)
{
   unsigned char buf[ENCRYPT_LEN + 1];

   GetSystemStats(&sysinfo);

   // Encrypt password
   MDString(config.password, buf);
   buf[ENCRYPT_LEN] = 0;

   debug(("Got partner code of %d.\n", (sysinfo.reserved & 0xFF00) >> 8));

   RequestLogin(MAJOR_REV, MINOR_REV, 
		sysinfo.platform, sysinfo.platform_major, sysinfo.platform_minor,
		sysinfo.memory, sysinfo.chip, 
		sysinfo.screen_width, sysinfo.screen_height, 
		sysinfo.color_depth, sysinfo.bandwidth, sysinfo.reserved,
		config.username, buf);
}
/****************************************************************************/
/*
 * LoginOk:  Got a message from the server that login worked.
 */
void LoginOk(BYTE type)
{
   logged_in = True;
   user_type = type;
}
/****************************************************************************/
void LoginTimeout(void)
{
   config.quickstart = FALSE;
   ClientError(hInst, hMain, IDS_TIMEOUT);
}
/****************************************************************************/
/*
 * GetMessageBoxParent:  Determine appropriate parent window for error message.
 */
HWND GetMessageBoxParent(void)
{
  return hMain;
}
/****************************************************************************/
/*
 * LoginErrorMessage:  Bring up an error message box with given login error message.
 */
void LoginErrorMessage(char *message, BYTE action)
{
   HWND hParent = GetMessageBoxParent();
   config.quickstart = FALSE;
   ClientMessageBox(hParent, message, szAppName, MB_APPLMODAL | MB_ICONEXCLAMATION);
   switch (action)
   {
   case LA_NOTHING:
      EnterGame();
      break;

   case LA_LOGOFF:
     Logoff();
     break;

   default:
     debug(("LoginErrorMessage got unknown action type %d\n", action));
     break;
   }
}
/****************************************************************************/
void LoginError(int err_string)
{
   HWND hParent = GetMessageBoxParent();
   ClientError(hInst, hParent, err_string);
   config.quickstart = FALSE;
   LoginReset();
}
/****************************************************************************/
/*
 * LoginReset:  We've gotten an error from the server in LOGIN state;
 *   bring up appropriate dialog.
 */ 
void LoginReset(void)
{
   /* If login failed here, bring up dialog again */
   config.quickstart = FALSE;
   if (!logged_in)
   { 
      if (connection != CON_NONE)
	 if (GetLogin())
	    LoginSendInfo();
	 else Logoff();
      return;
   }

   EnterGame();
}
/****************************************************************************/
/*
 * EnterGame:  Ask to go into game.
 */
void EnterGame(void)
{
   int encodednum;

   // Go into admin mode if person is admin and mode selected
   if (user_type == USER_ADMIN && admin_mode)
     {
       RequestAdmin();
       return;
     }
  
   // Go into game
   DownloadCheckDirs(hMain);
   encodednum = (((MAJOR_REV * 100) + MINOR_REV) * P_CATCH) + P_CATCH;
   RequestGame(config.download_time,encodednum,inihost);
}
/****************************************************************************/



/****************************************************************************/
/*
 * GetLogin:  Bring up dialog asking user to fill in login information.
 *   Return True iff user presses OK button.
 */
Bool GetLogin(void)
{
   admin_mode = False;
   // Have guests log in automatically
   if (config.guest)
      return GuestGetLogin();
   else if (config.quickstart)
   {
      logged_in = False;
   }
   else
   {
      logged_in = False;
      if (DialogBox(hInst, MAKEINTRESOURCE(IDD_LOGIN), hMain, LoginDialogProc) == IDCANCEL)
	 return False;
      // Go into admin mode if control key is held down
      admin_mode = (GetKeyState(VK_CONTROL) < 0);

      // See if "guest" typed as account name
      if (!stricmp(config.username, GetString(hInst, IDS_GUEST)))
      {
	 config.guest = True;
	 return GuestGetLogin();
      }
   }
      
   return True;
}
/****************************************************************************/
/*
 * LoginDialogProc:  Get login information.
 */
BOOL CALLBACK LoginDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   static HWND hUser, hPasswd, hServer;        /* Edit boxes in dialog */
   static HWND hGroupBox, hTryPreviewButton;
   int value;
   BOOL success;

   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, GetParent(hDlg));
      hUser = GetDlgItem(hDlg, IDC_USERNAME);
      hPasswd = GetDlgItem(hDlg, IDC_PASSWORD);
      hServer = GetDlgItem(hDlg, IDC_SERVERNUM);
      hGroupBox = GetDlgItem(hDlg, IDC_NEW_TO_MERIDIAN_59);

      Edit_LimitText(hUser, MAXUSERNAME);
      Edit_LimitText(hPasswd, MAXPASSWORD);
      Edit_LimitText(hServer, MAXSERVERNUM);

      SetWindowFont(hUser, GetFont(FONT_INPUT), FALSE);
      SetWindowFont(hPasswd, GetFont(FONT_INPUT), FALSE);
      SetWindowFont(hServer, GetFont(FONT_INPUT), FALSE);

      // Set server number, if it's valid
      if (config.comm.server_num != -1)
      {
         SetDlgItemInt(hDlg, IDC_SERVERNUM, config.comm.server_num, FALSE);
	 
	 // If already logged in, can't change server number
	 if (connection != CON_NONE)
	    EnableWindow(hServer, FALSE);
      }

      /* If we have a default name, go to password edit box */
      Edit_SetText(hUser, config.username);
      Edit_SetSel(hUser, 0, -1);
      if (config.guest)
      {
	 RECT rc;
	 int bottom;
	 Edit_SetText(hUser, "GUEST");
	 Edit_SetText(hPasswd, "GUEST");
	 EnableWindow(hUser, FALSE);
	 EnableWindow(hPasswd, FALSE);
	 EnableWindow(hServer, FALSE);	 
	 EnableWindow(GetDlgItem(hDlg,IDC_OK), FALSE);
	 GetWindowRect(hGroupBox, &rc);
	 bottom = rc.bottom + 5;
	 GetWindowRect(hDlg, &rc);
	 MoveWindow(hDlg, rc.left, rc.top, rc.right - rc.left, bottom - rc.top, TRUE);
      }
      else if (strlen(config.username) > 0)
      {
	 Edit_SetText(hPasswd, config.password);
	 Edit_SetSel(hPasswd, 0, -1);
	 SetFocus(hPasswd);
	 return 0;  /* We have already set focus */
      }
      return 1;     /* Set focus to default window */
      
   case BK_DIALOGDONE:
      EndDialog(hDlg, IDOK);
      return TRUE;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDC_GUEST:
	 strcpy(config.username, "GUEST");
	 strcpy(config.password, "GUEST");
	 ConfigSetServerNameByNumber(config.server_guest);
	 ConfigSetSocketPortByNumber(config.server_guest);
	 config.comm.server_num = config.server_guest;
	 EndDialog(hDlg, IDOK);
	 return TRUE;

      case IDC_HOMEPAGE:
	 {
	    char url[256];
	    LoadString(hInst,IDS_HOMEPAGEURL,url,sizeof(url));
	    if (*url)
	    {
	       WebLaunchBrowser(url);
	       EndDialog(hDlg, IDCANCEL);
	       PostMessage(hMain,WM_SYSCOMMAND,SC_CLOSE,0);
	    }
	 }
	 return TRUE;

      case IDOK:
	 /* User has pressed return on one of the edit boxes */
	 if (GetFocus() == hUser)
	 {
	    SetFocus(hPasswd);
	    return True;
	 }
	 
	 if (GetFocus() == hPasswd)
	 {
	    // Go to server edit box if it's empty
	    value = GetDlgItemInt(hDlg, IDC_SERVERNUM, &success, FALSE);
	    
	    if (success)
	       PostMessage(hDlg, WM_COMMAND, IDC_OK, 0);
	    else SetFocus(hServer);
	    return True;	    
	 }

	 if (GetFocus() == hServer)
	    PostMessage(hDlg, WM_COMMAND, IDC_OK, 0);
	 return TRUE;

      case IDC_OK:
	 /* Get username & password */
	 Edit_GetText(hUser, config.username, MAXUSERNAME + 1);
	 Edit_GetText(hPasswd, config.password, MAXPASSWORD + 1);

	 value = GetDlgItemInt(hDlg, IDC_SERVERNUM, &success, FALSE);
	 if (!success)
	 {
	    // If logging in as "guest", no server number required
	    if (!stricmp(config.username, GetString(hInst, IDS_GUEST)))
	       value = config.server_guest;
	    else
	    {
	       ClientError(hInst, hDlg, IDS_NOSERVERNUM);
	       return TRUE;
	    }
	 }
	 
	 // If value changed, set server name and socketport
	 if (value != config.comm.server_num)
	 {
	    ConfigSetServerNameByNumber(value);
	    ConfigSetSocketPortByNumber(value);
	    config.comm.server_num = value;
	 }

	 /* Kill off dialog */
	 EndDialog(hDlg, IDOK);
	 return TRUE;

      case IDC_HANGUP:
      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
      break;
   }

   return FALSE;
}
