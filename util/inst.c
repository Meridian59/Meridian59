/*
 * inst:  Press some buttons on Freeman Installer
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
void PressDialogButton(HWND hDialog, int count)
{
   HWND hEdit, hList;
   int i, index;

   hList = GetDlgItem(hDialog, 0x65);
   if (hList == NULL)
   {
      printf("Unable to find list box\n");
      return;
   }

   hEdit = GetDlgItem(hDialog, 0x77);
   if (hEdit == NULL)
   {
      printf("Unable to find edit box\n");
      return;
   }

   for (i=0; i < count; i++)
   {
      SetFocus(hEdit);
      // Set edit contents
      SendMessage(hEdit, WM_KEYDOWN, 'A', (LPARAM) 0);
      SendMessage(hEdit, WM_KEYUP, 'A', (LPARAM) 0);
//      SendMessage(hEdit, EM_SETSEL, 0, -1);
//      SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM) "$x");
      SendMessage(hDialog, WM_COMMAND, MAKELONG(0x77, EN_UPDATE), (LPARAM) hEdit);
      SendMessage(hDialog, WM_COMMAND, MAKELONG(0x77, EN_CHANGE), (LPARAM) hEdit);
      
      wait(1);

      // Move down in list box
      SetFocus(hList);
      index = ListBox_GetCurSel(hList);

      ListBox_SetCurSel(hList, index + 1);
      SendNotifyMessage(hDialog, WM_COMMAND, MAKELONG(0x65, LBN_SELCHANGE), (LPARAM) hList);

      wait(1);
   }
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
   PressDialogButton(hwnd, 1000);

   wait(2);

   return TRUE;
}
/************************************************************************/
main(int argc, char **argv)
{
   HWND hMain;
   int i, count;

   if (argc < 3)
   {
      printf("Usage: inst <window handle in hex> <iterations>\n");
      exit(1);
   }

   sscanf(argv[1], "%x", &hMain);
   sscanf(argv[2], "%d", &count);

   printf("window handle = %d, count = %d\n", hMain, count);

   if (hMain == NULL)
   {
      printf("Couldn't find window with caption %s\n", main_window_caption);
      exit(1);
   }

   PressDialogButton(hMain, count);
   
   return 0;
}
