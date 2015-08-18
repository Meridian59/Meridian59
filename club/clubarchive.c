// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * archive.c:  Handle decompression of downloaded file.
 */

#include "club.h"

static int extraction_error;         // Nonzero if extraction had an error

static int copy_data(struct archive *ar, struct archive *aw)
{
	int r;
	const void *buff;
	size_t size;
	long long offset;

	for (;;) 
	{
		r = archive_read_data_block(ar, &buff, &size, &offset);
		
		if (r == ARCHIVE_EOF)
			return (ARCHIVE_OK);
		
		if (r != ARCHIVE_OK)
			return (r);
		
		r = archive_write_data_block(aw, buff, size, offset);
		
		if (r != ARCHIVE_OK) 
		{
			fprintf(stderr, "%s\n", archive_error_string(aw));
			return (r);
		}
	}
}

bool IsArchive(char *filename)
{
	return true;
}

bool ExtractArchive(char *filename, char* destpath)
{
	struct archive *a;
	struct archive *ext;
	struct archive_entry *entry;
	int flags;
	int r;
	int pathlen = strlen(destpath);

	/* Select which attributes we want to restore. */
	flags = ARCHIVE_EXTRACT_TIME;
	flags |= ARCHIVE_EXTRACT_PERM;
	flags |= ARCHIVE_EXTRACT_ACL;
	flags |= ARCHIVE_EXTRACT_FFLAGS;

	// read from
	a = archive_read_new();
	archive_read_support_format_all(a);
	archive_read_support_compression_all(a);

	// write to
	ext = archive_write_disk_new();
	archive_write_disk_set_options(ext, flags);
	archive_write_disk_set_standard_lookup(ext);
	
	// try open archive
	if ((r = archive_read_open_filename(a, filename, 10240)))
		return false;
	
	// iterate entries
	for (;;) 
	{
		// next read entry
		r = archive_read_next_header(a, &entry);
		
		// end of archive? -> done
		if (r == ARCHIVE_EOF)
			break;
		
		// check error
		if (r != ARCHIVE_OK)
			return false;
		
		// get filename of entry
		const char* filename = archive_entry_pathname(entry);
		int filenamelen = strlen(filename);
		
		// build full extractionpath
		int size = filenamelen + pathlen + 1;
		char* fullfilepath = (char*)calloc(size, 1);
		strcpy_s(fullfilepath, size, destpath);
		strcat_s(fullfilepath, size, filename);
		
		// apply extraction path
		archive_entry_set_pathname(entry, fullfilepath);

		// extract header
		r = archive_write_header(ext, entry);
		
		if (r != ARCHIVE_OK)
			return false;

		else if (archive_entry_size(entry) > 0) 
		{
			// extract file data
			copy_data(a, ext);
			
			if (r != ARCHIVE_OK)
				return false;
		}
	
		// finish this entry
		r = archive_write_finish_entry(ext);
		
		// check for errors
		if (r != ARCHIVE_OK)
			return false;
	}
	
	archive_read_close(a);
	archive_read_free(a);
	archive_write_close(ext);
	archive_write_free(ext);

	// successful
	return true;
}

void Dearchive(char *dest_path, char *filename)
{
   char msg[500];
   
   SetDlgItemText(hwndMain,IDC_STATUS,GetString(hInst, IDS_UNARCHIVING));

   // Make sure archive is legal
   //WrapInit();

   // Does file exist?
   struct stat s;
   if (stat(filename, &s) != 0)
   {
      SetDlgItemText(hwndMain,IDC_STATUS, GetString(hInst, IDS_MISSINGFILE));   
      //WrapShutdown();
      return;
   }

   if (!IsArchive(filename))
   {
      SetDlgItemText(hwndMain, IDC_STATUS, GetString(hInst, IDS_FILECORRUPT));
      //WrapShutdown();
      return;
   }

   //WrapSetExtractionCallback(UnarchiveProgressCallback);

   while (1)
   {
      //char temp_path[MAX_PATH];
      extraction_error = 0;
      
      // Crusher's default temp directory doesn't work well in Vista or later;
      // can't extract if installed in Program Files.
      //GetTempPath(sizeof(temp_path), temp_path);
      ExtractArchive(filename, dest_path);
      
      if (extraction_error == 0)
      {
         success = True;
         break;
      }
      
      sprintf(msg, GetString(hInst, IDS_CANTUNPACK), GetString(hInst, extraction_error));
      
      if (MessageBox(hwndMain, msg, GetString(hInst, IDS_APPNAME), MB_YESNO) == IDNO)
      {
         SetDlgItemText(hwndMain,IDC_STATUS,GetString(hInst, IDS_DEARCHIVEERROR));   
         break;
      }
   }
   
   unlink(filename);
   //WrapSetExtractionCallback(NULL);
   //WrapShutdown();

   if (success)
   {
      Status(GetString(hInst, IDS_RESTARTING));   
      
      PostMessage(hwndMain,WM_CLOSE,0,0);
   }
}
/*****************************************************************************/
/*
 * UnarchiveProgressCallback:  Callback function for each file in an archive.
 */
/*bool UnarchiveProgressCallback(const char *filename, ExtractionStatus status)
{
   switch (status)
   {
   case EXTRACT_DONE:
      ClearMessageQueue();  // Check for user hitting abort button
      break;

   case EXTRACT_CANT_RENAME:
      extraction_error = IDS_CANTRENAME;
      break;

   case EXTRACT_BAD_PERMISSIONS:
      extraction_error = IDS_BADPERMISSION;
      break;

   case EXTRACT_OUT_OF_MEMORY:
      extraction_error = IDS_BADMEM;
      break;

   case EXTRACT_BAD_CRC:
   case EXTRACT_UNKNOWN:
      extraction_error = IDS_BADARCHIVE;
      break;

   case EXTRACT_DISK_FULL:
      extraction_error = IDS_DISKFULL;
      break;
   }

   // Other errors
   if (status != EXTRACT_OK && status != EXTRACT_DONE &&
       extraction_error == 0)
      extraction_error = IDS_UNKNOWNERROR;

   return true;
}*/
