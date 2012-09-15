// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * dm.h:  Header file for dm.c
 */

#ifndef _DM_H
#define _DM_H

#include "command.h"
#include "gchannel.h"
#include "qeditor.h"
#include "resource.h"

/***************************************************************************/

#define MODULE_ID 104    // Unique module id #

extern ClientInfo *cinfo;         // Holds data passed from main client
extern Bool exiting;              // True when module is exiting and should be unloaded

extern HINSTANCE hInst;  // module handle

// DM command types
#define DM_GO_ROOM    1
#define DM_GO_PLAYER  2
#define DM_GET_PLAYER 3

#define RequestDMCommand(type, str)  ToServer(BP_REQ_DM, msg_table, type, str)

extern client_message msg_table[];

#define DMDLGM_CLICKEDUSER (WM_APP+0x10)

/***************************************************************************/

#endif /* #ifndef _DM_H */
