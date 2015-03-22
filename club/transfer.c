// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * transfer.c:  Do actual file transfer for club.
 */

#include "club.h"

// Handles to Internet connection, session, and file to transfer
static HINTERNET hConnection, hSession, hFile;

#define BUFSIZE 4096
static char buf[BUFSIZE];

static int outfile;                   // Handle to output file
static DWORD size;                    // Size of block we're reading
static int bytes_read;                // Total # of bytes we've read

/************************************************************************/
Bool TransferStart(void)
{
   Bool done;
   const char *mime_types[2] = { "application/x-zip-compressed", NULL };
   char file_size_buf[20];
   DWORD file_size_buf_len = sizeof(file_size_buf);
   DWORD index = 0;
   int file_size;

   hConnection = InternetOpen(GetString(hInst, IDS_APPNAME), INTERNET_OPEN_TYPE_PRECONFIG, 
                              NULL, NULL, INTERNET_FLAG_RELOAD);
   
   if (hConnection == NULL)
   {
     Error(GetString(hInst, IDS_CANTINIT), GetLastError(), GetLastErrorStr());
     PostMessage(hwndMain, CM_RETRYABORT, 0, 0);
     return False;
   }

   hSession = InternetConnect(hConnection, transfer_machine.c_str(), 
                              INTERNET_INVALID_PORT_NUMBER, 
                              NULL, NULL, INTERNET_SERVICE_HTTP, 
                              0, 0);

   if (hSession == NULL)
   {
     if (GetLastError() != ERROR_IO_PENDING)
     {
        Error(GetString(hInst, IDS_NOCONNECTION), transfer_machine.c_str(),
              GetLastError(), GetLastErrorStr());
        InternetCloseHandle(hConnection);
        PostMessage(hwndMain, CM_RETRYABORT, 0, 0);
        return False;
      }
   }

   hFile = HttpOpenRequest(hSession, NULL, transfer_filename.c_str(), NULL, NULL,
                           mime_types, INTERNET_FLAG_NO_UI, 0);
   if (hFile == NULL)
   {
      if (GetLastError() != ERROR_IO_PENDING)
      {
         Error(GetString(hInst, IDS_CANTFINDFILE), transfer_filename.c_str(), transfer_machine.c_str(), 
               GetLastError(), GetLastErrorStr());
         InternetCloseHandle(hSession);
         InternetCloseHandle(hConnection);
         PostMessage(hwndMain, CM_RETRYABORT, 0, 0);
         return False;
      }
   }
   
   if (!HttpSendRequest(hFile, NULL, 0, NULL, 0)) {
      Error(GetString(hInst, IDS_CANTSENDREQUEST), transfer_filename.c_str(), transfer_machine.c_str(), 
            GetLastError(), GetLastErrorStr());
      InternetCloseHandle(hFile);
      InternetCloseHandle(hSession);
      InternetCloseHandle(hConnection);
      PostMessage(hwndMain, CM_RETRYABORT, 0, 0);
      return False;
   }
   
   // Get file size
   if (!HttpQueryInfo(hFile, HTTP_QUERY_CONTENT_LENGTH,
                      file_size_buf, &file_size_buf_len, &index)) {
      Error(GetString(hInst, IDS_CANTGETFILESIZE), transfer_filename.c_str(), transfer_machine.c_str(), 
            GetLastError(), GetLastErrorStr());
      InternetCloseHandle(hFile);
      InternetCloseHandle(hSession);
      InternetCloseHandle(hConnection);
      PostMessage(hwndMain, CM_RETRYABORT, 0, 0);
      return False;
   }
   
   sscanf(file_size_buf, "%d", &file_size);
   PostMessage(hwndMain, CM_FILESIZE, 0, file_size);
   
   outfile = open(transfer_local_filename.c_str(), O_BINARY | O_RDWR | O_CREAT | O_TRUNC,
                  S_IWRITE | S_IREAD);
   if (outfile < 0)
   {
      Error(GetString(hInst, IDS_CANTWRITELOCALFILE), transfer_local_filename.c_str(), 
           GetLastError(), GetLastErrorStr());
     InternetCloseHandle(hFile);
     InternetCloseHandle(hSession);
     InternetCloseHandle(hConnection);
     PostMessage(hwndMain, CM_RETRYABORT, 0, 0);
     return False;
   }

   // Read first block
   done = False;
   bytes_read = 0;
   while (!done)
   {
     if (!InternetReadFile(hFile, buf, BUFSIZE, &size))
     {
       if (GetLastError() != ERROR_IO_PENDING)
       {
          Error(GetString(hInst, IDS_CANTREADFTPFILE), transfer_filename.c_str(), 
                GetLastError(), GetLastErrorStr());
          PostMessage(hwndMain, CM_RETRYABORT, 0, 0);
       }
     }
     
     Status(GetString(hInst, IDS_READBLOCK));
     
     if (size > 0)
     {
       if (write(outfile, buf, size) != size)
       {
          Error(GetString(hInst, IDS_CANTWRITELOCALFILE), transfer_local_filename.c_str(), 
                GetLastError(), GetLastErrorStr());
          close(outfile);
          InternetCloseHandle(hFile);
          InternetCloseHandle(hSession);
          InternetCloseHandle(hConnection);
          PostMessage(hwndMain, CM_RETRYABORT, 0, 0);
          return False;
       }
     }
     
     // Update graph position
     bytes_read += size;
     PostMessage(hwndMain, CM_PROGRESS, 0, bytes_read);
     
     // See if done with file
     if (size == 0)
     {
       close(outfile);
       
       InternetCloseHandle(hFile);
       InternetCloseHandle(hSession);
       InternetCloseHandle(hConnection);   
       done = True;
     }
   }
   
   PostMessage(hwndMain, CM_DEARCHIVE, 0, 0);
   return True;
}
