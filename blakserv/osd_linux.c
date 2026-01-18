// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * osd_linux.c
 *

 This contains a linux implementation of some os-dependent functions.

 */

#include "blakserv.h"

static int sessions_logged_on = 0;

typedef std::pair<int, int> fd_conn_type;
static std::vector<fd_conn_type> accept_sockets;

bool IsAcceptingSocket(int sock)
{
	std::vector<fd_conn_type>::iterator it;
	for (it=accept_sockets.begin();it!=accept_sockets.end();++it)
	{
		if ((*it).first == sock)
		{
			return true;
		}
	}
	return false;
}

int GetAcceptingSocketConnectionType(int sock)
{
	std::vector<fd_conn_type>::iterator it;
	for (it=accept_sockets.begin();it!=accept_sockets.end();++it)
	{
		if ((*it).first == sock)
		{
			return (*it).second;
		}
	}
	return 0;
}

void AddAcceptingSocket(int sock, int connection_type)
{
	accept_sockets.push_back(std::make_pair(sock, connection_type));
}

int GetLastError()
{
   return errno;
}

char * GetLastErrorStr()
{
   return strerror(errno);
}

void InitInterface(void)
{
}

// this should probably be moved into core, os-independent code
int GetUsedSessions(void)
{
	return sessions_logged_on;
}

void StartupPrintf(const char *fmt,...)
{
	char s[200];
	va_list marker;

	va_start(marker,fmt);
	vsnprintf(s, sizeof(s), fmt,marker);


	if (strlen(s) > 0)
	{
		if (s[strlen(s)-1] == '\n') /* ignore \n char at the end of line */
			s[strlen(s)-1] = 0;
	}
	va_end(marker);

	printf("Startup: %s\n", s);
}


void InterfaceUpdate(void)
{

}

void InterfaceLogon(session_node *s)
{
	sessions_logged_on++;
}

void InterfaceLogoff(session_node *s)
{
	sessions_logged_on--;
}

void InterfaceUpdateSession(session_node *s)
{

}

void InterfaceUpdateChannel(void)
{

}

void InterfaceSignalConsole()
{
	printf("Paging the administrator\n");
}

void InterfaceSendBufferList(buffer_node *blist)
{
	// this doesn't happen on linux as we have no console admin interface
}

void InterfaceSendBytes(char *buf,int len_buf)
{
	// this doesn't happen on linux as we have no console admin interface
}


HANDLE StartAsyncNameLookup(char *peer_addr,char *buf)
{
	eprintf("StartAsyncNameLookup attempted on linux, not supported\n");
	return 0;
}


void FatalErrorShow(const char *filename,int line,const char *str)
{
	char s[5000];

	printf(s,"Fatal Error File %s line %i\r\n\r\n%s",filename,line,str);

	exit(1);
}
