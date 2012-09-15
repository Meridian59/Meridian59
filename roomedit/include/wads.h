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

	FILE:         wads.h
*/
#ifndef __wads_h
#define __wads_h

#ifndef __common_h
	#include "common.h"
#endif

/*
   the directory structure is the structre used by DOOM to order the
   data in it's WAD files
*/

typedef struct Directory *DirPtr;
struct Directory
{
   LONG start;      /* offset to start of data */
   LONG size;       /* byte size of data */
   char name[8];	/* name of data block */
};



/*
   The wad file pointer structure is used for holding the information
   on the wad files in a linked list.

   The first wad file is the main wad file. The rest are patches.
*/

typedef struct WadFileInfo *WadPtr;
struct WadFileInfo
{
   WadPtr next;         /* next file in linked list */
   char *filename;      /* name of the wad file */
   FILE *fileinfo;      /* C file stream information */
   char type[4];        /* type of wad file (IWAD or PWAD) */
   LONG dirsize;        /* directory size of WAD */
   LONG dirstart;       /* offset to start of directory */
   DirPtr directory;    /* array of directory information */
};



/*
   the master directory structure is used to build a complete directory
   of all the data blocks from all the various wad files
*/

typedef struct MasterDirectory *MDirPtr;
struct MasterDirectory
{
   MDirPtr   next;        	/* next in list */
   WadPtr    wadfile;      	/* file of origin */
   Directory dir;    		/* directory data */
};



/* from wads.cpp */
extern WadPtr  WadFileList; /* list of wad files */
extern MDirPtr MasterDir;   /* the master directory */


/* from wads.c */
void OpenMainWad (char *);
void OpenPatchWad (char *);
void CloseWadFiles (void);
void CloseUnusedWadFiles (void);
WadPtr BasicWadOpen (char *);
void BasicWadRead (WadPtr, void *, long);
void BasicWadSeek (WadPtr, long);
MDirPtr FindMasterDir( MDirPtr, char *);
void ListMasterDirectory( FILE *);
void ListFileDirectory( FILE *, WadPtr);
void BuildNewMainWad( char *, Bool);
void WriteBytes( FILE *, void *, long);
void CopyBytes( FILE *, FILE *, long);
int Exists( char *);
void DumpDirectoryEntry( FILE *, char *);
void SaveDirectoryEntry( FILE *, char *);
void SaveEntryToRawFile( FILE *, char *);
void SaveEntryFromRawFile( FILE *, FILE *, char *);

#endif