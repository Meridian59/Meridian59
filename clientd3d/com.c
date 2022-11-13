// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* com.c: Basic serial port & socket communications routines.
*
* Serial support removed 1/19/96 ARK, because it doesn't look like it's ever going to be
* used, and it vastly complicated the building process (needs universal thunk under NT to
* allow single executable for NT and Win 3.1).
*/

#include <assert.h>

#include "client.h"

#define COM_WRITE_TIMEOUT_MS 1000  /* # of milliseconds to wait for write to complete */

#define MAX_COPYBUF 100     /* Largest message size (in bytes) to copy before sending */

static char readbuf[COMBUFSIZE];  /* Buffer for stuff read from server */
static char tempbuf[COMBUFSIZE];  /* Temporary buffer to hold single message */
static int bufpos;

extern int connection;  /* What type of connection do we have? */

static Bool notification = False;  /* Is notification on? */

static SOCKET sock = INVALID_SOCKET; /* Socket communications handle */
static DWORD dwIP4 = 0;

static BYTE epoch;     // Epoch byte from last server message

// Keep several "streams" of random numbers, each of which is updated every time
// we send a message to the server.  Stream #NUM_STREAMS is used to select the stream
// we use in the current message.  This random number is verified by the server, which
// is running an exact copy of the algorithm.  This is meant to prevent attacks on
// the protocol.
static unsigned int streams[NUM_STREAMS];

/* local function prototypes */
static Bool WriteSocket(char *buf,int numbytes);
static int ReadServerSocket(void);
static void Resynchronize(void);
static unsigned int RandomStreamsStep(void);

SOCKET GetClientSocket()
{
	return sock;
}

DWORD GetHostIP4()
{
	return dwIP4;
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
		ClientError(hInst, hMain, IDS_CANTINITSOCKET);
		return False;
	}
	
	sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock == INVALID_SOCKET) 
	{
		debug(("Error on call to socket; error # was %d\n", WSAGetLastError()));
		closesocket(sock);
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
		debug(("Error on gethostbyname; error # was %d.\n Make sure '%s' is listed in the hosts file.\n",
			WSAGetLastError(), host));
		return False;
	}   
	
	dwIP4 = dest_sin.sin_addr.s_addr;
	
	dest_sin.sin_family = AF_INET;
	dest_sin.sin_port   = htons((WORD) sock_port);        /* Convert to network ordering */
	
	/* Set up notification when the socket is opened or closed, or when data arrives */
	if (!StartReadNotification())
	{
		closesocket(sock);
		return False;
	}
	
	if (connect(sock,(PSOCKADDR) &dest_sin, sizeof(dest_sin)) < 0) 
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			debug(("Error on connect; error # was %d\n", WSAGetLastError()));
			return False;
		}
		
		bufpos = 0;
		
		return True;
}
/********************************************************************/
/*
* ReadServerSocket: Read from the server into a static buffer.
*   Returns # of bytes read, or -1 on error.
*   Buf should be long enough to hold any possible message.
*   ReadServer just reads as much as is waiting to be read and puts
*   it in the buffer.
*/
int ReadServerSocket(void)
{
	int numread;
	
	/* Read as much as we can find */
	numread = recv(sock, readbuf + bufpos, COMBUFSIZE - bufpos, 0);
	
	if (numread <= 0)
	{
		ClientError(hInst, hMain, IDS_COMERROR);
		bufpos = 0;
		return 0;
	}
	
	if (numread >= (COMBUFSIZE - bufpos))
	{
		debug(("Available recv() reached end of buffer.\n"));
	}
	
#if 0
	{
		int i;
		for (i=0; i < numread; i++)
			debug(("%5u", (unsigned int)(unsigned char)(readbuf[bufpos + i])));
	}
#endif
	bufpos += numread;
	
	return numread;
}

/********************************************************************/
/* Return TRUE on success */
Bool WriteSocket(char *buf,int numbytes)
{
	int retval;
	
	while (1)
	{
		retval = send(sock, buf, numbytes, 0);
		if (retval != SOCKET_ERROR)
			return TRUE;
		
		if (WSAGetLastError() != WSAEWOULDBLOCK)
			return FALSE;
	}
}
/********************************************************************/
void CloseConnection(void)
{
	EndReadNotification();
	
	switch (connection)
	{
	case CON_SOCKET:
		closesocket(sock);
		WSACleanup();
		break;
		
	default:
		break;
	}
	connection = CON_NONE;
}

/********************************************************************/
/* Sendxxx adds the header, Writexxx is raw--works with comm programs */

