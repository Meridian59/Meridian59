//-----------------------------------------------------------------------------
// 
// Sample Name: DataRelay Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The DataRelay is similar to SimplePeer but differs by sending a single 
  target (or everyone) a packet of data with options specified in the 
  dialog's UI. 

Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\DataRelay 

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\Bin

User's Guide
============
  Host or connect to a session in the same manner as explained in SimplePeer.
  When the main dialog appears select the target, size, rate, and timeout values.
  Then click "Push to Send". This will send a packet of data to the target as
  the rate specified with the specified size.

Programming Notes
=================
  The DataRelay sample is very similar in form to the SimplePeer sample.  For 
  detailed programming notes on the basics this sample, refer to Programming 
  Notes section of the SimplePeer sample.

  The DataRelay differs by sending a single target (or everyone) a packet of 
  data with options specified in the dialog's UI. 

  When the "Push to Send" button is clicked, a timer goes off every number of 
  ms according to the UI and sends data.

  * When the timer notices it's time to send data, it does the following:
	1. Creates a byte array with the options specified in the UI.
	2. DirectPlay8Peer.SendTo is called passing in the byte array.  We 
           call SendTo with the flags DPNSEND_NOLOOPBACK.  DPNSEND_NOLOOPBACK tells
	   DirectPlay to not to send the buffer to us.
	
  * Handle DirectPlay system messages.  See implemented DirectPlay8Event interfaces
        The DataRelay handles the typical messages as described in the 
        SimplePeer programming notes, and in addition:
        
        - Upon Receive event
            1. Gets the message type from the received byte array.
            2. It then selects off the message type. 
            3. If its a MSG_GamePacket, then it adds the data to a collection,
               where a timer will notice the data and process it.  This is important
               so the DirectPlay threads stay working atfull speed.

        - Upon DPN_MSGID_SEND_COMPLETE
            1. It checks the dpnotify.hResultCode for DPNERR_TIMEDOUT.
            2. If this occurs then it updates the UI.
         
