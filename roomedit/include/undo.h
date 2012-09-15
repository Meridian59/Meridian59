/*----------------------------------------------------------------------------*
 | This file is part of WinDEU, the port of DEU to Windows.                   |
 | WinDEU was created by the DEU team:                                        |
 |  Renaud Paquay, Raphael Quinet, Brendon Wyber and others...                |
 |                                                                            |
 | DEU is an open project: if you think that you can contribute, please join  |
 | the DEU team.  You will be credited for any code (or ideas) included in    |
 | the next version of the program.                                           |
 |                                                                            |
 | If you want to make any modifications and re-distribute them on your own,  |
 | you must follow the conditions of the WinDEU license. Read the file        |
 | LICENSE or README.TXT in the top directory.  If do not  have a copy of     |
 | these files, you can request them from any member of the DEU team, or by   |
 | mail: Raphael Quinet, Rue des Martyrs 9, B-4550 Nandrin (Belgium).         |
 |                                                                            |
 | This program comes with absolutely no warranty.  Use it at your own risks! |
 *----------------------------------------------------------------------------*

	Project WinDEU
	DEU team
	Jul-Dec 1994, Jan-Mar 1995

	FILE:         undo.h

	OVERVIEW
	========
	Class definition for undo.
*/
#if !defined(__undo_h)              // Sentry, use file only if it's not already included.
#define __undo_h

#ifndef __common_h
	#include "common.h"
#endif

extern int MaxUndo;

void InitUndo();
void CleanupUndo();
void StartUndoRecording(const char *name);
void StopUndoRecording();
void Undo();
void Redo();
char *GetUndoName();	// NULL if no UNDO available
char *GetRedoName();	// NULL if no REDO available

#endif                                      // __undo_h sentry.

