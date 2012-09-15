// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef _STRINGINTHASH_H
#define _STRINGINTHASH_H

typedef struct sihash_node_struct
{
	struct sihash_node_struct *next;
	char *key;
	int value;
} sihash_node;

typedef struct
{
	sihash_node **nodes;
	int size;
} sihash_table,*sihash_type;

sihash_type CreateSIHash(int table_size);
void FreeSIHash(sihash_type sihash);
void SIHashInsert(sihash_type sihash,const char *key,int value);
Bool SIHashFind(sihash_type sihash,const char *key,int *value);
const char * SIHashFindByValue(sihash_type sihash,int value);

#endif

