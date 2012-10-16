//-----------------------------------------------------------------------------
// 
// Sample Name: VB Simple Client Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  A very simplistic Client application that can only connect to a server, and 
  make funny faces to other players on that server.
  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\SimpleClient

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\Bin

User's Guide
============
  Connect to a server, and make funny faces.

Programming Notes
=================
  The SimpleClient sample allows players to make funny faces at anyone else on the server.
  
  * The "Make Faces" button is pressed.  See cmdFace_Click.
        1. Fills out a byte array using nothing, since all we can do is make faces.
        2. Calls DirectPlay8Client.Send with the byte array.  The server will receive
           this message and handle it. 
   

  * Handle DirectPlay system messages.  See implemented DirectPlay8Event interfaces

  - Upon Receive event: 
  Checks message type, and if the following:
   Msg_NumPlayers - Change the number of current and max players in this session, update UI
   Msg_NoOtherPlayers - Notify the client there are no other players in the session
   Msg_SendWave - Notify that someone made a funny face at us.
