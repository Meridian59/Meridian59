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
   read bytes from a file and store it into an address with error checking
*/

void BasicWadRead (WadPtr wadfile, void *addr, long size)
{
	assert (size < UINT_MAX);
	if (fread( addr, 1, (size_t)size, wadfile->fileinfo) != size)
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

MDirPtr FindMasterDir (MDirPtr from, const char *name)
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
	if ( fread (ptr, 1, (size_t)size, file) != size )
		ProgError ("Error reading from file");
}



/*
   output bytes to a binary file with error checking
*/

void WriteBytes (FILE *file, const void *addr, long size)
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
	if (fwrite (ptr, 1, (size_t)size, file) != size)
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
	if (fread (data, 1, (size_t)size, source) != size)
		ProgError ("Error reading from file");
	if (fwrite (data, 1, (size_t)size, dest) != size)
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
