// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * smtpmail.c
 *

 This module is the interface between the blakston server and the smtp modules.

 */

#include "blakserv.h"
#include <winsock2.h>
#include <errno.h>

void DisplayErrnoText(void);
void StartListenSMTP(void);
void __cdecl ThreadAcceptSMTPSocketConnections(int socket_port);

void PageAlert(char *subject,char *message)
{
	char messagebuf[4000];
	char FAR hostname[256];
	unsigned int i;
	int res;
	char *mailServer;
	char *emailAddress;

	/* hostname of computer */
	gethostname((char *)&hostname ,255-1);

	mailServer = ConfigStr( EMAIL_HOST );
	emailAddress = ConfigStr( EMAIL_NAME );

	/* don`t want spaces in either subject or message */
	for(i=0;i<strlen( subject ); i++) {
		if(subject[ i ] == ' ' ) subject[ i ] = '_'; 
	}
	for(i=0;i<strlen( message ); i++) {
		if(message[ i ] == ' ' ) message[ i ] = '_'; 
	}

	/* build the command string , bypass the mail filter .. (its ok we`re inside 3do) */
	/* until i finish the internal mail sender, i`ve ignored the subject parameter */
	sprintf(messagebuf,"mailto -H %s -U %s -S Warning -M %s -D %s",mailServer,hostname,message,emailAddress);
	dprintf(messagebuf);
	
	/* calling system() requires a flush of all buffered streams */
	_flushall();

	/* handle any returns from system */
	res = system(messagebuf);

	/* handle any returns from system */
	if(res==-1) {
		DisplayErrnoText();
	} else	
		dprintf("system command returned %d\n",res);
}

void InitSMTP(void)
{
   if (ConfigBool(EMAIL_LISTEN) == True)
      AcceptSMTPSocketConnections(ConfigInt(EMAIL_PORT));

}

void DisplayErrnoText(void)
{
	switch(errno) {
		case E2BIG:
			dprintf("Argument list (which is system-dependent) is too big.");
			break;
		case ENOENT:
			dprintf("Command interpreter cannot be found.");
			break;
		case ENOEXEC:
			dprintf("Command-interpreter file has invalid format and is not executable.");
			break;
		case ENOMEM:
			dprintf("Not enough memory is available to execute command; or available memory has been corrupted; or invalid block exists, indicating that process making call was not allocated properly.");
			break;
		case ECHILD:
			dprintf("No spawned processes.");
			break;			
		case EAGAIN:			
			dprintf("No more processes. An attempt to create a new process failed because there are no more process slots, or there is not enough memory, or the maximum nesting level has been reached.");
			break;
		case EACCES:		
			dprintf("Permission denied. The file's permission setting does not allow the specified access. This error signifies that an attempt was made to access a file (or, in some cases, a directory) in a way that is incompatible with the file's attributes.");
			break;
		case EBADF:	
			dprintf("Bad file number. There are two possible causes: 1) The specified file handle is not a valid file-handle value or does not refer to an open file. 2) An attempt was made to write to a file or device opened for read-only access.");
			break;
		case EDEADLOCK:
			dprintf("Resource deadlock would occur. The argument to a math function is not in the domain of the function.");
			break;
		case EDOM:
			dprintf("Math argument.");
			break;
		case EEXIST:
			dprintf("Files exist. An attempt has been made to create a file that already exists. For example, the _O_CREAT and _O_EXCL flags are specified in an _open call, but the named file already exists.");
			break;
		case EINVAL:
			dprintf("Invalid argument. An invalid value was given for one of the arguments to a function. For example, the value given for the origin when positioning a file pointer (by means of a call to fseek) is before the beginning of the file.");
			break;
		case EMFILE:
			dprintf("Too many open files. No more file handles are available, so no more files can be opened.");
			break;
		case ENOSPC:
			dprintf("No space left on device. No more space for writing is available on the device (for example, when the disk is full).");
			break;
		case ERANGE:
			dprintf("Result too large. An argument to a math function is too large, resulting in partial or total loss of significance in the result. This error can also occur in other functions when an argument is larger than expected (for example, when the buffer argument to _getcwd is longer than expected).");
			break;
		case EXDEV:
			dprintf("Cross-device link. An attempt was made to move a file to a different device (using the rename function).");
			break;
		default:
			dprintf("Unknown errno condition (%d) !",errno);
			break;	
	}
}
