// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * stats.h:  Main header file for character stat adjustment DLL
 */

#ifndef _STATS_H
#define _STATS_H

#include "statsrc.h"

#define MODULE_ID 103  // Unique module id #

#define BK_TABCHANGED     (WM_USER + 101)

/* New game state */
enum {GAME_PICKCHAR = 100, };

/* Structures from server giving character information */
typedef struct {
   ID    id;           // Object id of character
   char *name;         // String name
   BYTE  flags;        // Extra info about character
} Character;

void GoToGame(ID character);
void AbortCharDialogs(void);

#define NUM_CHAR_STATS 6
#define NUM_CHAR_SCHOOLS 7

#define STAT_POINTS_INITIAL    50   // Initial # of available stat points

void MakeChar(int *stats_in, int *levels_in);
void initStatsFromServer(int *stats_in, int *levels_in);

void CharTabPageCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
Bool VerifySettings(void);
void CharInfoValid(void);
void CharInfoInvalid(void);

BOOL CALLBACK CharStatsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void CharStatsGetChoices(int *buf);
void CharSchoolsGetChoices(int *buf);
int  CharStatsGetPoints(void);

// Sending messages to server
extern client_message msg_table[];

extern ClientInfo *cinfo;         // Holds data passed from main client
extern Bool        exiting;       // True when module is exiting and should be unloaded

extern HINSTANCE hInst;  // module handle

/* messages to server */
#define SendNewCharInfo(stat1, stat2, stat3, stat4, stat5, stat6, school1,  school2,  school3,  school4,  school5,  school6,  school7) \
ToServer(BP_CHANGED_STATS, msg_table, stat1, stat2, stat3, stat4, stat5, stat6, school1,  school2,  school3,  school4,  school5,  school6,  school7)

#endif /* #ifndef _STATS_H */
