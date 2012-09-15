/*
 * remot:  Start VC++ 4.0 remote debug monitor remotely
 */

#include <windows.h>
#include <windowsx.h>

#include <time.h>
#include <stdio.h>

static char main_window_caption[] = "Visual C++ Debug Monitor";
static char settings_window_caption[] = "Win32 Network (TCP/IP) Settings";
static char dialog_class_name[] = "#32770";

static char remote_machine_name[] = "198.82.204.39";
static char remote_password[] = "blakston";

/********************************************************************/
void wait(long seconds)
{
   Sleep(seconds * 1000);
}                 
/************************************************************************/
/*
 * FindMainWindow:  Find and return first top-level window with given 
 *   class and caption, or NULL if none.
 */
HWND FindMainWindow(char *class, char *caption)
{
   return FindWindow(class, caption);
}
/************************************************************************/
/*
 * SendWindowEvents:  Hit buttons, fill in edit boxes, etc.
 *   Return TRUE on success.
 */
BOOL SendWindowEvents(HWND hwnd)
{
   HWND hList, hButton, hSettings, hEdit;

   // Set connection type to "TCP/IP"
   hList = GetDlgItem(hwnd, 0x03E8);
   ListBox_SetCurSel(hList, 1);  

   // Press "Settings" button
   hButton = GetDlgItem(hwnd, 0x03E9);
   SendNotifyMessage(hwnd, WM_COMMAND, MAKELONG(0x3E9, 0), (LPARAM) hButton);

   wait(2);

   // Find settings dialog
   hSettings = FindMainWindow(dialog_class_name, settings_window_caption);
   if (hSettings == NULL)
   {
      printf("Can't find settings dialog!\n");
      return FALSE;
   }

   wait(2);

   // Set remote machine name
   hEdit = GetDlgItem(hSettings, 0x065);
   SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM) remote_machine_name);

   // Set password
   hEdit = GetDlgItem(hSettings, 0x067);
   SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM) remote_password);

   // Press OK button
   hButton = GetDlgItem(hSettings, IDOK);
   SendNotifyMessage(hSettings, WM_COMMAND, MAKELONG(IDOK, 0), (LPARAM) hButton);
   
   wait(2);

   // Press main Connect button
   hButton = GetDlgItem(hwnd, 0x3EA);
   SendNotifyMessage(hwnd, WM_COMMAND, MAKELONG(0x3EA, 0), (LPARAM) hButton);

   wait(2);

   return TRUE;
}
/************************************************************************/
main(int argc, char **argv)
{
   HWND hMain;

   hMain = FindMainWindow(dialog_class_name, main_window_caption);

   if (hMain == NULL)
   {
      printf("Couldn't find window with caption %s\n", main_window_caption);
      exit(1);
   }

   if (!SendWindowEvents(hMain))
      return 1;

   return 0;
}
