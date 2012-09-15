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
 * We store the location of the Web browser in the "config" variable.  This location is
 * updated each time the client is run by finding the default browser in the registry.
 */

#include "client.h"

/************************************************************************/
/*
 * WebFindDefaultBrowser:  Look up the default browser in the registry, and
 *   save it in "config" if it's found.
 *   Return True on success, False otherwise.
 */
Bool WebFindDefaultBrowser(void)
{
  char *key_name;
  DWORD size = MAX_PATH;
  int retval;
  HKEY hKey;
  char *ptr1, *ptr2;
  
  key_name = GetString(hInst, IDS_HTTPKEY);

  retval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key_name, 0, KEY_READ, &hKey);
  if (retval != ERROR_SUCCESS)
    {
      debug(("error opening registry key for default browser\n"));
      return False;
    }

  retval = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE) config.browser, &size);
  if (retval != ERROR_SUCCESS)
    {
      debug(("Default Web browser key lookup failed\n"));
      RegCloseKey(hKey);
      return False;
    }

  // Remove junky command line arguments from command line; look for \ after - (- in pathname)
  ptr1 = strrchr(config.browser, '-');
  if (ptr1 != NULL)
  {
    ptr2 = strchr(ptr1, '\\');

    if (ptr2 == NULL)
      *ptr1 = 0;
  }

  debug(("Found browser %s\n", config.browser));
  RegCloseKey(hKey);
  return True;
}

/************************************************************************/
/*
 * WebLaunchBrowser:  Attempt to run browser on given URL.
 */
void WebLaunchBrowser(char *url)
{
   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   char command_line[MAX_CMDLINE];

   sprintf(command_line, "%s %s", config.browser, url);

   memset(&si, sizeof(si), 0);
   si.cb = sizeof(si);
   GetStartupInfo(&si); /* shouldn't need to do this.  very weird */

   if (!CreateProcess(NULL, command_line,
		      NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
     {
       debug(("Failed running browser, command line = %s\n", command_line));

       ClientError(hInst, hMain, IDS_NOBROWSER, config.browser);
     }
}
