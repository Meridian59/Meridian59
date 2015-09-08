// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* memory.c
*

  This module keeps track of memory usage by most of the system.
  
*/

#include "blakserv.h"

#define NMEMDEBUG

/* charlies little memory checker */



#define MCHK_START	0xBABEC0DE
#define MCHK_END	0xC0DEF00D

/*

  Charlie:
  
	MallocChk : 
	
	  If NDEBUG is *NOT* defined then this routine takes a size, filename and linenumber
	of where the malloc was called from.  (NDEBUG is the VC standard)
	  
		It then allocates a block of memory, plus a little more and sets up a couple
	of unlikely magic numbers at each end of the block of memory, it then returns
	the ptr to the memory just after the checks, the FreeCHK routine knows how
	to handle this , so it frees the correct amount of memory, also the size of the
	requested block of memory is stored in the first longword. so the memory after
	allocation ought to look like this
		
		  
	0x00000	size_requested
	0x00004 0xBABEC0DE
	->0x00008...to size_requested
	0x0000x 0xCODEF00D
			
			  The area pointed to by -> is the location returned. so if a badly behaved
	 program stomps on the last or previous longword it will be detected.
			  
				
			Of course this means that on top of the normal malloc overhead we add
	another 12 bytes..  This can be a stiff penalty, perhaps I may add an
	option that says if the size is less than X don`t do the checking, so
	tiny little mallocs aren`t affected... But given the benefits of having
	the checking it seems a small price to pay..
				  
*/


#ifndef NDEBUG
static void *MallocCHK(size_t size,const char*filename,int linenumber)
#else
static void *MallocCHK(size_t size)
#endif
{
	unsigned long *p;
	unsigned char *tmp;
	
#if defined BLAK_PLATFORM_WINDOWS && !defined NDEBUG
	tmp = (unsigned char*)_malloc_dbg( size + (sizeof(unsigned long)*3), _NORMAL_BLOCK,filename,linenumber );
#else
	tmp = (unsigned char *) malloc(size);
#endif
	
	
	if( tmp == NULL) {
		return NULL ;
	}
	
	p = (unsigned long *) tmp;
	
#ifdef MDEBUG
	printf("Size Req: %d   Size Got: %d\n",size,(size + (sizeof(unsigned long)*3) ));
	printf("Block at: 0x%lx\n",p);
#endif
	
	/* store actual size of allocated block */
	p[0] = size ;
	
	/* store our check token */
	p[1] = MCHK_START ;
	
	p = (unsigned long*) (tmp + ( size + (sizeof(unsigned long)*2) ));
	
	p[0] = MCHK_END ;
	
	tmp += 8;
	
#ifdef MDEBUG
	printf("End:      0x%lx \n",p);
	printf("Actual:   0x%lx \n",tmp);
#endif
	
	/*
	Charlie:
		Boundschecker 6.604 will complain about memory being leaked from this scope, 
		its half right, if theres two messages about it, then it really is
		being leaked
	*/
	return tmp ;
}

static void FreeCHK(void*ptr)
{
	unsigned long *p = (unsigned long *)ptr-2 ;
	size_t size;
	
	if( ptr == NULL) return ;
	
#ifdef MDEBUG
	printf("Check at: 0x%lx\n",&p[0]);
#endif
	
	if( p[1] != MCHK_START ) {
		ptr = (unsigned long*)ptr - 2;
		eprintf("possible memory underrun at 0x%lx\n",ptr);
		
		free( ptr ) ;
		return;
	}
	
	/* fetch the size from the block of memory */
	size = p[0];
	
#ifdef MDEBUG
	printf("Free: Size %ld\n",size);
#endif	
	/* it may have been corrupted */ 
	if(size==0) {
		ptr = (unsigned long*)ptr - 2;
		eprintf("freeing zero length memory block 0x%lx\n",ptr);
		
		free( ptr );
		return ;
	}
	
	p = (unsigned long*)((unsigned char*)ptr + (size));
	
	if( p[0] != MCHK_END ) {
		eprintf("possible memory overrun at 0x%lx\n",ptr);
	}
	
	ptr = (unsigned long*)ptr - 2;
#ifdef MDEBUG
	printf("Free Ok at 0x%lx!\n",ptr);
#endif
	free( ptr );
	
}

