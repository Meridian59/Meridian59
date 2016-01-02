// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* async.c
*

  This module contains functions to handle asynchronous socket events
  (i.e. new connections, reading/writing, and closing.
  
	Every function here is called from the interface thread!
	
*/

#include "blakserv.h"

/* local function prototypes */
void AcceptSocketConnections(int socket_port,int connection_type);
void AsyncEachSessionNameLookup(session_node *s);

Bool CheckMaintenanceMask(SOCKADDR_IN *addr,int len_addr);

void AsyncSocketClose(SOCKET sock);
void AsyncSocketWrite(SOCKET sock);
void AsyncSocketRead(SOCKET sock);

#define MAX_MAINTENANCE_MASKS 15
char *maintenance_masks[MAX_MAINTENANCE_MASKS];
int num_maintenance_masks = 0;
char *maintenance_buffer = NULL;

void InitAsyncConnections(void)
{
#ifdef BLAK_PLATFORM_WINDOWS
	WSADATA WSAData; 
	
	if (WSAStartup(MAKEWORD(1,1),&WSAData) != 0)
	{
		eprintf("InitAsyncConnections can't open WinSock!\n");
		return;
	}
#endif
   
	maintenance_buffer = (char *)malloc(strlen(ConfigStr(SOCKET_MAINTENANCE_MASK)) + 1);
	strcpy(maintenance_buffer,ConfigStr(SOCKET_MAINTENANCE_MASK));

	// now parse out each maintenance ip
	maintenance_masks[num_maintenance_masks] = strtok(maintenance_buffer,";");
	while (maintenance_masks[num_maintenance_masks] != NULL)
	{
		num_maintenance_masks++;
		if (num_maintenance_masks == MAX_MAINTENANCE_MASKS)
			break;
		maintenance_masks[num_maintenance_masks] = strtok(NULL,";");		
	}

	/*
	{
		int i;
		for (i=0;i<num_maintenance_masks;i++)
		{
			dprintf("mask %i is [%s]\n",i,maintenance_masks[i]);
		}
	}
	*/
}

void ExitAsyncConnections(void)
{
#ifdef BLAK_PLATFORM_WINDOWS
	if (WSACleanup() == SOCKET_ERROR)
		eprintf("ExitAsyncConnections can't close WinSock!\n");
#endif   
}

void AsyncSocketStart(void)
{
	AcceptSocketConnections(ConfigInt(SOCKET_PORT),SOCKET_PORT);
	AcceptSocketConnections(ConfigInt(SOCKET_MAINTENANCE_PORT),SOCKET_MAINTENANCE_PORT);
}

/* connection_type is either SOCKET_PORT or SOCKET_MAINTENANCE_PORT, so we
keep track of what state to send clients into. */
void AcceptSocketConnections(int socket_port,int connection_type)
{
	SOCKET sock;
	SOCKADDR_IN sin;
	struct linger xlinger;
	int opt;
	
	sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock == INVALID_SOCKET) 
	{
		eprintf("AcceptSocketConnections socket() failed WinSock code %i\n",
			GetLastError());
		closesocket(sock);
		return;
	}
	
	/* Set a couple socket options for niceness */
	
	xlinger.l_onoff=0;
	if (setsockopt(sock,SOL_SOCKET,SO_LINGER,(char *)&xlinger,sizeof(xlinger)) < 0)
	{
		eprintf("AcceptSocketConnections error setting sock opts 1\n");
		return;
	}
	
	opt=1;
	if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char *)&opt,sizeof opt) < 0)
	{
		eprintf("AcceptSocketConnections error setting sock opts 2\n");
		return;
	}
	
	if (!ConfigBool(SOCKET_NAGLE))
	{
		/* turn off Nagle algorithm--improve latency? */
		opt = true;
		if (setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,(char *)&opt,sizeof opt))
		{
			eprintf("AcceptSocketConnections error setting sock opts 3\n");
			return;
		}
	}
	
	memset(&sin,0,sizeof sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons((short)socket_port);
	
	if (bind(sock,(struct sockaddr *) &sin,sizeof(sin)) == SOCKET_ERROR) 
	{
		eprintf("AcceptSocketConnections bind failed, WinSock error %i\n",
			GetLastError());
		closesocket(sock);
		return;
	}	  
	
	if (listen(sock,5) < 0) /* backlog of 5 connects by OS */
	{
		eprintf("AcceptSocketConnections listen failed, WinSock error %i\n",
			GetLastError());
		closesocket(sock);
		return;
	}
	
	StartAsyncSocketAccept(sock,connection_type);
	/* when we get a connection, it'll call AsyncSocketAccept */
}

