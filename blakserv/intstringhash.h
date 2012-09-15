// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef _INTSTRINGHASH_H
#define _INTSTRINGHASH_H

typedef struct ishash_node_struct
{
	struct ishash_node_struct *next;
	int key;
	char *value;
} ishash_node;

typedef struct
{
	ishash_node **nodes;
	int size;
} ishash_table,*ishash_type;

ishash_type CreateISHash(int table_size);
void FreeISHash(ishash_type ishash);
void ISHashInsert(ishash_type ishash,int key,const char *value);
const char * ISHashFind(ishash_type ishash,int key);

#endif

