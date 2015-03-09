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

	FILE:         wads.cpp

	OVERVIEW
	========
	Source file for Wad files routines.
*/

#include "common.h"
#pragma hdrstop

#ifndef __wads_h
	#include "wads.h"
#endif

#ifndef __names_h
	#include "names.h"
#endif

#ifndef __workdlg_h
	#include "workdlg.h"
#endif

/* global variables */
WadPtr  WadFileList = NULL;       /* linked list of wad files */
MDirPtr MasterDir   = NULL;       /* the master directory */


/*
   open the main wad file, read in its directory and create the
   master directory.
   RP: Determine the 'DoomVersion' by looking for 'E?M?' or 'MAP??' entries.
*/

void OpenMainWad(char *filename)
{
	TRACE ("OpenMainWad (" << filename << "): start");
	MDirPtr lastp, newp;
	USHORT n;
	WadPtr wad;

	// Convert to lower case
	strlwr (filename);

	//
	MasterDir = NULL;
	WadFileList = NULL;

	// open the wad file
	wad = BasicWadOpen (filename);
	if ( wad == NULL || strncmp (wad->type, "IWAD", 4))
	{
		ProgError("\"%s\" is not a valid main WAD file.\n"
				  "Edit your WINDEU.INI file and update the \"main\" option "
				  "to point to a valid main WAD file.\n"
				  "Ex:   main=c:\\games\\doom2\\doom2.wad", filename);
	}

	/* create the master directory */
	assert(wad->dirsize < (LONG)USHRT_MAX);
	lastp = NULL;
	for (n = 0; n < (USHORT)wad->dirsize; n++)
	{
		newp = (MDirPtr)GetMemory (sizeof(MasterDirectory));
		newp->next = NULL;
		newp->wadfile = wad;
		memcpy (&(newp->dir), &(wad->directory[n]), sizeof(Directory));

		// Add entry in master directory
		if (MasterDir)
			lastp->next = newp;
		else
			MasterDir = newp;
		lastp = newp;
	}

	/* check if registered version */
	if ( FindMasterDir (MasterDir, "E1M1") != NULL )
	{
		DoomVersion = 1;
		if ( FindMasterDir (MasterDir, "MUMMA1") != NULL )
		{
			DoomVersion = 16;
			/*
			Notify ("The main WAD file in the initialization file is an "
					"HERETIC file. This version of WinDEU doesn't support (yet) "
					"all the features of HERETIC.");
			*/
		}
	}
	else if ( FindMasterDir (MasterDir, "MAP30") != NULL )
	{
		DoomVersion = 2;
		/*
		Notify ("The main WAD file in the initialization file is a "
				"DOOM 2 file. This version of WinDEU doesn't support (yet) "
				"all the features of DOOM 2.");
		*/
	}
	else
		ProgError ("The main WAD file in the initialization file is not "
				   "a valid file.");

#if 0   // junk removed ARK
	if ( DoomVersion == 1 && FindMasterDir (MasterDir, "E2M1" ) == NULL )
	{
		Notify ("This is the shareware version of DOOM.\n"
				"You won't be allowed to save your changes.\n"
				"PLEASE REGISTER YOUR COPY OF THE GAME") ;
		// If you remove this, bad things will happen to you...
		Registered = FALSE;
	}
	else if ( DoomVersion == 16 && FindMasterDir(MasterDir, "E2M1" ) == NULL)
	{
		Notify ("This is the shareware version of HERETIC.\n"
				"You won't be allowed to save your changes.\n"
				"PLEASE REGISTER YOUR COPY OF THE GAME") ;
		// If you remove this, bad things will happen to you...
		Registered = FALSE;
	}
	else
#endif
	   Registered = TRUE;
}



/*
   open a patch wad file, read in its directory and alter the master
   directory
*/

