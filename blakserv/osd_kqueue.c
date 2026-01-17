// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * osd_kqueue.c
 *

 This contains a linux implementation of some networking based on kqueue (Mac).

 */

#include "blakserv.h"

#include <sys/event.h>

int fd_kqueue;

void RunMainLoop(void)
{
    INT64 ms;
    const uint32_t num_notify_events = 500;
    struct kevent notify_events[num_notify_events];
    int i;
    signal(SIGPIPE, SIG_IGN);
    
    while (!GetQuit())
    {
        ms = GetMainLoopWaitTime();
        
        // Convert milliseconds to timespec for kevent
        struct timespec timeout;
        if (ms >= 0) {
            timeout.tv_sec = ms / 1000;
            timeout.tv_nsec = (ms % 1000) * 1000000;
        }
        
        // wait up to ms, dispatch any socket events
        int val = kevent(fd_kqueue, NULL, 0, notify_events, num_notify_events, 
                        ms >= 0 ? &timeout : NULL);
        
        if (val == -1)
        {
            eprintf("RunMainLoop error on kevent %s\n", GetLastErrorStr());
        }
        
        //printf("got events %i %lu\n", val, ms);
        for (i = 0; i < val; i++)
        {
            // Skip if no events (shouldn't happen with kqueue but being safe)
            if (notify_events[i].filter == 0)
                continue;
                
            int fd = (int)notify_events[i].ident;
            
            if (IsAcceptingSocket(fd))
            {
                // Check for error conditions
                if (notify_events[i].flags & EV_ERROR)
                {
                    eprintf("RunMainLoop error on accepting socket %i\n", fd);
                }
                else if (notify_events[i].filter == EVFILT_READ)
                {
                    AsyncSocketAccept(fd, FD_ACCEPT, 0, GetAcceptingSocketConnectionType(fd));
                }
            }
            else
            {
                // Check for error conditions
                if (notify_events[i].flags & EV_ERROR)
                {
                    // this means there was an error
                    AsyncSocketSelect(fd, 0, 1);
                }
                else
                {
                    if (notify_events[i].filter == EVFILT_READ)
                    {
                        AsyncSocketSelect(fd, FD_READ, 0);
                    }
                    if (notify_events[i].filter == EVFILT_WRITE)
                    {
                        AsyncSocketSelect(fd, FD_WRITE, 0);
                    }
                }
            }
        }
        
        EnterServerLock();
        PollSessions(); /* really just need to check session timers */
        TimerActivate();
        LeaveServerLock();
    }
    close(fd_kqueue);
}



void StartupComplete(void)
{
	fd_kqueue = kqueue();
}

void StartAsyncSocketAccept(SOCKET sock, int connection_type)
{
    struct kevent ev;
    
    // Set up kevent for read events on the listening socket
    EV_SET(&ev, sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    
    if (kevent(fd_kqueue, &ev, 1, NULL, 0, NULL) == -1)
    {
        eprintf("StartAsyncSocketAccept error adding socket %s\n", GetLastErrorStr());
        return;
    }
    
  AddAcceptingSocket(sock, connection_type);
}

void StartAsyncSession(session_node *s)
{
    struct kevent ev[2];  // Need two events for read and write
    
    // Set up kevent for read events
    EV_SET(&ev[0], s->conn.socket, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, NULL);
    
    // Set up kevent for write events  
    EV_SET(&ev[1], s->conn.socket, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, NULL);
    
    if (kevent(fd_kqueue, ev, 2, NULL, 0, NULL) == -1)
    {
        eprintf("StartAsyncSession error adding socket %s\n", GetLastErrorStr());
        return;
    }
}
