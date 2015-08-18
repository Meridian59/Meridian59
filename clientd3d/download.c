// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * download.c:  Handle downloading files.  The actual file reading is done in transfer.c.
 */

#include "client.h"
#include "archive.h"
#include "archive_entry.h"
#include "shlobj.h"

static DownloadInfo *info;  // Info on download

static char download_dir[]    = "download";
static char resource_dir[]    = "resource";
static char help_dir[]        = "help";
static char mail_dir[]        = "mail";
static char run_dir[]         = ".";
static char ad_dir[]          = "ads";

static HWND hDownloadDialog = NULL;   /* Non-NULL if download dialog is up */

static Bool abort_download = FALSE;   // True if user aborts download
static Bool advert = FALSE;           // True if user aborts download
static int  extraction_error = 0;        // Resource id of error string; 0 if none

static int total = 0;
static char format[256];

extern int  connection;        /* Type of connection (serial, etc.) */

// URL to download the patcher if not installed.
static char update_internet[]  = "http://openmeridian.org/patcher";
// Program to run to update the client executable
static char update_program[] = "\\OpenMeridian\\Open Meridian Patch and Client Management.appref-ms";
static char update_filename[] = "blakston.arq"; // Name to call updated archive

#define PING_DELAY 30000       // # of milliseconds between pings to server
static int  timer_id;          // Timer for sending pings to server during download (0 if none)

HANDLE hThread = NULL;         // Handle of transfer thread

/* local function prototypes */
static BOOL CALLBACK DownloadDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static BOOL CALLBACK AskDownloadDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static void _cdecl TransferMessage(char *fmt, ...);
static void AbortDownloadDialog(void);
static Bool DownloadDone(DownloadFileInfo *file_info);
static Bool DownloadDeleteFile(char *filename);
static Bool DownloadUnarchiveFile(char *zip_name, char *dir);
/*****************************************************************************/
bool FileExists(const char *filename)
{
  struct stat buffer;   
  return stat(filename, &buffer) == 0;
}
/*****************************************************************************/
/*
 * DownloadFiles:  Bring up download dialog.
 */
void DownloadFiles(DownloadInfo *params)
{
   int retval, dialog, i;

   info = params;
   info->current_file = 0;
   MainSetState(STATE_DOWNLOAD);

   debug(("machine = %s\n", info->machine));
   debug(("path = %s\n", info->path));
   for (i = 0; i < info->num_files; i++)
      debug(("file = %s, time = %d, flags = %d\n", 
	     info->files[i].filename, info->files[i].time, info->files[i].flags));

   // If downloading only advertisements, show a different dialog to avoid the appearance
   // of a "real" download.
   dialog = IDD_DOWNLOADAD;
   for (i = 0; i < info->num_files; i++)
      if (DownloadLocation(info->files[i].flags) != DF_ADVERTISEMENT)
      {
         dialog = IDD_DOWNLOAD;
         break;
      }

   advert = (IDD_DOWNLOADAD == dialog);
   if (!advert  && !config.avoidDownloadAskDialog)
   {
      retval = DialogBox(hInst, MAKEINTRESOURCE(IDD_ASKDOWNLOAD), NULL, AskDownloadDialogProc);
      switch (retval) {
      case 3: // do the demo button
         WebLaunchBrowser(info->demoPath);
         SendMessage(hMain,WM_SYSCOMMAND,SC_CLOSE,0);
         return;
      case IDOK: // proceed with download
         retval = DialogBox(hInst, MAKEINTRESOURCE(dialog), NULL, DownloadDialogProc);
         advert = FALSE;
         if (retval == IDOK)
         {
            MainSetState(STATE_LOGIN);
            i = (((MAJOR_REV * 100) + MINOR_REV) * P_CATCH) + P_CATCH;
            RequestGame(config.download_time,i,config.comm.hostname);
            return;
         }
         break;
      case IDCANCEL: // cancel
      default:
         break;
      }
   }
   abort_download = True;
   config.quickstart = FALSE;
   //MainSetState(STATE_OFFLINE);
   Logoff();
   ShowWindow(hMain, SW_SHOW);
   UpdateWindow(hMain);

#if 0
   // If we were hung up, just leave
   if (state != STATE_DOWNLOAD)
      return;
   
   MainSetState(STATE_LOGIN);

   switch (retval)
   {
   case IDOK:
      RequestGame(config.download_time);
      break;

   case IDCANCEL:
      Logoff();
      break;
   }
#endif
}
/*****************************************************************************/
/*
 * DownloadCheckDirs:  Make sure that directories needed for downloading exist.
 *   hParent is parent for error dialog.
 *   Return True iff they all exist.
 */
