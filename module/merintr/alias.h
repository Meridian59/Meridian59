// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * alias.h:  Header file for alias.c
 */

#ifndef _ALIAS_H
#define _ALIAS_H

void AliasInit(void);
void CmdAliasInit(void);
void AliasExit(void);
void AliasSave(void);
void AliasSetKey(KeyTable table, WORD key, WORD flags, WORD command, void *data);
BOOL AddVerbAlias(char* pVerb, char* pCommand);
void RemoveVerbAlias(char* pVerb);
void FreeVerbAliases();

BOOL ParseVerbAlias(char *pInput);

void CommandAlias(char *args);
void CommandVerbAlias(char *args);

#endif /* #ifndef _ALIAS_H */

