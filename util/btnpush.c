/*
 * btnpush:  Push button on window specified from command line.
 */

#include <windows.h>
#include <windowsx.h>

#include <time.h>
#include <stdio.h>

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

//   hButton = GetDlgItem(hDialog, id);
//   if (hButton == NULL)
//   {
//      printf("Unable to find dialog item %d\n", id);
//      return;
//   }
   SendNotifyMessage(hDialog, WM_COMMAND, MAKELONG(id, 0), (LPARAM) hButton);
}
/************************************************************************/
main(int argc, char **argv)
{
   char caption[500], class[500];
   HWND hMain;
   int id;

#if 0
   HWINSTA hwinsta;

   hwinsta = OpenWindowStation("WinSta0", FALSE, WINSTA_READSCREEN);
   if (hwinsta == NULL || SetProcessWindowStation(hwinsta) == FALSE)
      printf("Failed to set window station!\n");
#endif


   if (argc < 4)
   {
      printf("Usage: btnpush <window class> <window caption> <button id>\n");
      exit(1);
   }

   strcpy(class, argv[1]);
   strcpy(caption, argv[2]);
   sscanf(argv[3], "%d", &id);

   printf("window class = %s, caption = %s, button id = %d\n", class, caption, id);

   hMain = FindMainWindow(class, caption);

   if (hMain == NULL)
   {
      printf("Couldn't find window with class %s, caption %s\n", class, caption);
      exit(1);
   }

   PressDialogButton(hMain, id);
   
   return 0;
}