Bool DownloadCheckDirs(HWND hParent)
{
   // Make sure that necessary subdirectories exist
   if (MakeDirectory(download_dir) == False)
   {
      ClientError(hInst, hMain, IDS_CANTMAKEDIR, download_dir, GetLastErrorStr());
      return False;
   }
   if (MakeDirectory(resource_dir) == False)
   {
      ClientError(hInst, hMain, IDS_CANTMAKEDIR, resource_dir, GetLastErrorStr());
      return False;
   }
   if (MakeDirectory(help_dir) == False)
   {
      ClientError(hInst, hMain, IDS_CANTMAKEDIR, help_dir, GetLastErrorStr());
      return False;
   }
   if (MakeDirectory(mail_dir) == False)
   {
      ClientError(hInst, hMain, IDS_CANTMAKEDIR, mail_dir, GetLastErrorStr());
      return False;
   }
   if (MakeDirectory(ad_dir) == False)
   {
      ClientError(hInst, hMain, IDS_CANTMAKEDIR, ad_dir, GetLastErrorStr());
      return False;
   }
   return True;
}

BOOL CALLBACK AskDownloadDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   char buffer[256];
   int i,size;
   double hours, minutes, seconds;
   double bytes, kb, mb;

   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, hMain);
      ShowWindow(hMain, SW_HIDE);
      hDownloadDialog = hDlg;
      SetWindowText(GetDlgItem(hDlg,IDC_ASK_DOWNLOAD_REASON),info->reason);
      size = 0;
      for (i = 0; i < (int)info->num_files; i++)
         size += info->files[i].size;
      
      bytes = (double)size;
      kb = bytes / 1024;
      mb = kb / 1024;
      if ((int)info->num_files < 2)
         sprintf(buffer,"There is one file %d bytes in size.",size);
      else if (kb < 1.0)
         sprintf(buffer,"There are %d files totaling %d bytes in size.",(int)info->num_files,size);
      else if (mb < 1.0)
         sprintf(buffer,"There are %d files totaling %.1f Kb in size.",(int)info->num_files,kb);
      else
         sprintf(buffer,"There are %d files totaling %.1f Mb in size.",(int)info->num_files,mb);
      SetWindowText(GetDlgItem(hDlg,IDC_SIZE_UPDATE),buffer);
      
      seconds = floor(bytes / 5600.0 + 10.0);
      minutes = seconds / 60.0;
      hours = minutes / 60.0;
      
      if (hours > 1.0)
         sprintf(buffer,"Estimating %.1f hours with a 56k modem.",hours);
      else if (minutes > 1.0)
         sprintf(buffer,"Estimating %.1f minutes with a 56k modem.",minutes);
      else 
         sprintf(buffer,"Estimating %.0f seconds with a 56k modem.",seconds);
      SetWindowText(GetDlgItem(hDlg,IDC_TIME_UPDATE_566),buffer);
      
      seconds = floor(bytes / 2800.0 + 10.0);
      minutes = seconds / 60.0;
      hours = minutes / 60.0;
      
      if (hours > 1.0)
         sprintf(buffer,"Estimating %.1f hours with a 28k modem.",hours);
      else if (minutes > 1.0)
         sprintf(buffer,"Estimating %.1f minutes with a 28k modem.",minutes);
      else 
         sprintf(buffer,"Estimating %.0f seconds with a 28k modem.",seconds);
      SetWindowText(GetDlgItem(hDlg,IDC_TIME_UPDATE_288),buffer);
      
      //SetWindowText(GetDlgItem(hDlg,IDC_BTN_DEMO),info->demoPath);
      
      break;
   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDC_BTN_DEMO:
         EndDialog(hDlg, 3);
         return TRUE;
      case IDCANCEL:
         abort_download = True;
         EndDialog(hDlg, IDCANCEL);
         return TRUE;
      case IDC_UPDATE:
         EndDialog(hDlg, IDOK);
         return TRUE;
      }
   }
   return FALSE;
}
/*****************************************************************************/
/*
 * DownloadDialogProc:  Dialog procedure for displaying downloading progress.
 */