void OpenPatchWad (char *filename)
{
	WadPtr  wad;
	MDirPtr mdir;
	USHORT  n, l;
	char    entryname[9];

	// Wait cursor
	SAVE_WORK_MSG();
	SELECT_WAIT_CURSOR();

	// Convert to lower case
	strlwr (filename);

	/* ignore the file if it doesn't exist */
	if (! Exists( filename))
	{
		Notify ("Warning: patch WAD file \"%s\" doesn't exist.  Ignored.",
				filename);
		goto EndOpenPatchWad;
	}

	/* open the wad file */
	WorkMessage("Loading patch WAD file: %s...", filename);
	wad = BasicWadOpen (filename);
	if (wad == NULL || strncmp (wad->type, "PWAD", 4))
	{
		Notify ("\"%s\" is not a patch WAD file", filename);
		CloseUnusedWadFiles();
		goto EndOpenPatchWad;
	}

	/* alter the master directory */
	assert(wad->dirsize <= (LONG)USHRT_MAX);
	l = 0;
	for (n = 0; n < wad->dirsize; n++)
	{
		memcpy (entryname, wad->directory[n].name, 8L);
		entryname[8] = '\0';

		if (l == 0)
		{
			mdir = FindMasterDir (MasterDir, wad->directory[n].name);

			// if this entry is not in the master directory, then add it
			if (mdir == NULL)
			{
				SHORT NbInsert;

				// Go to end of list
				mdir = MasterDir;
				while (mdir->next)
					mdir = mdir->next;

				// Add only one entry by default
				NbInsert = 1;

				// If we are adding a level from other doom version,
				// create 10 entries for this level
				if ( IsDoomLevelName (entryname, 1) ||
					 IsDoomLevelName (entryname, 2) )
				{
					WorkMessage ("[Adding level %s]", entryname);
					l = 10;
					NbInsert += l;
				}
				else
					WorkMessage ("[Adding new entry %s]", entryname);

				MDirPtr TempPtr = mdir;
				for (int i = 0 ; i < NbInsert; i++)
				{
					// Add entry at end of list
					TempPtr->next = (MDirPtr)GetMemory(sizeof(MasterDirectory));
					TempPtr       = TempPtr->next;
					TempPtr->next = NULL;
				}

				// Go to first added entry
				mdir = mdir->next;
			}

			// if this is a level, then copy this entry and the next 10
			else if ( IsDoomLevelName (entryname, 1) ||
					  IsDoomLevelName (entryname, 2) )
			{
				WorkMessage ("[Updating level %s]", entryname);
				l = 10;
			}

			// Else, it's a regular entry
			else
				WorkMessage ("[Updating entry %s]", entryname);
		}
		else
		{
			mdir = mdir->next;
			// Notify ("mdir = %p, l = %d, n = %d", mdir, l, n);
			// the level data should replace an existing level
			/*RP Removed because we may insert doom 1 PWAD in a DOOM 2
				 master directory, so the entries may be new */
			/*
			if (mdir == NULL ||
				strncmp (mdir->dir.name, wad->directory[n].name, 8))
			{
				ProgError("\%s\" is not an understandable PWAD file "
						  "(error with %s)", filename, entryname);
			}
			*/
			l--;
		}

		mdir->wadfile = wad;
		memcpy (&(mdir->dir), &(wad->directory[n]), sizeof(Directory));
	}

EndOpenPatchWad:
	UNSELECT_WAIT_CURSOR();
	RESTORE_WORK_MSG();
}



/*
   close all the wad files, deallocating the WAD file structures
*/

void CloseWadFiles()
{
	WadPtr  curw, nextw;
	MDirPtr curd, nextd;

	/* close the wad files */
	curw = WadFileList;
	WadFileList = NULL;
	while (curw)
	{
		nextw = curw->next;
		if ( curw->fileinfo  != NULL )	fclose (curw->fileinfo);
		if ( curw->directory != NULL )	FreeMemory (curw->directory);
		if ( curw->filename  != NULL )	FreeMemory (curw->filename);
		FreeMemory (curw);
		curw = nextw;
	}

	/* delete the master directory */
	curd = MasterDir;
	MasterDir = NULL;
	while (curd)
	{
		nextd = curd->next;
		FreeMemory (curd);
		curd = nextd;
	}
}



/*
   forget unused patch wad files
*/

