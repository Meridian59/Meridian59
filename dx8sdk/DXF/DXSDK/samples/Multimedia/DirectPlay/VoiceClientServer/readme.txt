//-----------------------------------------------------------------------------
// 
// Sample Name: VoiceClientServer Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  VoiceClientServer is similar in form to SimpleClientServer, but 
  shows how to add DirectPlay Voice support. 

Path
====
  Source: DXSDK\Samples\Multimedia\DirectPlay\VoiceClientServer 

  Executable: DXSDK\Samples\Multimedia\DirectPlay\Bin

User's Guide
============
  Refer to User's Guide section of the SimpleClientServer sample 
  for the basics.
  
  In addition to what SimpleClientServer does, on the server
  you can set codec to use, and set the server to be either a 
  Forwarding server or Mixing server.  See the DirectX docs
  for more information about these server types.
  
  On the client side, once the chat session has begin any client 
  may alter the playback or capture settings by clicking on "Setup".  
