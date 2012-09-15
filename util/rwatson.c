/*
 * remot:  Hit button on Dr. Watson error popup
 */

#include <windows.h>
#include <windowsx.h>

#include <time.h>
#include <stdio.h>

static char main_window_caption[] = "Dr. Watson for Windows NT";
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
 * PressDialogButton:  Simulate pressing button with given id on given dialog.
 */
void PressDialogButton(HWND hDialog, int id)
{
   HWND hButton;

   hButton = GetDlgItem(hDialog, id);
   SendNotifyMessage(hDialog, WM_COMMAND, MAKELONG(id, 0), (LPARAM) hButton);
}
/************************************************************************/
/*
 * SendWindowEvents:  Hit buttons, fill in edit boxes, etc.
 *   Return TRUE on success.
 */
BOOL SendWindowEvents(HWND hwnd)
{
   HWND hButton;

   // Press Cancel button
   PressDialogButton(hwnd, 3);

   wait(2);

   return TRUE;
}
/************************************************************************/
main(int argc, char **argv)
{
   HWND hMain;
   HWINSTA hwinsta;

   hwinsta = OpenWindowStation("WinSta0", FALSE, WINSTA_READSCREEN);
   if (hwinsta == NULL || SetProcessWindowStation(hwinsta) == FALSE)
      printf("Failed to set window station!\n");

   if (argc > 1)
      sscanf(argv[1], "%d", &hMain);
   else hMain = FindMainWindow(dialog_class_name, main_window_caption);

   printf("%d\n", hMain);

   if (hMain == NULL)
   {
      printf("Couldn't find window with caption %s\n", main_window_caption);
      exit(1);
   }

   if (!SendWindowEvents(hMain))
      return 1;

   return 0;
}
