// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * rscload.h:  Header file for rscload.c, which loads rsc files.
 */

#ifndef _RSCLOAD_H
#define _RSCLOAD_H

#define MAX_LANGUAGE_ID 184 /* Max number of languages */
/*
 * This callback function is called once per resource.  If it returns true, the next resource
 * is loaded; if it returns false, loading of the file immediately stops and RscFileLoad
 * returns false.
 */
typedef bool (*RscCallbackProc)(char *filename, int resource_num, int lang_id, char *string);

bool RscFileLoad(char *fname, RscCallbackProc callback);


#endif /* #ifndef _RSCLOAD_H */
