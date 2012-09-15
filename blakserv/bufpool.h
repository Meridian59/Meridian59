// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * bufpool.h
 *
 */

#ifndef _BUFPOOL_H
#define _BUFPOOL_H

typedef struct buffer_struct
{
   int len_buf;  /* current amount of valid data in buf */
   char *buf;    /* points to where the data starts in prebuf; normally prebuf + HEADERBYTES */
                 /* but when we shove in the header bytes, it actually = prebuf */

   int size_buf;  /* useful for buffers used in reading, which don't use prebuf */

   char *prebuf;    /* this points to the real allocated memory.  */
   int size_prebuf; /* size of actually allocated memory */

   int buffer_id;
   
   struct buffer_struct *next;
} buffer_node;

void InitBufferPool(void);
void ResetBufferPool(void);
buffer_node * GetBuffer(void);
void DeleteBuffer(buffer_node *bn);
buffer_node * AddToBufferList(buffer_node *blist,void *buf,int len_buf);
buffer_node * AddByteToBufferList(buffer_node *blist,char ch);
buffer_node * CopyBufferList(buffer_node *blist);
void DeleteBufferList(buffer_node *blist);

#endif
