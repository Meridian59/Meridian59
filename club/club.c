// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * club.c
 *

 Client update program, using FTP!!!! Yeah!!!!

 */

#include "club.h"

/* time to wait at program start */
#define INIT_TIME 3000

HINSTANCE hInst;
HWND hwndMain;

Bool success;

static Bool retry = True;   // True when an error occurs and user asks to retry

char *restart_filename;

int transfer_progress = 0;
int transfer_file_size = 0;
char string[256];
char format[256];

char *transfer_machine;
char *transfer_filename;
char *transfer_local_filename;

char *dest_path;

/* local function prototypes */
Bool ParseCommandLine();
void RestartFilename();
void StartupError();
void RestartClient();
void Interface(int how_show);
long WINAPI InterfaceWindowProc(HWND hwnd,UINT message,UINT wParam,LONG lParam);
void OnTimer(HWND hwnd,int id);
BOOL CALLBACK ErrorDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

/************************************************************************/
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrev_instance,char *command_line,int how_show)
{
   hInst = hInstance;

   InitCommonControls();

   success = False; /* whether the copy compeletely succeeded */

   if (ParseCommandLine())
   {
      Interface(how_show);
   }

   if (success)
      RestartClient();

   if (success == False)
      return 1;
   
   return 0;
}   
/************************************************************************/
Bool ParseCommandLine()
{
   if (__argc != 7) /* six command line parameters, actually. */
   {
      StartupError();
      return False;
   }

   restart_filename = __argv[1];

   if (strncmp(__argv[2],"UPDATE",6) != 0)
   {
      StartupError();
      return False;
   }

   transfer_machine = __argv[3];
   transfer_filename = __argv[4];

   transfer_local_filename = __argv[5];

   dest_path = __argv[6];

   return True;
}
/************************************************************************/
void StartupError()
{
   MessageBox(NULL,GetString(hInst, IDS_BADSTARTUP),GetString(hInst, IDS_APPNAME),
	      MB_ICONSTOP | MB_OK);
}
/************************************************************************/
void RestartClient()
{
   char s[100+MAX_PATH+FILENAME_MAX];
   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   memset(&si,sizeof(si),0);
   si.cb = sizeof(si);
   GetStartupInfo(&si); /* shouldn't need to do this.  very weird */

   if (!CreateProcess(restart_filename,"",NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
   {
      sprintf(s,GetString(hInst, IDS_CANTRESTART),GetLastError(),restart_filename);
      MessageBox(NULL,s,GetString(hInst, IDS_APPNAME),MB_ICONSTOP);
   }
}
/************************************************************************/
void Interface(int how_show)
{
   WNDCLASS main_class;
   MSG msg;

   main_class.style = CS_HREDRAW | CS_VREDRAW;
   main_class.lpfnWndProc = InterfaceWindowProc;
   main_class.cbClsExtra = 0;
   main_class.cbWndExtra = DLGWINDOWEXTRA;
   main_class.hInstance = hInst;
   main_class.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON));
   main_class.hCursor = NULL;
   main_class.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
   main_class.lpszMenuName = NULL;
   main_class.lpszClassName = "club (Andrew & Christopher Kirmse)";

   RegisterClass(&main_class);

   hwndMain = CreateDialog(hInst,MAKEINTRESOURCE(IDD_MAIN_DLG),0,
                           (DLGPROC) InterfaceWindowProc);

   ShowWindow(hwndMain,how_show);

   Status(GetString(hInst, IDS_STARTING));
   
   while (GetMessage(&msg,NULL,0,0))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
}
/************************************************************************/
long WINAPI InterfaceWindowProc(HWND hwnd,UINT message,UINT wParam,LONG lParam)
{
   HWND hCtrl;

   switch (message)
   {
   case WM_INITDIALOG :
      SetTimer(hwnd,TIMER_START_TRANSFER,INIT_TIME,NULL);
      CenterWindow(hwnd,NULL);
      SetFocus(GetDlgItem(hwnd, IDCANCEL));

      GetDlgItemText(hwnd, IDC_BYTES1, format, sizeof(format));
      sprintf(string, format, transfer_progress, transfer_file_size);
      SetDlgItemText(hwnd, IDC_BYTES1, string);

      hCtrl = GetDlgItem(hwnd, IDC_ANIMATE1);
      Animate_Open(hCtrl, MAKEINTRESOURCE(IDA_DOWNLOAD));
      break;

   case WM_NCDESTROY:
      PostQuitMessage(0);
      return 0;

   HANDLE_MSG(hwnd, WM_TIMER, OnTimer);

   case CM_RETRYABORT:
      if (retry)
      {
        Status(GetString(hInst, IDS_CONNECTING), transfer_machine);
        _beginthread((void(*)(void*))TransferStart, 0, NULL);
      }
      else
      {
        PostMessage(hwndMain, WM_CLOSE, 0, 0);
      }
      break;
      
   case CM_DEARCHIVE :
      Dearchive(dest_path, transfer_local_filename);
      break;

   case CM_FILESIZE:
      // Set max value
     transfer_file_size = lParam;
      sprintf(string, format, transfer_progress, transfer_file_size);
      SetDlgItemText(hwndMain, IDC_BYTES1, string);
      SendDlgItemMessage(hwnd, IDC_PROGRESS, PBM_SETRANGE, 0,
                         MAKELPARAM(0, transfer_file_size / 100));
      break;

   case CM_PROGRESS:
      // Set current value
      transfer_progress = lParam;
      sprintf(string, format, transfer_progress, transfer_file_size);
      SetDlgItemText(hwndMain, IDC_BYTES1, string);
      SendDlgItemMessage(hwnd, IDC_PROGRESS, PBM_SETPOS, transfer_progress / 100, 0);
      break;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDCANCEL:
	 PostMessage(hwnd, WM_CLOSE, 0, 0);
	 break;
      }
      break;

   }
   return DefWindowProc(hwnd,message,wParam,lParam);    
}
/************************************************************************/
void OnTimer(HWND hwnd,int id)
{
   switch (id)
   {
   case TIMER_START_TRANSFER:
      KillTimer(hwndMain, TIMER_START_TRANSFER);
      PostMessage(hwndMain, CM_RETRYABORT, 0, 0);
      break;
   }
}
/************************************************************************/
/*
 * Status:  Display given status message.
 */