BOOL CALLBACK DownloadDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   int fraction, i;
   HWND hGraph;
   BOOL bResult = FALSE;
   char temp[256];

   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, hMain);
      if (!advert)
      {
         ShowWindow(hMain, SW_HIDE);
      }
      hDownloadDialog = hDlg;
      
      // Set up graph bar limits
      hGraph = GetDlgItem(hDlg, IDC_GRAPH);
      SendMessage(hGraph, GRPH_RANGESET, 0, 100);
      SendMessage(hGraph, GRPH_POSSET, 0, 0);
      SendMessage(hGraph, GRPH_COLORSET, GRAPHCOLOR_BAR, GetColor(COLOR_BAR1));
      SendMessage(hGraph, GRPH_COLORSET, GRAPHCOLOR_BKGND, GetColor(COLOR_BAR2));

      hGraph = GetDlgItem(hDlg, IDC_FILEGRAPH);
      SendMessage(hGraph, GRPH_RANGESET, 0, 100);
      SendMessage(hGraph, GRPH_POSSET, 0, 0);
      SendMessage(hGraph, GRPH_COLORSET, GRAPHCOLOR_BAR, GetColor(COLOR_BAR1));
      SendMessage(hGraph, GRPH_COLORSET, GRAPHCOLOR_BKGND, GetColor(COLOR_BAR2));

      hGraph = GetDlgItem(hDlg, IDC_ANIMATE1);
      bResult = Animate_Open(hGraph, MAKEINTRESOURCE(IDA_DOWNLOAD));

      abort_download = False;
      PostMessage(hDlg, BK_TRANSFERSTART, 0, 0);

      GetDlgItemText(hDlg, IDC_FILESIZE, format, sizeof(format));
      sprintf(temp, format, (int)0, (int)0);
      SetDlgItemText(hDlg, IDC_FILESIZE, temp);

      return TRUE;

   case WM_DESTROY:
      if (!advert)
      {
         ShowWindow(hMain, SW_SHOW);
         UpdateWindow(hMain);
      }
      hDownloadDialog = NULL;
      return TRUE;

   case BK_TRANSFERSTART:
      info->hPostWnd = hDlg;
      hThread = (HANDLE)(unsigned long)_beginthread(TransferStart, 0, info);
      TransferMessage(GetString(hInst, IDS_CONNECTING), info->machine);
      return TRUE;

   case BK_FILESIZE:  // wParam is file index, lParam is file size
      if (wParam == 0)
         TransferMessage(GetString(hInst, IDS_RETRIEVING));

      SetDlgItemText(hDlg, IDC_FILENAME, info->files[wParam].filename);
      total = lParam;
      sprintf(temp, format, 0, total);
      SetDlgItemText(hDlg, IDC_FILESIZE, temp);
      SendDlgItemMessage(hDlg, IDC_GRAPH, GRPH_POSSET, 0, 0);
      SendDlgItemMessage(hDlg, IDC_GRAPH, GRPH_RANGESET, 0, total);
      return TRUE;

   case BK_PROGRESS:

      // Update this file's progress indicator.
      SendDlgItemMessage(hDlg, IDC_GRAPH, GRPH_POSSET, 0, lParam);

      // Update this file's progress text message.
      sprintf(temp, format, (int)lParam, (int)total);
      SetDlgItemText(hDlg, IDC_FILESIZE, temp);

      // Compute the fraction for the overall graph.
      fraction = 0;
      if (total != 0)
         fraction = lParam * 100 / total;
      fraction = (fraction + 100 * info->current_file) / info->num_files;
      
      // Update overall progress indicator.
      SendDlgItemMessage(hDlg, IDC_FILEGRAPH, GRPH_POSSET, 0, fraction);

      return TRUE;

   case BK_FILEDONE:  /* lParam is index of file in info */
      if (abort_download)
      {
         AbortDownloadDialog();
         return TRUE;
      }
      
      if (DownloadDone(&info->files[lParam]))
      {
         if (abort_download)
         {
            AbortDownloadDialog();
            return TRUE;
         }
         
         // Set download time
         DownloadSetTime(info->files[lParam].time);
         
         // If we're a guest, there may be additional files that we are supposed to skip.
         // If so, we should set our download time to the last file, so that we will skip
         // the download on the next entry into the game.
         if (config.guest)
            for (i = lParam + 1; i < info->num_files; i++)
            {
               if (info->files[i].flags & DF_GUEST)
                  break;
               DownloadSetTime(info->files[i].time);
            }
         
         info->current_file++;
         
         // Tell transfer thread to continue
         TransferContinue();
         
         TransferMessage(GetString(hInst, IDS_RETRIEVING));
      }
      else AbortDownloadDialog();
      return TRUE;
      
   case BK_TRANSFERDONE: 
      EndDialog(hDlg, IDOK);
      return TRUE;
      
   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDCANCEL:
         abort_download = True;
         EndDialog(hDlg, IDCANCEL);
         return TRUE;
      }
   }
   return FALSE;
}
/*****************************************************************************/
/*
 * TransferMessage:  Display given message in download dialog.
 */
