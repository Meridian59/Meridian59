//-----------------------------------------------------------------------------
// 
// Sample Name: StagedPeer Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  StagedPeer connects players together with two dialog boxes to prompt 
  users on the connection settings to join or create a session. After 
  the user connects to a sesssion, the sample displays a multiplayer stage. 
  
  The stage allows all players connected to the same session to chat, 
  and start a new game at the same time when everyone is ready and the 
  host player decides to begin.  The host player may also reject 
  players or close player slots.  This allows host player to control 
  who is allowed to join the game.  

  After a new game has started the sample begins a very simplistic 
  game called "The Greeting Game"), where players have the option of 
  send a single simple DirectPlay message to all of the other players.
  
Path
====
  Source: DXSDK\Samples\Multimedia\DirectPlay\StagedPeer 

  Executable: DXSDK\Samples\Multimedia\DirectPlay\Bin


Programming Notes
=================
  The StagedPeer sample is very similar in form to the SimplePeer sample.  For 
  detailed programming notes on the basics this sample, refer to Programming 
  Notes section of the SimplePeer sample.
  
  The StagedPeer differs by displaying a multiplayer stage using a dialog box. 
  See netstage.cpp and netstage.h for how this works.
  
