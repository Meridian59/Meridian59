//-----------------------------------------------------------------------------
// 
// Sample Name: DXVB Messenger Client Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  DXVB Messenger is an instant messaging application.
  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\DXVBMessenger\Client

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\Bin

User's Guide
============
  Log onto a server, add friends, and send instant messages.

Programming Notes
=================
  * Handle DirectPlay system messages.  See implemented DirectPlay8Event interfaces
  - Upon Receive event (the following messages can be received): 
    Msg_LoginSuccess - Logged in successfully, update the UI
    Msg_InvalidPassword - The password for this account is invalid
    Msg_InvalidUser - This user doesn't exist
    Msg_UserAlreadyExists - This user already exists 

    'Friend Controls
    Msg_FriendAdded - A user was added to my list of friends
    Msg_FriendDoesNotExist - Tried to add a friend that doesn't exist
    Msg_BlockUserDoesNotExist - Tried to block a user that doesn't exist
    Msg_FriendBlocked - A user was added to my list of blocked users.
    Msg_SendClientFriends - Get the list of my friends from the server.

    Msg_UserBlocked - Can't send a message to this person, they've blocked you
    Msg_ReceiveMessage - Received a message, show the message UI and display the message

    Msg_FriendLogon - A friend has just logged on, update UI
    Msg_FriendLogoff - A friend has just logged off, update UI
