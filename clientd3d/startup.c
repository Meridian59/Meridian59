// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * startup.c
 *
 * Check user's hardware and software configurations at startup.
 */

#include "client.h"

/****************************************************************************/
Bool StartupCheck(void)
{
   Bool retval = True;
   HDC hdc = GetDC(NULL);
   long num_colors;
   int num_planes, num_bpp;

   // Make sure that user has at least 256 colors
   num_planes = GetDeviceCaps(hdc, PLANES);
   num_bpp    = GetDeviceCaps(hdc, BITSPIXEL);
      
   if (num_planes * num_bpp < 8)
   {
      num_colors = 1L << (num_bpp * num_planes);

      ClientError(hInst, NULL, IDS_TOOFEWCOLORS, NUM_COLORS, num_colors);
      retval = False;
   }
   
   ReleaseDC(NULL, hdc);

   return retval;
}