Bool WriteServer(char *buf, UINT numbytes)
{
	switch(connection)
	{
	case CON_SOCKET:
		return WriteSocket(buf, numbytes);
		
	default:
		debug(("WriteServer got bogus connection type %d\n", connection));
		return True;
	}      
}
/********************************************************************/
/*
* SendServer:  Given a message to send to the server, prepare the
*  message prefix, and send the prefix and the message to the server.
* 
*  If numbytes <= MAX_COPYBUF, the prefix and message are sent in a single call to WriteServer.
*  Otherwise, they are written separately.
*/
Bool SendServer(char *msg, UINT numbytes)
{
	WORD length = (WORD) numbytes;
	WORD randnum, temp, crc;
	char copybuf[MAX_COPYBUF + 20], *ptr;
	Bool single_write;
	BYTE byte = 0;
	
	/* 
    * Write out message prefix, then message itself.  The prefix consists of:
    * 2 bytes      length of message
    * 2 bytes      CRC of message
    * 2 bytes      length of message again, to aid error detection
    * 1 byte       server epoch (0 for login state messages)
    */
	
	// Only update random streams in game mode
	if (state == STATE_GAME)
	{
		crc = GetCRC16(msg, numbytes);
		temp = (WORD) msg[0];
		randnum = RandomStreamsStep() & 0xffff;
		randnum = randnum ^ length;
		randnum = randnum ^ (temp << 4);
		randnum = randnum ^ crc;
	}
	
	ptr = copybuf;
	memcpy(ptr, &length, LENBYTES);
	ptr += LENBYTES;
	memcpy(ptr, &randnum, CRCBYTES);
	ptr += CRCBYTES;
	memcpy(ptr, &length, LENBYTES);
	ptr += LENBYTES;
	if (state == STATE_LOGIN)
		memcpy(ptr, &byte, 1);
	else memcpy(ptr, &epoch, 1);
	ptr += 1;
	
	/* If message is small enough, put it into copybuf and send all at once */
	single_write = (numbytes <= MAX_COPYBUF);
	if (single_write)
	{
		memcpy(ptr, msg, numbytes);
		ptr += numbytes;
	}
	
	/* Write out what we have so far */
	if (!WriteServer(copybuf, ptr - copybuf))
	{
		ClientError(hInst, hMain, IDS_CANTWRITE);
		debug(("Socket write error = %d\n", WSAGetLastError()));
		return False;
	}
	
	/* If message is too large, write it out separately */    
	if (!single_write)
		if (!WriteServer(msg, numbytes))
		{
			ClientError(hInst, hMain, IDS_CANTWRITE);
			debug(("Socket write error = %d\n", WSAGetLastError()));
			return False;
		}
		
		return True;
}
/********************************************************************/
/*
* ReadServer:  Read pending bytes from the server into a static buffer.
*   Returns # of bytes read, or -1 on error.
*/
int ReadServer(void)
{
/* There might be extra messages floating around in the message queue, so don't
	perform read if we're not supposed to */
	if (!notification)
		return 0;
	
	switch(connection)
	{
	   case CON_SOCKET:
		   return ReadServerSocket();
		   break;
		   
	   default:
		   assert( 0 );
		   break;
	}      
	assert( 0 );
	return 0;
}
/********************************************************************/
/*
* StartReadNotification:  Turn on notification for reads from serial or socket.
*   Returns True on success.
*/
Bool StartReadNotification(void)
{
	Bool retval;
	
	//   debug(("Starting notification\n"));
	notification = True;
	switch (connection)
	{
	case CON_SOCKET:
	/* Send main window a BK_SOCKETEVENT when data is pending on the socket,
		or when the socket is opened or closed. */
		retval = WSAAsyncSelect(sock, hMain, BK_SOCKETEVENT, FD_READ | FD_CLOSE | FD_CONNECT) >= 0;
		if (retval == False)
			debug(("StartReadNotification failed, error was %d\n", WSAGetLastError()));      
		break;
	}
	
	
	return retval;
}
/********************************************************************/
/*
* EndReadNotification:  Turn off notification for reads from serial or socket.
*/
void EndReadNotification(void)
{
	int retval;
	
	//   debug(("Ending notification\n"));
	switch (connection)
	{
	case CON_SOCKET:
		/* Turn off notifications */
		retval = WSAAsyncSelect(sock, hMain, 0, 0);
		
		if (retval != 0)
			debug(("EndReadNotification failed, error was %d\n", WSAGetLastError()));
		break;
	}
	notification = False;
}


