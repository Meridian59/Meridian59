VERSION 5.00
Begin VB.Form frmChat 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "vbDirectPlay Chat"
   ClientHeight    =   5085
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   7695
   Icon            =   "frmChat.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5085
   ScaleWidth      =   7695
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdWhisper 
      Caption         =   "Whisper"
      Height          =   255
      Left            =   5820
      TabIndex        =   3
      Top             =   4740
      Width           =   1695
   End
   Begin VB.TextBox txtSend 
      Height          =   285
      Left            =   60
      TabIndex        =   0
      Top             =   4740
      Width           =   5595
   End
   Begin VB.ListBox lstUsers 
      Height          =   4545
      Left            =   5760
      TabIndex        =   2
      Top             =   120
      Width           =   1815
   End
   Begin VB.TextBox txtChat 
      Height          =   4635
      Left            =   60
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   3  'Both
      TabIndex        =   1
      TabStop         =   0   'False
      Top             =   60
      Width           =   5595
   End
End
Attribute VB_Name = "frmChat"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmChat.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Implements DirectPlay8Event

Private Sub cmdWhisper_Click()
    Dim lMsg As Long, lOffset As Long
    Dim sChatMsg As String
    Dim oBuf() As Byte
    
    If lstUsers.ListIndex < 0 Then
        MsgBox "You must select a user in the list before you can whisper to that person.", vbOKOnly Or vbInformation, "Select someone"
        Exit Sub
    End If
    
    If lstUsers.ItemData(lstUsers.ListIndex) = 0 Then
        MsgBox "Why are you whispering to yourself?", vbOKOnly Or vbInformation, "Select someone else"
        Exit Sub
    End If
    
    If txtSend.Text = vbNullString Then
        MsgBox "What's the point of whispering if you have nothing to say..", vbOKOnly Or vbInformation, "Enter text"
        Exit Sub
    End If
        
    'Send this message to the person you are whispering to
    lMsg = MsgWhisper
    lOffset = NewBuffer(oBuf)
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    sChatMsg = txtSend.Text
    AddStringToBuffer oBuf, sChatMsg, lOffset
    txtSend.Text = vbNullString
    dpp.SendTo lstUsers.ItemData(lstUsers.ListIndex), oBuf, 0, DPNSEND_NOLOOPBACK
    UpdateChat "**<" & gsUserName & ">** " & sChatMsg
    
End Sub

Private Sub Form_Load()

    'Oh good, we want to play a multiplayer game.
    'First lets get the dplay connection started
    
    'Here we will init our DPlay objects
    InitDPlay
    'Now we can create a new Connection Form (which will also be our message pump)
    Set DPlayEventsForm = New DPlayConnect
    'Start the connection form (it will either create or join a session)
    If Not DPlayEventsForm.StartConnectWizard(dx, dpp, AppGuid, 20, Me) Then
        Cleanup
        End
    Else 'We did choose to play a game
        gsUserName = DPlayEventsForm.UserName
        If DPlayEventsForm.IsHost Then
            Me.Caption = Me.Caption & " (HOST)"
        End If
    End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Me.Hide
    DPlayEventsForm.DoSleep 50
    Cleanup
End Sub

Private Sub UpdateChat(ByVal sString As String)
    'Update the chat window first
    txtChat.Text = txtChat.Text & sString & vbCrLf
    'Now limit the text in the window to be 16k
    If Len(txtChat.Text) > 16384 Then
        txtChat.Text = Right$(txtChat.Text, 16384)
    End If
    'Autoscroll the text
    txtChat.SelStart = Len(txtChat.Text)
End Sub

Private Sub txtSend_KeyPress(KeyAscii As Integer)
    Dim lMsg As Long, lOffset As Long
    Dim sChatMsg As String
    Dim oBuf() As Byte
    
    If KeyAscii = vbKeyReturn Then
        If txtSend.Text <> vbNullString Then 'Make sure they are trying to send something
            'Send this message to everyone
            lMsg = MsgChat
            lOffset = NewBuffer(oBuf)
            AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
            sChatMsg = txtSend.Text
            AddStringToBuffer oBuf, sChatMsg, lOffset
            txtSend.Text = vbNullString
            KeyAscii = 0
            dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
            UpdateChat "<" & gsUserName & ">" & sChatMsg
        End If 'We won't set KeyAscii to 0 here, because if they are trying to
               'send blank data, we don't care about the ding for hitting enter on
               'an empty line
    End If
End Sub

