//-----------------------------------------------------------------------------
// 
// Sample Name: DataRelay Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The DataRelay is similar to SimplePeer but differs by sending a single 
  target (or everyone) a packet of data with options specified in the 
  dialog's UI. It uses a worker thread to process received data, and 
  uses the ReturnBuffer() API so that no copying of the received buffers
  is done.

Path
====
  Source: DXSDK\Samples\Multimedia\DirectPlay\DataRelay 

  Executable: DXSDK\Samples\Multimedia\DirectPlay\Bin

User's Guide
============
  Host or connect to a session in the same manner as explained in SimplePeer.
  When the main dialog appears select the target, size, rate, and timeout values.
  Then click "Push to Send". This will send a packet of data to the target as
  the rate specified with the specified size.  Use the "Connection Info" dropdown 
  to specify a target to gather connection info on periodically.

Programming Notes
=================
  The DataRelay sample is very similar in form to the SimplePeer sample.  For 
  detailed programming notes on the basics this sample, refer to Programming 
  Notes section of the SimplePeer sample.

  The DataRelay differs by sending a single target (or everyone) a packet of 
  data with options specified in the dialog's UI. 

  When the "Push to Send" button is clicked, then win32 timer is created 
  that goes off every number of ms according to the UI.

  * Upon the WM_TIMER labeled TIMERID_NETWORK, it calls SendNetworkData().
	1. It creates a app defined struct, GAMEMSG_DATA_xxx on the heap.  
	   The struct derives from GAMEMSG_GENERIC.  GAMEMSG_GENERIC contains a 
	   packet type field so that the reciever and identify this app defined 
	   packet, and it contains a packet ID field.  This field is sequential 
	   and is displayed to the user whenever a packet is received.  The struct 
	   is created on the heap, since will use the DPNSEND_NOCOPY when calling 
	   SendTo() below.  The struct is filled with random data, however a real 
	   app would typically send player and world state data here. 	   
	2. It then creates a GAMEMSG_DATA_NODE which is then handed off to the
 	   app worker thread.  That thread will process the node, and
	   then update the UI to show that a packet was sent.  It is handed off
	   by adding the node to a linked list.  Since the worker thread also
	   accesses the linked list, we enter a critical section before adding the
	   node and leave it afterward.
	3. A DPN_BUFFER_DESC is then filled out passing in a pointer to the 
       app defined struct created above. 
	4. IDirectPlay8Peer::SendTo is called passing in the DPN_BUFFER_DESC, thereby
	   sending the app defined struct, GAMEMSG_DATA_xxx.  We call SendTo with
	   the flags DPNSEND_NOLOOPBACK | DPNSEND_NOCOPY.  DPNSEND_NOLOOPBACK tells
	   DirectPlay to not to send the buffer to us, and DPNSEND_NOCOPY means that
	   DirectPlay should not copy the buffer.  When the DPNSEND_NOCOPY is used,
	   the app itself owns the buffer, and the buffer must be on the heap.
	   When the DPN_MSGID_SEND_COMPLETE comes in, we will delete the buffer. 	    
	5. The event, g_hDPDataAvailEvent, is set telling the worker thread that 
	   there is data (a message to say that a packet was sent), is ready to 
	   be processed now.
	
  * Handle DirectPlay system messages.  See DirectPlayMessageHandler()
        The DataRelay handles the typical messages as described in the 
        SimplePeer programming notes, and in addition:
        
        - Upon DPN_MSGID_RECEIVE
            1. It casts the pReceiveMsg->pReceiveData to a GAMEMSG_GENERIC*.
            2. It then switches off the GAMEMSG_GENERIC's dwType. 
            3. If its a GAME_MSGID_GAMEPACKET, then it creates and fills out
               a GAMEMSG_DATA_NODE.  This node is then handed to a worker thread
               so it can be processed outside of the DirectPlay message handler.
               This is important since it keeps the DirectPlay threads working at
               full speed.
            4. After the node is added to the linked list using a critical section
               to lock, it returns DPNSUCCESS_PENDING.  This is important since 
               it tells DirectPlay that ownership of the buffer 
               has been transferred to the application, and so DirectPlay will 
               neither free nor modify it until ownership is returned 
               to DirectPlay through the ReturnBuffer() call.

        - Upon DPN_MSGID_SEND_COMPLETE
            1. It checks the pSendCompleteMsg->hResultCode for DPNERR_TIMEDOUT.
            2. If this occurs then it creates a new GAMEMSG_DATA_NODE with dwType
               set to DATA_TYPE_NETPACKET_TIMEOUT.  This will is passed to the 
               worker thread.  The worker thread will process this node, and 
               post a message to the dialog saying that the message timed out.
               A realistic application would want to take the appropriate steps here,
               such as resending new data or other measures.
            3. It deletes the buffer from the heap since we specified, DPNSEND_NOCOPY,
               so the buffer on the heap belows the app and it must clean it up. 
         
   * The worker thread.  See ProcessNetDataProc()
        - Upon the g_hDPDataAvailEvent
            1. When the event is signaled, then new data can be found in the 
               linked list, g_DataHead.  So it calls ProcessData().
            2. ProcessData() first enters the critical section, g_csDataList, so 
               that the other threads don't modify the linked list while it is 
               processing data.  It leaves this critical section at the end of the
               loop.  Typically better locking mechanisms would be used so that the 
               other threads (as well as the DirectPlay message handler threads) 
               aren't blocked while data is processed on this thread.               
            3. It runs through the linked list, processing each node.  For this
               simple sample all it does is posting a message to the dialog thread,
               containing a string for the dialog to display.
            4. After it is done processing the node, it calls 
               IDirectPlay8Peer::ReturnBuffer() so that DirectPlay can free
               buffer that it passed us in DPN_MSGID_RECEIVE.
             
               
            
	  
	   
	
	
	

