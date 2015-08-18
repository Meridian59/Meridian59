// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * transfer.c:  Do actual transfer for client downloads.
 *   This uses the WININET DLL from Microsoft.
 */

#include "client.h"

extern HANDLE hThread;  // Handle of transfer thread

static char download_dir[]    = "download";

// Handles to Internet connection, Internet session, and file to transfer
static HINTERNET hConnection, hSession, hFile;

#define BUFSIZE 4096
static char *buf;       // Buffer for reading data

// Semaphore to make transfer thread wait for processing of previous file to finish
static HANDLE hSemaphore;   

static Bool aborted;    // True when user has aborted transfer

static BOOL CALLBACK ErrorDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static void __cdecl DownloadError(HWND hParent, char *fmt, ...);
static void TransferCloseHandles(void);
/************************************************************************/
/*
 * TransferInit:  Prepare for transfer.
 */
void TransferInit(void)
{
   buf = (char *) SafeMalloc(BUFSIZE);
   hSemaphore = CreateSemaphore(NULL, 1, 1, NULL);  // Signaled initially
}
/************************************************************************/
/*
 * TransferClose:  Clean up after transfer.
 */
void TransferClose(void)
{
   SafeFree(buf);
   buf = NULL;
   CloseHandle(hSemaphore);
}
/************************************************************************/
/*
 * TransferStart: Retrieve files; the information needed to set up the
 * transfer is in info.
 *
 * This function is run in its own thread.
 */
void __cdecl TransferStart(void *download_info)
{
   Bool done;
   char filename[MAX_PATH + FILENAME_MAX];
   char local_filename[MAX_PATH + 1];  // Local filename of current downloaded file
   int i;
   int outfile;                   // Handle to output file
   DWORD size;                      // Size of block we're reading
   int bytes_read;                // Total # of bytes we've read
   const char *mime_types[2] = { "application/x-zip-compressed" };
   DWORD file_size;
   DWORD file_size_buf_len;
   DWORD index = 0;
   DownloadInfo *info = (DownloadInfo *) download_info;

   aborted = False;
   hConnection = NULL;
   hSession = NULL;
   hFile = NULL;
   
   hConnection = InternetOpen(szAppName, INTERNET_OPEN_TYPE_PRECONFIG, 
                              NULL, NULL, INTERNET_FLAG_RELOAD);
   
   if (hConnection == NULL)
   {
     DownloadError(info->hPostWnd, GetString(hInst, IDS_CANTINIT));
     return;
   }
   
   if (aborted)
   {
     TransferCloseHandles();
     return;
   }
   
   hSession = InternetConnect(hConnection, info->machine, INTERNET_INVALID_PORT_NUMBER, 
                              NULL, NULL, INTERNET_SERVICE_HTTP, 
                              0, 0);
   if (hSession == NULL)
   {
     DownloadError(info->hPostWnd, GetString(hInst, IDS_NOCONNECTION), info->machine);
     return;
   }
   
   for (i = info->current_file; i < info->num_files; i++)
   {
      if (aborted)
      {
         TransferCloseHandles();
         return;
      }
      
     // Skip non-guest files if we're a guest
     if (config.guest && !(info->files[i].flags & DF_GUEST))
     {
       PostMessage(info->hPostWnd, BK_FILEDONE, 0, i);
       continue;
     }
     
     // If not supposed to transfer file, inform main thread
     if (DownloadCommand(info->files[i].flags) != DF_RETRIEVE)
     {
        // Wait for main thread to finish processing previous file
        WaitForSingleObject(hSemaphore, INFINITE);

       if (aborted)
       {
         TransferCloseHandles();
         return;
       }
       
       PostMessage(info->hPostWnd, BK_FILEDONE, 0, i);
       continue;
     }
     
      sprintf(filename, "%s%s", info->path, info->files[i].filename);

      hFile = HttpOpenRequest(hSession, NULL, filename, NULL, NULL,
                              mime_types,
                              INTERNET_FLAG_NO_UI, 0);
      if (hFile == NULL)
      {
         debug(("HTTPOpenRequest failed, error = %d, %s\n",
                GetLastError(), GetLastErrorStr()));
        DownloadError(info->hPostWnd, GetString(hInst, IDS_CANTFINDFILE), 
                      filename, info->machine);
        return;
      }

      if (!HttpSendRequest(hFile, NULL, 0, NULL, 0)) {
         debug(("HTTPSendRequest failed, error = %d, %s\n",
                GetLastError(), GetLastErrorStr()));
        DownloadError(info->hPostWnd, GetString(hInst, IDS_CANTSENDREQUEST), 
                      filename, info->machine);
        return;
      }
      
      // Get file size
      file_size_buf_len = sizeof(file_size);
      index = 0;
      if (!HttpQueryInfo(hFile, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
                         &file_size, &file_size_buf_len, &index)) {
         debug(("HTTPQueryInfo failed, error = %d, %s\n",
                GetLastError(), GetLastErrorStr()));
        DownloadError(info->hPostWnd, GetString(hInst, IDS_CANTGETFILESIZE), 
                      filename, info->machine);
        return;
      }

      PostMessage(info->hPostWnd, BK_FILESIZE, i, file_size);
      
      sprintf(local_filename, "%s\\%s", download_dir, info->files[i].filename);
      
      outfile = open(local_filename, O_BINARY | O_RDWR | O_CREAT, S_IWRITE | S_IREAD);
      if (outfile <= 0)
      {
        debug(("Couldn't open local file %s for writing\n", local_filename));
        DownloadError(info->hPostWnd, GetString(hInst, IDS_CANTWRITELOCALFILE), 
                      local_filename);
        return;
      }
      
      // Read first block
      done = False;
      bytes_read = 0;
      while (!done)
      {
        if (!InternetReadFile(hFile, buf, BUFSIZE, &size))
        {
          DownloadError(info->hPostWnd, GetString(hInst, IDS_CANTREADFTPFILE), filename);
        }
        
        if (size > 0)
        {
          if (write(outfile, buf, size) != size)
          {
            DownloadError(info->hPostWnd, GetString(hInst, IDS_CANTWRITELOCALFILE), 
                          local_filename);
            close(outfile);
            return;
          }
        }
	 
        // Update graph position
        bytes_read += size;
        PostMessage(info->hPostWnd, BK_PROGRESS, 0, bytes_read);
        
        // See if done with file
        if (size == 0)
        {
          close(outfile);
          InternetCloseHandle(hFile);
          done = True;
          
          // Wait for main thread to finish processing previous file
          WaitForSingleObject(hSemaphore, INFINITE);

          if (aborted)
          {
            TransferCloseHandles();
            return;
          }
          
          PostMessage(info->hPostWnd, BK_FILEDONE, 0, i);
        }
      }
   }

   InternetCloseHandle(hSession);
   InternetCloseHandle(hConnection);   
   
   PostMessage(info->hPostWnd, BK_TRANSFERDONE, 0, 0);
}
/************************************************************************/
/*
 * TransferAbort:  Stop transfer thread.
 */
