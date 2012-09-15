/*
 * enumwin:  Enumerate titles of all top level windows
 */

#include <windows.h>

#include <stdio.h>

/************************************************************************/
BOOL CALLBACK PrintWindowStationProc(char *name, LPARAM lParam)
{
   printf("Found window station %s\n", name);

   if (lParam > 100)
      return FALSE;
   return TRUE;
}
/************************************************************************/
BOOL CALLBACK PrintWindowProc(HWND hwnd, LPARAM lParam)
{
   char buf[500];

   GetWindowText(hwnd, buf, 499);
   printf("name = %s handle = %d\n", buf, (int) hwnd);

   // Stop after 200--avoid loop
   if (lParam > 100)
      return FALSE;
   return TRUE;
}
/************************************************************************/
main(int argc, char **argv)
{
   int num;
   HWINSTA hwinsta;

   hwinsta = OpenWindowStation("WinSta0", FALSE, WINSTA_READSCREEN);
   if (hwinsta == NULL || SetProcessWindowStation(hwinsta) == FALSE)
      printf("Failed to set window station!\n");


#if 0
   num = 0;
   while (EnumWindowStations(PrintWindowStationProc, num))
      num++;
#endif

   num = 0;
   while (EnumWindows(PrintWindowProc, num))
      num++;
}
