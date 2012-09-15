// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * stringinthash.c
 *

  This module implements a hash from strings to integers. It makes
  a copy of the strings passed in.
*/

#include "blakserv.h"

sihash_type CreateSIHash(int size)
{
	int i;

	sihash_type sihash = (sihash_type)AllocateMemory(MALLOC_ID_KODBASE,sizeof(sihash_table));
	sihash->size = size;
	sihash->nodes = (sihash_node **)AllocateMemory(MALLOC_ID_KODBASE,size*sizeof(sihash_node *));
	for (i=0;i<size;i++)
		sihash->nodes[i] = NULL;

	return sihash;
}

void FreeSIHash(sihash_type sihash)
{
	int i;

	// free each node
	for (i=0;i<sihash->size;i++)
	{
		sihash_node *node = sihash->nodes[i];
		while (node != NULL)
		{
			sihash_node *next = node->next;
			FreeMemory(MALLOC_ID_KODBASE,node->key,strlen(node->key)+1);
			FreeMemory(MALLOC_ID_KODBASE,node,sizeof(sihash_node));
			node = next;
		}		
	}

	// free node table
	FreeMemory(MALLOC_ID_KODBASE,sihash->nodes,sihash->size*(sizeof(sihash_node *)));
	// free control structure

	FreeMemory(MALLOC_ID_KODBASE,sihash,sizeof(sihash_table));
}

void SIHashInsert(sihash_type sihash,const char *key,int value)
{
	unsigned int hash_value = GetBufferHash(key,strlen(key)) % sihash->size;
	sihash_node *node = sihash->nodes[hash_value];

	sihash_node *new_node = (sihash_node *)AllocateMemory(MALLOC_ID_KODBASE,sizeof(sihash_node));
	new_node->next = NULL;
	new_node->key = (char *)AllocateMemory(MALLOC_ID_KODBASE,strlen(key)+1);
	strcpy(new_node->key,key);
	new_node->value = value;
	if (node == NULL)
	{
		sihash->nodes[hash_value] = new_node;
	}
	else
	{
		sihash_node *next = node->next;
		while (next != NULL)
		{
			node = next;
			next = node->next;
		}
		node->next = new_node;
	}
}

Bool SIHashFind(sihash_type sihash,const char *key,int *value)
{
	unsigned int hash_value = GetBufferHash(key,strlen(key)) % sihash->size;
	sihash_node *node = sihash->nodes[hash_value];
	while (node != NULL)
	{
		if (stricmp(node->key,key) == 0)
		{
			*value = node->value;
			return True;
		}
		node = node->next;
	}
	return False;
}

const char * SIHashFindByValue(sihash_type sihash,int value)
{
	int i;

	for (i=0;i<sihash->size;i++)
	{
		sihash_node *node = sihash->nodes[i];
		while (node != NULL)
		{
			if (node->value == value)
				return node->key;
			node = node->next;
		}
	}
	return NULL;
}
