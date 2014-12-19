// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * version.c
 *

 This module is compiled every time Blakserv is linked, and has a
 string with the title and version of the program.

 */

#include "blakserv.h"

const char * BlakServLongVersionString(void)
{
   return "BlakSton Server v" BLAKSERV_VERSION " (" __DATE__ " " __TIME__ ")";
}

const char * BlakServNameString(void)
{
   return "BlakSton Server";
}
