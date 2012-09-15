// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * transfer.h:  Header for transfer.c
 */

#ifndef _TRANSFER_H
#define _TRANSFER_H

void TransferInit(void);
void TransferClose(void);

void __cdecl TransferStart(void *download_info);
void TransferContinue(void);
void TransferAbort(void);

#endif  // _TRANSFER_H
