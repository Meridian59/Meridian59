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

#ifdef __cplusplus
extern "C" {
#endif

void ModulesInit(void);
void ModulesClose(void);
void ModulesExitGame(void);

ModuleInfo * ModuleLoadByName(char *filename);
Bool ModuleLoadByRsc(ID name_rsc);
Bool ModuleExitByRsc(ID name_rsc);
Bool ModuleExitById(int module_id);
Bool ModuleUnloadById(int module_id);
void ModulesEnterGame(void);

/* event handler */
M59EXPORT Bool _cdecl ModuleEvent(int event, ...);

typedef void (WINAPI *ModuleProc)(ModuleInfo *module_info, ClientInfo *client_info);
typedef void (WINAPI *ModuleExitProc)();
  
#ifdef __cplusplus
};
#endif

#endif /* #ifndef _MODULES_H */
