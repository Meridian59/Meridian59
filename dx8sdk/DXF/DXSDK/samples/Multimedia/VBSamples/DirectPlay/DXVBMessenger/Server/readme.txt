//-----------------------------------------------------------------------------
// 
// Sample Name: DXVB Messenger Server Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========

  DXVB Messenger Server is the server portion of a client/server instant 
  messaging application.  This sample requires the use of XML to maintain the 
  data needed.  You must have at least IE5 installed on your machine to compile 
  or run this sample.  
  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\DXVBMessenger\Server

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\Bin

User's Guide
============
  Log onto a server, add friends, and send instant messages.

Programming Notes
=================
  * Handle DirectPlay system messages.  See implemented DirectPlay8Event interfaces
  - Upon Receive event (the following messages can be received): 
    'Login messages
    Msg_Login - Get login information, verify against the database
    Msg_CreateNewAccount - A new account needs to be created, try to create in database
    Msg_AddFriend - Add a friend to this users list
    Msg_BlockFriend - Block someone in this users list
    Msg_SendMessage - User is sending a message to someone, pass it on
