// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * download.h:  Header file for download.c
 */

#ifndef _DOWNLOAD_H
#define _DOWNLOAD_H

#define MAX_HOSTNAME	256	 // Maximum machine name length
#define MAX_CMDLINE	2048	 // Maximum program command line size
#define MAX_TEXT_REASON	4096	 // Maximum text for reason for update

// Info about each file to download
typedef struct {
   char filename[FILENAME_MAX + 1];
   int  time;           // Download time of file
   int  flags;          // Flags that say what to do with the file
   int	size;		// Size of file to download
} DownloadFileInfo;

// Info about files to download
typedef struct {
   char machine[MAX_HOSTNAME + 1];
   char path[MAX_PATH + 1];
   char reason[MAX_TEXT_REASON+1];
   char demoPath[MAX_PATH+1];
   WORD num_files;
   DownloadFileInfo *files;

   // These fields are used to communicate with the ftp thread
   int  current_file;                  // Index of next file to retrieve
   HWND hPostWnd;                      // Window to receive progress messages
} DownloadInfo;

void DownloadInit(void);
void DownloadExit(void);

void DownloadFiles(DownloadInfo *params);
Bool DownloadCheckDirs(HWND hParent);
void DownloadSetTime(int new_time);

Bool DownloadBferHandleMessage(char *ptr, long len);

void DownloadNewClient(char *hostname, char *filename);

#endif /* #ifndef _DOWNLOAD_H */