#ifndef NDEBUG
static void *CallocCHK(size_t count, size_t size, const char*filename, int linenumber)
#else
static void *CallocCHK(size_t count, size_t size)
#endif
{
   unsigned long *p;
   unsigned char *tmp;

#if defined BLAK_PLATFORM_WINDOWS && !defined NDEBUG
   tmp = (unsigned char*)_calloc_dbg(count, (sizeof(unsigned long) * 3), _NORMAL_BLOCK, filename, linenumber);
#else
   tmp = (unsigned char *)calloc(count, size);
#endif


   if (tmp == NULL) {
      return NULL;
   }

   p = (unsigned long *)tmp;

#ifdef MDEBUG
   printf("Size Req: %d   Size Got: %d\n", count * size, count * (size + (sizeof(unsigned long) * 3)));
   printf("Block at: 0x%lx\n", p);
#endif

   /* store actual size of allocated block */
   p[0] = size;

   /* store our check token */
   p[1] = MCHK_START;

   p = (unsigned long*)(tmp + (count * (size + (sizeof(unsigned long) * 2))));

   p[0] = MCHK_END;

   tmp += 8;

#ifdef MDEBUG
   printf("End:      0x%lx \n", p);
   printf("Actual:   0x%lx \n", tmp);
#endif

   /*
   Charlie:
   Boundschecker 6.604 will complain about memory being leaked from this scope,
   its half right, if theres two messages about it, then it really is
   being leaked
   */
   return tmp;
}

void *ReallocCHK(int malloc_id, void *p,size_t size, size_t old_size )
{

	void *tmp ; 

	tmp = MallocCHK(size,__FILE__,__LINE__) ;

	if( old_size < size )
	{
		/* we're increasing the memory block size */
		memcpy(tmp,p,old_size);
	} else
	{
		/* we're reducing the memory block size */
		memcpy(tmp,p,size);
	}

	FreeCHK( p );

	return tmp;
}

/* end of memory checker routines */

memory_statistics memory_stat;

const char *memory_stat_names[] = 
{
	"Timer", "String", "Kodbase", "Resource", 
		"Session", "Account", "User", "Motd",
		"Dllist", "LoadBof",
		"Systimer", "Nameid",
		"Class", "Message", "Object",
		"List", "Object properties",
		"Configuration", "Rooms",
		"Admin constants", "Buffers", "Game loading",
		"Tables", "Socket blocks", "Game saving",
		
		NULL
};

/* local function prototypes */


void InitMemory(void)
{
	int i;
	
	/* verify that memory_stat_names has a name for every malloc_id */
	i = 0;
	while (memory_stat_names[i] != NULL)
		i++;
	
	if (i != MALLOC_ID_NUM)
		StartupPrintf("InitMemory FATAL there aren't names for every malloc id\n");
	
	for (i=0;i<MALLOC_ID_NUM;i++)
		memory_stat.allocated[i] = 0;
}

memory_statistics * GetMemoryStats(void)
{
	return &memory_stat;
}

int GetMemoryTotal(void)
{
	int i,total;
	
	total = 0;
	
	for (i=0;i<MALLOC_ID_NUM;i++)
		total += memory_stat.allocated[i];
	
	return total;
}

int GetNumMemoryStats(void)
{
	return MALLOC_ID_NUM;
}

const char * GetMemoryStatName(int malloc_id)
{
	return memory_stat_names[malloc_id];
}


