// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * web.c:  Handle interface to Web browser.
 *
 */

#include "client.h"

/************************************************************************/
/*
 * WebLaunchBrowser:  Attempt to run browser on given URL.
 */
void WebLaunchBrowser(char *url)
{
  ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}
