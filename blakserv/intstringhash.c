// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * intstringhash.c
 *

  This module implements a hash from integers to strings. It makes
  a copy of the strings passed in.
*/

#include "blakserv.h"

ishash_type CreateISHash(int size)
{
	int i;

	ishash_type ishash = (ishash_type)AllocateMemory(MALLOC_ID_LOAD_GAME,sizeof(ishash_table));
	ishash->size = size;
	ishash->nodes = (ishash_node **)AllocateMemory(MALLOC_ID_LOAD_GAME,size*sizeof(ishash_node *));
	for (i=0;i<size;i++)
		ishash->nodes[i] = NULL;

	return ishash;
}

void FreeISHash(ishash_type ishash)
{
	int i;

	// free each node
	for (i=0;i<ishash->size;i++)
	{
		ishash_node *node = ishash->nodes[i];
		while (node != NULL)
		{
			ishash_node *next = node->next;
			FreeMemory(MALLOC_ID_LOAD_GAME,node->value,strlen(node->value)+1);
			FreeMemory(MALLOC_ID_LOAD_GAME,node,sizeof(ishash_node));
			node = next;
		}		
	}

	// free node table
	FreeMemory(MALLOC_ID_LOAD_GAME,ishash->nodes,ishash->size*(sizeof(ishash_node *)));
	// free control structure

	FreeMemory(MALLOC_ID_LOAD_GAME,ishash,sizeof(ishash_table));
}

void ISHashInsert(ishash_type ishash,int key,const char *value)
{
	unsigned int hash_value = ((unsigned int)key) % ishash->size;
	ishash_node *node = ishash->nodes[hash_value];

	ishash_node *new_node = (ishash_node *)AllocateMemory(MALLOC_ID_LOAD_GAME,sizeof(ishash_node));
	new_node->next = NULL;
	new_node->key = key;
	new_node->value = (char *)AllocateMemory(MALLOC_ID_LOAD_GAME,strlen(value)+1);
	strcpy(new_node->value,value);
	if (node == NULL)
	{
		ishash->nodes[hash_value] = new_node;
	}
	else
	{
		ishash_node *next = node->next;
		while (next != NULL)
		{
			node = next;
			next = node->next;
		}
		node->next = new_node;
	}
}

const char * ISHashFind(ishash_type ishash,int key)
{
	unsigned int hash_value = ((unsigned int)key) % ishash->size;
	ishash_node *node = ishash->nodes[hash_value];
	while (node != NULL)
	{
		if (node->key == key)
			return node->value;

		node = node->next;
	}
	return NULL;
}

