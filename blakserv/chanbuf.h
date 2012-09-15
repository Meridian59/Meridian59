// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * chanbuf.h
 *
 */

#ifndef _CHANBUF_H
#define _CHANBUF_H

enum
{
   CHANBUF_SIZE = 1000
};

typedef struct
{
   int channel_id;
   char buf[CHANBUF_SIZE];
} channel_buffer_node;

void InitChannelBuffer(void);
void WriteChannelBuffer(int channel_id,char *s);

Bool IsNewChannelText(void);
channel_buffer_node * GetChannelBuffer(void);
void DoneChannelBuffer(void);


#endif