void Status(char *fmt, ...)
{
   char s[200];
   va_list marker;
    
   va_start(marker,fmt);
   vsprintf(s,fmt,marker);
   va_end(marker);

   SetDlgItemText(hwndMain, IDC_STATUS, s);
}
/************************************************************************/
/*
 * Error:  Display given error message, and ask for retry.
 */
void Error(char *fmt, ...)
{
   int retval;
   char s[500];
   va_list marker;
    
   va_start(marker,fmt);
   vsprintf(s,fmt,marker);
   va_end(marker);

   retval = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ERROR), hwndMain, ErrorDialogProc, 
			   (LPARAM) s);

   if (retval == IDOK)
      retry = True;
   else retry = False;
}
/*****************************************************************************/
/*
 * ErrorDialogProc:  Dialog procedure for displaying error and asking for retry.
 */
BOOL CALLBACK ErrorDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
   case WM_INITDIALOG:
      SetDlgItemText(hDlg, IDC_MESSAGE, (char *) lParam);
      CenterWindow(hDlg, GetParent(hDlg));
      return FALSE;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
	 EndDialog(hDlg, IDOK);
	 return TRUE;
      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
   }
   return FALSE;
}
/*****************************************************************************/
char *GetLastErrorStr()
{ 
   char *error_str;
   
   error_str = "No error string"; /* in case the call  fails */

   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		 NULL,GetLastError(),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
		 (LPTSTR) &error_str,0,NULL);
   return error_str;
}
