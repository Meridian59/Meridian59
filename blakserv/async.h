// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * async.h
 *
 */

#ifndef _ASYNC_H
#define _ASYNC_H

void InitAsyncConnections(void);
void ExitAsyncConnections(void);
void AsyncSocketStart(void);
void AsyncSocketAccept(SOCKET sock,int event,int error,int connection_type);
void AsyncNameLookup(HANDLE hLookup,int error);
void AsyncSocketSelect(SOCKET sock,int event,int error);

#endif