void * AllocateMemoryDebug(int malloc_id,int size,const char *filename,int linenumber)
{
	void *ptr;
	
	if (size == 0)
	{
		eprintf("AllocateMemoryDebug zero byte memory block from %s at %d\n",filename,linenumber);
	}

	if (malloc_id < 0 || malloc_id >= MALLOC_ID_NUM)
		eprintf("AllocateMemory allocating memory of unknown type %i\n",malloc_id);
	else
		memory_stat.allocated[malloc_id] += size;
#ifndef NMEMDEBUG


#ifndef NDEBUG
	ptr = MallocCHK(size,filename,linenumber);
#else
	ptr = MallocCHK(size);
#endif

#else // NMEMDEUG

	ptr = malloc( size );

#endif

	if (ptr == NULL)
	{
	/* assume channels started up if allocation error, which might not be true,
		but if so, then there are more serious problems! */
		eprintf("AllocateMemory couldn't allocate %i bytes (id %i)\n",size,malloc_id);
		FatalError("Memory allocation failure");
	}
	/*if (InMainLoop())
	{
		dprintf("M0x%08x %i %i\n",ptr,malloc_id,size);
	}*/
	return ptr;
}

// Same as AllocateMemoryDebug, except calls calloc() for use in arrays. Faster than calling
// malloc and setting the array to NULL.
void * AllocateMemoryCallocDebug(int malloc_id, int count, int size, const char *filename, int linenumber)
{
   void *ptr;

   if (size == 0 || count == 0)
   {
      eprintf("AllocateMemoryCallocDebug zero byte memory block from %s at %d\n",
         filename, linenumber);
   }

   if (malloc_id < 0 || malloc_id >= MALLOC_ID_NUM)
      eprintf("AllocateMemoryCallocDebug allocating memory of unknown type %i\n", malloc_id);
   else
      memory_stat.allocated[malloc_id] += (count * size);
#ifndef NMEMDEBUG


#ifndef NDEBUG
   ptr = CallocCHK(count * size, filename, linenumber);
#else
   ptr = CallocCHK(count * size);
#endif

#else // NMEMDEUG

   ptr = calloc(count, size);

#endif

   if (ptr == NULL)
   {
      /* assume channels started up if allocation error, which might not be true,
      but if so, then there are more serious problems! */
      eprintf("AllocateMemoryCallocDebug couldn't allocate %i bytes (id %i)\n",
         size, malloc_id);
      FatalError("Memory allocation failure");
   }

   return ptr;
}

void FreeMemoryX(int malloc_id,void **ptr,int size)
{
	/*if (InMainLoop())
	{
		dprintf("F0x%08x %i %i\n",ptr,malloc_id,size);
	}*/

	if (malloc_id < 0 || malloc_id >= MALLOC_ID_NUM)
		eprintf("FreeMemory freeing memory of unknown type %i\n",malloc_id);
	else
		memory_stat.allocated[malloc_id] -= size;
	
#ifndef NMEMDEBUG
	FreeCHK(*ptr);
#else
	free( *ptr );
#endif
	
	/* we want to catch any references to this, after the free()  */
	*ptr = (void*)0xDEADC0DE ;
	
}

void * ResizeMemory(int malloc_id,void *ptr,int old_size,int new_size)
{
	void* new_mem;

	/*if (InMainLoop())
	{
		dprintf("R0x%08x %i %i %i\n",ptr,malloc_id,old_size,new_size);
	}*/
	if (malloc_id < 0 || malloc_id >= MALLOC_ID_NUM)
		eprintf("ResizeMemory resizing memory of unknown type %i\n",malloc_id);
	else
		memory_stat.allocated[malloc_id] += new_size-old_size;

#ifndef NMEMDEBUG
	new_mem = ReallocCHK(malloc_id,ptr,new_size,old_size);
#else
	new_mem = realloc(ptr,new_size);
#endif

	if (new_mem == NULL)
	{		
		eprintf("ResizeMemory couldn't reallocate from %i to %i bytes (id %i)\n",old_size,new_size,malloc_id);
		FatalError("Memory reallocation failure");
	}

	return new_mem;
}
