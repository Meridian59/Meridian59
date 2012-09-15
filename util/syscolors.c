/*
 */

#include <windows.h>

#define NUM_SYS_COLORS 25

main()
{
  int i;

  for (i=0; i < NUM_SYS_COLORS; i++)
    {
      COLORREF c = GetSysColor(i);
      
      printf("color = %3d, red = %3d, green = %3d, blue = %3d\n", i, GetRValue(c), GetGValue(c), GetBValue(c)); 
    }
}
