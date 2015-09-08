// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * memory.h
 *
 */

#ifndef _MEMORY_H
#define _MEMORY_H

enum
{
   MALLOC_ID_TIMER, MALLOC_ID_STRING, MALLOC_ID_KODBASE, MALLOC_ID_RESOURCE,
   MALLOC_ID_SESSION_MODES, MALLOC_ID_ACCOUNT, MALLOC_ID_USER, MALLOC_ID_MOTD,
   MALLOC_ID_DLLIST, MALLOC_ID_LOADBOF,
   MALLOC_ID_SYSTIMER, MALLOC_ID_NAMEID,
   MALLOC_ID_CLASS, MALLOC_ID_MESSAGE, MALLOC_ID_OBJECT,
   MALLOC_ID_LIST, MALLOC_ID_OBJECT_PROPERTIES,
   MALLOC_ID_CONFIG, MALLOC_ID_ROOM,
   MALLOC_ID_ADMIN_CONSTANTS, MALLOC_ID_BUFFER, MALLOC_ID_LOAD_GAME,
   MALLOC_ID_TABLE, MALLOC_ID_BLOCK, MALLOC_ID_SAVE_GAME,
   
   MALLOC_ID_NUM
};

typedef struct
{
   int allocated[MALLOC_ID_NUM];
} memory_statistics;

#define AllocateMemory(id,size) AllocateMemoryDebug(id,size,__FILE__,__LINE__)
#define AllocateMemoryCalloc(id,count,size) AllocateMemoryCallocDebug(id,count,size,__FILE__,__LINE__)

void InitMemory(void);
memory_statistics * GetMemoryStats(void);
int GetMemoryTotal(void);
int GetNumMemoryStats(void);
const char * GetMemoryStatName(int malloc_id);
void * AllocateMemoryDebug(int malloc_id,int size,const char *filename,int linenumber);
void * AllocateMemoryCallocDebug(int malloc_id, int count, int size, const char *filename, int linenumber);
void FreeMemoryX(int malloc_id,void **ptr,int size);
void * ResizeMemory(int malloc_id,void *ptr,int old_size,int new_size);


/* i want to be able to affect the passed ptr */
#define FreeMemory(m_id,ptr,size) FreeMemoryX(m_id,(void **) &ptr,size)


#endif