void CloseUnusedWadFiles()
{
	TRACE ("CloseUnusedWadFiles(): start");
	WadPtr curw, prevw;
	MDirPtr mdir;

	prevw = NULL;
	curw = WadFileList;
	while (curw)
	{
		// check if the wad file is used by a directory entry
		mdir = MasterDir;
		while ( (mdir != NULL)  &&  (mdir->wadfile != curw) )
			mdir = mdir->next;

		// If wad file used, go to next
		if (mdir != NULL)
			prevw = curw;

		// if wad file never used, close it
		else
		{

			// Link previous wad file to next wad file
			if (prevw)
				prevw->next = curw->next;
			else
				WadFileList = curw->next;

			// Cleanup wad file
			TRACE ("CloseUnusedWadFiles: closing <" << curw->filename << ">");
			if (curw->fileinfo  != NULL)	fclose (curw->fileinfo);
			if (curw->directory != NULL)	FreeMemory (curw->directory);
			if (curw->filename  != NULL)	FreeMemory (curw->filename);
			FreeMemory (curw);
		}

		/*RP No prevw if closing MainWad file! (error in start of WinDEU) */
		if ( prevw != NULL )
			curw = prevw->next;
		else
			curw = NULL;
	}
}



/*
   basic opening of WAD file and creation of node in Wad linked list
*/

WadPtr BasicWadOpen(char *filename)
{
	TRACE ("BasicWadOpen(" << filename << "): start");
	WadPtr curw, prevw;

	// find the wad file in the wad file list
	prevw = WadFileList;
	if (prevw)
	{
		curw = prevw->next;
		while (curw != NULL  &&  stricmp(filename, curw->filename) != 0)
		{
			prevw = curw;
			curw = prevw->next;
		}
	}
	else
		curw = NULL;

	// if this entry doesn't exist, add it to the WadFileList
	if (curw == NULL)
	{
		curw = (WadPtr)GetMemory( sizeof( struct WadFileInfo));
		if (prevw == NULL)
			WadFileList = curw;
		else
			prevw->next = curw;
		curw->next = NULL;
		curw->filename = (char *)GetMemory (strlen(filename)+1);
		strcpy (curw->filename, filename) ;
		curw->fileinfo = NULL;
		curw->directory = NULL;
	}

	// open the file
	if ((curw->fileinfo = fopen( filename, "rb")) == NULL)
	{
		Notify ("Error opening \"%s\"", filename);
		CloseUnusedWadFiles();
		return NULL;
	}

	// read in the WAD directory info
	BasicWadRead(curw, curw->type, 4);
	if (strncmp(curw->type, "IWAD", 4) && strncmp(curw->type, "PWAD", 4))
	{
		Notify ("\"%s\" is not a valid WAD (IWAD or PWAD) file", filename);
		CloseUnusedWadFiles();
		return NULL;
	}
	BasicWadRead(curw, &curw->dirsize, sizeof( curw->dirsize));
	BasicWadRead(curw, &curw->dirstart, sizeof( curw->dirstart));

	// read in the WAD directory itself
	LONG DirBytes = sizeof(struct Directory) * curw->dirsize;
	curw->directory = (DirPtr)GetMemory(DirBytes);
	BasicWadSeek (curw, curw->dirstart);
	BasicWadRead (curw, curw->directory, DirBytes);

	/* all done */
	return curw;
}



/*
   read bytes from a file and store it into an address with error checking
*/

void BasicWadRead (WadPtr wadfile, void *addr, long size)
{
	assert (size >= 0 && (UINT)size < UINT_MAX);
	if (fread(addr, 1, (size_t)size, wadfile->fileinfo) != (UINT)size)
		ProgError ("Error reading from \"%s\"", wadfile->filename);
}



/*
   go to offset of wad file with error checking
*/

void BasicWadSeek (WadPtr wadfile, long offset)
{
	if ( fseek (wadfile->fileinfo, offset, SEEK_SET) )
		ProgError ("Error reading from \"%s\"", wadfile->filename);
}



/*
   find an entry in the master directory
*/

MDirPtr FindMasterDir (MDirPtr from, char *name)
{
	while (from)
	{
		if (strncmp (from->dir.name, name, 8) == 0)
			break;
		from = from->next;
	}
	return from;
}