void TransferAbort(void)
{
   aborted = True;

   ReleaseSemaphore(hSemaphore, 1, NULL);  // If transfer thread waiting, it will abort

   // Wait for thread to end, so that we can clean up (e.g. free memory that
   // the thread might reference).
   
   if (hThread != NULL)
      if (WaitForSingleObject(hThread, 2000) == WAIT_TIMEOUT)
         TerminateThread(hThread, 0);   // Wait for 2 seconds, then kill thread
   hThread = NULL;
}
/************************************************************************/
/*
 * DownloadError:  Display given error message, and ask for retry.
 */
void __cdecl DownloadError(HWND hParent, char *fmt, ...)
{
   char s[200];
   va_list marker;
   Bool was_aborted = aborted;
   int retval;

   // Only show error dialog box if not from a user abort
   if (!was_aborted)
   {
      va_start(marker,fmt);
      vsprintf(s,fmt,marker);
      va_end(marker);
      
      retval = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FTPERROR), hMain, ErrorDialogProc, 
			   (LPARAM) s);
      
      if (retval == IDOK)
         PostMessage(hParent, BK_TRANSFERSTART, 0, 0);
      else PostMessage(hParent, WM_COMMAND, IDCANCEL, 0);
   }

   TransferAbort();
}
/*****************************************************************************/
/*
 * ErrorDialogProc:  Dialog procedure for displaying error and asking for retry.
 */
BOOL CALLBACK ErrorDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
   case WM_INITDIALOG:
      SetDlgItemText(hDlg, IDC_FTPMESSAGE, (char *) lParam);
      CenterWindow(hDlg, GetParent(hDlg));
      return FALSE;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
	 EndDialog(hDlg, IDOK);
	 return TRUE;
      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
   }
   return FALSE;
}

/************************************************************************/
/*
 * TransferContinue:  Tell transfer thread to stop waiting.
 */
void TransferContinue(void)
{
   ReleaseSemaphore(hSemaphore, 1, NULL);
}
/************************************************************************/
/*
 * TransferCloseHandles:  Close WININET handles when transfer aborted.
 */
void TransferCloseHandles(void)
{
   if (hConnection != NULL)
      InternetCloseHandle(hConnection);

   if (hSession != NULL)
      InternetCloseHandle(hSession);

   if (hFile != NULL)
      InternetCloseHandle(hFile);

   hConnection = NULL;
   hSession = NULL;
   hFile = NULL;   
}
