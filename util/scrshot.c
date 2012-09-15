/*
 * scrshot:  Save screen shot to a file
 */

#include <windows.h>
#include <stdio.h>

#include "DIBUTIL.H"
#include "DIBAPI.H"

main(int argc, char **argv)
{
   RECT r;
   HDIB hdib;

   if (argc != 2)
   {
      printf("Usage: scrshot <output filename>\n");
      exit(1);
   }

   r.left = r.top = 0;
   r.right = GetSystemMetrics(SM_CXSCREEN);
   r.bottom = GetSystemMetrics(SM_CYSCREEN);

   hdib = CopyScreenToDIB(&r);
   if (SaveDIB(hdib, argv[1]))
   {
      printf("scrshot: Failed to save to file %s\n", argv[1]);
      exit(1);
   }

   return 0;
}