/*
   list the master directory
*/
#if 0
void ListMasterDirectory (FILE *file)
{
	char dataname[ 9];
	MDirPtr dir;

	dataname[ 8] = '\0';
	fprintf( file, "The Master Directory\n");
	fprintf( file, "====================\n\n");
	fprintf( file, "NAME____  FILE________________  SIZE__  START____\n");
	for (dir = MasterDir; dir; dir = dir->next)
	{
		strncpy (dataname, dir->dir.name, 8);
		fprintf (file, "%-8s  %-20s  %6ld  x%08lx\n",
					   dataname, dir->wadfile->filename,
					   dir->dir.size, dir->dir.start);
	}
}
#endif



/*
   list the directory of a file
*/

#if 0
void ListFileDirectory( FILE *file, WadPtr wad)
{
   char dataname[9];
   LONG n;

   dataname[8] = '\0';
   fprintf (file, "WAD File Directory\n");
   fprintf (file, "==================\n\n");
   fprintf (file, "Wad File: %s\n\n", wad->filename);
   fprintf (file, "NAME____  SIZE__  START____  END______\n");
   for (n = 0; n < wad->dirsize; n++)
   {
	  strncpy(dataname, wad->directory[n].name, 8);
	  fprintf(file, "%-8s  %6ld  x%08lx  x%08lx\n",
					dataname,
					wad->directory[n].size,
					wad->directory[n].start,
					wad->directory[n].size + wad->directory[n].start - 1);
   }
}
#endif


/*
   build a new wad file from master dictionary
*/

void BuildNewMainWad (char *filename, BOOL patchonly)
{
	FILE   *file;
	LONG    counter = 12;
	MDirPtr cur;
	LONG    size;
	LONG    dirstart;
	LONG    dirnum;
	LONG    total_size;
	char    total_str[20];
	int     num_entries;
	char msg[128];

	SELECT_WAIT_CURSOR();
	SAVE_WORK_MSG();
	TWorkDialog *pWorkDlg = new TWorkDialog;
	pWorkDlg->Create();

	/* Count the total number of bytes to write */
	total_size = 0L;
	for (cur = MasterDir; cur; cur = cur->next)
	{
		// Don't save main wad data
		if (patchonly && cur->wadfile == WadFileList)
			continue;

		total_size += cur->dir.size;
	}
	total_size += 4;	// header
	total_size += 4;	// # of entries
	total_size += 4;	// start
	total_size /= 1024;
	strcpy (total_str, FormatNumber(total_size));

	pWorkDlg->SetRange(0, (int)total_size);
	pWorkDlg->SetValue(0);

	/* open the file and store signatures */
	if ( patchonly )
		wsprintf (msg, "Building a compound Patch Wad file \"%s\" "
					   "(size of %sKb).",
					   filename, total_str);
	else
		wsprintf (msg, "Building a new Main Wad file \"%s\" "
					   "(size of %sKb)",
					   filename, total_str);
	pWorkDlg->SetWorkText(msg);
	/*
	if (patchonly)
		pWorkDlg->SetWorkText ("Building a compound Patch Wad file \"%s\".",
							   filename);
	else
		WorkMessage ("Building a new Main Wad file \"%s\" "
					 "(approximative size of 10.000Kb)",
					 filename);
	*/

	/* Protection, not applicable with DOOM2 */
	if ( (DoomVersion == 1 || DoomVersion == 16) &&
		 FindMasterDir (MasterDir, "E2M4") == NULL)
	{
		ProgError ("You were warned: you are not allowed to do this.");
	}

	if ((file = fopen (filename, "wb")) == NULL)
		ProgError ("Unable to open file \"%s\"", filename);

	if (patchonly)
		WriteBytes (file, "PWAD", 4);
	else
		WriteBytes (file, "IWAD", 4);

	WriteBytes (file, &counter, 4L);      /* put true value in later */
	WriteBytes (file, &counter, 4L);      /* put true value in later */

	/* output the directory data chuncks */
	num_entries = 0;
	for (cur = MasterDir; cur; cur = cur->next)
	{
		// Don't save main wad data
		if (patchonly && cur->wadfile == WadFileList)
			continue;

		size = cur->dir.size;
		counter += size;
		BasicWadSeek (cur->wadfile, cur->dir.start);
		CopyBytes (file, cur->wadfile->fileinfo, size);
		num_entries++;	// Number of entry written ++

		// Update progress dialog (and test cancel button)
		pWorkDlg->SetValue((int)(counter/1024));
		COOPERATE();
		if ( pWorkDlg->Cancel )
		{
			if ( Confirm ("WARNING: Some entries will be missing in the file "
						  "built if you you cancel this operation\n\n"
						  "Do you really want to cancel ?") == TRUE )
			{
				break;
			}
			// Ignore Cancel!
			pWorkDlg->Cancel = FALSE;
		}
	}

	/* output the directory (until 'cur' if cancel) */
	pWorkDlg->SetRange(0, num_entries);
	pWorkDlg->SetValue(0);
	wsprintf(msg, "Outputting %d directory entries...", num_entries);
	pWorkDlg->SetWorkText (msg);

	dirstart = counter;
	counter = 12;
	dirnum = 0;
	for (MDirPtr ncur = MasterDir; ncur != cur; ncur = ncur->next)
	{
		// If patch, don't save files of main wad
		if (patchonly && ncur->wadfile == WadFileList)
			continue;

		if (dirnum % 10 == 0)
		{
			pWorkDlg->SetValue((int)dirnum);
		}

		if (ncur->dir.start)
			WriteBytes (file, &counter, 4L);
		else
			WriteBytes (file, &(ncur->dir.start), 4L);

		WriteBytes (file, &(ncur->dir.size), 4L);
		WriteBytes (file, &(ncur->dir.name), 8L);
		counter += ncur->dir.size;
		dirnum++;
	}

	// fix up the number of entries and directory start information
	if (fseek (file, 4L, 0))
		ProgError ("Error writing to file \"%s\"", filename);

	WriteBytes (file, &dirnum, 4L);
	WriteBytes (file, &dirstart, 4L);

	// close the file
	fclose( file);

	delete pWorkDlg;
	UNSELECT_WAIT_CURSOR();
	RESTORE_WORK_MSG();
}



