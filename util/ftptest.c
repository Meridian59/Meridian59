/*
 */

#include <windows.h>
#include <fcntl.h>
#include <sys\stat.h>
#include "wininet.h"

#define BUFSIZE 4096

static char *filename = "pub\\mer0320.arq";
static char *local_filename = "\\junk\\m.arq";
static char *hostname = "198.82.204.39";

static DWORD result;  // Result of last async call
static DWORD error;   // Error code of last async call

static BOOL done = FALSE;

/************************************************************************/
/*
 */
void CALLBACK FtpCallbackFunction(HINTERNET hSession, DWORD context, DWORD status,
			 void *info, DWORD info_length)
{
   LPINTERNET_ASYNC_RESULT r;

   printf("callback, status = %d\n", status);

   switch(status)
   {
   case INTERNET_STATUS_RESOLVING_NAME:
      printf( "Resolving ");
      break;

   case INTERNET_STATUS_NAME_RESOLVED:
      printf( "Resolved ");
      break;
      
   case INTERNET_STATUS_CONNECTING_TO_SERVER:
      printf( "Connecting to ");
      break;
      
   case INTERNET_STATUS_CONNECTED_TO_SERVER:
      printf("Connected to %s ...", (LPTSTR) info);
      break;
      
   case INTERNET_STATUS_SENDING_REQUEST:
      printf("Sending request ");
      break;
      
   case INTERNET_STATUS_REQUEST_SENT:
      printf( "Sent Request ");
      break;
      
   case INTERNET_STATUS_RECEIVING_RESPONSE:
      printf( "Receiving response ");
      break;
      
   case INTERNET_STATUS_RESPONSE_RECEIVED:
      printf( "Received response ");
      break;
      
   case INTERNET_STATUS_CLOSING_CONNECTION:
      printf( "Closing connection ");
      break;
      
   case INTERNET_STATUS_CONNECTION_CLOSED:
      printf( "Closed connection ");
      
      break;
      
   case INTERNET_STATUS_HANDLE_CREATED:
      printf("handle created\n");
      break;

   case INTERNET_STATUS_REQUEST_COMPLETE:
      printf("done\n");
      r = (LPINTERNET_ASYNC_RESULT) info;
      result = r->dwResult;
      error = r->dwError;
      done = TRUE;
      break;
   }
}
/************************************************************************/
main(int argc, char **argv)
{
   HINTERNET hConnection;
   HINTERNET hSession, hFile;
   int outfile;
   char buf[BUFSIZE];
   int size;
   BOOL retval;
   WIN32_FIND_DATA find_data;

#if 1
   hConnection = InternetOpen("Meridian 59", INTERNET_OPEN_TYPE_PRECONFIG, 
			      NULL, NULL, INTERNET_FLAG_ASYNC | INTERNET_FLAG_RELOAD);
#else
   hConnection = InternetOpen("Meridian 59", INTERNET_OPEN_TYPE_PRECONFIG, 
			      NULL, NULL, INTERNET_FLAG_RELOAD);
#endif
   if (hConnection == NULL)
   {
      printf("Unable to open Internet connection\n");
      return 1;
   }
   
#if 1
   if (InternetSetStatusCallback(hConnection, FtpCallbackFunction) == INTERNET_INVALID_STATUS_CALLBACK)
   {
      printf("Setting callback failed\n");
      return 1;      
   }
#endif   

   done = FALSE;
   hSession = InternetConnect(hConnection, hostname, INTERNET_INVALID_PORT_NUMBER, 
			      NULL, NULL, INTERNET_SERVICE_FTP, 0, 0);

   if (hSession == NULL)
   {
      printf("Unable to open ftp connection, error = %d\n", GetLastError());
      return 1;
   }

   printf("connect ok\n");

   done = FALSE;
   hFile = FtpFindFirstFile(hSession, filename, &find_data, INTERNET_FLAG_RELOAD, 1);
   if (hFile == NULL)
   {
      if (GetLastError() != ERROR_IO_PENDING)
      {
	 printf("FindFirst failed, code = %d\n", GetLastError());
	 exit(1);
      }
      while (!done)
	 Sleep(100);
      hFile = (HINTERNET) result;
      if (hFile == NULL)
      {
	 printf("FindFirst failed 2, code = %d\n", GetLastError());
	 exit(1);
      }
   }
   InternetCloseHandle(hFile);
   printf("file size = %d\n", find_data.nFileSizeLow);
  

   done = FALSE;
   hFile = FtpOpenFile(hSession, filename, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD, 1);

   if (hFile == NULL)
   {
      if (GetLastError() != ERROR_IO_PENDING)
      {
	 printf("FtpOpenFile; error = %d\n", GetLastError());
	 exit (1);
      }
      while (!done)
	 Sleep(100);

      hFile = (HINTERNET) result;
      if (hFile == NULL)
      {
	 printf("OpenFile failed, code = %d\n", error);
	 exit(1);
      }
   }

   outfile = open(local_filename, O_BINARY | O_RDWR | O_CREAT, S_IWRITE | S_IREAD);
   if (outfile <= 0)
   {
      printf("open failed\n");
      exit(1);
   }

   printf("starting read\n");

   while (1)
   {
      done = FALSE;
      if (!InternetReadFile(hFile, buf, BUFSIZE, &size))
      {
	 if (GetLastError() != ERROR_IO_PENDING)
	 {
	    printf("InternetReadFileDied; error = %d\n", GetLastError());
	    exit (1);
	 }
	 while (!done)
	    Sleep(100);
      }

      printf("size = %d\n", size);

      if (size == 0)
	 break;
      write(outfile, buf, size);
   }

   printf("ending read\n");

   close(outfile);

   InternetCloseHandle(hFile);
   InternetCloseHandle(hSession);
   InternetCloseHandle(hConnection);
}
