//-----------------------------------------------------------------------------
// 
// Sample Name: VB Memory Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  Memory is a simple game in which you match 'tiles' and try to score the most 
  points.
  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\Memory

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\Bin

User's Guide
============
  Choose Multiplayer or Solitaire, and then just select images and try to 
  get a match.  Score one point for every match.  In multiplayer mode try to
  get more matches than your opponent(s).  In solitaire mode, try to clear the 
  board in as few attempts as possible.

Programming Notes
=================
  The Memory sample is very similar in form to the SimplePeer sample.  For 
  detailed programming notes on the basics this sample, refer to Programming 
  Notes section of the SimplePeer sample.

  * An image is clicked.
        1. If it's your turn, show the picture.
        2. Fills out a byte array telling everyone else to show the picture.
        3. Calls DirectPlay8Peer.SendTo with the byte array.  It passes 
           DPNID_ALL_PLAYERS_GROUP so this message goes to everyone. 
   

  * Handle DirectPlay system messages.  See implemented DirectPlay8Event interfaces

        The Chat sample handles the typical messages as described in the 
        SimplePeer programming notes, and in addition:
        
        - Upon Receive event: 
            *Checks the type of this message.
            *Selects off of the message type.
            *If MSG_SETUPBOARD, retreive board information and udpate local state.
            *If MSG_SHOWPIECE, show the indicated piece.
            *If MSG_HIDEPIECES, hide all pieces that aren't currently matched.
            *If MSG_TURNEND, advance to the next players turn.
            *If MSG_CHAT, receive a text string, and update the UI.
            *If MSG_MATCHED, set the state so these two pictures are matched.
