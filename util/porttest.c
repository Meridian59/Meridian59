/*
 * porttest.c:  Try opening port given on command line, and reading bytes
 *   from the port.  Return 0 on success, 1 on error.
 */

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

typedef unsigned char Bool;
enum {False = 0, True = 1};

#define NUM_BYTES 5   // # of bytes to read from server (< amount in 1 message)

static int sock;      // Socket

static Bool debug = False;   // Print debugging messages when True

/************************************************************************/
void Usage(void)
{
   printf("Usage: porttest [options] <machine name> <port number> <timeout in seconds>\n");
   printf("  options =\n");
   printf("  -D      print debugging information");
   exit(1);
}
/************************************************************************/
void dprintf(char *fmt,...)
{
   char s[200];
   va_list marker;
    
   if (!debug)
      return;
    
   va_start(marker,fmt);
   vsprintf(s,fmt,marker);
   va_end(marker);

   printf(s);
}
/********************************************************************/
/*
 * OpenSocketConnection: Open a connection, given the host and socket #.
 *   Returns True on success.
 */
Bool OpenSocketConnection(char *host, int sock_port)
{
   WSADATA WSAData;
   SOCKADDR_IN dest_sin;
   PHOSTENT phe;
   Bool success = False;
   long addr;

   if (WSAStartup(MAKEWORD(1,1), &WSAData) != 0) 
   {
      dprintf("Can't initialize Winsock!\n");
      return False;
   }

   sock = socket(AF_INET,SOCK_STREAM,0);
   if (sock == INVALID_SOCKET) 
   {
      dprintf("Error on call to socket; error # was %d\n", WSAGetLastError());
      return False;
   }

   // Try to interpret host as an address with "." notation
   addr = inet_addr(host);
   if (addr != -1)
   {
      dest_sin.sin_addr.s_addr = addr;
      success = True;
   }
   else
   {
      // Interpret host as a name
      phe = gethostbyname(host);
      if (phe != NULL)
      {
	 memcpy((char *)&(dest_sin.sin_addr), phe->h_addr, phe->h_length);
	 success = True;
      }
   }

   if (!success) 
   {
      dprintf("Error on gethostbyname; error # was %d.\n", WSAGetLastError());
      return False;
   }   

   dest_sin.sin_family = AF_INET;
   dest_sin.sin_port   = htons((WORD) sock_port);        /* Convert to network ordering */

   if (connect(sock,(PSOCKADDR) &dest_sin, sizeof(dest_sin)) < 0) 
      if (WSAGetLastError() != WSAEWOULDBLOCK)
      {
	 dprintf("Error on connect; error # was %d\n", WSAGetLastError());
	 return False;
      }

   return True;
}
/************************************************************************/
/*
 * CanReadBytes:  Tries to read NUM_BYTES bytes from sock, with the given
 *   timeout (in seconds).  Returns True iff bytes are read.
 *   Sets sock to non-blocking mode.
 */
Bool CanReadBytes(int timeout)
{
   u_long temp = 1;
   int retval;
   fd_set set;
   struct timeval t;
   char buf[NUM_BYTES];

   if (ioctlsocket(sock, FIONBIO, &temp) != 0)
   {
      dprintf("Setting socket to non-blocking mode failed, error was %d\n",
	     WSAGetLastError());
      return False;
   }

   FD_ZERO(&set);
   FD_SET(sock, &set);

   t.tv_sec = timeout;
   t.tv_usec = 0;
   retval = select(0, &set, NULL, NULL, &t);

   switch (retval)
   {
   case 0:
      dprintf("Read timed out\n");
      return False;

   case SOCKET_ERROR:
      dprintf("Error in select, code = %d\n", WSAGetLastError());
      return False;
      
   case 1:
      break;

   default:
      dprintf("Select got unexpected return value %d\n", retval);
      return False;
   }

   // Try to read bytes
   retval = recv(sock, buf, NUM_BYTES, 0);

   switch (retval)
   {
   case NUM_BYTES:
      break;

   case 0:
      dprintf("Connection closed during read\n");
      return False;

   case SOCKET_ERROR:
      dprintf("Error in recv, code = %d\n", WSAGetLastError());
      return False;

   default:
      dprintf("Recv got unexpected return value %d\n", retval);
      return False;
   }

   return True;
}
/************************************************************************/
int main(int argc, char **argv)
{
   char machine[500];
   int port, timeout, arg;

   if (argc < 4)
      Usage();

   for (arg = 1; arg < argc; arg++)
   {
      if (argv[arg][0] != '-' && argv[arg][0] != '/')
	 break;

      switch (argv[arg][1])
      {
      case 'D':
      case 'd':
	 debug = True;
	 break;

      default:
	 printf("Ignoring unknown option %c\n", argv[arg][1]);
	 break;
      }
   }   

   if (arg + 2 >= argc)
      Usage();

   strcpy(machine, argv[arg]);
   sscanf(argv[arg + 1], "%d", &port);
   sscanf(argv[arg + 2], "%d", &timeout);

   dprintf("machine name = %s, port = %d\n", machine, port);

   if (!OpenSocketConnection(machine, port))
   {
      dprintf("Socket open failed!\n");
      return 1;
   }

   if (!CanReadBytes(timeout))
   {
      dprintf("Can't read bytes from server!\n");
      return 1;
   }

   dprintf("Connection normal.\n");
   return 0;
}