void AsyncSocketAccept(SOCKET sock,int event,int error,int connection_type)
{
	SOCKET new_sock;
	SOCKADDR_IN acc_sin;    /* Accept socket address - internet style */
	socklen_t acc_sin_len;        /* Accept socket address length */
	SOCKADDR_IN peer_info;
	socklen_t peer_len;
	struct in_addr peer_addr;
	connection_node conn;
	session_node *s;
	
	if (event != FD_ACCEPT)
	{
		eprintf("AsyncSocketAccept got non-accept %i\n",event);
		return;
	}
	
	if (error != 0)
	{
		eprintf("AsyncSocketAccept got error %i\n",error);
		return;
	}
	
	acc_sin_len = sizeof acc_sin; 
	
	new_sock = accept(sock,(struct sockaddr *) &acc_sin,&acc_sin_len);
	if (new_sock == SOCKET_ERROR) 
	{
		eprintf("AcceptSocketConnections accept failed, error %i\n",
			GetLastError());
		return;
	}
	
	peer_len = sizeof peer_info;
	if (getpeername(new_sock,(SOCKADDR *)&peer_info,&peer_len) < 0)
	{
		eprintf("AcceptSocketConnections getpeername failed error %i\n",
			GetLastError());
		return;
	}
	
	memcpy(&peer_addr,(long *)&(peer_info.sin_addr),sizeof(struct in_addr));
	memcpy(&conn.addr, &peer_addr, sizeof(struct in_addr));
	sprintf(conn.name,"%s",inet_ntoa(peer_addr));
	
	if (connection_type == SOCKET_MAINTENANCE_PORT)
	{
		if (!CheckMaintenanceMask(&peer_info,peer_len))
		{
			lprintf("Blocked maintenance connection from %s.\n", conn.name);
			closesocket(new_sock);
			return;
		}
	}
	else
	{
		if (!CheckBlockList(&peer_addr))
		{
			lprintf("Blocked connection from %s.\n", conn.name);
			closesocket(new_sock);
			return;
		}
	}
	
	conn.type = CONN_SOCKET;
	conn.socket = new_sock;
	
	EnterServerLock();
	
	s = CreateSession(conn);
	if (s != NULL)
	{
		StartAsyncSession(s);
		
		switch (connection_type)
		{
		case SOCKET_PORT :
			InitSessionState(s,STATE_SYNCHED);   
			break;
		case SOCKET_MAINTENANCE_PORT :
			InitSessionState(s,STATE_MAINTENANCE);
			break;
		default :
			eprintf("AcceptSocketConnections got invalid connection type %i\n",connection_type);
		}
		
		/* need to do this AFTER s->conn is set in place, because the async
		call writes to that mem address */
		
		if (ConfigBool(SOCKET_DNS_LOOKUP))
		{
			s->conn.hLookup = StartAsyncNameLookup((char *)&peer_addr,s->conn.peer_data);
		}
		else
		{
			s->conn.hLookup = 0;
		}
	}
	
	LeaveServerLock();
}

#ifdef BLAK_PLATFORM_WINDOWS
#define IN_ADDR_HELPER(x) x.S_un.S_addr
#elif BLAK_PLATFORM_LINUX
#define IN_ADDR_HELPER(x) x.s_addr
#endif

