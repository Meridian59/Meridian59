//-----------------------------------------------------------------------------
// 
// Sample Name: VB Chat Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  Chat is similar in form to SimplePeer.  Once a player hosts or connects
  to a session, the players can chat with either other by passing text 
  strings.
  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\Chat

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\Bin

User's Guide
============
  Refer to User's Guide section of the SimplePeer sample.

Programming Notes
=================
  The ChatPeer sample is very similar in form to the SimplePeer sample.  For 
  detailed programming notes on the basics this sample, refer to Programming 
  Notes section of the SimplePeer sample.

  The ChatPeer differs by letting clients send text strings to all players
  connected to the session.
  
  * The <Enter> key is pressed.  See txtSend_KeyPress.
        1. Retrieves the text string from the dialog.
        2. Fills out a byte array using the string.
        3. Calls DirectPlay8Peer.SendTo with the byte array.  It passes 
           DPNID_ALL_PLAYERS_GROUP so this message goes to everyone. 
   
  * The "Whisper" button is pressed.  See cmdWhisper_Click.
        1. Retrieves the text string from the dialog.
        2. Fills out a byte array using the string.
        3. Calls DirectPlay8Peer.SendTo with the byte array.  It passes 
           the DPNID of the player so this message only goes to the person you
           are whispering too. 

  * Handle DirectPlay system messages.  See implemented DirectPlay8Event interfaces

        The Chat sample handles the typical messages as described in the 
        SimplePeer programming notes, and in addition:
        
        - Upon Receive event: 
            *Checks if this is a chat message or a whisper message.
            *Retrieve the string from the byte array we receive.
            *Update the UI accordingly.
