// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * loadrsc.h
 *
 */

#ifndef _LOADRSC_H
#define _LOADRSC_H

#define MAXNAME      256    /* Maximum name resource length */
#define MAXRSCSTRING 8192   /* Maximum string length in resources */


typedef struct {
   ID    idnum;
   char *resource[MAX_LANGUAGE_ID];
} *resource_type, resource_struct;

#ifdef __cplusplus
extern "C" {
#endif

Bool LoadResources(void);
void FreeResources(void);
void ChangeResource(ID res, char *value);
Bool LoadRoomFile(char *fname, room_type *r);
M59EXPORT char *GetString(HINSTANCE hModule, ID idnum);
void MissingResource(void);
void DeleteRscFiles(list_type files);
void DeleteAllRscFiles(void);

M59EXPORT Bool *GetAvailableLanguages(void);
M59EXPORT char *LookupRsc(ID idnum);
M59EXPORT char *LookupNameRsc(ID idnum);
M59EXPORT char *LookupRscNoError(ID idnum);
M59EXPORT char *LookupRscRedbook(ID idnum);

#ifdef __cplusplus
};
#endif

#endif /* #ifndef _LOADRSC_H */

