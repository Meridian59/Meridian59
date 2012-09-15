// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * profane.h:  Header for profane.c
 */

#ifndef _PROFANE_H
#define _PROFANE_H

/***************************************************************************/

#define MAXPROFANETERM 20 /* letters */

void LoadProfaneTerms();
BOOL AddProfaneTerm(char* pszTerm);
BOOL RemoveProfaneTerm(char* pszTerm);
void SaveProfaneTerms();
void FreeProfaneTerms();
void RecompileAllProfaneExpressions();
BOOL ContainsProfaneTerms(char* pszText);
char* CleanseProfaneString(char* pszText);

/***************************************************************************/

#endif /* #ifndef _PROFANE_H */
