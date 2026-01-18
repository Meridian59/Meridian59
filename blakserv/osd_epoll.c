// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * osd_epoll.c
 *

 This contains a linux implementation of some networking based on epoll().

 */

#include "blakserv.h"
#include <sys/epoll.h>

int fd_epoll;

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


void StartupComplete(void)
{
	fd_epoll = epoll_create(1);
}


void StartAsyncSocketAccept(SOCKET sock,int connection_type)
{
	epoll_event ee;
	ee.events = EPOLLIN;
	ee.data.fd = sock;
	if (epoll_ctl(fd_epoll,EPOLL_CTL_ADD,sock,&ee) != 0)
	{
	    eprintf("StartAsyncSocketAccept error adding socket %s\n",GetLastErrorStr());
		return;
    }

  AddAcceptingSocket(sock, connection_type);
}

void StartAsyncSession(session_node *s)
{
	epoll_event ee;
	ee.events = EPOLLIN | EPOLLOUT | EPOLLET;
	ee.data.fd = s->conn.socket;
	if (epoll_ctl(fd_epoll,EPOLL_CTL_ADD,s->conn.socket,&ee) != 0)
	{
	    eprintf("StartAsyncSession error adding socket %s\n",GetLastErrorStr());
		return;
    }
}

