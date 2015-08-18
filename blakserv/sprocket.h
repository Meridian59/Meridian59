// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * sprocket.h
 *
 * Tables describing client -> server protocol.
 *
 * This used to be reloadable as a DLL, hence the odd layout and naming.
 */

#ifndef _SPROCKET_H
#define _SPROCKET_H

void InitSprocket(void);
client_table_node *GetParseClientTable(int table_type);

#endif
