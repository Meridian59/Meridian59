// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// block.c : Implements the block list for BLAKSERV.
//
//////////
//

#include "blakserv.h"

//////////

static block_node* block_head = NULL;

//////////

block_node* FindBlock(struct in_addr* piaPeer)
{
	block_node* pBlock = block_head;

	while (pBlock)
	{
		if (0 == memcmp(&pBlock->iaPeer, piaPeer, sizeof(struct in_addr)))
			return pBlock;

		pBlock = pBlock->next;
	}

	return NULL;
}

void AddBlock(int iSeconds, struct in_addr* piaPeer)
{
	block_node* pBlock = FindBlock(piaPeer);
	int iExpires = (iSeconds < 0)? -1 : GetTime() + iSeconds;

    // A block set to expire at -1 will stay in effect until all blocks are deleted.
	// (Usually when server is shut down and later restarted.)

	if (pBlock)
	{
		if (pBlock->iExpires >= 0)
			pBlock->iExpires = iExpires;
		return;
	}

	pBlock = (block_node *) AllocateMemory(MALLOC_ID_BLOCK,sizeof(block_node));
	if (pBlock)
	{
		pBlock->iExpires = iExpires;
		pBlock->iaPeer = *piaPeer;
		pBlock->next = block_head;
		block_head = pBlock;
	}
}

void DeleteBlock(struct in_addr* piaPeer)
{
	block_node** pHook = &block_head;
	block_node* pBlock = block_head;

	while (pBlock)
	{
		if (0 == memcmp(&pBlock->iaPeer, piaPeer, sizeof(struct in_addr)))
		{
			*pHook = pBlock->next;
			FreeMemory(MALLOC_ID_BLOCK,pBlock,sizeof(block_node));
			return;
		}

		pHook = &pBlock->next;
		pBlock = pBlock->next;
	}
}

void DeleteAllBlocks()
{
	while (block_head)
	{
		block_node* pBlock = block_head;
		block_head = pBlock->next;

		FreeMemory(MALLOC_ID_BLOCK,pBlock,sizeof(block_node));
	}
}

bool CheckBlockList(struct in_addr* piaPeer)
{
	block_node* pBlock = FindBlock(piaPeer);

	// true means not blocked and can connect
	// false means block still in effect

	if (!pBlock)
		return true;

	if (pBlock->iExpires < 0)
		return false;

	if (pBlock->iExpires <= GetTime())
	{
		DeleteBlock(piaPeer);
		return true;
	}

	return false;
}

/*
 * BuildBannedIPBlocks - Ban IPs from meridian
 *
 * Input : asciz string of filename of banned ips
 * Output :
 *
 * Author : Charlie
 *
 */

void BuildBannedIPBlocks( const char *filename )
{
  
  FILE*fp;
  char buffer[1024];
  struct in_addr blocktoAdd ;
  
  fp = fopen(filename,"rt");
  if( fp == NULL ) {
    eprintf("Cannot open banned log file %s\n",filename );
    return ;
  }
  dprintf("loading banned IP addresses\n");
  
  do {
    if(fgets(buffer,1023,fp) != NULL ) {
      /* lets be cautious */
      if(strlen(buffer)>0) {
	if( ( blocktoAdd.s_addr = inet_addr( buffer ) ) != -1 ) {
	  AddBlock( -1, &blocktoAdd );
	  dprintf("Banned IP address %s\n", inet_ntoa( blocktoAdd ) );
	} else {
	  eprintf("Warning invalid entry in %s is [%s]\n",filename,buffer);
	}
      }
    }
  } while( !feof( fp ) );
  
  fclose( fp );
}
