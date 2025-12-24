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
bool SendServer(char *buf, unsigned int numbytes);
bool WriteServer(char *buf,unsigned int numbytes);
void CloseSocketConnection(void);
bool OpenSocketConnection(char *hostname, int sock_port);
void CloseConnection(void);

void CommGetSettings(void);
void CommLoadSettings(void);
void CommSaveSettings(void);

void ProcessMsgBuffer(void);

bool StartReadNotification(void);
void EndReadNotification(void);

#define NUM_STREAMS 5
void RandomStreamsInit(unsigned int *init);

#endif /* #ifndef _COM_H */
