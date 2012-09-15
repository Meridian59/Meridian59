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

/*
 * This callback function is called once per resource.  If it returns TRUE, the next resource
 * is loaded; if it returns FALSE, loading of the file immediately stops and RscFileLoad
 * returns FALSE.
 */
typedef BOOL (*RscCallbackProc)(char *filename, int resource_num, char *string);

/*
 * NOTE: REQUIRES that the Crusher functions cxInit and cxBuf2BufInit have been called.
 *       This function sets the Crusher password.
 */
BOOL RscFileLoad(char *fname, RscCallbackProc callback);


#endif /* #ifndef _RSCLOAD_H */
