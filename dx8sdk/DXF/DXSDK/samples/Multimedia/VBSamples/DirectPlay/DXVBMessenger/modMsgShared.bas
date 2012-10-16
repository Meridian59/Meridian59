Attribute VB_Name = "modMsgShared"
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       modMsgShared.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'Constant encryption keys for both the server and client
Public Const glClientSideEncryptionKey As Long = 169
Public Const glServerSideEncryptionKey As Long = 131

'Unique GUID for the app (used by DPlay)
Public Const AppGuid = "{0AC3AAC4-5470-4cc0-ABBE-6EF0B614E52A}"
'Host and connect on this port
Public Const glDefaultPort As Long = 9123

'System Tray Declares/Constants/Types
Public Declare Function Shell_NotifyIcon Lib "shell32.dll" Alias "Shell_NotifyIconA" (ByVal dwMessage As Long, lpData As NOTIFYICONDATA) As Long
Public Type NOTIFYICONDATA
    cbSize As Long
    hwnd As Long
    uID As Long
    uFlags As Long
    uCallbackMessage As Long
    hIcon As Long
    sTip As String * 64
End Type
Public Const NIM_ADD = &H0
Public Const NIM_MODIFY = &H1
Public Const NIM_DELETE = &H2
Public Const NIF_MESSAGE = &H1
Public Const NIF_ICON = &H2
Public Const NIF_TIP = &H4
Public Const NIF_DOALL = NIF_MESSAGE Or NIF_ICON Or NIF_TIP
Public Const WM_MOUSEMOVE = &H200
Public Const WM_LBUTTONDBLCLK = &H203
Public Const WM_RBUTTONUP = &H205
Public sysIcon As NOTIFYICONDATA

Public Enum vbMessengerMsgTypes
    'Login messages
    Msg_Login 'Login information
    Msg_LoginSuccess 'Logged in successfully
    Msg_CreateNewAccount 'A new account needs to be created
    Msg_InvalidPassword 'The password for this account is invalid
    Msg_InvalidUser 'This user doesn't exist
    Msg_UserAlreadyExists 'This user already exists (only can be received after a CreateNewAcct msg)
    'Friend Controls
    Msg_AddFriend 'Add a friend to my list
    Msg_FriendAdded 'User was added
    Msg_FriendDoesNotExist 'Tried to add a friend that doesn't exist
    Msg_BlockUserDoesNotExist 'Tried to block a user that doesn't exist
    Msg_BlockFriend 'Block someone from contacting me
    Msg_FriendBlocked 'User was blocked
    Msg_DeleteFriend 'Delete this user from my list of friends
    Msg_SendClientFriends 'The Server will send the client it's list of friends
    'Messages
    Msg_SendMessage 'Send a message to someone
    Msg_UserBlocked 'Can't send a message to this person, they've blocked you
    Msg_ReceiveMessage 'Received a message
    Msg_UserUnavailable 'The user you are trying to send a message to is no longer logged on
    'Friend Logon messages
    Msg_FriendLogon 'A friend has just logged on
    Msg_FriendLogoff 'A friend has just logged off
End Enum

'Here we will use a very basic encrytion scheme.  We will encrypt the password
'on the client side, before we send it over to the server, and then decrypt it
'on the server side, and encrypt it once more before checking it against the database
Public Function EncodePassword(sOldStr As String, ByVal lEncryptKey) As String

    Dim lCount As Long, sNew As String
    
    'Do a simple replace on each character in the string
    For lCount = 1 To Len(sOldStr)
        sNew = sNew & Chr$(Asc(Mid$(sOldStr, lCount, 1)) Xor lEncryptKey)
    Next
    EncodePassword = sNew
End Function
