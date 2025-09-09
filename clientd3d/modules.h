// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * modules.h:  Header file for modules.c
 */

#ifndef _MODULES_H
#define _MODULES_H

void ModulesInit(void);
void ModulesClose(void);
void ModulesExitGame(void);

ModuleInfo * ModuleLoadByName(char *filename);
bool ModuleLoadByRsc(ID name_rsc);
bool ModuleExitByRsc(ID name_rsc);
bool ModuleExitById(int module_id);
bool ModuleUnloadById(int module_id);
void ModulesEnterGame(void);

/* event handler */
M59EXPORT bool _cdecl ModuleEvent(int event, ...);

typedef void (WINAPI *ModuleProc)(ModuleInfo *module_info, ClientInfo *client_info);
typedef void (WINAPI *ModuleExitProc)();
  
#endif /* #ifndef _MODULES_H */
