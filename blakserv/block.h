// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// block.h : Block list for BLAKSERV.
//
//////////
//

#ifndef BLOCK_H
#define BLOCK_H

typedef struct block_node_struct
{
   int iExpires;
   struct in_addr iaPeer;
   struct block_node_struct *next;
} block_node;

void AddBlock(int iSeconds, struct in_addr* piaPeer);
void DeleteBlock(struct in_addr* piaPeer);
void DeleteAllBlocks(void);

bool CheckBlockList(struct in_addr* piaPeer);

void BuildBannedIPBlocks( const char *filename );


#endif // BLOCK_H