/*
   Read bytes from a binary file with error checking
*/

void ReadBytes (FILE *file, void *addr, long size)
{
	BYTE *ptr = (BYTE *)addr;

	while (size > 0x8000)
	{
		if ( fread (ptr, 1, 0x8000, file) != 0x8000 )
			ProgError ("Error reading from file");

		ptr += 0x8000;
		size -= 0x8000;
	}
	if ( fread (ptr, 1, (size_t)size, file) != (ULONG)size )
		ProgError ("Error reading from file");
}



/*
   output bytes to a binary file with error checking
*/

void WriteBytes (FILE *file, void *addr, long size)
{
	BYTE *ptr = (BYTE *)addr;

	if (! Registered)
		return;
	while (size > 0x8000)
	{
		if ( fwrite (ptr, 1, 0x8000, file) != 0x8000)
			ProgError( "error writing to file");

		ptr += 0x8000;
		size -= 0x8000;
	}
	if (fwrite (ptr, 1, (size_t)size, file) != (ULONG)size)
		ProgError ("error writing to file");
}



/*
   copy bytes from a binary file to another with error checking
*/

void CopyBytes (FILE *dest, FILE *source, long size)
{
	void *data;

	if (! Registered)
		return;
	data = GetMemory (0x8000 + 2);
	while (size > 0x8000)
	{
		if (fread (data, 1, 0x8000, source) != 0x8000)
			ProgError ("Error reading from file");
		if (fwrite (data, 1, 0x8000, dest) != 0x8000)
			ProgError ("Error writing to file");
		size -= 0x8000;
	}
	if (fread (data, 1, (size_t)size, source) != (ULONG)size)
		ProgError ("Error reading from file");
	if (fwrite (data, 1, (size_t)size, dest) != (ULONG)size)
		ProgError ("Error writing to file");
	FreeMemory(data);
}



/*
   check if a file exists and is readable
*/

BOOL Exists (char *filename)
{
	FILE *test;

	if ((test = fopen( filename, "rb")) == NULL)
		return FALSE;

	fclose( test);
	return TRUE;
}