void _cdecl TransferMessage(char *fmt, ...)
{
   char s[200];
   va_list marker;

   if (hDownloadDialog == NULL)
      return;

   va_start(marker,fmt);
   vsprintf(s,fmt,marker);
   va_end(marker);

   SetDlgItemText(hDownloadDialog, IDC_MESSAGE, s);
}
/*****************************************************************************/
/*
 * DownloadSetTime:  Save the given time as the time of the last successful download.
 */
void DownloadSetTime(int new_time)
{
   TimeSettingsSave(new_time);
}
/*****************************************************************************/
/*
 * AbortDownloadDialog:  Kill off download dialog
 */
void AbortDownloadDialog(void)
{
   if (hDownloadDialog != NULL)
      SendMessage(hDownloadDialog, WM_COMMAND, IDCANCEL, 0);
   TransferAbort();
}
/*****************************************************************************/
/*
 * DownloadDone:  Unpack archive and move downloaded files into correct directory,
 *   if appropriate.
 *   zip_filename is the name of the file on the disk.
 *   file_info gives info about downloaded file.
 *   Return True iff successful.
 */
Bool DownloadDone(DownloadFileInfo *file_info)
{
   char zip_name[MAX_PATH + FILENAME_MAX];    // Name of uncompressed file
   char *destination_dir;

   // If we're a guest, skip non-guest files
   if (config.guest && !(file_info->flags & DF_GUEST))
      return True;
 
   sprintf(zip_name, "%s\\%s", download_dir, file_info->filename);

   switch (DownloadLocation(file_info->flags))
   {
   case DF_DIRRESOURCE:
      destination_dir = resource_dir;
      break;

   case DF_DIRCLIENT:
      destination_dir = run_dir;
      break;

   case DF_DIRHELP:
      destination_dir = help_dir;
      break;

   case DF_DIRMAIL:
      destination_dir = mail_dir;
      break;

   case DF_ADVERTISEMENT:
      destination_dir = ad_dir;
      break;

   case DF_DIRWIN:
   case DF_DIRWINSYS:
      // XXX Unimplemented
      debug(("Windows dir file flags unimplemented\n"));
      destination_dir = run_dir;
      break;

   default:
      debug(("Got bad file location flags %d\n", DownloadLocation(file_info->flags)));
      return False;
   }

   debug(("destination = %s\n", destination_dir));

   // Post-processing on file
   switch (DownloadCommand(file_info->flags))
   {
   case DF_DELETE:
      sprintf(zip_name, "%s\\%s", destination_dir, file_info->filename);
      if (!DownloadDeleteFile(zip_name))
         return False;
      break;

   case DF_RETRIEVE:
      if (!DownloadUnarchiveFile(zip_name, destination_dir))
      {
         unlink(zip_name);
         return False;
      }
      unlink(zip_name);
      break;

   default:
      debug(("Got unknown download command %d\n", DownloadCommand(file_info->flags)));
      break;
   }

   debug(("Got file successfully\n"));

   return True;
}
/*****************************************************************************/
/*
 * DownloadDeleteFile:  Delete given file.  Return True on success.
 */