Bool CheckMaintenanceMask(SOCKADDR_IN *addr,int len_addr)
{
	IN_ADDR mask;
	int i;

	for (i=0;i<num_maintenance_masks;i++)
	{
		IN_ADDR_HELPER(mask) = inet_addr(maintenance_masks[i]);
		if (IN_ADDR_HELPER(mask) == INADDR_NONE)
		{
			eprintf("CheckMaintenanceMask has invalid configured mask %s\n",
					  maintenance_masks[i]);
			continue;
		}
	
		unsigned char b1 = (unsigned char)((IN_ADDR_HELPER(mask) & 0xff000000) >> 24);
		unsigned char b2 = (unsigned char)((IN_ADDR_HELPER(mask) & 0x00ff0000) >> 16);
		unsigned char b3 = (unsigned char)((IN_ADDR_HELPER(mask) & 0x0000ff00) >> 8);
		unsigned char b4 = (unsigned char)(IN_ADDR_HELPER(mask) & 0xff);
		/* for each byte of the mask, if it's non-zero, the client must match it */
	
		unsigned char addr_b1 = (unsigned char)((IN_ADDR_HELPER(addr->sin_addr) & 0xff000000) >> 24);
		unsigned char addr_b2 = (unsigned char)((IN_ADDR_HELPER(addr->sin_addr) & 0x00ff0000) >> 16);
		unsigned char addr_b3 = (unsigned char)((IN_ADDR_HELPER(addr->sin_addr) & 0x0000ff00) >> 8);
		unsigned char addr_b4 = (unsigned char)((IN_ADDR_HELPER(addr->sin_addr) & 0xff));

		if ((b1 != 0) && (addr_b1 != b1))
			continue;
	
		if ((b2 != 0) && (addr_b2 != b2))
			continue;
	
		if ((b3 != 0) && (addr_b3 != b3))
			continue;
	
		if ((b4 != 0) && (addr_b4 != b4))
			continue;
	
		/*
		dprintf("%u\n",addr_b1);
		dprintf("%u\n",addr_b2);
		dprintf("%u\n",addr_b3);
		dprintf("%u\n",addr_b4);
		
		  dprintf("----\n");
		  
			dprintf("%u\n",b1);
			dprintf("%u\n",b2);
			dprintf("%u\n",b3);
			dprintf("%u\n",b4);
	*/
		return True;
	}
	return False;
}

static HANDLE name_lookup_handle;
void AsyncNameLookup(HANDLE hLookup,int error)
{
	if (error != 0)
	{
		/* eprintf("AsyncSocketNameLookup got error %i\n",error); */
		return;
	}
	
	name_lookup_handle = hLookup;
	
	EnterServerLock();
	ForEachSession(AsyncEachSessionNameLookup);
	LeaveServerLock();
	
}

void AsyncEachSessionNameLookup(session_node *s)
{
	if (s->conn.type != CONN_SOCKET)
		return;
	
	if (s->conn.hLookup == name_lookup_handle)
	{
		sprintf(s->conn.name,"%s",((struct hostent *)&(s->conn.peer_data))->h_name);
		InterfaceUpdateSession(s);
	}      
}

void AsyncSocketSelect(SOCKET sock,int event,int error)
{
	session_node *s;
	
	EnterSessionLock();
	
	if (error != 0)
	{
		LeaveSessionLock();
		s = GetSessionBySocket(sock);
		if (s != NULL)
		{
		/* we can get events for sockets that have been closed by main thread
			(and hence get NULL here), so be aware! */
			
			/* eprintf("AsyncSocketSelect got error %i session %i\n",error,s->session_id); */
			
			HangupSession(s);
			return;
		}
		
		/* eprintf("AsyncSocketSelect got socket that matches no session %i\n",sock); */
		
		return;
	}
	
	switch (event)
	{
	case FD_CLOSE :
		AsyncSocketClose(sock);
		break;
		
	case FD_WRITE :
		AsyncSocketWrite(sock);
		break;
		
	case FD_READ :
		AsyncSocketRead(sock);
		break;
		
	default :
		eprintf("AsyncSocketSelect got unknown event %i\n",event);
		break;
	}
	
	LeaveSessionLock();
}

void AsyncSocketClose(SOCKET sock)
{
	session_node *s;
	
	s = GetSessionBySocket(sock);
	if (s == NULL)
		return;
	
	/* dprintf("async socket close %i\n",s->session_id); */
	HangupSession(s);
	
}

