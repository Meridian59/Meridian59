// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * com.h
 */
 
#ifndef _COM_H
#define _COM_H

enum { CON_NONE, CON_SOCKET };  /* Connection types */

#define COMBUFSIZE 40000  /* Size of send and receive buffers */

int ReadServer(void);
Bool SendServer(char *buf, unsigned int numbytes);
Bool WriteServer(char *buf,unsigned int numbytes);
void CloseSocketConnection(void);
Bool OpenSocketConnection(char *hostname, int sock_port);
void CloseConnection(void);

void CommGetSettings(void);
void CommLoadSettings(void);
void CommSaveSettings(void);

void ProcessMsgBuffer(void);

Bool StartReadNotification(void);
void EndReadNotification(void);

#define NUM_STREAMS 5
void RandomStreamsInit(unsigned int *init);

#endif /* #ifndef _COM_H */
