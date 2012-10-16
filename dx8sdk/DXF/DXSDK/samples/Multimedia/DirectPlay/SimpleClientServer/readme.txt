//-----------------------------------------------------------------------------
// 
// Sample Name: SimpleClientServer Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The SimpleClientServer sample is a simple client/server application. 
  It is similar in form to SimplePeer, but using the client/server interfaces.  
  
  The game itself is very simple, passing a single DirectPlay message to all 
  connected players when the "Wave To other players" button is pressed.

Path
====
  Source: DXSDK\Samples\Multimedia\DirectPlay\SimpleClientServer 

  Executable: DXSDK\Samples\Multimedia\DirectPlay\Bin

User's Guide
============
  Start the SimpleServer.  Change the port if desired, and click "Start Server".  
  
  Start the SimpleClient.  Enter the player's name, and type in the IP address 
  or leave it blank to search the subnet.  Then click "Start Search".  Click 
  join when a session appears.
    
  After game has been joined or created, the game begins immediately.  Other players 
  may join the game at any time.  

