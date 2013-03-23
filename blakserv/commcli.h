// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * commcli.h
 *
 */

#ifndef _COMMCLI_H
#define _COMMCLI_H

enum
{
   SYSEVENT_GARBAGE = 1,
   SYSEVENT_SAVE = 2,
   SYSEVENT_RELOAD_SYSTEM = 3
};

void InitCommCli(void);

void AddBlakodToPacket(val_type obj_size,val_type obj_data);
void AddByteToPacket(unsigned char byte1);
void AddShortToPacket(short byte2);
void AddIntToPacket(int byte4);
void AddStringToPacket(int int_len,const char *ptr);
void SecurePacketBufferList(int session_id,buffer_node *blist);
void SendPacket(int session_id);
void SendCopyPacket(int session_id);
void ClearPacket(void);

void ClientHangupToBlakod(session_node *session);


void SendBlakodBeginSystemEvent(int type);
void SendBlakodEndSystemEvent(int type);

#endif