Private Function GetName(ByVal lID As Long) As String
    Dim lCount As Long
    
    GetName = vbNullString
    For lCount = 0 To lstUsers.ListCount - 1
        If lstUsers.ItemData(lCount) = lID Then 'This is the player
            GetName = lstUsers.List(lCount)
            Exit For
        End If
    Next
End Function

Private Sub DirectPlay8Event_AddRemovePlayerGroup(ByVal lMsgID As Long, ByVal lPlayerID As Long, ByVal lGroupID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_AppDesc(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_AsyncOpComplete(dpnotify As DxVBLibA.DPNMSG_ASYNC_OP_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_ConnectComplete(dpnotify As DxVBLibA.DPNMSG_CONNECT_COMPLETE, fRejectMsg As Boolean)
    If dpnotify.hResultCode <> 0 Then
        'For some reason we could not connect.  All available slots must be closed.
        MsgBox "Connect Failed.  Error: 0x" & CStr(Hex$(dpnotify.hResultCode)) & "  - This sample will now close.", vbOKOnly Or vbCritical, "Closing"
        DPlayEventsForm.CloseForm Me
    End If
End Sub

Private Sub DirectPlay8Event_CreateGroup(ByVal lGroupID As Long, ByVal lOwnerID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_CreatePlayer(ByVal lPlayerID As Long, fRejectMsg As Boolean)
    Dim dpPeer As DPN_PLAYER_INFO
    dpPeer = dpp.GetPeerInfo(lPlayerID)
        
    'Add this person to chat (even if it's me)
    lstUsers.AddItem dpPeer.Name
    If (dpPeer.lPlayerFlags And DPNPLAYER_LOCAL) <> DPNPLAYER_LOCAL Then 'this isn't me, someone just joined
        UpdateChat "- " & dpPeer.Name & " is chatting"
        'If it's not me, include an ItemData
        lstUsers.ItemData(lstUsers.ListCount - 1) = lPlayerID
    End If
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    Dim lCount As Long
    
    'We only care when someone leaves.  When they join we will receive a 'MSGJoin'
    'Remove this player from our list
    For lCount = 0 To lstUsers.ListCount - 1
        If lstUsers.ItemData(lCount) = lPlayerID Then 'This is the player
            UpdateChat "-- " & lstUsers.List(lCount) & " is no longer chatting."
            lstUsers.RemoveItem lCount
            Exit For
        End If
    Next
End Sub

Private Sub DirectPlay8Event_EnumHostsQuery(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_QUERY, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_EnumHostsResponse(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_RESPONSE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_HostMigrate(ByVal lNewHostID As Long, fRejectMsg As Boolean)
    Dim dpPeer As DPN_PLAYER_INFO
    dpPeer = dpp.GetPeerInfo(lNewHostID)
    If (dpPeer.lPlayerFlags And DPNPLAYER_LOCAL) = DPNPLAYER_LOCAL Then 'I am the new host
        Me.Caption = Me.Caption & " (HOST)"
    End If
End Sub

Private Sub DirectPlay8Event_IndicateConnect(dpnotify As DxVBLibA.DPNMSG_INDICATE_CONNECT, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_IndicatedConnectAborted(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_InfoNotify(ByVal lMsgID As Long, ByVal lNotifyID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_Receive(dpnotify As DxVBLibA.DPNMSG_RECEIVE, fRejectMsg As Boolean)
    'process what msgs we receive.
    Dim lMsg As Long, lOffset As Long
    Dim dpPeer As DPN_PLAYER_INFO, sName As String
    Dim sChat As String
    
    With dpnotify
    GetDataFromBuffer .ReceivedData, lMsg, LenB(lMsg), lOffset
    Select Case lMsg
    Case MsgChat
        sName = GetName(.idSender)
        sChat = GetStringFromBuffer(.ReceivedData, lOffset)
        UpdateChat "<" & sName & "> " & sChat
    Case MsgWhisper
        sName = GetName(.idSender)
        sChat = GetStringFromBuffer(.ReceivedData, lOffset)
        UpdateChat "**<" & sName & ">** " & sChat
    End Select
    End With
    
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    If dpnotify.hResultCode = DPNERR_HOSTTERMINATEDSESSION Then
        MsgBox "The host has terminated this session.  This sample will now exit.", vbOKOnly Or vbInformation, "Exiting"
    Else
        MsgBox "This session has been lost.  This sample will now exit.", vbOKOnly Or vbInformation, "Exiting"
    End If
    DPlayEventsForm.CloseForm Me
End Sub