Bool DownloadDeleteFile(char *filename)
{
   struct stat s;
   Bool done = False;

   debug(("deleting file %s\n", filename));

   // Ignore if file doesn't exist
   if (stat(filename, &s) != 0)
      return True;
   
   while (!done)
   {
      if (!DeleteFile(filename))
      {
         if (!AreYouSure(hInst, hMain, YES_BUTTON, IDS_CANTDELETEFILE, filename))
            return False;
      }
      else done = True;
   }
   return True;
}
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
bool ExtractArchive(const char *zip_file, const char *out_dir)
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
      debug(("Error opening archive %s: %s\n", zip_file, archive_error_string(input)));
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
         debug(("Error reading archive header: %s\n", archive_error_string(input)));
         extraction_error = IDS_BADARCHIVE;
         retval = false;
         break;
      }
      r = archive_write_header(output, entry);
      if (r != ARCHIVE_OK)
      {
         debug(("Error writing archive header: %s\n", archive_error_string(input)));
         extraction_error = IDS_UNKNOWNERROR;
         retval = false;
         break;
      }

      if (archive_entry_size(entry) > 0)
      {
         r = CopyArchiveData(input, output);
         if (r != ARCHIVE_OK)
         {
            debug(("CopyArchiveData error: %s\n", archive_error_string(input)));
            extraction_error = IDS_UNKNOWNERROR;
            retval = false;
            break;
         }
      }
      r = archive_write_finish_entry(output);
      if (r != ARCHIVE_OK)
      {
         debug(("write_finish_entry error: %s\n", archive_error_string(input)));
         extraction_error = IDS_UNKNOWNERROR;
         retval = false;
         break;
      }

      debug(("Archive file %s extracted\n", archive_entry_pathname(entry)));
      
      // Extraction went OK; process Windows messages
      ClearMessageQueue();  // Check for user hitting abort button

      // Check for user abort
      if (abort_download)
      {
         retval = false;
         break;
      }
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
/*
 * DownloadUnarchiveFile: Unarchive archive zip_name to given
 *   directory.  Return True on success.
 */
Bool DownloadUnarchiveFile(char *zip_name, char *dir)
{
   Bool retval = True;

   // Does file exist?
   struct stat s;
   if (stat(zip_name, &s) != 0)
   {
      ClientError(hInst, hDownloadDialog, IDS_MISSINGARCHIVE, zip_name);
      return False;
   }

   while (1)
   {
      extraction_error = 0;
      TransferMessage(GetString(hInst, IDS_DECOMPRESSING));

      ExtractArchive(zip_name, dir);
      
      if (extraction_error == 0)
         // This means the user hit the abort button
         break;
      
      if (!AreYouSure(hInst, hDownloadDialog, YES_BUTTON, IDS_CANTUNCOMPRESS, 
                      zip_name, GetString(hInst, extraction_error)))
      {
         retval = False;
         break;
      }
   }

   return retval;
}
/*****************************************************************************/
/*
 * DownloadPingProc:  In response to a timer going off, send a ping message to the server.
 */
void CALLBACK DownloadPingProc(HWND hwnd, UINT msg, UINT timer, DWORD dwTime)
{
   RequestLoginPing();
}
/*****************************************************************************/
/*
 * DownloadInit:  Enter STATE_DOWNLOAD.
 */
void DownloadInit(void)
{
   TransferInit();

   // Start ping timer
   timer_id = SetTimer(NULL, 0, PING_DELAY, DownloadPingProc);
}
/*****************************************************************************/
/*
 * DownloadExit:  Leave STATE_DOWNLOAD.
 */
void DownloadExit(void)
{
   AbortDownloadDialog();
   TransferClose();

   // Free structure from server message that started download
   SafeFree(info->files);
   SafeFree(info);
   info = NULL;

   // Kill ping timer
   if (timer_id != 0)
   {
      KillTimer(NULL, timer_id);
      timer_id = 0;
   }
}

/*****************************************************************************/
/*
 * DownloadNewClient:  Open patcher from the default location if installed,
 *                     otherwise send user to OpenMeridian.org webpage to
 *                     download it.
 */
void DownloadNewClient(char *hostname, char *filename)
{
   SHELLEXECUTEINFO shExecInfo;
   TCHAR szPath[MAX_PATH];

   if (AreYouSure(hInst, hMain, YES_BUTTON, IDS_NEEDNEWVERSION))
   {
      // No longer use club.exe to update the Meridian executable, instead
      // we run the patcher if installed, otherwise send the user to download it.

      if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL, 0, szPath)))
         strcat(szPath, TEXT(update_program));

      shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
      shExecInfo.fMask = 0;
      shExecInfo.hwnd = NULL;
      shExecInfo.lpVerb = TEXT("open");

      shExecInfo.lpFile = TEXT(szPath);
      shExecInfo.lpParameters = NULL;

      shExecInfo.lpDirectory = NULL;
      shExecInfo.nShow = SW_SHOW;
      shExecInfo.hInstApp = NULL;

      if (!ShellExecuteEx(&shExecInfo))
      {
         // Running patcher failed, send them to the webpage. If that fails,
         // throw an error (telling the user to contact us at the forums) and exit.
         shExecInfo.lpFile = TEXT(update_internet);
         if (!ShellExecuteEx(&shExecInfo))
            ClientError(hInst, hMain, IDS_CANTUPDATE);
      }
   }

   // Quit client
   PostMessage(hMain, WM_DESTROY, 0, 0);
}
