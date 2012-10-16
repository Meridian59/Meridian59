//-----------------------------------------------------------------------------
// 
// Sample Name: VB Simple Peer Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  Once a player hosts or connects to a session, the players can make funny faces.
  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\SimplePeer

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\Bin

User's Guide
============
  Select the protocol to use, enter your name.  Choose to search for sessions or create
  your own, or to wait for a lobby session.  Once connected, make funny faces.

Programming Notes
=================
  * The "Make Face" key is pressed.  See cmdMakeFace_Click.
        1. Fills out a byte array using 1 since this is the only message to send.
        2. Calls DirectPlay8Peer.SendTo with the byte array.  It passes 
           DPNID_ALL_PLAYERS_GROUP so this message goes to everyone. 
   
  * Handle DirectPlay system messages.  See implemented DirectPlay8Event interfaces

        The Chat sample handles the typical messages as described in the 
        SimplePeer programming notes, and in addition:
        
        - Upon Receive event: 
            *Get the name of the person sending the event, and update UI
