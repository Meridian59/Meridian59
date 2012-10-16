//-----------------------------------------------------------------------------
// 
// Sample Name: VB Conferencer Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  Conferencer is similar in form to MS Netmeeting (tm).
  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\Conferencer

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectPlay\Bin

User's Guide
============
  You may 'call' friends in this application, and then share files, 
  share a whiteboard, use voice chat, and text chat.

Programming Notes
=================

Here are the messages this application responds to:

MsgChat - This message is received when someone is chatting (text) in 
     the chat window.

MsgWhisper - This message is received when someone is whispering (text) in 
     the chat window.

MsgAskToJoin - When a user attempts to contact you, this message is sent, 
     which will pop up a dialog that enables the person receiving the
     event the opportunity to deny this person from joining.

MsgAcceptJoin - You will to allow this person to join your session.

MsgRejectJoin - You will not allow this person to join your session.

MsgCancelCall - You no longer want to wait for the session you are attempting to 
     join to either accept or deny you.

MsgShowChat - Someone has opened the text chat window, and wants this session to 
     do the same.

MsgSendFileRequest - Request a file transfer

MsgSendFileAccept - Accept the file transfer

MsgSendFileDeny - Deny the file transfer

MsgSendFileInfo - File information (size)

MsgSendFilePart - Send a chunk of the file

MsgAckFilePart - Acknowledge the file part

MsgSendDrawPixel - Send a drawn pixel (in the whiteboard window, including color)

MsgSendDrawLine - Send a drawn line (in the whiteboard window, including color)

MsgShowWhiteBoard - Show the whiteboard window

MsgClearWhiteBoard - Clear the contents of the whiteboard