/********************************************************************/
/*
* Resynchronize:  A transmission error has occurred; resynchronize with
*   server.
*   In login mode, we just synchronize (via STATE_STARTUP) and return to main menu.
*   In game mode, we also have to restore our game data.
*/
void Resynchronize(void)
{
	debug(("Error, resynchronizing...\n"));
	
	switch (state)
	{
	case STATE_LOGIN:
		RequestLoginResync();
		MainSetState(STATE_STARTUP);
		break;
		
	case STATE_GAME:
		GameDisplayResync();
		RequestGameResync();
		GameSetState(GAME_RESYNC);
		break;
	}
}
/********************************************************************/
/*
* ProcessMsgHeader:  Verify the header of a login, game, or download mode
*   message.  
*   Returns length of message if an entire legal message found; -1 otherwise.
*/
int ProcessMsgHeader(void)
{
	WORD length, length2;
	WORD crc;
	
	if (bufpos > COMBUFSIZE)
	{
		ClientError(hInst, hMain, IDS_READOVERFLOW);
		bufpos = 0;
		Resynchronize();
		return -1;
	}
	
	/* Wait for entire message header */
	if (bufpos < HEADER_SIZE)
		return -1;
	
	/* Get message length, then CRC, then redundant length */
	memcpy(&length, readbuf, LENBYTES);
	memcpy(&crc, readbuf + LENBYTES, CRCBYTES);
	memcpy(&length2, readbuf + LENBYTES + CRCBYTES, LENBYTES);
	// Save latest epoch byte for us to send in our messages.
	memcpy(&epoch, readbuf + 2 * LENBYTES + CRCBYTES, 1);
	
	/* Make sure that redundant lengths match */
	if (length != length2)
	{
		debug(("Got length mismatch\n"));
		Resynchronize();
		return -1;
	}
	
	if (length > COMBUFSIZE)
	{
		debug(("Got length too large %d\n", length));
		Resynchronize();
		return -1;
	}
	
	/* If we don't have a whole message, we're done for now */
	if (bufpos < length + HEADER_SIZE)
		return -1;
	
#if 0   
	/* Check CRC */
	if (crc != GetCRC16(readbuf + HEADER_SIZE, length))
	{
		debug(("Got bad CRC\n"));
		Resynchronize();
		return -1;
	}
#endif
	
	return length;
}
/********************************************************************/
/*
* ProcessMsgBuffer:  Called after data is read into the buffer; looks
*   for a complete message to send to appropriate handler.
*   Global variable state determines handler that receives message.
*/
void ProcessMsgBuffer(void)
{
	int length;
	
	for(;;) /* keep trying to read messages until buffer empty */
	{
		switch (state)
		{
		case STATE_GAME:
			/* If we're resynchronizing, check for resync stuff, else normal game message */
			if (GameGetState() == GAME_RESYNC)
			{
				length = bufpos;
				
				/* Remove message from buffer, whole thing will be handled */
				bufpos = 0;
				
				if (length > 0)
					AddCharsStartup(readbuf, length);
				return;
			}
			/* otherwise fall through */
			
		case STATE_LOGIN:
		case STATE_CONNECTING:
			length = ProcessMsgHeader();
			
			if (length < 0)
				return;
			
			/* We have a whole message, so handle it--don't pass on header */
			memcpy(tempbuf, readbuf + HEADER_SIZE, length);
			
			/* Remove message from buffer by moving up remainder of buffer */
			memmove(readbuf, readbuf + length + HEADER_SIZE, bufpos - (length + HEADER_SIZE));
			
			/* Note: MUST remove message from queue before handling it; otherwise message
			* arrives during processing of first message (if an error, for example),
			* first message is still in buffer ==> infinite loop 
			*/	 
			bufpos -= (length + HEADER_SIZE);
			
			HandleMessage(tempbuf, length);
			break;
			
		case STATE_STARTUP:
			if (bufpos == 0)
				return;
			
			/* Send a single byte at a time; other messages may lurk later in buffer */
			bufpos--;
			*tempbuf = *readbuf;
			
			/* Remove character from buffer by moving up remainder of buffer */
			memmove(readbuf, readbuf + 1, bufpos);
			
			AddCharsStartup(tempbuf, 1);
			break;
			
		case STATE_TERM:
			if (bufpos == 0)
			{
				DoneCharsTerm();
				return;
			}
			
			/* Send a single character to terminal mode; may switch modes at any time */
			bufpos--;
			*tempbuf = *readbuf;
			
			/* Remove character from buffer by moving up remainder of buffer */
			memmove(readbuf, readbuf + 1, bufpos);
			
			AddCharTerm(tempbuf, 1);
			break;
			
		default:   /* Throw away extra bytes */
		case STATE_DOWNLOAD:
			debug(("ProcessMsgBuffer got %d extra bytes in state %d\n", bufpos, state));
			bufpos = 0;
			return;
		}
		
	}
}

/********************************************************************/
/*
* RandomStreamsInit:  Initialize random streams with given values.
*/
void RandomStreamsInit(unsigned int *init)
{
	int i;
	
	for (i=0; i < NUM_STREAMS; i++)
	{
		streams[i] = init[i];
	}
}
/********************************************************************/
/*
* RandomStreamsStep:  Step each of the streams of pseudorandom numbers,
*   and return one.
*/
unsigned int RandomStreamsStep(void)
{
	int i, stream;
	
	for (i=0; i < NUM_STREAMS; i++)
	{
		// Step this stream
		streams[i] = (streams[i] * 9301 + 49297) % 233280;
	}
	
	stream = streams[NUM_STREAMS - 1] % (NUM_STREAMS - 1);
	return streams[stream];
}
