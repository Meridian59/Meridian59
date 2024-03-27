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

int sessions_logged_on = 0;

int fd_epoll;

typedef std::pair<int, int> fd_conn_type;
std::vector<fd_conn_type> accept_sockets;

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

void RunMainLoop(void)
{
   INT64 ms;
   const uint32_t num_notify_events = 500;
   struct epoll_event notify_events[num_notify_events];
   int i;

   signal(SIGPIPE, SIG_IGN);

   while (!GetQuit())
   {
	   ms = GetMainLoopWaitTime();

	   // wait up to ms, dispatch any socket events
	   int val = epoll_wait(fd_epoll, notify_events, num_notify_events, ms);
	   if (val == -1)
	   {
		   eprintf("RunMainLoop error on epoll_wait %s\n", GetLastErrorStr());
	   }
	   //printf("got events %i %lu\n", val, ms);
	   for (i=0;i<val;i++)
	   {
		   if (notify_events[i].events == 0)
			   continue;

		   if (IsAcceptingSocket(notify_events[i].data.fd))
		   {
			   if (notify_events[i].events & ~EPOLLIN)
			   {
				   eprintf("RunMainLoop error on accepting socket %i\n",notify_events[i].data.fd);
			   }
			   else
			   {
				   AsyncSocketAccept(notify_events[i].data.fd,FD_ACCEPT,0,GetAcceptingSocketConnectionType(notify_events[i].data.fd));
			   }
		   }
		   else
		   {
			   if (notify_events[i].events & ~(EPOLLIN | EPOLLOUT))
			   {
				   // this means there was an error
				   AsyncSocketSelect(notify_events[i].data.fd,0,1);
			   }
			   else
			   {
				   if (notify_events[i].events & EPOLLIN)
				   {
					   AsyncSocketSelect(notify_events[i].data.fd,FD_READ,0);
				   }
				   if (notify_events[i].events & EPOLLOUT)
				   {
					   AsyncSocketSelect(notify_events[i].data.fd,FD_WRITE,0);
				   }
			   }
		   }

	   }
	   EnterServerLock();
	   PollSessions(); /* really just need to check session timers */
	   TimerActivate();
	   LeaveServerLock();
   }

   close(fd_epoll);
}

int GetLastError()
{
   return errno;
}

char * GetLastErrorStr()
{
   return strerror(errno);
}

bool FindMatchingFiles(const char *path, const char *extension, std::vector<std::string> *files)
{
	struct dirent *entry;
	DIR *dir = opendir(path);
	if (dir == NULL)
		return false;

   files->clear();
	std::string ext(extension);
	while (entry = readdir(dir))
	{
		std::string filename = entry->d_name;
		if (filename != "." && filename != "..")
		{
			if (filename.length() >= ext.length())
				if (filename.compare(filename.length() - ext.length(), ext.length(), ext) == 0) {
					files->push_back(filename);
				}
		}
	}

	closedir(dir);

	return true;
}

bool BlakMoveFile(const char *source, const char *dest)
{
   // Doesn't work across filesystems, but probably fine for our purposes.
   return rename(source, dest) == 0;
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

void StartupComplete(void)
{
	fd_epoll = epoll_create(1);
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

void StartAsyncSocketAccept(SOCKET sock,int connection_type)
{
	//epoll_event *ee = (epoll_event *) AllocateMemory(MALLOC_ID_NETWORK, sizeof(epoll_event));
	epoll_event ee;
	ee.events = EPOLLIN;
	ee.data.fd = sock;
	if (epoll_ctl(fd_epoll,EPOLL_CTL_ADD,sock,&ee) != 0)
	{
	    eprintf("StartAsyncSocketAccept error adding socket %s\n",GetLastErrorStr());
		return;
    }

	accept_sockets.push_back(std::make_pair(sock, connection_type));
}

HANDLE StartAsyncNameLookup(char *peer_addr,char *buf)
{
	eprintf("StartAsyncNameLookup attempted on linux, not supported\n");
	return 0;
}

void StartAsyncSession(session_node *s)
{
	//epoll_event *ee = (epoll_event *) AllocateMemory(MALLOC_ID_NETWORK, sizeof(epoll_event));
	epoll_event ee;
	ee.events = EPOLLIN | EPOLLOUT | EPOLLET;
	ee.data.fd = s->conn.socket;
	if (epoll_ctl(fd_epoll,EPOLL_CTL_ADD,s->conn.socket,&ee) != 0)
	{
	    eprintf("StartAsyncSession error adding socket %s\n",GetLastErrorStr());
		return;
    }
}

void FatalErrorShow(const char *filename,int line,const char *str)
{
	char s[5000];

	printf(s,"Fatal Error File %s line %i\r\n\r\n%s",filename,line,str);

	exit(1);
}
