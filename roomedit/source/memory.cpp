/*----------------------------------------------------------------------------*
 | This file is part of WinDEU, the port of DEU to Windows.                   |
 | WinDEU was created by the DEU team:                                        |
 |  Renaud Paquay, Raphael Quinet, Brendon Wyber and others...                |
 |                                                                            |
 | DEU is an open project: if you think that you can contribute, please join  |
 | the DEU team.  You will be credited for any code (or ideas) included in    |
 | the next version of the program.                                           |
 |                                                                            |
 | If you want to make any modifications and re-distribute them on your own,  |
 | you must follow the conditions of the WinDEU license. Read the file        |
 | LICENSE or README.TXT in the top directory.  If do not  have a copy of     |
 | these files, you can request them from any member of the DEU team, or by   |
 | mail: Raphael Quinet, Rue des Martyrs 9, B-4550 Nandrin (Belgium).         |
 |                                                                            |
 | This program comes with absolutely no warranty.  Use it at your own risks! |
 *----------------------------------------------------------------------------*

	Project WinDEU
	DEU team
	Jul-Dec 1994, Jan-Mar 1995

	FILE:         memory.cpp

	OVERVIEW
	========
	Source file for Memory allocation routines.
*/

#include "common.h"
#pragma hdrstop


/*
   Note from RQ:
	  To prevent memory fragmentation on large blocks (greater than 1K),
	  the size of all blocks is rounded up to 8K.  Thus, "realloc" will
	  move the block if and only if it has grown or shrunk enough to
	  cross a 8K boundary.
	  I don't do that for smaller blocks (smaller than 1K), because this
	  would waste too much space if these blocks were rounded up to 8K.
	  There are lots of "malloc"'s for very small strings (9 characters)
	  or filenames, etc.
	  Thanks to Craig Smith (bcs@cs.tamu.edu) for some of his ideas
	  about memory fragmentation.
*/

#define SIZE_THRESHOLD	1024
#define SIZE_OF_BLOCK	4095  /* actually, this is (size - 1) */



/*
	allocate memory from the far heap with error checking
*/

void *AllocMemory (long size)
{
#if 0
	void *ptr;

	if (size < 1)
		size = 1;
	ptr = malloc(size);
	if (NULL == ptr)
		ProgError ("Out of memory (cannot allocate %lu bytes)", size);
	else
		memset(ptr,0,size);
	return ptr;
#else
	void *ret;
	// DEBUG
	// ULONG asksize = size;

	// limit fragmentation on large blocks
	if (size >= (LONG) SIZE_THRESHOLD)
		size = (size + (ULONG) SIZE_OF_BLOCK) & ~((ULONG) SIZE_OF_BLOCK);

	// TRACE ("Alloc: ask size = " << dec << asksize ", real size = " << size << " bytes");

	ret = malloc (size);

	if (ret == NULL)
		ProgError ("Out of memory (cannot allocate %lu bytes)", size);

	memset (ret, 0, size);

	return ret;
#endif
}



/*
	reallocate memory from the far heap with error checking
*/

void *ReallocMemory (void *old, long size)
{
#if 0
	void *ptr;

	if (size < 1)
		size = 1;
	ptr = realloc(old,size);
	if (NULL == ptr)
		ProgError ("Out of memory! (cannot reallocate %lu bytes)", size);
	return ptr;
#else
	void *ret;
	// DEBUG
	// ULONG asksize = size;

	/* limit fragmentation on large blocks */
	if (size >= (LONG) SIZE_THRESHOLD)
		size = (size + (ULONG) SIZE_OF_BLOCK) & ~((ULONG) SIZE_OF_BLOCK);

	// TRACE ("Realloc: ask size = " << dec << asksize << ", real size = " << size << " bytes");

	ret = realloc (old, size);

	if (ret == NULL)
		ProgError ("Out of memory! (cannot reallocate %lu bytes)", size);

	return ret;
#endif
}


/*
	free memory from the far heap
*/

void UnallocMemory (void *ptr)
{
	/* just a wrapper around farfree(), but provide an entry point */
	/* for memory debugging routines... */
	free( ptr);
}

/*
	Return the available memory (in bytes)
*/

long GetAvailMemory ()
{
	MEMORYSTATUS memstat;

	memstat.dwLength = sizeof (memstat);
	::GlobalMemoryStatus (&memstat);

	LONGLONG avail = (LONGLONG)(memstat.dwAvailPhys + memstat.dwAvailPageFile);

	if (avail > MAXLONG)
		avail = MAXLONG;

	else if (avail < 0)
		avail = 0;

	return (long)avail;
}

/* end of file */
