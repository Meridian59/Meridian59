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

void Dearchive(char *dest_path, char *filename)
{
   char msg[500];
   
   SetDlgItemText(hwndMain,IDC_STATUS,GetString(hInst, IDS_UNARCHIVING));

   // Make sure archive is legal
   WrapInit();

   // Does file exist?
   struct stat s;
   if (stat(filename, &s) != 0)
   {
      SetDlgItemText(hwndMain,IDC_STATUS, GetString(hInst, IDS_MISSINGFILE));   
      WrapShutdown();
      return;
   }

   if (!WrapIsArchive(filename))
   {
      SetDlgItemText(hwndMain, IDC_STATUS, GetString(hInst, IDS_FILECORRUPT));
      WrapShutdown();
      return;
   }

   WrapSetExtractionCallback(UnarchiveProgressCallback);

   while (1)
   {
      char temp_path[MAX_PATH];
      extraction_error = 0;
      
      // Crusher's default temp directory doesn't work well in Vista or later;
      // can't extract if installed in Program Files.
      GetTempPath(sizeof(temp_path), temp_path);
      WrapExtractArchive(filename, dest_path, temp_path);
      
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
   WrapSetExtractionCallback(NULL);
   WrapShutdown();

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
bool UnarchiveProgressCallback(const char *filename, ExtractionStatus status)
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
}
