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
#define LIBARCHIVE_STATIC
#include "archive.h"
#include "archive_entry.h"

static int extraction_error;         // Nonzero if extraction had an error

/*****************************************************************************/
static int CopyArchiveData(struct archive *ar, struct archive *aw)
{
  const void *buff;
  size_t size;
  __int64 offset;

  while (true)
  {
     int r = archive_read_data_block(ar, &buff, &size, &offset);
     if (r == ARCHIVE_EOF)
        return ARCHIVE_OK;
     if (r != ARCHIVE_OK)
        return r;
     r = archive_write_data_block(aw, buff, size, offset);
     if (r != ARCHIVE_OK)
        return r;
  }
}
/*****************************************************************************/
static bool ExtractArchive(const char *zip_file, const char *out_dir)
{
   struct archive *input = archive_read_new();
   archive_read_support_format_all(input);
   archive_read_support_compression_all(input);
   struct archive *output = archive_write_disk_new();
   archive_write_disk_set_options(output, ARCHIVE_EXTRACT_TIME);
   archive_write_disk_set_standard_lookup(output);
   const int BLOCK_SIZE = 65536;
   int r = archive_read_open_filename(input, zip_file, BLOCK_SIZE);

   if (r != ARCHIVE_OK)
   {
      extraction_error = IDS_BADARCHIVE;
      return false;
   }

   // libarchive can only extract into the current directory, so we
   // need to set it and restore it.
   char original_dir[MAX_PATH];
   getcwd(original_dir, sizeof(original_dir));
   chdir(out_dir);
   
   bool retval = true;
   while (true)
   {
      struct archive_entry *entry;
      r = archive_read_next_header(input, &entry);
      if (r == ARCHIVE_EOF)
         break;
      
      if (r != ARCHIVE_OK)
      {
         extraction_error = IDS_BADARCHIVE;
         retval = false;
         break;
      }
      r = archive_write_header(output, entry);
      if (r != ARCHIVE_OK)
      {
		 const char *msg = archive_error_string(output);
         extraction_error = IDS_UNKNOWNERROR;
         retval = false;
         break;
      }

      if (archive_entry_size(entry) > 0)
      {
         r = CopyArchiveData(input, output);
         if (r != ARCHIVE_OK)
         {
            const char *msg = archive_error_string(output);
            extraction_error = IDS_UNKNOWNERROR;
            retval = false;
            break;
         }
      }
      r = archive_write_finish_entry(output);
      if (r != ARCHIVE_OK)
      {
         extraction_error = IDS_UNKNOWNERROR;
         retval = false;
         break;
      }

      // Extraction went OK; process Windows messages
      ClearMessageQueue();
   }
   archive_read_close(input);
   archive_read_free(input);
   archive_write_close(output);
   archive_write_free(output);

   // Go back to original working directory
   chdir(original_dir);

   return retval;
}
/*****************************************************************************/
void Dearchive(const char *dest_path, const char *zip_name)
{
   char msg[500];
   
   SetDlgItemText(hwndMain,IDC_STATUS,GetString(hInst, IDS_UNARCHIVING));

   // Make sure archive is legal

   // Does file exist?
   struct stat s;
   if (stat(zip_name, &s) != 0)
   {
      SetDlgItemText(hwndMain,IDC_STATUS, GetString(hInst, IDS_MISSINGFILE));   
      return;
   }

   while (1)
   {
      extraction_error = 0;

      ExtractArchive(zip_name, dest_path);
      
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
   
   unlink(zip_name);

   if (success)
   {
      Status(GetString(hInst, IDS_RESTARTING));   
      
      PostMessage(hwndMain,WM_CLOSE,0,0);
   }
}