void AsyncSocketWrite(SOCKET sock)
{
	int bytes;  
	session_node *s;
	buffer_node *bn;
	
	s = GetSessionBySocket(sock);
	if (s == NULL)
		return;
	
	if (s->hangup)
		return;
	
	/* dprintf("got async write session %i\n",s->session_id); */
	if (!MutexAcquireWithTimeout(s->muxSend,10000))
	{
		eprintf("AsyncSocketWrite couldn't get session %i muxSend\n",s->session_id);
		return;
	}
	
	while (s->send_list != NULL)
	{
		bn = s->send_list;
		/* dprintf("async writing %i\n",bn->len_buf); */
		bytes = send(s->conn.socket,bn->buf,bn->len_buf,0);
		if (bytes == SOCKET_ERROR)
		{
			if (GetLastError() != WSAEWOULDBLOCK)
			{
				/* eprintf("AsyncSocketWrite got send error %i\n",GetLastError()); */
				if (!MutexRelease(s->muxSend))
					eprintf("File %s line %i release of non-owned mutex\n",__FILE__,__LINE__);
				HangupSession(s);
				return;
			}
			
			/* dprintf("got write event, but send would block\n"); */
			break;
		}
		else
		{
			if (bytes != bn->len_buf)
				dprintf("async write wrote %i/%i bytes\n",bytes,bn->len_buf);
			
			transmitted_bytes += bn->len_buf;
			
			s->send_list = bn->next;
			DeleteBuffer(bn);
		}
	}
	if (!MutexRelease(s->muxSend))
		eprintf("File %s line %i release of non-owned mutex\n",__FILE__,__LINE__);
}

void AsyncSocketRead(SOCKET sock)
{
	int bytes;
	session_node *s;
	buffer_node *bn;
	
	s = GetSessionBySocket(sock);
	if (s == NULL)
		return;
	
	if (s->hangup)
		return;
	
	if (!MutexAcquireWithTimeout(s->muxReceive,10000))
	{
		eprintf("AsyncSocketRead couldn't get session %i muxReceive",s->session_id);
		return;
	}
	
	if (s->receive_list == NULL)
	{
		s->receive_list = GetBuffer();
		/* dprintf("Read0x%08x\n",s->receive_list); */
	}
	
	// find the last buffer in the receive list
	bn = s->receive_list;
	while (bn->next != NULL)
		bn = bn->next;
	
	// if that buffer is filled to capacity already, get another and append it
	if (bn->len_buf >= bn->size_buf)
	{
		bn->next = GetBuffer();
		/* dprintf("ReadM0x%08x\n",bn->next); */
		bn = bn->next;
	}
	
	// read from the socket, up to the remaining capacity of this buffer
	bytes = recv(s->conn.socket,bn->buf + bn->len_buf,bn->size_buf - bn->len_buf,0);
	if (bytes == SOCKET_ERROR)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			/* eprintf("AsyncSocketRead got read error %i\n",GetLastError()); */
			if (!MutexRelease(s->muxReceive))
				eprintf("File %s line %i release of non-owned mutex\n",__FILE__,__LINE__);
			HangupSession(s);
			return;
		}
		if (!MutexRelease(s->muxReceive))
			eprintf("File %s line %i release of non-owned mutex\n",__FILE__,__LINE__);
	}
	if (bytes == 0)
	{
		// read of 0 bytes means it's been closed; on windows we're
		// sent a specific close event instead
		HangupSession(s);
		return;
	}


	if (bytes < 0 || bytes > bn->size_buf - bn->len_buf)
	{
		eprintf("AsyncSocketRead got %i bytes from recv() when asked to stop at %i\n",bytes,bn->size_buf - bn->len_buf);
		FlushDefaultChannels();
		bytes = 0;
	}
	
	bn->len_buf += bytes;
	
	/*
	dprintf("read %i bytes sess %i from %i\n",bytes,s->session_id,s->num_receiving,
	   s->num_receiving-bytes);
	   
		 if (s->num_receiving > 0)
		 {
		 int i;
		 dprintf("read got in %s\n",GetStateName(s));
		 for (i=s->num_receiving-bytes;i<s->num_receiving;i++)
		 dprintf("%5u",s->receiving_buf[i]);
		 dprintf("\n");
		 }
	*/ 
	if (!MutexRelease(s->muxReceive))
		eprintf("File %s line %i release of non-owned mutex\n",__FILE__,__LINE__);  
	
	SignalSession(s->session_id);
}