/*
   dump a directory entry in hex
*/
#if 0
void DumpDirectoryEntry (FILE *file, char *entryname)
{
	MDirPtr entry;
	char    dataname[9];
	ULONG   n, c;
	SHORT   i;
	char    buf[16];


	c = 0;
	entry = MasterDir;
	while (entry)
	{
		if ( strnicmp (entry->dir.name, entryname, 8) == 0)
		{
			strncpy( dataname, entry->dir.name, 8);
			dataname[8] = '\0';
			fprintf(file, "Contents of entry %s (size = %ld bytes):\n",
						  dataname, entry->dir.size);
			BasicWadSeek (entry->wadfile, entry->dir.start);
			n = 0;
			i = -1;
			for (c = 0; c < entry->dir.size; c += i)
			{
				fprintf (file, "%04X: ", n);
				for (i = 0; i < 16; i++)
				{
					BasicWadRead (entry->wadfile, &(buf[i]), 1);
					fprintf (file, " %02X", buf[i]);
					n++;
				}
				fprintf (file, "   ");
				for (i = 0; i < 16; i++)
				{
					if (buf[i] >= 32)
						fprintf (file, "%c", buf[i]);
					else
						fprintf (file, " ");
				}
				fprintf (file, "\n");
			}
		}
		entry = entry->next;
	}

	if (! c)
	{
		Notify ("Entry \"%s\" not in master directory", entryname);
		return;
	}
}
#endif



/*
   save a directory entry to disk
*/

void SaveDirectoryEntry (FILE *file, char *entryname)
{
	MDirPtr entry;
	LONG    counter;
	LONG    size;

	for (entry = MasterDir; entry; entry = entry->next)
		if ( strnicmp (entry->dir.name, entryname, 8) == 0 )
			break;

	if (entry)
	{
		WriteBytes (file, "PWAD", 4L);     /* PWAD file */
		counter = 1L;
		WriteBytes (file, &counter, 4L);   /* 1 entry */
		counter = 12L;
		WriteBytes (file, &counter, 4L);
		counter = 28L;
		WriteBytes (file, &counter, 4L);
		size = entry->dir.size;
		WriteBytes (file, &size, 4L);
		WriteBytes (file, &(entry->dir.name), 8L);
		BasicWadSeek (entry->wadfile, entry->dir.start);
		CopyBytes (file, entry->wadfile->fileinfo, size);
	}
	else
	{
		Notify ("Entry \"%s\" not in master directory", entryname);
		return;
	}
}



/*
   save a directory entry to disk, without a PWAD header
*/

void SaveEntryToRawFile (FILE *file, char *entryname)
{
	MDirPtr entry;

	for (entry = MasterDir; entry; entry = entry->next)
		if ( strnicmp( entry->dir.name, entryname, 8) == 0)
			break;

	if (entry)
	{
		BasicWadSeek (entry->wadfile, entry->dir.start);
		CopyBytes (file, entry->wadfile->fileinfo, entry->dir.size);
	}
	else
	{
		Notify ("Entry \"%s\" not in master directory", entryname);
		return;
	}
}



/*
   encapsulate a raw file in a PWAD file
*/

void SaveEntryFromRawFile (FILE *file, FILE *raw, char *entryname)
{
	LONG    counter;
	LONG    size;
	char    name8[8];

	memset (name8, 0, 8);
	strncpy (name8, entryname, 8);

	WriteBytes (file, "PWAD", 4L);     /* PWAD file */

	counter = 1L;
	WriteBytes (file, &counter, 4L);   /* 1 entry */

	counter = 12L;
	WriteBytes (file, &counter, 4L);

	counter = 28L;
	WriteBytes (file, &counter, 4L);

	if (fseek (raw, 0L, SEEK_END) != 0)
		ProgError ("Error reading from raw file");

	size = ftell (raw);
	if (size < 0)
		ProgError ("Error reading from raw file");

	if (fseek (raw, 0L, SEEK_SET) != 0)
		ProgError ("Error reading from raw file");

	WriteBytes (file, &size, 4L);
	WriteBytes (file, name8, 8L);
	CopyBytes (file, raw, size);
}


/* end of file */
